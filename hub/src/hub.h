#pragma once

#include <array>
#include <chrono>
#include <string>

#include "RF24X/RF24.h"
#include "RF24NetworkX/RF24Network.h"

namespace xiaxr {

const static std::string settings_path("/etc/xiaxr");
const static std::string settings_file("/etc/xiaxr/hub.json");
const static std::string default_sensor_log_file("/var/log/xiaxr/data.log");

const static std::string tsdb_connection("host=localhost port=5432 dbname=xiaxr_hub user=postgres password=ts_pwd connect_timeout=10");

namespace network {
using namespace std::chrono_literals;
constexpr auto default_channel = 90;
constexpr auto default_ce_pin = 22;
constexpr auto default_csn_pin = 9;
constexpr auto data_rate = RF24_2MBPS;
constexpr auto crc_length = RF24_CRC_16;
constexpr auto address_width = 5;
static std::string gateway_name("xiaxr hub");
constexpr auto gateway_pa_level = RF24_PA_MAX;
constexpr auto default_address = 04444;
constexpr auto network_delay = 200ms;
} // namespace network

constexpr auto device_id_length = 8;
using device_id_t = std::array<uint8_t, device_id_length>;
} // namespace xiaxr