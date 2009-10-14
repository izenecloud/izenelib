GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O3 -g -rdynamic -Wall -c -fmessage-length=0 


LIBS := -lpthread /usr/local/lib/libboost_unit_test_framework-gcc41-mt-1_35.so /usr/local/lib/libboost_thread-gcc41-mt-1_35.so /usr/local/lib/libboost_filesystem-gcc41-mt-1_35.so /usr/local/lib/libboost_date_time-gcc41-mt.so /usr/local/lib/libboost_serialization-gcc41-mt.so

INC_PATH := -I  /usr/local/include/boost-1_35/ -I ../../../../include/ -I ../../../../include/3rdparty/


OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
#./t_main.o


all: tg izenelib


.cc.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<"

.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<"

tg: tg_algo.o
	$(GPP)  -g -o "tg-algo"  $^ 
	-rm -f tg_algo.o

izenelib: $(OBJS)
	$(GPP)  -g -o "graph-index"  $^ $(LIBS) 
	-rm -f $(OBJS)

clean:
	-rm -f $(OBJS)
