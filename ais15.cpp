// ? - Interrogation

#include "ais.h"

Ais15::Ais15(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(message_id == 15);

  const int num_char = std::strlen(nmea_payload);
  // TODO(schwehr): verify the pad makes sense for each size
  if (num_char != 15 && num_char != 18 && num_char != 27) {
    // 88-160 bits
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<162> bs;  // 160 / 6 = 26.66
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  repeat_indicator = ubits(bs, 6, 2);
  mmsi = ubits(bs, 8, 30);
  spare = ubits(bs, 38, 2);
  mmsi_1 = ubits(bs, 40, 30);  // Destination ID 1
  msg_1_1 = ubits(bs, 70, 6);
  slot_offset_1_1 = ubits(bs, 76, 12);

  // TODO(schwehr): set remaining fields to -1
  if (num_char <= 15) {
    status = AIS_OK;
    return;
  }

  spare2 = ubits(bs, 88, 2);
  dest_msg_1_2 = ubits(bs, 90, 6);
  slot_offset_1_2 = ubits(bs, 96, 12);

  // TODO(schwehr): set remaining fields to -1
  if (num_char <= 18) {
    status = AIS_OK;
    return;
  }

  spare3 = ubits(bs, 108, 2);
  mmsi_2 = ubits(bs, 110, 30);
  msg_2 = ubits(bs, 140, 6);
  slot_offset_2 = ubits(bs, 146, 12);
  spare4 = ubits(bs, 158, 2);

  status = AIS_OK;
}
