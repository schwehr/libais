// -*- c++ -*-
// Way complicated
// Since 2011-Feb-03


/*
  Design notes:

  Matches IMO Circ 289, Page 39.  May or may not match RTCM working
  group final message.  That is a moving target and is in ais8_366_22

  For polygon and polyline, I am planning to keep the data structures
  matching exactly the VDL data structure.  From there, I am hoping to
  extend the message class to separately reassemble that polyline and polygon.
 */

const size_t AIS8_001_22_NUM_NAMES=128;
const size_t AIS8_001_22_SUBAREA_SIZE=87;
extern const char *ais8_001_22_notice_names[AIS8_001_22_NUM_NAMES];

enum Ais8_001_22_AreaShapeEnum {
  AIS8_001_22_SHAPE_ERROR = -1,
  AIS8_001_22_SHAPE_CIRCLE = 0, // OR Point
  AIS8_001_22_SHAPE_RECT = 1,
  AIS8_001_22_SHAPE_SECTOR = 2,
  AIS8_001_22_SHAPE_POLYLINE = 3,
  AIS8_001_22_SHAPE_POLYGON = 4,
  AIS8_001_22_SHAPE_TEXT = 5,
  AIS8_001_22_SHAPE_RESERVED_6 = 6,
  AIS8_001_22_SHAPE_RESERVED_7 = 7
};

extern const char *ais8_001_22_shape_names[8];

//////////////////////////////////////////////////////////////////////
// Sub-Areas for the Area Notice class
//////////////////////////////////////////////////////////////////////

class Ais8_001_22_SubArea {
 public:
  virtual Ais8_001_22_AreaShapeEnum getType() const = 0;
  virtual ~Ais8_001_22_SubArea() {}
};

Ais8_001_22_SubArea* ais8_001_22_subarea_factory(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);

// or Point if radius is 0
class Ais8_001_22_Circle : public Ais8_001_22_SubArea {
 public:
  float x,y; // longitude and latitude
  // Going to assume that the precision is load of crap
  int precision; // How many decimal places for x and y.  Track it, but it's useless in my opinion
  int radius_m;
  unsigned int spare; // 18 bits

  Ais8_001_22_Circle(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  ~Ais8_001_22_Circle() {}
  Ais8_001_22_AreaShapeEnum getType() const {return AIS8_001_22_SHAPE_CIRCLE;}
};

class Ais8_001_22_Rect : public Ais8_001_22_SubArea {
 public:
  float x,y; // longitude and latitude
  int precision; // How many decimal places for x and y.  Useless
  int e_dim_m; // East dimension in meters
  int n_dim_m;
  int orient_deg; // Orientation in degrees from true north
  unsigned int spare; // 5 bits

  Ais8_001_22_Rect(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  ~Ais8_001_22_Rect() {}
  Ais8_001_22_AreaShapeEnum getType() const {return AIS8_001_22_SHAPE_RECT;}
};

class Ais8_001_22_Sector : public Ais8_001_22_SubArea {
 public:
  float x,y; // longitude and latitude
  // TODO(schwehr): precision in IMO, but not RTCM.  Double check
  int precision; // How many decimal places for x and y
  int radius_m;
  int left_bound_deg;
  int right_bound_deg;

  Ais8_001_22_Sector(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  ~Ais8_001_22_Sector() {}
  Ais8_001_22_AreaShapeEnum getType() const {return AIS8_001_22_SHAPE_SECTOR;}
};

// Or Waypoint
// Must have a point before on the VDL
// TODO(schwehr): do I bring in the prior point x,y, precision?
class Ais8_001_22_Polyline : public Ais8_001_22_SubArea {
 public:
  // TODO(schwehr): int precision; // How many decimal places for x and y.  FIX: in IMO

  // Up to 4 points
  vector<float> angles;
  vector<float> dists_m;
  unsigned int spare; // 2 bit

  Ais8_001_22_Polyline(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  ~Ais8_001_22_Polyline() {}
  Ais8_001_22_AreaShapeEnum getType() const {return AIS8_001_22_SHAPE_POLYLINE;}
};

// TODO(schwehr): bring in the prior point?  And do we fold the sub area data
// into one polygon if there are more than one?
class Ais8_001_22_Polygon : public Ais8_001_22_SubArea {
 public:
  // TODO(schwehr): int precision; // How many decimal places for x and y.  FIX: in IMO

  // Up to 4 points in a first message, but aggregated if multiple sub areas
  vector<float> angles;
  vector<float> dists_m;
  unsigned int spare;  // 2 bit

  Ais8_001_22_Polygon(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  ~Ais8_001_22_Polygon() {}
  Ais8_001_22_AreaShapeEnum getType() const {return AIS8_001_22_SHAPE_POLYGON;}
};


class Ais8_001_22_Text : public Ais8_001_22_SubArea {
 public:
  string text;
  // TODO(schwehr): spare?

  Ais8_001_22_Text(const bitset<AIS8_MAX_BITS> &bs, const size_t offset);
  ~Ais8_001_22_Text() {}
  Ais8_001_22_AreaShapeEnum getType() const {return AIS8_001_22_SHAPE_TEXT;}
};

//////////////////////////////////////////////////////////////////////
// Area Notice class
//////////////////////////////////////////////////////////////////////

class Ais8_001_22 : public Ais8 {
 public:
  int link_id; // 10 bit id to match up text blocks
  int notice_type; // area_type / Notice Description
  int month; // These are in UTC
  int day;   // UTC!
  int hour;  // UTC!
  int minute;
  int duration_minutes; // Time from the start until the notice expires

  // 1 or more sub messages
  vector<Ais8_001_22_SubArea *> sub_areas;

  Ais8_001_22(const char *nmea_payload, const size_t pad);
  ~Ais8_001_22();

  /*
    TODO(schwehr): need some sort of higher level accessors and checks
    - return interpreted geometry and associated agreegated text
    - What formats to return?  GeoJSON, WKT, etc?
    - validate that the contents of sub_areas is sane
    - polylines and polygons start with a point
    - text has an associated geometry
  */
};
ostream& operator<< (ostream& o, Ais8_001_22 const& msg);
