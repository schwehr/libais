// Test parsing of the Class A position report messages 1-3.

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

std::unique_ptr<Ais1_2_3> Init(const string &nmea_string) {
  const string body(GetBody(nmea_string));
  const int pad = GetPad(nmea_string);

  // TODO(schwehr): Switch to c++14 make_unique.
  std::unique_ptr<Ais1_2_3> msg(new Ais1_2_3(body.c_str(), pad));
  if (!msg || msg->had_error()) {
    return nullptr;
  }
  return msg;
}

void Validate(
    const Ais1_2_3 *msg,
    const int message_id,
    const int repeat_indicator,
    const int mmsi,
    const int nav_status,
    const int rot_raw,
    const float rot,
    const bool rot_over_range,
    const float sog,
    const int position_accuracy,
    const double x,
    const double y,
    const int true_heading,
    const int timestamp,
    const bool special_manoeuvre,
    const int spare,
    const bool raim) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(message_id, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(nav_status, msg->nav_status);
  EXPECT_EQ(rot_raw, msg->rot_raw);
  EXPECT_FLOAT_EQ(rot, msg->rot);
  EXPECT_EQ(rot_over_range, msg->rot_over_range);
  EXPECT_FLOAT_EQ(sog, msg->sog);
  EXPECT_EQ(position_accuracy, msg->position_accuracy);
  EXPECT_DOUBLE_EQ(x, msg->position.lng_deg);
  EXPECT_DOUBLE_EQ(y, msg->position.lat_deg);
  EXPECT_EQ(true_heading, msg->true_heading);
  EXPECT_EQ(timestamp, msg->timestamp);
  EXPECT_EQ(special_manoeuvre, msg->special_manoeuvre);
  EXPECT_EQ(spare, msg->spare);
  EXPECT_EQ(raim, msg->raim);
}

void ValidateCommState(
    const Ais1_2_3 *msg,
    const int sync_state,
    const bool slot_timeout_valid,
    const int slot_timeout,
    const bool received_stations_valid,
    const int received_stations,
    const bool slot_number_valid,
    const int slot_number,
    bool utc_valid,
    const int utc_hour,
    const int utc_min,
    const int utc_spare,
    const bool slot_offset_valid,
    const int slot_offset,
    const bool slot_increment_valid,
    const int slot_increment,
    const bool slots_to_allocate_valid,
    const int slots_to_allocate,
    const bool keep_flag_valid,
    const bool keep_flag) {
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
}

TEST(Ais123Test, Decode1ReceivedStations) {
  // Verifies decode of Msg 1 SOTDMA with received_stations.
  std::unique_ptr<Ais1_2_3> msg = Init(
      "!AIVDM,1,1,,A,100WhdhP0nJRdiFFHFvm??v00L12,0*13,raishub,1342569600");
  Validate(
      msg.get(), 1, 0, 651443, 0, -128, -731.386474609375, true, 5.4, 0,
      -76.30584833333333, 39.112951666666667, 511, 0, 0, 0, false);

  ValidateCommState(
      msg.get(), 0, true, 7, true, 66, false, 0, false, 0, 0, 0, false, 0,
      false, 0, false, 0, false, false);
}

TEST(Ais123Test, Decode1Utc) {
  // Verifies decode of Msg with UTC comm state.
  std::unique_ptr<Ais1_2_3> msg = Init(
      "!AIVDM,1,1,,B,181:Kjh01ewHFRPDK1s3IRcn06sd,0*08,raishub,1342569600");

  Validate(
      msg.get(), 1, 0, 538090443, 0, 0, 0.0, false, 10.8999996185, 1,
      -8.6611999999999991, 35.690633333333331, 85, 59, 0, 0, false);

  ValidateCommState(
      msg.get(), 0, true, 1, false, 0, false, 0, true, 23, 59, 0, false, 0,
      false, 0, false, 0, false, false);
}

TEST(Ais123Test, Decode1SlotNumber) {
  // Verifies decode of Msg with slot number.
  std::unique_ptr<Ais1_2_3> msg = Init(
      "!AIVDM,1,1,,B,16<qIn0P018MNT6=v:KMH?wf0@Ph,0*29,raishub,1342569600");

  Validate(
      msg.get(), 1, 0, 416176600, 0, -128, -731.386474609, true, 0.10000000149,
      0, 118.28757833333333, 24.416608333333333, 511, 55, 0, 0, false);

  ValidateCommState(
      msg.get(), 0, true, 4, false, 0, true, 2096, false, 0, 0, 0, false, 0,
      false, 0, false, 0, false, false);
}

