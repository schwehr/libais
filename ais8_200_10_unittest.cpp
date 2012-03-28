#include <gtest/gtest.h>
#include "ais.h"


// samples from aishub
// 244630706,200,Inland,10,Voyage,id=02326687,length=819.1,beam=71,83aC4dPj2d<dteef=wwuS08r01H0
// 244100244,200,Inland,10,Voyage,id=H220@2@@,length=129.6,beam=5.6,83`jdU0j2R<dd0<P02R0L0<h3800
// 244660610,200,Inland,10,Voyage,id=2315759 ,length=71,beam=7.4,83aDqPPj2ddtMMuNH1HhU?aE9?00
// 211484674,200,Inland,10,Voyage,id=@@@@@@@@,length=172,beam=11.3,839d00Pj2P00000003G0pga@6@P0
// 244630026,200,Inland,10,Voyage,id=02327503,length=110,beam=13.5,83aC22Pj2d<dteuL<j9Q3gbU0000
// 244660097,200,Inland,10,Voyage,id=02315118,length=85,beam=10,83aDoP@j2d<dtMLLN1b@j?bP8hP0
// 211502930,200,Inland,10,Voyage,id=@@@@@@@@,length=55,beam=10,839e7DPj2P000000014hj@O56T00
// 211493170,200,Inland,10,Voyage,id=04016420,length=85,beam=8,839dQ<Pj2d=<<Me<d1b@`?aE3hP0


TEST(ais8dac200fi10, InlandVoyage) 
{
    const char * payloads[] = {"83aC4dPj2d<dteef=wwuS08r01H0","83`jdU0j2R<dd0<P02R0L0<h3800",
    "83aDqPPj2ddtMMuNH1HhU?aE9?00","839d00Pj2P00000003G0pga@6@P0","83aC22Pj2d<dteuL<j9Q3gbU0000",
    "83aDoP@j2d<dtMLLN1b@j?bP8hP0","839e7DPj2P000000014hj@O56T00","839dQ<Pj2d=<<Me<d1b@`?aE3hP0"};
    for(int i = 0; i < 8; ++i)
    {
        Ais8_200_10 msg(payloads[i]);
        ASSERT_EQ(AIS_OK, msg.get_error());
    }
}
