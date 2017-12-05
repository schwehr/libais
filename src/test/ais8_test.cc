// Test parsing AIS 8 binary broadcast messages (BBM).

#include <memory>

#include "gtest/gtest.h"
#include "ais.h"

namespace libais {
namespace {

void ValidateAis8(const Ais8 *msg, const int repeat_indicator, const int mmsi,
                  const int spare,
                  const int dac, const int fi) {
  ASSERT_NE(nullptr, msg);
  EXPECT_FALSE(msg->had_error());

  ASSERT_EQ(8, msg->message_id);
  EXPECT_EQ(repeat_indicator, msg->repeat_indicator);
  EXPECT_EQ(mmsi, msg->mmsi);
  EXPECT_EQ(spare, msg->spare);

  EXPECT_EQ(dac, msg->dac);
  ASSERT_EQ(fi, msg->fi);
}

void ValidateAis8_1_11(
    const Ais8_1_11 *msg,
    // TODO(schwehr): position.
    const int day, const int hour, const int minute, const int wind_ave,
    const int wind_gust, const int wind_dir, const int wind_gust_dir,
    const float air_temp, const int rel_humid, const float dew_point,
    const float air_pres, const int air_pres_trend, const float horz_vis,
    const float water_level, const int water_level_trend,
    const float surf_cur_speed, const int surf_cur_dir, const float cur_speed_2,
    const int cur_dir_2, const int cur_depth_2, const float cur_speed_3,
    const int cur_dir_3, const int cur_depth_3, const float wave_height,
    const int wave_period, const int wave_dir, const float swell_height,
    const int swell_period, const int swell_dir, const int sea_state,
    const float water_temp, const int precip_type, const float salinity,
    const int ice, const int spare2, const int extended_water_level) {
  ASSERT_NE(nullptr, msg);
  // TODO(schwehr): AisPoint position;
  EXPECT_EQ(day, msg->day);
  EXPECT_EQ(hour, msg->hour);
  EXPECT_EQ(minute, msg->minute);
  EXPECT_EQ(wind_ave, msg->wind_ave);
  EXPECT_EQ(wind_gust, msg->wind_gust);
  EXPECT_EQ(wind_dir, msg->wind_dir);
  EXPECT_EQ(wind_gust_dir, msg->wind_gust_dir);
  EXPECT_FLOAT_EQ(air_temp, msg->air_temp);
  EXPECT_EQ(rel_humid, msg->rel_humid);
  EXPECT_FLOAT_EQ(dew_point, msg->dew_point);
  EXPECT_FLOAT_EQ(air_pres, msg->air_pres);
  EXPECT_EQ(air_pres_trend, msg->air_pres_trend);
  EXPECT_FLOAT_EQ(horz_vis, msg->horz_vis);
  EXPECT_FLOAT_EQ(water_level, msg->water_level);
  EXPECT_EQ(water_level_trend, msg->water_level_trend);
  EXPECT_FLOAT_EQ(surf_cur_speed, msg->surf_cur_speed);
  EXPECT_EQ(surf_cur_dir, msg->surf_cur_dir);
  EXPECT_FLOAT_EQ(cur_speed_2, msg->cur_speed_2);
  EXPECT_EQ(cur_dir_2, msg->cur_dir_2);
  EXPECT_EQ(cur_depth_2, msg->cur_depth_2);
  EXPECT_FLOAT_EQ(cur_speed_3, msg->cur_speed_3);
  EXPECT_EQ(cur_dir_3, msg->cur_dir_3);
  EXPECT_EQ(cur_depth_3, msg->cur_depth_3);
  EXPECT_FLOAT_EQ(wave_height, msg->wave_height);
  EXPECT_EQ(wave_period, msg->wave_period);
  EXPECT_EQ(wave_dir, msg->wave_dir);
  EXPECT_FLOAT_EQ(swell_height, msg->swell_height);
  EXPECT_EQ(swell_period, msg->swell_period);
  EXPECT_EQ(swell_dir, msg->swell_dir);
  EXPECT_EQ(sea_state, msg->sea_state);
  EXPECT_FLOAT_EQ(water_temp, msg->water_temp);
  EXPECT_EQ(precip_type, msg->precip_type);
  EXPECT_FLOAT_EQ(salinity, msg->salinity);
  EXPECT_EQ(ice, msg->ice);
  EXPECT_EQ(spare2, msg->spare2);
  EXPECT_EQ(extended_water_level, msg->extended_water_level);
}

// Internation Maritime Organization (IMO) Circ 289 meteorology and hydrography.
TEST(Ais8_1_11Test, DecodeAnything) {
  // clang-format off
  // !AIVDM,1,1,,A,8@2<HV@0BkLN:0frqMPaQPtBRRIrwwejwwwwwwwwwwwwwwwwwwwwwwwwwt0,2*34  // NOLINT
  // clang-format on
  std::unique_ptr<Ais8_1_11> msg(new Ais8_1_11(
      "8@2<HV@0BkLN:0frqMPaQPtBRRIrwwejwwwwwwwwwwwwwwwwwwwwwwwwwt0", 2));

  ValidateAis8(msg.get(), 1, 2300057, 0, 1, 11);
  ValidateAis8_1_11(msg.get(), /* TODO(schwehr): Position. */ 22, 1, 19, 6, 7,
                    274, 276, 1.5, 87, 82.3, 1020, 2, 25.5, 41.1, 3, 25.5, 511,
                    25.5, 511, 31, 25.5, 511, 31, 25.5, 63, 511, 25.5, 63, 511,
                    15, 92.3, 7, 51.1, 3, 0, 0);
}

TEST(Ais8_1_11Test, IncorrectBitCount) {
  std::unique_ptr<Ais8_1_11> msg(new Ais8_1_11(
      "8@2<HV@0BkLN:0frqMPaQPtBRRIrwwejwwwwwwwwwwwwwwwwwwwwwwwwwt0", 0));
  EXPECT_TRUE(msg->had_error());
}

}  // namespace
}  // namespace libais
