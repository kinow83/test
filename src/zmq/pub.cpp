#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <sstream>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <zmq.h>

using namespace std;

int main(int argc, char **argv) {
  void *context = zmq_ctx_new();
  void *publisher = zmq_socket(context, ZMQ_PUB);
  int rhwm = 0;
  int nodrop = 0;
  uint64_t c = 1;

  if (argc < 3) {
    cout << "-USAGE: " << argv[0]
         << " <uint number of iterations> <uint log print modulo>" << endl;
    cout << "  -e.g. to send 100,000 messages and log every 100: $" << argv[0]
         << " 100000 100" << endl;
    abort();
  }

  uint64_t loop = atoi(argv[1]);
  size_t optsize = sizeof(int);
  int rcso;

  {
    rcso = zmq_getsockopt(publisher, ZMQ_SNDHWM, &rhwm, &optsize);
    if (!rcso) {
      cout << "Error: zmq_getsockopt(hwm)" << zmq_strerror(rcso) << endl;
    }
    cout << "hwm: " << rhwm << endl;

    rcso = zmq_getsockopt(publisher, ZMQ_XPUB_NODROP, &nodrop, &optsize);
    if (!rcso) {
      cout << "Error: zmq_getsockopt(nodrop)" << zmq_strerror(rcso) << endl;
    }
    cout << "nodrop: " << nodrop << endl;
  }

  rhwm = 10;
  nodrop = 1;
  rcso = zmq_setsockopt(publisher, ZMQ_SNDHWM, &rhwm, sizeof(rhwm));
  if (!rcso) {
    cout << "Error: zmq_setsockopt(hwm)" << zmq_strerror(rcso) << endl;
  }
  rcso = zmq_setsockopt(publisher, ZMQ_XPUB_NODROP, &nodrop, sizeof(nodrop));
  if (!rcso) {
    cout << "Error: zmq_setsockopt(nodrop)" << zmq_strerror(rcso) << endl;
  }

  {
    rcso = zmq_getsockopt(publisher, ZMQ_SNDHWM, &rhwm, &optsize);
    if (!rcso) {
      cout << "Error: zmq_getsockopt(hwm)" << zmq_strerror(rcso) << endl;
    }
    cout << "hwm: " << rhwm << endl;

    rcso = zmq_getsockopt(publisher, ZMQ_XPUB_NODROP, &nodrop, &optsize);
    if (!rcso) {
      cout << "Error: zmq_getsockopt(nodrop)" << zmq_strerror(rcso) << endl;
    }
    cout << "nodrop: " << nodrop << endl;
  }

  zmq_bind(publisher, "tcp://*:5563");

  sleep(1); // idle to ensure connection made to subscriber

  int wait = ZMQ_DONTWAIT;
  int rc = 0;
  int retry = 0;
  while (c <= loop) {
    std::stringstream ss;
    ss << "MESSAGE PAYLOAD OF A NONTRIVAL SIZE KIND OF AND SUCH #" << c;
    zmq_msg_t msg;
    int rc1 = zmq_msg_init_size(&msg, ss.str().length());
    assert(rc1 == 0);
    memcpy(zmq_msg_data(&msg), ss.str().c_str(), ss.str().length());

    rc = zmq_msg_send(&msg, publisher, wait);
    retry = 0;
    if (rc == -1) {
      do {
        usleep(100);
        rc = zmq_msg_send(&msg, publisher, wait);
        retry++;
      } while (rc == -1);
    }

    if (c % atoi(argv[2]) == 0)
      cout << "sent " << ss.str() << ":" << ss.str().size() << " with rc=" << rc
           << " retry=" << retry << endl;
    if (!rc) {
      cout << "Error: " << zmq_strerror(rc) << endl;
      abort();
      sleep(1);
    }

    ++c;
  }
  zmq_close(publisher);
  zmq_ctx_destroy(context);
}
