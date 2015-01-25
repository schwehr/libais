"""Utilities for working with NMEA strings."""

import re
import sys
import time

nmeaChecksumRegExStr = r"""\,[0-9]\*[0-9A-F][0-9A-F]"""
nmeaChecksumRE = re.compile(nmeaChecksumRegExStr)


def checksumStr(data):
    """Take a NMEA 0183 string and compute the checksum.
    @param data: NMEA message.  Leading ?/! and training checksum are optional
    @type data: str
    @return: hexidecimal value
    @rtype: str

    Checksum is calculated by xor'ing everything between ? or ! and the *

    >>> checksumStr("!AIVDM,1,1,,B,35MsUdPOh8JwI:0HUwquiIFH21>i,0*09")
    '09'
    >>> checksumStr("AIVDM,1,1,,B,35MsUdPOh8JwI:0HUwquiIFH21>i,0")
    '09'
    """

    # FIX: strip off new line at the end too
    if data[0]=='!' or data[0]=='?': data = data[1:]
    if data[-1]=='*': data = data[:-1]
    if data[-3]=='*': data = data[:-3]
    # FIX: rename sum to not shadown builting function
    checksum = 0
    for c in data:
      checksum = checksum ^ ord(c)
    sum_hex = "%x" % checksum
    if len(sum_hex) == 1:
      sum_hex = '0' + sum_hex
    return sum_hex.upper()


def isChecksumValid(nmeaStr, allowTailData=True):
    """Return True if the string checks out with the checksum.

    @param allowTailData: Permit handing of Coast Guard format with data after the checksum
    @param data: NMEA message.  Leading ?/! are optional
    @type data: str
    @return: True if the checksum matches
    @rtype: bool

    >>> isChecksumValid("!AIVDM,1,1,,B,35MsUdPOh8JwI:0HUwquiIFH21>i,0*09")
    True

    Corrupted:

    >>> isChecksumValid("!AIVDM,11,1,,B,35MsUdPOh8JwI:0HUwquiIFH21>i,0*09")
    False
    """

    if allowTailData:
        match = nmeaChecksumRE.search(nmeaStr)
        if not match:
            return False
        nmeaStr = nmeaStr[:match.end()]

    if nmeaStr[-3]!='*':
        return False  # Bad string without proper checksum.
    checksum=nmeaStr[-2:]
    if checksum.upper() == checksumStr(nmeaStr).upper():
        return True
    return False
