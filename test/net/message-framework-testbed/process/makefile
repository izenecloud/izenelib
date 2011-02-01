ifeq ($(COMPILE_FLAGS),)
export COMPILE_FLAGS = -Wall -O2
endif


# Setting the flag to USE_MF_LIGHT will build the processes using MF-Light
#export MF_FLAG = USE_MF_LIGHT


TOTAL_DIRS = $(DIRS) $(TEST_DIRS)

DIRS = \
	   ./common \
	   ./controller-process \
	   ./document-process \
	   ./index-process \
	   ./la-process \
	   ./main-process

ifeq ($(MF_FLAG), USE_MF_LIGHT)
	DIRS += ./main-process-light
endif

#TEST_DIRS = \
			./common \
			./client-process \
			./controller-process \
			./server-process



all : subdirs

# TODO: NEED TO DO
#light : export MF_FLAG = USE_MF_LIGHT
#light : subdirs

subdirs : 
	@for dir in $(DIRS); do \
		make -C $$dir || exit $?; \
		done

#testdirs :
#@for dir in $(TEST_DIRS); do \
#make -C $$dir || exit $?; \
#done


clean : 
	@for dir in $(DIRS); do \
		make -C $$dir clean; \
		done
	make -C ./main-process-light clean

