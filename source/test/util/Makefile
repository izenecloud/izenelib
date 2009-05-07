GPP=/usr/bin/g++

IZENELIB = ../../../include

INC_PATH = -I $(IZENELIB) -I $(IZENELIB)/3rdparty  -I /usr/local/include/boost-1_36

LIB_PATH = -lpthread -lrt  /usr/local/lib/libboost_serialization-gcc41-mt.so  -luuid  ../../../lib/libfebird.a

CC_OPTIONS= -O3 -Wall -c -g  -fPIC -rdynamic -DDATA_IO_ALLOW_DEFAULT_SERIALIZE

TARGET =  t_serialization #t_map t_wrapper

.cc.o:
	$(GPP) $(CC_OPTIONS) ${INC_PATH} $<

T_MAP_OBJ = t_map.o

T_WRAPPER_OBJ = t_wrapper.o

T_SERIALIZATION_OBJ = t_serialization.o

all:$(TARGET)

t_serialization:$(T_SERIALIZATION_OBJ)

	$(GPP) $(T_SERIALIZATION_OBJ) ${LIB_PATH} -o $@ 

t_map:$(T_MAP_OBJ)

	$(GPP) $(T_MAP_OBJ) ${LIB_PATH} -o $@ 
	
t_wrapper:$(T_WRAPPER_OBJ)

	$(GPP) $(T_WRAPPER_OBJ) ${LIB_PATH} -o $@ 

clean:
	rm -rf *.o *~

