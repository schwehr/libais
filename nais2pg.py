#!/usr/bin/env python

# Since 2010-May-06
# Yet another rewrite of my code to put ais data from a nais feed into postgis

# First just try to rip a whole day's log.  How long will it take?

# Skip normalization first...

import ais
import re

import psycopg2
#import psycopg2.extras

# Making this regex more like how I like!  lower_lower
uscg_ais_nmea_regex_str = r'''^!(?P<talker>AI)(?P<string_type>VD[MO])
,(?P<total>\d?)
,(?P<sen_num>\d?)
,(?P<seq_id>[0-9]?)
,(?P<chan>[AB])
,(?P<body>[;:=@a-zA-Z0-9<>\?\'\`]*)
,(?P<fill_bits>\d)\*(?P<checksum>[0-9A-F][0-9A-F])
(  
  (,S(?P<slot>\d*))
  | (,s(?P<s_rssi>\d*))
  | (,d(?P<signal_strength>[-0-9]*))
  | (,t(?P<t_recver_hhmmss>(?P<t_hour>\d\d)(?P<t_min>\d\d)(?P<t_sec>\d\d.\d*)))
  | (,T(?P<time_of_arrival>[^,]*))
  | (,x(?P<x_station_counter>[0-9]*))
  | (,(?P<station>(?P<station_type>[rbB])[a-zA-Z0-9_]*))
)*
,(?P<time_stamp>\d+([.]\d+)?)?
'''
uscg_ais_nmea_regex = re.compile(uscg_ais_nmea_regex_str,  re.VERBOSE)


import Queue

class LineQueue(Queue.Queue):
    'Break input into input lines'
    def __init__(self, separator='\n', maxsize=0, verbose=False):
        Queue.Queue.__init__(self,maxsize)
        self.input_buf = ''
        self.v = verbose
        self.separator = separator

    def put(self, text):
        self.input_buf += text
        groups = self.input_buf.split(self.separator)
        if len(groups) == 1:
            self.input_buf = groups[0]
            # no complete groups
            return
        elif len(groups[-1]) == 0:
            # last group was properly terminated
            for g in groups[:-1]:
                Queue.Queue.put(self,g)
            self.input_buf = ''
        else:
            # last part is a fragment
            for g in groups[:-1]:
                Queue.Queue.put(self,g)
            self.input_buf = groups[-1]

class NormQueue(Queue.Queue):
    '''Normalized AIS messages that are multiple lines

    - FIX: Can I get away with only keeping the body of beginning parts?

    - works based USCG dict representation of a line that comes back from the regex.
    - not worrying about the checksum.  Assume it already has been validated
    - 160 stations in the US with 10 seq channels... should not be too much data
    - assumes each station will send in order messages without duplicates
    '''
    def __init__(self, separator='\n', maxsize=0, verbose=False):
        Queue.Queue.__init__(self,maxsize)
        self.input_buf = ''
        self.v = verbose
        self.separator = separator
        self.stations = {}

    def put(self, msg):
        if not isinstance(msg, dict): raise TypeError('Message must be a dictionary')

        total = int(msg['total'])
        station = msg['station']
        if station not in self.stations:
            self.stations[station] = {0:[ ],1:[ ],2:[ ],3:[ ],4:[ ],
                                      5:[ ],6:[ ],7:[ ],8:[ ],9:[ ]}

        if total == 1:
            Queue.Queue.put(self,msg) # EASY case
            return

        seq = int(msg['seq_id'])
        sen_num = int(msg['sen_num'])

        if sen_num == 1:
            # Flush that station's seq and start it with a new msg component
            self.stations[station][seq] = [msg['body'],] # START
            return

        if sen_num != len(self.stations[station][seq]) + 1:
            self.stations[station][seq] = [ ] # DROP and flush... bad seq
            return
        
        if sen_num == total:
            msgs = self.stations[station][seq]
            self.stations[station][seq] = [ ] # FLUSH
            if len(msgs) != total - 1:
                return # INCOMPLETE was missing part - so just drop it

            # all parts should have the same metadata, but last has the fill bits
            msg['body'] = ''.join(msgs) + msg['body']
            msg['total'] = msg['seq_num'] = 1
            Queue.Queue.put(self,msg)
            return
        
        self.stations[station][seq].append(msg['body']) # not first, not last


