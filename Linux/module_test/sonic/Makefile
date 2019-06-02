# Makefile for a basic kernel module
obj-m += ext_sensor_driver.o
obj-m += hc-sr04_driver.o
KDIR := /work/achroimx6q/achroimx_kernel
all:
make -C $(KDIR) SUBDIRS=$(PWD) modules
app:
arm-none-linux-gnueabi-gcc ext_sensor_app.c -o ext_sens_test -static
arm-none-linux-gnueabi-gcc hc-sr04_app.c -o us_test -static
install:
cp -a *.ko /nfsroot/
cp -a *test /nfsroot/
clean:
rm -rf *.ko
rm -rf *.mod*