#include <unistd.h>

#include "src/server.h"

int main(int argc, char* argv[]) {
  int port = 8080;
  int thread_num = std::thread::hardware_concurrency();
  if (thread_num == 0) thread_num = 4;
  int timeout = 60;

  int opt;
  while ((opt = getopt(argc, argv, "p:t:m:")) != -1) {
    switch (opt) {
      case 'p':
        port = atoi(optarg);
        break;
      case 't':
        thread_num = atoi(optarg);
        break;
      case 'm':
        timeout = atoi(optarg);
        break;
      default:
        // TODO: print usage
        return 1;
    }
  }

  Server server(port, thread_num, timeout * 1000);
  server.Start();

  return 0;
}