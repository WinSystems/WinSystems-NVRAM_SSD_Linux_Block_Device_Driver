# Makefile for ssd

ifneq ($(KERNELRELEASE),) # called by kbuild
	obj-m := ssd.o
else # called from command line
	KERNEL_VERSION = `uname -r`
	KERNELDIR := /lib/modules/$(KERNEL_VERSION)/build
	PWD  := $(shell pwd)
	MODULE_INSTALLDIR = /lib/modules/$(KERNEL_VERSION)/kernel/drivers/block/
	
EXTRA_CFLAGS+=-fno-pie	

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

ssd48io.o: ssdio.c ssd.h Makefile
	gcc -c $(EXTRA_CFLAGS) ssdio.c

all:    default install lock

install:
	mkdir -p $(MODULE_INSTALLDIR)
	rm -f $(MODULE_INSTALLDIR)ssd.ko
	install -c -m 0644 ssd.ko $(MODULE_INSTALLDIR)
	/sbin/depmod -a

uninstall:
	rm -f $(MODULE_INSTALLDIR)ssd.ko
	/sbin/depmod -a

lock: lock.c ssd.h ssdio.o Makefile
	gcc -static lock.c ssdio.o -o lock
	chmod a+x lock

endif
 
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions /dev/ssd

spotless:
	rm -rf Module.* lock dump *.o *~ core .depend .*.cmd *.ko *.mod.c *.order .tmp_versions /dev/ssd
