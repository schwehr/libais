"""Tests for google3.third_party.py.ais.test.compatibility.gpsd."""

from ais.compatibility import gpsd

import unittest


class GpsdTest(unittest.TestCase):

  def setUp(self):
    self.mangle = gpsd.Mangler()

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
