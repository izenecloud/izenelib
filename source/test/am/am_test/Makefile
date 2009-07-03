GPP=/usr/bin/g++
GCC=/usr/bin/gcc -lstdc++

CC_OPTIONS= -O2 -Wall -c -g -pg  -DDEBUG

WISELIB = ~/workspace/wiselib


INC_PATH=    -I /usr/local/include/  -I  ../../../../../izenelib/include/ -I  ../../../../../ylib/include/\
             -I ../../../../../izenelib/include/3rdparty/ -I /usr/local/include/boost-1_36 \
             -I $(WISELIB)/include \


LIBS =    ../../../../../izenelib/lib/libsdb_btree.so   /usr/local/lib/libboost_thread-gcc41-mt.so \
           /usr/local/lib/libtokyocabinet.so   /usr/local/lib/libboost_serialization-gcc41-mt.so \
           ../../../../../izenelib/lib/libprocmeminfo.a  ../../../../../izenelib/lib/libfebird.a 
.cc.o:

	$(GPP) $(INC_PATH) $(CC_OPTIONS) $< 

am_obj =    t_am.o 
am_ylib_obj = t_am_ylib.o
test_obj = test_ustring.o
t_cccr_hash_obj = t_cccr_hash.o

all: test t_cccr_hash test t_am t_am_ylib

test: $(test_obj)

	$(GCC) $(test_obj) $(LIBS)  -o $@ -pg
		
t_cccr_hash: $(t_cccr_hash_obj)

	$(GCC) $(t_cccr_hash_obj) $(LIBS)  -o $@ -pg
	

t_am: $(am_obj)

	$(GPP) $(am_obj) $(LIBS)  -o $@ -pg
	
t_am_ylib: $(am_ylib_obj)

	$(GPP) $(am_ylib_obj) $(LIBS)  -o $@ -pg

clean:
	-rm -f *.o  *.dat t_am t_am_ylib
   
cleanup:
	-rm -f *.dat
