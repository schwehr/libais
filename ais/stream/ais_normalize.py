#!/usr/bin/env python
"""Normalize AIS messages so that each message takes exactly one line.

Hopefully should be able to handle streams with multiple receivers if they have
proper "r" or "b" tagged station names.  -t is needed for the USCG feed
for some reason.

@todo: report on the number of message fragments that got dropped
@todo: allow for a single receiver and no uscg station

@note: errors mostly seem to be comming from station r00370003 in Puerto Rico
@bug: The code currently thinks that all parts must be received in the same timestamp (second)
@todo: Allow the parts to be separated by one (or two?) seconds for the messages that go over timestamp boundaries between parts.

@todo: Use the normalize class!
"""
from optparse import OptionParser
import sys
import traceback

from nmea.checksum import isChecksumValid,checksumStr # Needed for checksums

def assembleAisNmeaMessages(infile=sys.stdin,
                            outfile=sys.stdout,
                            uscg=True,
                            validateChecksum=True,
                            verbose=True,
                            allowUnknown=False,
                            window=2,
                            treatABequal=False,
                            pass_invalid_checksums=False,
                            allow_missing_timestamps=False):
  """Put together messages.

  Args:
    infile: file stream like object to read from
    outfile: some object that can take write messages for output
    window: number of seconds to allow the later parts of a multiline message to span
  """
  o = outfile

  if not uscg:
    print 'Need to make a faster version that does not worry about the extra args and stations dict'
    assert False

  buffers = {}  # Put partial messages in a queue by station so that they can be reassembled
  line_num = 0
  invalid_checksums = 0

  for line in infile:
    try:
      line = line.strip() + '\n'  # Get rid of DOS issues.
      line_num += 1
      if len(line) < 7 or line[3:6] not in ('VDM|VDO'):
        o.write(line)  # Pass non AIS wireless message straight through.
        continue

      if validateChecksum and not isChecksumValid(line):
        invalid_checksums += 1
        print >> sys.stderr,'ERROR: Invalid checksum on line ',line_num
        print >> sys.stderr,'\t"%s"' % (line.strip(),)
        if not pass_invalid_checksums:
          continue

      fields = line.split(',')

      if len(fields) < 6:
        sys.stderr.write('ERROR line ' + str(line_num) + ': not enough fields in line...\n')
        sys.stderr.write('  Found ' + str(len(fields)) + ', but need at least 6\n')
        sys.stderr.write('  ' + line)
        continue

      totNumSentences = int(fields[1]) # Total nmea lines that compose this message 1..9 (numPackets)
      if 1 == totNumSentences:  # Easy case
        o.write (line)
        continue

      sentenceNum = int(fields[2])  # Message sequence number 1..9 (packetNum)
      payload = fields[5]  # AIS binary data encoded in whacky ways
      timestamp = fields[-1].strip()  # Seconds since Epoch UTC.  Always the last field

      station = None  # USCG receive stations.
      for i in range(len(fields)-1,5,-1):
        if 0 < len(fields[i]) and fields[i][0] in ('r','b'):
          station = fields[i]
          break  # Found it so ditch the for loop

      if None == station and options.allowUnknown:
        station = 'UNKNOWN'

      if None == station:
        sys.stderr.write('ERROR line ' + str(line_num) + ': No station found... skipping\n')
        sys.stderr.write('  ' + line)
        continue

      if treatABequal:
        bufferSlot = station + fields[3]  # seqId and Channel make a unique stream
      else:
        bufferSlot = station + fields[3] + fields[4]  # seqId and Channel make a unique stream

      newPacket = (payload,station,timestamp)
      if sentenceNum == 1:
        buffers[bufferSlot] = [newPacket] # Overwrite any partials
        continue

      if totNumSentences == sentenceNum:
          # Finished a message
          if bufferSlot not in buffers:
            if verbose:
              print 'Do not have the preceeding packets for line'
              print '  ',line
            continue
          buffers[bufferSlot].append(newPacket)
          parts = buffers[bufferSlot]  # Now have all the pieces.
          del buffers[bufferSlot]  # Clear out used packets to save memory.

          # Sanity check
          ok = True
          ts1 = None
          for part in parts:
              try:
                ts1 = float(part[2])
                ts2 = float(timestamp)
              except ValueError:
                if allow_missing_timestamps:
                  ts1 = ts2 = 0
                else:
                  ok = False
                  break
              if ts1 > ts2+window or ts1 < ts2-window:
                sys.stderr.write('ERROR: timestamps not all the same for ' + str(timestamp) + '\n')
                sys.stderr.write('  ** ' + line + '\n')
                sys.stderr.write('  parts:' + str(parts) + '\n')
                ok = False
                break
          if not ok:
            continue

          payload = ''.join([p[0] for p in parts])

          # Try to mirror the packet as much as possible... same seqId and channel
          checksumed_str = ','.join((fields[0],'1,1',fields[3],fields[4],payload, fields[6].split('*')[0] + '*'))
          if ts1 == 0:
            # allowed missing timestamp and it is missing
            if len(fields[7:-1]) == 0:
              out_str = checksumed_str + checksumStr(checksumed_str)
            else:
              out_str = checksumed_str + checksumStr(checksumed_str) + ',' + ','.join(fields[7:-1])
          else:
            out_str = checksumed_str + checksumStr(checksumed_str) + ',' + ','.join(fields[7:])

          if not isChecksumValid(out_str):
            print >> sys.stderr, 'ERROR: Invalid checksum in constructed 1 liner:\t',line,
            print >> sys.stderr, 'Checksum expected:',checksumStr(checksumed_str)
          o.write(out_str.strip() + '\n')  # FIX: Why do I have to do this last strip?

          continue

      buffers[bufferSlot].append(newPacket)
    except Exception, inst:
      # Catch all exceptions
      sys.stderr.write('ERROR... some exception for this line:\n')
      sys.stderr.write('\t' + line.strip() + '\n')
      sys.stderr.write(str(type(inst)) + '\n')
      sys.stderr.write( str(inst) + '\n')
      traceback.print_exc(file=sys.stderr)


  print >> sys.stderr, 'invalid checksums found...\t%d\t(out of %d)' % (invalid_checksums,line_num)


