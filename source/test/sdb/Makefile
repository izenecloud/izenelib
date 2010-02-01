GPP=/usr/bin/g++
GCC=/usr/bin/gcc

CC_OPTIONS= -O3 -Wall -c -g -pg  -DDEBUG


INC_PATH=    -I /usr/local/include/  -I  ../../../../izenelib/include/ -I  ../../../../ylib/include/ \
   -I ../../../../wiselib/include -I ../../../../izenelib/include/3rdparty/ -I /usr/local/include/boost-1_36  


LIBS =      /usr/local/lib/libboost_thread-gcc41-mt.so \
           /usr/local/lib/libtokyocabinet.so   /usr/local/lib/libboost_serialization-gcc41-mt.so \
           ../../../../ylib/lib/libylib.a   ../../../../izenelib/lib/libprocmeminfo.so -L ../../../ \
           -L ../../../../izenelib/lib -lfebird -lizene_log -lglog
            
.cc.o:

	$(GPP) $(INC_PATH) $(CC_OPTIONS) $< 
	
.cpp.o:

	$(GPP) $(INC_PATH) $(CC_OPTIONS) $< 


obj =    t_sdb.o 

obj1 =   t_btree1.o   

obj3 =   t_mul_sdb.o

obj4 =   t_overflow_btree.o

obj5 =   t_IndexSDB.o

obj6 = t_nsdb.o

obj7 = t_tc.o

obj8 = t_btree_str.o

obj9 = t_IndexSDB1.o

obj10 = t_btree2.o

obj11 = t_TrieIndexSDB.o

obj12 = t_TrieIndexSDB1.o

obj13 = t_sdb_query.o

obj14 = t_dm.o

all: t_dm t_mul_sdb #t_sdb_query #t_IndexSDB1 t_IndexSDB t_TrieIndexSDB1  t_TrieIndexSDB  t_btree2  t_btree1  t_sdb t_btree2 t_nsdb t_IndexSDB t_IndexSDB1 t_mul_sdb  t_btree_str \
     # t_btree1 t_mul_sdb  t_nsdb t_overflow_btree 
     
t_dm: $(obj14)

	$(GPP) $(obj14) $(LIBS)  -o $@ -pg     
     
      
t_sdb_query: $(obj13)

	$(GPP) $(obj13) $(LIBS)  -o $@ -pg

t_sdb: $(obj)

	$(GPP) $(obj) $(LIBS)  -o $@ -pg
	
t_btree_str: $(obj8)

	$(GPP) $(obj8) $(LIBS)  -o $@ -pg	
		
	
t_tc: $(obj7)

	$(GPP) $(obj7) $(LIBS)  -o $@ -pg	
	
t_nsdb: $(obj6)

	$(GPP) $(obj6) $(LIBS)  -o $@ -pg	
	
t_btree1: $(obj1)

	$(GPP) $(obj1) $(LIBS) -o $@ -pg	
	
		
t_btree2: $(obj10)

	$(GPP) $(obj10) $(LIBS) -o $@ -pg	
	
	

t_mul_sdb: $(obj3)

	$(GPP) $(obj3) $(LIBS) -o $@ -pg	
	
t_overflow_btree: $(obj4)

	$(GPP) $(obj4) $(LIBS) -o $@ -pg	
	
t_IndexSDB: $(obj5)

	$(GPP) $(obj5) $(LIBS) -o $@ -pg
	
t_IndexSDB1: $(obj9)

	$(GPP) $(obj9) $(LIBS) -o $@ -pg
	
		
t_TrieIndexSDB: $(obj11)

	$(GPP) $(obj11) $(LIBS) -o $@ -pg
	
t_TrieIndexSDB1: $(obj12)

	$(GPP) $(obj12) $(LIBS) -o $@ -pg
	


clean:
	-rm -f *.o  *.dat t_sdb t_btree1 t_btree2 t_nsdb t_TrieIndexSDB\
        t_sdb t_IndexSDB1 t_btree_str t_IndexSDB  t_mul_sdb  t_overflow_btree  gmon.out

   
cleanup:
	-rm -f *.dat
