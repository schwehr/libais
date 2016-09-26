"""Manage a stream of NMEA TAG block messages.

The TAG (Transport, Annotate, and Group) Block messages are defined in NMEA 4.0
section 7.

Some providers violate the NMEA standard by using lower case letters in NMEA
checksums.  This module forces checksums to always be upper case.

TODO(schwehr): Add a queue method to drop old groups caches.
TODO(schwehr): Catch a wider variety of incomplete groups.
TODO(schwehr): Compute running stats in the queue.
"""

import hashlib
import logging
import re

import six
import six.moves.queue as Queue

import ais
from ais import nmea
from ais import nmea_messages
from ais import util
from ais import vdm

logger = logging.getLogger('libais')

# Added a decimal value to time beyond the normal TAG BLOCK spec.
TAG_BLOCK_RE = re.compile(r"""
(\\
(?P<metadata>(
  (
    c:(?P<time>\d{10,15}(\.\d*)?) |  # Receiver Unix time in seconds or millisec
    d:(?P<dest>[^*,\\]{1,15}) |  # Destination
    g:(?P<group>(?P<sentence_num>\d)-(?P<sentence_tot>\d)-(?P<group_id>\d+)) |
    n:(?P<line_num>\d+) |  # Line count
    q:(?P<quality>\w) |  # Orbcomm specific character code
    r:(?P<rel_time>\d+) | # Relative time
    s:(?P<rcvr>[^$*,!\\]{1,15}) |  # Source / station
    t:(?P<text>[^$*,!\\]+) |  # Text string
    T:(?P<text_date>[^$*,!\\]+)  # Orbcomm human readable date
  )[,]?
)+([*](?P<tag_checksum>[0-9A-Fa-f]{2}))?)
\\)(?P<payload>.*)
""", re.VERBOSE)

NUMERIC_FIELDS = (
  'dest',
  'group',
  'group_id',
  'line_num',
  'rel_time',
  'sentence_num',
  'sentence_tot',
  'time'
)


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

  result.update({k: util.MaybeToNumber(v)
                 for k, v in six.iteritems(result) if k in NUMERIC_FIELDS})

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
      Queue.Queue.put(self, {
          'line_nums': [self.line_num],
          'lines': [line],
      })
      return

    time = util.MaybeToNumber(match['time'])

    if not match['group']:
      msg = {
          'line_nums': [self.line_num],
          'lines': [line],
          'matches': [match],
          'times': [time],
      }
      decoded = DecodeTagSingle(msg)
      if decoded:
        msg['decoded'] = decoded
      else:
        logger.info('Unable to decode. Passing without decoded block.')
        decoded = nmea_messages.DecodeLine(match['payload'])
        if decoded:
          msg['decoded'] = decoded
        else:
          logger.info('No NMEA match for line: %d, %s', line_num, line)
      Queue.Queue.put(self, msg)
      return

    sentence_num = int(match['sentence_num'])
    sentence_total = int(match['sentence_tot'])
    group_id = int(match['group_id'])

    if sentence_num == 1:
      self.groups[group_id] = {
          'line_nums': [self.line_num],
          'lines': [line],
          'matches': [match],
          'times': [time],
      }
      return

    if group_id not in self.groups:
      logger.error('group_id not in groups: %d', group_id)
      return

    entry = self.groups[group_id]
    entry['line_nums'].append(self.line_num)
    entry['lines'].append(line)
    entry['matches'].append(match)
    entry['times'].append(time)

    if sentence_num != sentence_total:
      # Found the middle part of a message.
      return

    # Found the final message in a group.
    decoded = DecodeTagMultiple(entry)
    if decoded:
      entry['decoded'] = decoded
    else:
      logger.info('Unable to process: %s', entry)
    Queue.Queue.put(self, entry)
    self.groups.pop(group_id)


def DecodeTagSingle(tag_block_message):
  """Decode the payload of one (but NOT more) NMEA TAG block.

  Args:
    tag_block_message: dict, A dictionary with a matches entry.

  Returns:
    A message dictionary compatible with vdm.BareQueue.
  """
  line = tag_block_message['matches'][0]['payload']
  match = vdm.Parse(line)
  if not match:
    logger.info('Single line NMEA TAG block decode failed for: %s',
                 tag_block_message)
    return

  sentence_total = int(match['sen_tot'])
  if sentence_total != 1:
    logger.error('Multi-line message? %s', tag_block_message)
    return

  body = match['body']
  fill_bits = int(match['fill_bits'])
  try:
    decoded = ais.decode(body, fill_bits)
  except ais.DecodeError as error:
    logger.error('Unable to decode: %s', error)
    return

  decoded['md5'] = hashlib.md5(body.encode('utf-8')).hexdigest()
  return decoded


def DecodeTagMultiple(tag_block_message):
  """Decode a TAG block message that spans multiple lines."""
  payloads = [msg['payload'] for msg in tag_block_message['matches']]

  q = vdm.BareQueue()
  for line in vdm.VdmLines(payloads):
    q.put(line)
  if q.qsize() != 1:
    logger.info('Error: Should get just one message decoded from this: %s',
                 tag_block_message)
    return
  msg = q.get()
  return msg['decoded']
