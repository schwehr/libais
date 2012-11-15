// Since Apr 2010

#include <cmath>

#include "ais.h"

using std::abs;

Ais1_2_3::Ais1_2_3(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(message_id >= 1 && message_id <= 3);

  if (pad != 0 || strlen(nmea_payload) != 28) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<168> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  nav_status = ubits(bs, 38, 4);

  rot_raw = sbits(bs, 42, 8);
  rot_over_range = abs(rot_raw) > 126 ? true : false;
  rot = pow((rot_raw/4.733), 2);
  if (rot_raw < 0) rot = -rot;

  sog = ubits(bs, 50, 10) / 10.;
  position_accuracy = bs[60];
  x = sbits(bs, 61, 28) / 600000.;
  y = sbits(bs, 89, 27) / 600000.;
  cog = ubits(bs, 116, 12) / 10.;
  true_heading = ubits(bs, 128, 9);
  timestamp = ubits(bs, 137, 6);
  special_manoeuvre = ubits(bs, 143, 2);
  spare = ubits(bs, 145, 3);
  raim = bs[148];

  sync_state = ubits(bs, 149, 2);

  // Set all to invalid.  This way we don't have to track it in multiple places.
  received_stations = -1;
  received_stations_valid = false;
  slot_number = -1;
  slot_number_valid = false;
  utc_hour = utc_min = -1;
  utc_valid = false;
  utc_spare = -1;
  slot_offset = -1;
  slot_offset_valid = false;

  slot_increment = -1;
  slot_increment_valid = false;
  slots_to_allocate = -1;
  slots_to_allocate_valid = false;
  keep_flag = false;
  keep_flag_valid = false;

  if (1 == message_id || 2 == message_id) {
    slot_timeout = ubits(bs, 151, 3);
    slot_timeout_valid = true;

    switch (slot_timeout) {
    case 0:
      slot_offset = ubits(bs, 154, 14);
      slot_offset_valid = true;
      break;
    case 1:
      utc_hour = ubits(bs, 154, 5);
      utc_min = ubits(bs, 159, 7);
      utc_spare = ubits(bs, 166, 2);
      utc_valid = true;
      break;
    case 2:  // FALLTHROUGH
    case 4:  // FALLTHROUGH
    case 6:
      slot_number = ubits(bs, 154, 14);
      slot_number_valid = true;
      break;
    case 3:  // FALLTHROUGH
    case 5:  // FALLTHROUGH
    case 7:
      received_stations = ubits(bs, 154, 14);
      received_stations_valid = true;
      break;
    default:
      assert(false);
    }
  } else {
    // ITDMA
    assert(3 == message_id);
    slot_increment = ubits(bs, 151, 13);
    slot_increment_valid = true;

    slots_to_allocate = ubits(bs, 164, 3);
    slots_to_allocate_valid = true;

    keep_flag = bs[167];
    keep_flag_valid = true;
  }

  status = AIS_OK;
}

ostream& operator<< (ostream &o, const Ais1_2_3 &msg) {
  return o << msg.message_id << ": " << msg.mmsi;
}
