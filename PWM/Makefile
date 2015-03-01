BIN = ESC
OBJS = ESC.o pwm.o ncurseswrapper.o
SRC = ESC.c pwm.c ncurseswrapper.c
LIBS = -lncurses -lmenu
all: $(BIN) $(LIB)

ESC: ESC.c
	$(CC) -g $(CFLAGS) $(INCLUDES) $(SRC) $(LIBS) -o $@ -Wno-deprecated-declarations

clean:
	for i in $(OBJS); do (if test -e "$$i"; then ( rm $$i ); fi ); done
	@rm -f $(BIN) $(LIB)


