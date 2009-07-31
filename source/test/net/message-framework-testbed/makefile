export COMPILE_FLAGS = -Wall -O2



DIRS = \
	   ./core \
	   ./process 



all : subdirs

light: export MF_FLAG = USE_MF_LIGHT
light: all

subdirs : 
	@for dir in $(DIRS); do \
		make -C $$dir || exit $?; \
		done



clean : 
	@for dir in $(DIRS); do \
		make -C $$dir clean; \
		done

