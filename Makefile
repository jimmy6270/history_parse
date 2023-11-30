CC       = gcc
WORKDIR  = 
INCLUDES = 
LIBS     =
LINKS    = 
TARGET   = history_parse

CFLAGS += -g -O2
src=$(wildcard *.c ./callback/*.c)
C_OBJS=$(patsubst %.c, %.o,$(src))
#C_OBJS=$(dir:%.c=%.o)

compile:$(TARGET)
	
$(C_OBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $*.o -c $*.c
	
$(TARGET):$(C_OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $^ $(LIBS) $(LINKS) 
	@echo 
	@echo Project has been successfully compiled.
	@echo
	
install: $(TARGET)
	cp $(TARGET) $(INSTALL_PATH)

uninstall:
	rm -f $(INSTALL_PATH)/$(TARGET)

rebuild: clean compile

clean:
	@echo $(PWD)
	rm -rf *.o  $(TARGET) *.log *~
