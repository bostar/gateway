EXEC = gateway
OBJS = main.o uart_hal.o listener.o zlg_cmd.o gpio.o serial.o menu.o zlg_protocol.o ota.o
SRC  = main.c uart_hal.c listener.c zlg_cmd.c gpio.c serial.c menu.c zlg_protocol.c ota.c

CC = arm-none-linux-gnueabi-gcc
CFLAGS += -O2 -Wall 
LDFLAGS += 

all:$(EXEC)

$(EXEC):$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) -lpthread

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -vf $(EXEC) *.o *~
