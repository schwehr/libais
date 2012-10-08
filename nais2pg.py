#!/usr/bin/env python
from __future__ import print_function

# Since 2010-May-06
# Yet another rewrite of my code to put ais data from a nais feed into postgis

__author__ = 'Kurt Schwehr'
#__version__ = '$Revision$'.split()[1]
#__revision__  = __version__ # For pylint
#__date__ = '$Date$'.split()[1]
__copyright__ = '2010'
__license__   = 'LGPL v3'
__contact__   = 'kurt at ccom.unh.edu'

import ais
from ais import decode # Make sure we have the correct ais module
import ais_lut # Look up tables for navigation status and such

import os
import sys
import re

from pyproj import Proj
import datetime, time
import math
from collections import deque

import psycopg2 # Bummer... not yet ready for python 3.1
#import psycopg2.extras

import sys, os
import time
import socket
import threading
import datetime
import exceptions # For KeyboardInterupt pychecker complaint
import traceback
import Queue
import select

ENABLE_DB = True  # Set to false to run without db commits

def print24(msg):
    if msg['part_num'] == 0:
        #print (msg)
        print ('24_A: name = ',msg['name'].rstrip(' @'),'\tmmsi = ',msg['mmsi'])
        return
    if msg['part_num'] == 1:
        print ('24_B: tac  = %3d' % msg['type_and_cargo'],'callsign=',msg['callsign'].rstrip(' @'))

def check_ref_counts(a_dict):
    print ('Checking ref counts on', a_dict)
    print ('\tmsg:', sys.getrefcount(a_dict))
    for d in a_dict:
        print ('\t',d, sys.getrefcount(d))

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
        self.input_buf = ''
        self.v = verbose
        self.separator = separator
        self.lq_maxsize = maxsize
        Queue.Queue.__init__(self)

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
                if self.qsize() > self.lq_maxsize:
                    continue # FIX: Would be better to add the most recent and drop one from the queue.
                Queue.Queue.put(self,g)
            self.input_buf = ''
        else:
            # last part is a fragment
            for g in groups[:-1]:
                if self.qsize() > self.lq_maxsize:
                    continue # FIX: Would be better to add the most recent and drop one from the queue.
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
        self.input_buf = ''
        self.v = verbose
        self.separator = separator
        self.stations = {}

        # For tracking rate of resulting normalized messages
        self.count = 0
        self.last_count = 0
        self.last_time = time.time()

        Queue.Queue.__init__(self,maxsize)

    def get_rate(self):
        now = time.time()
        rate = (self.count - self.last_count) / (now - self.last_time)
        self.last_time = now
        self.last_count = self.count
        return rate

    def put(self, msg):
        if not isinstance(msg, dict): raise TypeError('Message must be a dictionary')

        total = int(msg['total'])
        station = msg['station']
        if station not in self.stations:
            self.stations[station] = {0:[ ],1:[ ],2:[ ],3:[ ],4:[ ],
                                      5:[ ],6:[ ],7:[ ],8:[ ],9:[ ]}

        if total == 1:
            Queue.Queue.put(self,msg) # EASY case
            self.count += 1
            return

        seq = int(msg['seq_id'])
        sen_num = int(msg['sen_num'])

        if sen_num == 1:
            # FIX: would be a good place to check the message first letter to see if we can ditch it

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
            self.count += 1
            return

        self.stations[station][seq].append(msg['body']) # not first, not last



def insert_or_update(cx, cu, sql_insert, sql_update, values):
    'If the value might be new, try to insert first'
    #print 'insert_or_update:',values
    #print 'insert_or_update:',values['mmsi'],values['name'],values['type_and_cargo']
    success = True
    try:
        if ENABLE_DB: cu.execute(sql_insert,values)
    except psycopg2.IntegrityError:
        # psycopg2.IntegrityError: duplicate key value violates unique constraint
        #print ('insert failed')
        success = False

    cx.commit()
    if success: return

    #print (cu.mogrify(sql_update, values))
    if ENABLE_DB: cu.execute(sql_update, values)
    cx.commit()



