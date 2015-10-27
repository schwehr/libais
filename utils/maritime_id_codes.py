#!/usr/bin/env python
from __future__ import print_function

import sys
import urllib,re
import sqlite3

#con = sqlite3.connect('ais.db3')
#cx = sqlite3.connect('test.db3')
#cu = cx.cursor()

######################################################################
# MMSI Prefix handling

def mmsi_codes(filename=None):
    '''Create a table of 3 digit MMSI codes
    @return: code lookup table
    @rtype: dict
    '''

    entry_re_str = r'''\<tr\>\<td\>(?P<prefixes>[0-9 ,]*)\</td\>\<td\>(?P<country>[^<]*)\</td\>\</tr\>'''
    entry_re = re.compile(entry_re_str)

    if not filename:
        html = urllib.urlopen('http://www.itu.int/cgi-bin/htsh/glad/cga_mids.sh?lng=E')
    else:
        html = file(filename)

    # Lame parser.  Would be better to use BeautifulSoup or html5lib
    last_country = None
    codes = {}
    for line in html:
        if '<tr>' != line[0:4]: continue
        matches = entry_re.search(line)
        if not matches: continue

        prefixes = matches.group('prefixes')
        country = matches.group('country')

        if chr(244) in country:
            country = country.replace(chr(244),'o')
        if ' - ' == country:
            country = last_country

        for prefix in prefixes.split(','):
            codes[int(prefix)] = country

        last_country = country

    return codes

if __name__=='__main__':
    codes = mmsi_codes('mid.html')
    for code in codes:
        print ('%s,"%s"' % (code,codes[code]))
