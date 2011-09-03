// Since Apr 2010
// Class A shipdata
#include "ais.h"

//#include <iostream>
//#include <bitset>
//#include <string>
//#include <cassert>
//#include <cmath>


Ais5::Ais5(const char *nmea_payload) {
    assert(nmea_payload);
    init();

    if (strlen(nmea_payload) != 71) { status = AIS_ERR_BAD_BIT_COUNT; return; }

    std::bitset<426> bs; // 424 + 2 spare bits => 71 characters
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;
    
    message_id = ubits(bs, 0, 6);
    if (5 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);

    ais_version = ubits(bs, 38, 2);
    imo_num = ubits(bs, 40, 30);
    callsign = ais_str(bs, 70, 42);
    
    name = ais_str(bs, 112, 120);
    
    type_and_cargo = ubits(bs, 232, 8);
    dim_a = ubits(bs, 240, 9);
    dim_b = ubits(bs, 249, 9);
    dim_c = ubits(bs, 258, 6);
    dim_d = ubits(bs, 264, 6);
    fix_type = ubits(bs, 270, 4);
    eta_month = ubits(bs, 274, 4);
    eta_day = ubits(bs, 278, 5);
    eta_hour = ubits(bs, 283, 5);
    eta_minute = ubits(bs, 288, 6);
    draught = ubits(bs, 294, 8) / 10.;
    destination = ais_str(bs, 302, 120);
    //cout << "destination:" << destination << endl;
    dte = ubits(bs, 422, 1);
    spare = ubits(bs, 423, 1);
}

void Ais5::print() {
    std::cout << "Ais5 - shipdata\n"
         << "\tmessage_id: " << message_id << "  " << repeat_indicator << " " << mmsi << "\n"
         << "\tcallsign: " << callsign << "\n"
         << "\tname: " << name << "\n"
         << "\ttype_and_cargo: " << type_and_cargo << "\n"
         << "\tdim: " << dim_a << " " << dim_b  << " "<< dim_c << " " << dim_d  << " (m)\n"
         << "\tfix_type: " << fix_type << "\n"
         << "\teta: " << eta_month << "-" << eta_day << "T" << eta_hour << ":" << eta_minute << "\n"
         << "\tdestination: " << destination << "\n"
         << "\tdte: " << dte << "\n"
         << "\tspare: " << spare << "\n"
        ;
}

std::ostream& operator<< (std::ostream& o, Ais5 const& a)
{
    return o << 5 << ": " << a.mmsi << " \"" << a.name << "\" " << a.type_and_cargo 
             << " " << a.dim_a + a.dim_b << "x" << a.dim_c + a.dim_d << "x" << a.draught << "m" 
        ;
}

