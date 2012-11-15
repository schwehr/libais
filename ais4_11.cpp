// AIS message 4 or 11

#include "ais.h"

Ais4_11::Ais4_11(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(message_id == 4 || message_id == 11);

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

  year = ubits(bs, 38, 14);
  month = ubits(bs, 52, 4);
  day = ubits(bs, 56, 5);
  hour = ubits(bs, 61, 5);
  minute = ubits(bs, 66, 6);
  second = ubits(bs, 72, 6);

  position_accuracy = bs[78];
  x = sbits(bs, 79, 28) / 600000.;
  y = sbits(bs, 107, 27) / 600000.;

  fix_type = ubits(bs, 134, 4);
  transmission_ctl = bs[138];
  spare = ubits(bs, 139, 9);
  raim = bs[148];

  //
  // SOTDMA commstate
  //

  // Set all to invalid - this way we don't have to track it in multiple places
  received_stations = -1;  received_stations_valid = false;
  slot_number = -1; slot_number_valid = false;
  utc_hour = utc_min = -1; utc_valid = false;
  slot_offset = -1; slot_offset_valid = false;

  sync_state = ubits(bs, 149, 2);
  slot_timeout = ubits(bs, 151, 3);

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

  status = AIS_OK;
}

ostream& operator<< (ostream &o, const Ais4_11 &msg) {
  return o << msg.message_id << ": " << msg.mmsi;
}
