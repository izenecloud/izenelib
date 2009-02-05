CC=			gcc
CXX=		g++
CFLAGS=		-g -Wall -O2 -fomit-frame-pointer
CXXFLAGS=	$(CFLAGS)
OBJS=		t_rde_hashmap.o 
DFLAGS=
IZENELIB = ../../../../../include
INCLUDES=	-I $(IZENELIB)/3rdparty/am/
LIBS=		

.cc.o:
		$(CXX) -c $(INCLUDES) $(CXXFLAGS) $(DFLAGS) $< -o $@

all:test

test:$(OBJS)
		$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
		rm -f *.o *~ test
