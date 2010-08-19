GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O3 -g -Wall -c  -fmessage-length=0 -DDEBUG -pg

LIBS =    /usr/lib/libboost_thread-mt.so   /usr/lib/libboost_unit_test_framework-mt.so   /usr/lib/libboost_serialization-mt.so   ../../../../lib/libprocmeminfo.so	\
	   ../../../../lib/libfebird.so -L  ../../../../lib/  -lizene_log  -lprocmeminfo  -L /home/wps/wps-backup/codebase/sf1-r/wiselib/lib -lwiselib -L /usr/local/lib -lglog

INC_PATH := -I  /usr/include -I ../../../../include/ -I ../../../../include/3rdparty/ -I/home/wps/wps-backup/codebase/sf1-r/wiselib/include
SDB_HASH_OBJ  := t_sdb_hash.o 

SDB_SH_OBJ := t_sh.o

AM_OBJ := t_am.o

 
FLUSH_OBJ := t_flush.o 


OBJS := $(patsubst %.cc, %.o, $(wildcard *.cc))

.cc.o:

	$(GPP) $(INC_PATH) $(CC_OPTIONS) $< 
.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  $< 	


all: test t_flush t_sh  t_sdb_hash t_am

test:  $(OBJS)
	$(GPP) $(OBJS) $(LIBS)  -o $@ -pg  	
	
t_flush:  $(FLUSH_OBJ)
	$(GPP) $(FLUSH_OBJ) $(LIBS)  -o $@ -pg	

t_sdb_hash:  $(SDB_HASH_OBJ)
	$(GPP) $(SDB_HASH_OBJ) $(LIBS)  -o $@ -pg		
	
t_sh:  $(SDB_SH_OBJ)
	$(GPP) $(SDB_SH_OBJ) $(LIBS)  -o $@ -pg	
	
t_am:  $(AM_OBJ)
	$(GPP) $(AM_OBJ) $(LIBS)  -o $@ -pg	
	
t_am1:  $(AM_OBJ1)
	$(GPP) $(AM_OBJ1) $(LIBS)  -o $@ -pg	


clean:
	rm -f *.o  t_sh t_sdb_hash *.dat
