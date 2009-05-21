GPP=/usr/bin/g++

IZENELIB = ../../../../../include

INC_PATH = -I $(IZENELIB) -I /usr/local/include/boost-1_37

LIB_PATH = -lpthread -lrt ../../../../../lib/libboost_memory.a

CC_OPTIONS= -O3 -Wall -c -g  -fPIC -rdynamic 

TARGET = t_memory t_memory_perform

.cc.o:
	$(GPP) $(CC_OPTIONS) ${INC_PATH} $<

T_MEMORY_OBJ = t_memory.o

T_MEMORY_PERFORM_OBJ = t_memory_perform.o

all:$(TARGET)

t_memory:$(T_MEMORY_OBJ)

	$(GPP) $(T_MEMORY_OBJ) ${LIB_PATH} -o $@ 

t_memory_perform:$(T_MEMORY_PERFORM_OBJ)

	$(GPP) $(T_MEMORY_PERFORM_OBJ) ${LIB_PATH} -o $@ 

clean:
	rm -rf *.o *~ $(TARGET)

