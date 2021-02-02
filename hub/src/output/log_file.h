#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include "../config.h"
#include "../measurement.h"

#include "output_handler.h"

namespace xiaxr {
namespace output {
class LogFile : public OutputHandler {
public:
  bool begin(const Configuration &config) {

    if (config.sensor_log_file() = "") {
      throw std::runtime_error("no log file defined")
    }

    std::filesystem::path log_file = config.sensor_log_file();
    std::filesystem::path log_dir = log_file.parent_path();
    std::filesystem::create_directories(log_dir);

    log_file_ = config.sensor_log_file();

    return true;
  }

  void process(const Measurement &measurement) {
    std::fstream fs(log_file_, std::ios::ate | std::ios::out);

    // fail silently
    if (!fs.is_open()) {
      return;
    }
    
    fs << measurement.as_json();
  }

private:
  std::string log_file_;
};
} // namespace output
} // namespace xiaxr