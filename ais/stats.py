#!/usr/bin/env python

import argparse
import datetime
import collections
import logging
import pprint
import sys

from ais import nmea_queue

logger = logging.getLogger('libais')


class TrackRange(object):

  def __init__(self):
    self.min = None
    self.max = None

  def AddValues(self, *values):
    print ('AddValues', values)
    values = [v for v in values if v is not None]
    print ('AV3 ', values, self.min, self.max)
    if not len(values):
      raise ValueError('Must specify at least 1 value.')
    if self.min is None:
      self.min = min(values)
      self.max = max(values)
      return
    self.min = min(self.min, *values)
    self.max = max(self.max, *values)


class Stats(object):

  def __init__(self):
    self.counts = collections.Counter()
    self.queue = nmea_queue.NmeaQueue()
    self.time_range = TrackRange()
    self.time_delta_range = TrackRange()

  def AddFile(self, iterable, filename=None):
    self.counts['files'] += 1

    for line in iterable:
      self.AddLine(line)

  def AddLine(self, line):
    print(line.rstrip())
    self.counts['lines'] += 1
    self.queue.put(line)
    msg = self.queue.GetOrNone()
    if not msg:
      return

    # logging.info('stats found msg: %s', msg)
    # print ()
    # pprint.pprint(msg)
    self.counts[msg['line_type']] += 1
    if 'decoded' in msg:
      decoded = msg['decoded']
      if 'id' in decoded:
        self.counts['msg_VDM_%s' % decoded['id']] += 1
      if 'msg' in decoded:
        self.counts['msg_%s' % decoded['msg']] += 1

    if 'times' in msg:
      times = [t for t in msg['times'] if t is not None]
      if times:
        if self.time_range.min is None:
          self.time_range.AddValues(*times)
          # self.time_delta_range.AddValues(msg['times'])
        else:
          # print (self.time_range.min, self.time_range.max)
          time_delta = max(times) - self.time_range.max
          self.time_delta_range.AddValues(time_delta)
          self.time_range.AddValues(*times)


  def PrintSummary(self):
    pprint.pprint(self.counts)

    logger.info('time_range: [%s to %s]',
                 self.time_range.min,
                 self.time_range.max)

    logger.info('%s', datetime.datetime.utcfromtimestamp(self.time_range.min))
    logger.info('%s', datetime.datetime.utcfromtimestamp(self.time_range.max))

    logger.info('time_delta_range: [%s to %s]',
                 self.time_delta_range.min,
                 self.time_delta_range.max)



def main():
  logger.setLevel(logging.INFO)
  logger.info('in main')

  parser = argparse.ArgumentParser()
  parser.add_argument('filenames', type=str, nargs='+', help='NMEA files')
  args = parser.parse_args()
  logger.info('args: %s', args)

  stats = Stats()
  for filename in args.filenames:
      stats.AddFile(open(filename), filename)

  stats.PrintSummary()
