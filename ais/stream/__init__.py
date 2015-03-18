# Based on ais_normalize.py from https://github.com/schwehr/noaadata

import sys
import traceback

import ais
from ais.stream import checksum


def ErrorPrinter(e):
  sys.stderr.write('%s\n' % e)


# def ErrorRaiser(e):
#   raise e


class StreamError(Exception):

  description = 'Stream error'

  def __init__(self, **kw):
    self.kw = kw
    self.kw['description'] = self.description

  def __str__(self):
    return '%(description)s: %(line_num)s: %(line)s' % self.kw


class InvalidChecksumError(StreamError):
  description = 'Invalid checksum'


class InvalidChecksumInConstructedError(StreamError):
  description = 'Invalid checksum in constructed one-liner'


class NoStationFoundError(StreamError):
  description = 'No station found'


class TooFewFieldsError(StreamError):
  description = 'Too few fields'

  def __str__(self):
    return '%(description)s, got %(fields)s but needed 6: %(line_num)s: %(line)s' % self.kw


class DifferingTimestampsError(StreamError):
  description = 'Timestamps not all the same'

  def __str__(self):
    return '%(description)s for %(timestamp)s: %(line_num)s: %(line)s, parts: %(parts)s' % self.kw


def normalize(nmea=sys.stdin,
              uscg=True,
              validateChecksum=True,
              verbose=False,
              allowUnknown=False,
              window=2,
              treatABequal=False,
              pass_invalid_checksums=False,
              allow_missing_timestamps=False,
              errorcb=ErrorPrinter):
  """Assemble multi-line messages

  Args:
    nmea: iterator of lines to read
    window: number of seconds to allow the later parts of a multiline message to span
  """
  if not uscg:
    print 'Need to make a faster version that does not worry about the extra args and stations dict'
    assert False

  buffers = {} # Put partial messages in a queue by station so that they can be reassembled
  line_num = 0
  invalid_checksums = 0

  for idx, line in enumerate(nmea):
    try:
      line = line.strip() + '\n'  # Get rid of DOS issues.
      line_num += 1
      if len(line) < 7 or line[3:6] not in ('VDM', 'VDO'):
        yield line
        continue

      if validateChecksum and not checksum.isChecksumValid(line):
        invalid_checksums += 1
        errorcb(InvalidChecksumError(line_num=line_num, line=line.strip()))
        if not pass_invalid_checksums:
          continue

      fields = line.split(',')

      if len(fields) < 6:
        errorcb(TooFewFieldsError(line_num=line_num,
                                  line=line.strip(),
                                  fields=len(fields)))
        continue

      # Total NMEA lines that compose this message [1..9].
      totNumSentences = int(fields[1])
      if 1 == totNumSentences:
        # A single line needs no work, so pass it along.
        yield line
        continue

      sentenceNum = int(fields[2])  # Message sequence number 1..9 (packetNum)
      payload = fields[5]  # AIS binary data encoded in whacky ways
      timestamp = fields[-1].strip()   # Seconds since Epoch UTC.  Always the last field

      station = None  # USCG Receive Stations        #if None==station:
      for i in range(len(fields)-1, 5, -1):
        if len(fields[i]) and fields[i][0] in ('r', 'b'):
          station = fields[i]
          break  # Found it so ditch the for loop.

      if station is None and options.allowUnknown:
        station = 'UNKNOWN'

      if station is None:
        errorcb(NoStationFoundError(line_num=line_num, line=line.strip()))
        continue

      if treatABequal:
        bufferSlot = station + fields[3]  # seqId and Channel make a unique stream
      else:
        bufferSlot = station + fields[3] + fields[4]  # seqId and Channel make a unique stream

      newPacket = payload, station, timestamp
      if sentenceNum == 1:
        buffers[bufferSlot] = [newPacket]  # Overwrite any partials
        continue

      if totNumSentences == sentenceNum:
        # Finished a message
        if bufferSlot not in buffers:
          if verbose:
            print 'Do not have the preceeding packets for line'
            print '  ', line
          continue
        buffers[bufferSlot].append(newPacket)
        parts = buffers[bufferSlot]  # Now have all the pieces.
        del buffers[bufferSlot]  # Clear out the used packets to save memory.

        # Sanity check
        ok = True
        ts1 = None
        for part in parts:
          try:
            ts1 = float(part[2])
            ts2 = float(timestamp)
          except ValueError:
            if allow_missing_timestamps:
              ts1 = 0
              ts2 = 0
            else:
              ok = False
              break
          if ts1 > ts2+window or ts1 < ts2-window:
            errorcb(DifferingTimestampsError(line_num=line_num,
                                             line=line.strip(),
                                             timestamp=timestamp, parts=parts))
            ok = False
            break
        if not ok:
          continue

        payload = ''.join([p[0] for p in parts])

        # Try to mirror the packet as much as possible... same seqId and channel.
        checksumed_str = ','.join((fields[0], '1,1', fields[3], fields[4],
                                   payload, fields[6].split('*')[0]+'*'))
        if ts1 == 0:
            # allowed missing timestamp and it is missing
            if len(fields[7:-1]) == 0:
              out_str = checksumed_str + checksum.checksumStr(checksumed_str)
            else:
              out_str = checksumed_str + checksum.checksumStr(checksumed_str) +',' + ','.join(fields[7:-1])
        else:
          out_str = checksumed_str + checksum.checksumStr(checksumed_str) +',' + ','.join(fields[7:])

        if not checksum.isChecksumValid(out_str):
          errorcb(InvalidChecksumInConstructedError(line_num=line_num, line=line.strip()))
        yield out_str.strip()+'\n'  # FIX: Why do I have to do this last strip?

        continue

      buffers[bufferSlot].append(newPacket)
    except Exception, inst:
      errorcb(inst)


def decode(nmea=sys.stdin,
           errorcb=ErrorPrinter,
           keep_nmea=False,
           **kw):
  """Decodes a stream of AIS messages. Takes the same arguments as normalize."""

  for line in normalize(nmea=nmea, errorcb=errorcb, **kw):
    body = ''.join(line.split(',')[5])
    pad = int(line.split('*')[0][-1])
    try:
      res = ais.decode(body, pad)
      if keep_nmea:
        res['nmea'] = line
      yield res
    except Exception, e:
      errorcb(e)
