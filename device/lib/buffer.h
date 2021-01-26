#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace xiaxr {
template <size_t LENGTH> class buffer {
public:
  buffer() {}
  buffer(const buffer &lval) { *this = lval; }
  buffer(buffer &&rval) { move(rval); }

  buffer &operator=(const buffer &rhs) {
    if (this == &rhs)
      return *this;
    memcpy(data, rhs.data, LENGTH);
    return *this;
  }

  buffer &operator=(buffer &&rval) {
    if (this != &rval)
      move(rval);
    return *this;
  }

  uint8_t operator[](unsigned int index) const {
    if (index >= LENGTH)
      return 0;
    return data[index];
  }

  uint8_t &operator[](unsigned int index) {
    static uint8_t dummy;
    if (index >= LENGTH) {
      dummy = 0;
      return dummy;
    }
    return data[index];
  }

  uint8_t *begin() { return data; }
  uint8_t *end() { return data + LENGTH; }
  const uint8_t *begin() const { return data; }
  const uint8_t *end() const { return data + LENGTH; }
  auto length() -> const size_t { return LENGTH; }

  const void *bytes() { return data; }

  void copy(const void *values, size_t offset, size_t length) {
    if ((offset + length) >= LENGTH)
      return;
    memcpy(data + offset, values, length);
  }

private:
  void move(buffer &rhs) { memcpy(data, rhs.data, LENGTH); }
  uint8_t data[LENGTH];
};
} // namespace xiaxr