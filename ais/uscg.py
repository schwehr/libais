# pylint: disable=line-too-long
"""Manage the old USCG metadata format.

The US Coast Guard's older format consists of comma separated values
after the end of the traditional NMEA sentence.  The format was never
formalized in a standard.  The TAG Block format is in NMEA 0183 standard
and is now recognized as preferred.

Each field (except the timestamp) starts with a letter code followed
by the value.  The timestamp is a UNIX UTC seconds from the epoch
value and must come last.  Station name and the timestamp are the only
required fields.  However, some stations mangle the name field or
leave it out, so this module will substitute 'rUnknown' if none is
found.  It will also accept messages that are missing the timestamp.
The timestamp is typically the datalogger's time, not the receiver.
This can sometimes be large spans of time before or after the message
was received.  USCG loggers have been shown to be up to 24 hours off
in time.  Additionally some dataloggers only get periodic updates from the
receivers.  Often the time is not derived from the GPS receiver in the
AIS transceiver or receiver.

Many of the letter codes directly relate to Shine Micro receiver
features.  See Shine Micro AISRadar documentation for addition details.

Example single line AIS NMEA VDM message with USCG metadata:

  !AIVDM,1,1,,A,35Mqd3POj3rmIpjGSpmeCJaH00Qh,0*34,d-095,S1651,t161344.00,T44.03018211,r3669963,1429287142

Example single line with the minimum metadata:

  !ANVDM,1,1,,B,15N6CB0000r86SRFAS:<E@SH08Il,0*43,r08ACERDC,1429287223

Example two line message:

  !AIVDM,2,1,5,A,ENk`sPI17ab7W@6;Q@1@@@@@@@@=MwK3<7v:@00003v,0*4F,d-084,S1652,t161344.00,T44.05687473,r003669945,1429287224
  !AIVDM,2,2,5,A,P000,2*71,d-084,S1652,t161344.00,T44.05687473,r003669945,1429287224

"""

import hashlib
import logging
import re

import six
import six.moves.queue as Queue

import ais
from ais import util
from ais import vdm

logger = logging.getLogger('libais')

# TODO(schwehr): Sort the parts.
USCG_RE = re.compile(r"""
(?P<payload>[^*]*\*[A-F0-9][A-F0-9])
(?P<uscg_metadata>
(
  (,S(?P<slot>\d*))
  | (,s(?P<rssi>\d*))  # Receiver signal strength indicator.
  | (,d(?P<signal_strength>[-0-9]*))  # dB.
  | (,t(?P<receiver_time>(?P<hour>\d\d)(?P<minute>\d\d)  # UTC time.
      (?P<second>\d\d.\d*)))
  | (,T(?P<time_of_arrival>[^,]*))  # Seconds within the minute.
  | (,x(?P<counter>[0-9]*))
  | (,(?P<station>(?P<station_type>[rRbBbD])[a-zA-Z0-9_-]*))
)*
,(?P<time>\d+([.]\d*)?)?  # Logger timestamp.  Can be hours from receive time.
)
""", re.VERBOSE)

NUMERIC_FIELDS = (
  'counter',
  'hour',
  'minute',
  'receiver_time',
  'second',
  'signal_strength',
  'slot',
  'time',
  'time_of_arrival'
)


def Parse(data):
  """Unpack a USCG old metadata format line or return None.

  Makes sure that the line matches the regex and the checksum matches.

  Args:
    data: Line of text.

  Returns:
    A vdm dict or None and a metadata dict or None.
  """
  try:
    result = USCG_RE.search(data).groupdict()
  except AttributeError:
    return None

  result.update({k: util.MaybeToNumber(v)
                 for k, v in six.iteritems(result) if k in NUMERIC_FIELDS})

  return result


class UscgQueue(Queue.Queue):
  """Treats NMEA without USCG station in metadata as from rUnknown."""

  def __init__(self):
    self.groups = {}
    self.line_num = 0
    Queue.Queue.__init__(self)
    self.unknown_queue = vdm.BareQueue()

  def put(self, line, line_num=None):
    if line_num is not None:
      self.line_num = line_num
    else:
      self.line_num += 1

    line = line.rstrip()
    metadata_match = Parse(line)
    match = vdm.Parse(line)

    if not match:
      logger.info('not match')
      msg = {
          'line_nums': [self.line_num],
          'lines': [line],
      }
      if metadata_match:
        msg['match'] = metadata_match
      Queue.Queue.put(self, msg)
      return

    if not metadata_match:
      logger.info('not metadata match')
      self.unknown_queue.put(line)
      if not self.unknown_queue.empty():
        msg = Queue.Queue.get()
        self.put(msg)
      return

    match.update(metadata_match)

    if 'station' not in match:
      match['station'] = 'rUnknown'

    sentence_tot = int(match['sen_tot'])

    if sentence_tot == 1:
      body = match['body']
      fill_bits = int(match['fill_bits'])
      try:
        decoded = ais.decode(body, fill_bits)
      except ais.DecodeError as error:
        logger.error(
            'Unable to decode message: %s\n  %d %s', error, self.line_num, line)
        return
      decoded['md5'] = hashlib.md5(body.encode('utf-8')).hexdigest()
      Queue.Queue.put(self, {
          'line_nums': [line_num],
          'lines': [line],
          'decoded': decoded,
          'matches': [match]
      })
      return

    station = match['station'] or 'rUnknown'
    sentence_num = int(match['sen_num'])
    sequence_id = match['seq_id'] or ''
    group_id = station + str(sequence_id)
    time = util.MaybeToNumber(match['time'])

    if group_id not in self.groups:
      self.groups[group_id] = []

    if not self.groups[group_id]:
      if sentence_num != 1:
        # Drop a partial AIS message.
        return

    if sentence_num == 1:
      self.groups[group_id] = {
          'line_nums': [self.line_num],
          'lines': [line],
          'matches': [match],
          'times': [time],
      }
      return

    entry = self.groups[group_id]
    entry['line_nums'].append(self.line_num)
    entry['lines'].append(line)
    entry['matches'].append(match)
    entry['times'].append(time)

    if sentence_num != sentence_tot:
      # Found the middle part of a message.
      return

    decoded = DecodeMultiple(entry)

    if decoded:
      entry['decoded'] = decoded
    else:
      logger.info('Unable to process: %s', entry)
    Queue.Queue.put(self, entry)
    self.groups.pop(group_id)


def DecodeMultiple(message):
  """Decode a message that spans multiple lines."""
  payloads = [msg['payload'] for msg in message['matches']]

  q = vdm.BareQueue()
  for line in vdm.VdmLines(payloads):
    q.put(line)
  if q.qsize() != 1:
    logger.info('Error: Should get just one message decoded from this: %s',
                 message)
    return
  msg = q.get()
  return msg['decoded']
