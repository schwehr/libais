// Class B static data report.  Can be one of 4 different parts.  Only
// A and B defined for ITU 1371-3

#include "ais.h"

Ais24::Ais24(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(message_id == 24);

  const int num_bits = strlen(nmea_payload) * 6 - pad;
  if (num_bits != 160 && num_bits != 168) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<168> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  part_num = ubits(bs, 38, 2);

  switch (part_num) {
  case 0:  // Part A
    if (num_bits != 160) {
      status = AIS_ERR_BAD_BIT_COUNT;
      return;
    }
    name = ais_str(bs, 40, 120);
    type_and_cargo = -1;
    dim_a = dim_b = dim_c = dim_d = spare = -1;
    break;
  case 1:  // Part B
    if (num_bits != 168) {
      status = AIS_ERR_BAD_BIT_COUNT;
      return;
    }
    type_and_cargo = ubits(bs, 40, 8);
    vendor_id = ais_str(bs, 48, 42);
    callsign = ais_str(bs, 90, 42);
    dim_a = ubits(bs, 132, 9);
    dim_b = ubits(bs, 141, 9);
    dim_c = ubits(bs, 150, 6);
    dim_d = ubits(bs, 156, 6);
    spare = ubits(bs, 162, 6);
    break;
  case 2:  // FALLTHROUGH - not yet defined by ITU
  case 3:  // FALLTHROUGH - not yet defined by ITU
  default:
    status = AIS_ERR_BAD_MSG_CONTENT;
    return;
  }

  status = AIS_OK;
}
