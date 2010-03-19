IZENELIB = ../../../..

TEST_OBJS = tests.o
BENCH_OBJS = bench.o

LIBS := $(IZENELIB)/lib/libam.so
CPPFLAGS = -I $(IZENELIB)/include  -Wall -O2 -g

all : tests bench

tests : $(TEST_OBJS) 
	g++ $(TEST_OBJS)  -lpthread  $(LIBS) -o tests

bench: $(BENCH_OBJS)
	g++ $(BENCH_OBJS)  -lpthread  $(LIBS)  -o bench 
clean:
	rm -rf posix/*.o *.map *.a *.o bench tests
