#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <Arduino.h>

#include <RF24.h>
#include <RF24Network.h>

#include "buffer.h"

#define NETWORK_ADDRESS_WIDTH 5
#define NETWORK_CRC_LENGTH RF24_CRC_16
#define NETWORK_DATA_RATE RF24_2MBPS

namespace xiaxr {
constexpr auto default_device_network_address = 04444;

namespace eeprom {
constexpr auto device_id_offset = 0;
constexpr auto device_id_length = 8;

constexpr auto device_name_length_offset =
    (device_id_offset + device_id_length);
constexpr auto device_name_length_length = 1;

constexpr auto device_name_offset =
    (device_name_length_offset + device_name_length_length);
constexpr auto device_name_length = 32;
} // namespace eeprom

constexpr auto measurement_message_id = 0x10;

constexpr auto measurement_type_none = 0x00;
constexpr auto measurement_type_temperature = 0x01;
constexpr auto measurement_type_humidity = 0x02;
constexpr auto measurement_type_pressure = 0x03;
constexpr auto measurement_type_altitude = 0x04;

constexpr auto measurement_unit_none = 0x00;
constexpr auto measurement_unit_celsius = 0x01;
constexpr auto measurement_unit_relative_humidity = 0x10;
constexpr auto measurement_unit_Pa = 0x20;
constexpr auto measurement_unit_m = 0x30;

using device_id_t = buffer<eeprom::device_id_length>;

class device {
public:
  device() {}

  auto device_id() -> device_id_t;
  auto device_name() -> ::String;

  auto configure_radio(RF24 &radio) -> void {
    radio.setAddressWidth(NETWORK_ADDRESS_WIDTH);
    radio.setCRCLength(NETWORK_CRC_LENGTH);
    radio.setDataRate(NETWORK_DATA_RATE);
  }

private:
};

namespace {
template <typename T> auto get_type_flag() -> uint8_t { return 0; }
template <> auto get_type_flag<float>() -> uint8_t { return 1; }
template <> auto get_type_flag<double>() -> uint8_t { return 1; }
} // namespace

template <typename T> class measurement_t {
  using data_t = T;

public:
  measurement_t(device &device, uint8_t measurement_id,
                uint8_t measurement_type, uint8_t measurement_unit) {
    payload.copy(device.device_id().bytes(), id_offset, id_length);
    payload[message_type_offset] = measurement_message_id;
    payload[message_length_offset] = payload_length;
    payload.copy(&measurement_id, measurement_id_offset, sizeof(uint8_t));
    payload.copy(&measurement_type, measurement_type_offset, sizeof(uint8_t));
    payload.copy(&measurement_unit, measurement_unit_offset, sizeof(uint8_t));
    payload[measurement_length_offset] =
        (measurement_length << 4) | get_type_flag<data_t>();
  }

  auto send(RF24Network &network, data_t data) -> bool {
    payload.copy(&data, data_offset, sizeof(data_t));
    RF24NetworkHeader header(0, EXTERNAL_DATA_TYPE);
    return network.write(header, payload.bytes(), payload_length);
  }

private:
  static const auto id_offset = 0;
  static const auto id_length = eeprom::device_id_length;
  static const auto message_type_offset =
      (id_offset + eeprom::device_id_length);
  static const auto message_length_offset =
      (message_type_offset + sizeof(uint8_t));
  static const auto measurement_id_offset =
      (message_length_offset + sizeof(uint8_t));
  static const auto measurement_type_offset =
      (measurement_id_offset + sizeof(uint8_t));
  static const auto measurement_unit_offset =
      (measurement_type_offset + sizeof(uint8_t));
  static const auto measurement_length = sizeof(data_t);
  static const auto measurement_length_offset =
      (measurement_unit_offset + sizeof(uint8_t));
  static const auto data_offset = (measurement_length_offset + sizeof(uint8_t));
  static const auto data_length = 8;
  static const auto payload_length =
      (id_length + (sizeof(uint8_t) * 6) + data_length);
  buffer<payload_length> payload;
};

} // namespace xiaxr