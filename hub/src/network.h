#pragma once

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <vector>

#include <time.h>

#include "hub.h"

namespace xiaxr {
enum class MessageID : uint8_t { Measurement = 0x10 };

using network_message_t = std::tuple<std::vector<uint8_t>, timespec>;
static inline network_message_t make_network_message(uint8_t *buffer,
                                                     size_t buffer_size) {
  timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);

  return {{buffer, buffer + buffer_size}, ts};
}

struct network_header_t {
  uint8_t device_id[device_id_length];
  uint8_t message_type;
  uint8_t message_length;
} __attribute__((packed));
} // namespace xiaxr