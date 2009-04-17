CXX = g++ -DDEBUG

######################################################
# if $(COMPILE_FLAGS) is given by upper makefile 
# else if no $(COMPILE_FLAGS) is given 
ifneq ($(COMPILE_FLAGS),)
CXXFLAGS += $(COMPILE_FLAGS) 
else
CXXFLAGS = -Wall -O3
endif

WISELIB = /home/wps/workspace/wiselib
IZENELIB = /home/wps/workspace/izenelib

TARGET_PATH = $(IZENELIB)/lib
TARGET = $(TARGET_PATH)/libidmanager.a

INCLUDE_PATH = -I../include \
			   -I$(WISELIB)/include \
			   -I$(IZENELIB)/include \
			   -I$(IZENELIB)/include/3rdparty \
			   -I/usr/local/include/boost-1_36 

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))

%.o : %.cpp
	$(CXX) $(INCLUDE_PATH) $(CXXFLAGS) -c -o $@ $?


all: $(TARGET)

$(TARGET) : $(OBJECTS)
	ar rsuv $@ $^

clean:
	rm -rf *.o $(TARGET) 

