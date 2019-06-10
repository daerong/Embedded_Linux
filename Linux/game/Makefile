APP := move
TARGET := mouse_move.c
TFTPDIR := /tftpboot
CC = arm-none-linux-gnueabi-gcc
KDIR := /root/work/achroimx6q/kernel
PWD := $(shell pwd)

all: $(APP)

$(APP): $(TARGET)
	$(CC) -pthread -o $(APP) $(TARGET)

install:
	cp -a $(APP) $(TFTPDIR)

clean:
	rm -rf *.ko *.mod.* *.order *.symvers *.o $(APP)

new:
	$(MAKE) clean
	$(MAKE)