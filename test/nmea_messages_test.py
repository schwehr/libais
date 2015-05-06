#!/usr/bin/env python
"""Tests for ais.nmea."""

import datetime
import unittest

from ais import nmea_messages


class GgaTest(unittest.TestCase):

  def testGgaRegex(self):
    line = (
        '$GPGGA,174246.00,7119.6369,N,15640.8432,W,1,06,2.39,00011,M,000,M,,'
        '*7D') # ',rakbarrow85,1226943363.79')
    msg = nmea_messages.GGA_RE.search(line).groupdict()
    self.assertEqual(
        msg,
        {
            'antenna_height': '00011',
            'antenna_height_units': 'M',
            'checksum': '7D',
            'differential_age_sec': None,
            'differential_ref_station': '',
            'geoidal_height': '000',
            'geoidal_height_units': 'M',
            'gps_quality': '1',
            'hdop': '2.39',
            'hours': '17',
            'lat_deg': '71',
            'lat_min': '19.6369',
            'latitude': '7119.6369',
            'latitude_hemisphere': 'N',
            'lon_deg': '156',
            'lon_min': '40.8432',
            'longitude': '15640.8432',
            'longitude_hemisphere': 'W',
            'minutes': '42',
            'satellites': '06',
            'seconds': '46.00',
            'sentence': 'GGA',
            'talker': 'GP',
            'time_utc': '174246.00'})

  def testGgaDecode(self):
    line = (
        '$GPGGA,174246.00,7119.6369,N,15640.8432,W,1,06,2.39,00011,M,000,M,,'
        '*7D')
    msg = nmea_messages.Gga(line)
    self.assertEqual(
        msg,
        {
            'antenna_height': 11,
            'antenna_height_units': 'M',
            'geoidal_height': 0,
            'geoidal_height_units': 'M',
            'gps_quality': 1,
            'hdop': 2.39,
            'latitude': 71.32728166666666,
            'longitude': -156.68072,
            'satellites': 6,
            'time': datetime.time(17, 42, 46)})


class ZdaTest(unittest.TestCase):

  def testZdaRegex(self):
    line = '$INZDA,082015.0007,30,04,2009,,*73'
    msg = nmea_messages.ZDA_RE.search(line).groupdict()
    self.assertEqual(
      msg,
      {
          'checksum': '73',
          'day': '30',
          'hours': '08',
          'minutes': '20',
          'month': '04',
          'seconds': '15.0007',
          'sentence': 'ZDA',
          'talker': 'IN',
          'time_utc': '082015.0007',
          'year': '2009',
          'zone_hours': None,
          'zone_minutes': None})

  def testZdaDecode(self):
    line = '$INZDA,082015.0007,30,04,2009,,*73'
    msg = nmea_messages.Zda(line)
    self.assertEqual(
        msg,
        {
            'msg': 'ZDA',
        'datetime': datetime.datetime(2009, 4, 30, 8, 20, 15, 700)})


if __name__ == '__main__':
  unittest.main()
