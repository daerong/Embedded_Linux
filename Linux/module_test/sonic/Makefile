# Makefile for a basic kernel module
obj-m += ext_sensor_driver.o
obj-m += hc-sr04_driver.o
KDIR := /work/achroimx6q/achroimx_kernel
all:
make -C $(KDIR) SUBDIRS=$(PWD) modules
app:
arm-none-linux-gnueabi-gcc -o ext_sensor_test ext_sensor_app.c -static
arm-none-linux-gnueabi-gcc -o us_test hc-sr04_app.c -static
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