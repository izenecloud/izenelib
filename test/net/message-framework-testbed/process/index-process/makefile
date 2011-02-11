CXX = g++

ifneq ($(MF_FLAG),USE_MF_LIGHT)
TARGET_PATH = ../../bin
TARGET = $(TARGET_PATH)/IndexProcess
else
TARGET_PATH = ../../lib
TARGET = $(TARGET_PATH)/libmain_processes.a
endif


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


LIB =  		 	-lsf1v5_common \
				-lmessage_framework \
				-lfebird \
				-lindex_manager \
				-ldoc_manager \
				-lboost_thread-gcc41-mt \
				-lboost_system-gcc41-mt \
				-lboost_serialization-gcc41-mt \
				-lboost_program_options-gcc41-mt \
				-lglog \
				-lprocmeminfo \


OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))



%.o : %.cpp 
	$(CXX) $(INCLUDE_PATH) $(CXXFLAGS) -c -o $@ $?


all : $(OBJECTS)
ifneq ($(MF_FLAG),USE_MF_LIGHT)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $? $(LIB_PATH) $(LIB) 
else
	ar rsuv $(TARGET) $?
endif

clean:
	rm -rf *.o $(TARGET) 
