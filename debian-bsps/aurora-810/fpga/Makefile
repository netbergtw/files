KSRC=$(shell uname -r)
CFILES=netberg_fpga_node.c utils.c platform.c inc/nb_fpga_drv.c
obj-m += netberg_fpga.o
netberg_fpga-objs := $(CFILES:.c=.o)

all:
	$(MAKE) -C /lib/modules/$(KSRC)/build M=$(shell pwd) modules

clean:
	$(MAKE) -C /lib/modules/$(KSRC)/build M=$(shell pwd) clean