class VesselNames:

    def __init__(self, db_cx):
        self.vessels={} # Keyed by MMSI
        self.cx = db_cx
        self.cu = db_cx.cursor()
        try:
            self.cu.execute(self.get_sql_create_str())
        except psycopg2.ProgrammingError, e:
            # already exists - this is okay
            pass
        self.cx.commit()
        
    def get_sql_create_str(self):
        'return SQL to create the name database'
        # Need mmsi, name, shipandcargo   # but not , cg_timestamp
        return '''CREATE TABLE vessel_name (
       mmsi INT PRIMARY KEY,
       name VARCHAR(25), -- only need 20
       type_and_cargo INT
);'''

    def insert_or_update(self, mmsi, name, type_and_cargo):
        'Try update first for speed'
        if False: # Does not want to work?        
            self.cu.execute('''BEGIN;
SAVEPOINT sp1;
  UPDATE vessel_name SET name=%(name)s, type_and_cargo=%(type_and_cargo)s WHERE mmsi = %(mmsi)s;
ROLLBACK TO sp1;
  INSERT INTO vessel_name VALUES(%(mmsi)s, %(name)s, %(type_and_cargo)s );
COMMIT;  
''',
                        {'mmsi':mmsi, 'name':name, 'type_and_cargo':type_and_cargo}
                        )
        # Lame, but try it.


        try:
            self.cu.execute('UPDATE vessel_name SET name=%s, type_and_cargo=%s WHERE mmsi = %s;',
                            (mmsi, name, type_and_cargo)
                            )
            self.cx.commit()
            print 'UPDATED vessel name'
            return
        
        except Exception, e:
            print
            print 'Exception for',name
            print Exception
            print e
            print type(mmsi), type(name), type(type_and_cargo)
            print
            pass

        #try
        print 'Trying_insert:',mmsi, type(mmsi), name, type(name), type_and_cargo, type(type_and_cargo)
        
        self.cu.execute('INSERT INTO vessel_name (mmsi,name,type_and_cargo) VALUES(%s, %s, %s);', (mmsi, str(name.rstrip() ), int(type_and_cargo)) )
        self.cx.commit()
        print 'HERE...'
        #print 'INSERTED vessel name'
            
        
    def update(self, mmsi, name, type_and_cargo):
        '''dict must have mmsi, name, type_and_cargo
        returns False if vessel already setup okay
        return True if vessel must be put in the db
        '''

        if mmsi in (0,1):
            # Drop these... useless
            print 'USELESS'
            return

        if mmsi not in self.vessels:
            print 'NEW',name
            self.vessels[mmsi] = (name, type_and_cargo)
            self.insert_or_update(mmsi,name,type_and_cargo)
            return

        old_name, old_type_and_cargo = self.vessels[mmsi]

        if name != old_name or type_and_cargo != old_type_and_cargo:
            self.vessels[mmsi] = (name, type_and_cargo)
            self.insert_or_update(mmsi,name,type_and_cargo)
            return

        # Nothing to do.
        
        
if __name__ == '__main__':


    match_count = 0
    counters = {}
    for i in range(30):
        counters[i] = 0

    cx = psycopg2.connect("dbname='ais_test'")
    
    vessel_names = VesselNames(cx)

    line_queue = LineQueue()
    norm_queue = NormQueue()


    #for line_num, line in enumerate(file('/nobackup/dl1/20100505.norm')):
    for line_num, text in enumerate(open('1e6.log')):
        #if line_num > 300: break
        #print 
        if line_num % 100000 == 0: print ("line: %d   %d" % (line_num,match_count) )

        line_queue.put(text)

        while line_queue.qsize() > 0:
            line = line_queue.get(False)
            #print 'line:',line
            try:
                match = uscg_ais_nmea_regex.search(line).groupdict()
                match_count += 1
            except AttributeError:
                if 'AIVDM' in line:
                    print 'BAD_MATCH:',line
                continue

            norm_queue.put(match)

            while norm_queue.qsize()>0:
       
                try:
                    result = norm_queue.get(False)
                except Queue.Empty:
                    continue

                try:
                     msg = ais.decode(result['body'])
                #except ais.decode.error:
                except Exception, e:
                    #print
                    #print 'E:',Exception
                    #print 'e:',e
#                    if 'not yet handled' not in str(e):
#                        print 'BAD Decode:',result
                    continue
                    #pass # Bad or unhandled message


                counters[msg['id']] += 1
                if msg['id'] in (5,19):
                    #print 'UPDATING vessel name', msg
                    vessel_names.update(msg['mmsi'], msg['name'].rstrip('@'), msg['type_and_cargo'])
                        
    print ('match_count:',match_count)
    print (counters)
