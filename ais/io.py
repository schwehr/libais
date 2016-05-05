"""A file-like interface to NMEA parsing.

>>> import ais
>>> import json
>>> with ais.open('test/data/typeexamples.nmea') as src:
...     msg = next(src)
...     print(json.dumps(next(msg), indent=4, sort_keys=True))
{
    "decoded": {
        "day": 0,
        "fix_type": 1,
        "hour": 24,
        "id": 4,
        "md5": "7ecb187e7edc1789de436b0c2ccf2963",
        "minute": 60,
        "mmsi": 3669713,
        "month": 0,
        "position_accuracy": 0,
        "raim": false,
        "repeat_indicator": 0,
        "second": 60,
        "slot_number": 2105,
        "slot_timeout": 2,
        "spare": 0,
        "sync_state": 1,
        "transmission_ctl": 0,
        "x": 181.0,
        "y": 91.0,
        "year": 0
    },
    "line_nums": [
        1
    ],
    "line_type": "USCG",
    "lines": [
        "!AIVDM,1,1,,A,403Ovl@000Htt<tSF0l4Q@100`Pq,0*28,d-109,S2105,t050056.00,T56.13718694,r003669946,1325394060,1325394001"
    ],
    "matches": [
        {
            "body": "403Ovl@000Htt<tSF0l4Q@100`Pq",
            "chan": "A",
            "checksum": "28",
            "counter": null,
            "fill_bits": 0,
            "hour": 5,
            "minute": 0,
            "payload": "!AIVDM,1,1,,A,403Ovl@000Htt<tSF0l4Q@100`Pq,0*28",
            "receiver_time": 50056.0,
            "rssi": null,
            "second": 56.0,
            "sen_num": 1,
            "sen_tot": 1,
            "seq_id": null,
            "signal_strength": -109,
            "slot": 2105,
            "station": "r003669946",
            "station_type": "r",
            "talker": "AI",
            "time": 1325394060,
            "time_of_arrival": 56.13718694,
            "uscg_metadata": ",d-109,S2105,t050056.00,T56.13718694,r003669946,1325394060",
            "vdm": "!AIVDM,1,1,,A,403Ovl@000Htt<tSF0l4Q@100`Pq,0*28",
            "vdm_type": "VDM"
        }
    ]
}

"""

import codecs
import sys

import six

import ais.nmea_queue


def open(name, mode='r', **kwargs):
  """Open a file containing NMEA and instantiate an instance of `NmeaFile()`.
  Lke Python's `open()`, set the `mode` parameter to 'r' for normal reading or
  or 'rU' for opening the file in universal newline mode.

  Args:
    name: A file path, file-like object, or '-' for stdin.
    mode: I/O mode for opening the input file.  r, rU, or U

  Raises:
    TypeError: Invalid object for name parameter.
    ValueError: Invalid value for mode parameter.

  Returns:
    An instance of NmeaFile that is ready for reading.
  """

  io_modes = ('r', 'rU', 'U')

  if mode not in io_modes:
    raise ValueError("Mode '{m}' is unsupported.  Must be one of: {ms}".format(
      m=mode, ms=', '.join(io_modes)))

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
  """Provides a file-like object interface to the `ais.nmea_queue` module."""

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
