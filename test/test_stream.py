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


known_bad = set((
    'addressed',
    'app_id',
    'data',
    'eta',  # TODO(schwehr): Fix this.
    'radio',
    'regional',
    'reserved',
    'structured',
))
precision = 5.0

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


def TextToNumber(s):
  try:
    return float(s)
  except (TypeError, ValueError):
    return s


def IsNumber(value):
  if isinstance(value, float):
    return True
  if isinstance(value, six.integer_types):
    return True
  return False


def DictDiff(a, b):
  def Compare(x, y):
    if x == y:
      return True
    x = TextToNumber(x)
    y = TextToNumber(y)
    if isinstance(x, six.string_types) and isinstance(y, six.string_types):
      # Collapse strings to just lower case a-z to avoid simple mismatches.
      new_x = re.sub(r'[^a-z]', r'', six.text_type(x).lower())
      new_y = re.sub(r'[^a-z]', r'', six.text_type(y).lower())
      if new_x == new_y:
        return True
    if IsNumber(x) and IsNumber(y):
      if abs(float(x) - float(y)) < precision:
        return True
    return False

  # TODO(redhog): Use sets and make this easier to follow.
  return {
      'removed': {key: a[key] for key in a
                  if key not in b and key not in known_bad},
      'changed': {key: (a[key], b[key]) for key in a
                  if key in b
                  and key not in known_bad
                  and not Compare(a[key], b[key])},
      'added': {key: b[key] for key in b
                if key not in a and key not in known_bad}
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

        diff = DictDiff(gmsg, amsg)
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
      self.validate_file("typeexamples")

  def testTagblock(self):
      self.validate_file("tagblock")


if __name__ == '__main__':
  unittest.main()
