DEV := game
APP := fpga_$(DEV)_test
TFTPDIR := /tftpboot
CC = arm-none-linux-gnueabi-gcc
KDIR := /root/work/achroimx6q/kernel
PWD := $(shell pwd)

all: $(APP)

$(APP): $(APP).c
	$(CC) -o $(APP) $(APP).c

install:
	cp -a $(APP) $(TFTPDIR)

clean:
	rm -rf *.ko *.mod.* *.order *.symvers *.o $(APP)

new:
	$(MAKE) clean
	$(MAKE)