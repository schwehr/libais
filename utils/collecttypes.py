#! /usr/bin/env python

import ais
import sys
import ais.compatibility.gpsd
import ais.stream

args = {}
files = []
for arg in sys.argv[1:]:
    if arg.startswith("--"):
        arg = arg[2:]
        if '=' in arg:
            arg, value = arg.split("=", 1)
            args[arg] = value
        else:
            args[arg] = True
    else:
        files.append(arg)

if "help" in args:
    print """Usage: collecttypes.py < INFILE.nmea > OUTFILE.nmea

Collects one message of each type.
"""
    sys.exit(0)

types = set()
for msg in ais.stream.decode(sys.stdin, keep_nmea=True, allowUnknown=True, allow_missing_timestamps=True):
    if msg['id'] in types: continue
    types.add(msg['id'])
    sys.stdout.write(msg['nmea'])
