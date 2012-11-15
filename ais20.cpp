// Msg 20 D - data link management

#include "ais.h"

Ais20::Ais20(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(message_id == 20);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  if (num_bits < 72 || num_bits > 160) {
    status = AIS_ERR_BAD_BIT_COUNT;  return;
  }

  bitset<162> bs;  // 160, but must be 6 bit aligned
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  spare = ubits(bs, 38, 2);

  offset_1 = ubits(bs, 40, 12);
  num_slots_1 = ubits(bs, 52, 4);
  timeout_1 = ubits(bs, 56, 3);
  incr_1 = ubits(bs, 59, 11);

  if (72 == num_bits) {
    group_valid_2 = group_valid_3 = group_valid_4 = false;
    spare2 = ubits(bs, 70, 2);
    status = AIS_OK;
    return;
  }

  group_valid_2 = true;
  offset_2 = ubits(bs, 70, 12);
  num_slots_2 = ubits(bs, 82, 4);
  timeout_2 = ubits(bs, 86, 3);
  incr_2 = ubits(bs, 89, 11);
  // 100 bits for the message
  // 104 is the next byte boundary
  // 108 is the next 6 bit boundary -> 18 characters
  if (num_bits >= 100 && num_bits <=108) {
    group_valid_3 = group_valid_4 = false;
    spare2 = ubits(bs, 100, 4);
    status = AIS_OK;
    return;
  }

  group_valid_3 = true;
  offset_3 = ubits(bs, 100, 12);
  num_slots_3 = ubits(bs, 112, 4);
  timeout_3 = ubits(bs, 116, 3);
  incr_3 = ubits(bs, 119, 11);
  // 130 bits for the message
  // 136 is the next byte boundary
  // 138 is the next 6 bit boundary -> 23 characters
  if (num_bits >= 130 && num_bits <= 138) {
    group_valid_4 = false;
    spare2 = ubits(bs, 130, 6);  // Makes the result 8 bit / 1 byte aligned
    status = AIS_OK;
    return;
  }

  group_valid_4 = true;
  offset_4 = ubits(bs, 130, 12);
  num_slots_4 = ubits(bs, 142, 4);
  timeout_4 = ubits(bs, 146, 3);
  incr_4 = ubits(bs, 149, 11);

  spare2 = 0;

  status = AIS_OK;
}
