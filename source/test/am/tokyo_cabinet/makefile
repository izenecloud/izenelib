GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O3 -g -Wall -c  -DDEBUG -pg

LIBS =    /usr/local/lib/libtokyocabinet.so   /usr/local/lib/libboost_thread-gcc41-mt.so  /usr/local/lib/libboost_unit_test_framework-gcc41-mt.so  /usr/local/lib/libboost_serialization-gcc41-mt.so 

INC_PATH := -I  /usr/local/include/boost-1_35/  -I /usr/local/include/  -I ../../../../include/ -I ../../../../include/3rdparty/


TC_OBJ := t_tc.o
 

all:  t_tc 

.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  $< 
	

t_tc:  $(TC_OBJ)

	$(GPP) $(TC_OBJ) $(LIBS)  -o $@ -pg		
	

clean:
	rm -f *.o  t_tc *.dat