class VesselNames:
    'Local caching of vessel names for more speed and less DB load'
    sql_insert = 'INSERT INTO vessel_name VALUES (%(mmsi)s,%(name)s,%(type_and_cargo)s);'
    sql_insert_full = 'INSERT INTO vessel_name VALUES (%(mmsi)s,%(name)s,%(type_and_cargo)s, %(response_class)s);'
    sql_update_full = 'UPDATE vessel_name SET name = %(name)s, type_and_cargo=%(type_and_cargo)s, response_class=%(response_class)s WHERE mmsi = %(mmsi)s;'
    sql_update = 'UPDATE vessel_name SET name = %(name)s, type_and_cargo=%(type_and_cargo)s WHERE mmsi = %(mmsi)s;'

    sql_update_tac  = 'UPDATE vessel_name SET type_and_cargo=%(type_and_cargo)s WHERE mmsi = %(mmsi)s;'
    sql_update_name = 'UPDATE vessel_name SET name = %(name)s WHERE mmsi = %(mmsi)s;'

    def __init__(self, db_cx, dump_interval=12*60*60, verbose=False):
        self.cx = db_cx
        self.cu = self.cx.cursor()
        self.vessels = {}
        self.v = verbose
        self.dump_interval = dump_interval
        force = (dump_interval -  120) if dump_interval > 120 else 0

        self.last_dump_time = time.time() - force # Force a quick dump 2 min after start
        # Could do a create and commit here of the table

    def dump(self, filename):
        if self.v: print ('dumping vessel_names to ',filename)
        o = open(filename,'w')
        o.write('mmsi,name,type_and_cargo,response_class\n')
        self.cu.execute('SELECT mmsi,name,type_and_cargo,response_class FROM vessel_name;')
        for row in self.cu.fetchall():
            o.write(','.join(['0' if i is None else str(i) for i in row])+'\n')
        o.close()
        self.last_dump_time = time.time()

    def preload_db(self,filename):
        'Load a dump file into the database'
        if self.v: print ('loading vessel name ',filename)
        infile = open(filename)
        infile.readline() # Skip header
        for line in infile:
            if len(line)<5 or line[0] == '#': continue
            mmsi,name,type_and_cargo,response_class = line.split(',')
            mmsi= int(mmsi)
            type_and_cargo = int(type_and_cargo)
            response_class = int(response_class)
            insert_or_update(self.cx, self.cu,
                             self.sql_insert_full,
                             self.sql_update_full,
                             {'mmsi':mmsi, 'type_and_cargo':type_and_cargo, 'name':name, 'response_class':response_class}
                             )
            self.vessels[mmsi] = (name, type_and_cargo)
