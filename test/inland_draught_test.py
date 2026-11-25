"""Inland AIS 8:200:10 draught is in centimetres, unlike length and beam."""
import ais
import pytest


@pytest.mark.parametrize('centimetres', [0, 1, 123, 2000])
def test_inland_draught_in_metres(centimetres):
  # Modify only bits 144..154 in the existing inland-message test fixture.
  payload = '85NLn@0j2d<8000000BhI?`50000'
  bits = ''.join(format(ord(c) - 48 - (8 if ord(c) > 87 else 0), '06b') for c in payload)
  bits = bits[:144] + format(centimetres, '011b') + bits[155:]
  values = [int(bits[i:i + 6], 2) for i in range(0, len(bits), 6)]
  encoded = ''.join(chr(v + 48 + (8 if v >= 40 else 0)) for v in values)
  result = ais.decode(encoded, 0)
  assert result['draught'] == pytest.approx(centimetres / 100.0)
  assert result['length'] == 15
  assert result['beam'] == 5
  assert result['mmsi'] == 367474240
