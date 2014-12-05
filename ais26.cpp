// 'J' - Multi slot binary message with comm state
// TODO(schwehr): handle payload
// BAD: the comm-state is after the veriable payload.  This is a bad design.

#include "ais.h"

Ais26::Ais26(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad)  {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(message_id == 26);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  // TODO(schwehr): check for off by one.
  const size_t comm_flag_offset = num_bits - 20 + 1;

  if (num_bits < 52 || num_bits > 1064) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<1064> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  const bool addressed = bs[38];
  use_app_id = bs[39];
  if (addressed) {
    dest_mmsi_valid = true;
    dest_mmsi = ubits(bs, 40, 30);
    if (use_app_id) {
      dac = ubits(bs, 70, 10);
      fi = ubits(bs, 80, 6);
    }
    // TODO(schwehr): deal with payload
  } else {
    dest_mmsi_valid = false;
    // broadcast
    if (use_app_id) {
      dac = ubits(bs, 40, 10);
      fi = ubits(bs, 50, 6);
    }
    // TODO(schwehr): Handle the payload.
  }

  commstate_flag = bs[comm_flag_offset];
  sync_state = ubits(bs, comm_flag_offset + 1, 2);  // Both SOTDMA and TDMA

#ifndef NDEBUG
  slot_timeout = -1;
  received_stations = -1;
  slot_number = -1;
  utc_hour = -1;
  utc_min = -1;
  utc_spare = -1;
  slot_offset = -1;
  slot_increment = -1;
  slots_to_allocate = -1;
  keep_flag = false;
#endif

  slot_timeout_valid = false;
  received_stations_valid = false;
  slot_number_valid = false;
  utc_valid = false;
  slot_offset_valid = false;
  slot_increment_valid = false;
  slots_to_allocate_valid = false;
  keep_flag_valid = false;

  if (!commstate_flag) {
    // SOTDMA
    slot_timeout = ubits(bs, comm_flag_offset + 3, 3);
    slot_timeout_valid = true;
    switch (slot_timeout) {
    case 0:
      slot_offset = ubits(bs, comm_flag_offset + 6, 14);
      slot_offset_valid = true;
      break;
    case 1:
      utc_hour = ubits(bs, comm_flag_offset + 6, 5);
      utc_min = ubits(bs, comm_flag_offset + 11, 7);
      utc_spare = ubits(bs, comm_flag_offset + 18, 2);
      utc_valid = true;
      break;
    case 2:  // FALLTHROUGH
    case 4:  // FALLTHROUGH
    case 6:
      slot_number = ubits(bs, comm_flag_offset + 6, 14);
      slot_number_valid = true;
      break;
    case 3:  // FALLTHROUGH
    case 5:  // FALLTHROUGH
    case 7:
      received_stations = ubits(bs, comm_flag_offset + 6, 14);
      received_stations_valid = true;
      break;
    default:
      assert(false);
    }
  } else {
    // ITDMA
    slot_increment = ubits(bs, comm_flag_offset + 3, 13);
    slot_increment_valid = true;

    slots_to_allocate = ubits(bs, comm_flag_offset + 16, 3);
    slots_to_allocate_valid = true;

    keep_flag = bs[comm_flag_offset + 19];
    keep_flag_valid = true;
  }
  status = AIS_OK;
}
