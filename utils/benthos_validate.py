#!/usr/bin/env python


"""
Validator to ensure libais can produce Benthos's expected input
"""


import json
import os
import unittest
import sys


# In order to ease test maintenance as outputs and inputs change the data structure below contains a test for every
# field, a value that will pass the test, and a value that will fail the test.  All information is pulled from GPSD
# (http://catb.org/gpsd/AIVDM.html) and assumes two things:
#   1. Fieldnames are unique across all messages
#   2. Fields appearing in different message types contain the same information.  For instance, the field 'sog'
#      appears in multiple types but is always speed over ground in the same units in every message.
#
# Keys are fieldnames and values are dictionaries containing three keys:
#   1. test - a function that verifies a value is acceptable for this field
#   2. good - a value that will pass the test function
#   3. bad  - a value that will not pass the test function
#
# Some fields are flags with values 0 or 1 that may be switched to Python's bool in the future but in the meantime
# we want to be positive these values are int's.  Since bool subclasses int expressions like `0 in (1, 0)` and
# `True in (0, 1)` both evaluate as `True` which could yield unexpected results.  Any test that expects an int
# also checks to make sure that int is NOT a bool, even if the field is a range and will never be bool.  Better to be
# safe here than be forced to debug some potentially ambiguous bugs elsewhere.
MSG_VALIDATION_LOOKUP = {

    # Comments about where fields are pulled from just reference where the description was found in the GPSD
    # specification http://catb.org/gpsd/AIVDM.html
    # They're not critical but if there is a discrepancy between what is expected and how the test is written
    # it may help us figure out why.

    # Pulled from types 1, 2, and 3 GPSD spec
    'type': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in range(1, 28),
        'good': 5,
        'bad': -1
    },
    'repeat': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and 0 <= x <= 2 ** 2,
        'good': 4,
        'bad': -1
    },
    'mmsi': {
        'test': lambda x: isinstance(x, str) and len(x) <= 2 ** 30,
        'good': '12345678902345678',
        'bad': 1234
    },
    'status': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in range(0, 16),
        'good': 2,
        'bad': -10
    },
    'turn': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in range(-127, 129),
        'good': 125,
        'bad': -1000
    },
    'sog': {
        'test': lambda x: isinstance(x, float) and 0 <= x <= 102.2 or x == 1022,
        'good': 1022,
        'bad': 103
    },
    'accuracy': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in (0, 1),
        'good': 0,
        'bad': 2,
    },
    'lat': {
        'test': lambda x: isinstance(x, float) and -90 <= x <= 90 or x == 91,
        'good': 91,
        'bad': -100
    },
    'lon': {
        'test': lambda x: isinstance(x, float) and -180 <= x <= 180 or x == 181,  # TODO: Should -180 be a valid value?  Maybe `-180 < x` instead?
        'good': 181,
        'bad': -180.1
    },
    'course': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and 0 <= x <= 359 or x == 3600,  # TODO: Should -90 be a valid value?  Maybe `-90 < x` instead?
        'good': 3600,
        'bad': 360
    },
    'heading': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and 0 <= x <= 359 or x == 511,
        'good': 511,
        'bad': -102
    },
    'second': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in range(0, 64),
        'good': 63,
        'bad': 64
    },
    'maneuver': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in (1, 2),
        'good': 2,
        'bad': 3
    },
    'raim': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in (0, 1),  # TODO: bool is more Pythonic if the field is actually boolean and not state
        'good': 0,
        'bad': -2
    },
    'radio': {
        'test': lambda x: x is None,  # TODO: What will this value be?
        'good': None,
        'bad': False
    },

    # Pulled from type 5 GPSD spec
    'ais_version': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in (0, 1, 2, 3),  # TODO: Should always be 0 right now.  The other vals are reserved.
        'good': 2,
        'bad': True
    },
    'imo': {
        'test': lambda x: isinstance(x, str) and len(x) <= 2 ** 30,
        'good': 'value',
        'bad': True,
    },
    'callsign': {
        'test': lambda x: isinstance(x, str) and len(x) <= 2 ** 42,
        'good': 'good',
        'bad': json
    },
    'shipname': {
        'test': lambda x: isinstance(x, str) and len(x) <= 2 ** 120,
        'good': 'good value',
        'bad': False
    },
    'shiptype': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in range(0, 100),
        'good': 87,
        'bad': str
    },
    'to_bow': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and 0 <= x <= 2 ** 9,
        'good': 1,
        'bad': -1
    },
    'to_stern': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and 0 <= x <= 2 ** 9,
        'good': 0,
        'bad': tuple
    },
    'to_port': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and 0 <= x <= 2 ** 6,
        'good': 1,
        'bad': -34
    },
    'to_starboard': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and 0 <= x <= 2 ** 6,
        'good': 0,
        'bad': False,
    },
    'epfd': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in range(0, 9),
        'good': 8,
        'bad': 10
    },
    'month': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in range(0, 13),
        'good': 12,
        'bad': 'asdf'
    },
    'day': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in range(0, 32),
        'good': 1,
        'bad': ':'
    },
    'hour': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in range(0, 25),
        'good': 23,
        'bad': ','
    },
    'minute': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in range(0, 61),
        'good': 54,
        'bad': '*'
    },
    'draught': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and 0 < x <= 2 ** 8,
        'good': 1,
        'bad': 2 ** 8 + 1
    },
    'destination': {
        'test': lambda x: isinstance(x, str) and len(x) <= 2 ** 120,
        'good': 'v',
        'bad': 2 * 2
    },
    'dte': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in (0, 1),  # TODO: Switch to a more Pythonic bool if this is actually bolean and not a status
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
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x <= 2 ** 10,
        'good': 0,
        'bad': str
    },
    'cs': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in (0, 1),  # Not bool - state
        'good': 0,
        'bad': 7
    },
    'display': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in (0, 1),  # Not bool - state
        'good': 1,
        'bad': 'j'
    },
    'dsc': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in (0, 1),  # TODO: Switch to a more Pythonic bool?
        'good': 1,
        'bad': -45
    },
    'band': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in (0, 1),  # TODO: Switch to a more Pythonic bool?
        'good': 0,
        'bad': 4
    },
    'msg22': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in (0, 1),  # TODO: Switch to a more Pythonic bool?
        'good': 0,
        'bad': -2
    },
    'assigned': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in (0, 1),  # TODO: Switch to a more Pythonic bool?
        'good': 1,
        'bad': -33
    },

    # All type 19 are covered between 1, 2, 3, and 18

    # Pulled from type 24 GPSD spec
    'partno': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in (0, 1),
        'good': 0,
        'bad': -1
    },
    'vendorid': {
        'test': lambda x: isinstance(x, str) and len(x) <= 2 ** 18,
        'good': 'this is a gooooooooood value',
        'bad': int
    },
    'model': {
        'test': lambda x: isinstance(x, str) and len(x) <= 2 ** 4,
        'good': 'something',
        'bad': 333
    },
    'serial': {
        'test': lambda x: isinstance(x, str) and len(x) <= 2 ** 20,
        'good': 'pawoeiras',
        'bad': -1
    },
    'mothership_mmsi': {
        'test': lambda x: isinstance(x, str) and len(x) <= 2 ** 30,
        'good': 'done ... finally ...',
        'bad': -200
    },

    # Pulled from type 27 GPSD spec
    'gnss': {
        'test': lambda x: isinstance(x, int) and not isinstance(x, bool) and x in (0, 1),  # Not bool - state
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
    27: [
        'type', 'repeat', 'mmsi', 'accuracy', 'raim', 'status', 'lon', 'lat', 'speed', 'course', 'gnss'
    ]
}


