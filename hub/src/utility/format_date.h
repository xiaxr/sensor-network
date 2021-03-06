#pragma once

#include <string>

#include <time.h>

namespace xiaxr {
namespace{
static const std::string date_format_string =  {"%a, %d %b %Y %T UTC"};
static const std::string ts_format_string = {"%F %T"}; 
}

inline auto format_date(const timespec &ts) -> std::string {
  char buffer[100];
  strftime(buffer, sizeof(buffer), date_format_string.c_str(), gmtime(&ts.tv_sec));
  return buffer;
}

inline auto format_ts(const timespec &ts) -> std::string {
  char buffer[100];
  strftime(buffer, sizeof(buffer), ts_format_string.c_str(), gmtime(&ts.tv_sec));
  return buffer;
}

} // namespace xiaxr
