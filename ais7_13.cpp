// ACK to ABM or safety ABM

#include "ais.h"

Ais7_13::Ais7_13(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(message_id == 7 || message_id == 13);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (((num_bits - 40) % 32) != 0 || num_bits > 168) {
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
  const size_t num_acks = (num_bits - 40) / 32;
  for (size_t i = 0; i < num_acks; i++) {
    dest_mmsi.push_back(ubits(bs, 40 + i*32, 30));
    seq_num.push_back(ubits(bs, 40 + i*32 + 30, 2));
  }

  status = AIS_OK;
}
