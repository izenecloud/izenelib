GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O3 -g -Wall -c -fmessage-length=0 -DDEBUG -pg

LIBS =     /usr/local/lib/libboost_thread-gcc41-mt.so  /usr/local/lib/libboost_unit_test_framework-gcc41-mt.so  /usr/local/lib/libboost_serialization-gcc41-mt.so


INC_PATH := -I  /usr/local/include/boost-1_35/ -I ../../../../include/ -I ../../../../include/3rdparty/


OBJ1 := t_master_suite.o t_btree.o
OBJ2 := t_master_suite.o t_btree_v2.o


all: t_btree #t_btree_v2


.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<"

t_btree:  $(OBJ1)
	$(GPP) $(OBJ1) $(LIBS) -o $@ -pg
	#-rm -f $(OBJS)
	
t_btree_v2:  $(OBJ2)
	$(GPP) $(OBJ2) $(LIBS) -o $@ -pg
	#-rm -f $(OBJS)

clean:
	rm -f *.o t_btree t_btree_v2 sdb.dat gmon.out
