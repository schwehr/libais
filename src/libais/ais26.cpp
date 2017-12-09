// 'J' - Multi slot binary message with comm state
// TODO(schwehr): handle payload
// BAD: the comm-state is after the veriable payload.  This is a bad design.

// See also: http://www.e-navigation.nl/asm

#include "ais.h"

namespace libais {

Ais26::Ais26(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), use_app_id(false), dest_mmsi_valid(false),
      dest_mmsi(0), dac(0), fi(0), commstate_flag(0), sync_state(0),
      slot_timeout_valid(false), slot_timeout(0),
      received_stations_valid(false), received_stations(0),
      slot_number_valid(false), slot_number(0),
      utc_valid(false), utc_hour(0), utc_min(0), utc_spare(0),
      slot_offset_valid(false),  slot_offset(0),
      slot_increment_valid(false), slot_increment(0),
      slots_to_allocate_valid(false), slots_to_allocate(0),
      keep_flag_valid(false), keep_flag(false) {
  if (!CheckStatus()) {
    return;
  }

  // TODO(schwehr): Check for off by one.
  const int comm_flag_offset = num_bits - 20;

  if (num_bits < 52 || num_bits > 1064) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }
  
  assert(message_id == 26);

  bits.SeekTo(38);

  using BitGet = AisBitset::BitGetter;
  using UIGet = AisBitset::UIntGetter;
  
  bool addressed = false;
  status = bits.GetValues(BitGet{&addressed,  38},
                          BitGet{&use_app_id, 39});
  if (!CheckStatus()) return;

  if (addressed) {
    dest_mmsi_valid = true;
    status = bits.GetValues(UIGet{&dest_mmsi, 40, 30});
    if (!CheckStatus()) return;
    
    if (use_app_id) {
      if (num_bits < 86) {
        status = AIS_ERR_BAD_BIT_COUNT;
        return;
      }
      status = bits.GetValues(UIGet{&dac, 70, 10},
                              UIGet{&fi,  80,  6});
      if (!CheckStatus()) return;
    }
    // TODO(schwehr): Handle the payload.
  } else {
    // broadcast
    if (use_app_id) {
      status = bits.GetValues(UIGet{&dac, 40, 10},
                              UIGet{&fi,  50,  6});
      if (!CheckStatus()) return;
    }
  }

  bits.SeekTo(comm_flag_offset);
  status = bits.GetValues(UIGet{&commstate_flag, comm_flag_offset,     1},
                          // SOTDMA and TDMA
                          UIGet{&sync_state,     comm_flag_offset + 1, 2});
  if (!CheckStatus()) return;

  if (!commstate_flag) {
    // SOTDMA
    slot_timeout_valid = true;
    status = bits.GetValues(UIGet{&slot_timeout, comm_flag_offset + 3, 3});
    if (!CheckStatus()) return;

    switch (slot_timeout) {
    case 0:
      slot_offset_valid = true;
      status = bits.GetValues(UIGet{&slot_offset, comm_flag_offset + 6, 14});
      if (!CheckStatus()) return;
      break;
    case 1:
      utc_valid = true;
      status = bits.GetValues(UIGet{&utc_hour,  comm_flag_offset +  6, 5},
                              UIGet{&utc_min,   comm_flag_offset + 11, 7},
                              UIGet{&utc_spare, comm_flag_offset + 18, 2});
      if (!CheckStatus()) return;
      break;
    case 2:  // FALLTHROUGH
    case 4:  // FALLTHROUGH
    case 6:
      slot_number_valid = true;
      status = bits.GetValues(UIGet{&slot_number, comm_flag_offset + 6, 14});
      if (!CheckStatus()) return;
      break;
    case 3:  // FALLTHROUGH
    case 5:  // FALLTHROUGH
    case 7:
      received_stations_valid = true;
      status = bits.GetValues(UIGet{&received_stations, comm_flag_offset + 6, 14});
      if (!CheckStatus()) return;
      break;
    default:
      assert(false);
    }
  } else {
    // ITDMA

    slot_increment_valid = true;
    slots_to_allocate_valid = true;
    keep_flag_valid = true;

    status = bits.GetValues(UIGet {&slot_increment,    comm_flag_offset +  3, 13},
                            UIGet {&slots_to_allocate, comm_flag_offset + 16,  3},
                            BitGet{&keep_flag,         comm_flag_offset + 19});
    if (!CheckStatus()) return;
  }

  // TODO(schwehr): Add assert(bits.GetRemaining() == 0);
  status = AIS_OK;
}

}  // namespace libais
