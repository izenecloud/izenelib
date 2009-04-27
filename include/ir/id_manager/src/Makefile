CXX = g++ 

######################################################
# if $(COMPILE_FLAGS) is given by upper makefile 
# else if no $(COMPILE_FLAGS) is given 
ifneq ($(COMPILE_FLAGS),)
CXXFLAGS += $(COMPILE_FLAGS) 
else
CXXFLAGS = -Wall -O3 -g
endif

TARGET_PATH = .
WISELIB = /home/wps/workspace/wiselib
IZENELIB = /home/wps/workspace/izenelib

IDMANAGER_PATH = ../include/

INCLUDE_PATH = \
			   -I $(IDMANAGER_PATH) \
			   -I $(WISELIB)/include \
			   -I $(IZENELIB)/include \
			   -I $(IZENELIB)/include/3rdparty \
			   -I /usr/local/include/boost-1_36 \

		  
EXT_LIB_PATH = \
			   -L $(IZENELIB)/lib \
			   -L $(WISELIB)/lib \
			   -L /usr/local/lib \

EXT_LIB = \
	      -l wiselib \
		  -l boost_system-gcc41-mt-1_36 \
		  -l boost_date_time-gcc41-mt-1_36 \
		  -l boost_serialization-gcc41-mt-1_36 \
		  -l boost_program_options-gcc41-mt-1_36 \
		  -l boost_thread-gcc41-mt-1_36 \
		  -l boost_unit_test_framework-gcc41-mt-1_36 \
		  -l boost_memory \
		  -l tokyocabinet \
		  -l sdb_btree \
		  


OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))

%.o : %.cpp
	$(CXX) $(INCLUDE_PATH) $(CXXFLAGS) -c -o $@ $? 

TARGET = test

all: $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CXX) -o $@ $^  $(EXT_LIB_PATH) $(EXT_LIB) 

clean:
	rm -rf *.o $(TARGET) *.sdb 
