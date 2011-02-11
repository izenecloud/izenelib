GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS := -O3 -g -Wall -c -fmessage-length=0 


LIBS :=  -lpthread /usr/local/lib/libboost_thread-gcc41-mt-1_36.so /home/Kevin/izenelib/lib/libizene_log.so /home/Kevin/izenelib/lib/libprocmeminfo.so /usr/local/lib/libboost_unit_test_framework-gcc41-mt-1_36.so /usr/local/lib/libboost_thread-gcc41-mt-1_36.so /usr/local/lib/libboost_filesystem-gcc41-mt-1_36.so 

INC_PATH := -I /home/Kevin/wiselib/include/ -I /home/Kevin/izenelib/include/ -I /usr/local/include/boost-1_36/ -I ../../../../include/ -I ../../../../include/3rdparty/


OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))
#./t_main.o


all: izenelib gen-integers fp-cluster


.cc.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<"

.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<"


gen-integers: t_gen_integers.o
	$(GPP)  -g -o "gen-integers"  $^ $(LIBS) 
	-rm -f t_gen_integers.o

fp-cluster: t_cluster.o
	$(GPP)  -g -o "fp-cluster"  $^ $(LIBS) 
	-rm -f t_cluster.o

izenelib: t_graph_index.o
	$(GPP)  -g -o "graph-index"  $^ $(LIBS) 
	-rm -f $(OBJS)

clean:
	-rm -f $(OBJS)
