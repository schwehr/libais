#!/usr/bin/env python

"""Test the top level decoders with a single message each."""

import ais
import unittest
from . import test_data
import sys

class AisTopLevelDecoders(unittest.TestCase):

  def setUp(self):
    self.maxDiff = None

  def testAll(self):
    """Decode one of each top level message"""
    # TODO: message 20
    for entry in test_data.top_level:
      body = ''.join([line.split(',')[5] for line in entry['nmea']])
      pad = int(entry['nmea'][-1].split('*')[0][-1])
      msg = ais.decode(body, pad)
      expected = entry['result']
      if msg.keys() != expected.keys():
        sys.stderr.write('key mismatch: %s\n' % set(msg).symmetric_difference(set(expected)))
      self.assertDictEqual(
          expected, msg,
          'Mismatch for id:%d\n%s\n%s\n  From: %s' % (
              msg['id'], msg, expected, entry['nmea']))


class Ais6Decoders(unittest.TestCase):

  def testDecodeUnknownMessage6(self):
    # !AIVDM,1,1,,B,6B?n;be:cbapalgc;i6?Ow4,2*4A'
    # TODO(schwehr): Expose the C++ Python exception to Python.
    self.assertRaisesRegex(ais.DecodeError, '6:669:11',
                           ais.decode, '6B?n;be:cbapalgc;i6?Ow4', 2)

class Ais8AreaNoticeDecoders(unittest.TestCase):

  # Recorded USCG messages already exercised by the C++ decoder tests in
  # src/test/ais8_367_test.cpp. Exercise the public Python decoder as well.
  CIRCLE = '8h3Ovq1KmPA`08b8007P3ct5uAPmtlAkh000'

  def testCircle(self):
    msg = ais.decode(self.CIRCLE, 0)
    self.assertEqual((8, 367, 22, 3669732),
                     (msg['id'], msg['dac'], msg['fi'], msg['mmsi']))
    self.assertEqual((104, 0, 4, 10, 17, 0, 60),
                     (msg['link_id'], msg['notice_type'], msg['month'],
                      msg['day'], msg['hour'], msg['minute'],
                      msg['durations_minutes']))
    self.assertEqual(1, len(msg['sub_areas']))
    circle = msg['sub_areas'][0]
    self.assertEqual('circle', circle['sub_area_type_str'])
    self.assertAlmostEqual(-70.1184, circle['x'], places=4)
    self.assertAlmostEqual(42.3113, circle['y'], places=4)
    self.assertEqual(9260, circle['radius'])

  def testText(self):
    msg = ais.decode('803Ow2iKmPFJwP37P000bbHHsrPbJP000000', 0)
    self.assertEqual('text', msg['sub_areas'][0]['sub_area_type_str'])
    self.assertEqual('USCG-TEST@@@@@@', msg['sub_areas'][0]['text'])

  def testPolylines(self):
    msg = ais.decode(
        '8h3Ovq1KmP@N<95=`2l01=dN<b7pGeP00000LL8PSV8RQ8cTs5H0LTHh477PRpus@000', 0)
    self.assertEqual(['point', 'polyline', 'polyline'],
                     [area['sub_area_type_str'] for area in msg['sub_areas']])
    self.assertEqual([225, 230, 265, 315], msg['sub_areas'][1]['angles'])
    self.assertEqual([13000, 27300, 17400, 17200], msg['sub_areas'][1]['dists_m'])

  def testTruncatedArea(self):
    with self.assertRaisesRegex(ais.DecodeError, 'AIS_ERR_BAD_BIT_COUNT'):
      ais.decode(self.CIRCLE[:-1], 0)

  def testReservedAreaShape(self):
    # Subarea shape 6 is reserved. Preserve the header and remaining bits.
    payload = self.CIRCLE[:20] + 'h' + self.CIRCLE[21:]
    with self.assertRaisesRegex(ais.DecodeError, 'AIS_ERR_BAD_SUB_SUB_MSG'):
      ais.decode(payload, 0)


if __name__=='__main__':
  unittest.main()
