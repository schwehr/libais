#include <cmath>
#include <iomanip>

#include "ais.h"
#include "ais8_001_22.h"

// TODO(schwehr): field on class
const char *ais8_001_22_shape_names[8] = {"Circle/Pt", "Rect", "Sector",
                                          "Polyline", "Polygon", "Text",
                                          "Reserved_6", "Reserved_7"};

const char *ais8_001_22_notice_names[AIS8_001_22_NUM_NAMES] = {
  // 0 has extra text compared to the spec
  "Caution Area: Marine mammals habitat (implies whales NOT observed)",  // 0
  "Caution Area: Marine mammals in area - reduce speed",  // 1
  "Caution Area: Marine mammals in area - stay clear",  // 2
  "Caution Area: Marine mammals in area - report sightings",  // 3
  "Caution Area: Protected habitat - reduce speed",  // 4
  "Caution Area: Protected habitat - stay clear",  // 5
  "Caution Area: Protected habitat - no fishing or anchoring",  // 6
  "Caution Area: Derelicts (drifting objects)",  // 7
  "Caution Area: Traffic congestion",  // 8
  "Caution Area: Marine event",  // 9
  "Caution Area: Divers down",  // 10
  "Caution Area: Swim area",  // 11
  "Caution Area: Dredge operations",  // 12
  "Caution Area: Survey operations",  // 13
  "Caution Area: Underwater operation",  // 14
  "Caution Area: Seaplane operations",  // 15
  "Caution Area: Fishery - nets in water",  // 16
  "Caution Area: Cluster of fishing vessels",  // 17
  "Caution Area: Fairway closed",  // 18
  "Caution Area: Harbour closed",  // 19
  "Caution Area: Risk (define in Associated text field)",  // 20
  "Caution Area: Underwater vehicle operation",  // 21
  "(reserved for future use)",  // 22
  "Environmental Caution Area: Storm front (line squall)",  // 23
  "Environmental Caution Area: Hazardous sea ice",  // 24
  "Environmental Caution Area: Storm warning "
  "(storm cell or line of storms)",  // 25
  "Environmental Caution Area: High wind",  // 26
  "Environmental Caution Area: High waves",  // 27
  "Environmental Caution Area: Restricted visibility (fog, rain, etc.)",  // 28
  "Environmental Caution Area: Strong currents",  // 29
  "Environmental Caution Area: Heavy icing",  // 30
  "(reserved for future use)",  // 31
  "Restricted Area: Fishing prohibited",  // 32
  "Restricted Area: No anchoring.",  // 33
  "Restricted Area: Entry approval required prior to transit",  // 34
  "Restricted Area: Entry prohibited",  // 35
  "Restricted Area: Active military OPAREA",  // 36
  "Restricted Area: Firing - danger area.",  // 37
  "Restricted Area: Drifting Mines",  // 38
  "(reserved for future use)",  // 39
  "Anchorage Area: Anchorage open",  // 40
  "Anchorage Area: Anchorage closed",  // 41
  "Anchorage Area: Anchoring prohibited",  // 42
  "Anchorage Area: Deep draft anchorage",  // 43
  "Anchorage Area: Shallow draft anchorage",  // 44
  "Anchorage Area: Vessel transfer operations",  // 45
  "(reserved for future use)",  // 46
  "(reserved for future use)",  // 47
  "(reserved for future use)",  // 48
  "(reserved for future use)",  // 49
  "(reserved for future use)",  // 50
  "(reserved for future use)",  // 51
  "(reserved for future use)",  // 52
  "(reserved for future use)",  // 53
  "(reserved for future use)",  // 54
  "(reserved for future use)",  // 55
  "Security Alert - Level 1",  // 56
  "Security Alert - Level 2",  // 57
  "Security Alert - Level 3",  // 58
  "(reserved for future use)",  // 59
  "(reserved for future use)",  // 60
  "(reserved for future use)",  // 61
  "(reserved for future use)",  // 62
  "(reserved for future use)",  // 63
  "Distress Area: Vessel disabled and adrift",  // 64
  "Distress Area: Vessel sinking",  // 65
  "Distress Area: Vessel abandoning ship",  // 66
  "Distress Area: Vessel requests medical assistance",  // 67
  "Distress Area: Vessel flooding",  // 68
  "Distress Area: Vessel fire/explosion",  // 69
  "Distress Area: Vessel grounding",  // 70
  "Distress Area: Vessel collision",  // 71
  "Distress Area: Vessel listing/capsizing",  // 72
  "Distress Area: Vessel under assault",  // 73
  "Distress Area: Person overboard",  // 74
  "Distress Area: SAR area",  // 75
  "Distress Area: Pollution response area",  // 76
  "(reserved for future use)",  // 77
  "(reserved for future use)",  // 78
  "(reserved for future use)",  // 79
  "Instruction: Contact VTS at this point/juncture",  // 80
  "Instruction: Contact Port Administration at this point/juncture",  // 81
  "Instruction: Do not proceed beyond this point/juncture",  // 82
  "Instruction: Await instructions prior to proceeding beyond "
  "this point/juncture",  // 83
  "Proceed to this location - await instructions",  // 84
  "Clearance granted - proceed to berth",  // 85
  "(reserved for future use)",  // 86
  "(reserved for future use)",  // 87
  "Information: Pilot boarding position",  // 88
  "Information: Icebreaker waiting area",  // 89
  "Information: Places of refuge",  // 90
  "Information: Position of icebreakers",  // 91
  "Information: Location of response units",  // 92
  "VTS active target",  // 93
  "Rouge or suspicious vessel",  // 94
  "Vessel requesting non-distress assistance",  // 95
  "Chart Feature: Sunken vessel",  // 96
  "Chart Feature: Submerged object",  // 97
  "Chart Feature: Semi-submerged object",  // 98
  "Chart Feature: Shoal area",  // 99
  "Chart Feature: Shoal area due north",  // 100
  "Chart Feature: Shoal area due east",  // 101
  "Chart Feature: Shoal area due south",  // 102
  "Chart Feature: Shoal area due west",  // 103
  "Chart Feature: Channel obstruction",  // 104
  "Chart Feature: Reduced vertical clearance",  // 105
  "Chart Feature: Bridge closed",  // 106
  "Chart Feature: Bridge partially open",  // 107
  "Chart Feature: Bridge fully open",  // 108
  "(reserved for future use)",  // 109
  "(reserved for future use)",  // 110
  "(reserved for future use)",  // 111
  "Report from ship: Icing info",  // 112
  "(reserved for future use)",  // 113
  "Report from ship: Miscellaneous information - "
  "define in Associated text field",  // 114
  "(reserved for future use)",  // 115
  "(reserved for future use)",  // 116
  "(reserved for future use)",  // 117
  "(reserved for future use)",  // 118
  "(reserved for future use)",  // 119
  "Route: Recommended route",  // 120
  "Route: Alternative route",  // 121
  "Route: Recommended route through ice",  // 122
  "(reserved for future use)",  // 123
  "(reserved for future use)",  // 124
  "Other - Define in associated text field",  // 125
  "Cancellation - cancel area as identified by Message Linkage ID",  // 126
  "Undefined (default)"  // 127
};

