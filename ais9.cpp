// Since 2010-May-14

#include "ais.h"

Ais9::Ais9(const char *nmea_payload) {
    assert(nmea_payload);
    init();

    if (strlen(nmea_payload) != 168/6) {
        status = AIS_ERR_BAD_BIT_COUNT;
        return;
    }

    std::bitset<168> bs; // 1 slot
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (9 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
    repeat_indicator = ubits(bs,6,2);
    mmsi = ubits(bs,8,30);
    alt = ubits(bs,38, 12);
    sog = ubits(bs,50,10) / 10.;

    position_accuracy = ubits(bs,60,1);
    x = sbits(bs, 61, 28) / 600000.;
    y = sbits(bs, 89, 27) / 600000.;

    cog = ubits(bs, 116, 12) / 10.;
    timestamp = ubits(bs, 128, 6);
    alt_sensor = int(bs[129]); // ubits(bs, 129, 1);
    spare = ubits(bs, 136, 7);
    dte = int(bs[143]);//ubits(bs, 143, 1);
    spare2 = ubits(bs, 146, 3);
    raim = bool(bs[147]);
    commstate_flag = bs[148];  // 0 SOTDMA, 1 ITDMA

#ifndef NDEBUG
    slot_timeout = -1;
    received_stations = slot_number = utc_hour = utc_min = utc_spare -1;
    slot_offset = slot_increment = slots_to_allocate = -1;
    keep_flag = false;
#endif    
    slot_timeout_valid = false;
    received_stations_valid = slot_number_valid = utc_valid = false;
    slot_offset_valid = slot_increment_valid = slots_to_allocate_valid = keep_flag_valid = 0;

    if (0 == commstate_flag) {//unit_flag) {

        // SOTDMA
        slot_timeout = ubits(bs,151,3);
        slot_timeout_valid = true;

        switch (slot_timeout) {
        case 0:
            slot_offset = ubits(bs, 154, 14);
            slot_offset_valid = true;
            break;
        case 1:
            utc_hour = ubits(bs, 154, 5); 
            utc_min = ubits(bs, 159, 7);
            utc_spare = ubits(bs, 166, 2);
            utc_valid = true;
            break;
        case 2: // FALLTHROUGH
        case 4: // FALLTHROUGH
        case 6:
            slot_number = ubits(bs, 154, 14);
            slot_number_valid = true;
            break;
        case 3: // FALLTHROUGH
        case 5: // FALLTHROUGH
        case 7:
            received_stations = ubits(bs, 154, 14);
            received_stations_valid = true;
            break;
        default:
            assert (false);
        }

    } else {
        // ITDMA
        slot_increment = ubits(bs, 151, 13);
        slot_increment_valid = true;

        slots_to_allocate = ubits(bs, 164, 3);
        slots_to_allocate_valid = true;

        keep_flag = bool(bs[167]);
        keep_flag_valid = true;
    }
}


void Ais9::print() {
    std::cout << "SAR position: "  << message_id << "\n"
              << "\tmmsi: " << mmsi << " repeat: " << repeat_indicator << "\n"
              << "\talt: " << alt << " (m)  alt_sensor: " << alt_sensor << "\n"
              << "\tposition_accuracy: " << position_accuracy << "\n"
              << "\tlocation: " << x << " " << y << "\n"
              << "\ttimestamp: " << timestamp << "spare: " << spare << "\n"
              << "\tdte: " << dte << "spare2: " << spare2 << "\n"
              << "\traim: " << (raim?"true":"false") << "\n"
              << "\tcommstate_flag: " << commstate_flag 
              << std::endl;
    // SOTDMA
    if (slot_timeout_valid) std::cout << "\t\tslot_timeout: " << slot_timeout << "\n";
    if (slot_offset_valid) std::cout << "\t\tslot_offset" << slot_offset << "\n";
    if (utc_valid) std::cout <<"\t\tUTC: " << utc_hour << ":"<<utc_min << " spare:" << utc_spare << "\n";
    if (slot_number_valid) std::cout << "\t\tslot_number: " << slot_number << "\n;";
    if (received_stations_valid) std::cout << "\t\treceived_stations: " << received_stations << "\n";
    
    // ITDMA
    if (slot_increment_valid) std::cout << "\t\tslot_increment: " << slot_increment << "\n";
    if (slots_to_allocate_valid) std::cout << "\t\tslots_to_allocate: " << slots_to_allocate << "\n";
    if (keep_flag_valid) std::cout << "\t\tkeep_flag: " << keep_flag << "\n";

}
