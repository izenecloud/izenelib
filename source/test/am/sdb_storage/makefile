GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O3 -g -Wall -c  -fmessage-length=0 -DDEBUG -pg

LIBS =  -L /usr/local/lib  -lboost_thread  -lboost_unit_test_framework   -lboost_serialization   ../../../../lib/libprocmeminfo.so	\
	   ../../../../lib/libfebird.so	 -lglog -L ../../../../../wiselib/lib -L /usr/local/lib/ -lboost_filesystem -lboost_iostreams

INC_PATH := -I  /usr/local/include/boost-1_36/ -I ../../../../include/ -I ../../../../include/3rdparty/   -I ../../../../../wiselib/include


SDB_HASH_OBJ  := t_sdb_storage.o 

SDB_SH_OBJ := t_sh.o

AM_OBJ := t_am.o

AM_OBJ1 := t_am1.o
 
OBJS := $(patsubst %.cc, %.o, $(wildcard *.cc)) 

all: t_sdb_storage t_am 

.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  $< 
	
.cc.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  $< 

test:  $(OBJS)
	$(GPP) $(OBJS) $(LIBS)  -o $@

t_sdb_storage:  $(SDB_HASH_OBJ)
	$(GPP) $(SDB_HASH_OBJ) $(LIBS)  -o $@ -pg		
	
t_sh:  $(SDB_SH_OBJ)
	$(GPP) $(SDB_SH_OBJ) $(LIBS)  -o $@ -pg	
	
t_am:  $(AM_OBJ)
	$(GPP) $(AM_OBJ) $(LIBS)  -o $@ -pg	
	
t_am1:  $(AM_OBJ1)
	$(GPP) $(AM_OBJ1) $(LIBS)  -o $@ -pg	


clean:
	rm -f *.o  t_sdb_storage *.dat *.mm_0 *.mm_1
