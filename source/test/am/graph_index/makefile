GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O3 -g -rdynamic -Wall -c -fmessage-length=0 


LIBS := -lpthread /usr/local/lib/libboost_unit_test_framework-gcc41-mt-1_35.so /usr/local/lib/libboost_thread-gcc41-mt-1_35.so /usr/local/lib/libboost_filesystem-gcc41-mt-1_35.so 

INC_PATH := -I  /usr/local/include/boost-1_35/ -I ../../../../include/ -I ../../../../include/3rdparty/


OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
#./t_main.o


all: izenelib


.cc.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<"

.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<"


izenelib: $(OBJS)
	$(GPP)  -g -o "graph-index"  $^ $(LIBS) 
	-rm -f $(OBJS)

clean:
	-rm -f $(OBJS)
