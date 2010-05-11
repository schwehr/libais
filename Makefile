VERSION:=${shell cat VERSION}

CXXFLAGS :=
#CXXFLAGS += -O3 -funroll-loops -fexpensive-optimizations -DNDEBUG
#CXXFLAGS += -ffast-math 

#CXXFLAGS := -g3 -O0
CXXFLAGS := -g3 -O2
#CXXFLAGS += -D_GLIBCXX_DEBUG  # Appears broken
CXXFLAGS += -Wall
CXXFLAGS += -Wimplicit 
CXXFLAGS += -pedantic 
CXXFLAGS += -W 
CXXFLAGS += -Wredundant-decls
CXXFLAGS += -Werror

#CFLAGS := ${CXXFLAGS}

SRCS := ais.cpp 
SRCS += ais1_2_3.cpp ais4_11.cpp ais5.cpp ais7_13.cpp
#
SRCS += ais14.cpp
#
SRCS += ais18.cpp ais19.cpp
SRCS += ais24.cpp
OBJS := ${SRCS:.cpp=.o}

default:
	@echo
	@echo
	@echo "        Welcome to libais ${VERSION}"
	@echo
	@echo "Build options:"
	@echo
	@echo "  clean    - remove all objects and executables"
	@echo "  all      - build everything"
	@echo "  tar      - create a release source tar using VERSION"
	@echo
	@echo "  python   - build the python module"
	@echo "  libais.a - build a static archive library"
	@echo 
	@echo "Read the README for more information"

all: python libais.a

DIST:=libais-${shell cat VERSION}
TAR:=${DIST}.tar
tar:
	rm -f ${TAR}.bz2 ${TAR}
	rm -rf ${DIST}
	mkdir ${DIST}
	cp -p *.cpp *.h [A-Z]* *.py ${DIST}/
	tar cf ${TAR} ${DIST}
	bzip2 -9 ${TAR}
	rm -rf ${DIST}


# Remove the NDEBUG that python tries to use
python2:
	CFLAGS="-m32 -O3 -funroll-loops -fexpensive-optimizations -ffast-math" /sw/bin/python2.6 setup.py build
#	CFLAGS="-m32 -O0 -g -UNDEBUG" /sw/bin/python2.6 setup.py build

# -D_GLIBCXX_DEBUG  This option does not seem to work for g++ versions 4.[0-2]
python3:
	CFLAGS="-m32 -O0 -g -UNDEBUG" /sw/bin/python3 setup.py build

libais.a: ${OBJS}
	ls ${OBJS}
	ar rv $@ $?
	ranlib $@

clean:
	-rm -rf *.o build *.a

#.cpp.o:
#	${CXX} -c $< ${CXXFLAGS}

test_libais: ${OBJS} test_libais.cpp ais.h
	@echo SRCS: ${SRCS}
	@echo OBJS: ${OBJS}
	${CXX} -o $@ ${OBJS} test_libais.cpp ${CXXFLAGS}

ais_decode_normed: ${OBJS} ais_decode_normed.o ais.h
	@echo SRCS: ${SRCS}
	@echo OBJS: ${OBJS}
	${CXX} -o $@ ${OBJS} ais_decode_normed.o ${CXXFLAGS}

# Hard coded depends
ais.o: ais.h
ais123.o: ais.h
ais4_11.o: ais.h
ais5.o: ais.h
ais7_13.o: ais.h
ais14.o: ais.h
ais18.o: ais.h
ais19.o: ais.h
ais_py.o: ais.h
test_libais.o: ais.h

