"""Tests for ais.nmea_queue."""

import contextlib
import unittest

import pytest
import six
from six.moves import StringIO

import ais
from ais import nmea
from ais import nmea_queue

BARE_NMEA = """
# pylint: disable=line-too-long
$GPZDA,203003.00,12,07,2009,00,00,*47
!AIVDM,1,1,,B,23?up2001gGRju>Ap:;R2APP08:c,0*0E
!BSVDM,1,1,,A,15Mj23`PB`o=Of>KjvnJg8PT0L2R,0*7E
!SAVDM,1,1,,B,35Mj2p001qo@5tVKLBWmIDJT01:@,0*33
!AIVDM,1,1,,A,B5NWV1P0<vSE=I3QdK4bGwoUoP06,0*4F
!SAVDM,1,1,,A,403Owi1utn1W0qMtr2AKStg020S:,0*4B
!SAVDM,2,1,4,A,55Mub7P00001L@;SO7TI8DDltqB222222222220O0000067<0620@jhQDTVG,0*43
!SAVDM,2,2,4,A,30H88888880,2*49
"""

TAG_BLOCK = r"""
# pylint: disable=line-too-long
\n:440661,s:r3669963,c:1428537660*0F\$GPZDA,000253,09,04,2015,+00,00*6C
\g:1-2-4372,s:rORBCOMM109,c:1426032000,T:2015-03-11 00.00.00*32\!AIVDM,2,1,2,B,576u>F02>hOUI8AGR20tt<j104p4l62222222216H14@@Hoe0JPEDp1TQH88,0*16
\s:rORBCOMM999u,c:1426032000,T:2015-03-11 00.00.00*36\!AIVDM,1,1,,,;5Qu0v1utmGssvvkA`DRgm100000,0*46
\g:2-2-4372,s:rORBCOMM109,c:1426032000,T:2015-03-11 00.00.00*31\!AIVDM,2,2,2,B,88888888880,2*25
\g:1-2-27300,n:636994,s:b003669710,c:1428621738*5F\!SAVDM,2,1,2,B,55Mw@A7J1adAL@?;7WPl58F0U<h4pB222222220t1PN5553fN4g?`4iSp5Rc,0*26
\g:2-2-27300,n:636995*15\!SAVDM,2,2,2,B,iP`88888880,2*5E
\n:636996,s:b003669710,c:1428621738*19\!SAVDM,1,1,,B,35Mv4LPP@Go?FFtEbDDWQmlT20k@,0*04
\g:4-4-993623,n:577969*22\$ARVSI,r003669930,,233948.825272,1831,-97,0*24
\n:80677,s:b003669952,c:1428884269*2A\!SAVDM,1,1,,B,K8VSqb9LdU28WP8<,0*17
"""

USCG = r"""
# pylint: disable=line-too-long
!SAVDM,1,1,,A,15N4OMPP01I<cGrA1v>Id?vF060l,0*22,b003669978,1429287189
!SAVDM,2,1,4,B,54h@7?02BAF=`L4wN21<eTH4hj2222222222220U4HG6553U06T0C3H0Q@@j,0*5D,d-86,S389,t161310.00,T10.377780,D07MN-MI-LAKBS1,1429287190
!SAVDM,2,2,4,B,88888888880,2*39,d-86,S389,t161310.00,T10.377780,D07MN-MI-LAKBS1,1429287190
!AIVDM,1,1,,B,3592u`iP03GWEflBRosm0Ov@0000,0*70,d-107,S0297,t161407.00,T07.92201452,r11CSDO1,1429287248
!SAVDM,1,1,,B,K8VSqb9LdU28WP8<,0*17,rMySat,1429287258
"""

MIXED = r"""
!SAVDM,1,1,,A,15N4OMPP01I<cGrA1v>Id?vF060l,0*22,b003669978,1429287189
!SAVDM,1,1,,A,403Owi1utn1W0qMtr2AKStg020S:,0*4B
\n:80677,s:b003669952,c:1428884269*2A\!SAVDM,1,1,,B,K8VSqb9LdU28WP8<,0*17
random text
"""


