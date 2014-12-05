// Since 2010-May-14

#include "ais.h"

Ais9::Ais9(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(message_id == 9);

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

  alt = ubits(bs, 38, 12);
  sog = ubits(bs, 50, 10) / 10.;

  position_accuracy = bs[60];
  x = sbits(bs, 61, 28) / 600000.;
  y = sbits(bs, 89, 27) / 600000.;

  cog = ubits(bs, 116, 12) / 10.;
  timestamp = ubits(bs, 128, 6);
  alt_sensor = bs[134];
  spare = ubits(bs, 135, 7);
  dte = bs[142];
  spare2 = ubits(bs, 143, 3);
  assigned_mode = bs[146];
  raim = bs[147];
  commstate_flag = bs[148];  // 0 SOTDMA, 1 ITDMA

  sync_state = ubits(bs, 149, 2);

#ifndef NDEBUG
  slot_timeout = -1;
  received_stations = slot_number = utc_hour = utc_min = utc_spare -1;
  slot_offset = slot_increment = slots_to_allocate = -1;
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

  if (0 == commstate_flag) {
    // SOTDMA
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
    slot_increment = ubits(bs, 151, 13);
    slot_increment_valid = true;

    slots_to_allocate = ubits(bs, 164, 3);
    slots_to_allocate_valid = true;

    keep_flag = bs[167];
    keep_flag_valid = true;
  }

  status = AIS_OK;
}
