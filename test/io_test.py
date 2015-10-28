"""Unittests for ais.io"""

import contextlib
import sys

import ais

import pytest
from six.moves import StringIO


def test_open_right_object(typeexamples_nmea_path):
    """Make sure ais.open() returns the right object"""
    with ais.open(typeexamples_nmea_path) as src:
        assert isinstance(src, ais.NmeaFile)
    # pytest places a different object in sys.stdin so do an old-shool
    # type comparison
    with ais.open('-') as src:
        assert isinstance(src._fobj, type(sys.stdin))


def test_open_path(typeexamples_nmea_path):
    """ais.open() with a file path on disk"""
    with ais.open(typeexamples_nmea_path) as src:
        for idx, line in enumerate(src):
            assert isinstance(line, dict)
        assert idx >= 20, "20 messages was kind of chosen arbitrarily.  " \
                          "Adjust if necessary."


def test_open_file_like_object(bare_nmea):
    """Make sure ais.open() can handle a file-like object."""
    with contextlib.closing(StringIO(bare_nmea)) as f, ais.open(f) as src:
        for idx, line in enumerate(src):
            assert isinstance(line, dict)
            print(line)
        assert idx >= 6


def test_open_exceptions():
    """Trigger the exceptions in ais.open()"""

    # Bad value for mode parameter
    with pytest.raises(ValueError):
        ais.open('something', mode='bad-mode')

    # Invalid file-like object
    with pytest.raises(TypeError):
        ais.open(None)


def test_NmeaFile_io_states(typeexamples_nmea_path):
    """Make sure NmeaFile is tearing down properly."""
    with ais.open(typeexamples_nmea_path) as src:
        assert not src.closed
        assert not src._fobj.closed
    assert src.closed
    assert src._fobj.closed


def test_NmeaFile_properties(typeexamples_nmea_path):
    with ais.open(typeexamples_nmea_path) as src:
        assert src.name == typeexamples_nmea_path
