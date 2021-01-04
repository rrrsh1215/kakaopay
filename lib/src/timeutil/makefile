include $(SH_HOME)/cfg/make.mk

##################################
OBJ=timeutil.o
##################################
SRC=$(OBJ:.o=.c)
TARGET=lib$(OBJ:.o=).a
HEAD=$(OBJ:.o=.h)

CFLAGS=$(CFLAG)
CPPFLAGS=$(CPPFLAG) 

#LIBADD
LDFLAGS=$(LDFLAG)

install:$(TARGET)
	$(CP) -r $(TARGET) $(LIBBIN)
	$(CP) -r $(HEAD) $(INC)
	$(RM) $(OBJ) $(TARGET)

$(OBJ):$(SRC)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(SRC)

$(TARGET):$(OBJ)
	$(AR) ruv $(TARGET) $(OBJ)

clean:
	$(RM) -f $(OBJ) $(TARGET) core*
