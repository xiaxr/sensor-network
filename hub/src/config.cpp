
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <filesystem>
#include <string>
#include <tuple>

#include "config.h"
#include "hub.h"
#include "utility/hexify.h"

namespace xiaxr {
namespace {
namespace pt = boost::property_tree;
static auto parse_device_id(const std::string &value) -> device_id_t {
  if (value.empty()) {
    return device_id_t{0};
  }
  return unhexify<device_id_length>(value);
}

static auto sprint_device_id(const device_id_t &device_id) -> std::string {
  return hexify(device_id);
}

using config_tuple_t = std::tuple<device_id_t, uint8_t, uint8_t, uint8_t, std::string>;
static auto load_config() -> config_tuple_t {
  try {
    pt::ptree root;
    pt::read_json(settings_file, root);
    auto gateway_device_id =
        parse_device_id(root.get<std::string>("gateway_device_id"));

    auto network_channel =
        root.get<int>("network_channel", network::default_channel);

    auto ce_pin = root.get<int>("ce_pin", network::default_ce_pin);
    auto csn_pin = root.get<int>("csn_pin", network::default_csn_pin);

    auto sensor_log_file = root.get<std::string>("sensor_log_file", "");

    return {gateway_device_id, network_channel, ce_pin, csn_pin,
            sensor_log_file};
  } catch (...) {
    return {device_id_t{}, network::default_channel, network::default_ce_pin,
            network::default_csn_pin, ""};
  }
}

void create_settings_directory() {
  std::filesystem::create_directories(settings_path);
}

} // namespace

void Configuration::save() {
  create_settings_directory();

  try {
    pt::ptree root;
    root.put("gateway_device_id", sprint_device_id(device_id_));
    root.put("network_channel", network_channel_);
    root.put("ce_pin", ce_pin_);
    root.put("csn_pin", csn_pin_);
    root.put("sensor_log_file", sensor_log_file_);
    pt::write_json(settings_file, root);
  } catch (...) {
  }
}

void Configuration::reload() {
  auto values = load_config();
  device_id_ = std::get<0>(values);
  network_channel_ = std::get<1>(values);
  ce_pin_ = std::get<2>(values);
  csn_pin_ = std::get<3>(values);
  sensor_log_file_ = std::get<4>(values);
}

Configuration Configuration::load() {
  auto values = load_config();
  return Configuration(std::get<0>(values), std::get<1>(values),
                       std::get<2>(values), std::get<3>(values),
                       std::get<4>(values));
}
} // namespace xiaxr