# Makefile for a basic kernel module
obj-m += ext_sensor_driver.o
obj-m += hc-sr04_driver.o
KDIR := /root/work/achroimx6q/kernel
PWD := $(shell pwd)
CC = arm-none-linux-gnueabi-gcc

all:
	make -C $(KDIR) SUBDIRS=$(PWD) modules
app:
	$(CC) ext_sensor_app.c -o ext_sens_test -static
	$(CC) hc-sr04_app.c -o us_test -static
install:
	cp -a *.ko /tftpboot/
	cp -a *test /tftpboot/
clean:
	rm -rf *.ko
	rm -rf *.mod*
	rm -rf .*.cmd
	rm -rf Module.symvers
	rm -rf modules.order
	rm -rf *.o
	rm -rf ext_sens_test
	rm -rf us_test