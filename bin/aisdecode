#!/usr/bin/env python

"""
DEPRECATED: Will be removed in v1.0
"""

import warnings

import ais
import sys
import ais.compatibility.gpsd
import ais.stream

try:
    import json
except:
    json = None
try:
    import msgpack
except:
    msgpack = None


warnings.warn(
    "This utility is deprecated and will be removed in 1.0",
    FutureWarning,
    stacklevel=2)


args = {"encoding": "json"}
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

# Support for legacy script
if 'validateChecksum' in args: args['validate_checksum'] = args.pop('validateChecksum')
if 'allowUnknown' in args: args['allow_unknown'] = args.pop('allowUnknown')
if 'treatABequal' in args: args['treat_ab_equal'] = args.pop('treatABequal')

for key in ("uscg", "validate_checksum", "verbose", "allow_unknown", "treat_ab_equal", "pass_invalid_checksums", "allow_missing_timestamps"):
    if key in args and isinstance(args[key], str):
        args[key] = args[key] == 'True'

if "help" in args:
    print("""Usage: aisdecode [OPTIONS] < INFILE.nmea > OUTFILE.json
Available opptions:
    --gpsd Use gpsd compatible output
        --copy-tagblock-timestamp=False

    --encoding=msgpack Use msgpack encoding
    --encoding=json Use json encoding (default)

    --keep_nmea Include the raw nmea as a field in the decoded output

    --verbose
    --max_errors=NUM Terminate if error rate is > NUM%.
    --allow_unknown
    --ignore_tagblock_station
    --treat_ab_equal
    --pass_invalid_checksums
    --allow_missing_timestamps

    --uscg=False
    --validate_checksum=False
""")
    sys.exit(0)

encoding = args.pop('encoding', 'json')
if encoding == 'json':
    def write(msg):
        json.dump(msg, sys.stdout)
        sys.stdout.write("\n")

if encoding == 'msgpack':
    def write(msg):
        msgpack.pack(msg, sys.stdout)

use_gpsd = args.pop("gpsd", False)
copy_tagblock_timestamp = args.pop("copy-tagblock-timestamp", True)


if 'window' in args:
    args['window'] = int('window')

if "max_errors" in args:
    args["max_errors"] = float(args["max_errors"])

gpsd_mangle = ais.compatibility.gpsd.Mangler(copy_tagblock_timestamp=copy_tagblock_timestamp)

for msg in ais.stream.decode(sys.stdin, **args):
    if use_gpsd:
        msg = gpsd_mangle(msg)
    write(msg)