#            try:
#                self.cu.execute(self.sql_insert_full,{'mmsi':mmsi, 'type_and_cargo':type_and_cargo, 'name':name, 'response_class':response_class} )
#            except:
#                print ('Failed to insert:',mmsi,name)
#            self.cx.commit()


    def update_partial(self, vessel_dict):
        # Only do the name or type_and_cargo... msg 24 class B
        mmsi = vessel_dict['mmsi']
        if 'name' in vessel_dict:
            name = vessel_dict['name'].rstrip(' @')
            type_and_cargo = None
        else:
            assert (vessel_dict['part_num']==1)
            name = None
            type_and_cargo = vessel_dict['type_and_cargo']

        if name is not None and len(name)==0:
            if self.v: print ('EMPTY NAME:',mmsi)
            return

        # if mmsi < 200000000:
        #     if name is not None:
        #         #print ('USELESS: mmsi =',mmsi, 'name:',name)
        #         pass
        #     else:
        #         #print ('USELESS: mmsi =',mmsi, 'name:',name, 'type_and_cargo:',type_and_cargo)
        #         pass
        #     #return # Drop these... useless

        new_vessel = {'mmsi':mmsi,'name':name,'type_and_cargo':type_and_cargo}

        # Totally new ship to us
        if mmsi not in self.vessels:
            if name is None:
                # FIX: add logic to hold on to the type_and_cargo for those we do not have names for
                #self.vessels[mmsi] = (None, type_and_cargo)
                # Don't update db until we have a name
                return

            # Have a name - try to insert - FIX: make it insert or update?
            #print ('before',mmsi)
            try:
                if ENABLE_DB: self.cu.execute(self.sql_insert,new_vessel)
                if self.v: print ('ADDED:',name, mmsi)
            except psycopg2.IntegrityError, e:
                if self.v: print (mmsi,'already in the database', str(e))
                pass # Already in the database
            #print ('after',mmsi)
            self.cx.commit()
            self.vessels[mmsi] = (name, type_and_cargo)
            return

        old_name,old_type_and_cargo = self.vessels[mmsi]

        if name is None:
            if type_and_cargo == old_type_and_cargo or old_name == None:
                #print ('NO_CHANGE_TO_CARGO:',mmsi)
                return

            #print ('UPDATING_B:',mmsi,'  ',old_name,old_type_and_cargo,'->',old_name,type_and_cargo)
            if ENABLE_DB: self.cu.execute(self.sql_update_tac, new_vessel)
            self.cx.commit()
            #print ('\ttac: ',old_name, type_and_cargo)
            self.vessels[mmsi] = (old_name, type_and_cargo)
            return

        # Update the name
        if name == old_name:
            #print ('NO_CHANGE_TO_NAME:',mmsi,old_name)
            return

        if self.v: print ('UPDATING_B:',mmsi,'  ',old_name,'->',name)
        # FIX: what if someone deletes the entry from the db?

        if ENABLE_DB: self.cu.execute(self.sql_update_name, new_vessel)
        self.cx.commit()
        #print ('\tname: ',name, old_type_and_cargo)
        self.vessels[mmsi] = (name, old_type_and_cargo)


    def update(self, vessel_dict):
        # vessel_dict must have mmsi, name, type_and_cargo

        if self.dump_interval + self.last_dump_time < time.time():
            self.dump(datetime.datetime.utcnow().strftime('vessel_names-%Y%m%dT%H%M%S.csv'))


        mmsi = vessel_dict['mmsi']
        #if mmsi in (0,1,1193046):
        #if mmsi < 200000000:
            # These vessels should be reported to the USCG
            #if self.v: print ('USELESS: mmsi =',mmsi, 'name:',vessel_dict['name'])
            #return # Drop these... useless

        name = vessel_dict['name'];
        type_and_cargo = vessel_dict['type_and_cargo']

        if mmsi not in self.vessels:
            #print ('NEW: mmsi =',mmsi)
            insert_or_update(self.cx, self.cu, self.sql_insert, self.sql_update, vessel_dict)
            self.vessels[mmsi] = (name, type_and_cargo)
            return

        #if mmsi in self.vessels:
        old_name,old_type_and_cargo = self.vessels[mmsi]
        if old_name == name and old_type_and_cargo == type_and_cargo:
            #print ('NO_CHANGE:',mmsi)
            return

        # Know we have inserted it, so safe to update
        #if mmsi == 367178330:
        #print ('UPDATING:',mmsi,'  ',old_name,old_type_and_cargo,'->',name,type_and_cargo)
        if old_name != name:
            print ('UPDATING:',mmsi,'  "%s" -> "%s"' % (old_name,name))
        #print (old_name,old_type_and_cargo,'->',)
        #print (name,type_and_cargo)

        # FIX: what if someone deletes the entry from the db?
        self.cu.execute(self.sql_update, vessel_dict)
        self.cx.commit()
        self.vessels[mmsi] = (name, type_and_cargo)


# taking from ais_decimate_traffic
def lon_to_utm_zone(lon):
    return int(( lon + 180 ) / 6) + 1

def dist (lon1, lat1, lon2, lat2):
    'calculate 2D distance.  Should be good enough for points that are close together'
    dx = (lon1-lon2)
    dy = (lat1-lat2)
    return math.sqrt(dx*dx + dy*dy)


def dist_utm_km (p1, p2):
    return dist_utm_m (p1[0],p1[1], p2[0],p2[1]) / 1000.

def dist_utm_m (lon1, lat1, lon2, lat2):
    'calculate 2D distance.  Should be good enough for points that are close together'
    zone = lon_to_utm_zone( (lon1 + lon2 ) / 2.) # Just don't cross the dateline!
    params = {'proj':'utm', 'zone':zone}
    proj = Proj(params)

    utm1 = proj(lon1,lat1)
    utm2 = proj(lon2,lat2)

    return dist(utm1[0],utm1[1],utm2[0],utm2[1])

def wkt_line(series_of_xy):
    assert(len(series_of_xy) > 1)

    segments = ["%.8f %.8f" % (pt['x'],pt['y']) for pt in series_of_xy]

    return 'LINESTRING(' + ','.join(segments) + ')'

class PositionCache:

    sql_insert = """
    INSERT INTO vessel_pos (mmsi, cog, sog, time_stamp, nav_status, pos)
    VALUES (%(mmsi)s, %(cog)s, %(sog)s, %(datetime)s, %(nav_status)s,
    ST_GeomFromText('POINT(%(x)s %(y)s)',4326)
    );"""
    sql_update_without_track = """
    UPDATE vessel_pos SET
       cog = %(cog)s,
       sog = %(sog)s,
       time_stamp = %(datetime)s,
       nav_status = %(nav_status)s,
       pos = ST_GeomFromText('POINT(%(x)s %(y)s)', 4326),
       track = NULL
    WHERE mmsi = %(mmsi)s;"""
    sql_update_with_track = """
    UPDATE vessel_pos SET
       cog = %(cog)s,
       sog = %(sog)s,
       time_stamp = %(datetime)s,
       nav_status = %(nav_status)s,
       pos = ST_GeomFromText('POINT(%(x)s %(y)s)', 4326),
       track = ST_GeomFromText(%(track)s, 4326)
    WHERE mmsi = %(mmsi)s;"""


