// @ - Assigned mode command

#include "ais.h"

Ais16::Ais16(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(message_id == 16);

  // 96 or 144 bits
  // 168 bits violates the spec but is common
  // TODO(schwehr): check the pad
  const int num_char = std::strlen(nmea_payload);
  if (num_char != 16 && num_char != 24 && num_char != 28) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<168> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  spare = ubits(bs, 38, 2);

  dest_mmsi_a = ubits(bs, 40, 30);
  offset_a = ubits(bs, 70, 12);
  inc_a = ubits(bs, 82, 10);
  if (num_char == 16) {
    dest_mmsi_b = -1;
    offset_b = -1;
    inc_b = -1;
    spare2 = ubits(bs, 92, 4);

    status = AIS_OK;
    return;
  }

  dest_mmsi_b = ubits(bs, 92, 30);
  offset_b = ubits(bs, 122, 12);
  inc_b = ubits(bs, 134, 10);
  spare2 = -1;

  status = AIS_OK;
}
