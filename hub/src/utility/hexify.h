#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace xiaxr {

template <auto input_length>
std::string hexify(const std::array<uint8_t, input_length> &in) {
  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (auto v : in) {
    ss << std::setw(2) << std::uppercase << static_cast<unsigned int>(v);
  }

  return ss.str();
}

template <size_t max_length>
std::array<uint8_t, max_length> unhexify(const std::string &in) {
  std::array<uint8_t, max_length> output{};

  if ((in.length() % 2) != 0) {
    throw std::runtime_error("String is not valid length ...");
  }

  size_t cnt = std::min(in.length() / 2, max_length);

  for (size_t i = 0; cnt > i; ++i) {
    uint32_t s = 0;
    std::stringstream ss;
    ss << std::hex << in.substr(i * 2, 2);
    ss >> s;

    output[i] = static_cast<uint8_t>(s);
  }

  return output;
}

} // namespace xiaxr