#include "ais.h"

using namespace std;

#define UNUSED __attribute((__unused__))

int main(UNUSED char *argc, UNUSED char* argv[]) {
    build_nmea_lookup();

    ////////////////////////////////////////
    // 1
    ////////////////////////////////////////

    if (true) {
        // !AIVDM,1,1,,B,15Mq4J0P01EREODRv4@74gv00HRq,0*72,b003669970,1272412824
        Ais1_2_3("15Mq4J0P01EREODRv4@74gv00HRq");
        // !AIVDM,1,1,,B,1EN2?oWP00ER5SLRvNPi9gwl0000,0*51,b003669970,1272412824
        Ais1_2_3("");
        // !AIVDM,1,1,,B,15N3QPPP0dI?uu>@smtj8wv028Rs,0*23,b003669977,1272412827
        Ais1_2_3("");
        // !AIVDM,1,1,,B,15N3J`P01tqr<CDJlP1DKSUn0<04,0*43,b003665001,1272412825
        Ais1_2_3("15N3J`P01tqr<CDJlP1DKSUn0<04");

        // Bad sized packets
        Ais1_2_3("15N3J`P01tqr<CDJlP1DKSUn0<0400000"); // too large
        Ais1_2_3("15N3J`P01tqr<CDJlP1DKSUn"); // too small
        Ais1_2_3("95N3J`P01tqr<CDJlP1DKSUn0<04"); // wrong message type in the 1st char
        std::cout << "Expecting bad characters 'zXYZ[\\]^_'" << std::endl;
        Ais1_2_3("1zXYZ[\\]^_tr<CDJlP1DKSUn0<04"); // invalid character

    }

    ////////////////////////////////////////
    // 7 and 13
    ////////////////////////////////////////

    if (false) {
        Ais7_13 msg("75Mu6d0P17IP?PfGSC29WOvb0<14");
        msg.print();
    }
    if (false) {
        CHECKPOINT;
        Ais7_13 msg("74eGSe@0owtf");
        msg.print();
    }
    if (false) {
        CHECKPOINT;
        Ais7_13 msg("74eGSe@0owt");
        assert(msg.had_error());
        cout << "expect error " << AIS_ERR_BAD_BIT_COUNT << ": " << msg.get_error() << endl;
        //msg.print();
    }
    return 0;
}
