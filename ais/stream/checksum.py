#!/usr/bin/env python
# Copied from https://github.com/schwehr/noaadata
# NOTE: GPL v3 license (check with Kurt if he can relicense this)
__version__ = '$Revision: 2189 $'.split()[1]
__date__ = '$Date: 2006-05-29 15:40:45 -0400 (Mon, 29 May 2006) $'.split()[1]
__author__ = 'Kurt Schwehr'

__doc__='''
Utilities for working with NMEA strings.
@author: '''+__author__+'''
@version: ''' + __version__ +'''
@copyright: 2006

@var __date__: Date of last svn commit
@undocumented: __version__ __author__ __doc__ myparser
'''

# Python standard libraries
import time, sys

# Local Modules
#import binary

#import verbosity
#from verbosity import BOMBASTIC,VERBOSE,TRACE,TERSE,ALWAYS


def checksumStr(data):
    """
    Take a NMEA 0183 string and compute the checksum.
    @param data: NMEA message.  Leading ?/! and training checksum are optional
    @type data: str
    @return: hexidecimal value
    @rtype: str

    Checksum is calculated by xor'ing everything between ? or ! and the *

    >>> checksumStr("!AIVDM,1,1,,B,35MsUdPOh8JwI:0HUwquiIFH21>i,0*09")
    '09'
    >>> checksumStr("AIVDM,1,1,,B,35MsUdPOh8JwI:0HUwquiIFH21>i,0")
    '09'
    """

    # FIX: strip off new line at the end too
    if data[0]=='!' or data[0]=='?': data = data[1:]
    if data[-1]=='*': data = data[:-1]
    if data[-3]=='*': data = data[:-3]
    # FIX: rename sum to not shadown builting function
    sum=0
    for c in data: sum = sum ^ ord(c)
    sumHex = "%x" % sum
    if len(sumHex)==1: sumHex = '0'+sumHex
    return sumHex.upper()


######################################################################
# common variables
import re
nmeaChecksumRegExStr = r"""\,[0-9]\*[0-9A-F][0-9A-F]"""
nmeaChecksumRE = re.compile(nmeaChecksumRegExStr)

def isChecksumValid(nmeaStr, allowTailData=True):
    """Return True if the string checks out with the checksum

    @param allowTailData: Permit handing of Coast Guard format with data after the checksum
    @param data: NMEA message.  Leading ?/! are optional
    @type data: str
    @return: True if the checksum matches
    @rtype: bool

    >>> isChecksumValid("!AIVDM,1,1,,B,35MsUdPOh8JwI:0HUwquiIFH21>i,0*09")
    True

    Corrupted:

    >>> isChecksumValid("!AIVDM,11,1,,B,35MsUdPOh8JwI:0HUwquiIFH21>i,0*09")
    False
    """

    if allowTailData:
	match = nmeaChecksumRE.search(nmeaStr)
	if not match: return False
	nmeaStr = nmeaStr[:match.end()]
	#if checksum.upper()==checksumStr(nmeaStr[match.end()


    if nmeaStr[-3]!='*': 
	print 'FIX: warning... bad nmea string'
	return False  # Bad string without proper checksum
    checksum=nmeaStr[-2:]
    if checksum.upper()==checksumStr(nmeaStr).upper(): return True
    return False

# ######################################################################
# if __name__=='__main__':
#     from optparse import OptionParser
#     myparser = OptionParser(usage="%prog [options]",
# 			    version="%prog "+__version__)
#     myparser.add_option('--test','--doc-test',dest='doctest',default=False,action='store_true',
#                         help='run the documentation tests')
#     verbosity.addVerbosityOptions(myparser)
#     (options,args) = myparser.parse_args()

#     success=True

#     if options.doctest:
# 	import os; print os.path.basename(sys.argv[0]), 'doctests ...',
# 	sys.argv= [sys.argv[0]]
# 	if options.verbosity>=VERBOSE: sys.argv.append('-v')
# 	import doctest
# 	numfail,numtests=doctest.testmod()
# 	if numfail==0: print 'ok'
# 	else: 
# 	    print 'FAILED'
# 	    success=False

#     if not success:
# 	sys.exit('Something Failed')
