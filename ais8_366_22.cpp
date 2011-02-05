// Defined sort of by Nav55, but being tweaked by the USCG and
// will hopefully become a RTCM standard.  Also hoping that this
// will be harmonized with the IMO Circ 289

#include "ais.h"
#include <cmath>

const char *shape_names[8] = {"Circle/Pt","Rect", "Sector","Polyline","Polygon","Text","Reserved_6","Reserved_7"};

const char *ais8_366_22_notice_names[AIS8_366_22_NUM_NAMES] = { // 128] = {
   "Caution Area: Marine mammals habitat (no whales obs)", // 0
   "Caution Area: Marine mammals in area – reduce speed", // 1
   "Caution Area: Marine mammals in area – stay clear", // 2
   "Caution Area: Marine mammals in area – report sightings", // 3
   "Caution Area: Protected habitat – reduce speed", // 4
   "Caution Area: Protected habitat – stay clear", // 5
   "Caution Area: Protected habitat – no fishing or anchoring", // 6
   "Caution Area: Derelicts (drifting objects)", // 7
   "Caution Area: Traffic congestion", // 8
   "Caution Area: Marine event", // 9
   "Caution Area: Divers down", // 10
   "Caution Area: Swim area", // 11
   "Caution Area: Dredge operations", // 12
   "Caution Area: Survey operations", // 13
   "Caution Area: Underwater operation", // 14
   "Caution Area: Seaplane operations", // 15
   "Caution Area: Fishery – nets in water", // 16
   "Caution Area: Cluster of fishing vessels", // 17
   "Caution Area: Fairway closed", // 18
   "Caution Area: Harbour closed", // 19
   "Caution Area: Risk (define in Associated text field)", // 20
   "Caution Area: Underwater vehicle operation", // 21
   "(reserved for future use)", // 22
   "Environmental Caution Area: Storm front (line squall)", // 23
   "Environmental Caution Area: Hazardous sea ice", // 24
   "Environmental Caution Area: Storm warning (storm cell or line of storms)", // 25
   "Environmental Caution Area: High wind", // 26
   "Environmental Caution Area: High waves", // 27
   "Environmental Caution Area: Restricted visibility (fog, rain, etc.)", // 28
   "Environmental Caution Area: Strong currents", // 29
   "Environmental Caution Area: Heavy icing", // 30
   "(reserved for future use)", // 31
   "Restricted Area: Fishing prohibited", // 32
   "Restricted Area: No anchoring.", // 33
   "Restricted Area: Entry approval required prior to transit", // 34
   "Restricted Area: Entry prohibited", // 35
   "Restricted Area: Active military OPAREA", // 36
   "Restricted Area: Firing – danger area.", // 37
   "Restricted Area: Drifting Mines", // 38
   "(reserved for future use)", // 39
   "Anchorage Area: Anchorage open", // 40
   "Anchorage Area: Anchorage closed", // 41
   "Anchorage Area: Anchoring prohibited", // 42
   "Anchorage Area: Deep draft anchorage", // 43
   "Anchorage Area: Shallow draft anchorage", // 44
   "Anchorage Area: Vessel transfer operations", // 45
   "(reserved for future use)", // 46
   "(reserved for future use)", // 47
   "(reserved for future use)", // 48
   "(reserved for future use)", // 49
   "(reserved for future use)", // 50
   "(reserved for future use)", // 51
   "(reserved for future use)", // 52
   "(reserved for future use)", // 53
   "(reserved for future use)", // 54
   "(reserved for future use)", // 55
   "Security Alert – Level 1", // 56
   "Security Alert – Level 2", // 57
   "Security Alert – Level 3", // 58
   "(reserved for future use)", // 59
   "(reserved for future use)", // 60
   "(reserved for future use)", // 61
   "(reserved for future use)", // 62
   "(reserved for future use)", // 63
   "Distress Area: Vessel disabled and adrift", // 64
   "Distress Area: Vessel sinking", // 65
   "Distress Area: Vessel abandoning ship", // 66
   "Distress Area: Vessel requests medical assistance", // 67
   "Distress Area: Vessel flooding", // 68
   "Distress Area: Vessel fire/explosion", // 69
   "Distress Area: Vessel grounding", // 70
   "Distress Area: Vessel collision", // 71
   "Distress Area: Vessel listing/capsizing", // 72
   "Distress Area: Vessel under assault", // 73
   "Distress Area: Person overboard", // 74
   "Distress Area: SAR area", // 75
   "Distress Area: Pollution response area", // 76
   "(reserved for future use)", // 77
   "(reserved for future use)", // 78
   "(reserved for future use)", // 79
   "Instruction: Contact VTS at this point/juncture", // 80
   "Instruction: Contact Port Administration at this point/juncture", // 81
   "Instruction: Do not proceed beyond this point/juncture", // 82
   "Instruction: Await instructions prior to proceeding beyond this point/juncture", // 83
   "Proceed to this location – await instructions", // 84
   "Clearance granted – proceed to berth", // 85
   "(reserved for future use)", // 86
   "(reserved for future use)", // 87
   "Information: Pilot boarding position", // 88
   "Information: Icebreaker waiting area", // 89
   "Information: Places of refuge", // 90
   "Information: Position of icebreakers", // 91
   "Information: Location of response units", // 92
   "VTS active target", // 93
   "Rouge or suspicious vessel", // 94
   "Vessel requesting non-distress assistance", // 95
   "Chart Feature: Sunken vessel", // 96
   "Chart Feature: Submerged object", // 97
   "Chart Feature: Semi-submerged object", // 98
   "Chart Feature: Shoal area", // 99
   "Chart Feature: Shoal area due north", // 100
   "Chart Feature: Shoal area due east", // 101
   "Chart Feature: Shoal area due south", // 102
   "Chart Feature: Shoal area due west", // 103
   "Chart Feature: Channel obstruction", // 104
   "Chart Feature: Reduced vertical clearance", // 105
   "Chart Feature: Bridge closed", // 106
   "Chart Feature: Bridge partially open", // 107
   "Chart Feature: Bridge fully open", // 108
   "(reserved for future use)", // 109
   "(reserved for future use)", // 110
   "(reserved for future use)", // 111
   "Report from ship: Icing info", // 112
   "(reserved for future use)", // 113
   "Report from ship: Miscellaneous information – define in Associated text field", // 114
   "(reserved for future use)", // 115
   "(reserved for future use)", // 116
   "(reserved for future use)", // 117
   "(reserved for future use)", // 118
   "(reserved for future use)", // 119
   "Route: Recommended route", // 120
   "Route: Alternative route", // 121
   "Route: Recommended route through ice", // 122
   "(reserved for future use)", // 123
   "(reserved for future use)", // 124
   "Other – Define in associated text field", // 125
   "Cancellation – cancel area as identified by Message Linkage ID", // 126
   "Undefined (default)" //, // 127
};

