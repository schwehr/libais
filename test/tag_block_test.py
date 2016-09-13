#!/usr/bin/env python
"""Tests for ais.tag_block."""

import unittest

from ais import tag_block


# Large sequence of single sentence AIS messages in TAG block format.
SINGLE_LINE_MESSAGES = r"""
# pylint: disable=line-too-long
\s:rORBCOMM000,q:u,c:1426032001,T:2015-03-11 00.00.01*58\!AIVDM,1,1,,B,19NWuDP0006D=1B<qOuGg6<00000,0*34
\s:rORBCOMM000,q:u,c:1426032499,T:2015-03-11 00.08.19*5c\!AIVDM,1,1,,A,232S8G0P01Q;WaHKSmBH;OvN0H9T,0*6A
\s:rORBCOMM000,q:u,c:1426032001,T:2015-03-11 00.00.01*58\!AIVDM,1,1,,B,38v6Cl5000QDF;MdVMFLC9ol2Dcr,0*0A
\n:3140,s:rZKYSAL1,c:1428451253*1C\!AIVDM,1,1,,A,15NNDKgP00Ic:00EB7v1JgwT0<0f,0*29
\n:486866,s:b003669710,c:1428451208*1D\!SAVDM,1,1,,A,23?v;T5000G?vCjE`Sk9>7H80<1D,0*1D
\n:568297,s:b003669708,c:1428453854*10\!SAVDM,1,1,,B,23?v;T0003G?vC@E`SmMOWJD042h,0*08
\n:3909,s:r08ACERDC,c:1428451287*2D\!ANVDM,1,1,,B,35MvvTg00bqS@H4H0@p0APBl0CNu,0*29
\n:528300,s:b2003669981,c:1428451280*27\!SAVDM,1,1,,B,35NSG`gP@4qDjSlA@Lq6<Cbb013P,0*37
\n:460935,s:b003669987,c:1428451301*1A\!SAVDM,1,1,,A,4@3QiWAuu401Wo=vUTJE:KG00HGg,0*43
\n:812178,s:rB0003160047,c:1428451225*7C\!AIVDM,1,1,,A,73fD4l00h=vt,0*0F
\n:532130,s:b2003669981,c:1428451453*25\!SAVDM,1,1,,A,:3TfS<1GRKkp,0*1C
\n:910748,s:b003669955,c:1428453199*1A\!ABVDM,1,1,,A,;5MuRHQuu40QBIQSq6A<A>i00000,0*10
\n:14595,s:r17MANG1,c:1428452881*35\!AIVDM,1,1,,B,<kBrgmkB77?Q2?Av,0*35
\n:7263,s:r17MYAK1,c:1428452980*12\!AIVDM,1,1,,B,<=AufR?Ff=us2P,4*66
\n:14814,s:r17MANP1,c:1428453934*28\!AIVDM,1,1,,B,>UowsOwM70,4*53
\n:5412,s:r17MCRA1,c:1428453978*15\!AIVDM,1,1,,A,>vHB2oe?w3m=,0*77
\n:755467,s:b003669978,c:1428451310*15\!SAVDO,1,1,,A,?03OwnQGN=CHD00,2*7C
\n:867562,s:b003669953,c:1428451312*14\!SAVDM,1,1,,A,?3S>Rb1GO4lPD00,2*1A
\n:76351,s:b2003669705,c:1428451373*12\!BSVDM,1,1,,B,B5NED1P04uk3Pe6u5116KwqWkP06,0*1E
\n:187203,s:b003669979,c:1428451371*1E\!SAVDM,1,1,,B,B52NRUP00>C>jc4>NkGQ3wqUoP06,0*49
\n:529629,s:b2003669981,c:1428451342*27\!SAVDM,1,1,,B,D03Ow3iehNfp00N01H0f9IH8v9H,2*1B
\n:769973,s:b2003669950,c:1428451343*2C\!SAVDO,1,1,,B,D03OwPAehNfp00N01H8w6D0,2*0E
\n:755872,s:b003669950,c:1428451343*11\!SAVDM,1,1,,B,D03OwPAehNfp00N01H8w6D0,2*0C
\n:276125,s:b003669711,c:1428451243*14\!SAVDM,1,1,,A,ENkb9M940a24W3h97QUh61@41@@;WgTu:lmch00003vP000,2*01
\n:511516,s:b003669709,c:1428451243*1F\!SAVDM,1,1,,B,E>kb9HhaS@:9ah4W17bW2@HW@@@;VrJf:kprh00003vP100,2*7F
\n:816206,s:rB0003160047,c:1428451611*71\!AIVDM,1,1,,A,F030owj2N2P6Ubib@=4q35b10000,0*58
\n:814241,s:rB0003160047,c:1428451423*73\!AIVDM,1,1,,B,Gh30ot3AVTjd6S:9U2h00000;00,2*0C
\n:1723,s:rCUDJOE_01,c:1428451339*7B\!AIVDM,1,1,,A,H52OwN0HtthU<R10hD5=E8F2220,2*2A
\n:186235,s:b003669979,c:1428451341*19\!SAVDM,1,1,,B,H5NIlU4U0000000G46jhpn108410,0*78
\s:rORBCOMM000,q:u,c:1426032579,T:2015-03-11 00.09.39*50\!AIVDM,1,1,,A,H3n6rc4TCBD5PRV<7@2000203330,0*18
\n:9194,s:r17MELF1,c:1428452339*03\!AIVDM,1,1,,A,Is17FMq42OpOKj0,2*05
\n:15342,s:r17MHAI1,c:1428453059*3C\!AIVDM,1,1,,B,JOisW0rQaOuVCl;OE0,4*59
\n:6000,s:r17MPIP1,c:1428453567*0A\!AIVDM,1,1,,A,JE`twenu>wrDpurS,0*6F
\n:4923,s:r17MWAI1,c:1428453625*13\!AIVDM,1,1,,B,JioS<NKkB;7iw0,4*0C
\n:773486,s:b2003669950,c:1428451549*2C\!SAVDM,1,1,,B,K8VSqb9LdU28WP7l,0*48
"""


