#include "tcp_logger.h"

int main(int argc, char** argv) {
  TcpLogger logger(640, 480, 30);
  logger.start();
  return 0;
}
