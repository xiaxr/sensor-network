#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <time.h>
#include <vector>

#include <time.h>

#include "hub.h"
#include "measurement.h"
#include "network.h"

#include "utility/format_date.h"
#include "utility/hexify.h"

namespace xiaxr {
Measurement::Measurement(const network_header_t &header, uint8_t *payload,
                         size_t payload_length, const timespec &ts)
    : device_id_(), header_(header), ts_(ts), terminate_(false) {
  if (payload_length < sizeof(measurement_message_t)) {
    value_ = (uint64_t)0;
    return;
  }

  memcpy(&message_, payload, payload_length);
  memcpy(device_id_.data(), header_.device_id, device_id_length);

  switch (message_.length) {
  case 1:
    value_ = (uint64_t) * reinterpret_cast<uint8_t *>(message_.data);
    break;
  case 2:
    value_ = (uint64_t) * reinterpret_cast<uint16_t *>(message_.data);
    break;
  case 4:
    if (is_integer()) {
      value_ = (uint64_t) * reinterpret_cast<uint32_t *>(message_.data);
    } else {
      value_ = (double)(*reinterpret_cast<float *>(message_.data));
    }
    break;
  case 8:
    if (is_integer()) {
      value_ = *reinterpret_cast<uint64_t *>(message_.data);
    } else {
      value_ = *reinterpret_cast<double *>(message_.data);
    }
    break;
  default:
    if (is_integer()) {
      value_ = (uint64_t)0;
    } else {
      value_ = 0.0;
    }
    break;
  }
}

auto Measurement::as_json() const -> std::string {
  std::stringstream ss;

  ss << "{\"device_id\": \"" << hexify(device_id_) << std::dec
     << "\", \"timestamp\": \"" << format_date(ts_)
     << "\", \"id\": " << (int)id() << ", \"type\": \"" << get_type_str(type())
     << "\", \"value\": "
     << std::setprecision(std::numeric_limits<long double>::digits10 + 1)
     << double_value() << ", \"suffix\": \"" << get_unit_suffix(unit()) << "\"}"
     << std::endl;

  return ss.str();
}

auto Measurement::db_ts() const -> std::string { return format_ts(ts_); }
auto Measurement::db_device_id() const -> int64_t {
  int64_t id;
  assert(sizeof(id) == device_id_length);
  std::memcpy(&id, device_id_.data(), sizeof(id));
  return id;
}
auto Measurement::db_id() const -> int { return id(); }
auto Measurement::db_tag() const -> std::string {
  std::stringstream ss;
  ss << hexify(device_id_) << "-" << db_type() << "-" << std::dec << db_id();
  return ss.str();
}
auto Measurement::db_type() const -> std::string {
  switch (type()) {
  case MeasurementType::Temperature:
    return "temperature";
  case MeasurementType::Humidity:
    return "humidity";
  case MeasurementType::Pressure:
    return "pressure";
  case MeasurementType::Altitude:
    return "altitude";
  default:
    return "";
  }
}
auto Measurement::db_unit() const -> std::string {
  switch (unit()) {
  case MeasurementUnit::Celsius:
    return "C";
  case MeasurementUnit::RelativeHumidity:
    return "%";
  case MeasurementUnit::Pascals:
    return "Pa";
  case MeasurementUnit::Meters:
    return "m";
  default:
    return "";
  }
}
auto Measurement::db_value() const -> double { return double_value(); }

} // namespace xiaxr
