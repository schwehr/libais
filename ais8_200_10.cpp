#include "ais8_200_10.h"

Ais8_200_10::Ais8_200_10(const char* nmea_payload): Ais8(nmea_payload)
{
    std::bitset<MAX_BITS> bs;
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;
    
    assert(200==dac);
    assert(10==fi);
    vessel_id_eu = ais_str(bs,56,48);
    length = ubits(bs,104,13)*0.1;
    beam = ubits(bs,117,10)*0.1;
    ship_type = ubits(bs,127,14);
    hazardous_cargo = ubits(bs,141,3);
    draught = ubits(bs,144,11)*0.01;
    loaded = ubits(bs,155,2);
    speed_quality = ubits(bs,157,1);
    course_quality = ubits(bs,158,1);
    heading_quality = ubits(bs,159,1);
}

Ais8_200_10::~Ais8_200_10()
{

}

void Ais8_200_10::print()
{
    std::cout << "Inland voyage info:" << std::endl;
    std::cout << "\tvessel id: " << vessel_id_eu << std::endl;
    std::cout << "\tlength: " << length << "\tbeam: " << beam << std::endl;
    std::cout << "\ttype: " << ship_type << "\thazardous: " << hazardous_cargo << std::endl;
    std::cout << "\tdraught: " << draught << "\tloaded: " << loaded << std::endl;
    std::cout << "\tquality (speed,course,heading): " << speed_quality << "," << course_quality << "," << heading_quality << std::endl;
}
