IZENELIB=/home/newpeak/CodeBase/izenelib

WISELIB=/home/newpeak/CodeBase/wiselib

INDEXER_ROOT=/home/newpeak/Desktop/sf1-alone

SF1V5_ROOT=/home/newpeak/CodeBase/sf1-revolution

LIBRARY_HOME=/home/newpeak/CodeBase/shared

LA=/home/newpeak/CodeBase/la-manager

WISEKMA=/home/newpeak/CodeBase/kma

CXX = g++

LINK_FLAGS = -Xlinker -zmuldefs

CXXFLAGS = -Wall -O3 -g #-DUDT3 -DSTANDALONE

TARGET = t_Indexer

INCLUDE_PATH = \
			   -I/usr/local/include/boost-1_38 \
			   -I$(SF1V5_ROOT)/source/core/include/\
			   -I$(WISELIB)/include \
			   -I$(IZENELIB)/include \
			   -I$(LA)/include \
			   -I$(WISEKMA)/interface

BOOST_LIB = -lboost_serialization-gcc41-mt -lboost_system-gcc41-mt -lboost_thread-gcc41-mt -lboost_filesystem-gcc41-mt
IZENELIB_LIB = -L$(IZENELIB)/lib  -lindex_manager -lfebird -ludt3 #-lstreamflow#-ludt3
WISELIB_LIB =  -L$(WISELIB)/lib -lticpp
LA_LIB = -L$(SF1V5_ROOT)/lib -lsf1v5_la_manager -lsf1v5_configuration_manager -L$(LA)/lib -lla -L$(WISEKMA) -lwko

LIB_PATH = $(BOOST_LIB) $(LA_LIB) $(IZENELIB_LIB) $(WISELIB_LIB)

OBJECTS = t_Indexer.o XmlConfigParser.o ScdParser.o

%.o : %.cpp
	$(CXX) $(INCLUDE_PATH) $(CXXFLAGS) -c -o $@ $?

$(TARGET) : $(OBJECTS) 
	$(CC) $(CXXFLAGS) -o $@  $(OBJECTS)  $(LIB_PATH) $(LINK_FLAGS)

all: $(TARGET)

	
clean:
	rm -rf *.o $(TARGET) 
	rm -rf *.sdb
	rm -rf idm*


