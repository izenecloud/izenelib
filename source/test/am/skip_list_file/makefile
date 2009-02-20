GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O3 -g -Wall -c  -fmessage-length=0 -DDEBUG -pg

LIBS =     /usr/local/lib/libboost_thread-gcc41-mt.so  /usr/local/lib/libboost_unit_test_framework-gcc41-mt.so  /usr/local/lib/libboost_serialization-gcc41-mt.so


INC_PATH := -I  /usr/local/include/boost-1_35/ -I ../../../../include/ -I ../../../../include/3rdparty/


SLF_OBJ  := t_slf.o 

iDSLF_OBJ := t_dslf.o

SASL_OBJ := t_sasl.o

NSLF_OBJ := t_nslf.o

SEQSLF_OBJ := t_seqslf.o
 

all:  t_slf #t_seqslf t_dslf t_sasl t_nslf t_seqslf

.cpp.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  $< 

t_slf:  $(SLF_OBJ)
	$(GPP) $(SLF_OBJ) $(LIBS)  -o $@ -pg	
	
t_dslf:  $(DSLF_OBJ)
	$(GPP) $(DSLF_OBJ) $(LIBS) -o $@ -pg	
	
t_sasl:  $(SASL_OBJ)
	$(GPP) $(SASL_OBJ) $(LIBS) -o $@ -pg	
	
t_nslf:  $(NSLF_OBJ)
	$(GPP) $(NSLF_OBJ) $(LIBS) -o $@ -pg		
		
t_seqslf:  $(SEQSLF_OBJ)
	$(GPP) $(SEQSLF_OBJ) $(LIBS) -o $@ -pg		
	
	

clean:
	rm -f *.o  t_slf t_dslf t_sasl t_seqslf sdb.dat
