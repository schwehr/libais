# pylint: disable=line-too-long
"""Handle AIS VD[MO] messages without extra metadata.

An example single line message:

  !AIVDM,1,1,,A,14VIk0002sMM04vE>V9jGimn08RP,0*0D

An example two line message:

  !AIVDM,2,1,2,B,5KNp?6@00000=Phb220u1@Tlv0TV22222222220N0h:22400000000000000,0*2A
  !AIVDM,2,2,2,B,00000000000,2*25'

The Parse class calculates the checksum and compares that to expected
checksum at the end of the message.  It then uses the VDM_RE regular
expression to create a dictionary for a single line.  These
dictionaries are used by the BareQueue class to construct a queue of
complete AIS messages.  It also passes non AIS VDM messages through.
Invalid AIS messages are dropped.

The non AIS VDM messages have a simpler data structure like this:

  {
      'line_nums': [42],
      'lines': ['A line of text']
  }

A single line AIS VDM message adds a 'decoded' dictionary with the
unpacked message and a list of matches for each of the lines that came
in.  Additionally, the queue adds the md5 hash of the body of the
message to assist with the process of looking for duplicate messages.

  {
        'line_nums': [1],
        'lines': ['!SAVDM,1,1,,B,K8VSqb9LdU28WP8P,0*7B'],
        'decoded': {
            'md5': '6e1a4872825054e91ee7cfcfb9cc87e0',
            'id': 27,
            'mmsi': 577305000,
            # SNIP
            },
        'matches': [{
            'body': 'K8VSqb9LdU28WP8P',
            'fill_bits': '0',
            'sen_num': '1',
            'sen_tot': '1',
            'vdm_type': 'VDM',
            'talker': 'SA',
            'chan': 'B',
            'seq_id': None,
            'checksum': '7B'}]
  }

TODO(schwehr): Make sure that multi-line messages are coming in on the
               same channel.

See Also:
  http://catb.org/gpsd/AIVDM.html
  http://www.itu.int/rec/R-REC-M.1371/en
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

logger = logging.getLogger('libais')

# Orbcomm sometimes leaves out the channel.
# TAG BLOCKS use "sentence" as the regex group name.  Use sen here to
# avoid collision.
VDM_RE_STR = r"""(?P<vdm>
!(?P<talker>[A-Z][A-Z])(?P<vdm_type>VD[MO])
,(?P<sen_tot>\d)  # Total number of sentences.
,(?P<sen_num>\d)  # Current sentence number.  Starts from 1.
,(?P<seq_id>[0-9])?  # Receiver sequence number.
,(?P<chan>[AB])?  # VHF channel.
,(?P<body>[;:=@a-zA-Z0-9<>\?\'\`]*)
,(?P<fill_bits>\d)\*(?P<checksum>[0-9A-F][0-9A-F])
)  # end vdm
"""

VDM_RE = re.compile(VDM_RE_STR, re.VERBOSE)
NUMERIC_FIELDS = ('fill_bits', 'sen_num', 'sen_tot', 'seq_id')


def VdmLines(lines):
  """Yield only the lines that contain AIS messages.

  Args:
    lines: An iterable series of strings.

  Yields:
    Lines that look like they probably contain valid VDM/VDO messages.
  """
  for line in lines:
    line = line.rstrip()
    if nmea.ID_BARE_VDM_RE.match(line):
      yield line


def Parse(data):
  """Unpack a NMEA VDM AIS message line(s)."""

  if not isinstance(data, six.string_types):
    raise NotImplementedError

  try:
    result = VDM_RE.search(data).groupdict()
  except AttributeError:
    return

  result.update({k: util.MaybeToNumber(v)
                 for k, v in six.iteritems(result) if k in NUMERIC_FIELDS})

  actual = nmea.Checksum(result['vdm'])
  expected = result['checksum']
  if actual != expected:
    return

  return result


class BareQueue(Queue.Queue):
  """Build complete AIS messages and create a queue of decoded content.

  Manages parsing a stream of NMEA AIS VDM messages.  Single line
  messages are decoded and added to the queue.  Parts of multi-line VDM
  messages are stored until a complete message is available.  The
  bodies of each part are joined with the fill_bit count from the final
  sentence are decoded and added to the queue.
  """

  def __init__(self):
    self.groups = {}
    self.line_num = 0
    Queue.Queue.__init__(self)

  def put(self, line, line_num=None):
    """Add a line of NMEA or raw text to the queue."""

    if line_num is not None:
      self.line_num = line_num
    else:
      self.line_num += 1
      line_num = self.line_num

    line = line.rstrip()
    match = Parse(line)

    if not match:
      logger.info('No VDM match for line: %d, %s', line_num, line)
      msg = {
          'line_nums': [line_num],
          'lines': [line]}
      decoded = nmea_messages.DecodeLine(line)
      if decoded:
        msg['decoded'] = decoded
      else:
        logger.info('No NMEA match for line: %d, %s', line_num, line)
      Queue.Queue.put(self, msg)
      return

    sentence_total = int(match['sen_tot'])
    if sentence_total == 1:
      body = match['body']
      fill_bits = int(match['fill_bits'])
      try:
        decoded = ais.decode(body, fill_bits)
      except ais.DecodeError as error:
        logger.error(
            'Unable to decode message: %s\n  %d %s', error, line_num, line)
        return
      decoded['md5'] = hashlib.md5(body.encode('utf-8')).hexdigest()
      Queue.Queue.put(self, {
          'line_nums': [line_num],
          'lines': [line],
          'decoded': decoded,
          'matches': [match]
      })
      return

    sentence_num = int(match['sen_num'])
    group_id = int(match['seq_id'])

    if sentence_num == 1:
      if group_id in self.groups:
        logger.error('Incomplete message overwritten by new start.  '
                      'Dropped:\n  %s', self.groups[group_id])
      self.groups[group_id] = {
          'line_nums': [line_num],
          'lines': [line],
          'matches': [match]
      }
      return

    if group_id not in self.groups:
      logger.error('Do not have the prior lines in group_id %d. '
                    'Dropping: \n  %s', group_id, line)
      return

    entry = self.groups[group_id]
    if len(entry['lines']) != sentence_num - 1:
      logger.error('Out of sequence message.  Dropping: %d != %d \n %s',
                    len(entry['lines']), sentence_num - 1, line)
      return

    entry['lines'].append(line)
    entry['matches'].append(match)
    entry['line_nums'].append(line_num)

    if sentence_num != sentence_total:
      # Nothing more to do in the middle of a sequence of sentences.
      return

    body = ''.join([match['body'] for match in entry['matches']])
    fill_bits = int(entry['matches'][-1]['fill_bits'])
    try:
      decoded = ais.decode(body, fill_bits)
    except ais.DecodeError as error:
      logger.error(
          'Unable to decode message: %s\n%s', error, entry)
      return
    decoded['md5'] = hashlib.md5(body.encode('utf-8')).hexdigest()
    entry['decoded'] = decoded

    # Found the final message in a group.
    Queue.Queue.put(self, entry)
    self.groups.pop(group_id)
