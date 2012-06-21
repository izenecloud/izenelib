.PHONY: all check dist clean install rpm

SRCS = NUMA_cpu.c NUMA_memnode.c
OBJS = $(SRCS:.c=.o)
MANS = $(wildcard *.3)

ABI = $(shell sed -n 's/%define ABI \(.*\)/\1/p' libNUMA.spec)

ALLSRCS = Makefile $(SRCS) test.c $(HEADERS) $(MANS) libNUMA.map libNUMA.spec

HEADERS = libNUMA.h

VERSION := $(shell sed -n 's/Version: \(.*\)/\1/p' libNUMA.spec)

CC = gcc
CFLAGS = -O0 -g -Wall -Werror -D_GNU_SOURCE -fpic -I. -std=gnu99

INSTALL_DATA = install -cD -m 644
INSTALL_PROGRAM = install -cD

includedir = /usr/include
libdir = /usr/lib
mandir = /usr/man

all: libNUMA.so

libNUMA.so: $(OBJS)
	$(CC) $(LDFLAGS) -shared -o $@ $(OBJS) -Wl,--soname,libNUMA.so.$(ABI),--version-script,libNUMA.map
	-ln -fs $@ libNUMA.so.$(ABI)

test: test.o
	$(CC) -o $@ $^ -L. -lNUMA -Wl,-rpath,\$$ORIGIN

check: test
	./test > /dev/null

install:
	$(INSTALL_DATA) libNUMA.h $(includedir)/libNUMA.h
	$(INSTALL_PROGRAM) libNUMA.so $(libdir)/libNUMA-$(VERSION).so.$(ABI)
	ln -fs libNUMA-$(VERSION).so.$(ABI) $(libdir)/libNUMA.so.$(ABI)
	for n in $(MANS); do \
	  $(INSTALL_DATA) $$n $(mandir)/man3/$$n; \
	done
	for n in MEMNODE_ZERO_S MEMNODE_CLR_S MEMNODE_ISSET_S MEMNODE_COUNT_S MEMNODE_AND_S MEMNODE_OR_S MEMNODE_XOR_S MEMNODE_EQUAL_S MEMNODE_ALLOC MEMNODE_FREE MEMNODE_ALLOC_SIZE; do \
	  echo ".so man3/MEMNODE_SET_S.3" > $(mandir)/man3/$$n.3; \
	done

dist:
	-ln -fs . libNUMA-$(VERSION)
	tar jcvfh libNUMA-$(VERSION).tar.bz2 $(ALLSRCS:%=libNUMA-$(VERSION)/%)
	rm libNUMA-$(VERSION)

rpm: dist
	rpmbuild -ts libNUMA-$(VERSION).tar.bz2

clean:
	-rm -f $(OBJS) libNUMA.so libNUMA.so.$(ABI) test test.o
