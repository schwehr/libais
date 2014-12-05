// Msg 19 C - Extended Class B equipment position report

#include "ais.h"

Ais19::Ais19(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(message_id == 19);

  if (pad != 0 || strlen(nmea_payload) != 52) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<312> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  spare = ubits(bs, 38, 8);
  sog = ubits(bs, 46, 10) / 10.;

  position_accuracy = bs[56];
  x = sbits(bs, 57, 28) / 600000.;
  y = sbits(bs, 85, 27) / 600000.;

  cog = ubits(bs, 112, 12) / 10.;
  true_heading = ubits(bs, 124, 9);
  timestamp = ubits(bs, 133, 6);
  spare2 = ubits(bs, 139, 4);

  name = ais_str(bs, 143, 120);

  type_and_cargo = ubits(bs, 263, 8);
  dim_a = ubits(bs, 271, 9);
  dim_b = ubits(bs, 280, 9);
  dim_c = ubits(bs, 289, 6);
  dim_d = ubits(bs, 295, 6);

  fix_type = ubits(bs, 301, 4);
  raim = bs[305];
  dte = bs[306];
  assigned_mode = bs[307];
  spare3 = ubits(bs, 308, 4);

  status = AIS_OK;
}
