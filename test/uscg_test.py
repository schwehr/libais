"""Tests for ais.uscg."""

import unittest

from ais import uscg


METADATA_TYPES = r"""
# pylint: disable=line-too-long
# S
!SAVDM,1,1,1,A,13mHu<001Kr7KRN=oHCkLjj00<0P,0*29,d-98,S0,t000000.00,T00.004775,D07MN-KW-PRIBS1,1428796800
!SAVDM,1,1,7,B,18158j001ErSVGdE>>1Unlcp0l1n,0*44,d-105,S2235,t000059.00,T59.605270,D05MN-NC-MAMBS1,1428796799
# s
!AIVDM,1,1,,A,177jR48P00JrdqBH@CwBcOw`08N`,0*7F,s27064,d-106,t000113.82,T28.09677041,S1053,r003669945,1136498115
# d
!SAVDM,1,1,3,B,D03Owh0m9N>4gPfGLfpNfp0,2*1D,d-9,S1500,t000440.00,T40.004650,D08MN-NO-VENBS1,1428797080
!AIVDM,1,1,,B,8h3OdJQKT001GNQTp2V8QeQgR;Rt00,4*31,d-089,S2243,t235959.00,T59.81635087,r09SIGN1,1428796801
!SAVDM,1,1,8,A,15NC6HPP00JRh42E3a`LK?wn2<13,0*5F,d-108,S2236,t000059.00,T59.631854,D05MN-NC-MAMBS1,1428796799
# t
!SAVDM,1,1,9,A,15N4GOP000IS3iR@bPg:n`T00D3w,0*74,d-107,S2229,t235959.00,T59.445101,D08MN-NO-VENBS1,1428796799
!SAVDM,1,1,5,A,1E?GH@0022o4l9`JLk1SNRih0000,0*71,d-101,S2233,t000059.00,T59.552019,D13MN-CR-ELKBS1,1428796799
# T
!AIVDM,1,1,,B,4h3OdJQuu5ossIw1T`JFhg700p00,0*5F,d-088,S0000,t000000.00,T00.00303381,r09SIGN1,1428796801
!SAVDM,1,1,3,B,D03Owh0m9N>4gPfGLfpNfp0,2*1D,d-9,S1500,t000440.00,T40.004650,D08MN-NO-VENBS1,1428797080
# x
!AIVDM,1,1,,A,KnLBOtVM4EWJbNH,2*27,s22613,d-125,T48.04022115,x4928,r003669946,1218999946
$PSHI,001,47.04372373,0,02792,2I4$PSq:,0023$6`O,0$GPGGA,191948,4241.3391,N,07037.9474,W,$GP$,,x5675,r003669946,1219000825

$PRDCB,CONRPT,1428797525,D17MN-ANC-MMX1,72.215.150.163,3,0,*01
"""

SINGLE_LINE_MESSAGES = r"""
# pylint: disable=line-too-long
!SAVDM,1,1,7,B,18158j001ErSVGdE>>1Unlcp0l1n,0*44,d-105,S2235,t000059.00,T59.605270,D05MN-NC-MAMBS1,1428796799
!SAVDO,1,1,9,B,4h3Ovq1uu60INqoKkrEpsww020S:,0*42,b003669732,1428798330
!BSVDM,1,1,,A,23?v;T002:G2hUFKhOP:JpHD0D0@,0*2A,b003669974,1428969792
!ABVDM,1,1,,A,35NJ5OPP@9qPECnA:O?QDI`8015Q,0*0F,b003669955,1429041003
!BSVDO,1,1,,B,4h3OdJQuu5ossIw1T`JFhg700p00,0*44,b003665002,1428796801
!AIVDM,1,1,,B,74eGuSP0h=w0,0*50,rB0003160047,1428883425
!AIVDM,1,1,,B,:4eGcI1;EkKD,0*08,rB0003160047,1428890747
!AIVDM,1,1,,B,<27spkv6RCBfrql1sh,4*40,d-117,S1768,t004447.00,T47.16255138,r08NFTK1,1428885887
!AIVDM,1,1,,A,<so<bfHBtM:rkh,4*42,d-124,S0548,t042314.00,T14.62954173,r08XSEW1,1428898995
!AIVDM,1,1,,A,>b;7JGv9qqKuTw2G,0*5C,d-121,S0083,t230202.00,T02.2371735,r09SGHA1,1428899944
!SAVDO,1,1,,B,?03OwnRG`hh<D00,2*58,b003669978,1428885504
!BSVDM,1,1,,A,@h3OwphkwRq03h0000000000,0*50,b003669705,1428886986
!AIVDM,1,1,,B,B5MwIk00;VdW@M5grNOBKwtUkP06,0*5A,d-115,S2181,t233658.00,T58.160652,r003669930,1428883200
!AIVDM,1,1,,B,Dh3Owoib5N>5N4fGMMhNfp0,4*49,d-110,S0753,t001720.00,T20.079342,r003669930,1428885622
!BSVDM,1,1,,B,E>kb9Ma89h:9V@9Q@@@@@@@@@@@;W@2v=hvOP00003vP100,2*7D,b003669706,1428887581
!AIVDM,1,1,,A,F030owj2N2P6Ubib@=4q35b10000,0*58,rB0003160047,1428886673
!AIVDM,1,1,,A,Gh30ot3AW0jefS:9U2h00000;00,2*3B,rB0003160047,1428885144
!AIVDM,1,1,,B,H52Q5K@dDtpV222222222222220,2*45,d-084,S1704,t002745.00,T45.44369607,r11CSDO1,1428884866
!AIVDM,1,1,,B,IJwUuc?Ncj:B:5Qepm0,2*06,r17MAUM1,1428884214
!AIVDM,1,1,,B,Jq4luMRUIu2s@w0,2*01,r17MPYB1,1428886550
!SAVDM,1,1,,B,K8VSqb9LdU28WP>`,0*4D,b003669952,1428906229
"""

