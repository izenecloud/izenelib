CXX = g++

TARGET_PATH = ../../bin
TARGET = $(TARGET_PATH)/MainProcessLight


######################################################
# if $(COMPILE_FLAGS) is given by upper makefile 
# else if no $(COMPILE_FLAGS) is given 

ifneq ($(COMPILE_FLAGS),)
CXXFLAGS += $(COMPILE_FLAGS) 
else
CXXFLAGS = -Wall -O3 
endif


#####################################################
ifeq ($(MF_FLAG),USE_MF_LIGHT)
CXXFLAGS += -DUSE_MF_LIGHT
endif


INCLUDE_PATH = -I../common \
			   -I../../core/include \
			   -I/usr/local/include/boost-1_36 \
			   -I$(WISELIB)/include \
			   -I$(IZENELIB)/include


LIB_PATH =  	\
				-L../common \
				-L../../lib \
				-L$(WISELIB)/lib \
				-L$(IZENELIB)/lib


LIB =  		 	\
				-lmain_processes \
				-ldoc_manager \
				-lindex_manager \
				-lla_manager \
				-lsf1v5_common \
				-lmessage_framework \
				-lfebird \
				-lboost_thread-gcc41-mt \
				-lboost_system-gcc41-mt \
				-lboost_serialization-gcc41-mt \
				-lboost_program_options-gcc41-mt \
				-lglog \
				-lprocmeminfo \
				-lwiselib
				

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))



%.o : %.cpp 
	$(CXX) $(INCLUDE_PATH) $(CXXFLAGS) -c -o $@ $?


all: $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $? $(LIB_PATH) $(LIB) 

clean:
	rm -rf *.o $(TARGET) ../../../lib/libmain_processes.a
