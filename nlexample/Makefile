KDIR := /lib/modules/$(shell uname -r)/build

obj-m += netlinkKernel.o

all:
	$(MAKE) -C $(KDIR) M=$(shell pwd) modules

clean:
	rm -rf *.o *.ko *.mod.* *.cmd .module* modules* Module* .*.cmd .tmp*
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