Ais8_366_22::Ais8_366_22(const char *nmea_payload) {
    assert(nmea_payload);
    assert(strlen(nmea_payload) >= 28);
    init();
    const int num_bits = (strlen(nmea_payload) * 6);
    // FIX: make the bit checks more exact
    if (208 <= num_bits && num_bits >= 1020) { status = AIS_ERR_BAD_BIT_COUNT; return; }
    std::bitset<MAX_BITS> bs;

    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    if (!decode_header8(bs)) return;
    assert(366==dac);
    assert(22==fi);
    link_id = ubits(bs,56,10);
    notice_type = ubits(bs,66,7);
    month = ubits(bs,73,4);
    day = ubits(bs,77,5);
    utc_hour = ubits(bs,82,5);
    utc_minute = ubits(bs,87,6);

    duration_minutes = ubits(bs,93,18);
    
    const int num_sub_areas = int( floor( (num_bits - 111)/90.) );
    std::cout << "num_sub_areas: " << num_bits << " " << num_bits - 111 
         << " " << (num_bits - 111)/90.
         << " " << floor( (num_bits - 111)/90.)
         << " -> " << int( floor( (num_bits - 111)/90.) ) 
              << " " << num_sub_areas
              <<"\n";
    for (int sub_area_idx=0; sub_area_idx < num_sub_areas; sub_area_idx++) {
        //const sub_area_
        Ais8_366_22_SubArea *sub_area = ais8_366_22_subarea_factory(bs, 111+90*sub_area_idx);
        if (sub_area) { sub_areas.push_back(sub_area); }
        else {
            std::cout << "ERROR: bad sub area " << sub_area_idx << std::endl;
        }
    }

}

