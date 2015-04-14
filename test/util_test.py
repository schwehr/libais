#!/usr/bin/env python
"""Tests for ais.util."""

import unittest
from ais import util
import six


class UtilTest(unittest.TestCase):

  def testMaybeToNumber(self):
    self.assertEqual(util.MaybeToNumber(None), None)
    self.assertEqual(util.MaybeToNumber([]), [])
    self.assertEqual(util.MaybeToNumber({}), {})
    self.assertEqual(util.MaybeToNumber('a'), 'a')
    self.assertEqual(util.MaybeToNumber(1), 1)
    self.assertEqual(util.MaybeToNumber(-3.12), -3.12)

    self.assertEqual(util.MaybeToNumber('-1'), -1)
    self.assertIsInstance(util.MaybeToNumber('-1'), int)

    self.assertEqual(util.MaybeToNumber('42.0'), 42.0)
    self.assertIsInstance(util.MaybeToNumber('42.0'), float)

    value = 9999999999999999999999999
    value_str = '9999999999999999999999999'
    self.assertEqual(util.MaybeToNumber(value_str), value)
    self.assertIsInstance(util.MaybeToNumber(value_str), six.integer_types)

    self.assertEqual(
        util.MaybeToNumber('1e99999999999999999999999'), float('inf'))
    self.assertEqual(
        util.MaybeToNumber('-1e99999999999999999999999'), float('-inf'))


if __name__ == '__main__':
  unittest.main()
