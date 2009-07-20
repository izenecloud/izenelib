GPP=/usr/bin/g++
GCC=/usr/bin/gcc

CC_OPTIONS= -O3 -Wall -c -g -pg  -DDEBUG


INC_PATH=  -I  ../../../../izenelib/include/  -I /usr/local/include/boost-1_36 \
             -I  ../../../../ylib/include/   -I ../../../../izenelib/include/3rdparty/

LIBS =  ../../../../izenelib/lib/libsdb_btree.a   ../../../../izenelib/lib/libprocmeminfo.a  /usr/local/lib/libboost_thread-gcc41-mt.so  ../../../../ylib/lib/libylib.a   /usr/local/lib/libboost_serialization-gcc41-mt.so  

.cc.o:

	$(GPP) $(INC_PATH) $(CC_OPTIONS) $< 


TMF_SR_obj = t_mfserialize.o	

TM_SR_obj = t_mserialize.o	
	
TMF_obj = t_MFCache.o 

TM_obj = t_MCache.o

TCDB_obj = t_CacheDB.o

TPM_obj = t_MLRUCache.o

TMC_obj = t_mul_cache.o

t_mul_cc_obj = t_mul_cc.o


all: t_MLRUCache t_mfserialize  t_MFCache  t_CacheDB \
	t_MCache t_MLRUCache t_mfserialize t_mserialize # t_mul_cache t_mul_cc


t_mfserialize: $(TMF_SR_obj)

	$(GPP) $(TMF_SR_obj) $(LIBS) -o $@ 
	
t_mserialize: $(TM_SR_obj)

	$(GPP) $(TM_SR_obj) $(LIBS) -o $@ 


t_MFCache: $(TMF_obj)

	$(GPP) $(TMF_obj) $(LIBS) -o  $@ -pg

t_MCache: $(TM_obj)

	$(GPP) $(TM_obj) $(LIBS) -o $@ -pg
	
t_MCache1: $(TM_obj1)

	$(GPP) $(TM_obj1) $(LIBS) -o  $@ -pg
	
		
t_MLRUCache: $(TPM_obj)

	$(GPP) $(TPM_obj) $(LIBS) -o $@ -pg
	
t_mul_cache: $(TMC_obj)

	$(GPP) $(TMC_obj) $(LIBS) -o $@ -pg		
	
t_mul_sdb: $(TMS_obj)

	$(GPP) $(TMS_obj) $(LIBS) -o  $@ -pg		
	
t_overflow_sdb: $(TMO_obj)

	$(GPP) $(TMO_obj) $(LIBS) -o $@ -pg		
	

t_CacheDB: $(TCDB_obj)

	$(GPP) $(TCDB_obj) $(LIBS) -o $@

t_mul_cc:$(t_mul_cc_obj)	

	$(GPP) $(t_mul_cc_obj) $(LIBS) -o $@	


t_sdb: $(obj)

	$(GPP) $(obj) $(LIBS) -o  ../build/$@ -pg

	
test: $(obj1)

	$(GPP) $(obj1) $(LIBS) -o ../build/$@  -pg
	
	
t_IndexSDB: $(obj2)

	$(GPP) $(obj2) $(LIBS) -o ../build/$@ 


clean:
	rm -f *.o *.dat gmon.out t_mul_cc t_MFCache t_MCache  t_CacheDB t_mfserialize t_mserialize t_mul_cache  t_MLRUCache


   
cleanup:
	-rm -f *.o *.dat
