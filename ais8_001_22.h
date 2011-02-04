// -*- c++ -*-
// Way complicated
// Since 2011-Feb-03

// Matches IMO Circ 289, Page 39
// May or may not match RTCM working group final message.  That is a moving target
// and is in ais8_366_22

const size_t AIS8_001_22_NUM_NAMES=128;
extern const char *notice_names[AIS8_001_22_NUM_NAMES];

enum Ais8_001_22_AreaShapeEnum {
    AIS8_001_22_SHAPE_ERROR = -1,
    AIS8_001_22_SHAPE_CIRCLE = 0,
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
    //Ais8_001_22_AreaShapeEnum area_shape;
    virtual Ais8_001_22_AreaShapeEnum getType()=0;  
    // FIX: make the destructor pure virtual
    virtual ~Ais8_001_22_SubArea() { std::cout << "Ais8_001_22_Circle: destructor" << std::endl; };
    virtual void print()=0;
};

Ais8_001_22_SubArea* ais8_001_22_subarea_factory(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);

// or Point if radius is 0
class Ais8_001_22_Circle : public Ais8_001_22_SubArea {
public:
    float x,y; // longitude and latitude
    // Going to assume that the precision is load of crap
    //int precision; // How many decimal places for x and y.  FIX: is this really supposed to be here????
    int radius_m;
    unsigned int spare; // 18 bits

    Ais8_001_22_Circle(const std::bitset<AIS8_MAX_BITS> &bs, const size_t offset);
    ~Ais8_001_22_Circle() { std::cout << "Ais8_001_22_Circle: destructor" << std::endl;};
    Ais8_001_22_AreaShapeEnum getType() {return AIS8_001_22_SHAPE_CIRCLE;}
    void print();
};



//////////////////////////////////////////////////////////////////////
// Area Notice class
//////////////////////////////////////////////////////////////////////

class Ais8_001_22 : public Ais8 {
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

    std::vector<Ais8_001_22_SubArea *> sub_areas;

    Ais8_001_22(const char *nmea_payload);
    ~Ais8_001_22();
    void print();
};
std::ostream& operator<< (std::ostream& o, Ais8_001_22 const& msg);


