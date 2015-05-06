"""Manage a stream of NMEA messages with optional metadata.

TODO(schwehr): Add support for decoding non-AIS NMEA messages (e.g. ZDA).
"""

from ais import nmea
from ais import tag_block
from ais import uscg
from ais import vdm
import six.moves.queue as Queue


class Error(Exception):
  pass


def GetOrNone(queue):
  try:
    return queue.get(block=False)
  except Queue.Empty:
    return


class NmeaQueue(Queue.Queue):
  # pylint: disable=line-too-long
  r"""Process mixed text, bare NMEA or NMEA with TAG BLOCK or USCG metadata.

  Raw text lines are passed through without a decode stage.  "Bare"
  AIS NMEA lines are decoded and passed through without a decode or
  time field in resulting decodes.  The USCG and TAG Block metadata
  messages will get a list of times and a "decoded" field.  The
  decoded field contains the AIS message contents.  The extra metadata is
  expected to be accessed through the matches field.

  Example usage:

    q = nmea_queue.NmeaQueue()
    q.qsize()  # Returns 0

    # Raw text.
    q.put('Hello world')
    q.qsize()  # Returns 1
    q.get()  # Returns a dictionary with lines containing the input.
    q.qsize()  # Returns 0

    # Bare NMEA AIS VDM.
    q.put('!SAVDM,2,1,9,B,55NG9T000001L@GKCSPLDlTpV22222222222220t2PI5540Ht00`88888888,0*33')
    q.put('!SAVDM,2,2,9,B,88888888880,2*34')
    q.get()  # Returns a dictionary with no times or extra metadata in matches.

    # USCG NMEA AIS VDM.
    q.put('!SAVDM,2,1,9,B,55NG9T000001L@GKCSPLDlTpV22222222222220t2PI5540Ht00`88888888,0*33,b003669709,1428944320')
    q.put('!SAVDM,2,2,9,B,88888888880,2*34,b003669709,1428944320')
    q.get()  # Returns a dictionary with times and extra metadata.

    # TAG Block NMEA AIS VDM.
    q.put('\g:1-3-144865,n:325372,s:r11CSDO1,c:1428944580*6C\!AIVDM,2,1,9,A,55MkD8000001L@7?37L4AHDqAE8F0Pu8p8huLE8t28P,0*1E')
    q.put('\g:2-3-144865,n:325373*2D\!AIVDM,2,2,9,A,666v<04ThC`12AAkp0hH88888880,2*7A')
    q.get()  # Return a dictionary with times and extra metadata.


  This sample line is a single line TAG Block position report intended
  for a satellite:

    \n:80677,s:b003669952,c:1428884269*2A\!SAVDM,1,1,,B,K8VSqb9LdU28WP8<,0*17

  A queue get() returns:

    {'decoded': {
            'cog': 131,
            'gnss': True,
            'id': 27,
            'md5': '50898a3435865cf76f1b502b2821672b',
            'mmsi': 577305000,
            'nav_status': 5,
            'position_accuracy': 1,
            'raim': False,
            'repeat_indicator': 0,
            'sog': 0,
            'spare': 0,
            'x': -90.2066650390625,
            'y': 29.145000457763672},
         'line_nums': [9],
         'line_type': ('TAGB',),
         'lines': [
             '\\n:80677,s:b003669952,c:1428884269*2A'
             '\\!SAVDM,1,1,,B,K8VSqb9LdU28WP8<,0*17'],
         'matches': [{
             'dest': None,
             'group': None,
             'group_id': None,
             'line_num': '80677',
             'metadata': 'n:80677,s:b003669952,c:1428884269*2A',
             'payload': '!SAVDM,1,1,,B,K8VSqb9LdU28WP8<,0*17',
             'quality': None,
             'rcvr': 'b003669952',
             'rel_time': None,
             'sentence_num': None,
             'sentence_tot': None,
             'tag_checksum': '2A',
             'text': None,
             'text_date': None,
             'time': '1428884269'}],
         'times': [1428884269]})
  """

  def __init__(self):
    self.bare_queue = vdm.BareQueue()
    self.tagb_queue = tag_block.TagQueue()
    self.uscg_queue = uscg.UscgQueue()
    self.line_num = 0
    Queue.Queue.__init__(self)

  def put(self, line, line_num=None):
    """Add a line to the queue.

    Args:
      line: str, Text to add to the queue.
      line_num: int, Optional line number override.
    """
    if line_num is not None:
      self.line_num = line_num
    else:
      self.line_num += 1
      line_num = self.line_num

    line = line.rstrip()
    line_type = nmea.LineType(line)

    msg = None

    if line_type == nmea.TEXT:
      msg = {'line_nums': [self.line_num], 'lines': [line]}
    elif line_type == nmea.BARE:
      self.bare_queue.put(line, self.line_num)
      msg = GetOrNone(self.bare_queue)
    elif line_type == nmea.TAGB:
      self.tagb_queue.put(line, self.line_num)
      msg = GetOrNone(self.tagb_queue)
    elif line_type == nmea.USCG:
      self.uscg_queue.put(line, self.line_num)
      msg = GetOrNone(self.uscg_queue)
    else:
      assert False  # Should never reach here.

    if msg:
      msg['line_type'] = line_type
      Queue.Queue.put(self, msg)

  def GetOrNone(self):
    try:
      return self.get(block=False)
    except Queue.Empty:
      return
