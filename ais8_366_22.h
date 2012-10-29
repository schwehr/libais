#ifndef AIS8_366_22_H
#define AIS8_366_22_H

// -*- c++ -*-
// Way complicated

// Which spec was I coding to?  Probably the Nav55

// TODO(schwehr): should spare be int or unsigned int?

#include <iostream>

enum Ais8_366_22_AreaShapeEnum {
    AIS8_366_22_SHAPE_ERROR = -1,
    AIS8_366_22_SHAPE_CIRCLE = 0,
    AIS8_366_22_SHAPE_RECT = 1,
    AIS8_366_22_SHAPE_SECTOR = 2,
    AIS8_366_22_SHAPE_POLYLINE = 3,
    AIS8_366_22_SHAPE_POLYGON = 4,
    AIS8_366_22_SHAPE_TEXT = 5,
    AIS8_366_22_SHAPE_RESERVED_6 = 6,
    AIS8_366_22_SHAPE_RESERVED_7 = 7
};

extern const char *shape_names[8];

class Ais8_366_22_SubArea {
 public:
    virtual Ais8_366_22_AreaShapeEnum getType()=0;
    virtual ~Ais8_366_22_SubArea() { }
};

Ais8_366_22_SubArea* ais8_366_22_subarea_factory(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);

// or Point if radius is 0
class Ais8_366_22_Circle : public Ais8_366_22_SubArea {
 public:
    float x,y;
    // TODO(schwehr): int precision
    int radius_m;
    unsigned int spare;

    Ais8_366_22_Circle(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);
    ~Ais8_366_22_Circle() {}
    Ais8_366_22_AreaShapeEnum getType() {return AIS8_366_22_SHAPE_CIRCLE;}
};

class Ais8_366_22_Rect : public Ais8_366_22_SubArea {
 public:
    float x,y; // longitude and latitude
    // TODO(schwehr): int precision
    int e_dim_m; // East dimension in meters
    int n_dim_m;
    int orient_deg; // Orientation in degrees from true north
    unsigned int spare; // 5 bits

    Ais8_366_22_Rect(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);
    ~Ais8_366_22_Rect() {}
    Ais8_366_22_AreaShapeEnum getType() {return AIS8_366_22_SHAPE_RECT;}

};

class Ais8_366_22_Sector : public Ais8_366_22_SubArea {
 public:
    float x,y;
    // TODO(schwehr): int precision
    int radius_m;
    int left_bound_deg;
    int right_bound_deg;
    // TODO(schwehr): spare?

    Ais8_366_22_Sector(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);
    ~Ais8_366_22_Sector() {}
    Ais8_366_22_AreaShapeEnum getType() {return AIS8_366_22_SHAPE_SECTOR;}
};

// Or Waypoint
// Must have a point before on the VDL, but pulled together here.
class Ais8_366_22_Polyline : public Ais8_366_22_SubArea {
 public:
    float x,y; // longitude and latitude
    // TODO(schwehr)? precision

    // Up to 4 points
    std::vector<float> angles;
    std::vector<float> dists_m;
    unsigned int spare;

    Ais8_366_22_Polyline(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);
    ~Ais8_366_22_Polyline() {}
    Ais8_366_22_AreaShapeEnum getType() {return AIS8_366_22_SHAPE_POLYLINE;}

};

class Ais8_366_22_Polygon : public Ais8_366_22_SubArea {
 public:
    float x,y; // longitude and latitude
    // TODO(schwehr): precision?

    // Up to 4 points in a first message, but aggregated if multiple sub areas
    std::vector<float> angles;
    std::vector<float> dists_m;
    unsigned int spare;

    Ais8_366_22_Polygon(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);
    ~Ais8_366_22_Polygon() {}
    Ais8_366_22_AreaShapeEnum getType() {return AIS8_366_22_SHAPE_POLYGON;}
};

class Ais8_366_22_Text : public Ais8_366_22_SubArea {
 public:
    std::string text;
    unsigned int spare; // 3 bits

    Ais8_366_22_Text(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);
    ~Ais8_366_22_Text() {}
    Ais8_366_22_AreaShapeEnum getType() {return AIS8_366_22_SHAPE_TEXT;}
};

class Ais8_366_22 : public Ais8 {
 public:
    // Common block at the front
    int link_id; // 10 bit id to match up text blocks
    int notice_type; // area_type / Notice Description
    int month; // These really are in utc
    int day;
    int utc_hour;
    int utc_minute;
    int duration_minutes; // Time from the start until the notice expires
    // 1 or more sub messages

    std::vector<Ais8_366_22_SubArea *> sub_areas;

  Ais8_366_22(const char *nmea_payload, const size_t pad);
    ~Ais8_366_22();
};
std::ostream& operator<< (std::ostream& o, Ais8_366_22 const& msg);

const size_t AIS8_366_22_NUM_NAMES=128;
extern const char *ais8_366_22_notice_names[AIS8_366_22_NUM_NAMES];

#endif