static int scale_multipliers[4] = {1, 10, 100, 1000};
// For the Scale factor so that we don't have to do a power of 10 calculation

//////////////////////////////////////////////////////////////////////
// Sub-Areas for the Area Notice class
//////////////////////////////////////////////////////////////////////

static void decode_xy(const bitset<AIS8_MAX_BITS> &bs, const size_t offset,
                      float &x, float &y) {
    // Offset is the start of the sub area.  Same as the caller's offset
    // This is the same for all but the text subarea
    // OLD Nav 55: sbits(bs, offset + 5, 28) / 600000.;
  x = sbits(bs, offset + 5, 25) / 60000.;
  y = sbits(bs, offset + 30, 24) / 60000.;
}

Ais8_001_22_Circle::Ais8_001_22_Circle(const bitset<AIS8_MAX_BITS> &bs,
                                       const size_t offset) {
  const int scale_factor = ubits(bs, offset + 3, 2);
  decode_xy(bs, offset, x, y);
  precision = ubits(bs, offset + 54, 3);  // useless
  radius_m  = ubits(bs, offset + 57, 12) * scale_multipliers[scale_factor];
  spare     = ubits(bs, offset + 69, 18);
}

Ais8_001_22_Rect::Ais8_001_22_Rect(const bitset<AIS8_MAX_BITS> &bs,
                                   const size_t offset) {
  const int scale_factor = ubits(bs, offset + 3, 2);
  decode_xy(bs, offset, x, y);

  precision  = ubits(bs, offset + 54, 3);  // useless
  e_dim_m    = ubits(bs, offset + 57, 8) * scale_multipliers[scale_factor];
  n_dim_m    = ubits(bs, offset + 65, 8) * scale_multipliers[scale_factor];
  orient_deg = ubits(bs, offset + 73, 9);
  spare      = ubits(bs, offset + 82, 5);
}

