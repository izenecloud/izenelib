GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O3 -g -Wall -c  -fmessage-length=0 -DDEBUG -pg

LIBS =     /usr/local/lib/libboost_thread-gcc41-mt.so  /usr/local/lib/libboost_unit_test_framework-gcc41-mt.so  /usr/local/lib/libboost_serialization-gcc41-mt.so


INC_PATH := -I  /usr/local/include/boost-1_35/ -I ../../../../include/ -I ../../../../include/3rdparty/


SDB_HASH_OBJ  := t_sdb_hash.o 

SDB_SH_OBJ := t_sh.o

 

all:  t_sh  t_sdb_hash

.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  $< 

t_sdb_hash:  $(SDB_HASH_OBJ)
	$(GPP) $(SDB_HASH_OBJ) $(LIBS)  -o $@ -pg		
	
t_sh:  $(SDB_SH_OBJ)
	$(GPP) $(SDB_SH_OBJ) $(LIBS)  -o $@ -pg	


clean:
	rm -f *.o  t_sh t_sdb_hash sdb.dat
