
# TODO(schwehr): Make sure this works with proprietary messages.

import datetime
import logging
import math
import re

from ais import util

NMEA_HEADER_RE_STR = r'[$!](?P<talker>[A-Z][A-Z])'
NMEA_SENTENCE_RE_STR = NMEA_HEADER_RE_STR + r'(?P<sentence>[A-Z]{3,4}),'
NMEA_CHECKSUM_RE_STR = r'\*(?P<checksum>[0-9A-F][0-9A-F])'

NMEA_HEADER_RE = re.compile(NMEA_HEADER_RE_STR)
NMEA_SENTENCE_RE = re.compile(NMEA_SENTENCE_RE_STR)
NMEA_CHECKSUM_RE = re.compile(NMEA_CHECKSUM_RE_STR)

HANDLERS = {}

ABK_RE_STR = (
    NMEA_HEADER_RE_STR +
    r'(?P<sentence>ABK),'
    r'(?P<mmsi>\d+)?,'
    r'(?P<chan>[AB])?,'
    r'(?P<msg_id>\d+)?,'
    r'(?P<seq_num>\d+)?,'
    r'(?P<ack_type>\d+)' +
    NMEA_CHECKSUM_RE_STR
)

ABK_RE = re.compile(ABK_RE_STR)


def Abk(line):
  """Decode AIS Addressed and Binary Broadcast Acknowledgement (ABK)."""
  try:
    fields = ABK_RE.match(line).groupdict()
  except TypeError:
    return

  result = {
      'msg': 'ABK',
      'talker': fields['talker'],
      'chan': fields['chan'],
  }

  for field in ('mmsi', 'msg_id', 'seq_num', 'ack_type'):
    result[field] = util.MaybeToNumber(fields[field])

  return result


HANDLERS['ABK'] = Abk

ADS_RE_STR = (
    NMEA_HEADER_RE_STR +
    r'(?P<sentence>ADS),'
    r'(?P<id>[^,]+?),'
    r'(?P<time>\d\d\d\d\d\d(\.\d\d)?)?,'
    r'(?P<alarm>)[AV]?,'
    r'(?P<time_sync_method>\d)?,'
    r'(?P<pos_src>[EINS])?,'
    r'(?P<time_src>[EIN])?' +
    NMEA_CHECKSUM_RE_STR
)

ADS_RE = re.compile(ADS_RE_STR)


def Ads(line):
  """Decode Automatic Device Status (ADS)."""
  try:
    fields = ADS_RE.match(line).groupdict()
  except TypeError:
    return

  result = {
      'msg': 'ADS',
      'talker': fields['talker'],
      'id': fields['id'],
      'time': fields['time'],
      'alarm': fields['alarm'],
      'time_sync_method': util.MaybeToNumber(fields['time_sync_method']),
      'pos_src': fields['pos_src'],
      'time_src': fields['time_src'],
  }

  return result


HANDLERS['ADS'] = Ads

BBM_RE_STR = (
    NMEA_HEADER_RE_STR +
    r'(?P<sentence>BBM),'
    r'(?P<sen_tot>\d),'
    r'(?P<sen_num>\d),'
    r'(?P<seq_num>\d),'
    r'(?P<chan>\d),'
    r'(?P<msg_id>\d),'
    r'(?P<body>[^,*]*),'
    r'(?P<fill_bits>\d)' +
    NMEA_CHECKSUM_RE_STR
)

BBM_RE = re.compile(BBM_RE_STR)


def Bbm(line):
  """Decode Binary Broadcast Message (BBM) sentence."""
  try:
    fields = BBM_RE.match(line).groupdict()
  except TypeError:
    return

  result = {
      'msg': 'BBM',
      'talker': fields['talker'],
      'body': fields['body'],
  }

  for field in ('sen_tot', 'sen_num', 'seq_num', 'chan', 'msg_id', 'fill_bits'):
    result[field] = util.MaybeToNumber(fields[field])

  return result


HANDLERS['BBM'] = Bbm

