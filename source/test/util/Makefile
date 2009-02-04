GPP=/usr/bin/g++

IZENELIB = ../../../include

INC_PATH = -I $(IZENELIB) -I $(IZENELIB)/3rdparty  -I /usr/local/include/boost-1_37

LIB_PATH = -lpthread -lrt 

CC_OPTIONS= -O3 -Wall -c -g  -fPIC -rdynamic 

TARGET = t_map

.cc.o:
	$(GPP) $(CC_OPTIONS) ${INC_PATH} $<

T_MAP_OBJ = t_map.o


all:$(TARGET)

t_map:$(T_MAP_OBJ)

	$(GPP) $(T_MAP_OBJ) ${LIB_PATH} -o $@ 

clean:
	rm -rf *.o *~

