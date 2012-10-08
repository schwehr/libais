// Address Binary Message (ABM) 6

#include "ais.h"

#include <iomanip>


Ais6::Ais6(const char *nmea_payload) {
    assert(nmea_payload);
	assert(nmea_ord_initialized); // Make sure we have the lookup table built
    init();
    const int payload_len = strlen(nmea_payload)*6 - 46; // in bits w/o DAC/FI

    if (payload_len < 0 or payload_len > 952) {
        status = AIS_ERR_BAD_BIT_COUNT;
        return;
    }

    std::bitset<MAX_BITS> bs;
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    if (!decode_header6(bs)) return; // side effect - sets status

    // Handle all the byte aligned payload
    for (int i=0; i<payload_len/8; i++) {
        const int start = 88+i*8;
        payload.push_back(ubits(bs,start,8));
    }
    const int remainder = payload_len % 8; // FIX: need to handle spare bits!!
    if (remainder > 0) {
        const int start = (payload_len/8) * 8;
        payload.push_back(ubits(bs, start, remainder));
    }
}


bool Ais6::decode_header6(const std::bitset<MAX_BITS> &bs) {
    message_id = ubits(bs, 0, 6);
    if (6 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return false; }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);
    seq = ubits(bs,38,2);
    mmsi_dest = ubits(bs, 40, 30);
    retransmit = !bool(bs[70]);
    spare = bs[71];
    dac = ubits(bs,72,10);
    fi = ubits(bs,82,6);

    return true;
}


void Ais6::print() {
    std::cout << "AIS_addressed_binary_message: " << message_id
              << "\tmmsi: " << mmsi << "\n"
              << "\tseq: " << seq << "\n"
              << "\tmmsi_dest" << mmsi_dest << "\n"
              << "\tretransmit" << retransmit << "\n"
              << "\tspare" << spare << "\n"
              << "\t\tdac: " << dac << "\tfi:" << fi << "\n";
    std::cout << "\tpayload: "; // << std::hex << std::uppercase; // << std::setfill('0') << std::setw(2) << "\n";
    for (std::vector<unsigned char>::iterator i = payload.begin(); i != payload.end(); i++) {
        std::cout << std::hex <<std::setfill('0') << std::setw(2)<< int(*i);
    }
    std::cout << std::dec << std::nouppercase << std::endl;
}
