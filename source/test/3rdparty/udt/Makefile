C++ = g++

ifndef os
   os = LINUX
endif

ifndef arch
   arch = IA32
endif

IZENELIB = ../../../../include

INC_PATH = -I $(IZENELIB)/3rdparty -I /usr/local/include/boost-1_38

CCFLAGS = -Wall -D$(os) $(INC_PATH) -O2 -finline-functions

ifeq ($(arch), IA32)
   CCFLAGS += -DIA32 #-mcpu=pentiumpro -march=pentiumpro -mmmx -msse
endif


ifeq ($(arch), IA64)
   CCFLAGS += -DIA64
endif


LDFLAGS = ../../../../lib/libudt.a  -lboost_thread-gcc41-mt-1_38

ifeq ($(os), UNIX)
   LDFLAGS += -lsocket
endif

OBJS = client.o server.o

DIR = $(shell pwd)

all: client server

%.o: %.cpp
	$(C++) $(CCFLAGS) $< -c

server: server.o
	$(C++) $^ -o $@ $(LDFLAGS)
client: client.o
	$(C++) $^ -o $@ $(LDFLAGS)

clean:
	rm -f *.o
	rm -f server client