class TagTestCase(unittest.TestCase):

  def assertDictContainsSubset2(self, actual, expected):
    # assertDictContainsSubset was deprecated because of incorrect arg order.
    # This method has the correct order.
    self.assertIsInstance(expected, dict)
    self.assertIsInstance(actual, dict)
    for key, value in expected.items():
      self.assertEqual(actual[key], value)


class TagBlockTest(TagTestCase):

  def testFractionalTime(self):
    # TAG BLOCK only allows for integer times, but allow for greater precision.
    line = '\\c:1425327399.*70\\'
    match = tag_block.TAG_BLOCK_RE.match(line).groupdict()
    match['time'] = float(match['time'])
    match['time'] = int(match['time'])
    expected = {'time': 1425327399.0}
    self.assertDictContainsSubset2(match, expected)
    self.assertDictContainsSubset2(tag_block.Parse(match), expected)
    self.assertDictContainsSubset2(tag_block.Parse(line), expected)

    line = '\\c:1425327399.0*40\\'
    match = tag_block.TAG_BLOCK_RE.match(line).groupdict()
    match['time'] = float(match['time'])
    expected = {'time': 1425327399.0}
    self.assertDictContainsSubset2(match, expected)
    self.assertDictContainsSubset2(tag_block.Parse(match), expected)
    self.assertDictContainsSubset2(tag_block.Parse(line), expected)

  def testShortSingleLine(self):
    line = (r'\n:121650,s:r17MHOP1,c:1425327399*1D\$'
            'ANZDA,201638.00,02,03,2015,00,00*77')
    match = tag_block.TAG_BLOCK_RE.match(line).groupdict()
    match['line_num'] = int(match['line_num'])
    match['time'] = int(match['time'])
    expected = {
        'line_num': 121650,
        'metadata': 'n:121650,s:r17MHOP1,c:1425327399*1D',
        'payload': '$ANZDA,201638.00,02,03,2015,00,00*77',
        'rcvr': 'r17MHOP1',
        'tag_checksum': '1D',
        'time': 1425327399}
    self.assertDictContainsSubset2(match, expected)
    self.assertDictContainsSubset2(tag_block.Parse(match), expected)
    self.assertDictContainsSubset2(tag_block.Parse(line), expected)

  def testSingleLine(self):
    line = (r'\s:Station,d:somewhere,n:2,q:u,r:123,t:A string.,'
            r'c:1425168552,T:2015-03-01 00.09.12*3A\content')
    match = tag_block.TAG_BLOCK_RE.match(line).groupdict()
    match['line_num'] = int(match['line_num'])
    match['rel_time'] = int(match['rel_time'])
    match['time'] = int(match['time'])
    expected = {
        'dest': 'somewhere',
        'group': None,
        'group_id': None,
        'line_num': 2,
        'payload': 'content',
        'quality': 'u',
        'rcvr': 'Station',
        'rel_time': 123,
        'sentence_num': None,
        'sentence_tot': None,
        'tag_checksum': '3A',
        'text': 'A string.',
        'text_date': '2015-03-01 00.09.12',
        'time': 1425168552}
    self.assertDictContainsSubset2(match, expected)
    self.assertDictContainsSubset2(tag_block.Parse(match), expected)
    self.assertDictContainsSubset2(tag_block.Parse(line), expected)

  def testFirstOfGroup(self):
    line = (r'\g:1-3-42349,n:111458,s:r003669945,c:1425327424*41\!'
            'AIVDM,2,1,5,A,ENk`sPa17ab7W@7@1T@6;Q@0h@@=MeR4<7rpH00003v,0*16')
    match = tag_block.TAG_BLOCK_RE.match(line).groupdict()
    match['group_id'] = int(match['group_id'])
    match['line_num'] = int(match['line_num'])
    match['sentence_num'] = int(match['sentence_num'])
    match['sentence_tot'] = int(match['sentence_tot'])
    match['time'] = int(match['time'])
    expected = {
        'group': '1-3-42349',
        'group_id': 42349,
        'line_num': 111458,
        'metadata': 'g:1-3-42349,n:111458,s:r003669945,c:1425327424*41',
        'payload':
            '!AIVDM,2,1,5,A,ENk`sPa17ab7W@7@1T@6;Q@0h@@=MeR4<7rpH00003v,0*16',
        'rcvr': 'r003669945',
        'sentence_num': 1,
        'sentence_tot': 3,
        'tag_checksum': '41',
        'time': 1425327424}
    self.assertDictContainsSubset2(match, expected)
    self.assertDictContainsSubset2(tag_block.Parse(match), expected)
    self.assertDictContainsSubset2(tag_block.Parse(line), expected)

  def testMiddleOfGroup(self):
    line = (r'\g:2-3-42349,n:111459*15\!AIVDM,2,2,5,A,P000,2*71')
    match = tag_block.TAG_BLOCK_RE.match(line).groupdict()
    match['group_id'] = int(match['group_id'])
    match['line_num'] = int(match['line_num'])
    match['sentence_num'] = int(match['sentence_num'])
    match['sentence_tot'] = int(match['sentence_tot'])
    expected = {
        'group': '2-3-42349',
        'group_id': 42349,
        'line_num': 111459,
        'metadata': 'g:2-3-42349,n:111459*15',
        'payload': '!AIVDM,2,2,5,A,P000,2*71',
        'sentence_num': 2,
        'sentence_tot': 3,
        'tag_checksum': '15'}
    self.assertDictContainsSubset2(match, expected)
    self.assertDictContainsSubset2(tag_block.Parse(match), expected)
    self.assertDictContainsSubset2(tag_block.Parse(line), expected)

  def testEndOfGroup(self):
    line = (r'\g:3-3-42349,n:111460*1E\$'
            'ARVSI,r003669945,5,201704.05687473,0152,-085,0*2E')
    match = tag_block.TAG_BLOCK_RE.match(line).groupdict()
    match['group_id'] = int(match['group_id'])
    match['line_num'] = int(match['line_num'])
    match['sentence_num'] = int(match['sentence_num'])
    match['sentence_tot'] = int(match['sentence_tot'])
    expected = {
        'group': '3-3-42349',
        'group_id': 42349,
        'line_num': 111460,
        'metadata': 'g:3-3-42349,n:111460*1E',
        'payload': '$ARVSI,r003669945,5,201704.05687473,0152,-085,0*2E',
        'sentence_num': 3,
        'sentence_tot': 3,
        'tag_checksum': '1E'}
    self.assertDictContainsSubset2(match, expected)
    self.assertDictContainsSubset2(tag_block.Parse(match), expected)
    self.assertDictContainsSubset2(tag_block.Parse(line), expected)

  def testOrbcommFieldT(self):
    line = (r'\s:rORBCOMM999,q:u,c:1424995200,T:2015-02-27 00.00.00*51\!'
            'AIVDM,1,1,,A,14VIk0002sMM04vE>V9jGimn08RP,0*0D')
    match = tag_block.TAG_BLOCK_RE.match(line).groupdict()
    match['time'] = int(match['time'])
    expected = {
        'metadata': 's:rORBCOMM999,q:u,c:1424995200,T:2015-02-27 00.00.00*51',
        'payload': '!AIVDM,1,1,,A,14VIk0002sMM04vE>V9jGimn08RP,0*0D',
        'quality': 'u',
        'rcvr': 'rORBCOMM999',
        'tag_checksum': '51',
        'text_date': '2015-02-27 00.00.00',
        'time': 1424995200}
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
        'line_nums': [1],
        'lines': ['1'],
    })
    self.assertEqual(queue.qsize(), 1)
    self.assertEqual(queue.get(), {
        'line_nums': [2],
        'lines': ['2'],
    })
    self.assertEqual(queue.qsize(), 0)
    queue.put('3')
    self.assertEqual(queue.get(), {
        'line_nums': [3],
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
        'line_nums': [1],
        'lines': ['\\s:station1,c:1425344187*78\\a'],
        'times': [1425344187]})
    expected_match = {
        'group': None,
        'group_id': None,
        'metadata': 's:station1,c:1425344187*78',
        'payload': 'a',
        'rcvr': 'station1',
        'tag_checksum': '78',
        'time': 1425344187}
    self.assertDictContainsSubset2(item['matches'][0], expected_match)

    item = queue.get()
    self.assertDictContainsSubset2(item, {
        'line_nums': [2],
        'lines': ['\\s:spacecraft2,c:1425344304*0E\\b'],
        'times': [1425344304]})
    expected_match = {
        'group': None,
        'group_id': None,
        'metadata': 's:spacecraft2,c:1425344304*0E',
        'payload': 'b',
        'rcvr': 'spacecraft2',
        'tag_checksum': '0E',
        'time': 1425344304}
    self.assertDictContainsSubset2(item['matches'][0], expected_match)

    item = queue.get()
    self.assertDictContainsSubset2(item, {
        'line_nums': [3],
        'lines': ['\\c:1425344187,s:station1*78\\c'],
        'times': [1425344187]})
    expected_match = {
        'group': None,
        'group_id': None,
        'metadata': 'c:1425344187,s:station1*78',
        'rcvr': 'station1',
        'tag_checksum': '78',
        'time': 1425344187}
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