class NmeaQueueTest(unittest.TestCase):

  def testTextData(self):
    # These lines should all pass straight through.
    src_lines = (
        '',
        'a',
        '123',
        # Not quite NMEA strings.
        '$GPZDA',
        '!AIVDM',
        '*FF',)
    queue = nmea_queue.NmeaQueue()
    for line in src_lines:
      queue.put(line)

    self.assertEqual(queue.qsize(), len(src_lines))
    for i in range(1, queue.qsize() + 1):
      msg = queue.get()
      self.assertEqual(msg['line_nums'], [i])
      self.assertEqual(msg['line_type'], nmea.TEXT)
      self.assertEqual(msg['lines'], list(src_lines[i-1:i]))

    self.assertEqual(msg,
                     {'line_nums': [6], 'line_type': 'TEXT', 'lines': ['*FF']})

  def testBareSingleLineData(self):
    queue = nmea_queue.NmeaQueue()
    lines = [line for line in BARE_NMEA.split('\n') if ',' in line]
    for line in lines:
      queue.put(line)
    self.assertEqual(queue.qsize(), 7)
    msgs = []
    while not queue.empty():
      msgs.append(queue.get())

    self.assertEqual(msgs[0],
                     {'line_nums': [1],
                      'line_type': 'BARE',
                      'lines': ['$GPZDA,203003.00,12,07,2009,00,00,*47']})
    self.assertEqual(
        msgs[1],
        {'decoded': {
            'cog': 52.099998474121094,
            'id': 2,
            'md5': '99c8c2804fde0481e6143051930b66c4',
            'mmsi': 218069000,
            'nav_status': 0,
            'position_accuracy': 0,
            'raim': False,
            'repeat_indicator': 0,
            'rot': 0.0,
            'rot_over_range': False,
            'slot_number': 683,
            'slot_timeout': 2,
            'sog': 11.100000381469727,
            'spare': 0,
            'special_manoeuvre': 0,
            'sync_state': 0,
            'timestamp': 16,
            'true_heading': 48,
            'x': -118.227775,
            'y': 31.24317},
         'line_nums': [2],
         'line_type': 'BARE',
         'lines': ['!AIVDM,1,1,,B,23?up2001gGRju>Ap:;R2APP08:c,0*0E'],
         'matches': [{
             'body': '23?up2001gGRju>Ap:;R2APP08:c',
             'chan': 'B',
             'checksum': '0E',
             'fill_bits': 0,
             'sen_num': 1,
             'sen_tot': 1,
             'seq_id': None,
             'talker': 'AI',
             'vdm_type': 'VDM',
             'vdm': '!AIVDM,1,1,,B,23?up2001gGRju>Ap:;R2APP08:c,0*0E'}]}
    )

  def testTagBlockLines(self):
    queue = nmea_queue.NmeaQueue()
    lines = [line for line in TAG_BLOCK.split('\n') if ',' in line]
    for line in lines:
      queue.put(line)
    self.assertEqual(queue.qsize(), 6)
    msgs = []
    while not queue.empty():
      msgs.append(queue.get())

    # self.assertNotIn('decoded', msgs[0])
    # TODO(schwehr): Check the ZDA message decoding.
    for msg_num in range(1, 5):
      self.assertIn('decoded', msgs[msg_num])
    ids = [msg['decoded']['id'] for msg in msgs[1:] if 'decoded' in msg]
    self.assertEqual(ids, [11, 5, 5, 3, 27])

    self.assertEqual(
        msgs[-1],
        {'decoded': {
            'cog': 131,
            'gnss': True,
            'id': 27,
            'md5': '50898a3435865cf76f1b502b2821672b',
            'mmsi': 577305000,
            'nav_status': 5,
            'position_accuracy': 1,
            'raim': False,
            'repeat_indicator': 0,
            'sog': 0,
            'spare': 0,
            'x': -90.20666666666666,
            'y': 29.145},
         'line_nums': [9],
         'line_type': 'TAGB',
         'lines': [
             '\\n:80677,s:b003669952,c:1428884269*2A'
             '\\!SAVDM,1,1,,B,K8VSqb9LdU28WP8<,0*17'],
         'matches': [{
             'dest': None,
             'group': None,
             'group_id': None,
             'line_num': 80677,
             'metadata': 'n:80677,s:b003669952,c:1428884269*2A',
             'payload': '!SAVDM,1,1,,B,K8VSqb9LdU28WP8<,0*17',
             'quality': None,
             'rcvr': 'b003669952',
             'rel_time': None,
             'sentence_num': None,
             'sentence_tot': None,
             'tag_checksum': '2A',
             'text': None,
             'text_date': None,
             'time': 1428884269}],
         'times': [1428884269]})

  def testUscgLines(self):
    queue = nmea_queue.NmeaQueue()
    lines = [line for line in USCG.split('\n') if ',' in line]
    for line in lines:
      queue.put(line)

    self.assertEqual(queue.qsize(), 4)
    msgs = []
    while not queue.empty():
      msgs.append(queue.get())

    for msg in msgs:
      self.assertIn('decoded', msg)
    ids = [msg['decoded']['id'] for msg in msgs]
    self.assertEqual(ids, [1, 5, 3, 27])

    self.assertEqual(
        msgs[3],
        {
            'decoded': {
                'cog': 131,
                'gnss': True,
                'id': 27,
                'md5': '50898a3435865cf76f1b502b2821672b',
                'mmsi': 577305000,
                'nav_status': 5,
                'position_accuracy': 1,
                'raim': False,
                'repeat_indicator': 0,
                'sog': 0,
                'spare': 0,
                'x': -90.20666666666666,
                'y': 29.145},
            'line_nums': [5],
            'line_type': 'USCG',
            'lines': ['!SAVDM,1,1,,B,K8VSqb9LdU28WP8<,0*17,rMySat,1429287258'],
            'matches': [{
                'body': 'K8VSqb9LdU28WP8<',
                'chan': 'B',
                'checksum': '17',
                'counter': None,
                'fill_bits': 0,
                'hour': None,
                'minute': None,
                'payload': '!SAVDM,1,1,,B,K8VSqb9LdU28WP8<,0*17',
                'receiver_time': None,
                'rssi': None,
                'second': None,
                'sen_num': 1,
                'sen_tot': 1,
                'seq_id': None,
                'signal_strength': None,
                'slot': None,
                'station': 'rMySat',
                'station_type': 'r',
                'talker': 'SA',
                'time': 1429287258,
                'time_of_arrival': None,
                'uscg_metadata': ',rMySat,1429287258',
                'vdm': '!SAVDM,1,1,,B,K8VSqb9LdU28WP8<,0*17',
                'vdm_type': 'VDM'}]})

  def testMixedLines(self):
    queue = nmea_queue.NmeaQueue()
    lines = [line for line in MIXED.split('\n') if line.strip()]
    for line in lines:
      queue.put(line)

    self.assertEqual(queue.qsize(), 4)
    msgs = []
    while not queue.empty():
      msgs.append(queue.get())

    for msg in msgs[:-1]:
      self.assertIn('decoded', msg)
    ids = [msg['decoded']['id'] for msg in msgs[:-1]]
    self.assertEqual(ids, [1, 4, 27])

    line_types = [msg['line_type'] for msg in msgs]
    self.assertEqual(
        line_types,
        [nmea.USCG, nmea.BARE, nmea.TAGB, nmea.TEXT])


@pytest.mark.parametrize("nmea", [
    six.text_type(BARE_NMEA.strip()),
    six.text_type(TAG_BLOCK.strip()),
    six.text_type(USCG.strip()),
    six.text_type(MIXED.strip())
])
def test_NmeaFile_against_queue(nmea):

    queue = nmea_queue.NmeaQueue()
    for line in nmea.splitlines():
        queue.put(line)

    expected = []
    msg = queue.GetOrNone()
    while msg:
        expected.append(msg)
        msg = queue.GetOrNone()

    with contextlib.closing(StringIO(nmea)) as f, ais.open(f) as src:
        actual = list(src)

    for e, a in zip(expected, actual):
        assert e == a


if __name__ == '__main__':
  unittest.main()
