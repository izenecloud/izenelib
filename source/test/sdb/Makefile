GPP=/usr/bin/g++
GCC=/usr/bin/gcc

CC_OPTIONS= -O3 -Wall -c -g -pg  -DDEBUG


INC_PATH=    -I /usr/local/include/  -I  ../../../../izenelib/include/ -I  ../../../../ylib/include/\
             -I ../../../../izenelib/include/3rdparty/ -I /usr/local/include/boost-1_36  


LIBS =    ../../../../izenelib/lib/libsdb_btree.so   /usr/local/lib/libboost_thread-gcc41-mt.so \
           /usr/local/lib/libtokyocabinet.so   /usr/local/lib/libboost_serialization-gcc41-mt.so \
           ../../../../ylib/lib/libylib.a
.cc.o:

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


all:  t_overflow_btree  t_sdb  t_btree1 #t_btree2 t_nsdb t_IndexSDB t_IndexSDB1 t_mul_sdb  t_btree_str \
      t_btree1 t_mul_sdb  t_nsdb t_overflow_btree 

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


clean:
	-rm -f *.o  *.dat .t_sdb t_btree1 t_btree2 t_nsdb \
        t_sdb t_IndexSDB1 t_btree_str t_IndexSDB  t_mul_sdb  t_overflow_btree  gmon.out

   
cleanup:
	-rm -f *.dat
