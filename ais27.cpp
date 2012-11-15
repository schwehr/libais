// 'K' - 27 - Long-range AIS broadcast message

#include "ais.h"

Ais27::Ais27(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(message_id == 27);

  const size_t num_bits = strlen(nmea_payload) * 6 - pad;

  if (pad != 0 || num_bits != 96) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<96> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  position_accuracy = bs[38];
  raim = bs[39];
  nav_status = ubits(bs, 40, 4);
  x = sbits(bs, 44, 18) / 600.;
  y = sbits(bs, 62, 17) / 600.;
  sog = ubits(bs, 79, 6);
  cog = ubits(bs, 85, 9);
  // 0 is a current GNSS position.  1 is NOT the current GNSS position
  gnss = !bs[94];
  spare = bs[95];

  status = AIS_OK;
}
