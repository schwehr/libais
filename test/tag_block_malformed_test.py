"""Recorded malformed input must pass through the TAG queue without stopping it."""
from ais import tag_block


MISSING_CHECKSUM = (
    r'\g:1-2-94\g:2-2-9470,n:11222*1D\$SAVSI,D13MN-PS-MTEBS1,5,141030.377988,1139,-49,66*58')
BAD_MULTIPART = [
    r'10/07/19 17:36:42:075 \s:*49\g:1-2-11262,n:235196,s:r17MDUT1,c:1570469798*49\!AIVDM,2,1,9,A,55Na3b400001L@W;3GTu84<hF22000000000000N0p:,0*78',
    r'10/07/19 17:36:42:075 \s:*49\g:2-2-11262,n:235197*18\!AIVDM,2,2,9,A,44t0Ht:000000000000000000000,,0*41',
]
VALID = (r'\n:773486,s:b2003669950,c:1428451549*2C\!SAVDM,1,1,,B,K8VSqb9LdU28WP7l,0*48')


def test_missing_checksum_is_not_a_valid_tag():
  assert tag_block.Parse(MISSING_CHECKSUM) is None


def test_missing_checksum_passes_through_and_next_message_decodes():
  q = tag_block.TagQueue()
  q.put(MISSING_CHECKSUM)
  q.put(VALID)
  assert q.get_nowait() == {'lines': [MISSING_CHECKSUM], 'line_nums': [1]}
  assert q.get_nowait()['decoded']['mmsi'] == 577305000
  assert q.empty()


def test_malformed_multipart_passes_through_and_releases_group():
  q = tag_block.TagQueue()
  for line in BAD_MULTIPART:
    q.put(line)
  entry = q.get_nowait()
  assert entry['lines'] == BAD_MULTIPART
  assert entry['line_nums'] == [1, 2]
  assert 'decoded' not in entry
  assert not q.groups
  q.put(VALID)
  assert q.get_nowait()['decoded']['mmsi'] == 577305000
  assert q.empty()
