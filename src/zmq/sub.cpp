#include <assert.h>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zmq.h>
using namespace std;

int main(int argc, char **argv) {
  void *context = zmq_ctx_new();
  void *subscriber = zmq_socket(context, ZMQ_SUB);
  uint64_t rhwm = 10;

  if (argc < 2) {
    cout << "-USAGE: " << argv[0] << " <uint log print modulo>" << endl;
    abort();
  }
  uint64_t mod = atoi(argv[1]);

  int rcso = zmq_setsockopt(subscriber, ZMQ_RCVHWM, &rhwm, sizeof(rhwm));
  if (!rcso) {
    cout << "Error: " << zmq_strerror(rcso) << endl;
  }

  int rcc = zmq_connect(subscriber, "tcp://127.0.0.1:5563");
  assert(rcc == 0);

  int rc = zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", strlen(""));
  assert(rc == 0);
  uint64_t c = 0;

  while (true) {

    zmq_msg_t msg;
    int rc = zmq_msg_init(&msg);
    assert(rc == 0);

    int rcr = zmq_msg_recv(&msg, subscriber, 0);
    assert(rcr != -1);
    if (++c % mod == 0) {
      string rpl = std::string(static_cast<char *>(zmq_msg_data(&msg)),
                               zmq_msg_size(&msg));

      cout << "RECV:" << c << "|" << rpl << endl;
    }
    zmq_msg_close(&msg);
    usleep(1000);
  }
}
