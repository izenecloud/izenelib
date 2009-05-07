GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O3 -g -Wall -c  -fmessage-length=0 -DDEBUG -pg

LIBS =     /usr/local/lib/libboost_thread-gcc41-mt.so  /usr/local/lib/libboost_unit_test_framework-gcc41-mt.so \
	   /usr/local/lib/libboost_serialization-gcc41-mt.so  ../../../../lib/libsdb_btree.a

INC_PATH := -I  /usr/local/include/boost-1_35/ -I ../../../../include/ -I ../../../../include/3rdparty/


SDB_BTREE_OBJ  := t_sdb_btree.o 

SDB_BTREE1_OBJ := t_sdb_btree1.o

SDB_BTREE2_OBJ := t_sdb_btree2.o


all:  t_sdb_btree  t_sdb_btree1 #t_sdb_btree2

.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  $< 

t_sdb_btree:  $(SDB_BTREE_OBJ)
	$(GPP) $(SDB_BTREE_OBJ) $(LIBS)  -o $@ -pg		
	
t_sdb_btree1:  $(SDB_BTREE1_OBJ)

	$(GPP) $(SDB_BTREE1_OBJ) $(LIBS)  -o $@ -pg	
	
t_sdb_btree2:  $(SDB_BTREE2_OBJ)

	$(GPP) $(SDB_BTREE2_OBJ) $(LIBS)  -o $@ -pg		


clean:
	rm -f *.o  t_sdb_btree t_sdb_btree1 sdb.dat