GGA_RE_STR = (
    NMEA_HEADER_RE_STR +
    r'(?P<sentence>GGA),'
    r'(?P<time_utc>(?P<hours>\d\d)(?P<minutes>\d\d)(?P<seconds>\d\d\.\d*))?,'
    r'(?P<latitude>(?P<lat_deg>\d\d)(?P<lat_min>\d\d\.\d*))?,'
    r'(?P<latitude_hemisphere>[NS])?,'
    r'(?P<longitude>(?P<lon_deg>\d{3})(?P<lon_min>\d\d\.\d*))?,'
    r'(?P<longitude_hemisphere>[EW])?,'
    r'(?P<gps_quality>\d+)?,'
    r'(?P<satellites>\d+)?,'
    r'(?P<hdop>\d+\.\d+)?,'
    r'(?P<antenna_height>[+-]?\d+(\.\d+)?)?,'
    r'(?P<antenna_height_units>M)?,'
    r'(?P<geoidal_height>[+-]?\d+(\.\d+)?)?,'
    r'(?P<geoidal_height_units>M)?,'
    r'(?P<differential_ref_station>[A-Z0-9.]*)?,'
    r'(?P<differential_age_sec>\d+)?'
    + NMEA_CHECKSUM_RE_STR
)

GGA_RE = re.compile(GGA_RE_STR)  # ; GGA_RE.search(line).groupdict()


def Gga(line):
  try:
    fields = GGA_RE.match(line).groupdict()
  except TypeError:
    return

  seconds, fractional_seconds = FloatSplit(float(fields['seconds']))
  microseconds = int(math.floor(fractional_seconds * 1e6))

  when = datetime.time(
      int(fields['hours']),
      int(fields['minutes']),
      seconds,
      microseconds
  )

  x = int(fields['lon_deg']) + float(fields['lon_min']) / 60.0
  if fields['longitude_hemisphere'] == 'W':
    x = -x

  y = int(fields['lat_deg']) + float(fields['lat_min']) / 60.0
  if fields['latitude_hemisphere'] == 'S':
    y = -y

  result = {
      'time': when,
      'longitude': x,
      'latitude': y,
  }
  for field in ('gps_quality', 'satellites', 'hdop', 'antenna_height',
                'antenna_height_units', 'geoidal_height',
                'geoidal_height_units', 'differential_ref_station',
                'differential_age_sec'):
    if fields[field] is not None and fields[field]:
      result[field] = util.MaybeToNumber(fields[field])

  return result


HANDLERS['GGA'] = Gga


# Time in UTC.
ZDA_RE_STR = (
    NMEA_HEADER_RE_STR +
    r'(?P<sentence>ZDA),'
    r'(?P<time_utc>(?P<hours>\d\d)(?P<minutes>\d\d)(?P<seconds>\d\d(\.\d*)?))?,'
    r'(?P<day>\d\d)?,'
    r'(?P<month>\d\d)?,'
    r'(?P<year>\d{4})?,'
    r'(?P<zone_hours>[+-]?(\d+))?,'
    r'(?P<zone_minutes>(\d+))?'  # ',?'
    + NMEA_CHECKSUM_RE_STR
)

ZDA_RE = re.compile(ZDA_RE_STR)


def FloatSplit(value):
  base = math.trunc(value)
  fractional = value - base
  return base, fractional


def Zda(line):
  logging.info('zda line: %s', line)
  try:
    fields = ZDA_RE.match(line).groupdict()
  except TypeError:
    return

  seconds, fractional_seconds = FloatSplit(float(fields['seconds']))
  microseconds = int(math.floor(fractional_seconds * 1e6))
  when = datetime.datetime(
      int(fields['year']),
      int(fields['month']),
      int(fields['day']),
      int(fields['hours']),
      int(fields['minutes']),
      seconds,
      microseconds)

  # TODO(schwehr): Convert this to Unix UTC seconds.
  result = {
      'msg': 'ZDA',
      'datetime': when,
  }

  try:
    result['zone_hours'] = int(fields['zone_hours'])
  except TypeError:
    pass
  try:
    result['zone_minutes'] = int(fields['zone_minutes'])
  except TypeError:
    pass

  return result

HANDLERS['ZDA'] = Zda


def Decode(line):
  line = line.rstrip()
  try:
    sentence = NMEA_SENTENCE_RE.match(line).groupdict()['sentence']
  except AttributeError:
    # Not NMEA.
    return

  if sentence not in HANDLERS:
    logging.info('skipping: %s', line)
    return

  try:
    msg = HANDLERS[sentence](line)
  except AttributeError:
    logging.info('Unable to decode line with handle: %s', line)
    return
  logging.info('decoded: %s', msg)
  return msg

