"""Decode complete environmental reports through the public Python binding."""
import json

import ais
import pytest


def uint(value, width):
  return format(value, '0%db' % width)


def report(kind, site, data=None):
  return (uint(kind, 4) + uint(25, 5) + uint(14, 5) + uint(37, 6) +
          uint(site, 7) + (data if data is not None else '0' * 85))


def decode(*reports):
  # ITU message 8 header, DAC 1 / FI 26, followed by 112-bit sensor reports.
  bits = (uint(8, 6) + uint(0, 2) + uint(123456789, 30) + uint(0, 2) +
          uint(1, 10) + uint(26, 6) + ''.join(reports))
  pad = -len(bits) % 6
  bits += '0' * pad
  values = [int(bits[i:i + 6], 2) for i in range(0, len(bits), 6)]
  payload = ''.join(chr(v + 48 + (8 if v >= 40 else 0)) for v in values)
  return ais.decode(payload, pad)


@pytest.mark.parametrize('kind', range(11))
def test_every_sensor_report_has_identity_and_time(kind):
  sensor = decode(report(kind, 42))['reports'][0]
  assert sensor.get('report_type') == kind
  assert sensor.get('site_id') == 42
  assert sensor.get('utc_day') == 25
  assert sensor.get('utc_hr') == 14
  assert sensor.get('utc_min') == 37


def test_multiple_current_reports_retain_their_own_samples():
  # Two independent 2D current profiles must not overwrite one another.
  first = (uint(12, 8) + uint(90, 9) + uint(5, 9)) * 3 + uint(2, 3) + '0000'
  second = (uint(23, 8) + uint(180, 9) + uint(10, 9)) * 3 + uint(3, 3) + '0000'
  decoded = decode(report(4, 1, first), report(4, 2, second))
  profiles = decoded['reports']
  assert len(profiles) == 2
  assert profiles[0].get('currents') is not None
  assert profiles[1].get('currents') is not None
  assert profiles[0]['currents'] == [{'speed': pytest.approx(1.2), 'dir': 90, 'depth': 5}] * 3
  assert profiles[1]['currents'] == [{'speed': pytest.approx(2.3), 'dir': 180, 'depth': 10}] * 3
  assert 'currents' not in decoded


@pytest.mark.parametrize('kind', [5, 6])
def test_two_sample_current_reports_are_complete_and_serializable(kind):
  if kind == 5:
    sample = uint(10, 8) + uint(20, 8) + uint(5, 8) + uint(7, 9)
    data = sample * 2 + uint(1, 3) + '0' * 16
    expected = {'north': 1.0, 'east': 2.0, 'up': 0.5, 'depth': 7}
  else:
    sample = uint(90, 9) + uint(12, 7) + uint(15, 8) + uint(180, 9) + uint(8, 9)
    data = sample * 2 + '0'
    expected = {'bearing': 90, 'dist': 12, 'speed': 1.5, 'dir': 180, 'level': 8}
  decoded = decode(report(kind, 4, data))
  sensor = decoded['reports'][0]
  assert sensor.get('currents') is not None
  assert len(sensor['currents']) == 2
  assert sensor['currents'] == [expected, expected]
  assert json.loads(json.dumps(decoded))['reports'][0]['currents'] == [expected, expected]
