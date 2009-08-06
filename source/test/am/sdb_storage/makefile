GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O3 -g -Wall -c  -fmessage-length=0 -DDEBUG -pg

LIBS =    /usr/local/lib/libboost_thread-gcc41-mt.so   /usr/local/lib/libboost_unit_test_framework-gcc41-mt.so   /usr/local/lib/libboost_serialization-gcc41-mt.so   ../../../../lib/libprocmeminfo.a	\
	   ../../../../lib/libfebird.a	     -lglog 

INC_PATH := -I  /usr/local/include/boost-1_36/ -I ../../../../include/ -I ../../../../include/3rdparty/


SDB_HASH_OBJ  := t_sdb_storage.o 

SDB_SH_OBJ := t_sh.o

AM_OBJ := t_am.o

AM_OBJ1 := t_am1.o
 

all: t_sdb_storage t_am 

.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  $< 

t_sdb_storage:  $(SDB_HASH_OBJ)
	$(GPP) $(SDB_HASH_OBJ) $(LIBS)  -o $@ -pg		
	
t_sh:  $(SDB_SH_OBJ)
	$(GPP) $(SDB_SH_OBJ) $(LIBS)  -o $@ -pg	
	
t_am:  $(AM_OBJ)
	$(GPP) $(AM_OBJ) $(LIBS)  -o $@ -pg	
	
t_am1:  $(AM_OBJ1)
	$(GPP) $(AM_OBJ1) $(LIBS)  -o $@ -pg	


clean:
	rm -f *.o  t_sdb_storage *.dat
