BIN=$(SH_HOME)/bin

LIB=$(SH_HOME)/lib
LIBBIN=$(SH_HOME)/lib/bin
LIBSRC=$(SH_HOME)/lib/src

INC=$(SH_HOME)/inc

CFLAG=-g -Wall -p -O2
CPPFLAG=-I. -I$(INC)

LIBADD=-L$(LIBBIN)




CC=gcc
RM=rm
CP=cp
MV=mv

AR=ar

# link option
# pthread : -pthread
# time : -lrt

#openssl compile
LIBCRYPTO_CFLAGS    = -I/usr/local/include
LIBCRYPTO_LIB_CFLAG = -L/usr/lib -L/usr/lib64
LIBCRYPTO_LIBS      = -lssl -lcrypto -ldl

