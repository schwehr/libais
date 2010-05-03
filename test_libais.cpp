#include "ais.h"

#define UNUSED __attribute((__unused__))

int main(UNUSED char *argc, UNUSED char* argv[]) {
    build_nmea_lookup();
    {
        Ais7_13 msg("75Mu6d0P17IP?PfGSC29WOvb0<14");
        msg.print();
    }
    CHECKPOINT;
    {
        Ais7_13 msg("74eGSe@0owtf");
        msg.print();
    }
    return 0;
}
