GPP :=g++

CC_OPTIONS :=  -O3 -rdynamic -g -Wall -c -fmessage-length=0 

IZENELIB = ../../../../include

LIB_PATH  :=  -lboost_serialization-gcc41-mt-1_38 -lboost_system-gcc41-mt-1_38 -lboost_iostreams-gcc41-mt-1_38 -lboost_date_time-gcc41-mt-1_38 -lboost_thread-gcc41-mt-1_38 -luuid ../../../../lib/libfebird.a
INC_PATH = -I $(IZENELIB)/3rdparty -I /usr/local/include/boost-1_38

.cc.o:
	$(GPP) $(CC_OPTIONS) ${INC_PATH} $<

TARGET = t_bench t_febird

T_BENCH_OBJ = t_bench.o
T_FEBIRD_OBJ = t_febird.o

all:$(TARGET)

t_bench:$(T_BENCH_OBJ)
	$(GPP) $(T_BENCH_OBJ) ${LIB_PATH} -o $@ 

t_febird:$(T_FEBIRD_OBJ)
	$(GPP) $(T_FEBIRD_OBJ) ${LIB_PATH} -o $@ 

clean:
	-rm -f $(TARGET) ${T_BENCH_OBJ} ${T_FEBIRD_OBJ} *.bin
