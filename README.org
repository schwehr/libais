Library for decoding maritime Automatic Identification System messages.

http://en.wikipedia.org/wiki/Automatic_Identification_System

Building with legacy Makefile:

make -f Makefile-custom test


Building with python:

python setup.py build
python setup.py install


Building with CMake

cmake .
make


Usage:

ipython
import ais
ais.decode('15PIIv7P00D5i9HNn2Q3G?wB0t0I', 0)
ais.decode('402u=TiuaA000r5UJ`H4`?7000S:', 0)
ais.decode('55NBjP01mtGIL@CW;SM<D60P5Ld000000000000P0`<3557l0<50@kk@K5h@00000000000', 0)
