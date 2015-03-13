CC = gcc

# optimizations [-O1/2/3] or debugging [-g]
# OPTFLAGS = -O3 -fno-strict-aliasing -D_GNU_SOURCE
OPTFLAGS = 
# COPTIONS = -DLINUX -D_FILE_OFFSET_BITS=64 -std=c99 -Wall\
           -Wno-unused-function -Wno-unused-label -Wno-unused-variable\
           -Wno-parentheses -Wsequence-point
COPTIONS = 

CFLAGS = $(COPTIONS)  $(OPTFLAGS)

# libraries: math.h, [-lm]
LIBS = -lm

# pthreads source files 
PTHRDSRC = lcs_pthreads.c 

# openmp source files 
OPNMPSRC = lcs_openmp.c 

# mpi source files 
MPISRC = lcs_mpi.c

default:
	( $(CC) $(CFLAGS) -I. -pthread -o lcs_pthreads $(PTHRDSRC) $(LIBS))
	( $(CC) $(CFLAGS) -I. -fopenmp -o lcs_openmp $(OPNMPSRC) $(LIBS))
	( mpicc -o lcs_mpi.ex $(MPISRC) )
	
clean:
	rm -f *.o  ;

realclean:
	rm -f *.o \
	rm -f lcs_pthreads \
	rm -f lcs_openmp \
	rm -f lcs_mpi ;

