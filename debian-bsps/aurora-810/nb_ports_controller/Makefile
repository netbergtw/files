.PHONY: thrift

NB_TEMPLETE=NO
NB_FPGA_UTIL=YES
NB_UNIT_TEST=NO

CC     := gcc
INC    := -I.
CFLAGS += -Wall -Werror -fPIC
TARGET := nb_ports_controller
THRIFT_SRC := thrift
LIBRARY += -lm -lrt -ldl

ifeq ($(NB_TEMPLETE),YES)
	CFILES := $(wildcard ./nb_templete/*.c)
endif
ifeq ($(NB_FPGA_UTIL),YES)
	CFILES := $(wildcard ./nb_fpga_utility/src/*.c)
endif
ifeq ($(NB_UNIT_TEST),YES)
	COMPILE_UNIT_TEST = -DUNIT_TEST
	CFILES += $(wildcard ./unit_test/*.c)
endif

#CFILES += $(wildcard ./*.c)
CFILES += nb_ports_controller.c
COBJS  := $(CFILES:.c=.o)



NAME = nb_ports_controller-$(USER)

docker:
ifeq ($(shell docker ps -a | grep  $(NAME)), "")
	@docker run --rm -di -v $(PWD):/nb_ports_controller --name $(NAME) thrift:0.14.1
endif


attach:
	@docker exec -it $(NAME) bash

thrift:
	@echo Compiling thrift IDL + server so
	@cd $(THRIFT_SRC); make all; make all;

all: $(TARGET)
	make thrift
	rm $(TARGET)
	@make $(TARGET)

$(TARGET): $(COBJS)
		$(CC) $(CFLAGS) $(COBJS) -o $@ $(LIBRARY)

%.o: %.c
		@$(CC) $(INC) $(CFLAGS) $(COMPILE_UNIT_TEST) -c $< -o $@

clean:
	@cd $(THRIFT_SRC); make clean
	@rm -rfv *.o
	@rm -rfv $(TARGET)
ifeq ($(NB_TEMPLETE),YES)
	@rm -rfv ./nb_templete/*.o
endif
ifeq ($(NB_FPGA_UTIL),YES)
	@rm -rfv ./nb_fpga_utility/src/*.o
endif
ifeq ($(NB_UNIT_TEST),YES)
	@rm -rfv ./unit_test/*.o
endif
