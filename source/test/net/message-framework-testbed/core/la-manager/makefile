CXX = g++

TARGET_PATH = ../../lib
TARGET = $(TARGET_PATH)/libla_manager.a


######################################################
# if $(COMPILE_FLAGS) is given by upper makefile 
# else if no $(COMPILE_FLAGS) is given 

ifneq ($(COMPILE_FLAGS),)
CXXFLAGS += $(COMPILE_FLAGS) 
else
CXXFLAGS = -Wall -O3 -DSF1_DEBUG
endif



INCLUDE_PATH = -I../include \
               -I$(IZENELIB)/include 

#LIB = 			
#LIB_PATH =  	

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))


%.o : %.cpp 
	$(CXX) $(INCLUDE_PATH) $(CXXFLAGS) -c -o $@ $?


all: $(TARGET)

$(TARGET) : $(OBJECTS)
	ar rsuv $@ $?

clean:
	rm -rf *.o $(TARGET) 
