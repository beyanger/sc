



PWD := $(shell pwd)

KERNEL_DIR := /usr/src/linux-headers-$(shell uname -r)

obj-m := vmxnet3.o
vmxnet3-objs := vmxnet3_drv.o vmxnet3_ethtool.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean



