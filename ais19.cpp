// Since 2010-May-05
// Msg 19 C - Extended Class B equipment position report
#include "ais.h"

Ais19::Ais19(const char *nmea_payload) {
    assert(nmea_payload);

    init();
    if (strlen(nmea_payload) != 52) { status = AIS_ERR_BAD_BIT_COUNT; return; }

    std::bitset<312> bs; // 1 + a partial slot
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (19 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);

    spare = ubits(bs,38,8);
    sog = ubits(bs,46,10) / 10.;

    position_accuracy = ubits(bs,56,1);
    x = sbits(bs, 57, 28) / 600000.;
    y = sbits(bs, 85, 27) / 600000.;

    cog = ubits(bs, 112, 12) / 10.;
    true_heading = ubits(bs, 124, 9);
    timestamp = ubits(bs, 133, 6);
    spare2 = ubits(bs, 139, 4);

    name = ais_str(bs, 143, 120);

    type_and_cargo = ubits(bs, 263, 8);
    dim_a = ubits(bs, 271, 9);
    dim_b = ubits(bs, 280, 9);
    dim_c = ubits(bs, 289, 6);
    dim_d = ubits(bs, 295, 6);

    fix_type = ubits(bs, 301, 4);
    raim = bool(bs[305]);
    dte = bs[306];
    assigned_mode = bs[307];
    spare3 = ubits(bs,308,4);
}

void
Ais19::print() {
    std::cout << "Class B Extended Position and ship: " << message_id
              << "\tmmsi: " << mmsi << " repeat: " << repeat_indicator << "\n"
              << "\tspare: " << spare << " sog: " << sog << "\n"
              << "\tposacc: " << position_accuracy << "\n"
              << "\tx: " << x << "  y: " << y << "\n"
              << "\tcog: " << cog << " true_heading: " << true_heading << "\n"
              << "\ttimestamp: " << timestamp << " spare2: " << spare2 << "\n"
              << "\tname: " << name << "\n"
              << "\ttype_and_cargo: " << type_and_cargo << "\n"
              << "\tdim: " << dim_a << " " << dim_b  << " "<< dim_c << " " << dim_d  << " (m)\n"
              << "\tfix_type: " << fix_type << "\n"
              << "\traim: " << (raim?"true":"false") << "\n"
              << "\tdte: " << dte << " assigned_mode: " << assigned_mode << "\n"
              << "\tspare3: " << spare3 << std::endl
        ;
}
