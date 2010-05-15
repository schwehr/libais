// Since 2010-May-14
#include "ais.h"

Ais12::Ais12(const char *nmea_payload) {
    assert(nmea_payload);
    init();

    const int num_char = strlen(nmea_payload);
    // WARNING: the spec says max of 1008 bits, but 168 + 4*256 => 1192 bits or 199 characters
    if ( num_char < 12 or num_char > 199 ) { status = AIS_ERR_BAD_BIT_COUNT; return; }

    std::bitset<1192> bs; // Spec says 1008
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;
    
    message_id = ubits(bs, 0, 6);
    if (12 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);

    seq_num = ubits(bs,38,2);
    dest_mmsi = ubits(bs,40,30);
    retransmitted = bool(bs[70]);
    spare = bool(bs[71]);
    int num_txt_bits = 6 * ((num_char * 6 - 72) / 6);
    std::cout << "FIX: num_txt_bits: " << num_txt_bits <<  std::endl;
    text = ais_str(bs,72,num_txt_bits);
}

void Ais12::print() {
    std::cout << "Addressed safety message: " << message_id << "\n"
              << "\tmmsi: " << mmsi << " repeat: " << repeat_indicator << "\n"
              << "\tseq_num: " << seq_num << " retransmitted: " << retransmitted << "\n"
              << "\tdest_mmsi: " << dest_mmsi << "\n"
              << "\ttext: " << text << std::endl;
}
