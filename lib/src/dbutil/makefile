include $(SH_HOME)/cfg/make.mk

##################################
OBJ=dbutil.o
##################################
SRC=$(OBJ:.o=.c)
TARGET=lib$(OBJ:.o=.a)
HEADER=$(OBJ:.o=.h)

CFLAGS=$(CFLAG)
CPPFLAGS=$(CPPFLAG)
LIBADDS=$(LIBADD)
LDFLAGS=$(LDFLAG) 

install:$(TARGET)
	$(CP) -r $(TARGET) $(LIBBIN)
	$(CP) -r $(HEADER) $(INC)
	$(RM) -f $(OBJ) $(TARGET)

$(OBJ):$(SRC)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $(SRC)

$(TARGET):$(OBJ)
	$(AR) ruv $(TARGET) $(OBJ)

clean:
	$(RM) -f $(OBJ) $(TARGET) core*
