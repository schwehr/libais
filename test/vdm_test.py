#!/usr/bin/env python

"""Tests for ais.vdm."""

import unittest

from ais import vdm
import six


class TestCase(unittest.TestCase):

  def assertDictContainsSubset2(self, actual, expected):
    # assertDictContainsSubset was deprecated because of incorrect arg order.
    # This method has the correct order.
    self.assertIsInstance(expected, dict)
    self.assertIsInstance(actual, dict)
    for key, value in expected.items():
      self.assertEqual(actual[key], value, 'kv: %s, %s' % (key, value))


class VdmRegexTest(unittest.TestCase):
  """Test handling simple NMEA messages without TAG BLOCK info."""

  def testRegexSingleLineMessage(self):
    line = '!AIVDM,1,1,,A,14VIk0002sMM04vE>V9jGimn08RP,0*0D'
    match = vdm.VDM_RE.match(line).groupdict()
    expected = {
        'body': '14VIk0002sMM04vE>V9jGimn08RP',
        'chan': 'A',
        'checksum': '0D',
        'fill_bits': '0',
        'sen_num': '1',
        'seq_id': None,
        'vdm_type': 'VDM',
        'talker': 'AI',
        'sen_tot': '1',
        'vdm': '!AIVDM,1,1,,A,14VIk0002sMM04vE>V9jGimn08RP,0*0D'}
    self.assertEqual(match, expected)

  def testRegexTwoLineMessage(self):
    lines = (
        # pylint: disable=line-too-long
        '!AIVDM,2,1,2,B,5KNp?6@00000=Phb220u1@Tlv0TV22222222220N0h:22400000000000000,0*2A',
        '!AIVDM,2,2,2,B,00000000000,2*25')
    expected = (
        {'body': '5KNp?6@00000=Phb220u1@Tlv0TV22222222220N0h:22400000000000000',
         'chan': 'B',
         'checksum': '2A',
         'fill_bits': '0',
         'sen_num': '1',
         'seq_id': '2',
         'vdm_type': 'VDM',
         'talker': 'AI',
         'sen_tot': '2',
         # pylint: disable=line-too-long
         'vdm': '!AIVDM,2,1,2,B,5KNp?6@00000=Phb220u1@Tlv0TV22222222220N0h:22400000000000000,0*2A'
        },
        {'body': '00000000000',
         'chan': 'B',
         'checksum': '25',
         'fill_bits': '2',
         'sen_num': '2',
         'seq_id': '2',
         'vdm_type': 'VDM',
         'talker': 'AI',
         'sen_tot': '2',
         'vdm': '!AIVDM,2,2,2,B,00000000000,2*25'
        })
    matches = [vdm.VDM_RE.match(line).groupdict() for line in lines]
    self.assertEqual(matches[0], expected[0])
    self.assertEqual(matches[1], expected[1])


class VdmLinesTest(unittest.TestCase):

  def testVdmLinesGenerator(self):
    lines = (
        '',
        '!AIVDM,2,2,2,B,00000000000,2*25 \n',
        '$ARVSI,r003669945,5,201704.05687473,0152,-085,0*2E',
        r'\g:3-3-42349,n:111460*1E\$',
    )
    generator = vdm.VdmLines(lines)
    self.assertEqual(six.next(generator), '!AIVDM,2,2,2,B,00000000000,2*25')
    self.assertRaises(StopIteration, six.next, generator)


class ParseTest(TestCase):

  def testSingleLineParse(self):
    line = '!AIVDM,1,1,,A,B7OeqLP0mB4<LFKO7noASwcUoP06,0*51'
    message = vdm.Parse(line)
    expected = {
        'body': 'B7OeqLP0mB4<LFKO7noASwcUoP06',
        'checksum': '51',
        'vdm_type': 'VDM',
        'fill_bits': 0,
        'talker': 'AI',
        'chan': 'A',
        'sen_num': 1,
        'seq_id': None,
        'sen_tot': 1}
    self.assertDictContainsSubset2(message, expected)


class BareQueueTest(unittest.TestCase):

  def setUp(self):
    self.queue = vdm.BareQueue()

  def testPassThroughText(self):
    # Pass through anything that does not totally match a NMEA sentence.
    lines = (
        '',
        '\n',
        ' \n',
        ' \n\r',
        '# comment',
        '$GPZDA,050004,29,11,2012,-5,00*',  # No checksum.
        'GPZDA,050013,29,11,2012,-5,00*5D',  # No initial [$!].
    )
    for line_num, line in enumerate(lines):
      self.queue.put(line)
      self.assertEqual(self.queue.qsize(), 1)
      expected = {
          'line_nums': [line_num + 1],
          'lines': [line.rstrip()]}
      message = self.queue.get()
      self.assertEqual(message, expected,
                       'pass through fail. %d "%s"\n  %s != %s'
                       % (line_num, line, message, expected))
      self.assertEqual(self.queue.qsize(), 0)

  def testSingleLineNmeaSentencesIgnored(self):
    lines = (
        '$GPGGA,000000,4308.1252,N,07056.3763,W,2,9,0.9,35.2,M,,,,*0A',
        '$HCHDT,25.6,T*18',
        '$GPVTG,269.5,T,284.9,M,0.0,N,0.0,K,D*29',
        '$WIMDA,29.1166,I,0.9860,B,19.6,C,,,,,,,175.8,T,191.2,M,2.0,N,1.0,M*2B',
        '$GPVTG,268.6,T,284.0,M,0.1,N,0.1,K,D*2',
        '$GPGGA,000003,4308.1252,N,07056.3763,W,2,9,0.9,35.3,M,,,,*08')

    for line_num, line in enumerate(lines):
      self.queue.put(line)
      self.assertEqual(self.queue.qsize(), 1)
      expected = {
          'line_nums': [line_num + 1],
          'lines': [line.rstrip()]}
      message = self.queue.get()
      self.assertEqual(message, expected)
      self.assertEqual(self.queue.qsize(), 0)

  def testSingleLineVdm(self):
    # Fully check a single line AIS VDM message going through the
    # queue and being decoded.

    line_num = 100

    line = '!SAVDM,1,1,,B,K8VSqb9LdU28WP8P,0*7B'
    self.queue.put(line, line_num)
    self.assertEqual(self.queue.qsize(), 1)
    expected = {
        'line_nums': [line_num],
        'lines': [line.rstrip()],
        'decoded': {
            'sog': 0,
            'spare': 0,
            'raim': False,
            'gnss': True,
            'position_accuracy': 1,
            'nav_status': 5,
            'repeat_indicator': 0,
            'y': 29.145,
            'x': -90.20666666666666,
            'cog': 136,
            'md5': '6e1a4872825054e91ee7cfcfb9cc87e0',
            'mmsi': 577305000,
            'id': 27},
        'matches': [{
            'body': 'K8VSqb9LdU28WP8P',
            'fill_bits': 0,
            'sen_num': 1,
            'sen_tot': 1,
            'vdm_type': 'VDM',
            'talker': 'SA',
            'chan': 'B',
            'seq_id': None,
            'checksum': '7B',
            'vdm': '!SAVDM,1,1,,B,K8VSqb9LdU28WP8P,0*7B'}]
    }
    message = self.queue.get()
    self.assertEqual(message, expected)
    self.assertEqual(self.queue.qsize(), 0)

  def testManySingleLineVdms(self):
    # Pass a series of independent one line messages through a queue.
    # All of these lines should be decoded.  The only interaction
    # between messages is that the line number is incremented.
    # Only do spot checks on the results.
    lines = (
        '!AIVDM,1,1,,A,15N:pmP002Jd``FGB:hm619`00R5,0*42',
        '!BSVDO,1,1,,B,4h3OdJQutqGssIw1T`JFhg700d09,0*34',
        '!SAVDM,1,1,,B,78KDut1BAFeu,0*5A',
        '!AIVDM,1,1,,B,9oVAuAI5;rRRv2OqTi?1uoP?=a@1,0*74',
        '!AIVDM,1,1,,A,:4`bLl0p3;Qd,0*77',
        '!AIVDM,1,1,,A,;3P<f6iuiq00aOUu8DOD@j100000,0*44',
        '!AIVDM,1,1,,B,B6:VU2P0<:;2r84N5obLOwR2P0S9,0*23')

    for count, line in enumerate(lines):
      self.queue.put(line)
      self.assertEqual(self.queue.qsize(), count+1)

    messages = []
    while not self.queue.empty():
      messages.append(self.queue.get())

    self.assertEqual(len(messages), len(lines))
    self.assertEqual(
        [message['line_nums'][0] for message in messages],
        [1, 2, 3, 4, 5, 6, 7])
    for message in messages:
      self.assertEqual(len(message['line_nums']), 1)
      self.assertEqual(len(message['lines']), 1)
      self.assertEqual(len(message['matches']), 1)

    # Spot check each message.
    self.assertEqual(messages[0]['decoded']['id'], 1)
    self.assertEqual(messages[0]['decoded']['true_heading'], 36)
    self.assertEqual(messages[0]['matches'][0]['chan'], 'A')

    self.assertEqual(messages[1]['decoded']['id'], 4)
    self.assertEqual(messages[1]['decoded']['fix_type'], 7)
    self.assertEqual(messages[1]['matches'][0]['vdm_type'], 'VDO')

    self.assertEqual(messages[2]['decoded']['id'], 7)
    self.assertEqual(messages[2]['decoded']['acks'][0], (345070303, 1))
    self.assertEqual(messages[2]['matches'][0]['talker'], 'SA')

    self.assertEqual(messages[3]['decoded']['id'], 9)
    self.assertEqual(messages[3]['decoded']['alt'], 2324)
    self.assertEqual(messages[3]['matches'][0]['sen_num'], 1)

    self.assertEqual(messages[4]['decoded']['id'], 10)
    self.assertEqual(messages[4]['decoded']['dest_mmsi'], 235089435)
    self.assertEqual(messages[4]['matches'][0]['sen_tot'], 1)

    self.assertEqual(messages[5]['decoded']['id'], 11)
    self.assertEqual(messages[5]['decoded']['year'], 2012)
    self.assertIsNone(messages[5]['matches'][0]['seq_id'])

    self.assertEqual(messages[6]['decoded']['id'], 18)
    self.assertEqual(messages[6]['decoded']['band_flag'], 1)
    self.assertEqual(messages[6]['matches'][0]['checksum'], '23')

  def testTwoLineMessage(self):
    lines = (
        # pylint: disable=line-too-long
        '!ABVDM,2,1,2,A,55NJPwP00001L@K?77@DhhU>0@5HU>222222220O18@374B<08CCm2EPH0kk,0*6D',
        '!ABVDM,2,2,2,A,UQCU8888880,2*3F')
    self.assertEqual(len(lines), 2)

    self.queue.put(lines[0])
    self.assertEqual(self.queue.qsize(), 0)
    self.queue.put(lines[1])
    self.assertEqual(self.queue.qsize(), 1)
    expected = {
        'line_nums': [1, 2],
        'lines': [line.rstrip() for line in lines],
        'decoded': {
            'ais_version': 0,
            'callsign': 'WDF3114',
            'destination': 'MOTIVA CONVENT      ',
            'dim_a': 9,
            'dim_b': 16,
            'dim_c': 3,
            'dim_d': 7,
            'draught': 3.299999952316284,
            'dte': 0,
            'eta_day': 4,
            'eta_hour': 12,
            'eta_minute': 0,
            'eta_month': 1,
            'fix_type': 1,
            'id': 5,
            'imo_num': 0,
            'md5': '403a171048302b7f1515f09131238db4',
            'mmsi': 367436030,
            'name': 'ELLIS DAVIS         ',
            'repeat_indicator': 0,
            'spare': 0,
            'type_and_cargo': 31},
        'matches': [
            {
                'body': ('55NJPwP00001L@K?77@DhhU>0@5HU>222222220O18@374B<08C'
                         'Cm2EPH0kk'),
                'chan': 'A',
                'checksum': '6D',
                'fill_bits': 0,
                'sen_num': 1,
                'sen_tot': 2,
                'seq_id': 2,
                'talker': 'AB',
                'vdm_type': 'VDM',
                # pylint: disable=line-too-long
                'vdm': '!ABVDM,2,1,2,A,55NJPwP00001L@K?77@DhhU>0@5HU>222222220O18@374B<08CCm2EPH0kk,0*6D'},
            {
                'body': 'UQCU8888880',
                'chan': 'A',
                'checksum': '3F',
                'fill_bits': 2,
                'sen_num': 2,
                'sen_tot': 2,
                'seq_id': 2,
                'talker': 'AB',
                'vdm_type': 'VDM',
                'vdm': '!ABVDM,2,2,2,A,UQCU8888880,2*3F'}]}
    message = self.queue.get()
    self.assertEqual(message, expected)
    self.assertEqual(self.queue.qsize(), 0)

  def testThreeLineMessage(self):
    lines = (
        # pylint: disable=line-too-long
        '!AIVDM,3,1,4,A,81mg=5@0EP:4R40807P>0<D1>MNt00000f>FNVfnw7>6>FNU=?B5PD5HDPD8,0*26',
        '!AIVDM,3,2,4,A,1Dd2J09jL08JArJH5P=E<D9@<5P<9>0`bMl42Q0d2Pc2T59CPCE@@?C54PD?,0*60',
        '!AIVDM,3,3,4,A,d0@d0IqhH:Pah:U54PD?75D85Bf00,0*03')
    self.assertEqual(len(lines), 3)

    self.queue.put(lines[0])
    self.assertEqual(self.queue.qsize(), 0)

    self.queue.put(lines[1])
    self.assertEqual(self.queue.qsize(), 0)

    self.queue.put(lines[2])
    self.assertEqual(self.queue.qsize(), 1)

    expected = {
        'line_nums': [1, 2, 3],
        'lines': [line.rstrip() for line in lines],
        'decoded': {
            'md5': 'f463fcabb7540e412af5d7238f955b0e',
            'dac': 1,
            'day': 1,
            'duration_minutes': 1,
            'fi': 22,
            'hour': 0,
            'id': 8,
            'link_id': 10,
            'minute': 1,
            'mmsi': 123456789,
            'month': 1,
            'notice_type': 9,
            'notice_type_str': 'Caution Area: Marine event',
            'repeat_indicator': 0,
            'spare': 0,
            'sub_areas': [
                {
                    'precision': 4,
                    'radius': 0,
                    'sub_area_type': 0,
                    'sub_area_type_str': 'point',
                    'x': -69.8,
                    'y': 42.849983333333334},
                {
                    'sub_area_type': 5,
                    'sub_area_type_str': 'text',
                    'text': '12345678901234'},
                {
                    'sub_area_type': 5,
                    'sub_area_type_str': 'text',
                    'text': 'MORE TEXT THAT'},
                {
                    'sub_area_type': 5,
                    'sub_area_type_str': 'text',
                    'text': ' SPANS ACROSS@'},
                {
                    'sub_area_type': 5,
                    'sub_area_type_str': 'text',
                    'text': ' MULTIPLE LIN@'},
                {
                    'sub_area_type': 5,
                    'sub_area_type_str': 'text',
                    'text': 'ES.  THE TEXT '},
                {
                    'sub_area_type': 5,
                    'sub_area_type_str': 'text',
                    'text': 'IS SUPPOSED TO'},
                {
                    'sub_area_type': 5,
                    'sub_area_type_str': 'text',
                    'text': ' BE CONCATENAT'},
                {
                    'sub_area_type': 5,
                    'sub_area_type_str': 'text',
                    'text': 'ED TOGETHER.@@'}]},
        'matches': [
            {
                'talker': 'AI',
                'seq_id': 4,
                'vdm_type': 'VDM',
                'sen_tot': 3,
                'sen_num': 1,
                'body': ('81mg=5@0EP:4R40807P>0<D1>MNt00000f>FNVfnw7>6>FNU=?B'
                         '5PD5HDPD8'),
                'checksum': '26',
                'chan': 'A',
                'fill_bits': 0,
                # pylint: disable=line-too-long
                'vdm': '!AIVDM,3,1,4,A,81mg=5@0EP:4R40807P>0<D1>MNt00000f>FNVfnw7>6>FNU=?B5PD5HDPD8,0*26'},
            {
                'talker': 'AI',
                'seq_id': 4,
                'vdm_type': 'VDM',
                'sen_tot': 3,
                'sen_num': 2,
                'body': ('1Dd2J09jL08JArJH5P=E<D9@<5P<9>0`bMl42Q0d2Pc2T59CPCE@@'
                         '?C54PD?'),
                'checksum': '60',
                'chan': 'A',
                'fill_bits': 0,
                # pylint: disable=line-too-long
                'vdm': '!AIVDM,3,2,4,A,1Dd2J09jL08JArJH5P=E<D9@<5P<9>0`bMl42Q0d2Pc2T59CPCE@@?C54PD?,0*60',},
            {
                'talker': 'AI',
                'seq_id': 4,
                'vdm_type': 'VDM',
                'sen_tot': 3,
                'sen_num': 3,
                'body': 'd0@d0IqhH:Pah:U54PD?75D85Bf00',
                'checksum': '03',
                'chan': 'A',
                'fill_bits': 0,
                'vdm': '!AIVDM,3,3,4,A,d0@d0IqhH:Pah:U54PD?75D85Bf00,0*03'}]
    }
    message = self.queue.get()

    self.assertEqual(message, expected)
    self.assertEqual(self.queue.qsize(), 0)

  def testInterspersedMessages(self):
    # The grand test of mixing messages between each other.  The
    # queue should correctly use all lines here to decode 4 messages.
    lines = (
        # pylint: disable=line-too-long
        '!AIVDM,3,1,4,A,81mg=5@0EP:4R40807P>0<D1>MNt00000f>FNVfnw7>6>FNU=?B5PD5HDPD8,0*26',
        '!SAVDM,2,1,7,B,54QBqQ403dR4dP`j220`tPr1N098uLr2222222168pqB16Ne0<PPC52CClQH,0*6E',
        '!AIVDM,3,2,4,A,1Dd2J09jL08JArJH5P=E<D9@<5P<9>0`bMl42Q0d2Pc2T59CPCE@@?C54PD?,0*60',
        '!SAVDM,1,1,1,A,BEN:gg00bekP?aVR9C9UCwUUoP00,0*05',
        '!SAVDM,2,1,6,A,55NVS2000001L@??W3DU8tr0D4LhF22222222200000006hd07SClR1@A80j,0*57',
        '!SAVDM,2,2,6,A,E6H3Pp88880,2*10',
        '!AIVDM,3,3,4,A,d0@d0IqhH:Pah:U54PD?75D85Bf00,0*03',
        '!SAVDM,2,2,7,B,88888888880,2*3A')
    self.assertEqual(len(lines), 8)

    expected_queue_size = [0, 0, 0, 1, 1, 2, 3, 4]
    for count, line in enumerate(lines):
      self.queue.put(line)
      self.assertEqual(self.queue.qsize(), expected_queue_size[count])

    messages = []
    while not self.queue.empty():
      messages.append(self.queue.get())

    self.assertEqual(len(messages), 4)

    self.assertEqual(
        [message['line_nums'] for message in messages],
        [[4], [5, 6], [1, 3, 7], [2, 8]])

    # Spot check each message.
    self.assertEqual(messages[0]['decoded']['id'], 18)
    self.assertAlmostEqual(messages[0]['decoded']['x'], -122.767435)
    self.assertEqual(messages[0]['matches'][0]['seq_id'], 1)

    self.assertEqual(messages[1]['decoded']['id'], 5)
    self.assertEqual(messages[1]['decoded']['name'], 'IRON EAGLE          ')
    self.assertEqual(messages[1]['matches'][0]['seq_id'], 6)

    self.assertEqual(messages[2]['decoded']['id'], 8)
    self.assertEqual(
        messages[2]['decoded']['sub_areas'][8]['text'], 'ED TOGETHER.@@')
    self.assertEqual(messages[2]['matches'][0]['seq_id'], 4)

    self.assertEqual(messages[3]['decoded']['id'], 5)
    self.assertEqual(messages[3]['decoded']['callsign'], 'KHJL   ')
    self.assertEqual(messages[3]['matches'][0]['seq_id'], 7)

  def testUnhandledSingleLineVdmMessageType(self):
    # AIS 6:669:11 not handled.
    line = '!AIVDM,1,1,,B,6B?n;be:cbapalgc;i6?Ow4,2*4A'
    self.queue.put(line)
    self.assertEqual(self.queue.qsize(), 0)


if __name__ == '__main__':
  unittest.main()
