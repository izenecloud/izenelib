GPP=g++

IZENELIB = ../../../../include

INC_PATH = -I$(IZENELIB)  -I /usr/local/include/boost-1_35 

CC_OPTIONS= -O3 -Wall -c -g

LIB_PATH=-lboost_thread-gcc41-mt-1_35 -lboost_system-gcc41-mt-1_35 -lboost_iostreams-gcc41-mt-1_35 -lboost_filesystem-gcc41-mt-1_35 -lboost_date_time-gcc41-mt-1_35

TARGET=main

.cpp.o:
	$(GPP) $(CC_OPTIONS) ${INC_PATH} $<

main_OBJ = t_BlockManager.o


main: $(main_OBJ)
	$(GPP) $(main_OBJ) ${LIB_PATH} -o $@

clean:
	rm -rf *.o *~ $(TARGET) stxxl
