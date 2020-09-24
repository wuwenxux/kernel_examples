KERNELRELEASE ?= $(shell uname -r)
DEPMOD ?= depmod

obj-m := ngmwan.o 

ngmwan-objs := main.o device.o netlink.o udpsocket.o

KDIR:=/lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

all:
	make -C $(KDIR)  M=$(PWD)  modules

install:
	make -C $(KDIR)  M=$(PWD)  modules_install
	$(DEPMOD) -a $(KERNELRELEASE)

clean:
	make -C $(KDIR)  M=$(PWD)  clean
