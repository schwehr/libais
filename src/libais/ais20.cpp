// Msg 20 D - data link management

#include "ais.h"

namespace libais{

Ais20::Ais20(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), spare(0), offset_1(0), num_slots_1(0),
      timeout_1(0), incr_1(0), group_valid_2(false), offset_2(0),
      num_slots_2(0), timeout_2(0), incr_2(0), group_valid_3(false),
      offset_3(0), num_slots_3(0), timeout_3(0), incr_3(0),
      group_valid_4(false), offset_4(0), num_slots_4(0), timeout_4(0),
      incr_4(0), spare2(0) {
  if (!CheckStatus()) {
    return;
  }
  if (num_bits < 72 || num_bits > 160) {
    status = AIS_ERR_BAD_BIT_COUNT;  return;
  }

  using UIGet = AisBitset::UIntGetter;

  // 160, but must be 6 bit aligned
  assert(message_id == 20);

  bits.SeekTo(38);
  status = bits.GetValues(UIGet{&spare,       38,  2},
                          UIGet{&offset_1,    40, 12},
                          UIGet{&num_slots_1, 52,  4},
                          UIGet{&timeout_1,   56,  3},
                          UIGet{&incr_1,      59, 11});
  if (!CheckStatus()) return;

  if (num_bits == 72) {
    status = bits.GetValues(AisBitset::UIntGetter{&spare2, 70, 2});
    if (!CheckStatus()) return;
      
    assert(bits.GetRemaining() == 0);
    status = AIS_OK;
    return;
  }

  group_valid_2 = true;
  status = bits.GetValues(UIGet{&offset_2,    70, 12},
                          UIGet{&num_slots_2, 82,  4},
                          UIGet{&timeout_2,   86,  3},
                          UIGet{&incr_2,      89, 11});
  if (!CheckStatus()) return;
  
  // 100 bits for the message
  // 104 is the next byte boundary
  // 108 is the next 6 bit boundary -> 18 characters
  if (num_bits >= 100 && num_bits <=108) {
    status = bits.GetValues(UIGet{&spare2, 100, bits.GetRemaining()});
    if (!CheckStatus()) return;
      
    status = AIS_OK;
    return;
  }

  group_valid_3 = true;
  status = bits.GetValues(UIGet{&offset_3,    100, 12},
                          UIGet{&num_slots_3, 112,  4},
                          UIGet{&timeout_3,   116,  3},
                          UIGet{&incr_3,      119, 11});
  if (!CheckStatus()) return;
  
  // 130 bits for the message
  // 136 is the next byte boundary
  // 138 is the next 6 bit boundary -> 23 characters
  if (num_bits >= 130 && num_bits <= 138) {
    // Makes the result 8 bit / 1 byte aligned.
    status = bits.GetValues(UIGet{&spare2, 130, bits.GetRemaining()});
    if (!CheckStatus()) return;
    
    status = AIS_OK;
    return;
  }

  group_valid_4 = true;
  status = bits.GetValues(UIGet{&offset_4,    130, 12},
                          UIGet{&num_slots_4, 142,  4},
                          UIGet{&timeout_4,   146,  3},
                          UIGet{&incr_4,      149, 11});
  if (!CheckStatus()) return;

  spare2 = 0;

  assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

}  // namespace libais
