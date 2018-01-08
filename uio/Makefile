



PWD := $(shell pwd)

KERNEL_DIR := /usr/src/linux-headers-$(shell uname -r)

obj-m := kpart.o

all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
	@gcc user_write.c -o write

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
	@rm -f write



