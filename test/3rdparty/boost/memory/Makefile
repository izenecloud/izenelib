GPP=/usr/bin/g++

IZENELIB = ../../../../include

IZENELIB = ../../../../include/3rdparty

INC_PATH = -I $(IZENELIB) -I /usr/local/include/boost-1_39

LIB_PATH = -lpthread -lrt 

CC_OPTIONS= -O3 -Wall -c -g  -fPIC -rdynamic 

TARGET = t_memory 

.cc.o:
	$(GPP) $(CC_OPTIONS) ${INC_PATH} $<

T_MEMORY_OBJ = t_memory.o

T_MEMORY_PERFORM_OBJ = t_memory_perform.o

all:$(TARGET)

t_memory:$(T_MEMORY_OBJ)

	$(GPP) $(T_MEMORY_OBJ) ${LIB_PATH} -o $@ 

clean:
	rm -rf *.o *~ $(TARGET)

