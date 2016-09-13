// Test parsing message 18 "B" - Class B.

// TODO(schwehr): Decode these.  They might not be valid, but they were
// received in the wild.

// !AIVDM,1,1,,B,:T0Vd`,3*2A
// !AIVDM,1,1,,B,:1pwe20<m>FB,0*25
// !AIVDM,1,1,,B,:A6P?cNiukR`,0*6F
// !AIVDM,1,1,,B,:WHeWAEwGWgw,0*41
// !AIVDM,1,1,,B,:iCn44crw>GI,0*0D
// !AIVDM,1,1,,B,:lM>=N?JwNM?,0*4D
// !SAVDM,1,1,,B,:5@SW:19pedP,0*67
// !AIVDM,1,1,,B,:8MEJAmIknK;5cm5:ogsp,3*49
// !AIVDM,1,1,,A,:RFLT2OO>ITV@VUC4IRHpavl,5*3E
// !AIVDM,1,1,,A,:J`=1W5qKhACoa<7:hhk<k=wP,5*14
// !AIVDM,1,1,,B,:CFlQi3vqjM8;W98tPnSK`g<W9cqE;h<tP,1*12

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais18> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais18> msg(new Ais18(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(const Ais18 *msg, const int repeat_indicator, const int mmsi,
              const int spare, const float sog, const int position_accuracy,
              const double x, const double y, const float cog,
              const int true_heading, const int timestamp, const int spare2,
              const int unit_flag, const int display_flag, const int dsc_flag,
              const int band_flag, const int m22_flag, const int mode_flag,
              const bool raim) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(18, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(spare, msg->spare);
  EXPECT_FLOAT_EQ(sog, msg->sog);
  EXPECT_EQ(position_accuracy, msg->position_accuracy);
  EXPECT_DOUBLE_EQ(x, msg->position.lng_deg);
  EXPECT_DOUBLE_EQ(y, msg->position.lat_deg);
  EXPECT_FLOAT_EQ(cog, msg->cog);
  EXPECT_EQ(true_heading, msg->true_heading);
  EXPECT_EQ(timestamp, msg->timestamp);
  EXPECT_EQ(spare2, msg->spare2);
  EXPECT_EQ(unit_flag, msg->unit_flag);
  EXPECT_EQ(display_flag, msg->display_flag);
  EXPECT_EQ(dsc_flag, msg->dsc_flag);
  EXPECT_EQ(band_flag, msg->band_flag);
  EXPECT_EQ(m22_flag, msg->m22_flag);
  EXPECT_EQ(mode_flag, msg->mode_flag);
  EXPECT_EQ(raim, msg->raim);
}

void ValidateCommState(
    const Ais18 *msg, const int sync_state, const bool slot_timeout_valid,
    const int slot_timeout, const bool received_stations_valid,
    const int received_stations, const bool slot_number_valid,
    const int slot_number, const bool utc_valid, const int utc_hour,
    const int utc_min, const int utc_spare, const bool slot_offset_valid,
    const int slot_offset, const bool slot_increment_valid,
    const int slot_increment, const bool slots_to_allocate_valid,
    const int slots_to_allocate, const bool keep_flag_valid,
    const bool keep_flag, const bool commstate_cs_fill_valid,
    const int commstate_cs_fill) {
  EXPECT_EQ(sync_state, msg->sync_state);

  EXPECT_EQ(slot_timeout_valid, msg->slot_timeout_valid);
  if (slot_timeout_valid) {
    EXPECT_EQ(slot_timeout, msg->slot_timeout);
  }

  EXPECT_EQ(received_stations_valid, msg->received_stations_valid);
  if (received_stations_valid) {
    EXPECT_EQ(received_stations, msg->received_stations);
  }

  EXPECT_EQ(slot_number_valid, msg->slot_number_valid);
  if (slot_number_valid) {
    EXPECT_EQ(slot_number, msg->slot_number);
  }

  EXPECT_EQ(utc_valid, msg->utc_valid);
  if (utc_valid) {
    EXPECT_EQ(utc_hour, msg->utc_hour);
    EXPECT_EQ(utc_min, msg->utc_min);
    EXPECT_EQ(utc_spare, msg->utc_spare);
  }

  EXPECT_EQ(slot_offset_valid, msg->slot_offset_valid);
  if (slot_offset_valid) {
    EXPECT_EQ(slot_offset, msg->slot_offset);
  }

  EXPECT_EQ(slot_increment_valid, msg->slot_increment_valid);
  if (slot_increment_valid) {
    EXPECT_EQ(slot_increment, msg->slot_increment);
  }

  EXPECT_EQ(slots_to_allocate_valid, msg->slots_to_allocate_valid);
  if (slots_to_allocate_valid) {
    EXPECT_EQ(slots_to_allocate, msg->slots_to_allocate);
  }

  EXPECT_EQ(keep_flag_valid, msg->keep_flag_valid);
  if (keep_flag_valid) {
    EXPECT_EQ(keep_flag, msg->keep_flag);
  }

  EXPECT_EQ(commstate_cs_fill_valid, msg->commstate_cs_fill_valid);
  if (commstate_cs_fill_valid) {
    EXPECT_EQ(commstate_cs_fill, msg->commstate_cs_fill);
  }
}

TEST(Ais18Test, DecodeAnything) {
  std::unique_ptr<Ais18> msg =
      Init("!SAVDM,1,1,4,B,B5NU=J000=l0BD6l590EkwuUoP06,0*61");

  Validate(msg.get(), 0, 367611240, 0, 0.0, 1, -122.32996,
           47.631599999999999, 34.8, 511, 59, 0, 1,  // unit_flag
           0, 1, 1, 1, 0, true);

  ValidateCommState(msg.get(),
                    0,               // sync state
                    false, 0,        // slot timeout
                    false, 0,        // received_stations
                    false, 0,        // slot_numbe
                    false, 0, 0, 0,  // utc
                    false, 0,        // slot_offset
                    false, 0,        // slot inc
                    false, 0,        // slots to alloc
                    false, 0,        // keep
                    true, 393222);  // cs fill
}

}  // namespace
}  // namespace libais
