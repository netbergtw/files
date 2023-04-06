#include <dlfcn.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// clang-format off
#include <inc/nb_ports_ctrl.h>
// clang-format on
#ifdef UNIT_TEST
#include <unit_test/unit_test.h>
#endif
#define VERSION "1.0.1"

static int start_rpc_server() {
  int (*start_func)();
  void *handle = NULL;
  char *error;

  handle = dlopen("./thrift/lib/libthrift_rpc_server.so", RTLD_LAZY);
  if ((error = dlerror()) != NULL) {
    printf("ERROR:%s:%d: dlopen failed for %s, err=%s\n", __func__, __LINE__,
           "libthrift_rpc_server", error);
    return EIO;
  }
  start_func = (int (*)(void))dlsym(handle, "start_rpc_server");
  if ((error = dlerror()) != NULL) {
    printf("ERROR:%s:%d: dlopen failed for %s, err=%s\n", __func__, __LINE__,
           "libthrift_rpc_server", error);
    return EIO;
  }
  int status = (*start_func)();
  return status;
}

static int stop_rpc_server() {
  int (*start_func)();
  void *handle = NULL;
  char *error;

  handle = dlopen("./thrift/lib/libthrift_rpc_server.so", RTLD_LAZY);
  if ((error = dlerror()) != NULL) {
    printf("ERROR:%s:%d: dlopen failed for %s, err=%s\n", __func__, __LINE__,
           "libthrift_rpc_server", error);
    return EIO;
  }
  start_func = (int (*)(void))dlsym(handle, "stop_rpc_server");
  if ((error = dlerror()) != NULL) {
    printf("ERROR:%s:%d: dlopen failed for %s, err=%s\n", __func__, __LINE__,
           "libthrift_rpc_server", error);
    return EIO;
  }
  int status = (*start_func)();
  return status;
}

int nb_ports_controller_init(bool rpc) {
  int status;
  nb_info("Ports Ctrl IO Init Start\n");

  if (rpc) {
    nb_info("RPC server initializing\n");
    status = start_rpc_server();
    if (status) return status;
  }

  /*SN Ports Init*/
  if (nb_ports_ctrl_init() != NB_SUCCESS) {
    nb_err("Ports Ctrl IO Init Fail\n");
    return -NB_FAIL;
  }

  /*SN Cache Init*/
  if (nb_ports_cache_init() != NB_SUCCESS) {
    nb_err("Ports Ctrl Cache Init Fail\n");
    return -NB_FAIL;
  }
  return NB_SUCCESS;
}

void nb_ports_controller_destroy(bool rpc) {
  if(rpc){
    stop_rpc_server();
  }
  nb_ports_ctrl_deinit();
}

int main(int argc, char **argv) {
  if (strcmp(argv[1], "-v") == 0){
      printf("version=%s\n", VERSION);
      return NB_SUCCESS;
  }

  bool rpc = false;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-rpc") == 0) rpc = true;
  }

  if (nb_ports_controller_init(rpc) != NB_SUCCESS) {
    nb_err("Port Controller Init Fail\n");
    nb_ports_controller_destroy(rpc);
    return -NB_FAIL;
  }
  /*Start Ctrl*/
#ifdef UNIT_TEST
  if (argc > 1) {
    if ((strcmp(argv[1], "test") == 0) && argc == 3) {
      BSP_behavior_test(strtol(argv[2], NULL, 0));
    }
  } else {
    printf("needs args\n");
  }
#endif
  if (rpc)
    while (true) sleep(60);
  nb_ports_controller_destroy(rpc);
  return NB_SUCCESS;
}
