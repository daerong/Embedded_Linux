APP := move
TARGET := mouse_move.c
TFTPDIR := /tftpboot
CC = arm-none-linux-gnueabi-gcc
KDIR := /root/work/achroimx6q/kernel
PWD := $(shell pwd)

all: $(APP)

$(APP): $(TARGET)
	$(CC) -pthread -o $(APP) $(TARGET)

hold:
	arm-none-linux-gnueabi-gcc -pthread -o work mouse_move_work.c
install:
	cp -a work $(APP) $(TFTPDIR)

clean:
	rm -rf *.ko *.mod.* *.order *.symvers *.o $(APP)

new:
	$(MAKE) clean
	$(MAKE)