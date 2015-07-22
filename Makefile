EXEC = gateway
OBJS = main.o uart_hal.o listener.o zlg_cmd.o gpio.o serial.o menu.o zlg_protocol.o ota.o server_duty.o xbee_api.o xbee_protocol.o xbee_routine.o xbee_atcmd.o dul_link_list.o ctl_cmd_cache.o parking_state_management.o
SRC  = main.c uart_hal.c listener.c zlg_cmd.c gpio.c serial.c menu.c zlg_protocol.c ota.c server_duty.c xbee_api.c xbee_protocol.c xbee_routine.c xbee_atcmd.c dul_link_list.c ctl_cmd_cache.c parking_state_management.c

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
