#!/usr/bin/env python
from __future__ import print_function

# Track the status of the NAIS feed into PostGIS for nagios

__author__ = 'Kurt Schwehr'
__version__ = '$Revision: 13787 $'.split()[1]
__revision__  = __version__ # For pylint
__date__ = '$Date: 2010-05-25 14:27:19 -0400 (Tue, 25 May 2010) $'.split()[1]
__copyright__ = '2010'
__license__   = 'LGPL v3'
__contact__   = 'kurt at ccom.unh.edu'

import psycopg2 # Bummer... not yet ready for python 3.1

import sys, os
import time
import datetime
import exceptions # For KeyboardInterupt pychecker complaint
import traceback



def get_parser():
    from optparse import OptionParser
    parser = OptionParser(usage="%prog [options] ",version="%prog "+__version__)

    parser.add_option('-d','--database-name',default='realtime_ais'
                      ,help='Name of database within the postgres server [default: %default]')

    parser.add_option('-D','--database-host',default='localhost'
                      ,help='Host name of the computer serving the dbx [default: %default]')

    defaultUser = os.getlogin()
    parser.add_option('-u','--database-user',default=defaultUser
                      ,help='Host name of the to access the database with [default: %default]')

    parser.add_option('-p','--database-passwd',default=None
                      ,help='Password to access the database with [default: None]')


#    parser.add_option('--database-table',default=None
#                      ,help='Password to access the database with [default: None]')


    parser.add_option('-v','--verbose',default=False,action='store_true'
                      ,help='Make program output more verbose info as it runs')

    return parser


######################################################################

def main():
    (options,args) = get_parser().parse_args()
    v = options.verbose

    options_dict = vars(options) # Turn options into a real dictionary
    cx_str = "dbname='{database_name}' user='{database_user}' host='{database_host}'".format(**options_dict)
    if v: print ('cx_str:',cx_str)
    try:
        cx = psycopg2.connect(cx_str)

        cu = cx.cursor()
        cu.execute("SELECT count(*) FROM vessel_pos WHERE time_stamp>(now() + interval '3 hours 50 minutes');")
        row = cu.fetchone()
        count = row[0]
    except:
        print ('Unknow error with the database')
        sys.exit(3)

    print ('{count} vessels received in the last 10 minutes'.format(count=count))
    if count < 100: sys.exit(2)
    if count < 1000: sys.exit(1)
    sys.exit(0)

if __name__ == '__main__':
    main()
