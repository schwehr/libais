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

class GPSDCompatibility(unittest.TestCase):

  def setUp(self):
    self.dir = os.path.split(__file__)[0]

  def validate_file(self, base):
    nmea_name = os.path.join(self.dir, base + '.nmea')
    json_name = os.path.join(self.dir, base + '.json')

    def Json():
      with open(json_name) as f:
        for msg in f:
          yield json.loads(msg)

    def Libais():
      with open(nmea_name) as f:
        for msg in ais.stream.decode(f):
          yield ais.compatibility.gpsd.mangle(msg)

    g = iter(Json())
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

  def testTypeExamples(self):
      self.validate_file("data/typeexamples")

  def testTagblock(self):
      self.validate_file("data/tagblock")


if __name__ == '__main__':
  unittest.main()
