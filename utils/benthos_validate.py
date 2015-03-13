#!/usr/bin/env python


"""
Validator to ensure libais output is what Benthos expects to receive
"""


import json
import os
import unittest
import sys


# Keys are fieldnames and values are dictionaries containing 'test', a function verifying if a value is acceptable,
# 'good' a value that will pass the test, and 'bad' a value that will not pass the test.
# The unittest for this utility check to see if good and bad values are iterable so testing multiple values is possible.
MSG_VALIDATION_LOOKUP = {

    # Pulled from types 1, 2, and 3 GPSD spec
    'type': {
        'test': lambda x: x in range(1, 28),
        'good': 5,
        'bad': -1
    },
    'repeat': {
        'test': lambda x: 0 <= x <= 2 ** 2,
        'good': 10,
        'bad': -1
    },
    'mmsi': {
        'test': lambda x: isinstance(x, str) and len(x) <= 2 ** 30,
        'good': '12345678902345678',
        'bad': 1234
    },
    'status': {
        'test': lambda x: x in range(0, 16),
        'good': 2,
        'bad': -10
    },
    'turn': {
        'test': lambda x: x in range(-127, 129),
        'good': 125,
        'bad': -1000
    },
    'sog': {
        'test': lambda x: 0 <= x <= 102.2 or x is 1022,
        'good': 1022,
        'bad': 103
    },
    'accuracy': {
        'test': lambda x: x in (0, 1),
        'good': 0,
        'bad': 2,
    },
    'lat': {
        'test': lambda x: -90 <= x <= 90 or x is 91,
        'good': 91,
        'bad': -100
    },
    'lon': {
        'test': lambda x: -180 <= x <= 180 or x is 181,  # TODO: Should -180 be a valid value?  Maybe `-180 < x` instead?
        'good': 181,
        'bad': -180.1
    },
    'course': {
        'test': lambda x: 0 <= x <= 359 or x is 3600,  # TODO: Should -90 be a valid value?  Maybe `-90 < x` instead?
        'good': 3600,
        'bad': 360
    },
    'heading': {
        'test': lambda x: 0 <= x <= 359 or x is 511,
        'good': 511,
        'bad': -102
    },
    'second': {
        'test': lambda x: x in range(0, 64),
        'good': 63,
        'bad': 64
    },
    'maneuver': {
        'test': lambda x: x in (1, 2),
        'good': 2,
        'bad': 3
    },
    'raim': {
        'test': lambda x: x in (0, 1),  # TODO: bool is more Pythonic if the field is actually boolean and not state
        'good': 0,
        'bad': -2
    },
    'radio': {
        'test': lambda x: x is not False,  # TODO: What will this value be?
        'good': True,
        'bad': False
    },

    # Pulled from type 5 GPSD spec
    'ais_version': {
        'test': lambda x: x in (0, 1, 2, 3),  # TODO: Should always be 0 right now.  The other vals are reserved.
        'good': 2,
        'bad': True
    },
    'imo': {
        'test': lambda x: isinstance(x, str) and len(x) <= 2 ** 30,  # TODO: Length=30 bits
        'good': 'value',
        'bad': True,
    },
    'callsign': {
        'test': lambda x: isinstance(x, str) and len(x) <= 2 ** 42,
        'good': 'good',
        'bad': json
    },
    'shipname': {
        'test': lambda x: isinstance(x, str) and len(x) <= 2 ** 120,  # TODO: Length=120 bits
        'good': 'good value',
        'bad': False
    },
    'shiptype': {
        'test': lambda x: x in range(0, 100),
        'good': 87,
        'bad': str
    },
    'to_bow': {
        'test': lambda x: 0 <= x <= 2 ** 9,
        'good': 1,
        'bad': -1
    },
    'to_stern': {
        'test': lambda x: 0 <= x <= 2 ** 9,
        'good': 0,
        'bad': tuple
    },
    'to_port': {
        'test': lambda x: 0 <= x <= 2 ** 6,
        'good': 1,
        'bad': -34
    },
    'to_starboard': {
        'test': lambda x: 0 <= x <= 2 ** 6,
        'good': 0,
        'bad': False,
    },
    'epfd': {
        'test': lambda x: x in range(0, 9),
        'good': 8,
        'bad': 10
    },
    'month': {
        'test': lambda x: x in range(0, 13),
        'good': 12,
        'bad': 'asdf'
    },
    'day': {
        'test': lambda x: x in range(0, 32),
        'good': 1,
        'bad': ':'
    },
    'hour': {
        'test': lambda x: x in range(0, 25),
        'good': 23,
        'bad': ','
    },
    'minute': {
        'test': lambda x: x in range(0, 61),
        'good': 54,
        'bad': '*'
    },
    'draught': {
        'test': lambda x: 0 < x <= 2 ** 8,
        'good': 1,
        'bad': 2 ** 8 + 1
    },
    'destination': {
        'test': lambda x: isinstance(x, str) and x <= 2 ** 120,
        'good': 'v',
        'bad': 2 * 2
    },
    'dte': {
        'test': lambda x: x in (0, 1),  # TODO: Switch to a more Pythonic bool if this is actually bolean and not a status
        'good': 0,
        'bad': 8
    },

    # Pulled from type 18 GPSD spec
    'reserved': {
        'test': lambda x: x is None,
        'good': None,
        'bad': 1
    },
    'regional': {
        'test': lambda x: x is None,
        'good': None,
        'bad': -1
    },
    'speed': {
        'test': lambda x: x <= 2 ** 10,
        'good': 0,
        'bad': str
    },
    'cs': {
        'test': lambda x: x in (0, 1),  # Not bool - state
        'good': 0,
        'bad': 7
    },
    'display': {
        'test': lambda x: x in (0, 1),  # Not bool - state
        'good': 1,
        'bad': 'j'
    },
    'dsc': {
        'test': lambda x: x in (0, 1),  # TODO: Switch to a more Pythonic bool?
        'good': 1,
        'bad': -45
    },
    'band': {
        'test': lambda x: x in (0, 1),  # TODO: Switch to a more Pythonic bool?
        'good': 0,
        'bad': 4
    },
    'msg22': {
        'test': lambda x: x in (0, 1),  # TODO: Switch to a more Pythonic bool?
        'good': 0,
        'bad': -2
    },
    'assigned': {
        'test': lambda x: x in (0, 1),  # TODO: Switch to a more Pythonic bool?
        'good': 1,
        'bad': -33
    },

    # All type 19 are covered between 1, 2, 3, and 18

    # Pulled from type 24 GPSD spec
    'partno': {
        'test': lambda x: x in (0, 1),
        'good': 0,
        'bad': -1
    },
    'vendorid': {
        'test': lambda x: isinstance(x, str) and x <= 2 ** 18,
        'good': 'this is a gooooooooood value',
        'bad': int
    },
    'model': {
        'test': lambda x: isinstance(x, str) and x <= 2 ** 4,
        'good': 'something',
        'bad': 333
    },
    'serial': {
        'test': lambda x: isinstance(x, str) and x <= 2 ** 20,
        'good': 'pawoeiras',
        'bad': -1
    },
    'mothership_mmsi': {
        'test': lambda x: isinstance(x, str) and x <= 2 ** 30,
        'good': 'done ... finally ...',
        'bad': -200
    },
    # Pulled from type 27 GPSD spec
    'gnss': {
        'test': lambda x: x in (0, 1),  # Not bool - state
        'good': 0,
        'bad': 3
    }
}


