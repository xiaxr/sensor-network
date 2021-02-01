#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <thread>

#include "RF24NetworkX/RF24Network.h"
#include "RF24X/RF24.h"

#include "config.h"
#include "gateway.h"
#include "hub.h"

namespace xiaxr {
using namespace std::chrono_literals;
Gateway::Gateway(const Configuration &config)
    : radio_(config.ce_pin(), config.csn_pin()), network_(radio_),
      device_(config.device_id(), network::gateway_name),
      network_channel_(config.network_channel()) {}

auto Gateway::update_device_id(const device_id_t &device_id) -> void {
  auto config = Configuration::load();
  device_.device_id() = device_id;
  config.device_id() = device_id;
  config.save();
}

auto Gateway::update_network_channel(const uint8_t network_channel) -> void {
  radio_.stopListening();
  radio_.setChannel(network_channel);
  std::this_thread::sleep_for(200ms);
  radio_.startListening();

  auto config = Configuration::load();
  network_channel_ = network_channel;
  config.network_channel() = network_channel;
  config.save();
}

auto Gateway::begin() -> void {
  if (!radio_.begin()) {
    throw std::runtime_error("unable to start radio");
  }

  radio_.setAddressWidth(network::address_width);
  radio_.setCRCLength(network::crc_length);
  radio_.setDataRate(network::data_rate);
  std::this_thread::sleep_for(100ms);
  network_.begin(network_channel_, 0);
}

auto Gateway::queue_incoming_messages() -> void {
  while (network_.update()) {
  }

  RF24NetworkFrame f;
  while (!network_.external_queue.empty()) {
    f = network_.external_queue.front();
    if (f.message_size > 0) {
      rx_queue_.push(make_network_message(f.message_buffer, f.message_size));
    }
    network_.external_queue.pop();
  }

  // Clean out frames
  while (!network_.frame_queue.empty()) {
    network_.frame_queue.pop();
  }

  // debug to test for memory leak
  if (network_.frameFragmentsCache.size() > 0) {
    std::cout << network_.frameFragmentsCache.size() << std::endl;
  }
}

auto Gateway::sleep() -> void {
  std::this_thread::sleep_for(network::network_delay);
}

auto Gateway::power_up() -> void { radio_.powerUp(); }
auto Gateway::power_down() -> void { radio_.powerDown(); }

} // namespace xiaxr

// #include <iostream>
// #include <iomanip>

//   std::cout << std::hex << std::setfill('0');
//   for (int i = 0; i < buffer_size; ++i) {
//     std::cout << std::setw(2) << std::uppercase
//               << static_cast<unsigned int>(buffer[i]);
//   }

// 6E1E67D7 D22C4A010000000000000000101600000040