#        track = ST_GeomFromText('%(track)s', 4326),
    def __init__(self, db_cx, min_dist_m=200, min_time_s=5 * 60,
                 #max_tail_count=10, max_tail_time_s = 15*60,
                 max_tail_count=100, max_tail_time_s = 30*60,
                 #max_age=60*60, # Drop the vessel after this time?
                 verbose=True):
        # 15 minutes is the recommended time?

        print ('THRESHOLDS dt:',min_time_s, 'dm:',min_dist_m,'\n')
        self.v = verbose
        self.vessels = {} # indexed by MMSI
        self.vessel_tails = {}
        self.min_dist_m = min_dist_m
        self.min_time_s = min_time_s
        self.max_tail_count = max_tail_count
        self.max_tail_time_s = max_tail_time_s
        self.cx = db_cx
        self.cu = db_cx.cursor()
        return

    def _update_tail(self, mmsi, msg):
        'Called from update.  Do not call this yourself'
        # Assume update figured out that this is a good position to use
        #print msg
        #mmsi = msg['mmsi']

        # Downsample to minimum set to save memory
        new_pos = {}
        for field in ('time_stamp', 'y', 'x'):
            try:
                new_pos[field] = msg[field]
            except:
                new_pos[field] = None

        if mmsi not in self.vessel_tails:
            #print ('TAIL_NEW:',mmsi) #,new_pos)
            self.vessel_tails[mmsi] = deque([new_pos,])
            #print ('deque:',self.vessel_tails[mmsi])
            return

        d = self.vessel_tails[mmsi]


        #print ('before:',self.vessel_tails[mmsi])
        while len(d) > self.max_tail_count:
            #print ('POPPING: too many',len(d),self.max_tail_count)
            d.pop() # Too many so ditch
        #print ('after:',self.vessel_tails[mmsi])

        # now nuke messages that are too old
        now = msg['time_stamp']

        while len(d) > 1 and (now - d[-1]['time_stamp'] > self.max_tail_time_s):
            #print ('POPPING: too old',now - d[-1]['time_stamp'],  self.max_tail_time_s)
            d.pop()


        # rough geographical distance is good enough
        # Only append if the point hasn't jumped too far -- aka multiple ships with the same mmsi
        # This will not work in polar regions
        if len(d) == 0:
            d.appendleft(new_pos)
        else:
            geo_dist = dist(new_pos['x'],new_pos['y'], d[0]['x'], d[0]['y'])
            if geo_dist < 0.5:
                d.appendleft(new_pos)
                #print ('.....keep:',geo_dist)
            else:
                print ('.....tail_test_DROP:',mmsi,'%.3f deg away' % geo_dist)


        if len(d)<2: return # Too short to make a line
        # Make a WKT line and push it into the db
        wkt = wkt_line(d)
        #print ('WKT:',wkt)
        return wkt

    def update(self,vessel_pos_report, time_stamp=None):
        msg = vessel_pos_report
        #
        # First, is this message useful?
        #
        mmsi = msg['mmsi']

        # FIX!!!!!!!!!!!!!!!!!!! just for testing
        #if mmsi != 367173260:    return


        #print ('mmsi:',mmsi)
        #if mmsi < 200000000:
            #bad_mmsi.add(mmsi)
            #print ('BAD_MMSI:',mmsi)
        #    return
        if msg['x']>180: return # No GPS, so what can we do?
        if 'repeat_indicator' in msg:
            if 0 != msg['repeat_indicator']:
                #print ('REPEAT')
                return # we do not touch repeated material.  danger

        #
        # Prep the world
        #
        #print ('time_stamp_before:', msg['time_stamp'])
        if time_stamp is not None: msg['time_stamp'] = time_stamp
        elif 'time_stamp' not in msg or msg['time_stamp'] is None:
            print ('Setting to now')
            msg['time_stamp'] = time.time()

        msg['time_stamp'] = int(float(msg['time_stamp']))
        msg['datetime'] = datetime.datetime.utcfromtimestamp(int(float(msg['time_stamp'])))

        # else: leave the entry as it came in


        new_pos = {} # copy just what we need to conserve memory
        for field in ('mmsi', 'true_heading', 'sog', 'nav_status', 'cog', 'time_stamp', 'datetime', 'y', 'x'):
            try:
                new_pos[field] = msg[field]
            except:
                new_pos[field] = None

        if new_pos['nav_status'] is not None:
            new_pos['nav_status'] = ais_lut.nav_status_table[new_pos['nav_status']]
        else:
            new_pos['nav_status'] = 'N/A - Class B device'


        if mmsi not in self.vessels:
            #print ('NEW:',mmsi)
            #print ('NEW:',mmsi,new_pos)
            self.vessels[mmsi] = new_pos
            self._update_tail(mmsi, new_pos)
            insert_or_update(self.cx, self.cu, self.sql_insert, self.sql_update_without_track, new_pos)
            return

        last = self.vessels[mmsi]
        #print ('last:',last)

        # FIX: move into if when working for speed to try to avoid the distance calc
        dt = msg['time_stamp'] - last['time_stamp']

        dm = dist_utm_m(msg['x'],msg['y'],last['x'],last['y'])

        if self.min_time_s > dt and dm < self.min_dist_m:
            #print ('DROPPED:  %d %.0f %.1f' %(mmsi, dt, dm))
            return # Has not been long enough or moved far enough

        # Go ahead and update it
        #print ('UPDATING_PATH:',mmsi, dt, dt >= self.min_time_s , dm, dm >= self.min_dist_m)
        self.vessels[mmsi] = new_pos
        wkt = self._update_tail(mmsi, new_pos)
        if wkt is None:
            #print ('UPDATE:',self.cu.mogrify(self.sql_update_without_track, new_pos))
            self.cu.execute(self.sql_update_without_track, new_pos)
        else:
            new_pos['track'] = wkt
            #print ('UPDATE:',self.cu.mogrify(self.sql_update_with_track, new_pos))
            self.cu.execute(self.sql_update_with_track, new_pos)

        self.cx.commit()

        return


