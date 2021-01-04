include $(SH_HOME)/cfg/make.mk

##################################
OBJ=client.o \
##################################
#OBJS=	11cert.o		\
#		12authterm.o	\
#		13authuser.o	\
#
##################################
SRC=$(OBJ:.o=.c)
TARGET=$(OBJ:.o=)

CFLAGS=$(CFLAG) 
CPPFLAGS=$(CPPFLAG) -I$(SH_HOME)/opensource/include

LIBADDS=$(LIBADD) -L$(SH_HOME)/opensource/lib

LDFLAGS= -ltimeutil -lcastutil -ldbutil -lseedutil -lcfgutil -lsqlite3 -ljson-c -lssl -lcrypto -lm

install:$(TARGET)
	$(CP) $(TARGET) $(BIN)
	$(RM) $(OBJ) $(OBJS) $(TARGET)

$(OBJ):$(SRC)
	$(CC) -D_REENTRANT -D_THREAD_SAFE $(CFLAGS) $(CPPFLAGS) -c $(SRC)

$(TARGET):$(OBJ) $(OBJS)
	$(CC) -D_REENTRANT -D_THREAD_SAFE -o $(TARGET) $(OBJ) $(OBJS) $(LIBADDS) $(LDFLAGS) 

clean:
	$(RM) -f $(OBJ) $(OBJS) $(TARGET) core*
