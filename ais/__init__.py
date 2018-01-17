"""Bring the C++ extension into the ais namespace."""

import logging

logging.basicConfig()

from _ais import decode
from _ais import DecodeError
from ais.io import open
from ais.io import NmeaFile

__license__ = 'Apache 2.0'
__version__ = '0.17'