if __name__=='__main__':
  parser = OptionParser(usage="%prog [options] file1.ais [file2.ais ...]")

  parser.add_option('-a','--allow-unknown',dest='allowUnknown'
                    ,default=False
                    ,action='store_true'
                    ,help='Allow unknown station messages (adds ",sUNKNOWN")')

  parser.add_option('-t','--treat-ab-euqal',dest='treatABequal'
                    ,default=False
                    ,action='store_true'
                    ,help='Treat message parts in A and B as in the same channel.')


  parser.add_option('-o','--outfile',dest='outFile',default=None,#default='normalized.ais',
                    help='Name of the AIS file to write [default: stdout]')

  parser.add_option('-w','--window',dest='window'
                     ,type='int'
                     ,default=2
                     ,help='Amount of time that training nmea strings can be considered for joining [default: %default]')

  parser.add_option('-T','--allow-missing-timestamps', default=False, action='store_true',
                    help='Process lines without timestamps.  [default: drop lines without timestampes]')

  # Perhaps making the checksum valid is not he best idea
  parser.add_option('-p','--pass-invalid-checksums',default=False, action='store_true',
                    help = 'Pass messages with invalid checksums.  Multiline messages will get n valid checksum')

  parser.add_option('-v','--verbose',dest='verbose',default=False,action='store_true',
          help='Make the output verbose')

  (options,args) = parser.parse_args()

  out = sys.stdout
  if options.outFile != None:
      out = file(options.outFile,'w')

  if len(args)==0:
    assembleAisNmeaMessages(
        sys.stdin,
        out,
        allowUnknown=options.allowUnknown,
        window=options.window,
        treatABequal = options.treatABequal,
        pass_invalid_checksums=options.pass_invalid_checksums,
        allow_missing_timestamps = options.allow_missing_timestamps,
        )

  for filename in args:
    if options.verbose:
      sys.stderr.write('processing file: ' + filename + '\n')

    assembleAisNmeaMessages(
        file(filename),
        out,
        allowUnknown=options.allowUnknown,
        window=options.window,
        treatABequal = options.treatABequal,
        pass_invalid_checksums=options.pass_invalid_checksums,
        allow_missing_timestamps = options.allow_missing_timestamps,
        )
