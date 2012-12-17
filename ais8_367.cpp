// 8:367:22 Defined by an email from Greg Johnson representing the
// USCG, Fall 2012.  Breaks from the RTCM and IMO Circular 289.
// "Area Notice Message Release Version: 1" 13 Aug 2012

#include <cmath>

#include "ais.h"

const size_t SUB_AREA_BITS = 96;

static int scale_multipliers[4] = {1, 10, 100, 1000};

Ais8_367_22_Circle::Ais8_367_22_Circle(const bitset<AIS8_MAX_BITS> &bs,
                                       const size_t offset) {
  const int scale_factor = ubits(bs, offset + 3, 2);
  x = sbits(bs, offset + 5, 28) / 600000.;
  y = sbits(bs, offset + 33, 27) / 600000.;
  precision = ubits(bs, offset + 60, 3);
  radius_m = ubits(bs, offset + 63, 12) * scale_multipliers[scale_factor];
  spare = ubits(bs, offset + 75, 21);
}

Ais8_367_22_Rect::Ais8_367_22_Rect(const bitset<AIS8_MAX_BITS> &bs,
                                   const size_t offset) {
  const int scale_factor = ubits(bs, offset + 3, 2);
  x = sbits(bs, offset + 5, 28) / 600000.;
  y = sbits(bs, offset + 33, 27) / 600000.;
  precision = ubits(bs, offset + 60, 3);
  e_dim_m = ubits(bs, offset + 63, 8) * scale_multipliers[scale_factor];
  n_dim_m = ubits(bs, offset + 71, 8) * scale_multipliers[scale_factor];
  orient_deg = ubits(bs, offset + 79, 9);
  spare = ubits(bs, offset + 88, 8);
}

Ais8_367_22_Sector::Ais8_367_22_Sector(const bitset<AIS8_MAX_BITS> &bs,
                                       const size_t offset) {
  const int scale_factor = ubits(bs, offset + 3, 2);
  x = sbits(bs, offset + 5, 28) / 600000.;
  y = sbits(bs, offset + 33, 27) / 600000.;
  precision = ubits(bs, offset + 60, 3);
  radius_m = ubits(bs, offset + 63, 12) * scale_multipliers[scale_factor];
  left_bound_deg = ubits(bs, offset + 75, 9);
  right_bound_deg = ubits(bs, offset + 84, 9);
  spare = ubits(bs, offset + 93, 3);
}

// polyline or polygon
Ais8_367_22_Poly::Ais8_367_22_Poly(const bitset<AIS8_MAX_BITS> &bs,
                                       const size_t offset) {
  shape = static_cast<Ais8_366_22_AreaShapeEnum>(ubits(bs, offset, 3));
  const int scale_factor = ubits(bs, offset + 3, 2);
  for (size_t i = 0; i < 4; i++) {
    const int angle = ubits(bs, offset + 5 + (i*21), 10);
    const int dist =
        ubits(bs, offset + 15 + (i*21), 11) * scale_multipliers[scale_factor];
    if (0 == dist)
      break;
    angles.push_back(angle);
    dists_m.push_back(dist);
  }
  spare = ubits(bs, offset + 89, 7);
}

Ais8_367_22_Text::Ais8_367_22_Text(const bitset<AIS8_MAX_BITS> &bs,
                                   const size_t offset) {
  text = string(ais_str(bs, offset + 3, 90));
  spare = ubits(bs, offset + 90, 3);
}


Ais8_367_22::Ais8_367_22(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 367);
  assert(fi == 22);

  const int num_bits = (strlen(nmea_payload) * 6) - pad;
  if (num_bits <= 216 && num_bits >= 1016) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<MAX_BITS> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  version = ubits(bs, 56, 6);
  link_id = ubits(bs, 62, 10);
  notice_type = ubits(bs, 72, 7);
  month = ubits(bs, 79, 4);
  day = ubits(bs, 83, 5);
  hour = ubits(bs, 88, 5);
  minute = ubits(bs, 93, 6);
  duration_minutes = ubits(bs, 99, 18);
  spare2 = ubits(bs, 117, 3);

  const int num_sub_areas = static_cast<int>(floor((num_bits - 120)/float(SUB_AREA_BITS)));

  // TODO(schwehr): warn if we see extra bits
  for (int area_idx = 0; area_idx < num_sub_areas; area_idx++) {
    const size_t start = 120 + area_idx*SUB_AREA_BITS;
    Ais8_367_22_SubArea *area =
        ais8_367_22_subarea_factory(bs, start);
    if (area)
      sub_areas.push_back(area);
    else
      status = AIS_ERR_BAD_SUB_SUB_MSG;
  }

}

Ais8_367_22::~Ais8_367_22() {
  for (size_t i = 0; i < sub_areas.size(); i++) {
    delete sub_areas[i];
    sub_areas[i] = NULL;
  }
}

Ais8_367_22_SubArea *
ais8_367_22_subarea_factory(const bitset<AIS8_MAX_BITS> &bs,
                            const size_t offset) {
  const Ais8_366_22_AreaShapeEnum area_shape =
    static_cast<Ais8_366_22_AreaShapeEnum>(ubits(bs, offset, 3));
  Ais8_367_22_SubArea *area = NULL;
  switch (area_shape) {
  case AIS8_366_22_SHAPE_CIRCLE:
    area = new Ais8_367_22_Circle(bs, offset);
    break;
  case AIS8_366_22_SHAPE_RECT:
    area = new Ais8_367_22_Rect(bs, offset);
    return area;
  case AIS8_366_22_SHAPE_SECTOR:
    area = new Ais8_367_22_Sector(bs, offset);
    break;
  case AIS8_366_22_SHAPE_POLYLINE:  // FALLTHROUGH
  case AIS8_366_22_SHAPE_POLYGON:
    area = new Ais8_367_22_Poly(bs, offset);
    break;
  case AIS8_366_22_SHAPE_TEXT:
    area = new Ais8_367_22_Text(bs, offset);
    break;
  case AIS8_366_22_SHAPE_RESERVED_6:  // FALLTHROUGH
  case AIS8_366_22_SHAPE_RESERVED_7:  // FALLTHROUGH
    // Leave area as 0 to indicate error
    break;
  case AIS8_366_22_SHAPE_ERROR:
    break;
  default:
    assert(false);
  }
  return area;
}
