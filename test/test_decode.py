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
      self.assertDictEqual(msg, expected,
                           'Mismatch for id:%d\n%s\n%s' % (msg['id'] ,msg, expected))


if __name__=='__main__':
  unittest.main()
