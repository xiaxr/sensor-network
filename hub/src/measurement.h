#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <variant>
#include <vector>

#include <time.h>

#include "hub.h"
#include "network.h"

namespace xiaxr {
enum class MeasurementType : uint8_t {
  None = 0x00,
  Temperature = 0x01,
  Humidity = 0x02,
  Pressure = 0x03,
  Altitude = 0x04,
};

namespace {
inline std::string get_type_str(MeasurementType type) {
  if (type == MeasurementType::Temperature) {
    return "temperature";
  }
  if (type == MeasurementType::Humidity) {
    return "humidity";
  }
  if (type == MeasurementType::Pressure) {
    return "pressure";
  }
  if (type == MeasurementType::Altitude) {
    return "altitude";
  }

  return "";
}
} // namespace

enum class MeasurementUnit : uint8_t {
  None = 0x00,
  Celsius = 0x01,
  RelativeHumidity = 0x10,
  Pascals = 0x20,
  Meters = 0x30,
};

namespace {
inline std::string get_unit_suffix(MeasurementUnit unit) {
  if (unit == MeasurementUnit::Celsius) {
    return "°C";
  }
  if (unit == MeasurementUnit::RelativeHumidity) {
    return "%";
  }
  if (unit == MeasurementUnit::Pascals) {
    return "Pa";
  }
  if (unit == MeasurementUnit::Meters) {
    return "m";
  }

  return "";
}
} // namespace

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
  Measurement() : terminate_(false) {}
  Measurement(const network_header_t &header, uint8_t *payload,
              size_t payload_length, const timespec &ts);

  static Measurement terminate_message() {
    auto m = Measurement();
    m.terminate_ = true;
    return m;
  }

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
  auto timestamp_ns() const -> const int64_t {
    return (int64_t)(ts_.tv_sec) * (int64_t)1000000000 + (int64_t)(ts_.tv_nsec);
  }
  auto message_type() const -> const MessageID {
    return static_cast<MessageID>(header_.message_type);
  }

  auto db_ts() const ->  std::string;
  auto db_device_id() const ->  int64_t;
  auto db_id() const -> int;
  auto db_tag() const -> std::string;
  auto db_type() const ->  std::string;
  auto db_unit() const ->  std::string;
  auto db_value() const ->  double;

  auto is_terminate() const -> const bool { return terminate_; }

  auto as_json() const -> std::string;

private:
  device_id_t device_id_;
  network_header_t header_;
  measurement_message_t message_;
  timespec ts_;
  std::variant<uint64_t, double> value_;
  bool terminate_;
};

} // namespace xiaxr