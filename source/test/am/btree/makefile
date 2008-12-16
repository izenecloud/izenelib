GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O0 -g -Wall -c -fmessage-length=0 

#LIBS := -lpthread   -lboost_unit_test_framework-gcc41-mt -libboost_serialization-gcc41-mt
LIBS =     /usr/local/lib/libboost_thread-gcc41-mt.so  /usr/local/lib/libboost_unit_test_framework-gcc41-mt.so  /usr/local/lib/libboost_serialization-gcc41-mt.so

#/usr/local/lib/libboost_thread-gcc41-mt-1_37.so
#/usr/local/lib/libboost_serialization-gcc41-mt-1_37.so /usr/local/lib/libboost_system-gcc41-mt-1_35.so /usr/local/lib/libboost_iostreams-gcc41-mt-1_37.so

INC_PATH := -I  /usr/local/include/boost-1_35/ -I ../../../include/ -I ../../../include/3rdparty/

OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
#./t_main.o


all: amlib


.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<"

amlib: $(OBJS)
	$(GPP)  -g -o "t-btree"  $^ $(LIBS) 
	-rm -f $(OBJS)

clean:
	-rm -f $(OBJS)
