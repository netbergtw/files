#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TServerSocket.h>

#include "PortCtrl.h"
#include "rpc_server.h"
#include "nb_ports_ctrl.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using namespace ::sn;

static const int port = 8777;

class PortCtrlHandler : virtual public PortCtrlIf {
 public:
  PortCtrlHandler() {
    // Your initialization goes here
  }

  nb_ports_ctrl_ports_presence_t nb_all_presence_get() {
    uint32_t all_presence;
    int status = nb_ports_ctrl_presnece_get(&all_presence);
    if (status != NB_SUCCESS) {
      InvalidPortCtrlOperation op = sn::InvalidPortCtrlOperation();
      op.code = status;
      throw op;
    };
    return (nb_ports_ctrl_ports_presence_t)all_presence;
  }

  nb_port_presence_t nb_port_presence_get(const int32_t port_num) {
    nb_port_presence_t presence;
    int status = nb_ports_ctrl_port_presnece_get(port_num, &presence);
    if (status != NB_SUCCESS) {
      InvalidPortCtrlOperation op = sn::InvalidPortCtrlOperation();
      op.code = status;
      throw op;
    };
    return presence;
  }

  nb_ports_ctrl_ports_lpmode_t nb_all_lpmode_get() {
    uint32_t all_lpmode;
    int status = nb_ports_ctrl_lpmode_get(&all_lpmode);
    if (status != NB_SUCCESS) {
      InvalidPortCtrlOperation op = sn::InvalidPortCtrlOperation();
      op.code = status;
      throw op;
    };
    return (nb_ports_ctrl_ports_lpmode_t)all_lpmode;
  }

  nb_port_lpmode_t nb_port_lpmode_get(const int32_t port_num) {
    nb_port_lpmode_t lpmode;
    int status = nb_ports_ctrl_port_lpmode_get(port_num, &lpmode);
    if (status != NB_SUCCESS) {
      InvalidPortCtrlOperation op = sn::InvalidPortCtrlOperation();
      op.code = status;
      throw op;
    };
    return lpmode;
  }

  void nb_port_lpmode_set(const int32_t port_num, const bool lpmode) {
    int status = nb_ports_ctrl_port_lpmode_set(port_num, lpmode);
    if (status != NB_SUCCESS) {
      InvalidPortCtrlOperation op = sn::InvalidPortCtrlOperation();
      op.code = status;
      throw op;
    };
  }

  void nb_port_reset(const int32_t port_num, const bool reset) {
    int status = nb_ports_ctrl_port_reset_set(port_num, reset);
    if (status != NB_SUCCESS) {
      InvalidPortCtrlOperation op = sn::InvalidPortCtrlOperation();
      op.code = status;
      throw op;
    };
  }

  void nb_port_led_set(const int32_t port_num, const int8_t led_mode) {
    int status = nb_ports_ctrl_port_led_set(port_num, led_mode);
    if (status != NB_SUCCESS) {
      InvalidPortCtrlOperation op = sn::InvalidPortCtrlOperation();
      op.code = status;
      throw op;
    };
  }

  void nb_port_eeprom_get(std::string &_return, const int32_t port_num,
                          const int16_t offset, const int16_t length) {
    if (length >= 256) throw sn::InvalidPortCtrlOperation();
    std::vector<uint8_t> data(length);
    int status =
        nb_ports_ctrl_port_eeprom_get(port_num, offset, length, &data.front());
    _return.assign(data.begin(), data.end());
    if (status != NB_SUCCESS) {
      InvalidPortCtrlOperation op = sn::InvalidPortCtrlOperation();
      op.code = status;
      throw op;
    };
  }

  void nb_port_eeprom_set(const int32_t port_num, const int16_t offset,
                          const std::string &data) {
    uint8_t length = data.size();
    std::vector<uint8_t> u8_data(data.begin(), data.end());
    int status =
        nb_ports_ctrl_port_eeprom_set(port_num, offset, length, u8_data.data());
    if (status != NB_SUCCESS) {
      InvalidPortCtrlOperation op = sn::InvalidPortCtrlOperation();
      op.code = status;
      throw op;
    };
  }
};

static void *rpc_server_thread(void *) {
  ::std::shared_ptr<PortCtrlHandler> handler(new PortCtrlHandler());
  ::std::shared_ptr<TProcessor> processor(new PortCtrlProcessor(handler));
  ::std::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  ::std::shared_ptr<TTransportFactory> transportFactory(
      new TBufferedTransportFactory());
  ::std::shared_ptr<TProtocolFactory> protocolFactory(
      new TBinaryProtocolFactory());

  TThreadedServer server(processor, serverTransport, transportFactory,
                         protocolFactory);
  server.serve();
  return NULL;
}

static pthread_t rpc_thread_id = 0;

int start_rpc_server(void) {
  printf("Starting RPC server\n");
  int status = pthread_create(&rpc_thread_id, NULL, rpc_server_thread, NULL);
  if (status) return status;
  printf("Started RPC server\n");
  return 0;
}

int stop_rpc_server(void) {
  int status = 0;

  if (rpc_thread_id) {
    status = pthread_cancel(rpc_thread_id);
    if (status == 0) {
      pthread_join(rpc_thread_id, NULL);
    }
    rpc_thread_id = 0;
  }
  return 0;
}
