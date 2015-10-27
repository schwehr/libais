"""
A file-like interface to NMEA parsing.

>>> import ais
>>> import json
>>> with ais.open('infile.nmea') as src, open('outfile.json', 'w') as dst:
...   for msg in src:
...      dst.write(json.dumps(msg))
"""

import codecs
import sys

import six

import ais.nmea_queue


def open(name, mode='r', **kwargs):
  """Open a file containing NMEA and instantiate an instance of `NmeaFile()`.
  Lke Python's `open()`, use 'r' for reading.  Writing and updating are not
  supported.

  Args:
    name: A file path, file-like object, or '-' for stdin.
    mode: Open file in this mode for reading.

  Raises:
    TypeError: Invalid object for name parameter.
    ValueError: Invalid value for mode parameter.

  Returns:
    An instance of NmeaFile that is ready for reading.
  """

  if 'r' not in mode:
    raise ValueError("Only read modes are supported.")

  if name == '-':
    fobj = sys.stdin
  elif isinstance(name, six.string_types):
    fobj = codecs.open(name, **kwargs)
  elif hasattr(name, 'close') and \
          (hasattr(name, 'next') or hasattr(name, '__next__')):
    fobj = name
  else:
    raise TypeError("'name' must be a file path, file-like object, "
                    "or '-' for stdin.")

  return NmeaFile(fobj)


class NmeaFile(object):

  """A file-like object for parsing and reading NMEA data."""

  def __init__(self, fobj):
    """Construct a parsing stream.

    Args:
      fobj: File-like object.
    """

    self._fobj = fobj
    self._queue = ais.nmea_queue.NmeaQueue()

  @property
  def closed(self):
    """Is the file-like object from which we are reading open for reading?"""
    return self._fobj.closed

  @property
  def name(self):
    """Name of the file-like object from which we are reading."""
    return self._fobj.name

  def close(self):
    """Close the file-like object from which we are reading and dump whats left
    in the queue."""
    return self._fobj.close()

  def __iter__(self):
    return self

  def __enter__(self):
    return self

  def __exit__(self, exc_type, exc_val, exc_tb):
    # Destroy the queue, which could be a large in-memory object.
    self._queue = None
    return self.close()

  def __next__(self):
    """Return the next decoded AIS message."""
    msg = None
    while not msg:
        self._queue.put(next(self._fobj))
        msg = self._queue.GetOrNone()
    return msg

  next = __next__
