"""Tests for google3.third_party.py.ais.test.compatibility.gpsd."""


import ais
from ais.compatibility import gpsd

import unittest


class GpsdTest(unittest.TestCase):

  def setUp(self):
    self.mangle = gpsd.Mangler()

  def testMsg1(self):
    fields = '!AIVDM,1,1,,B,169A91O005KbT4gUoUl9d;5j0D0U,0*2D'.split(',')
    decoded = ais.decode(fields[5], int(fields[6][0]))
    mangled = self.mangle(decoded)
    expected = {
        'type': 1,
        'repeat': 0,
        'mmsi': 412371205,
        'status': 15,
        'status_text': 'Not defined',
        'turn': 0,
        'speed': 0.5,
        'accuracy': False,
        'course': 248.0,
        'heading': 354,
        'second': 57,
        'maneuver': 0,
        'raim': False}
    self.assertDictContainsSubset(expected, mangled)

    # Float values will not match, so just test existence.
    for field in ('lat', 'lon'):
      self.assertIn(field, mangled)

  def testMsg5LargeTypeAndCargo(self):
    # Test based on this AIS message:
    #   ais.decode(r'568rWSP000009@9D000hT4r0L4pN3;D000000<o<000004000'
    #              r'0000000000000000000000',2)
    msg = {
        'id': 5,
        'type_and_cargo': 204}
    mangled = self.mangle(msg)

    expected = {
        'shiptype': 204,
        'shiptype_text': '204 - Unknown',
        'type': 5}
    self.assertDictContainsSubset(expected, mangled)


if __name__ == '__main__':
  unittest.main()
