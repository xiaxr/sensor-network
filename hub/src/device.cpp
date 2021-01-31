#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include "hub.h"
#include "device.h"

namespace xiaxr {
device_id_t Device::generate_device_id() {
  device_id_t device_id{0};
  std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
  urandom.read(reinterpret_cast<char *>(device_id.data()), device_id.size());
  urandom.close();
  return device_id;
}
} // namespace xiaxr
