GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O2 -g -Wall -c  -fmessage-length=0 -DDEBUG -pg

LIBS =     /usr/local/lib/libboost_thread-gcc41-mt.so  /usr/local/lib/libboost_unit_test_framework-gcc41-mt.so \
	   /usr/local/lib/libboost_serialization-gcc41-mt.so  -L ../../../../lib -lprocmeminfo -lglog -lfebird ../../../../lib/libizene_log.a
INC_PATH := -I /home/wps/workspace/wiselib/include -I  /usr/local/include/boost-1_36/ -I ../../../../include/ -I ../../../../include/3rdparty/


SDB_BTREE_OBJ  := t_sdb_btree.o 

SDB_BTREE1_OBJ := t_sdb_btree1.o

SDB_BTREE2_OBJ := t_sdb_btree2.o

TEST_OBJ := t_sdb_unit.o

BUG_OBJ := t_OrderedSDB.o #t_master_suite.o

OBJS := $(patsubst %.cc, %.o, $(wildcard *.cc))


all: t_sdb_btree2  t_sdb_btree #t_bug  t_sdb_unit  t_sdb_btree  t_sdb_btree1 t_sdb_btree2


.cc.o:

	$(GPP) $(INC_PATH) $(CC_OPTIONS) $< 
.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  $< 
	
test:  $(OBJS)
	$(GPP) $(OBJS) $(LIBS)  -o $@ -pg  

t_sdb_unit:  $(TEST_OBJ)
	$(GPP) $(TEST_OBJ) $(LIBS)  -o $@ -pg  

t_bug:  $(BUG_OBJ)
	$(GPP) $(BUG_OBJ) $(LIBS)  -o $@ -pg  

t_sdb_btree:  $(SDB_BTREE_OBJ)
	$(GPP) $(SDB_BTREE_OBJ) $(LIBS)  -o $@ -pg		
	
t_sdb_btree1:  $(SDB_BTREE1_OBJ)

	$(GPP) $(SDB_BTREE1_OBJ) $(LIBS)  -o $@ -pg	
	
t_sdb_btree2:  $(SDB_BTREE2_OBJ)

	$(GPP) $(SDB_BTREE2_OBJ) $(LIBS)  -o $@ -pg		


clean:
	rm -f *.o  t_sdb_btree t_sdb_btree1 *.dat *.sdb