Ais8_366_22::~Ais8_366_22() {
    std::cout << "Ais8_366_22: destructor" << std::endl;
    for (size_t i=0; i < sub_areas.size(); i++) {
        delete sub_areas[i];
    }
    std::cout << "\n\n";
}

void
Ais8_366_22::print() {
    std::cout << "Area_Notice: " << message_id << "\n"
              << "\tdac: " << dac << "\tfi:" << fi << "\n"
              << "\tArea_type: " << notice_type 
              << " -> [" << ais8_366_22_notice_names[notice_type] << "]\n";
    for (size_t i=0; i < sub_areas.size(); i++) {
        std::cout << "\tSubarea: " << i << std::endl;
        sub_areas[i]->print();
    }
}

// Lookup table for the scale factors to decode the length / distance fields. 
// The index is the "Scale Factor"
static int scale_multipliers[4] = {1,10,100,1000};

Ais8_366_22_Circle::Ais8_366_22_Circle(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
    const int scale_factor = ubits(bs,offset+3,2);
    x         = sbits(bs, offset+5, 28) / 600000.;
    y         = sbits(bs, offset+5+28, 27) / 600000.;
    //precision = ubits(bs,offset+5+28+27,2);  // FIX: should precision
    radius_m  = ubits(bs,offset+5+28+27/*+2*/,12) * scale_multipliers[scale_factor];
    spare     = ubits(bs,offset+5+28+27/*+2*/+12,16);
}

void Ais8_366_22_Circle::print() {
    if (radius_m == 0) 
        std::cout << "\t\tPoint: " << " " << x << " " << y << "  (Can start a polyline or polygon)" << std::endl;
    else 
        std::cout << "\t\tCircle: " << " " << x << " " << y << " radius_m: " << radius_m << std::endl;
              
    //<< " precision FIX(!?!?) " << precision 
}

Ais8_366_22_Rect::Ais8_366_22_Rect(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
    const int scale_factor = ubits(bs,offset+3,2);
    x          = sbits(bs, offset+5, 28) / 600000.;
    y          = sbits(bs, offset+5+28, 27) / 600000.;
    e_dim_m    = ubits(bs, offset+5+28+27, 8) * scale_multipliers[scale_factor];
    n_dim_m    = ubits(bs, offset+5+28+27+8, 8) * scale_multipliers[scale_factor];
    orient_deg = ubits(bs, offset+5+28+27+8+8, 9);
    spare      = ubits(bs, offset+5+28+27+8+8+9, 5);
}

void Ais8_366_22_Rect::print() {
    std::cout << "\t\tRectangle: " << " " << x << " " << y 
              << " e_dim_m: " << e_dim_m << " n_dim_m: " << n_dim_m
              << " orient_deg: " << orient_deg
              << std::endl;
}

Ais8_366_22_Sector::Ais8_366_22_Sector(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
    const int scale_factor = ubits(bs,offset+3,2);
    x          = sbits(bs, offset+5, 28) / 600000.;
    y          = sbits(bs, offset+5+28, 27) / 600000.;
    radius_m        = ubits(bs,offset+5+28+27,12) * scale_multipliers[scale_factor];
    left_bound_deg  = ubits(bs, offset+5+28+27+12, 9);
    right_bound_deg = ubits(bs, offset+5+28+27+12+9, 9);
}

void Ais8_366_22_Sector::print() {
    std::cout << "\t\tSector: " << " " << x << " " << y << " radius_m: " << radius_m
              << " left_bound_deg: " << left_bound_deg << " right_bound_deg: " << right_bound_deg
              << std::endl;
}