def get_parser():
    from optparse import OptionParser
    parser = OptionParser(usage="%prog [options] [replay_log_file]",version="%prog "+__version__)

    parser.add_option('-i', '--in-port', type='int', default=31414,
			help='Where the data comes from [default: %default]')
    parser.add_option('-I', '--in-host', type='string', default='localhost',
			help='What host to read data from [default: %default]')

    parser.add_option('-d','--database-name',default='realtime_ais'
                      ,help='Name of database within the postgres server [default: %default]')

    parser.add_option('-D','--database-host',default='localhost'
                      ,help='Host name of the computer serving the dbx [default: %default]')
    defaultUser = os.getlogin()
    parser.add_option('-u','--database-user',default=defaultUser
                      ,help='Host name of the to access the database with [default: %default]')
    parser.add_option('-p','--database-passwd',default=None
                      ,help='Password to access the database with [default: None]')


    parser.add_option('-c','--create-database',default=False, action='store_true', help='Create the database and setup PostGIS (fink only)')
    parser.add_option('-C','--create-tables',default=False, action='store_true', help='Create the tables in the database')

    parser.add_option('--drop-database',       default=False, action='store_true', help = 'Remove the database.  DANGER - destroys data')
    parser.add_option('--drop-tables',         default=False, action='store_true', help = 'Remove the tables in the database.  DANGER - destroys data')
    parser.add_option('--delete-table-entries',default=False, action='store_true', help = 'Remove the contents of vessel_ tables in the database.  DANGER - destroys data')

    parser.add_option('--preload-names', default=None, help = 'Load ships from a vessel.csv dump file... mmsi,name,type_and_cargo,response_class')

    parser.add_option('-v','--verbose',default=False,action='store_true'
                      ,help='Make program output more verbose info as it runs')

    return parser

def drop_database(db_name,v=True):
    if v: sys.stderr.write('Dropping database...\n')
    r=os.system('dropdb -U postgres {db_name}'.format(db_name=db_name))
    if 0 != r: sys.stderr.write('Failed to dropdb\n')