MULTI_LINE_MESSAGES = """
!SAVDM,2,1,3,A,55NPRF400001L@OC;K4pThE>1<PtDl4dE:22220O1@D3740HtEP000000000,0*0D,d-81,S2231,t040459.00,T59.498602,D08MN-MO-ROBBS1,1428796802
!SAVDM,2,2,3,A,00000000000,2*3D,d-81,S2231,t040459.00,T59.498602,D08MN-MO-ROBBS1,1428796802

!ANVDM,2,1,1,A,55NH7SP00001L@GOO7QHT=@u8T6222222222220O0000040Ht00000000000,0*06,r08ACERDC,1428798766
!ANVDM,2,2,1,A,00000000000,2*22,r08ACERDC,1428798766

!SAVDM,2,1,9,A,6h30ot1;EkJ0Ch82AP;H888852CH4p13kk88883<<TH0i<i=tk3=61KC<CO<,0*1B,d-92,S330,t004508.00,T08.804593,D09MN-BU-RIPBS1,1428799508
!SAVDM,2,2,9,A,lGAPW000000000,4*5C,d-92,S330,t004508.00,T08.804593,D09MN-BU-RIPBS1,1428799508

!SAVDM,2,1,9,A,6h30ot1;EkJ0Ch82AP;H888852CH4p13kk88883<<TH0i<i=tk3=61KC<CO<,0*1B,d-82,S330,t004508.00,T08.804427,D09MN-BU-EDEBS1,1428799508
!SAVDM,2,2,9,A,lGAPW000000000,4*5C,d-82,S330,t004508.00,T08.804427,D09MN-BU-EDEBS1,1428799508

!SAVDM,2,1,9,A,6h30ot1;EnRpCh82AP;H8888880mF@B3ihH8883EtDH21<i=tk3M61QC<CO<,0*46,d-92,S490,t004513.00,T13.071313,D09MN-BU-RIPBS1,1428799513
!SAVDM,2,2,9,A,n5AP`@00000000,4*6B,d-92,S490,t004513.00,T13.071313,D09MN-BU-RIPBS1,1428799513

!SAVDM,2,1,9,A,6h3Owji;EkJTFp82APc@C1kk@H3PERAhE3p8883C3lH49<i=tU8u6=5C<CO9,0*56,d-60,S340,t024509.00,T09.071138,D09MN-BU-SUNBS1,1428806709
!SAVDM,2,2,9,A,;5ASr@00000000,4*2F,d-60,S340,t024509.00,T09.071138,D09MN-BU-SUNBS1,1428806709


!SAVDM,2,1,6,A,85Mwp@QKfBcUKjRd>qgPw0SB5W<ujTiIusgwH=DSakg3Mhd0db4uL9Ha;DVu,0*16,d-78,S597,t000215.00,T15.924474,D07MN-JA-SAIBS1,1428796936
!SAVDM,2,2,6,A,:OSHBmF>@qos@VW1wsoBT0mIi0FBQNEpB`T,2*04,d-78,S597,t000215.00,T15.924474,D07MN-JA-SAIBS1,1428796936

!SAVDM,2,1,7,A,8@3QiWAKpAH042d2L6Nbd:U111101P0@8EkUS<6QhrQ?a20F0119PTW:@42V,0*29,d-88,S726,t000219.00,T19.364725,D13MN-CR-MEGBS1,1428796939
!SAVDM,2,2,7,A,2W9h@0H0445LoQA1`vPHCr@P,0*2F,d-88,S726,t000219.00,T19.364725,D13MN-CR-MEGBS1,1428796939

!SAVDM,2,1,4,A,8@3QiWAKpAH04:2V`NTB3JJHHg101P0@`EjFuP6W2JQ?a20F0139UPVWUPcP,0*3D,d-88,S743,t000219.00,T19.818064,D13MN-CR-MEGBS1,1428796940
!SAVDM,2,2,4,A,h@@@@0H04<5LcQd1aqI8Cr@P,0*7E,d-88,S743,t000219.00,T19.818064,D13MN-CR-MEGBS1,1428796940

!SAVDM,2,1,8,A,85PH6tAKfEoc0@DehR9FpGse2Rre`9?97tqBtBUh`lCwTnv:SEuK4FWw0<K:,0*5A,d-94,S774,t000220.00,T20.644616,D05MN-HR-MERBS1,1428796940
!SAVDM,2,2,8,A,3ovc?s7BFVm?Fjsadw9aLkH1Vfp``@nP9Q@,2*09,d-94,S774,t000220.00,T20.644616,D05MN-HR-MERBS1,1428796940

!SAVDM,2,1,1,B,85PH6tAKfEoc0@DehR9FpGse2Rre`9?97tqBtBUh`lCwTnv:SEuK4FWw0<K:,0*50,d-100,S823,t000221.00,T21.951383,D05MN-HR-FARBS1,1428796942
!SAVDM,2,2,1,B,3ovc?s7BFVm?Fjsadw9aLkH1Vfp``@nP9Q@,2*03,d-100,S823,t000221.00,T21.951383,D05MN-HR-FARBS1,1428796942

!AIVDM,2,1,5,A,85Np7J1Kf4j5lTUDTFNrtFc:0fM;quuUDBU6SV?dTI2,0*55,r17MKET3,1428796942
!AIVDM,2,2,5,A,0g=V9WToh,0*12,r17MKET3,1428796942

!AIVDM,2,1,4,A,85Np7J1Kf4j5lTUDTFNrtFc:0fM;quuUDBU6SV?dTI2,0*54,r17MKET1,1428796942
!AIVDM,2,2,4,A,0g=V9WToh,0*13,r17MKET1,1428796942

!SAVDM,2,1,1,B,8P3QiWAKpAH04>6H2`VNQ0VPBa101P0@pEitV06W7d1?a20F01440VVWW2FV,0*54,d-44,S846,t000222.00,T22.564474,D13MN-CR-MEGBS1,1428796942
!SAVDM,2,2,1,B,V6;h@0H04@5LRJ61ah:PCr@P,0*65,d-44,S846,t000222.00,T22.564474,D13MN-CR-MEGBS1,1428796942

!SAVDM,2,1,0,B,8P3QiWAKpAH04>6H2`VNQ0VPBa101P0@pEitV06W7d1?a20F01440VVWW2FV,0*55,d-91,S846,t000222.00,T22.564932,D13MN-CR-ELKBS1,1428796942
!SAVDM,2,2,0,B,V6;h@0H04@5LRJ61ah:PCr@P,0*64,d-91,S846,t000222.00,T22.564932,D13MN-CR-ELKBS1,1428796942

!SAVDM,2,1,9,B,8h3QiWAKpAH04>6H2`VNQ0VPBa101P0@pEitV06W7d1?a20F01440VVWW2FV,0*64,d-59,S868,t000223.00,T23.151189,D13MN-CR-KELBS1,1428796943
!SAVDM,2,2,9,B,V6;h@0H04@5LRJ61ah:PCr@P,0*6D,d-59,S868,t000223.00,T23.151189,D13MN-CR-KELBS1,1428796943

!SAVDM,2,1,9,B,8h3QiWAKpAH04>6H2`VNQ0VPBa101P0@pEitV06W7d1?a20F01440VVWW2FV,0*64,d-56,S868,t000223.00,T23.151273,D13MN-CR-ELKBS1,1428796943
!SAVDM,2,2,9,B,V6;h@0H04@5LRJ61ah:PCr@P,0*6D,d-56,S868,t000223.00,T23.151273,D13MN-CR-ELKBS1,1428796943

!SAVDM,2,1,1,A,85NQB:1Kf@BDjURLGvFQeWBwfiONL:oN4UP4ns8CwVO`lGOhJJwi:Oo?p1qt,0*79,d-48,S899,t000223.00,T23.977783,D13MN-PS-SEABS1,1428796944
!SAVDM,2,2,1,A,4Tmq43sTQCrRcW8gPMt6mea192P7RKObLmh,2*1C,d-48,S899,t000223.00,T23.977783,D13MN-PS-SEABS1,1428796944

!AIVDM,2,1,3,A,85MwpViKf3dH4vHepwg9M>6:6:>cfRuSE3tHD93grIp,0*7E,d-81,S0857,t233922.00,T22.852219,r003669930,1428796944
!AIVDM,2,2,3,A,3@D:5LNoU,0*15,d-81,S0857,t233922.00,T22.852219,r003669930,1428796944

!AIVDM,2,1,0,A,>MIv3elkWG;9M?vTgOao95OQgAva9qSSdfT2IkhFV5C,0*4F,d-124,S0665,t024017.00,T17.75172871,r09SIGN1,1428806419
!AIVDM,2,2,0,A,T,0*42,d-124,S0665,t024017.00,T17.75172871,r09SIGN1,1428806419

!AIVDM,2,1,7,B,>v4kNGwiaajsirqtCNngTaN4K7oAowu;Lk;c4Nl?bnH,0*4B,d-127,S1504,t045140.00,T40.111042,r003669930,1428815682
!AIVDM,2,2,7,B,M=BT1RnAq:;Ab4bMF@,2*74,d-127,S1504,t045140.00,T40.111042,r003669930,1428815682

!AIVDM,2,1,1,A,AMkeWOfOPl6q6tOsqjls;eg<wELn7Pn?uuw8GrA;2W`,0*00,r17MCHI1,1428813162
!AIVDM,2,2,1,A,G,0*50,r17MCHI1,1428813162

!AIVDM,2,1,1,A,ABo8USwc:g7>plwwrOmCb@OruNu>rHsocw6vB?`SwON,0*2C,r17MKET3,1428822381
!AIVDM,2,2,1,A,2,0*25,r17MKET3,1428822381

!AIVDM,2,1,0,B,C5NMbDQdD>KeM8VCcLdo8dv44T28V@2g0J6F::00000,0*6D,d-075,S1553,t023441.00,T41.42342273,r09STWO1,1428806089
!AIVDM,2,2,0,B,0J70<RRS0,0*37,d-075,S1553,t023441.00,T41.42342273,r09STWO1,1428806089

!AIVDM,2,1,7,A,C8gvgIhKqO5BvtMF:Fr1o?wswPeF8C;vM?usbbo6DtC,0*32,r17MWRA1,1428835629
!AIVDM,2,2,7,A,;BmWoasCLR7MbMKaL=coseoL,2*47,r17MWRA1,1428835629


!AIVDM,2,1,8,A,DmpwSacL?uRSO?t:IiDcO3U95ts3lOk?HHv?IwojkCU,0*35,d-130,S0921,t001424.00,T24.557781,r003669930,1428799045
!AIVDM,2,2,8,A,P,3*4D,d-130,S0921,t001424.00,T24.557781,r003669930,1428799045

!AIVDM,2,1,7,B,DvVqNKP1Mondj<lJAB2wdOaOtwuTccugOs?lO2:4MlV,0*20,d-129,S0401,t000110.00,T10.68684,r003669930,1428799152
!AIVDM,2,2,7,B,p,0*62,d-129,S0401,t000110.00,T10.68684,r003669930,1428799152

!AIVDM,2,1,5,A,DNSD;INlw7RBa2Mwb:FIJUT8vT<NqQIwroBnwTb3HvN,0*3D,d-127,S1215,t112232.00,T32.397243,r003669930,1428839132
!AIVDM,2,2,5,A,KlE:bTq3hS8P6j3roN;9A@GJ38t>A=4LWjgp,4*35,d-127,S1215,t112232.00,T32.397243,r003669930,1428839132

!AIVDM,2,1,9,B,ENk`sO70VQ97aRh1T@611@Hr@@@=FVj<;V5d@00003v,0*1D,d-101,S1635,r003669959,1428800479
!AIVDM,2,2,9,B,P000,2*7E,d-101,S1635,r003669959,1428800479

!AIVDM,2,1,5,A,E=Mk`t@:aQSh:2ab@1:WdhHHHP0=Jw6U;m3r0108880,0*55,d-91,S0101,t040602.00,T02.691849,r003669930,1428812944
!AIVDM,2,2,5,A,>20,2*2D,d-91,S0101,t040602.00,T02.691849,r003669930,1428812944


!AIVDM,2,1,2,B,Jlttij=1Ct=wG4AwOtgsotWssq6wi=D4:bnwwiqq>Ns,0*2E,r17MKET3,1428805696
!AIVDM,2,2,2,B,?OdL,2*4D,r17MKET3,1428805696
--
!AIVDM,2,1,9,A,Jr:wA3lmgDOFo7fTugFoBVb?tLJUepabWvKuuNcKCrS,0*77,r17MGAM1,1428807541
!AIVDM,2,2,9,A,npfqv;WWd7vewOw0,2*66,r17MGAM1,1428807541
--
!AIVDM,2,1,9,B,JOg<PB;Crvcg>sS9waBwfOlALLUv7ch7feOrq?6IfOL,0*69,d-108,S1447,t032538.00,T38.597771,r003669945,1428809138
!AIVDM,2,2,9,B,79I:UEltj,0*03,d-108,S1447,t032538.00,T38.597771,r003669945,1428809138
--
!AIVDM,2,1,1,B,JHVBNvfuurLe1GwwdJ77uOK6;erAo8g<9Q9t7PvmOoQ,0*52,d-125,S0729,t103019.00,T19.46385228,r09SMIL1,1428834621
!AIVDM,2,2,1,B,9TfsIG7?rftLr4S<HG<C;Cfgw3s;=wWagj:rn,0*3D,d-125,S0729,t103019.00,T19.46385228,r09SMIL1,1428834621
--
!AIVDM,2,1,5,A,JOU7TI4SNbClHt7ukEdIoQe9ESgEm?AfnBFB:<7pvVu,0*6C,r17MPYB1,1428846735
!AIVDM,2,2,5,A,fu;Rph0,4*45,r17MPYB1,1428846735

!ANVDM,3,1,3,A,8h30otA?0@55oUPPP121IoCol54cd1;ws;wwhhvmTP15oVD9e2B94oCPH54M,0*64,r08ACERDC,1428798685
!ANVDM,3,2,3,A,`45wshwwhdw1dP15oaPPP121IoCol54cd2UwrUwwhhvmTP15obD9e2B94oCP,0*50,r08ACERDC,1428798685
!ANVDM,3,3,3,A,H54M`4MwskwwhdvudP0,2*69,r08ACERDC,1428798685

!SAVDM,3,1,8,A,8h3OwjQKP@55o5D9e2B94oCPH54M`45wsiwwhkvqdP1608PPP121IoCol54c,0*2F,d-110,S1399,t003137.00,T37.311616,D09MN-BU-SUNBS1,1428798697
!SAVDM,3,2,8,A,d0MwskwwheviTP1608D9e2B94oCPH54M`4=wsdwwhbw5dP160:PPP121IoCo,0*67,d-110,S1399,t003137.00,T37.311616,D09MN-BU-SUNBS1,1428798697
!SAVDM,3,3,8,A,l54cd2ewrwwwhdviTP0,2*6E,d-110,S1399,t003137.00,T37.311616,D09MN-BU-SUNBS1,1428798697
!AIVDM,3,1,5,B,85MwpViKfGM3;LgDUfeqCt0uO6r>hhUP1UCmw9nNUAN,0*31,d-101,S0247,r003669959,1428797682
!AIVDM,3,2,5,B,:=TAfR65lA7oHU`sIuKPPFmsJNN2Vdrf3rLSCuFrqe`,0*16,d-101,S0247,r003669959,1428797682
!AIVDM,3,3,5,B,b`1QwIReh,2*11,d-101,S0247,r003669959,1428797682
!SAVDM,3,1,5,A,8h3OwjQKP@=60>PPPP<j>nsphTtHBR:1@0160>PPPP<iCnsm<4tPG29g@016,0*44,d-109,S1119,t001529.00,T29.844856,D09MN-BU-SUNBS1,1428797730
!SAVDM,3,2,5,A,0>PPPP<jCnssG4tCk2OT@0160>PPPP<i>nskl4tSp1lE@0160>PPP<pC5nsT,0*52,d-109,S1119,t001529.00,T29.844856,D09MN-BU-SUNBS1,1428797730
!SAVDM,3,3,5,A,`4r9d4@A@0160>PPPP<k>nsuPTt6m2Oa@00,2*4A,d-109,S1119,t001529.00,T29.844856,D09MN-BU-SUNBS1,1428797730

"""