def validate_messages(messages, err=None):

    """
    Determine whether or not an input message conforms to the Benthos spec.

    Example:

        >>> import json
        >>> with open('Messages.json') as infile:
        ...     with open('Logfile') as logfile:
        ...         print(validate_messages(
        ...             (json.loads(msg) for msg in infile), err=logfile))

    Parameters
    ----------
    msg : iter
        An iterable producing one AIS message as a dictionary every iteration.
    err : file, optional
        File-like object where errors are logged and failed messages are written.
        A message with multiple invalid fields will have multiple errors in this
        file.

    Returns
    -------
    bool
        True if every message passes
    """

    return_val = True

    for msg in messages:

        # Make sure the message specifies its type and that the type is one we can validate
        if 'type' not in msg or msg['type'] not in MSG_TYPE_FIELDS:
            if err is not None:
                err.write("No 'type' key in msg or type is invalid or not testable: %s" % msg)
            return_val = False

        # Normal field validation
        else:
            msg_type = msg['type']
            for field in MSG_TYPE_FIELDS[msg_type]:
                if not MSG_VALIDATION_LOOKUP[field]['test'](msg[field]):
                    if err is not None:
                        sys.stdout.write("Field `%s' failed: %s" % (field, json.dumps(msg) + os.linesep))
                    return_val = False

    return return_val


class TestValidateMessages(unittest.TestCase):

    def test_all_types(self):
        for msg_type, msg_fields in MSG_TYPE_FIELDS.items():

            # Check type field individually since the other tests force it to be correct
            assert not validate_messages([{'field': 'val'}])
            assert not validate_messages([{'type': MSG_VALIDATION_LOOKUP['type']['bad']}])

            # Construct a good message
            good_message = {f: MSG_VALIDATION_LOOKUP[f]['good'] for f in msg_fields}
            good_message['type'] = msg_type

            assert validate_messages([good_message]), \
                "Supposed 'good' msg failed validation: %s" % good_message

            # Creating a bad message from all of the bad values is an insufficient test because the validator
            # will start checking fields and as soon as it gets to a bad one it will flag the message as invalid.
            # Every field is checked in every message and every bad field is logged but we can't validate individual
            # fields without taking a good message and then changing one field at a time to a bad field.
            for field in msg_fields:
                if field != 'type':
                    bad_message = good_message.copy()
                    bad_message[field] = MSG_VALIDATION_LOOKUP[field]['bad']
                    assert not validate_messages([bad_message]), \
                        "Field `%s' should have caused message to fail: %s" % (field, bad_message)


if __name__ == '__main__':
    if len(sys.argv) > 1 and sys.argv[1].lower() == 'test':
        del sys.argv[1]
        sys.exit(unittest.main())
