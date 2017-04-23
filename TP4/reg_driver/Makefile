obj-m := reg_driver.o

KERNEL_DIR = /usr/src/linux-headers-4.4.0-57-generic/

all:
		$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules

clean:
		rm -rf *.o *.ko *.mod.* *.symvers *.order *~
