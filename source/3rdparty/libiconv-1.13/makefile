GPP :=g++

GCC :=/usr/bin/gcc

CC_OPTIONS :=  -O3 -g -rdynamic -fPIC -Wall -c -fmessage-length=0 


INC_PATH := -I ../../../include/3rdparty/libiconv-1.13/include/


OBJS := $(patsubst %.c, %.o, $(wildcard *.c))
#./t_main.o


all: iconv.so



.c.o:
	$(GPP) $(CC_OPTIONS) $(INC_PATH)  -o"./$@" "$<"



iconv.so: $(OBJS)
	$(GCC)  -shared -o "libiconv.so" iconv.o relocatable.o localcharset.o
	-rm -f $(OBJS)


clean:
	-rm -f $(OBJS)
