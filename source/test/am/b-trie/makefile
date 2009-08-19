GPP :=g++-4.1

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -g -rdynamic -Wall -c -fmessage-length=0 


LIBS :=   /usr/local/lib/libboost_system-gcc41-mt-1_39.so /usr/local/lib/libboost_serialization-gcc41-mt.so /usr/local/lib/libglog.a ../../../../lib/libprocmeminfo.a ../../../../lib/libbtrie.a
INC_PATH := -I  /usr/local/include/boost-1_39/ -I ../../../../include/ -I ../../../../include/3rdparty/ -I /home/wei/dev/wiselib/include/

OBJS := $(patsubst %.cpp, %.o, $(wildcard *.cpp))

all: amlib


.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<"

.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<"

.cc.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<"


amlib: $(OBJS)
	$(GPP)  -g -o "b-trie"  $^ $(LIBS) /usr/local/lib/libboost_unit_test_framework-gcc41-mt-1_39.so

gen_input: gen_input.o 
	$(GPP)  -g -o "gen_input"  $^ $(LIBS)

gen_alphabet: gen_alphabet.o 
	$(GPP)  -g -o "gen_alphabet"  $^ $(LIBS)

example: example_searcher.o example_builder.o
	$(GPP)  -g -o "searcher"  example_searcher.o $(LIBS)
	$(GPP)  -g -o "builder"  example_builder.o $(LIBS)


clean:
	-rm -f $(OBJS)
	-rm -f *.o *.buk *.nod *.has
	-rm -f builder searcher gen_input gen_alphabet b-trie
