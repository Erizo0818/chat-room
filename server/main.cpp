#include "server.hpp"
#include <pthread.h>

int main() {
  chatroom::net::Server server("0.0.0.0", 8888, 4);

  server.run();
  return 0;
}