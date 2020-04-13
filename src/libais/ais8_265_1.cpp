// STM Route Exchange

#include <cmath>

#include "ais.h"

namespace libais {

Ais8_265_1::Ais8_265_1(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad), month(0) {
  assert(dac == 265);
  assert(fi == 1);

  if (!CheckStatus()) {
    return;
  }

  month = 100;

  status = AIS_OK;
}


}  // namespace libais
