#!/usr/bin/env python

"""Test the top level decoders with a single message each.

"""

import ais
import unittest
import test_data

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
      self.assertDictEqual(msg, entry['result'])


if __name__=='__main__':
  unittest.main()
