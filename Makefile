BIN     = mpi-bench

############################################################################
# List object files that comprise BIN.

OBJS    = mpi-bench.o

############################################################################
# Build flags
# Override with "make CC=gcc", "make CC=icc", etc.
# Do not add non-portable options (such as -Wall) using +=

# Portable defaults.  Can be overridden by mk.conf or command line.
CC          ?= mpicc
MKDIR       ?= mkdir
INSTALL     ?= install

PREFIX      ?= /usr/local

INCLUDES    += -I${PKGSRC}/include
CFLAGS      += ${INCLUDES}
LFLAGS      += -L${PKGSRC}/lib

############################################################################
# Standard targets required by ports

all:    ${BIN}

# Link rules
${BIN}: ${OBJS}
	${CC} -o ${BIN} ${OBJS} ${LFLAGS}

mpi-bench.o: mpi-bench.c mpi-bench.h protos.h
	${CC} -c ${CFLAGS} mpi-bench.c


install:
	${MKDIR} -p ${DESTDIR}${PREFIX}/bin
	${INSTALL} -c -s mpi-bench ${DESTDIR}${PREFIX}/bin

############################################################################
# Remove generated files (objs and nroff output from man pages)

clean:
	rm -f ${OBJS} ${BIN} *.nr

# Keep backup files during normal clean, but provide an option to remove them
realclean: clean
	rm -f .*.bak *.bak *.BAK *.gmon core *.core

