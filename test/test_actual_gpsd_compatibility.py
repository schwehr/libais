#!/usr/bin/env python
"""Test the ais compatibility layer."""

import json
import os
import re
import subprocess
import unittest

import ais.compatibility.gpsd
import ais.stream
import six
import testutils


known_problems = {
    2: set(('turn', 'status_text')),
    9: set(['speed']),
    15: set(['mmsi2']),
    17: set(('lat', 'lon')),
    20: set((
        'increment3', 'number3', 'offset3', 'timeout3',
        'increment4', 'number4', 'offset4', 'timeout4',
    )),
    27: set(['status']),
}


def HaveGpsdecode():
  """Return true if the gpsdecode binary is on the path or false if not."""
  try:
    subprocess.check_call(['gpsdecode', '-V'])
    return True
  except OSError:
    return False


class GPSDCompatibility(unittest.TestCase):

  def setUp(self):
    self.dir = os.path.split(__file__)[0]
    self.nmea = os.path.join(self.dir, 'data/typeexamples.nmea')
    self.json = os.path.join(self.dir, 'data/typeexamples.gpsdecode.json')

    subprocess.check_call('gpsdecode < %s > %s' % (self.nmea, self.json),
                          shell=True)

  def tearDown(self):
    os.unlink(self.json)

  @unittest.skipIf(not HaveGpsdecode(), 'gpsdecode not in the path')
  def testAll(self):
    def Gpsd():
      with open(self.json) as f:
        for msg in f:
          yield json.loads(msg)

    def Libais():
      with open(os.path.join(self.dir, 'data/typeexamples.nmea')) as f:
        for msg in ais.stream.decode(f):
          yield ais.compatibility.gpsd.mangle(msg)

    g = iter(Gpsd())
    a = iter(Libais())

    try:
      while True:
        gmsg = six.advance_iterator(g)
        amsg = six.advance_iterator(a)
        while amsg['type'] != gmsg['type']:
          amsg = six.advance_iterator(a)

        if gmsg['type'] in known_problems:
          for key in known_problems[gmsg['type']]:
            if key in gmsg: del gmsg[key]
            if key in amsg: del amsg[key]

        diff = testutils.DictDiff(gmsg, amsg)
        self.assertFalse(diff['changed'])
        self.assertFalse(
            diff['removed'],
            'Removed not empty: %s\n  %s\n  %s' % (
                diff['removed'],
                amsg,
                gmsg))

    except StopIteration:
      pass


if __name__ == '__main__':
  unittest.main()
