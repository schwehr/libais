// < - ASRM

#include "ais.h"

Ais12::Ais12(const char *nmea_payload, const size_t pad) : AisMsg(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;
#ifndef NDEBUG
  if (12 != message_id) {
    status = AIS_ERR_WRONG_MSG_TYPE;
    return;
  }
#endif
  // WARNING: the spec says max 1008 bits, but 168 + 4*256 => 1192 bits or 199 characters
  const size_t num_bits = strlen(nmea_payload) * 6 - pad;
  if (num_bits < 72 || num_bits > 1192)  { status = AIS_ERR_BAD_BIT_COUNT; return; }

  bitset<MAX_BITS> bs;  // Spec says 1008
  {
    const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
    if (r != AIS_OK) {
      status = r;
      return;
    }
  }

  seq_num = ubits(bs, 38, 2);
  dest_mmsi = ubits(bs, 40, 30);
  retransmitted = bs[70];
  spare = bs[71];
  const int num_txt = (num_bits - 72) / 6;
  const int num_txt_bits = num_txt * 6;
  text = ais_str(bs, 72, num_txt_bits);
  // TODO(schwehr): watch for trailing spares

  status = AIS_OK;
}