# Keys are message types and values are lists of fields that type expects
MSG_TYPE_FIELDS = {
    1: [
        'type', 'repeat', 'mmsi', 'status', 'turn', 'sog', 'accuracy', 'lat', 'lon', 'course', 'heading', 'second',
        'maneuver', 'raim', 'radio'
    ],
    2: [
        'type', 'repeat', 'mmsi', 'status', 'turn', 'sog', 'accuracy', 'lat', 'lon', 'course', 'heading', 'second',
        'maneuver', 'raim', 'radio'
    ],
    3: [
        'type', 'repeat', 'mmsi', 'status', 'turn', 'sog', 'accuracy', 'lat', 'lon', 'course', 'heading', 'second',
        'maneuver', 'raim', 'radio'
    ],
    5: [
        'type', 'repeat', 'mmsi', 'ais_version', 'imo', 'callsign', 'shipname', 'shiptype', 'to_bow', 'to_stern',
        'to_port', 'to_starboard', 'epfd', 'month', 'day', 'hour', 'minute', 'draught', 'destination', 'dte'
    ],
    18: [
        'type', 'repeat', 'mmsi', 'reserved', 'speed', 'accuracy', 'lon', 'lat', 'course', 'heading', 'second',
        'regional', 'cs', 'display', 'dsc', 'band', 'msg22', 'assigned', 'raim', 'radio', 'dte', 'assigned'
    ],
    19: [
        'type', 'repeat', 'mmsi', 'reserved', 'speed', 'accuracy', 'lon', 'lat', 'course', 'heading', 'second',
        'regional', 'shipname', 'shiptype', 'to_bow', 'to_stern', 'to_port', 'to_starboard', 'epfd', 'raim', 'dte',
        'assigned'
    ],
    24: [
        'type', 'repeat', 'mmsi', 'partno', 'shipname', 'shiptype', 'vendorid', 'model', 'serial', 'callsign',
        'to_bow', 'to_stern', 'to_port', 'to_starboard', 'mothership_mmsi'
    ],
    27: None
}


def validate_messages(messages, err=None):

    """
    Determine whether or not an input message conforms to the Benthos spec.

    Example:

        >>> import json
        >>> with open('Messages.json') as i_f:
        ...     with open('Logfile') as l_f:
        ...         print(validate_messages((json.loads(msg) for msg in i_f), err=l_f))

    Parameters
    ----------
    msg : iter
        An iterable producing one AIS message as a dictionary every iteration.
    err : file, optional
        File-like object where failed messages are written.

    Returns
    -------
    bool
        True if every message passes
    """

    return_val = True

    for msg in messages:
        msg_type = msg['type']
        for field in MSG_TYPE_FIELDS[msg_type]:
            if not MSG_VALIDATION_LOOKUP[field]['test'](msg[field]):
                if err is not None:
                    err.write("Field `%s' failed: %s" % (field, json.dumps(msg) + os.linesep))
                return_val = False

    return return_val


class TestValidateMessages(unittest.TestCase):

    def test_all_types(self):
        for msg_type, fields in MSG_TYPE_FIELDS.items():
            good_message = {f: MSG_VALIDATION_LOOKUP[f]['good'] for f in fields}
            good_message['type'] = msg_type
            bad_message = {f: MSG_VALIDATION_LOOKUP[f]['bad'] for f in fields}
            bad_message['type'] = msg_type
            from StringIO import StringIO
            e = StringIO()
            print(validate_messages([good_message], err=e))
            e.seek(0)
            print(e.read())
            return
            assert validate_messages([good_message])
            assert not validate_messages([bad_message])


if __name__ == '__main__':
    sys.exit(unittest.main())
