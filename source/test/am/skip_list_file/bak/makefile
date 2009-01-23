GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O3 -g -Wall -c  -fmessage-length=0 -DDEBUG -pg

LIBS =     /usr/local/lib/libboost_thread-gcc41-mt.so  /usr/local/lib/libboost_unit_test_framework-gcc41-mt.so  /usr/local/lib/libboost_serialization-gcc41-mt.so


INC_PATH := -I  /usr/local/include/boost-1_35/ -I ../../../../include/ -I ../../../../include/3rdparty/


OBJS := t_slf.o

all: t_slf

.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  $< 

t_slf:  $(OBJS)
	$(GPP) $(OBJS) $(LIBS) -o $@ -pg	

clean:
	rm -f $(OBJS) t_slf sdb.dat
