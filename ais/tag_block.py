"""Manage a stream of NMEA TAG block messages.

The TAG (Transport, Annotate, and Group) Block messages are defined in NMEA 4.0
section 7.

Some providers violate the NMEA standard by using lower case letters in NMEA
checksums.  This module forces checksums to always be upper case.

TODO(schwehr): Add a queue method to drop old groups caches.
TODO(schwehr): Catch a wider variety of incomplete groups.
TODO(schwehr): Compute running stats in the queue.
"""
import Queue
import re

from . import nmea
from . import util


# Added a decimal value to time beyond the normal TAG BLOCK spec.
TAG_BLOCK_RE = re.compile(r"""
(\\
(?P<metadata>(
  (
    c:(?P<time>\d{10,15}(\.\d*)?) | # Receiver Unix time in seconds or millisec
    d:(?P<dest>[^*,\\]{1,15}) | # Destination
    g:(?P<group>(?P<sentence_num>\d)-(?P<sentence_tot>\d)-(?P<group_id>\d+)) |
    n:(?P<line_num>\d+) | # Line count
    q:(?P<quality>\w) | # Orbcomm specific character code
    r:(?P<rel_time>\d+) | # Relative time
    s:(?P<rcvr>[^$*,!\\]{1,15}) | # Source / station
    t:(?P<text>[^$*,!\\]+) | # Text string
    T:(?P<text_date>[^$*,!\\]+) # Orbcomm human readable date
  )[,]?
)+([*](?P<tag_checksum>[0-9A-Fa-f]{2}))?)
\\)(?P<payload>.*)
""", re.VERBOSE)


def Parse(data):
  """Unpack a TAG Block line or return None.

  Makes sure that the line matches the regex and the checksum matches.

  Args:
    data: Line of text or a dict from at TAG_BLOCK_RE.

  Returns:
    A NMEA TAG Block dict or None if the line would not parse or has
    an invalid checksum.
  """
  if isinstance(data, str):
    try:
      result = TAG_BLOCK_RE.search(data).groupdict()
    except AttributeError:
      return
  elif isinstance(data, dict):
    result = data
  else:
    return

  actual = nmea.Checksum(result['metadata'])
  expected = result['tag_checksum'].upper()
  if actual != expected:
    return

  return result


class TagQueue(Queue.Queue):
  """Aggregate TAG Block group messages into logical units.

  This queue tracks message lines with the "g" group TAG and finds matching
  lines.  It will pass single line messages straight through.
  """

  def __init__(self):
    self.groups = {}
    self.line_num = 0
    Queue.Queue.__init__(self)

  def put(self, line, line_num=None):
    if line_num is not None:
      self.line_num = line_num
    else:
      self.line_num += 1

    line = line.rstrip()
    match = Parse(line)
    if not match:
      # Pass unknown lines through.
      Queue.Queue.put(self, {
          'line_num': [self.line_num],
          'lines': [line],
      })
      return

    msg = {
        'line_num': [self.line_num],
        'lines': [line],
        'matches': [match],
        'times': [util.MaybeToNumber(match['time'])],
    }

    if not match['group']:
      Queue.Queue.put(self, msg)
      return

    sentence_num = int(match['sentence_num'])
    sentence_total = int(match['sentence_tot'])
    group_id = int(match['group_id'])

    if sentence_num == 1:
      self.groups[group_id] = [msg]
      return

    if group_id not in self.groups:
      # Just drop the message.
      return

    self.groups[group_id].append(msg)

    if sentence_num != sentence_total:
      # Incomplete - middle part of a message.
      return

    # Found the final message in a group.
    Queue.Queue.put(self, self.groups[group_id])
    self.groups.pop(group_id)
