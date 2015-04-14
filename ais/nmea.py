"""Handling of NMEA encoded formatting.

The National Marine Electronics Association (NMEA) added a TAG BLOCK
metadata data specification to NMEA 0183 4.0.

Some providers violate the NMEA standard by using lower case letters in NMEA
checksums.  This module forces checksums to always be upper case.

See Also:
  http://catb.org/gpsd/NMEA.html
  http://catb.org/gpsd/AIVDM.html#_nmea_tag_blocks
"""

import re

# The lowercase a-f violates NMEA 0183, but allow it.
CHECKSUM_RE = re.compile(r'\*(?P<checksum>[0-9A-Fa-f]{2})$')
SENTENCE_START_RE = re.compile(r'^[\\$!]')

# Types of NMEA lines.
TEXT = 'TEXT'
BARE = 'BARE'
TAGB = 'TAGB'
USCG = 'USCG'

# Generally match line type.
# TODO(schwehr): Add ^ to the beginning of each?  Faster or slower?
ID_BARE_RE = re.compile(r'[!$][A-Z]{4}[^*!$]*\*[0-9A-Fa-f]{2}$')
ID_USCG_RE = re.compile(r'[!$][A-Z]{4}[^*!$]*\*[0-9A-Fa-f]{2},[^*!$]+$')
ID_TAGB_RE = re.compile(r'\\([a-zA-Z]:[^*,\\]*[,]?)+\*[0-9A-Fa-f]{2}(\\.*)?$')

# Specific match for an AIS line.
ID_BARE_VDM_RE = re.compile(r'[!$][A-Z]{2}VD[MO][^*!$]*\*[0-9A-Fa-f]{2}')


def _Checksum(sentence):
  """Compute the NMEA checksum for a payload."""
  checksum = 0
  for char in sentence:
    checksum ^= ord(char)
  checksum_str = '%02x' % checksum
  return checksum_str.upper()


def Checksum(sentence):
  """Compute the checksum for a NMEA sentence or TAG BLOCK prefix.

  Args:
    sentence: str, NMEA tag block sentence starting with a backslash, bang, or
        dollar sign.  The string must end with a star and the 2 character
        checksum.

  Returns:
    The computed checksum.
  """
  sentence = SENTENCE_START_RE.sub('', sentence)
  sentence = CHECKSUM_RE.sub('', sentence)
  return _Checksum(sentence)


def LineType(line):
  """Give the type of NMEA line this is.

  Args:
    line: str, Text to try to interpret.

  Returns:
    String that is one of 'TEXT', 'BARE', 'USCG', or 'TAGB'.
  """
  if not line:
    return TEXT

  if line[0] in '!$':
    if ID_BARE_RE.match(line):
      return BARE
    elif ID_USCG_RE.match(line):
      return USCG
    return TEXT

  if line[0] == '\\' and ID_TAGB_RE.match(line):
    return TAGB

  return TEXT
