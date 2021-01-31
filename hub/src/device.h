#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "hub.h"

namespace xiaxr {
class Device {
public:
  Device() : device_id_{}, name_{}, description_{} {}
  Device(device_id_t device_id, std::string name, std::string description = "")
      : device_id_{device_id}, name_{name}, description_{description} {}

  static device_id_t generate_device_id();

  auto device_id() -> device_id_t & { return device_id_; }
  auto device_id() const -> const device_id_t & { return device_id_; }

  auto name() -> std::string & { return name_; }
  auto name() const -> const std::string & { return name_; }

  auto description() -> std::string & { return description_; }
  auto description() const -> const std::string & { return description_; }

private:
  device_id_t device_id_;
  std::string name_;
  std::string description_;
};
} // namespace xiaxr