Ais8_001_22_Sector::Ais8_001_22_Sector(const bitset<AIS8_MAX_BITS> &bs,
                                       const size_t offset) {
  const int scale = ubits(bs, offset + 3, 2);
  decode_xy(bs, offset, x, y);

  precision       = ubits(bs, offset + 54, 3);
  radius_m        = ubits(bs, offset + 57, 12) * scale_multipliers[scale];
  left_bound_deg  = ubits(bs, offset + 69, 9);
  right_bound_deg = ubits(bs, offset + 78, 9);
}

// Size of one point angle and distance
static const size_t PT_AD_SIZE = 10 + 10;

Ais8_001_22_Polyline::Ais8_001_22_Polyline(const bitset<AIS8_MAX_BITS> &bs,
                                           const size_t offset) {
  const int scale_factor = ubits(bs, offset + 3, 2);
  const int multiplier = scale_multipliers[scale_factor];
  for (size_t i = 0; i < 4; i++) {
    const int angle = ubits(bs, offset + 5 + (i*PT_AD_SIZE), 10);
    const int dist  = ubits(bs, offset + 15 + (i*PT_AD_SIZE), 10) * multiplier;
    if (0 == dist)
      break;
    angles.push_back(angle);
    dists_m.push_back(dist);
  }
  spare = ubits(bs, offset + AIS8_001_22_SUBAREA_SIZE - 2, 2);
}

// TODO(schwehr): fold into polyline
Ais8_001_22_Polygon::Ais8_001_22_Polygon(const bitset<AIS8_MAX_BITS> &bs,
                                         const size_t offset) {
  const int scale_factor = ubits(bs, offset + 3, 2);
  const int multiplier = scale_multipliers[scale_factor];
  for (size_t i = 0; i < 4; i++) {
    const int angle = ubits(bs, offset + 5 + (i*PT_AD_SIZE), 10);
    const int dist  = ubits(bs, offset + 15 + (i*PT_AD_SIZE), 10) * multiplier;
    if (0 == dist)
      break;
    angles.push_back(angle);
    dists_m.push_back(dist);
  }
  spare = ubits(bs, offset + AIS8_001_22_SUBAREA_SIZE - 2, 2);
}

Ais8_001_22_Text::Ais8_001_22_Text(const bitset<AIS8_MAX_BITS> &bs,
                                   const size_t offset) {
  text = string(ais_str(bs, offset + 3, 84));
  // TODO(schwehr): spare?
}

