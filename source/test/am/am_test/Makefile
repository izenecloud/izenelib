GPP=/usr/bin/g++-4.1
GCC=/usr/bin/gcc-4.1

CC_OPTIONS= -O3 -Wall -c -g -DDEBUG

WISELIB = /home/ian/CodeBase/wiselib


INC_PATH=    -I /usr/local/include/  -I  ../../../../../izenelib/include/ -I  ../../../../../ylib/include/\
             -I ../../../../../izenelib/include/3rdparty/ -I /usr/local/include/boost-1_36 \
             -I $(WISELIB)/include


LIBS =      /lib/libboost_thread-gcc41-mt.so \
           /opt/gcc41/lib/libtokyocabinet.so   /lib/libboost_serialization-gcc41-mt.so   \
           ../../../../../izenelib/lib/libprocmeminfo.a  ../../../../../izenelib/lib/libfebird.a   \
	$(WISELIB)/lib/libwiselib.so /opt/gcc41/lib/libglog.so #    /usr/local/lib64/libstdc++.so 


.cc.o:

	$(GPP) $(INC_PATH) $(CC_OPTIONS) $< 

am_obj =    t_am.o 
am_ylib_obj = t_am_ylib.o 
test_obj = test_ustring.o 
t_cccr_hash_obj = t_cccr_hash.o 
t_am_sf_obj = t_am_sf.o

all:  t_cccr_hash #test t_am t_am_ylib

test: $(test_obj)
	$(GCC) $(test_obj) $(LIBS)  -o $@ -pg

t_cccr_hash: $(t_cccr_hash_obj) 
	$(GCC) $(t_cccr_hash_obj) $(LIBS)  -o $@ -pg

t_am: $(am_obj)
	$(GPP) $(am_obj) $(LIBS)  -o $@ -pg

t_am_ylib: $(am_ylib_obj)
	$(GPP) $(am_ylib_obj) $(LIBS)  -o $@ -pg

t_am_sf: $(t_am_sf_obj)
	$(GPP) $(t_am_sf_obj) $(LIBS)  -o $@ -pg

clean:
	-rm -f *.o  *.dat t_am t_am_ylib

cleanup:
	-rm -f *.dat