class UscgRegexTest(unittest.TestCase):

  def testComplete(self):
    # pylint: disable=line-too-long
    line = '!AIVDM,1,1,,B,15N1R5PP00rlhF0GQ8M00?wL28KP,0*4F,d-117,S1760,t205246.00,T46.932137,r003669930,1429046147'
    expected = {
        'counter': None,
        'hour': '20',
        'minute': '52',
        'payload': '!AIVDM,1,1,,B,15N1R5PP00rlhF0GQ8M00?wL28KP,0*4F',
        'receiver_time': '205246.00',
        'rssi': None,
        'second': '46.00',
        'signal_strength': '-117',
        'slot': '1760',
        'station': 'r003669930',
        'station_type': 'r',
        'time': '1429046147',
        'time_of_arrival': '46.932137',
        'uscg_metadata': ',d-117,S1760,t205246.00,T46.932137,r003669930,1429046147'}
    self.assertEqual(uscg.USCG_RE.match(line).groupdict(), expected)

  def testSlot(self):
    # pylint: disable=line-too-long
    line = '!SAVDM,1,1,7,B,18158j001ErSVGdE>>1Unlcp0l1n,0*44,d-105,S2235,t000059.00,T59.605270,D05MN-NC-MAMBS1,1428796799'
    self.assertEqual(uscg.USCG_RE.match(line).groupdict()['slot'], '2235')

  def testLowerRssi(self):
    # pylint: disable=line-too-long
    line = '!AIVDM,1,1,,A,177jR48P00JrdqBH@CwBcOw`08N`,0*7F,s27064,d-106,t000113.82,T28.09677041,S1053,r003669945,1136498115'
    self.assertEqual(uscg.USCG_RE.match(line).groupdict()['rssi'], '27064')

  def testLowerSignalStrength(self):
    lines = (
        # pylint: disable=line-too-long
        '!SAVDM,1,1,3,B,D03Owh0m9N>4gPfGLfpNfp0,2*1D,d-9,S1500,t000440.00,T40.004650,D08MN-NO-VENBS1,1428797080',
        '!AIVDM,1,1,,B,8h3OdJQKT001GNQTp2V8QeQgR;Rt00,4*31,d-089,S2243,t235959.00,T59.81635087,r09SIGN1,1428796801',
        '!SAVDM,1,1,8,A,15NC6HPP00JRh42E3a`LK?wn2<13,0*5F,d-108,S2236,t000059.00,T59.631854,D05MN-NC-MAMBS1,1428796799',
        )
    self.assertEqual(
        uscg.USCG_RE.match(lines[0]).groupdict()['signal_strength'], '-9')
    self.assertEqual(
        uscg.USCG_RE.match(lines[1]).groupdict()['signal_strength'], '-089')
    self.assertEqual(
        uscg.USCG_RE.match(lines[2]).groupdict()['signal_strength'], '-108')

  def testReceiverTime(self):
    # pylint: disable=line-too-long
    line = '!SAVDM,1,1,9,A,15N4GOP000IS3iR@bPg:n`T00D3w,0*74,d-107,S2229,t235959.00,T59.445101,D08MN-NO-VENBS1,1428796799'
    result = uscg.USCG_RE.match(line).groupdict()
    self.assertEqual(result['receiver_time'], '235959.00')
    self.assertEqual(result['hour'], '23')
    self.assertEqual(result['minute'], '59')
    self.assertEqual(result['second'], '59.00')

  def testTimeOfArrival(self):
    # pylint: disable=line-too-long
    line = '!SAVDM,1,1,8,A,15NC6HPP00JRh42E3a`LK?wn2<13,0*5F,d-108,S2236,t000059.00,T59.631854,D05MN-NC-MAMBS1,1428796799'
    self.assertEqual(
        uscg.USCG_RE.match(line).groupdict()['time_of_arrival'], '59.631854')

  def testStationCounter(self):
    # pylint: disable=line-too-long
    line = '!AIVDM,1,1,,A,KnLBOtVM4EWJbNH,2*27,s22613,d-125,T48.04022115,x4928,r003669946,1218999946'
    self.assertEqual(uscg.USCG_RE.match(line).groupdict()['counter'], '4928')

  def testStation(self):
    # pylint: disable=line-too-long
    line = '$GPTXT,01,01,02,ANTSTATUS=OK*3B,rnrwais1,1241447967.14'
    self.assertEqual(
        uscg.USCG_RE.match(line).groupdict()['station'], 'rnrwais1')
    self.assertEqual(
        uscg.USCG_RE.match(line).groupdict()['station_type'], 'r')

    line = '!AIVDM,1,1,,A,btKp7;kfb@J?rAvs,0*6B,s20552,d-123,T55.10376258,x373,R11SMOR1,1236152694'
    self.assertEqual(
        uscg.USCG_RE.match(line).groupdict()['station'], 'R11SMOR1')

    line = '!BSVDO,1,1,,B,4h3OdJQuu5ossIw1T`JFhg700p00,0*44,b003665002,1428796801'
    self.assertEqual(
        uscg.USCG_RE.match(line).groupdict()['station'], 'b003665002')

    line = '!AIVDM,3,3,4,A,PP3D<oPPEU;M418g@02PSpPPP2hlEoRQgU;j@17p@00,2*63,B0003160047,1064981014'
    self.assertEqual(
        uscg.USCG_RE.match(line).groupdict()['station'], 'B0003160047')

    line = '!SAVDM,2,2,1,A,KKKKKKKKKK@,2*4F,d-84,S1688,t000245.00,T45.018254,D08MN-NO-BSABS1,1428969765'
    self.assertEqual(
        uscg.USCG_RE.match(line).groupdict()['station'], 'D08MN-NO-BSABS1')

  def testTime(self):
    line = '!AIVDM,1,1,,B,:3TsvT2G`mGL,0*5A,r00370003,1428890499'
    self.assertEqual(uscg.USCG_RE.match(line).groupdict()['time'], '1428890499')
    line = '$GPTXT,01,01,02,ANTSTATUS=OK*3B,rnrwais1,1241447967.14'
    self.assertEqual(
        uscg.USCG_RE.match(line).groupdict()['time'], '1241447967.14')


