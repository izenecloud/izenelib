GPP=/usr/bin/g++  -DDEBUG  -D_DEBUG

IZENELIB = ../../../include
WISELIB = /home/wps/workspace/wiselib

INC_PATH = -I $(IZENELIB) -I $(IZENELIB)/3rdparty  -I /usr/local/include/boost-1_36 -I /home/wps/workspace/wiselib/include -I /usr/local/include/  -I $(WISELIB)/include 

LIB_PATH = -lpthread -lrt  /usr/local/lib/libboost_serialization-gcc41-mt.so  -luuid ../../../lib/libfebird.a ../../../lib/libnvwa.a ../../../lib/libprocmeminfo.a -lglog 	-L$(WISELIB)/lib -lwiselib 

CC_OPTIONS= -O3 -Wall -c -g  -fPIC -rdynamic #-DDATA_IO_ALLOW_DEFAULT_SERIALIZE

TARGET =   t_log #t_serialization  t_wrapper #t_map t_wrapper

.cc.o:
	$(GPP) $(CC_OPTIONS) ${INC_PATH} $<

T_MAP_OBJ = t_map.o

T_WRAPPER_OBJ = t_wrapper.o

T_SERIALIZATION_OBJ = t_serialization.o

T_LOG_OBJ = t_log.o

all:$(TARGET)

t_serialization:$(T_SERIALIZATION_OBJ)

	$(GPP) $(T_SERIALIZATION_OBJ) ${LIB_PATH} -o $@ 

t_map:$(T_MAP_OBJ)

	$(GPP) $(T_MAP_OBJ) ${LIB_PATH} -o $@ 
	
t_wrapper:$(T_WRAPPER_OBJ)

	$(GPP) $(T_WRAPPER_OBJ) ${LIB_PATH} -o $@ 
	
t_log:$(T_LOG_OBJ)

	$(GPP) $(T_LOG_OBJ) ${LIB_PATH} -o $@ 

clean:
	rm -rf *.o *~

