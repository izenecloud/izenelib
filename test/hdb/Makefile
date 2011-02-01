CXX=g++-4.1

CXXFLAGS = -O3 -Wall -c -g -DVERBOSE_HDB


INC_PATH=	-I /usr/local/include/ \
		-I /home/wei/dev/izenelib/include/ \
		-I /home/wei/dev/wiselib/include \
		-I /home/wei/dev/izenelib/include/3rdparty/ \
		-I /usr/local/include/boost-1_39 \


LIB_PATH=	-L /usr/local/lib \
		-L /home/wei/dev/izenelib/lib/ \
		-L /home/wei/dev/wiselib/lib/ \

LIBS =		-l boost_system-gcc41-mt-1_39 \
		-l boost_date_time-gcc41-mt-1_39 \
		-l boost_serialization-gcc41-mt-1_39 \
                -l boost_program_options-gcc41-mt-1_39 \
                -l boost_thread-gcc41-mt-1_39 \
                -l boost_unit_test_framework-gcc41-mt-1_39 \
                -l tokyocabinet \
                -l procmeminfo  \

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp)) 

%.o : %.cpp
	$(CXX) $(INC_PATH) $(CXXFLAGS) -c -o $@ $? 

TARGET = test

all: $(TARGET)

$(TARGET) : $(OBJECTS) 
	$(CXX) -o $@ $^ $(LIB_PATH) $(LIBS) 

clean:
	-rm -f *.o  t_hdb_* bench* $(TARGET) \

cleanup:
	-rm -f t_hdb_* bench*