def create_database(db_name, v=True):
    if v: sys.stderr.write('Creating database...\n')
    r = os.system('createdb -U postgres '+db_name)
    if 0 != r: sys.exit('Unable to create the database.  Exit code: '+str(r))
    r = os.system('createlang plpgsql '+db_name)
    #if 0 != r: sys.stderr.write('Failed to add plpgsql\n') # probably already in the template.  likely okay
    # Add postgis and spatialref sys.  Sorry this is fink specific
    r = os.system('psql -f /sw/share/doc/postgis84/contrib/postgis-1.5/postgis.sql '+db_name)
    r = os.system('psql -f /sw/share/doc/postgis84/contrib/postgis-1.5/spatial_ref_sys.sql '+db_name)

sql_create_vessel_name = """
CREATE TABLE vessel_name (
       mmsi INTEGER PRIMARY KEY,
       name VARCHAR(25), -- only need 20
       type_and_cargo INTEGER,
       response_class INTEGER -- 0 or Null == do not code differently.  1 == response vessel
);
-- FIX: Add indexes for mmsi and response_class
"""

sql_create_vessel_pos = """
CREATE TABLE vessel_pos (
       mmsi INTEGER PRIMARY KEY,
       cog INTEGER,
       sog REAL,
       time_stamp TIMESTAMP WITH TIME ZONE DEFAULT now(),
       nav_status VARCHAR(30) -- convert the code to a string for presentation
       );
SELECT AddGeometryColumn('vessel_pos','pos',4326,'POINT',2);
SELECT AddGeometryColumn('vessel_pos','track',4326,'LINESTRING',2);

-- FIX: Add index on mmsi
"""

def create_tables(cx, v):
    cu = cx.cursor()
    if v: print (sql_create_vessel_name)
    cu.execute(sql_create_vessel_name)
    if v: print (sql_create_vessel_pos)
    cu.execute(sql_create_vessel_pos)
    cx.commit()

def drop_tables(cx,v):
    cu = cx.cursor()
    try:
        cu.execute('DROP TABLE vessel_name')
    except:
        print ('FAILED: DROP TABLE vessel_name')
    try:
        cu.execute('DROP TABLE vessel_pos')
    except:
        print ('FAILED: DROP TABLE vessel_pos')
    cx.commit()

def delete_table_entries(cx, v):
    cu = cx.cursor()
    cu.execute('DELETE FROM vessel_name')
    cu.execute('DELETE FROM vessel_pos')
    cx.commit()

######################################################################
# Main network handling code - threaded
######################################################################

class ListenThread(threading.Thread):
    def __init__(self, host_name, port_num, line_queue, verbose=True):
        self.host_name = host_name
        self.port_num = port_num
        self.line_queue = line_queue
        self.v = verbose
        self.running = True

        threading.Thread.__init__(self)

    def stop(self):
        self.running = False

    def run(self):
        print ('ListenThread:',self.host_name, self.port_num)

        connected = False

        count = 0
        queue_count = 0
        while self.running:
            count += 1
            if count % 100 == 0:
                print ('outer_ListenThread:',count,datetime.datetime.now().strftime('%dT%H:%M:%S'), 'EST\t connected:',connected )
            time.sleep(.25)
            try:
                soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                soc.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                soc.connect((self.host_name, self.port_num))
            except socket.error, inst:
                print ('soc connect failed:', str(inst))
            else:
                connected = True
                print ('CONNECT to ',self.host_name, self.port_num)

            while self.running and connected:
                count += 1
                if count % 1000 == 0:
                    print ('inner_ListenThread:',count,datetime.datetime.now().strftime('%dT%H:%M:%S'), 'EST\tconnected:',connected,'queue_count:',queue_count )
                if count % 100 == 0:
                    #print ('sleeping')
                    time.sleep(0.001)
                readersready,outputready,exceptready = select.select([soc,],[],[],0.1)
                if len(readersready) == 0: continue
                data = soc.recv(160)
                if len(data) == 0:
                    connected = False
                    print ('DISCONNECT')
                    break
                try:
                    self.line_queue.put(data)
                    queue_count += 1
                except Queue.Full:
                    print ('QUEUE_FULL dropping data') # Not currently handled the normal queue way


