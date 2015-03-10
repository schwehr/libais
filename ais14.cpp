// Safety related broadcast message (SRBM)

#include "ais.h"

namespace libais {

Ais14::Ais14(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), spare(0), expected_num_spare_bits(0) {

  assert(message_id == 14);

  if (num_bits < 46 || num_bits > 1008) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  AisBitset bs;  // 424 + 2 spare bits => 71 characters
  const AIS_STATUS r = bs.ParseNmeaPayload(nmea_payload, pad);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  bs.SeekTo(38);
  spare = bs.ToUnsignedInt(38, 2);

  const int num_char = (num_bits - 40) / 6;
  text = bs.ToString(40, num_char * 6);
  expected_num_spare_bits = num_bits - 40 - num_char*60;
  // TODO(schwehr): fix processing of spare bits if any

  status = AIS_OK;
}

}  // namespace libais
