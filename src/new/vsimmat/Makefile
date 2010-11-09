# ------------------ Compilation options ------------------------
# Location of include files for Vista
INC = 

# Libraries to link with:
LIBS = -lvia -lviaio -lgslcblas -lgsl -lm


# Flags for the C compiler:
CFLAGS = -ansi -Wall -O2
CC = gcc
# --------------------- Code modules ----------------------------

# Name of final program module
PROG = vsimmat

# List all source and object files
SRC = vsimmat.c
OBJ = vsimmat.o
 
# List header files
HDRS = 

# ------------------------ Rules --------------------------------

${PROG}: ${OBJ}
	${CC} -o ${PROG} ${CFLAGS} ${OBJ} ${LIBS}

%.o:%.c
	${CC} -c ${CFLAGS} ${INC} -c $< -o $@

clean:
	-rm -f *.o *~
