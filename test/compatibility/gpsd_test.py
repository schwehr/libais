#!/usr/bin/env python

"""Tests for ais.compatibility.gpsd."""

import ais
import ais.compatibility.gpsd
import ais.stream

import unittest
import json
import os
import re
import subprocess
import six
from .. import testutils

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


class SingleMessageTestsTest(unittest.TestCase):

  def setUp(self):
    self.mangle = ais.compatibility.gpsd.Mangler()

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

  def testTimestamps(self):
    msg = {
        'id': 1,
        'tagblock_timestamp': 1431682043,
        'year': 2015,
        'month': 5,
        'day': 15,
        'hour': 9,
        'minute': 27,
        'second': 23,
        }
    mangled = self.mangle(msg)

    expected = {
        'type': 1,
        'timestamp': '2015-05-15T09:27:23Z',
        'tagblock_timestamp': '2015-05-15T09:27:23.000000Z'
        }
    self.assertDictContainsSubset(expected, mangled)


class StreamingTest(unittest.TestCase):
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
      self.validate_file("../data/typeexamples")

  def testTagblock(self):
      self.validate_file("../data/tagblock")



def HaveGpsdecode():
  """Return true if the gpsdecode binary is on the path or false if not."""
  try:
    subprocess.check_call(['gpsdecode', '-V'])
    return True
  except OSError:
    return False


class TestActualGPSDCompatibility(unittest.TestCase):
  def setUp(self):
    self.dir = os.path.split(__file__)[0]

  def validate_file(self, base):
    nmea_name = os.path.join(self.dir, base + '.nmea')
    json_name = os.path.join(self.dir, base + '.gpsdecode.json')

    subprocess.check_call('gpsdecode < %s > %s' % (nmea_name, json_name),
                          shell=True)

    try:
      def Gpsd():
        with open(json_name) as f:
          for msg in f:
            yield json.loads(msg)

      def Libais():
        with open(nmea_name) as f:
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
    finally:
      os.unlink(json_name)

  @unittest.skipIf(not HaveGpsdecode(), 'gpsdecode not in the path')
  def testTypeExamples(self):
      self.validate_file("../data/typeexamples")

  @unittest.skipIf(not HaveGpsdecode(), 'gpsdecode not in the path')
  def testTagblock(self):
      self.validate_file("../data/tagblock")


if __name__ == '__main__':
  unittest.main()
