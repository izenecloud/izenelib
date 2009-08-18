IZENELIB=/home/newpeak/CodeBase/izenelib

WISELIB=/home/newpeak/CodeBase/wiselib

INDEXER_ROOT=/home/newpeak/Desktop/sf1-alone

SF1V5_ROOT=/home/newpeak/CodeBase/sf1-revolution

LIBRARY_HOME=/home/newpeak/CodeBase/shared

CXX = g++

LINK_FLAGS = -Xlinker -zmuldefs


DOCUMENT = ./document-manager/
TEST = ./index-manager/test

CXXFLAGS = -Wall -O3 -g #-DUDT3 -DSTANDALONE


TARGET_DIR = ../bin

MAIN = $(TARGET_DIR)/indexer
AGENT = $(TARGET_DIR)/agent
TARGET = $(MAIN) $(AGENT)

DOC = $(TARGET_DIR)/doc

INCLUDE_PATH = \
			   -I/usr/local/include/boost-1_38 \
			   -I$(SF1V5_ROOT)/source/core/include/\
			   -I$(SF1V5_ROOT)/source/external/stemmer \
			   -I$(SF1V5_ROOT)/source/external/kmaorange/interface \
			   -I$(WISELIB)/include \
			   -I$(IZENELIB)/include \


BOOST_LIB = -lboost_serialization-gcc41-mt -lboost_system-gcc41-mt -lboost_thread-gcc41-mt -lboost_filesystem-gcc41-mt
IZENELIB_LIB = -L$(LIBRARY_HOME) -lsdb_btree -lindex_manager -lfebird -ludt3 -lstreamflow#-ludt3
WISELIB_LIB =  -L$(WISELIB)/lib -lticpp
LA_LIB = -L$(SF1V5_ROOT)/lib -lsf1v5_la_manager -lsf1v5_configuration_manager -lstemmer -lwko
		   
LIB_PATH = $(BOOST_LIB) $(IZENELIB_LIB) $(WISELIB_LIB) $(LA_LIB)


DOCUMENT_OBJS = $(patsubst %.cpp, %.o, $(wildcard $(DOCUMENT)/*.cpp))

MAIN_OBJ = $(TEST)/t_Indexer.o $(TEST)/XmlConfigParser.o $(TEST)/ScdParser.o
AGENT_OBJ = $(TEST)/t_agent.o $(TEST)/XmlConfigParser.o $(TEST)/ScdParser.o

OBJECTS = $(LA_OBJS) #$(DOCUMENT_OBJS) 


%.o : %.cpp
	$(CXX) $(INCLUDE_PATH) $(CXXFLAGS) -c -o $@ $?


all: $(TARGET)
	@echo $(DOCUMENT)
	@echo $(DOCUMENT_OBJS)
	@echo $(OBJECTS)

doc:$(DOC)

$(MAIN) : $(OBJECTS) $(MAIN_OBJ)
	$(CC) $(CXXFLAGS) -o $@  $(OBJECTS) $(MAIN_OBJ) $(LIB_PATH) $(LINK_FLAGS)

$(AGENT) : $(OBJECTS) $(AGENT_OBJ)
	$(CC) $(CXXFLAGS) -o $@  $(OBJECTS) $(AGENT_OBJ) $(LIB_PATH) $(LINK_FLAGS)


$(DOC) : $(OBJECTS) $(DOC_OBJ)
	$(CC) $(CXXFLAGS) -o $@  $(OBJECTS) $(DOC_OBJ) $(LIB_PATH) $(LINK_FLAGS)
	
clean:
	rm -rf *.o $(TARGET) 
	rm -rf $(DOCUMENT)/*.o
	rm -rf $(MAIN_OBJ) $(AGENT_OBJ)
	rm -rf test/*.o


