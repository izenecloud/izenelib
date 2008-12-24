GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O3 -g -Wall -c -fmessage-length=0 


LIBS := -lpthread   -lboost_unit_test_framework-gcc41-mt
#/usr/local/lib/libboost_thread-gcc41-mt-1_37.so
#/usr/local/lib/libboost_serialization-gcc41-mt-1_37.so /usr/local/lib/libboost_system-gcc41-mt-1_37.so /usr/local/lib/libboost_iostreams-gcc41-mt-1_37.so
INC_PATH := -I  /usr/local/include/boost-1_37/ -I ../../../../include/ -I ../../../../include/3rdparty/


OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
#./t_main.o


all: amlib



.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<"

amlib: $(OBJS)
	$(GPP)  -g -o "skip-list"  $^ $(LIBS) 
	-rm -f $(OBJS)

clean:
	-rm -f $(OBJS)
