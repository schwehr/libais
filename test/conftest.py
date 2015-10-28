"""pytest fixtures"""

import os

import pytest


@pytest.fixture(scope='function')
def typeexamples_nmea_path():
    return os.path.join('test', 'data', 'typeexamples.nmea')


@pytest.fixture(scope='function')
def bare_nmea():
    return """
$GPZDA,203003.00,12,07,2009,00,00,*47
!AIVDM,1,1,,B,23?up2001gGRju>Ap:;R2APP08:c,0*0E
!BSVDM,1,1,,A,15Mj23`PB`o=Of>KjvnJg8PT0L2R,0*7E
!SAVDM,1,1,,B,35Mj2p001qo@5tVKLBWmIDJT01:@,0*33
!AIVDM,1,1,,A,B5NWV1P0<vSE=I3QdK4bGwoUoP06,0*4F
!SAVDM,1,1,,A,403Owi1utn1W0qMtr2AKStg020S:,0*4B
!SAVDM,2,1,4,A,55Mub7P00001L@;SO7TI8DDltqB222222222220O0000067<0620@jhQDTVG,0*43
!SAVDM,2,2,4,A,30H88888880,2*49
""".strip()
