// Since 2010-05-09

// Class B static data report.  Can be one of 4 different parts.  Only
// A and B defined for ITU 1371-3

#include "ais.h"

Ais24::Ais24(const char *nmea_payload) {
    assert (nmea_payload);
    init();
    const int num_bits = (strlen(nmea_payload) * 6);
    // 160 + 2 spare for part A
    if (162 != num_bits && 168 != num_bits) { status = AIS_ERR_BAD_BIT_COUNT; return; }
    std::bitset<168> bs;

    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (24 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);

    part_num = ubits(bs,38,2);

    switch(part_num) {

    case 0: // Part A
        if (162 != num_bits) { status = AIS_ERR_BAD_BIT_COUNT; return; }
        name = ais_str(bs, 40, 120);
        break;

    case 1: // Part B
        if (168 != num_bits) { status = AIS_ERR_BAD_BIT_COUNT; return; }
        type_and_cargo = ubits(bs, 40, 8);
        vendor_id = ais_str(bs, 48, 42);
        callsign = ais_str(bs, 90, 42);
        dim_a = ubits(bs, 132, 9);
        dim_b = ubits(bs, 141, 9);
        dim_c = ubits(bs, 150, 6);
        dim_d = ubits(bs, 156, 6);
        spare = ubits(bs, 162,6);
        break;

    case 2: // FALLTHROUGH - not yet defined by ITU
    case 3: // FALLTHROUGH - not yet defined by ITU
    default:
        status = AIS_ERR_BAD_MSG_CONTENT;
        return;
    }
}

void
Ais24::print() {
    std::cout << "Class B static data: " << message_id << "  part " << part_num << "\n";

    switch (part_num) {

    case 0:
        std::cout << "\tname: " << name << std::endl;
        break;

    case 1:
        std::cout << "\ttype_and_cargo: " << type_and_cargo << "\n"
                  << "\tvendor_id: " << vendor_id << "\n"
                  << "\tcallsign: " << callsign << "\n"
                  << "\tdim: " << dim_a << " " << dim_b  << " "<< dim_c << " " << dim_d  << " (m)\n"
                  << "\tspare: " << spare << std::endl;
            ;
        break;

    case 2: // FALLTHROUGH - not yet defined by ITU
    case 3: // FALLTHROUGH - not yet defined by ITU
    default:
        assert(false);
        return;
    }


}
