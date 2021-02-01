#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>

#include <unistd.h>

#include "config.h"
#include "device.h"
#include "gateway.h"
#include "hub.h"
#include "utility/hexify.h"

using namespace xiaxr;

void check_root() {
  if (geteuid()) {
    std::cerr << "Xiaxr Hub must be run as root - Exiting" << std::endl;
    exit(EXIT_FAILURE);
  }
}

int main() {
  check_root();
  auto gateway = Gateway::create();
  gateway.begin();

  while (1) {
    gateway.update([]<typename T>(const T &msg) {
      std::cout << hexify(msg.device_id()) << " " << msg.integer_value()
                << std::endl;
    });
    gateway.sleep();
  }

  return EXIT_SUCCESS;
}