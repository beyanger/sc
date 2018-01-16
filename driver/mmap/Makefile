



PWD := $(shell pwd)

KERNEL_DIR := /usr/src/linux-headers-$(shell uname -r)

obj-m := mmap_demo.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean



