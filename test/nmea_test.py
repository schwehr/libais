#!/usr/bin/env python
"""Tests for ais.nmea."""

import unittest

from ais import nmea


class ChecksumTest(unittest.TestCase):

  def testInternalChecksum(self):
    # $NVXDR,G,142983,,WPTTTG*00
    self.assertEqual(nmea._Checksum('NVXDR,G,142983,,WPTTTG'), '00')
    # $GPHDT,143.59,T*0F
    self.assertEqual(nmea._Checksum('GPHDT,143.59,T'), '0F')
    # $INHDT,89.4,T*10
    self.assertEqual(nmea._Checksum('INHDT,89.4,T'), '10')
    # $INROT,0,A*3F
    self.assertEqual(nmea._Checksum('INROT,0,A'), '3F')
    # \n:121650,s:r17MHOP1,c:1425327399*1D\$ANZDA,201638.00,02,03,2015,00,00*77
    self.assertEqual(nmea._Checksum(r'n:121650,s:r17MHOP1,c:1425327399'), '1D')
    # \g:2-2-223509,n:453068*26\$ARVSI,r08XSTL1,,201704.05846325,0152,-057,0*0B
    self.assertEqual(nmea._Checksum(r'g:2-2-223509,n:453068'), '26')

  def testChecksum(self):
    self.assertEqual(nmea.Checksum('$NVXDR,G,142983,,WPTTTG*00'), '00')
    self.assertEqual(nmea.Checksum('$GPHDT,143.59,T*0F'), '0F')
    self.assertEqual(nmea.Checksum('$INHDT,89.4,T*10'), '10')
    self.assertEqual(nmea.Checksum('$INROT,0,A*3F'), '3F')
    self.assertEqual(
        nmea.Checksum('!AIVDM,1,1,,B,14Qle<001mDoOKD`v:`Rc2;R1d00,0*00'), '00')
    self.assertEqual(
        nmea.Checksum(r'\n:121650,s:r17MHOP1,c:1425327399*1D'), '1D')
    self.assertEqual(nmea.Checksum(r'\g:2-2-223509,n:453068*26'), '26')


class LineTypeTest(unittest.TestCase):

  def testNone(self):
    self.assertEqual(nmea.LineType(''), nmea.TEXT)
    self.assertEqual(nmea.LineType('a'), nmea.TEXT)
    self.assertEqual(nmea.LineType('$'), nmea.TEXT)
    self.assertEqual(nmea.LineType('!'), nmea.TEXT)
    self.assertEqual(nmea.LineType('\\'), nmea.TEXT)
    self.assertEqual(nmea.LineType('$AI*FF'), nmea.TEXT)
    self.assertEqual(nmea.LineType('\\foo:junk*AA'), nmea.TEXT)

  def testBare(self):
    self.assertEqual(nmea.LineType('$ABCD*04'), nmea.BARE)
    self.assertEqual(
        nmea.LineType('$PSWDA,24.46,5.61,86.44,3.86*54'), nmea.BARE)
    self.assertEqual(nmea.LineType('$INROT,-1,A*13'), nmea.BARE)
    self.assertEqual(
        nmea.LineType('!AIVDM,1,1,,A,14Qle<001fCNvfbR75JRuBGD00Rt,0*35'),
        nmea.BARE)
    self.assertEqual(
        nmea.LineType('$PKEL99,------,29042009,193138.939,00312,HF,00.00,0,'
                      '+008.50,LF,00.00,0,+008.50,1500,-0005 ,   0,59 32.87'
                      '9118N,175 05.007569W,0601*04'),
        nmea.BARE)
    self.assertEqual(
        nmea.LineType('$PRDCB,CONRPT,1425514093,SLS_AIS,example.com,242,0,*1C'),
        nmea.BARE)
    self.assertEqual(
        nmea.LineType('!AIVDM,1,1,,B,Dh3OwmPA5N>4,0*0F'), nmea.BARE)
    self.assertEqual(nmea.LineType('$UPSAQ,BCE*2E'), nmea.BARE)
    self.assertEqual(nmea.LineType('$AIALR,,,,,*7B'), nmea.BARE)

  def testUscg(self):
    self.assertEqual(
        nmea.LineType('$PRDCB,CONRPT,1425514093,SLS_AIS,example.com,242,0,*1C'
                      ',bEXAMPLE,1370787180'),
        nmea.USCG)
    self.assertEqual(
        nmea.LineType('!AIVDM,2,2,9,B,00000000000,2*2E,b003669952,1370785759'),
        nmea.USCG)
    self.assertEqual(
        nmea.LineType('$AIDLM,9,B,,,,,,,,,,,,,,,,,,,,,R*48,D05MN-BL-GBPBS1,'
                      '1370786280'),
        nmea.USCG)
    self.assertEqual(
        nmea.LineType('$ANABK,,B,8,0,3*12,r17MHOP1,1409443341'),
        nmea.USCG)
    self.assertEqual(
        nmea.LineType('$AIALR,,,,,*7B,D08MN-HG-GCHBS1,1396488078'),
        nmea.USCG)
    self.assertEqual(
        nmea.LineType('$SAADS,D14MN-HO-KAWBS1,112359.89,V,0,I,I*38,'
                      'D14MN-HO-KAWBS1,1409484239'),
        nmea.USCG)
    self.assertEqual(
        nmea.LineType('$ANZDA,000258.00,03,04,2014,00,00*71,r17MHOP1,'
                      '1396483379'),
        nmea.USCG)

  def testTag(self):
    self.assertEqual(
        nmea.LineType(r'\s:station1,c:1425344187*78'), nmea.TAGB)
    self.assertEqual(
        nmea.LineType(r'\s:station1,c:1425344187*78\a'), nmea.TAGB)
    self.assertEqual(
        nmea.LineType(r'\c:1425344187,s:station1*78\c'), nmea.TAGB)
    self.assertEqual(
        nmea.LineType(r'\n:121650,s:r17MHOP1,c:1425327399*1D\$'
                      'ANZDA,201638.00,02,03,2015,00,00*77'),
        nmea.TAGB)
    self.assertEqual(
        nmea.LineType(r'\s:Station,d:somewhere,n:2,q:u,r:123,t:A string.,'
                      r'c:1425168552,T:2015-03-01 00.09.12*3A\content'),
        nmea.TAGB)
    self.assertEqual(
        nmea.LineType(r'\g:1-3-42349,n:111458,s:r003669945,c:1425327424*41\!'
                      r'AIVDM,2,1,5,A,ENk`sPa17ab7W@7@1T@6;Q@0h@@=MeR4<7rpH'
                      '00003v,0*16'),
        nmea.TAGB)
    self.assertEqual(
        nmea.LineType(r'\n:5499,s:r17MHAW1,c:1425513601*11\!AIVDM,1,1,,A,34Q'
                      '=tf002;nF:NjQ>Th4bken2151,0*02'),
        nmea.TAGB)
    self.assertEqual(
        nmea.LineType(r'\n:428320,s:b003669976,c:1425513598*1A\!BSVDM,1,1,,B,'
                      '14eH@`h000G<J?`L=ge;u0wn00S`,0*25'),
        nmea.TAGB)
    self.assertEqual(
        nmea.LineType(r'\g:2-2-64856,n:154678*15\$ARVSI,r14RHAL1,,235959.'
                      '92296498,2247,-100,0*0D'),
        nmea.TAGB)
    self.assertEqual(
        nmea.LineType(r'\g:2-2-8288,s:rORBCOMM008,c:1421625653,T:2015-01-19 '
                      r'00.00.53*32\!AIVDM,2,2,7,A,00000000008,2*2B'),
        nmea.TAGB)
    self.assertEqual(
        nmea.LineType(r'\s:rORBCOMM010,q:u,c:1421625633,T:2015-01-19 '
                      r'00.00.33*52\!AIVDM,1,1,,B,13aFh>?01H0KMUpNWGWc'
                      '1HLv0>@<,0*68'), nmea.TAGB)


if __name__ == '__main__':
  unittest.main()
