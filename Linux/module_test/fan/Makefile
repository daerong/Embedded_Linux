DEV := led
DD := fpga_$(DEV)_driver
TFTPDIR := /tftpboot
CC = arm-none-linux-gnueabi-gcc
KDIR := /root/work/achroimx6q/kernel
PWD := $(shell pwd)
obj-m := $(DD).o

all: $(DD)

$(DD): $(DD).c
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules ARCH=arm

install:
	cp -a $(DD).ko $(TFTPDIR)

clean:
	rm -rf *.ko *.mod.* *.order *.symvers *.o

new:
	$(MAKE) clean
	$(MAKE)