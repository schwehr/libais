#!/usr/bin/env python
"""Tests for ais.tag_block."""

import unittest

from ais import tag_block


class TagTestCase(unittest.TestCase):

  def assertDictContainsSubset2(self, actual, expected):
    # assertDictContainsSubset was deprecated because of incorrect arg order.
    # This method has the correct order.
    self.assertIsInstance(expected, dict)
    self.assertIsInstance(actual, dict)
    for key, value in expected.iteritems():
      self.assertEqual(actual[key], value)


class TagBlockTest(TagTestCase):

  def testFractionalTime(self):
    # TAG BLOCK only allows for integer times, but allow for greater precision.
    line = '\\c:1425327399.*70\\'
    match = tag_block.TAG_BLOCK_RE.match(line).groupdict()
    expected = {'time': '1425327399.'}
    self.assertDictContainsSubset2(match, expected)
    self.assertDictContainsSubset2(tag_block.Parse(match), expected)
    self.assertDictContainsSubset2(tag_block.Parse(line), expected)

    line = '\\c:1425327399.0*40\\'
    match = tag_block.TAG_BLOCK_RE.match(line).groupdict()
    expected = {'time': '1425327399.0'}
    self.assertDictContainsSubset2(match, expected)
    self.assertDictContainsSubset2(tag_block.Parse(match), expected)
    self.assertDictContainsSubset2(tag_block.Parse(line), expected)

  def testShortSingleLine(self):
    line = (r'\n:121650,s:r17MHOP1,c:1425327399*1D\$'
            'ANZDA,201638.00,02,03,2015,00,00*77')
    match = tag_block.TAG_BLOCK_RE.match(line).groupdict()
    expected = {
        'line_num': '121650',
        'metadata': 'n:121650,s:r17MHOP1,c:1425327399*1D',
        'payload': '$ANZDA,201638.00,02,03,2015,00,00*77',
        'rcvr': 'r17MHOP1',
        'tag_checksum': '1D',
        'time': '1425327399'}
    self.assertDictContainsSubset2(match, expected)
    self.assertDictContainsSubset2(tag_block.Parse(match), expected)
    self.assertDictContainsSubset2(tag_block.Parse(line), expected)

  def testSingleLine(self):
    line = (r'\s:Station,d:somewhere,n:2,q:u,r:123,t:A string.,'
            r'c:1425168552,T:2015-03-01 00.09.12*3A\content')
    match = tag_block.TAG_BLOCK_RE.match(line).groupdict()
    expected = {
        'dest': 'somewhere',
        'group': None,
        'group_id': None,
        'line_num': '2',
        'payload': 'content',
        'quality': 'u',
        'rcvr': 'Station',
        'rel_time': '123',
        'sentence_num': None,
        'sentence_tot': None,
        'tag_checksum': '3A',
        'text': 'A string.',
        'text_date': '2015-03-01 00.09.12',
        'time': '1425168552'}
    self.assertDictContainsSubset2(match, expected)
    self.assertDictContainsSubset2(tag_block.Parse(match), expected)
    self.assertDictContainsSubset2(tag_block.Parse(line), expected)

  def testFirstOfGroup(self):
    line = (r'\g:1-3-42349,n:111458,s:r003669945,c:1425327424*41\!'
            'AIVDM,2,1,5,A,ENk`sPa17ab7W@7@1T@6;Q@0h@@=MeR4<7rpH00003v,0*16')
    match = tag_block.TAG_BLOCK_RE.match(line).groupdict()
    expected = {
        'group': '1-3-42349',
        'group_id': '42349',
        'line_num': '111458',
        'metadata': 'g:1-3-42349,n:111458,s:r003669945,c:1425327424*41',
        'payload':
            '!AIVDM,2,1,5,A,ENk`sPa17ab7W@7@1T@6;Q@0h@@=MeR4<7rpH00003v,0*16',
        'rcvr': 'r003669945',
        'sentence_num': '1',
        'sentence_tot': '3',
        'tag_checksum': '41',
        'time': '1425327424'}
    self.assertDictContainsSubset2(match, expected)
    self.assertDictContainsSubset2(tag_block.Parse(match), expected)
    self.assertDictContainsSubset2(tag_block.Parse(line), expected)

  def testMiddleOfGroup(self):
    line = (r'\g:2-3-42349,n:111459*15\!AIVDM,2,2,5,A,P000,2*71')
    match = tag_block.TAG_BLOCK_RE.match(line).groupdict()
    expected = {
        'group': '2-3-42349',
        'group_id': '42349',
        'line_num': '111459',
        'metadata': 'g:2-3-42349,n:111459*15',
        'payload': '!AIVDM,2,2,5,A,P000,2*71',
        'sentence_num': '2',
        'sentence_tot': '3',
        'tag_checksum': '15'}
    self.assertDictContainsSubset2(match, expected)
    self.assertDictContainsSubset2(tag_block.Parse(match), expected)
    self.assertDictContainsSubset2(tag_block.Parse(line), expected)

  def testEndOfGroup(self):
    line = (r'\g:3-3-42349,n:111460*1E\$'
            'ARVSI,r003669945,5,201704.05687473,0152,-085,0*2E')
    match = tag_block.TAG_BLOCK_RE.match(line).groupdict()
    expected = {
        'group': '3-3-42349',
        'group_id': '42349',
        'line_num': '111460',
        'metadata': 'g:3-3-42349,n:111460*1E',
        'payload': '$ARVSI,r003669945,5,201704.05687473,0152,-085,0*2E',
        'sentence_num': '3',
        'sentence_tot': '3',
        'tag_checksum': '1E'}
    self.assertDictContainsSubset2(match, expected)
    self.assertDictContainsSubset2(tag_block.Parse(match), expected)
    self.assertDictContainsSubset2(tag_block.Parse(line), expected)

  def testOrbcommFieldT(self):
    line = (r'\s:rORBCOMM999,q:u,c:1424995200,T:2015-02-27 00.00.00*51\!'
            'AIVDM,1,1,,A,14VIk0002sMM04vE>V9jGimn08RP,0*0D')
    match = tag_block.TAG_BLOCK_RE.match(line).groupdict()
    expected = {
        'metadata': 's:rORBCOMM999,q:u,c:1424995200,T:2015-02-27 00.00.00*51',
        'payload': '!AIVDM,1,1,,A,14VIk0002sMM04vE>V9jGimn08RP,0*0D',
        'quality': 'u',
        'rcvr': 'rORBCOMM999',
        'tag_checksum': '51',
        'text_date': '2015-02-27 00.00.00',
        'time': '1424995200'}
    self.assertDictContainsSubset2(match, expected)
    self.assertDictContainsSubset2(tag_block.Parse(match), expected)
    self.assertDictContainsSubset2(tag_block.Parse(line), expected)


