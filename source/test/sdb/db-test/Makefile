GPP=/usr/bin/g++
GCC=/usr/bin/gcc

CC_OPTIONS= -O3 -Wall -c -g -pg  -DDEBUG


INC_PATH=   -I ../../../../../izenelib/include  -I /usr/local/include/  -I /usr/local/BerkeleyDB.4.6/include    -I ../../../../../izenelib/include/3rdparty/ -I /usr/local/include/boost-1_35   


LIBS =     ../../../../../izenelib/lib/libsdb_btree.so  /usr/local/lib/libboost_thread-gcc41-mt.so  /usr/local/lib/libtokyocabinet.so  /usr/local/BerkeleyDB.4.6/lib/libdb-4.6.so  /usr/local/lib/libboost_serialization-gcc41-mt.so 

.cc.o:

	$(GPP) $(INC_PATH) $(CC_OPTIONS) $< 


sdb1_obj =    t_sdb1.o 

sdb2_obj = t_sdb2.o

tc_obj = t_tc.o

bdb_obj = t_bdb.o

all: t_bdb t_tc t_sdb1 t_sdb2 
	
t_tc: $(tc_obj)

	$(GPP) $(tc_obj) $(LIBS)  -o $@ -pg	
	
t_sdb1: $(sdb1_obj)

	$(GPP) $(sdb1_obj) $(LIBS)  -o $@ -pg	
	
t_sdb2: $(sdb2_obj)

	$(GPP) $(sdb2_obj) $(LIBS)  -o $@ -pg	
	
t_bdb: $(bdb_obj)

	$(GPP) $(bdb_obj) $(LIBS)  -o  $@ -pg	

clean:
	-rm -f *.o  *.dat t_sdb1 t_sdb2 t_tc t_bdb *.bdbh *.bdbb *.tch* *.tcb* gmon.out
   
cleanup:
	-rm -f *.bdbh *.bdbb *.tch* *.tcb* *.dat
