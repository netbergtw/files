KSRC=$(shell uname -r)

obj-m += x86-64-netberg-aurora-621-cpld.o

all:
	$(MAKE) -C /lib/modules/$(KSRC)/build M=$(shell pwd) modules
	rm -f *.o* *.mod*

clean:
	$(MAKE) -C /lib/modules/$(KSRC)/build M=$(shell pwd) clean

install:
	$(MAKE) -C /lib/modules/$(KSRC)/build M=$(shell pwd) modules_install