class DecodeTagBlockTest(TagTestCase):

  def testSingleLineMessage(self):
    # Do one detailed comparison.
    expected = {
        'decoded': {
            'cog': 142,
            'gnss': True,
            'id': 27,
            'md5': 'fb6a0946b7dc8a6a4f5d06db113793d5',
            'mmsi': 577305000,
            'nav_status': 5,
            'position_accuracy': 1,
            'raim': False,
            'repeat_indicator': 0,
            'sog': 0,
            'spare': 0,
            'x': -90.20666666666666,
            'y': 29.145},
        'line_nums': [1],
        # pylint: disable=line-too-long
        'lines': ['\\n:852057,s:b003669955,c:1428451729*1B\\!ABVDM,1,1,,A,K8VSqb9LdU28WP8p,0*49'],
        'matches': [{
            'dest': None,
            'group': None,
            'group_id': None,
            'line_num': 852057,
            'metadata': 'n:852057,s:b003669955,c:1428451729*1B',
            'payload': '!ABVDM,1,1,,A,K8VSqb9LdU28WP8p,0*49',
            'quality': None,
            'rcvr': 'b003669955',
            'rel_time': None,
            'sentence_num': None,
            'sentence_tot': None,
            'tag_checksum': '1B',
            'text': None,
            'text_date': None,
            'time': 1428451729}],
        'times': [1428451729]}

    line = (
        # pylint: disable=line-too-long
        r'\n:852057,s:b003669955,c:1428451729*1B\!ABVDM,1,1,,A,K8VSqb9LdU28WP8p,0*49'
    )

    queue = tag_block.TagQueue()
    queue.put(line)
    self.assertEqual(queue.qsize(), 1)
    msg = queue.get(line)
    self.assertEqual(msg, expected)

  def testManySingleLineMessages(self):
    lines = [line for line in SINGLE_LINE_MESSAGES.split('\n') if 'VD' in line]
    queue = tag_block.TagQueue()
    for line in lines:
      queue.put(line)
    self.assertEqual(queue.qsize(), len(lines))
    msgs = []
    while not queue.empty():
      msgs.append(queue.get())
    self.assertEqual(len(msgs), len(lines))
    for msg in msgs:
      self.assertIn('decoded', msg)
    id_list = [msg['decoded']['id'] for msg in msgs]
    self.assertEqual(
        id_list,
        [
            1, 2, 3, 1, 2, 2, 3, 3, 4, 7, 10, 11, 12, 12, 14, 14, 15, 15, 18,
            18, 20, 20, 20, 21, 21, 22, 23, 24, 24, 24, 25, 26, 26, 26, 27
        ])

  def testTwoLineGroupWithSingleLineMessage(self):

    lines = (
        # pylint: disable=line-too-long
        r'\g:1-2-89372,n:192113,s:r003669946,c:1428451252*41\!AIVDM,1,1,,A,181520h000JstMLHbOFc1CUd0<07,0*35',
        r'\g:2-2-89372,n:192114*1C\$ARVSI,r003669946,,000052.59031009,1972,-108,0*1B'
        )
    queue = tag_block.TagQueue()
    for line in lines:
      queue.put(line)
    self.assertEqual(queue.qsize(), 1)
    msg = queue.get()
    self.assertEqual(msg['decoded']['id'], 1)

  def testTwoLineGroupWithTwoLineMessage(self):
    lines = (
        # pylint: disable=line-too-long
        r'\g:1-2-9459,s:rORBCOMM009,c:1426032120,T:2015-03-11 00.02.00*31\!AIVDM,2,1,9,A,59NS9142>SW@7PQWR20u84pLF1=Dr2222222221SDHa?A0l;`CDhCU3lp888,0*5B',
        r'\g:2-2-9459,s:rORBCOMM009,c:1426032120,T:2015-03-11 00.02.00*32\!AIVDM,2,2,9,A,88888888880,2*2D',
        )
    queue = tag_block.TagQueue()
    for line in lines:
      queue.put(line)
    self.assertEqual(queue.qsize(), 1)
    msg = queue.get()
    self.assertEqual(msg['decoded']['id'], 5)
    self.assertEqual(msg['times'], [1426032120, 1426032120])

  def testThreeLineGroupWithTwoLineMessage(self):
    lines = (
        # pylint: disable=line-too-long
        r'\g:1-3-6417,n:4504,s:D13MN-PS-MTEBS1,c:1428451206*07\!SAVDM,2,1,7,B,59NSGLD2Cn5@CDLkN21Tu8dL5@F2222222222216EHMC=4w`0L@hEPC`8888,0*1A',
        r'\g:2-3-6417,n:4505*24\!SAVDM,2,2,7,B,88888888880,2*3A',
        r'\g:3-3-6417,n:4506*26\$SAVSI,D13MN-PS-MTEBS1,7,000006.831194,256,-97,19*67',
    )
    queue = tag_block.TagQueue()
    for line in lines:
      queue.put(line)
    self.assertEqual(queue.qsize(), 1)
    msg = queue.get()
    self.assertEqual(msg['decoded']['id'], 5)
    self.assertEqual(msg['times'], [1428451206, None, None])


if __name__ == '__main__':
  unittest.main()
