#!/usr/bin/env python

# Since 2010-May-06
# Yet another rewrite of my code to put ais data from a nais feed into postgis

# First just try to rip a whole day's log.  How long will it take?

# Skip normalization first...

import ais
import re

uscg_ais_nmea_regex_str = r'''^!(?P<talker>AI)(?P<stringType>VD[MO])
,(?P<total>\d?)
,(?P<senNum>\d?)
,(?P<seqId>[0-9]?)
,(?P<chan>[AB])
,(?P<body>[;:=@a-zA-Z0-9<>\?\'\`]*)
,(?P<fillBits>\d)\*(?P<checksum>[0-9A-F][0-9A-F])
(  
  (,S(?P<slot>\d*))
  | (,s(?P<s_rssi>\d*))
  | (,d(?P<signal_strength>[-0-9]*))
  | (,t(?P<t_recver_hhmmss>(?P<t_hour>\d\d)(?P<t_min>\d\d)(?P<t_sec>\d\d.\d*)))
  | (,T(?P<time_of_arrival>[^,]*))
  | (,x(?P<x_station_counter>[0-9]*))
  | (,(?P<station>(?P<station_type>[rbB])[a-zA-Z0-9_]*))
)*
,(?P<timeStamp>\d+([.]\d+)?)?
'''
uscg_ais_nmea_regex = re.compile(uscg_ais_nmea_regex_str,  re.VERBOSE)

match_count = 0
counters = {}
for i in range(30):
    counters[i] = 0

#for line_num, line in enumerate(file('/nobackup/dl1/20100505.norm')):
for line_num, line in enumerate(open('test-2M.norm')):
    if line_num % 100000 == 0: print ("line: %d   %d" % (line_num,match_count) )

    try:
        match = uscg_ais_nmea_regex.search(line).groupdict()
        match_count += 1
    except:
        continue
    try:
        msg = ais.decode(match['body'])
    except:
        pass
    counters[msg['id']] += 1

print ('match_count:',match_count)
print (counters)
