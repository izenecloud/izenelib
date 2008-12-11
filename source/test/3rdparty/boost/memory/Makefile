GPP=/usr/bin/g++

IZENELIB = ~/CodeBase/izenelib/include

INC_PATH = -I $(IZENELIB)/3rdparty -I /usr/local/include/boost-1_37

LIB_PATH = -lpthread

CC_OPTIONS= -O3 -Wall -c -g  -fPIC -rdynamic 

.cc.o:
	$(GPP) $(CC_OPTIONS) ${INC_PATH} $<

T_MEMORY_OBJ = t_memory.o


t_memory:$(T_MEMORY_OBJ)

	$(GPP) $(T_MEMORY_OBJ) ${LIB_PATH} -o $@ 

clean:
	rm -rf *.o *~

