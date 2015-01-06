#!/usr/bin/env python

"""Test the ais compatibility layer

"""

import ais.stream
import ais.compatibility.gpsd
import unittest
import os.path
import json
import itertools
import re
import subprocess


known_bad = set(('radio','data', 'addressed', 'reserved', 'regional', 'structured', 'app_id'))
precission = 5.0

known_problems = {
    27: set(('status',)),
    17: set(('lat', 'lon')),
    15: set(('mmsi2',)),
    20: set(('offset4', 'increment4', 'increment3', 'offset3', 'timeout3', 'timeout4', 'number4', 'number3')),
    2:  set(('turn', 'status_text')),
    9:  set(('speed',))
    }


def strNum(s):
    try:
        return float(s)
    except:
        return s

def dictDiff(a, b):
    def cmp(x, y):
        if x == y:
            return True
        x = strNum(x)
        y = strNum(y)
        if isinstance(x, (str, unicode)) and isinstance(y, (str, unicode)):
            if re.sub(r"[^a-z]", r"", unicode(x).lower()) == re.sub(r"[^a-z]", r"", unicode(y).lower()): return True
        if isinstance(x, (int, float, long)) and isinstance(y, (int, float, long)):
            if abs(float(x) - float(y)) < precission: return True
        return False

    return {
        "removed": {key: a[key] for key in a if key not in b and key not in known_bad},
        "changed": {key: (a[key], b[key]) for key in a if key in b and key not in known_bad and not cmp(a[key], b[key])},
        "added": {key: b[key] for key in b if key not in a and key not in known_bad}
        }

class GPSDCompatibility(unittest.TestCase):

    def setUp(self):
        self.dir = os.path.split(__file__)[0]
        self.nmea = os.path.join(self.dir, "typeexamples.nmea")
        self.json = os.path.join(self.dir, "typeexamples.gpsdecode.json")

        if subprocess.call("gpsdecode < %s > %s" % (self.nmea, self.json), shell=True) != 0:
            raise Exception("Unable to run gpsdecode. Is gpsd installed and working?")

    def tearDown(self):
        os.unlink(self.json)
        
    def testAll(self):
        def gpsd():
            with open(self.json) as f:
                for msg in f:
                    yield json.loads(msg)

        def libais():
            with open(os.path.join(self.dir, "typeexamples.nmea")) as f:
                for msg in ais.stream.decode(f):
                    yield ais.compatibility.gpsd.mangle(msg)

        g = iter(gpsd())
        a = iter(libais())

        try:
            while True:
                gmsg = g.next()
                amsg = a.next()
                while amsg['type'] != gmsg['type']:
                    amsg = a.next()

                if gmsg['type'] in known_problems:
                    for key in known_problems[gmsg['type']]:
                        if key in gmsg: del gmsg[key]
                        if key in amsg: del amsg[key]

                diff = dictDiff(gmsg, amsg)
                self.assertTrue(not diff['changed'])
                self.assertTrue(not diff['removed'])
        except StopIteration:
            pass

if __name__=='__main__':
    unittest.main()
