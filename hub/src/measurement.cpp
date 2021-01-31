#include <cstddef>
#include <cstdint>
#include <time.h>
#include <vector>

#include "hub.h"
#include "measurement.h"
#include "network.h"

#include <iostream>

namespace xiaxr {
Measurement::Measurement(const network_header_t &header, uint8_t *payload,
                         size_t payload_length, const timespec &ts)
    : device_id_(), header_(header), ts_(ts) {
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
      value_ = (double)*reinterpret_cast<float *>(message_.data);
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

} // namespace xiaxr