class ParseTest(unittest.TestCase):

  def testSingleLine(self):
    line = '!AIVDM,1,1,,B,=gLGu1tCm;vwvpwm,0*4C,r17MLAR1,1428885091'
    metadata = uscg.Parse(line)

    self.assertEqual(
        metadata['payload'], '!AIVDM,1,1,,B,=gLGu1tCm;vwvpwm,0*4C')
    self.assertEqual(metadata['station'], 'r17MLAR1')
    self.assertEqual(metadata['time'], 1428885091)


class UsgsQueueTest(unittest.TestCase):

  def setUp(self):
    self.queue = uscg.UscgQueue()

  def testPassThroughText(self):
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
      msg = self.queue.get()
      self.assertEqual(msg, expected, 'pass through fail. %d "%s"\n  %s != %s'
                       % (line_num, line, msg, expected))
      self.assertEqual(self.queue.qsize(), 0)

  def testSingleLine(self):
    # pylint: disable=line-too-long
    line = '!SAVDM,1,1,7,B,18158j001ErSVGdE>>1Unlcp0l1n,0*44,d-105,S2235,t000059.00,T59.605270,D05MN-NC-MAMBS1,1428796799'
    # self.assertEqual(ais.decode('18158j001ErSVGdE>>1Unlcp0l1n'), {})
    metadata = uscg.Parse(line)
    self.assertEqual(
        metadata,
        {
            'counter': None,
            'hour': 0,
            'minute': 0,
            'payload': '!SAVDM,1,1,7,B,18158j001ErSVGdE>>1Unlcp0l1n,0*44',
            'receiver_time': 59.00,
            'rssi': None,
            'second': 59.00,
            'signal_strength': -105,
            'slot': 2235,
            'station': 'D05MN-NC-MAMBS1',
            'station_type': 'D',
            'time': 1428796799,
            'time_of_arrival': 59.605270,
            'uscg_metadata':
            ',d-105,S2235,t000059.00,T59.605270,D05MN-NC-MAMBS1,1428796799'})

  def testSingleLineDecode(self):
    line = '!SAVDM,1,1,,B,K8VSqb9LdU28WP8<,0*17,rMySat,1218999946'

    self.queue.put(line)
    self.assertEqual(self.queue.qsize(), 1)
    msg = self.queue.get()
    self.assertEqual(
        msg,
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
            'line_nums': [None],
            'lines': ['!SAVDM,1,1,,B,K8VSqb9LdU28WP8<,0*17,rMySat,1218999946'],
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
                'time': 1218999946,
                'time_of_arrival': None,
                'uscg_metadata': ',rMySat,1218999946',
                'vdm': '!SAVDM,1,1,,B,K8VSqb9LdU28WP8<,0*17',
                'vdm_type': 'VDM'}]})

  def testManySingleLineMessages(self):
    count = 0
    for line in SINGLE_LINE_MESSAGES.split('\n'):
      if not line.startswith('!'):
        continue
      count += 1
      self.queue.put(line)
      self.assertEqual(self.queue.qsize(), count, 'line failed: %s' % line)

    msgs = []
    while not self.queue.empty():
      msg = self.queue.get()
      msgs.append(msg)
      self.assertIn('decoded', msg)
      self.assertIn('line_nums', msg)
      self.assertIn('lines', msg)
      self.assertIn('matches', msg)

    msg_ids = [msg['decoded']['id'] for msg in msgs]
    self.assertEqual(
        msg_ids,
        [1, 4, 2, 3, 4, 7, 10, 12, 12, 14, 15, 16, 18, 20, 21, 22, 23, 24, 25,
         26, 27])

  def testOneMultiLineMessage(self):
    lines = (
        # pylint: disable=line-too-long
        '!SAVDM,2,1,2,A,55@=M<02=rO7<Dm7B20EHE:1<4HEAV2222222217JQHQ:57c0Rk3lp0CQiC1,0*47,d-62,S2149,t135257.00,T57.311866,D11MN-LA-CATBS1,1429278777',
        '!SAVDM,2,2,2,A,Dp888888880,2*08,d-62,S2149,t135257.00,T57.311866,D11MN-LA-CATBS1,1429278777'
    )
    self.queue.put(lines[0])
    self.assertEqual(self.queue.qsize(), 0)
    self.queue.put(lines[1])
    self.assertEqual(self.queue.qsize(), 1)
    msg = self.queue.get()
    self.assertEqual(
        msg,
        {
            # pylint: disable=line-too-long
            'decoded': {
                'ais_version': 0,
                'callsign': '3EMQ4  ',
                'destination': 'LOS ANGELES         ',
                'dim_a': 212,
                'dim_b': 88,
                'dim_c': 33,
                'dim_d': 10,
                'draught': 13.899999618530273,
                'dte': 0,
                'eta_day': 15,
                'eta_hour': 11,
                'eta_minute': 0,
                'eta_month': 4,
                'fix_type': 1,
                'id': 5,
                'imo_num': 9300465,
                'md5': '3ce09e77864abc4b9766573e1de13c3c',
                'mmsi': 352542000,
                'name': 'EVER SAFETY         ',
                'repeat_indicator': 0,
                'spare': 0,
                'type_and_cargo': 71},
            'line_nums': [1, 2],
            'lines': [
                '!SAVDM,2,1,2,A,55@=M<02=rO7<Dm7B20EHE:1<4HEAV2222222217JQHQ:57c0Rk3lp0CQiC1,0*47,d-62,S2149,t135257.00,T57.311866,D11MN-LA-CATBS1,1429278777',
                '!SAVDM,2,2,2,A,Dp888888880,2*08,d-62,S2149,t135257.00,T57.311866,D11MN-LA-CATBS1,1429278777'],
            'matches': [
                {
                    'body': '55@=M<02=rO7<Dm7B20EHE:1<4HEAV2222222217JQHQ:57c0Rk3lp0CQiC1',
                    'chan': 'A',
                    'checksum': '47',
                    'counter': None,
                    'fill_bits': 0,
                    'hour': 13,
                    'minute': 52,
                    'payload': '!SAVDM,2,1,2,A,55@=M<02=rO7<Dm7B20EHE:1<4HEAV2222222217JQHQ:57c0Rk3lp0CQiC1,0*47',
                    'receiver_time': 135257.00,
                    'rssi': None,
                    'second': 57.0,
                    'sen_num': 1,
                    'sen_tot': 2,
                    'seq_id': 2,
                    'signal_strength': -62,
                    'slot': 2149,
                    'station': 'D11MN-LA-CATBS1',
                    'station_type': 'D',
                    'talker': 'SA',
                    'time': 1429278777,
                    'time_of_arrival': 57.311866,
                    'uscg_metadata': ',d-62,S2149,t135257.00,T57.311866,D11MN-LA-CATBS1,1429278777',
                    'vdm': '!SAVDM,2,1,2,A,55@=M<02=rO7<Dm7B20EHE:1<4HEAV2222222217JQHQ:57c0Rk3lp0CQiC1,0*47',
                    'vdm_type': 'VDM'},
                {
                    'body': 'Dp888888880',
                    'chan': 'A',
                    'checksum': '08',
                    'counter': None,
                    'fill_bits': 2,
                    'hour': 13,
                    'minute': 52,
                    'payload': '!SAVDM,2,2,2,A,Dp888888880,2*08',
                    'receiver_time': 135257.00,
                    'rssi': None,
                    'second': 57.00,
                    'sen_num': 2,
                    'sen_tot': 2,
                    'seq_id': 2,
                    'signal_strength': -62,
                    'slot': 2149,
                    'station': 'D11MN-LA-CATBS1',
                    'station_type': 'D',
                    'talker': 'SA',
                    'time': 1429278777,
                    'time_of_arrival': 57.311866,
                    'uscg_metadata': ',d-62,S2149,t135257.00,T57.311866,D11MN-LA-CATBS1,1429278777',
                    'vdm': '!SAVDM,2,2,2,A,Dp888888880,2*08',
                    'vdm_type': 'VDM'}],
            'times': [1429278777, 1429278777]})

if __name__ == '__main__':
  unittest.main()