class TagQueueTestSingleLines(TagTestCase):

  def testNonTagBlockLines(self):
    queue = tag_block.TagQueue()
    queue.put('1')
    self.assertEqual(queue.qsize(), 1)
    queue.put('2')
    self.assertEqual(queue.qsize(), 2)
    self.assertEqual(queue.get(), {
        'line_num': [1],
        'lines': ['1'],
    })
    self.assertEqual(queue.qsize(), 1)
    self.assertEqual(queue.get(), {
        'line_num': [2],
        'lines': ['2'],
    })
    self.assertEqual(queue.qsize(), 0)
    queue.put('3')
    self.assertEqual(queue.get(), {
        'line_num': [3],
        'lines': ['3'],
    })

  def testMultipleTagLines(self):
    queue = tag_block.TagQueue()
    queue.put(r'\s:station1,c:1425344187*78\a')
    queue.put(r'\s:spacecraft2,c:1425344304*0E\b')
    queue.put(r'\c:1425344187,s:station1*78\c')

    self.assertEqual(queue.qsize(), 3)

    item = queue.get()
    self.assertDictContainsSubset2(item, {
        'line_num': [1],
        'lines': ['\\s:station1,c:1425344187*78\\a'],
        'times': [1425344187]})
    expected_match = {
        'group': None,
        'group_id': None,
        'metadata': 's:station1,c:1425344187*78',
        'payload': 'a',
        'rcvr': 'station1',
        'tag_checksum': '78',
        'time': '1425344187'}
    self.assertDictContainsSubset2(item['matches'][0], expected_match)

    item = queue.get()
    self.assertDictContainsSubset2(item, {
        'line_num': [2],
        'lines': ['\\s:spacecraft2,c:1425344304*0E\\b'],
        'times': [1425344304]})
    expected_match = {
        'group': None,
        'group_id': None,
        'metadata': 's:spacecraft2,c:1425344304*0E',
        'payload': 'b',
        'rcvr': 'spacecraft2',
        'tag_checksum': '0E',
        'time': '1425344304'}
    self.assertDictContainsSubset2(item['matches'][0], expected_match)

    item = queue.get()
    self.assertDictContainsSubset2(item, {
        'line_num': [3],
        'lines': ['\\c:1425344187,s:station1*78\\c'],
        'times': [1425344187]})
    expected_match = {
        'group': None,
        'group_id': None,
        'metadata': 'c:1425344187,s:station1*78',
        'rcvr': 'station1',
        'tag_checksum': '78',
        'time': '1425344187'}
    self.assertDictContainsSubset2(item['matches'][0], expected_match)


class TagQueueGroupsTest(TagTestCase):

  def testTwoLines(self):
    lines = (r'\s:rcvr42,g:1-2-13*2A\abc', r'\g:2-2-13*5F\def')
    queue = tag_block.TagQueue()
    queue.put(lines[0])
    self.assertEqual(queue.qsize(), 0)

    queue.put(lines[1])
    self.assertEqual(queue.qsize(), 1)

  def testMissingFirst(self):
    queue = tag_block.TagQueue()
    # Queue should drop the message without the first part.
    queue.put(r'\g:2-2-13*5F\def')
    self.assertEqual(queue.qsize(), 0)
    self.assertEqual(len(queue.groups), 0)

if __name__ == '__main__':
  unittest.main()
