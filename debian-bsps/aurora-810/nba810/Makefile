KSRC=$(shell uname -r)

CPLATS=platform/plat.c \
       platform/nba810/plat_nba810.c \
       platform/nba810/plat_port.c \
	   platform/nba810/plat_fpga.c

CPORTSFS=interface/sysfs/ports/ports.c

CPORTSIO=interface/ioctl/ports_io.c
CLEDSIO=interface/ioctl/leds_io.c

obj-m += nb_platform.o \
         nb_ports_sysfs.o \
         nb_ports_ioctl.o \
         nb_leds_ioctl.o 

nb_platform-objs := $(CPLATS:.c=.o)
nb_ports_sysfs-objs := $(CPORTSFS:.c=.o)
nb_ports_ioctl-objs := $(CPORTSIO:.c=.o)
nb_leds_ioctl-objs := $(CLEDSIO:.c=.o)


all:
	$(MAKE) -C /lib/modules/$(KSRC)/build M=$(shell pwd) modules
	rm -f *.o* *.mod*

clean:
	$(MAKE) -C /lib/modules/$(KSRC)/build M=$(shell pwd) clean

install:
	$(MAKE) -C /lib/modules/$(KSRC)/build M=$(shell pwd) modules_install