class ProcessingThread(threading.Thread):
    '# FIX: break into more threads if it helps performance'
    def __init__(self, vessel_names, pos_cache, line_queue, norm_queue, verbose=True):
        self.v = verbose
        self.vessel_names = vessel_names
        self.pos_cache = pos_cache

        self.line_queue = line_queue
        self.norm_queue = norm_queue
        self.running = True

        threading.Thread.__init__(self)

    def stop(self):
        self.running = False

    def run(self):
        print ('Started processing thread...')

        while self.running:
            try:
                self.loop()
            except Exception, e:
                print ('\nERROR: Top level exception in ProcessingThread')
                print (str(e))
                traceback.print_exc(file=sys.stderr)
                time.sleep(5) # Throttle back so not as to fill up the disk with error mesages


    def loop(self):
        print ('Started processing thread...')

        while self.running:
            try:
                line = self.line_queue.get(True,1) # Block until we have a line
            except Queue.Empty:
                continue # just a timeout with no data
            #print ('line:',line)
            if len(line) < 15 or '!AIVDM' != line[:6]: continue # try to avoid the regex if possible

            try:
                match = uscg_ais_nmea_regex.search(line).groupdict()
            except AttributeError:
                if 'AIVDM' in line:
                    print ('BAD_MATCH:',line)
                continue

            self.norm_queue.put(match)

            # FIX: possibly decouple here
            while self.norm_queue.qsize()>0:

                try:
                    result = self.norm_queue.get(False)
                except Queue.Empty:
                    continue

                if len(result['body'])<10 or result['body'][0] not in ('1', '2', '3', '5', 'B', 'C', 'H') :
                    continue

                try:
                     msg = ais.decode(result['body'])
                except Exception as e:
                    if 'not yet handled' in str(e) or 'not known' in str(e): continue
                    print ('BAD Decode:',result['body'][0])
                    print ('\tE:',Exception)
                    print ('\te:',e)
                    continue

                if msg['id'] in (1,2,3,18,19):
                    try:
                        msg['time_stamp'] = float(result['time_stamp'])
                    except TypeError, e:
                        print ('BAD time_stamp:',str(e))
                        traceback.print_exc(file=sys.stderr)
                        continue

                    self.pos_cache.update(msg)
                    continue

                if msg['id'] == 24:
                    self.vessel_names.update_partial(msg)
                    continue

                if msg['id'] in (5,19):
                    msg['name'] = msg['name'].strip(' @')
                    if len(msg['name']) == 0: continue # Skip blank names
                    self.vessel_names.update(msg)



def run_network_app(host_name, port_num, vessel_names, pos_cache, line_queue, norm_queue, verbose=False):

    print ('Initial threads:',threading.active_count())

    print ('Starting listen thread...',host_name, port_num)
    listen_thread = ListenThread(host_name, port_num, line_queue)
    listen_thread.start()

    processing_thread = ProcessingThread(vessel_names, pos_cache, line_queue, norm_queue, verbose)
    processing_thread.start()

    running = True
    count = 0
    while running:
        count += 1
        if count % 20 == 0:
            print ('main:',count,'\tnow:',datetime.datetime.now().strftime('%dT%H:%M:%S'), ' EST \tthreads:',  threading.active_count() )
            print ('\tline_q_size:',line_queue.qsize())
            print ('\tnorm_q_size:',norm_queue.qsize(),'\t\t%.1f (msgs/sec)' % norm_queue.get_rate())

        try:
            time.sleep(.5)
        except exceptions.KeyboardInterrupt:
            running = False
            print ('shutting down...\nstopping listener...')
            listen_thread.stop()
            time.sleep(.2) # Give it time to clear the buffers
            print ('stopping processing thread...')
            processing_thread.stop()

    print ('falling off the end')


######################################################################

