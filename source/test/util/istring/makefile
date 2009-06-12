GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O3 -g -Wall -c -fmessage-length=0 


LIBS := ./libiconv.so /usr/local/lib/libboost_unit_test_framework-gcc41-mt-1_36.so /usr/local/lib/libboost_serialization-gcc41-mt-1_36.so /usr/local/lib/libboost_system-gcc41-mt-1_36.so /usr/local/lib/libboost_iostreams-gcc41-mt-1_36.so


INC_PATH := -I /usr/local/include/boost-1_36/ -I ../../../../include/ -I ../../../../include/3rdparty/ -I ../../../../include/3rdparty/libiconv-1.13/include/


OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
#./t_main.o


all: vstring bbtstring dstring 



.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<"



vstring: t_vstring.o t_master_suite.o
	$(GPP)  -g -o "vstring"  $^ $(LIBS) 
	-rm -f t_vstring.o

bbtstring: t_bbtstring.o t_master_suite.o
	$(GPP)  -g -o "bbtstring"  $^ $(LIBS) 
	-rm -f t_bbtstring.o

dstring: t_dstring.o t_master_suite.o
	$(GPP)  -g -o "dstring"  $^ $(LIBS) 
	-rm -f t_dstring.o t_master_suite.o

clean:
	-rm -f $(OBJS)