TEST(Ais123Test, Decode1NoGPS) {
  // Verifies decode of Msg without a GPS lock.
  std::unique_ptr<Ais1_2_3> msg = Init(
      "!AIVDM,1,1,,B,13aIjG?P?w<tSF0l4Q@>401v1`FO,0*04,raishub,1342569600");
  Validate(
      msg.get(), 1, 0, 244740700, 15, -128, -731.386474609, true, 102.3, 0,
      181.0, 91.0, 0, 63, 0, 0, false);

  ValidateCommState(
      msg.get(), 3, true, 2, false, 0, true, 1439, false, 0, 0, 0, false, 0,
      false, 0, false, 0, false, false);
}

TEST(Ais123Test, Decode2Rot0) {
  // Verifies decode of Msg 2 driving straight ahead.
  std::unique_ptr<Ais1_2_3> msg = Init(
      "!AIVDM,1,1,,B,2341N:0000PCTfPMHAQoP8442<;0,0*33,raishub,1342569601");

  Validate(
      msg.get(), 2, 0, 205545000, 0, 0, 0.0, false, 0.0, 1, 4.275973333333333,
      51.344011666666667, 258, 2, 0, 0, true);

  ValidateCommState(
      msg.get(), 0, true, 3, true, 704, false, 0, false, 0, 0, 0, false, 0,
      false, 0, false, 0, false, false);
}

TEST(Ais123Test, Decode2Turning) {
  // Verifies decode of Msg 2 for a ship in a turn.
  std::unique_ptr<Ais1_2_3> msg = Init(
      "!AIVDM,1,1,,B,284;UGTdP4301>3L;B@Wk3TnU@A1,0*7C,raishub,1342572913");

  Validate(
      msg.get(), 2, 0, 541255006, 4, -78, -271.591522217, false, 0.40000000596,
      0, 41.947201666666665, -62.606396666666669, 114, 27, 1, 1, false);

  ValidateCommState(
      msg.get(), 2, true, 4, false, 0, true, 1089, false, 0, 0, 0, false, 0,
      false, 0, false, 0, false, false);
}

TEST(Ais123Test, Decode3) {
  // Verifies decode of Msg 3.
  std::unique_ptr<Ais1_2_3> msg = Init(
      "!AIVDM,1,1,,B,34hoV<5000Jw95`GWokbFTuf0000,0*6C,raishub,1342569600");

  Validate(
      msg.get(), 3, 0, 319678000, 5, 0, 0.0, false, 0.0, 0, -70.0925,
      41.284183333333331, 158, 55, 0, 0, false);

  ValidateCommState(
      msg.get(), 0, false, 0, false, 0, false, 0, false, 0, 0, 0, false, 0,
      true, 0, true, 0, true, false);
}


TEST(Ais123Test, Decode3KeepTrue) {
  // Verifies decode of Msg 3 with non-zero slot and keep flag true.
  std::unique_ptr<Ais1_2_3> msg = Init(
      "!AIVDM,1,1,,B,35PH9M0P1VJe0GHG@NCeqh5n006S,0*10,raishub,1342569600");

  Validate(
      msg.get(), 3, 0, 369494388, 0, -128, -731.386474609, true, 10.1999998093,
      0, -74.054433333333336, 40.64525, 2, 59, 0, 0, false);;

  ValidateCommState(
      msg.get(), 0, false, 0, false, 0, false, 0, false, 0, 0, 0, false, 0,
      true, 26, true, 1, true, true);
}

TEST(Ais123Test, Decode3Special) {
  // Verifies decode of Msg 3 with a really large slot increment.
  std::unique_ptr<Ais1_2_3> msg = Init(
      "!AIVDM,1,1,,A,33aI;sPP00PD<sPMd8<P0?v0RC?C,0*14,raishub,1342569600");

  Validate(
      msg.get(), 3, 0, 244730862, 0, -128, -731.386474609, true, 0.0, 1,
      4.4132, 51.886163333333336, 511, 0, 1, 0, true);

  ValidateCommState(
      msg.get(), 0, false, 0, false, 0, false, 0, false, 0, 0, 0, false, 0,
      true, 4925, true, 1, true, true);
}

}  // namespace
}  // namespace libais
