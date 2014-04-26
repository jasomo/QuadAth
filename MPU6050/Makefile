BIN = MPU6050
OBJS = I2C.o MPU6050.o util.o
SRC = MPU6050.c I2C.c util.c
LIBS = -lm -lrt
all: $(BIN) $(LIB)

MPU6050: MPU6050.c
	$(CC) -g $(CFLAGS) $(INCLUDES) $(SRC) $(LIBS) -o $@ -Wno-deprecated-declarations

clean:
	for i in $(OBJS); do (if test -e "$$i"; then ( rm $$i ); fi ); done
	@rm -f $(BIN) $(LIB)


