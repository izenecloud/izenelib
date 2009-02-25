GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O3 -g -rdynamic -Wall -c -fmessage-length=0 


LIBS := -lpthread /usr/local/lib/libboost_unit_test_framework-gcc41-mt-1_35.so /usr/local/lib/libboost_thread-gcc41-mt-1_35.so /usr/local/lib/libboost_filesystem-gcc41-mt-1_35.so /usr/local/lib/libboost_date_time-gcc41-mt.so /usr/local/lib/libboost_serialization-gcc41-mt.so

INC_PATH := -I  /usr/local/include/boost-1_35/ -I ../../../../include/ -I ../../../../include/3rdparty/


OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
#./t_main.o


all: amlib get-input



.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<"

get-input: gen_input.o
	$(GPP)  -g -o "get-input"  $^ 
	-rm -f gen_input.o

amlib: $(OBJS)
	$(GPP)  -g -o "dp-ht"  $^ $(LIBS) 
	-rm -f $(OBJS)

clean:
	-rm -f $(OBJS)
