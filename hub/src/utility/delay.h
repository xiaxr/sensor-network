#pragma once

#include <chrono>
#include <cstdint>
#include <thread>

uint32_t millis();

inline void delay(uint32_t millis) {
  std::this_thread::sleep_for(std::chrono::milliseconds{millis});
}

inline void delayMicroseconds(uint64_t micros){
  std::this_thread::sleep_for(std::chrono::microseconds{micros});
}
