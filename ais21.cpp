// Since 2010-May-05
// Msg 21 - ATON status
#include "ais.h"

Ais21::Ais21(const char *nmea_payload, const int pad) {
    assert(nmea_payload);
    init();

    const size_t num_bits = strlen(nmea_payload) * 6 - pad;

    // 272-360 - FIX: make this more careful
    if (272 > num_bits || num_bits > 360) { status = AIS_ERR_BAD_BIT_COUNT; return; }

    std::bitset<360> bs; // 360 % 6 == 0 -> 60 NMEA characters exactly
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;
    
    message_id = ubits(bs, 0, 6);
    if (message_id != 21) {status = AIS_ERR_WRONG_MSG_TYPE; return;}
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);
    aton_type = ubits(bs,38,5);
    name = ais_str(bs, 43, 120);
    position_accuracy = bs[163];
    x = sbits(bs, 164, 28) / 600000.;
    y = sbits(bs, 192, 27) / 600000.;
    dim_a = ubits(bs, 219, 9);
    dim_b = ubits(bs, 228, 9);
    dim_c = ubits(bs, 237, 6);
    dim_d = ubits(bs, 243, 6);
    fix_type = ubits(bs, 249, 4);
    timestamp = ubits(bs, 253, 6);
    off_pos = bool(bs[259]);
    aton_status = ubits(bs, 260, 8);
    raim = bool(bs[268]);
    virtual_aton = bool(bs[269]);
    assigned_mode = bool(bs[270]);
    spare = bs[271];

    const size_t extra_total_bits = num_bits - 272;
    const size_t extra_chars = extra_total_bits / 6;
    const size_t extra_char_bits = extra_chars * 6;
    const size_t extra_bits = extra_total_bits % 6;

    if (extra_chars > 0) {
        name += ais_str(bs,272,extra_char_bits);
    }

    if (extra_bits > 0) {
        spare2 = ubits(bs,272+extra_char_bits, extra_bits);
    } else {
        spare2 = 0;
    }
    
}


void Ais21::print() {
    //CHECKPOINT;
    std::cout << "ATON status: " << message_id << "\n"
              << "\tmmsi: " << mmsi << " repeat: " << repeat_indicator << "\n"

              << "\taton_type: " << aton_type << "\n"
              << "\tname: " << name << "\n"
              << "\tposition_accuracy: " << position_accuracy << "\n"
              << "\tloc: " << x << " " << y << "\n"
              << "\tdim_a: " << dim_a << "\n"
              << "\tdim_b: " << dim_b << "\n"
              << "\tdim_c: " << dim_c << "\n"
              << "\tdim_d: " << dim_d << "\n"
              << "\tfix_type: " << fix_type << "\n"
              << "\ttimestamp: " << timestamp << "\n"
              << "\toff_pos: " << (off_pos?"true": "false") << "\n"
              << "\tstatus: " << aton_status << "\n"
              << "\traim: " << (raim?"true": "false") << "\n"
              << "\tvirtual_aton: " << (virtual_aton?"true": "false") << "\n"
              << "\tassigned_mode: " << (assigned_mode?"true": "false") << "\n"
              << "\tspare: " << spare << "\n"
              // Extended name goes on the end of name
              << "\tspare2: " << spare2 << std::endl;
}
