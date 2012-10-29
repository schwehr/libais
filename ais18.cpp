// Class B position report - 18 "B"

#include "ais.h"

Ais18::Ais18(const char *nmea_payload, const size_t pad) {
    assert(nmea_payload);
    assert(pad < 6);
    init();

    if (pad != 0 || strlen(nmea_payload) != 28) { status = AIS_ERR_BAD_BIT_COUNT; return; }

    std::bitset<168> bs;
    status = aivdm_to_bits(bs, nmea_payload);
    if (had_error()) return;

    message_id = ubits(bs, 0, 6);
    if (18 != message_id) { status = AIS_ERR_WRONG_MSG_TYPE; return; }
    repeat_indicator = ubits(bs, 6, 2);
    mmsi = ubits(bs, 8, 30);

    spare = ubits(bs, 38, 8);
    sog = ubits(bs, 46, 10) / 10.;

    position_accuracy = bs[56];
    x = sbits(bs, 57, 28) / 600000.;
    y = sbits(bs, 85, 27) / 600000.;

    cog = ubits(bs, 112, 12) / 10.;
    true_heading = ubits(bs, 124, 9);
    timestamp = ubits(bs, 133, 6);
    spare2 = ubits(bs, 139, 2);
    unit_flag = bs[141];
    display_flag = bs[142];
    dsc_flag = bs[143];
    band_flag = bs[144];
    m22_flag = bs[145];
    mode_flag = bs[146];
    raim = bs[147];
    commstate_flag = bs[148];  // 0 SOTDMA, 1 ITDMA

    // TODO(schwehr): set all to -1 and set valids to NOT!

    if (1 == unit_flag) {
        // CS - carrier sense - fixed commstate payload of 1100000000000000110
        int commstate = ubits(bs, 149, 19);
        if (393222 != commstate) {
            // TODO(schwehr): is this the right value?
            // TODO(schwehr): return an error?
        }
    } else {
        sync_state = ubits(bs, 149, 2);
        if (0 == commstate_flag) {
            // SOTDMA
            slot_timeout = ubits(bs, 151, 3);

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
            case 2:  // FALLTHROUGH
            case 4:  // FALLTHROUGH
            case 6:
                slot_number = ubits(bs, 154, 14);
                slot_number_valid = true;
                break;
            case 3:  // FALLTHROUGH
            case 5:  // FALLTHROUGH
            case 7:
                received_stations = ubits(bs, 154, 14);
                received_stations_valid = true;
                break;
            default:
                assert(false);
            }

        } else {
            // ITDMA
            slot_increment = ubits(bs, 151, 13);
            slot_increment_valid = true;

            slots_to_allocate = ubits(bs, 164, 3);
            slots_to_allocate_valid = true;

            keep_flag = bs[167];
            keep_flag_valid = true;
        }
    }
}