// Call the appropriate constructor
Ais8_001_22_SubArea*
ais8_001_22_subarea_factory(const bitset<AIS8_MAX_BITS> &bs,
                            const size_t offset) {
  const Ais8_001_22_AreaShapeEnum area_shape =
      (Ais8_001_22_AreaShapeEnum)ubits(bs, offset, 3);
  Ais8_001_22_SubArea *area = NULL;
  switch (area_shape) {
  case AIS8_001_22_SHAPE_CIRCLE:
    area = new Ais8_001_22_Circle(bs, offset);
    break;
  case AIS8_001_22_SHAPE_RECT:
    area = new Ais8_001_22_Rect(bs, offset);
    return area;
  case AIS8_001_22_SHAPE_SECTOR:
    area = new Ais8_001_22_Sector(bs, offset);
    break;
  case AIS8_001_22_SHAPE_POLYLINE:
    area = new Ais8_001_22_Polyline(bs, offset);
    break;
  case AIS8_001_22_SHAPE_POLYGON:
    area = new Ais8_001_22_Polygon(bs, offset);
    break;
  case AIS8_001_22_SHAPE_TEXT:
    area = new Ais8_001_22_Text(bs, offset);
    break;
  case AIS8_001_22_SHAPE_RESERVED_6:  // FALLTHROUGH
  case AIS8_001_22_SHAPE_RESERVED_7:  // FALLTHROUGH
    // Keep area==0 to indicate error.
    break;
  case AIS8_001_22_SHAPE_ERROR:
    break;
  default:
    assert(false);
  }
  return area;
}


//////////////////////////////////////////////////////////////////////
// Area Notice class
//////////////////////////////////////////////////////////////////////

Ais8_001_22::Ais8_001_22(const char *nmea_payload, const size_t pad)
    : Ais8(nmea_payload, pad) {
  assert(nmea_ord_initialized);  // Make sure we have the lookup table built

  if (status != AIS_UNINITIALIZED)
    return;

  assert(dac == 1);
  assert(fi == 22);

  const int num_bits = strlen(nmea_payload) * 6 - pad;
  // TODO(schwehr): Make checks more exact.  Table 11.3, Circ 289 Annex, page 41
  // Spec is not byte aligned.  BAD!
  if (num_bits < 198 || num_bits > 984) {
    status = AIS_ERR_BAD_BIT_COUNT;
    return;
  }

  bitset<MAX_BITS> bs;
  const AIS_STATUS r = aivdm_to_bits(bs, nmea_payload);
  if (r != AIS_OK) {
    status = r;
    return;
  }

  link_id = ubits(bs, 56, 10);
  notice_type = ubits(bs, 66, 7);
  month = ubits(bs, 73, 4);
  day = ubits(bs, 77, 5);
  hour = ubits(bs, 82, 5);
  minute = ubits(bs, 87, 6);

  duration_minutes = ubits(bs, 93, 18);

  // Use floor to be able to ignore any spare bits
  const int num_sub_areas = static_cast<int>(floor((num_bits - 111)/87.));
  for (int sub_area_idx = 0; sub_area_idx < num_sub_areas; sub_area_idx++) {
    const size_t start = 111 + AIS8_001_22_SUBAREA_SIZE*sub_area_idx;
    Ais8_001_22_SubArea *sub_area = ais8_001_22_subarea_factory(bs, start);
    if (sub_area) {
      sub_areas.push_back(sub_area);
    } else {
      status = AIS_ERR_BAD_SUB_SUB_MSG;
    }
  }
  /* TODO(schwehr): inspect the subareas to make sure the are sane.
     - polyline/polygon have a point first
     - text has geometry to go through it all
  */
  // TODO(schwehr): watch out for mandatory spare bits to byte align payload
  if (status == AIS_UNINITIALIZED)
    status = AIS_OK;
}

Ais8_001_22::~Ais8_001_22() {
  for (size_t i = 0; i < sub_areas.size(); i++) {
    delete sub_areas[i];
    sub_areas[i] = NULL;
  }
}
