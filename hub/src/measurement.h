#pragma once

#include <cstddef>
#include <cstdint>
#include <variant>
#include <vector>

#include <time.h>

#include "hub.h"
#include "network.h"

namespace xiaxr {

enum class MeasurementType : uint8_t {
  None = 0x00,
  Temperature = 0x01,
  Humidity = 0x2,
  Pressure = 0x10,
};

enum class MeasurementUnit : uint8_t {
  None = 0x00,
  Celsius = 0x01,
};

struct measurement_message_t {
  uint8_t id;
  uint8_t type;
  uint8_t unit;
  uint8_t flags : 4;
  uint8_t length : 4;
  uint8_t data[8];
} __attribute__((packed));

class Measurement {
public:
  Measurement(const network_header_t &header, uint8_t *payload,
              size_t payload_length, const timespec &ts);

  auto id() const -> const uint8_t { return message_.id; }
  auto type() const -> const MeasurementType {
    return static_cast<MeasurementType>(message_.type);
  }
  auto unit() const -> const MeasurementUnit {
    return static_cast<MeasurementUnit>(message_.unit);
  }
  auto double_value() const -> const double {
    if (message_.flags & 0x1) {
      return std::get<double>(value_);
    }
    return (double)std::get<uint64_t>(value_);
  }
  auto integer_value() const -> const uint64_t {
    if (!message_.flags) {
      return std::get<uint64_t>(value_);
    }
    return (uint64_t)std::get<double>(value_);
  }
  auto is_integer() const -> const bool { return !message_.flags; }

  auto device_id() const -> const device_id_t & { return device_id_; }
  auto ts() const -> const timespec & { return ts_; }
  auto message_type() const -> const MessageID {
    return static_cast<MessageID>(header_.message_type);
  }

private:
  device_id_t device_id_;
  network_header_t header_;
  measurement_message_t message_;
  timespec ts_;
  std::variant<uint64_t, double> value_;
};

} // namespace xiaxr