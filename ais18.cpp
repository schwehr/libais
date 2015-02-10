// Class B position report - 18 "B"

#include "ais.h"

Ais18::Ais18(const char *nmea_payload, const size_t pad)
    : AisMsg(nmea_payload, pad), spare(0), sog(0.0), position_accuracy(0),
      cog(0.0), true_heading(0), timestamp(0), spare2(0), unit_flag(0),
      display_flag(0), dsc_flag(0), band_flag(0), m22_flag(0), raim(false),
      commstate_flag(0), sync_state(0), slot_timeout(0), received_stations(0),
      received_stations_valid(false), slot_number(0), slot_number_valid(0),
      utc_valid(false), utc_hour(0), utc_min(0), utc_spare(0), slot_offset(0),
      slot_offset_valid(false), slot_increment(0), slot_increment_valid(false),
      slots_to_allocate(0), slots_to_allocate_valid(false), keep_flag(0),
      keep_flag_valid(false) {

  assert(message_id == 18);

  if (pad != 0 || num_chars != 28) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  AisBitset bs;
  const AIS_STATUS r = bs.ParseNmeaPayload(nmea_payload, pad);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  bs.SeekTo(38);
  spare = bs.ToUnsignedInt(38, 8);
  sog = bs.ToUnsignedInt(46, 10) / 10.;

  position_accuracy = bs[56];
  x = bs.ToInt(57, 28) / 600000.;
  y = bs.ToInt(85, 27) / 600000.;

  cog = bs.ToUnsignedInt(112, 12) / 10.;
  true_heading = bs.ToUnsignedInt(124, 9);
  timestamp = bs.ToUnsignedInt(133, 6);
  spare2 = bs.ToUnsignedInt(139, 2);
  unit_flag = bs[141];
  display_flag = bs[142];
  dsc_flag = bs[143];
  band_flag = bs[144];
  m22_flag = bs[145];
  mode_flag = bs[146];
  raim = bs[147];
  commstate_flag = bs[148];  // 0 SOTDMA, 1 ITDMA

  received_stations_valid = slot_number_valid = utc_valid = false;
  slot_offset_valid = slot_increment_valid = slots_to_allocate_valid = false;
  keep_flag = false;

  // if unit_flag is 1:
  // CS - carrier sense - fixed commstate payload of 1100000000000000110
  // TODO(schwehr): What if commstate is not 393222?
  // commstate = bs.ToUnsignedInt(149, 19);
  if (unit_flag == 1) {
    status = AIS_OK;
    return;
  }

  // unit_flag is 0
  sync_state = bs.ToUnsignedInt(149, 2);
  if (commstate_flag == 0) {
    // SOTDMA
    slot_timeout = bs.ToUnsignedInt(151, 3);

    switch (slot_timeout) {
    case 0:
      slot_offset = bs.ToUnsignedInt(154, 14);
      slot_offset_valid = true;
      break;
    case 1:
      utc_hour = bs.ToUnsignedInt(154, 5);
      utc_min = bs.ToUnsignedInt(159, 7);
      utc_spare = bs.ToUnsignedInt(166, 2);
      utc_valid = true;
      break;
    case 2:  // FALLTHROUGH
    case 4:  // FALLTHROUGH
    case 6:
      slot_number = bs.ToUnsignedInt(154, 14);
      slot_number_valid = true;
      break;
    case 3:  // FALLTHROUGH
    case 5:  // FALLTHROUGH
    case 7:
      received_stations = bs.ToUnsignedInt(154, 14);
      received_stations_valid = true;
      break;
    default:
      assert(false);
    }

  } else {
    // ITDMA
    slot_increment = bs.ToUnsignedInt(151, 13);
    slot_increment_valid = true;

    slots_to_allocate = bs.ToUnsignedInt(164, 3);
    slots_to_allocate_valid = true;

    keep_flag = bs[167];
    keep_flag_valid = true;
  }

  status = AIS_OK;
}
