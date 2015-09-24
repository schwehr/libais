"""
Objects for reading and parsing NMEA from a file.
"""


import codecs
import sys

import six

import ais.nmea_queue


def open(name):

    """
    Open a file containing NMEA and instantiate an instance of `NMEAFile()`.

    Args:
      name: A file path, file-like object, or '-' for stdin.
    """

    if name == '-':
      fobj = sys.stdin
    elif isinstance(name, six.string_types):
      fobj = codecs.open(name, encoding='utf-8')
    elif hasattr(name, 'close') and (
                hasattr(name, 'next') or hasattr(name, '__next__')):
      fobj = name
    else:
      raise TypeError("'name' must be a file path, file-like object, "
                      "or '-' for stdin.")

    return NMEAFile(fobj)


class NMEAFile(object):

    """A file-like object for reading and parsing NMEA data."""

    def __init__(self, f):

      """
      Construct a parsing stream.

      Args:
        f: File-like object.
      """

      self._f = f
      self._queue = ais.nmea_queue.NmeaQueue()

    @property
    def closed(self):
      """Is the file-like object from which we are reading open for reading?"""
      return self._f.closed

    @property
    def name(self):
      """Name of the file-like object from which we are reading."""
      return self._f.name

    def close(self):
      """Close the file-like object from which we are reading."""
      return self._f.close()

    def __iter__(self):
      return self

    def __enter__(self):
      return self

    def __exit__(self, exc_type, exc_val, exc_tb):
      return self.close()

    def __next__(self):
      """Return the next decoded AIS message."""
      msg = None
      while not msg:
          self._queue.put(next(self._f))
          msg = self._queue.get()
      return msg

    next = __next__
