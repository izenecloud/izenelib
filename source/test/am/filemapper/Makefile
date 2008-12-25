IZENELIB = ../../../../include

TEST_OBJS = tests.o
BENCH_OBJS = bench.o

CPPFLAGS = -I $(IZENELIB)  -Wall -O2 -g

all : tests bench

tests : $(TEST_OBJS) 
	g++ $(TEST_OBJS)  -lpthread -o tests

bench: $(BENCH_OBJS)
	g++ $(BENCH_OBJS)  -lpthread -o bench 
clean:
	rm -rf posix/*.o *.map *.a *.o bench tests
