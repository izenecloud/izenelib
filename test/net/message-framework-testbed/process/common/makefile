CXX = g++
CXXFLAGS = -Wall -O3 $(INCLUDE_PATH) -g

#####################################################
ifeq ($(MF_FLAG),USE_MF_LIGHT)
CXXFLAGS += -DUSE_MF_LIGHT
endif


TARGET_PATH = ../../lib
TARGET = $(TARGET_PATH)/libsf1v5_common.a

INCLUDE_PATH = -I/usr/local/include/boost-1_36 \
			   -I../../core/include \
			   -I$(IZENELIB)/include \
			   -I$(WISELIB)/include

LIB_PATH = 
LIB = 

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))

#SRCS = $(wildcard *.cpp)
#OBJECTS = $(SRCS:.c=.o)

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $?


all: $(TARGET)

$(TARGET) : $(OBJECTS)
	ar rsuv $@ $^


clean:
	rm -rf *.o $(TARGET) 
