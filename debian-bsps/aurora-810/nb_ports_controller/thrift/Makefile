LIB_NAME = thrift_rpc_server

THRIFT_CPP_SERVER = n
ifeq ($(THRIFT_CPP_SERVER),n)
THRIFT_CPP_FLAGS = ,no_skeleton
endif

THRIFT_CPP = y
THRIFT_C_GLIB = y
THRIFT_PY = y

CXX_FILES := $(wildcard gen-cpp/*.cpp)
CXX_FILES += $(wildcard src/nb_port_ctrl_rpc_server.cpp)
CXX_OBJS = $(patsubst %.cpp, build/%.o, $(CXX_FILES))

FPGA_O_FILES = $(wildcard ../nb_fpga_utility/src/*.o)

CXX_FLAGS = -I$(SDE)/install/include -I$(PWD)/../inc/ -I$(PWD)/gen-cpp -fPIC
LXX_FLAGS = -L$(SDE)/install/lib -lthrift -Wl,-rpath,\$$ORIGIN/../lib

.PHONY: all clean

all: idl lib/lib$(LIB_NAME).so

idl:
ifeq ($(THRIFT_CPP_SERVER),y)
	@echo Generating IDL Cpp with server
endif
ifeq ($(THRIFT_CPP),y)
	@echo Generating IDL Cpp
	@thrift --gen cpp:cob_style$(THRIFT_CPP_FLAGS) -r ./port_ctrl_rpc.thrift
endif
ifeq ($(THRIFT_C_GLIB),y)
	@echo Generating IDL C glib
	@thrift --gen c_glib -r ./port_ctrl_rpc.thrift
endif
ifeq ($(THRIFT_PY),y)
	@echo Generating IDL Py
	@thrift --gen py -r ./port_ctrl_rpc.thrift
endif

build/%.o: %.cpp
	@mkdir -p $(@D)
	@echo Compiling $@
	@g++ -c $(CXX_FLAGS) $< -o $@ $(LXX_FLAGS)

lib/lib$(LIB_NAME).so: $(CXX_OBJS)
	@mkdir -p $(@D)
	@echo Compiling $@
	@g++ $(CXX_FLAGS) $^ -shared -Wl,-soname,lib$(LIB_NAME).so -o $@ $(LXX_FLAGS) $(FPGA_O_FILES)

THIRFT_C_GLIB=/thrift/lib/c_glib/.libs/libthrift_c_glib.so
GLIB_CFLAGS=-I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
CFLAGS = -g -Wall -Wextra -pedantic -I$(SDE)/install/include -I$(PWD)/../inc/ -I$(PWD)/gen-c_glib -I/usr/local/lib/ $(GLIB_CFLAGS) -fPIC -I/thrift/lib/c_glib/src/thrift
GLIB_LFLAGS = -lgobject-2.0 -lglib-2.0
LFLAGS = $(GLIB_LFLAGS)

C_CLIENT_FILES = $(wildcard gen-c_glib/*.c)
C_CLIENT_OBJS = $(patsubst %.c, build/%.o, $(C_CLIENT_FILES))

build/%.o: %.c
	@mkdir -p $(@D)
	@echo Compiling $@
	@gcc $(CFLAGS) -c $< -o $@ $(LFLAGS)

c_client: $(C_CLIENT_OBJS)
	@echo Compiling Thrift C Client nb_port_ctrl_rpc_client
	@gcc $(CFLAGS) src/nb_port_ctrl_rpc_client.c -o nb_port_ctrl_rpc_client $(C_CLIENT_OBJS) $(THIRFT_C_GLIB) $(LFLAGS)

clean:
	@rm -rf lib/
	@rm -rf build/
	@rm -rf gen-*/
