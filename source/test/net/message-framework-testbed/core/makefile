ifeq ($(COMPILE_FLAGS),)
export COMPILE_FLAGS = -Wall -O2
endif
#export MF_FLAG =


DIRS = \
	   ./document-manager/ \
	   ./index-manager/ \
	   ./la-manager	 

all : subdirs

subdirs : 
	@for dir in $(DIRS); do \
		make -C $$dir || exit $?; \
		done

clean : 
	@for dir in $(DIRS); do \
		make -C $$dir clean; \
		done

