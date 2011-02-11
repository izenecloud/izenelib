GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O3 -g -rdynamic -Wall -c -fmessage-length=0 


LIBS := /usr/local/lib/libboost_unit_test_framework-gcc41-mt-1_36.so /usr/local/lib/libboost_serialization-gcc41-mt-1_36.so /usr/local/lib/libboost_system-gcc41-mt-1_36.so /usr/local/lib/libboost_iostreams-gcc41-mt-1_36.so


INC_PATH := -I /usr/local/include/boost-1_36/ -I ../../../../include/ -I ../../../../include/3rdparty/


OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
#./t_main.o


all: amlib get-input



.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<"

get-input: gen_input.o
	$(GPP)  -g -o "get-input"  $^ 
	-rm -f gen_input.o

amlib: $(OBJS)
	$(GPP)  -g -o "map"  $^ $(LIBS) 
	-rm -f $(OBJS)

clean:
	-rm -f $(OBJS)
