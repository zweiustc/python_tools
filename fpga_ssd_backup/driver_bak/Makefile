obj-m:=ssd_drv.o
KERNELDIR:=/home/ssd/linux2-6-32_1-9-0-0_PD_BL_ssd
CC:=gcc
PWD := $(shell pwd)
INCLUDE :=-I.
###EXTRA_CFLAGS+=-fprofile-arcs -ftest-coverage
default:
	$(MAKE) -C $(KERNELDIR) $(INCLUDE) M=$(PWD) -Wall -Werror  modules
	sh build.sh
clean:
	rm -f *.o *.ko *.mod.c *.order *.symvers
	rm -fr .ssd_drv.*
	rm -fr output