Ais8_366_22_Polyline::Ais8_366_22_Polyline(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
    const int scale_factor = ubits(bs,offset+3,2);
    for (size_t i=0; i<4; i++) {
        const int angle = ubits(bs, offset+5+ (i*21), 10);
        const int dist  = ubits(bs, offset+5+10+ (i*21), 11) * scale_multipliers[scale_factor];
        if (0==dist) break;
        angles.push_back(angle);
        dists_m.push_back(dist);
    }
    spare = bs[offset+89];
}

void Ais8_366_22_Polyline::print() {
    std::cout << "\t\tPolyline: " << std::endl;
    for (size_t i=0; i<angles.size(); i++) {
        std::cout << "\t\t" << i << ": " << angles[i] << " deg, " << dists_m[i] << " meters" << std::endl;
    }
}

Ais8_366_22_Polygon::Ais8_366_22_Polygon(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
    const int scale_factor = ubits(bs,offset+3,2);
    for (size_t i=0; i<4; i++) {
        const int angle = ubits(bs, offset+5+ (i*21), 10);
        const int dist  = ubits(bs, offset+5+10+ (i*21), 11) * scale_multipliers[scale_factor];
        if (0==dist) break;
        angles.push_back(angle);
        dists_m.push_back(dist);
    }
    spare = bs[offset+89];
}

void Ais8_366_22_Polygon::print() {
    std::cout << "\t\tPolygon: " << std::endl;
    for (size_t i=0; i<angles.size(); i++) {
        std::cout << "\t\t" << i << ": " << angles[i] << " deg, " << dists_m[i] << " meters" << std::endl;
    }
}

Ais8_366_22_Text::Ais8_366_22_Text(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
    text = std::string(ais_str(bs, offset+3, 84));
    spare = ubits(bs,offset+87,3);
}

void Ais8_366_22_Text::print() {
    std::cout << "\t\tText: [" << text << "]" <<std::endl;
}




// Ais8_366_22_AreaShapeEnum getAreaShape(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
//     // Figure out which shape type or set as an error
//     const int area_shape_int = ubits(bs,offset,3);
    
// }

// Call the appropriate constructor
Ais8_366_22_SubArea* ais8_366_22_subarea_factory(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset) {
    const Ais8_366_22_AreaShapeEnum area_shape = (Ais8_366_22_AreaShapeEnum)ubits(bs, offset, 3);
    std::cout << "area_shape: off: " << offset << " shape: "<< area_shape << " [" << shape_names[area_shape] << "]\n";
    if (AIS8_366_22_SHAPE_ERROR == area_shape) {
        std::cerr << "ERROR: Bad area shape!  Bummer" << std::endl;
        return 0;
    }
    Ais8_366_22_SubArea *area=0;
    switch (area_shape) {
    case AIS8_366_22_SHAPE_CIRCLE:
        std::cout << "Found circle" << std::endl;
        area = new Ais8_366_22_Circle(bs, offset);
        break;
    case AIS8_366_22_SHAPE_RECT:
        std::cout << "Found rect" << std::endl;
        area = new Ais8_366_22_Rect(bs, offset);
        return area;
    case AIS8_366_22_SHAPE_SECTOR:
        std::cout << "Found sector" << std::endl;
        area = new Ais8_366_22_Sector(bs, offset);
        break;
    case AIS8_366_22_SHAPE_POLYLINE:
        std::cout << "Found polyline" << std::endl;
        area = new Ais8_366_22_Polyline(bs, offset);
        break;
    case AIS8_366_22_SHAPE_POLYGON:
        std::cout << "Found polygon" << std::endl;
        area = new Ais8_366_22_Polygon(bs, offset);
        break;
    case AIS8_366_22_SHAPE_TEXT:
        area = new Ais8_366_22_Text(bs, offset);
        break;
    case AIS8_366_22_SHAPE_RESERVED_6: // FALLTHROUGH
    case AIS8_366_22_SHAPE_RESERVED_7: // FALLTHROUGH
        std::cerr << "Warning: bad area shape" << std::endl;
        break;
    case AIS8_366_22_SHAPE_ERROR: // FALLTHROUGH
    default:
        assert(false);
    }
    return area; 
}
