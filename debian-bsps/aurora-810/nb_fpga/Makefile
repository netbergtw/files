## check your glibc version strings /lib/x86_64-linux-gnu/libc.so.6 |grep GLIBC_
## https://www.itread01.com/content/1544406967.html
TARGET  := nb_fpga
CSRCS   := $(wildcard *.c)
COBJS   := $(CSRCS:.c=.o)
CFLAGS += -Wall -Werror


all: $(TARGET)
	ln -fs $(abspath nb_fpga) /usr/bin
$(TARGET): $(COBJS)
	$(CC) $(CFLAGS) $(COBJS) -o $@

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@
	
clean: 
	@rm -rfv *.o
	@rm -rfv $(TARGET)
