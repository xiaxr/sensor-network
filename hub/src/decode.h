#pragma once

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <vector>

#include <time.h>

#include "hub.h"
#include "measurement.h"
#include "network.h"

namespace xiaxr {

template <typename F>
static void decode_network_message(const network_message_t &message_tuple,
                                   F &&func) {
  auto msg = std::get<0>(message_tuple);
  auto ts = std::get<1>(message_tuple);

  if (msg.size() < sizeof(network_header_t)) {
    return;
  }

  network_header_t header;
  memcpy(&header, msg.data(), sizeof(header));

  if (msg.size() < header.message_length) {
    return;
  }

  switch (static_cast<MessageID>(header.message_type)) {
  case MessageID::Measurement:
    func(Measurement(header, msg.data() + sizeof(network_header_t),
                     msg.size() - sizeof(network_header_t), ts));
    return;
  default:
    return;
  }
}

} // namespace xiaxr