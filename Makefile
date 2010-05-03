
#CXXFLAGS += -O3 -funroll-loops -fexpensive-optimizations -DNDEBUG
#CXXFLAGS += -ffast-math 

CXXFLAGS += -g3 -O0
CXXFLAGS += -D_GLIBCXX_DEBUG
CXXFLAGS := -Wall
CXXFLAGS += -Wimplicit 
CXXFLAGS += -pedantic 
CXXFLAGS += -W 
CXXFLAGS += -Wredundant-decls
CXXFLAGS += -Werror


SRCS:= ais.cpp ais123.cpp ais4_11.cpp ais5.cpp
OBJS:=${SRCS:.cpp=.o}

# Remove the NDEBUG that python tries to use
default:
	CFLAGS="-m32 -O0 -g -D_GLIBCXX_DEBUG -UNDEBUG" /sw/bin/python setup.py build

libais.a: ${OBJS}
	ls ${OBJS}
	ar rv $@ $?
	ranlib $@

clean:
	-rm *.o
	-rm -rf build
	-rm *.a