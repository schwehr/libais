#include "ais.h"

//using namespace std;

#define UNUSED __attribute((__unused__))

int main(UNUSED int argc, UNUSED char* argv[]) {
    build_nmea_lookup();

    ////////////////////////////////////////
    // 1
    ////////////////////////////////////////

    if (false) {
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

    if (true) {
        // Causes bus error
        // !AIVDM,1,1,,A,74i:pT000000,0*52,b003669977,1273190417
        CHECKPOINT;
        {Ais7_13 msg("74i:pT000000"); msg.print();}
        CHECKPOINT;

        {Ais7_13 msg("75Mu6d0P17IP?PfGSC29WOvb0<14"); msg.print();}

        CHECKPOINT;
        { Ais7_13 msg("74eGSe@0owtf"); msg.print(); }

        CHECKPOINT;
        { 
            Ais7_13 msg("74eGSe@0owt");
            assert(msg.had_error());
            std::cout << "expect error " << AIS_ERR_BAD_BIT_COUNT << ": " << msg.get_error() << std::endl;
            //msg.print();
        }
    }


    //////////////////////////////////////////////////////////////////////
    // 14 - Safety Broadcast
    //////////////////////////////////////////////////////////////////////

    if (false) {
        // From GPSD - !AIVDM,1,1,,A,>5?Per18=HB1U:1@E=B0m<L,2*51
        { 
            Ais14 msg(">5?Per18=HB1U:1@E=B0m<L"); 
            if (!msg.had_error()) msg.print(); else std::cout<<"FAILED 14 1\n";
            std::cout << "\n"; 
        }
        // From GPSD - !AIVDM,1,1,,A,>3R1p10E3;;R0USCR0HO>0@gN10kGJp,2*7F
        { 
            Ais14 msg(">3R1p10E3;;R0USCR0HO>0@gN10kGJp"); 
            if (!msg.had_error()) msg.print(); else std::cout<<"FAILED\n";
            std::cout << "\n"; 
        }
        // From GPSD - !AIVDM,1,1,,A,>4aDT81@E=@,2*2E
        { 
            Ais14 msg(">4aDT81@E=@"); 
            if (!msg.had_error()) msg.print(); else std::cout<<"FAILED\n";
            std::cout << "\n"; 
        }
    }

    //////////////////////////////////////////////////////////////////////
    // 18 - Class B
    //////////////////////////////////////////////////////////////////////

    if (false) {
        // !AIVDM,1,1,,A,B5N3SRP0FFJFC`4:CQDFKwiP200>,0*75,b003669952,1272413103
        { Ais18 msg("B5N3SRP0FFJFC`4:CQDFKwiP200>"); msg.print(); std::cout << std::endl; }
        // !AIVDM,1,1,,B,B5NGjdP03MkEvV6vJTN`SwuUoP06,0*08,b003669703,1272412832
        { Ais18 msg("B5NGjdP03MkEvV6vJTN`SwuUoP06"); msg.print(); std::cout << std::endl; }
        // !AIVDM,1,1,,B,BU2K5MP005kN8WVSrcP03wb5oP00,0*44,d-077,S0791,t000021.00,T21.09527304,r13SCDS1,1272412822
        { Ais18 msg("B5MtL4P00FK?Pa4I98`G`uS0200>"); msg.print(); std::cout << std::endl; }
        // !AIVDM,1,1,,B,B5MtL4P00FK?Pa4I98`G`uS0200>,0*61,d-079,S0269,t000007.00,T07.17528574,r08RROB1,1272412808
        { Ais18 msg(""); msg.print(); }
   

        // !AIVDM,1,1,,B,Bov9gKvWbf;juI`NN@CeG1DF;3Gs,0*6D,d-126,S2121,t101756.00,T56.56587008,r09SMRQ1,1272449876
        { Ais18 msg("Bov9gKvWbf;juI`NN@CeG1DF;3Gs"); msg.print(); std::cout << std::endl; }
        // !AIVDM,1,1,,B,B@MMSv=E@JJd``ksAupQpveN`VH?,0*3F,d-126,S1407,t130537.00,T37.53680769,r09SSAG1,1272459940
        { Ais18 msg("B@MMSv=E@JJd``ksAupQpveN`VH?"); msg.print(); std::cout << std::endl; }

    }



    //////////////////////////////////////////////////////////////////////
    // 19 - Class B equip pos report
    //////////////////////////////////////////////////////////////////////

    if (false) {
        // !AIVDM,1,1,,B,C5N3SRP0IFJKmE4:v6pBGw`@62PaLELTBJ:V00000000S0D:R220,0*3A,b003669952,1272415462
        { 
            Ais19 msg("C5N3SRP0IFJKmE4:v6pBGw`@62PaLELTBJ:V00000000S0D:R220"); 
            if (!msg.had_error()) msg.print(); else std::cout<<"FAILED\n";
            std::cout << "\n"; 
        }

        // !AIVDM,1,1,2,B,C5MtL4al06K?Pa4I99@G`us>@2fF0000000000000000?P000020,0*73,d-080,S2043,t013454.00,T54.48188603,r08RROB1,1272418496
        { 
            Ais19 msg("C5MtL4al06K?Pa4I99@G`us>@2fF0000000000000000?P000020"); 
            if (!msg.had_error()) msg.print(); else std::cout<<"FAILED\n";
            std::cout << "\n"; 
        }

    }

    //////////////////////////////////////////////////////////////////////
    // 24 - Class B static data msgs A and B
    //////////////////////////////////////////////////////////////////////

    if (true) {
        // !AIVDM,1,1,,B,H5NHcTP<51@4TrM>10584@U<D00,2*77,x337805,b003669710,1241895000
        { 
            Ais24 msg("H5NHcTP<51@4TrM>10584@U<D00"); 
            if (!msg.had_error()) msg.print(); else std::cout<<"FAILED\n";
            std::cout << "\n"; 
        }
        // !AIVDM,1,1,,B,HU2K5NTn13BijklG44oppk103210,0*06,s23294,d-114,T44.21624911,x731910,r13RSMT1,1241894986
        { 
            Ais24 msg("HU2K5NTn13BijklG44oppk103210"); 
            if (!msg.had_error()) msg.print(); else std::cout<<"FAILED\n";
            std::cout << "\n"; 
        }


    }

    //////////////////////////////
    // DONE
    //////////////////////////////

    return 0;

}
