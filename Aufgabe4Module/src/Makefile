PWD		:= $(shell pwd)
KERNELDIR ?= /lib/modules/3.7.10-1.40-desktop/build
obj-m	+= translate.o
all:
	make -C $(KERNELDIR) M=$(PWD) modules
	
clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	