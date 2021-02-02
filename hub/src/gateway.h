#pragma once

#include <cstddef>
#include <cstdint>
#include <queue>

#include "RF24X/RF24.h"
#include "RF24NetworkX/RF24Network.h"

#include "config.h"
#include "decode.h"
#include "device.h"
#include "hub.h"
#include "network.h"

namespace xiaxr {

class Gateway {
public:
  Gateway(const Configuration &config);

  auto device_id() -> const device_id_t { return device_.device_id(); }
  auto network_channel() -> const uint8_t { return network_channel_; }

  auto update_device_id(const device_id_t &device_id) -> void;
  auto update_network_channel(const uint8_t network_channel) -> void;

  auto begin() -> void;

  template <typename F> auto update(F &&func) -> void {
    queue_incoming_messages();
    while (!rx_queue_.empty() && network_.external_queue.size() == 0) {
      auto message = rx_queue_.front();
      decode_network_message(message, func);
      rx_queue_.pop();
    }
  }

  auto power_up() -> void;
  auto power_down() -> void;

private:
  auto queue_incoming_messages() -> void;

  RF24 radio_;
  RF24Network network_;
  Device device_;
  uint8_t network_channel_;
  std::queue<network_message_t> rx_queue_;
};
} // namespace xiaxr