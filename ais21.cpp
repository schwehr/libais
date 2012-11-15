// Msg 21 - ATON status

#include "ais.h"

Ais21::Ais21(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad)  {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(message_id == 21);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  // TODO(schwehr): make this more careful than 272-360
  if (num_bits < 272 || num_bits > 360) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<360> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  aton_type = ubits(bs, 38, 5);
  name = ais_str(bs, 43, 120);
  position_accuracy = bs[163];
  x = sbits(bs, 164, 28) / 600000.;
  y = sbits(bs, 192, 27) / 600000.;
  dim_a = ubits(bs, 219, 9);
  dim_b = ubits(bs, 228, 9);
  dim_c = ubits(bs, 237, 6);
  dim_d = ubits(bs, 243, 6);
  fix_type = ubits(bs, 249, 4);
  timestamp = ubits(bs, 253, 6);
  off_pos = bs[259];
  aton_status = ubits(bs, 260, 8);
  raim = bs[268];
  virtual_aton = bs[269];
  assigned_mode = bs[270];
  spare = bs[271];

  const size_t extra_total_bits = num_bits - 272;
  const size_t extra_chars = extra_total_bits / 6;
  const size_t extra_char_bits = extra_chars * 6;
  const size_t extra_bits = extra_total_bits % 6;

  if (extra_chars > 0) {
    name += ais_str(bs, 272, extra_char_bits);
  }

  if (extra_bits > 0) {
    spare2 = ubits(bs, 272 + extra_char_bits, extra_bits);
  } else {
    spare2 = 0;
  }

  status = AIS_OK;
}
