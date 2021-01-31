#pragma once

/*
 * TMRh20 2015
 * SPI layer for RF24 <-> BCM2835
 */

#include <stdio.h>

#include "RF24X/RF24_config.h"
#include "bcm2835.h"
#include "interrupt.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <thread>

#define SPI_HAS_TRANSACTION
#define MSBFIRST BCM2835_SPI_BIT_ORDER_MSBFIRST
#define SPI_MODE0 BCM2835_SPI_MODE0
//#define RF24_SPI_SPEED 10000000 //BCM2835_SPI_SPEED_4MHZ

using namespace std::chrono_literals;

class SPISettings {
public:
  SPISettings(uint32_t clock = RF24_SPI_SPEED, uint8_t bitOrder = MSBFIRST,
              uint8_t dataMode = SPI_MODE0)
      : clck{clock}, border{bitOrder}, dmode{dataMode} {}

  uint32_t clck;
  uint8_t border;
  uint8_t dmode;
};

class SPI {
public:
  SPI() {}
  ~SPI() {}

  uint8_t transfer(uint8_t _data) { return bcm2835_spi_transfer(_data); }

  template <typename T, typename R> void transfernb(const T &tbuf, R &rbuf) {
    if (tbuf.size() == 1) {
      bcm2835_spi_transfer(tbuf[0]);
    } else {
      bcm2835_spi_transfernb((char *)tbuf.data(), (char *)rbuf.data(),
                             std::min(tbuf.size(), rbuf.size()));
    }
  }

  static void begin(int busNo, uint32_t spi_speed = RF24_SPI_SPEED);

  void end() { bcm2835_spi_end(); }

  static void setBitOrder(uint8_t bit_order) {
    bcm2835_spi_setBitOrder(bit_order);
  }

  static void setDataMode(uint8_t data_mode) {
    bcm2835_spi_setDataMode(data_mode);
  }

  static void setClockDivider(uint32_t spi_speed) {
    bcm2835_spi_set_speed_hz(spi_speed);
  }

  void chipSelect(int csn_pin) {
    bcm2835_spi_chipSelect(csn_pin);
    std::this_thread::sleep_for(5us);
  }

  static void beginTransaction(SPISettings settings);

  static void endTransaction();
};
