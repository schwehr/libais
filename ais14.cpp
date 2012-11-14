// Safety related broadcast message (SRBM)

#include "ais.h"

Ais14::Ais14(const char *nmea_payload, const size_t pad) : AisMsg(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;
#ifndef NDEBUG
  if (14 != message_id) {
    status = AIS_ERR_WRONG_MSG_TYPE;
    return;
  }
#endif
  const int num_bits = strlen(nmea_payload) * 6 - pad;
  if (num_bits < 46 || num_bits > 1008) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<1008> bs;  // 424 + 2 spare bits => 71 characters
  {
    const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
    if (r != AIS_OK) {
      status = r;
      return;
    }
  }

  spare = ubits(bs, 38, 2);

  // TODO(schwehr): fix processing of spare bits if any
  const int num_char = (num_bits - 40) / 6;
  text = ais_str(bs, 40, num_char * 6);
  expected_num_spare_bits = num_bits - 40 - num_char*60;  // Can use this to check later

  status = AIS_OK;
}
