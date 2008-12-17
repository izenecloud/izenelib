GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O0 -g -Wall -c -fmessage-length=0 -DDEBUG -pg

LIBS =     /usr/local/lib/libboost_thread-gcc41-mt.so  /usr/local/lib/libboost_unit_test_framework-gcc41-mt.so  /usr/local/lib/libboost_serialization-gcc41-mt.so


INC_PATH := -I  /usr/local/include/boost-1_35/ -I ../../../../include/ -I ../../../../include/3rdparty/


OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))

all: t_btree


.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<"

t_btree:  $(OBJS)
	$(GPP) $(OBJS) $(LIBS) -o $@ -pg
	#-rm -f $(OBJS)

clean:
	-rm -f $(OBJS) t_btree sdb.dat
