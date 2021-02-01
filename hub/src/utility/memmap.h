#pragma once

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

#include <stddef>
#include <stdint>
#include <string>

namespace xiaxr {
auto map_file(const std::string &path, const size_t offset, const size_t length)
    -> boost::iostreams::mapped_file {
  return boost::iostreams::mapped_file(
      path, boost::iostreams::mapped_file::mapmode::readwrite, length, offset);
}
} // namespace xiaxr