def main():
    (options,args) = get_parser().parse_args()
    v = options.verbose

    match_count = 0
    counters = {}
    for i in range(30):
        counters[i] = 0

    if v: print ('connecting to db')
    options_dict = vars(options) # Turn options into a real dictionary

    if options.drop_database:   drop_database(  options.database_name, v)
    if options.create_database: create_database(options.database_name, v)

    cx_str = "dbname='{database_name}' user='{database_user}' host='{database_host}'".format(**options_dict)
    if v: print ('cx_str:',cx_str)
    cx = psycopg2.connect(cx_str)

    if options.drop_tables: drop_tables(cx, v)
    if options.create_tables: create_tables(cx, v)

    if options.delete_table_entries: delete_table_entries(cx, v)


    if v: print ('initilizing caches...')

    # Caches
    vessel_names = VesselNames(cx, verbose = v)
    pos_cache = PositionCache(db_cx=cx, verbose=v)

    if options.preload_names is not None: vessel_names.preload_db(options.preload_names)


    # FIFOs

    line_queue = LineQueue(maxsize = 1000) # If we have to drop, drop early
    norm_queue = NormQueue()

    if len(args) == 0:
        print ('GOING_LIVE: no log files specified for reading')

        run_network_app(
            host_name = options.in_host,
            port_num = options.in_port,
            vessel_names = vessel_names,
            pos_cache = pos_cache,
            line_queue = line_queue,
            norm_queue = norm_queue,
            verbose=v
            )

        print ('GOODBYE... main thread ending')
        return

    print ('USING_LOG_FILES: non-threaded')
    for infile in args:
        if v: print ('reading data from ...',infile)
        last_time = time.time()
        last_count = 0
        last_match_count = 0

        last_ais_msg_cnt = 0 # FULL AIS message decoded
        ais_msg_cnt = 0

        for line_num, text in enumerate(open(infile)):

            #if line_num > 300: break
            #print ()
            if line_num % 10000 == 0:
                print ("line: %d   %d" % (line_num,match_count),
                       #'\tupdate_rate:',(match_count - last_match_count) / (time.time() - last_time), '(lines/sec)'
                       #'\tupdate_rate:',(line_num - last_count) / (time.time() - last_time), '(msgs/sec)'
                       '\tupdate_rate:',(ais_msg_cnt - last_ais_msg_cnt) / (time.time() - last_time), '(msgs/sec)'
                       )
                last_time = time.time()
                last_count = line_num
                last_match_count = match_count
                last_ais_msg_cnt = ais_msg_cnt

            if 'AIVDM' not in text:
                continue

            line_queue.put(text)

            while line_queue.qsize() > 0:
                line = line_queue.get(False)
                if len(line) < 15 or '!' != line[0]: continue # Try to go faster

                #print ('line:',line)
                try:
                    match = uscg_ais_nmea_regex.search(line).groupdict()
                    match_count += 1
                except AttributeError:
                    if 'AIVDM' in line:
                        print ('BAD_MATCH:',line)
                    continue

                norm_queue.put(match)

                while norm_queue.qsize()>0:

                    try:
                        result = norm_queue.get(False)
                    except Queue.Empty:
                        continue

                    if len(result['body']) < 10: continue
                    # FIX: make sure we have all the critical messages

                    # FIX: add 9
                    if result['body'][0] not in ('1', '2', '3', '5', 'B', 'C', 'H') :
                        #print( 'skipping',result['body'])
                        continue
                    #print( 'not skipping',result['body'])

                    try:
                         msg = ais.decode(result['body'])
                    #except ais.decode.error:
                    except Exception as e:
                        #print ()

                        if 'not yet handled' in str(e):
                            continue
                        if ' not known' in str(e): continue

                        print ('BAD Decode:',result['body'][0]) #result
                            #continue
                        print ('E:',Exception)
                        print ('e:',e)
                        continue
                        #pass # Bad or unhandled message

                    #continue #  FIX: Remove after debugging

                    counters[msg['id']] += 1

                    if msg['id'] in (1,2,3,5,18,19,24): ais_msg_cnt += 1

                    #continue  # Skip all the database stuff

                    if msg['id'] in (1,2,3,18,19):
                        # for field in ('received_stations', 'rot', 'raim', 'spare','timestamp', 'position_accuracy', 'rot_over_range', 'special_manoeuvre','slot_number',
                        #               'utc_spare', 'utc_min', 'slots_to_allocate', 'slot_increment','commstate_flag', 'mode_flag', 'utc_hour', 'band_flag', 'keep_flag',
                        #               ):
                        #     try:
                        #         msg.pop(field)
                        #     except:
                        #         pass
                        #print (msg['mmsi'])
                        #print (','.join(["'%s'" %(key,)for key in msg.keys()]))
                        #print (result)
                        msg['time_stamp'] = float(result['time_stamp'])
                        #if msg['mmsi'] in (304606000, 366904910, 366880220): dump_file.write(str(msg)+',\n')
                        pos_cache.update(msg)
                        continue

                    #continue # FIX for debugging


                    if msg['id'] == 24:
                        #print24(msg)
                        vessel_names.update_partial(msg)

                    #continue # FIX remove

                    if msg['id'] in (5,19):
                        msg['name'] = msg['name'].strip(' @')
                        if len(msg['name']) == 0: continue # Skip blank names
                        #print ('UPDATING vessel name', msg)
                        #vessel_names.update(msg['mmsi'], msg['name'].rstrip('@'), msg['type_and_cargo'])
                        #if msg['mmsi'] == 367178330:
                        #    print (' CHECK:', msg['mmsi'], msg['name'])
                        vessel_names.update(msg)
                        #print ()

    print ('match_count:',match_count)
    #print (counters)
    for key in counters:
        if counters[key] < 1: continue
        print ('%d: %d' % (key,counters[key]))

if __name__ == '__main__':
    main()
