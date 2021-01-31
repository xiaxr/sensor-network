#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "hub.h"

namespace xiaxr {
class Configuration {
public:
  Configuration()
      : device_id_{}, network_channel_{network::default_channel},
        ce_pin_{network::default_ce_pin}, csn_pin_{network::default_csn_pin} {}
  Configuration(device_id_t device_id, uint8_t network_channel, uint8_t ce_pin,
                uint8_t csn_pin)
      : device_id_{device_id},
        network_channel_{network_channel}, ce_pin_{ce_pin}, csn_pin_{csn_pin} {}

  void save();
  void reload();
  static Configuration load();

  auto device_id() -> device_id_t & { return device_id_; }
  auto device_id() const -> const device_id_t & { return device_id_; }

  auto network_channel() -> uint8_t & { return network_channel_; }
  auto network_channel() const -> const uint8_t & { return network_channel_; }

  auto ce_pin() -> uint8_t & { return ce_pin_; }
  auto ce_pin() const -> const uint8_t & { return ce_pin_; }

  auto csn_pin() -> uint8_t & { return csn_pin_; }
  auto csn_pin() const -> const uint8_t & { return csn_pin_; }

private:
  device_id_t device_id_;
  uint8_t network_channel_;
  uint8_t ce_pin_;
  uint8_t csn_pin_;
};
} // namespace xiaxr
