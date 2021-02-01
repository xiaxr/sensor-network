#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <thread>

#include "RF24X/RF24_config.h"

#include "interrupt.h"

#include "bcm2835_defs.h"

#include "bcm2835.h"
#include "gpio.h"

using namespace std::chrono_literals;

namespace xiaxr {
namespace bcm2835 {
class spi {
public:
  spi(nt32_t clock_speed = RF24_SPI_SPEED,
      uint8_t bit_order = BCM2835_SPI_BIT_ORDER_MSBFIRST,
      uint8_t data_mode = BCM2835_SPI_MODE0)
      : clock_speed_(clock_speed), bit_order_(bit_order),
        data_mode_(data_mode) {}
  bool begin();
  void end();

  uint8_t transfer(uint8_t _data) {
    return BCM2835::bcm2835()->bcm2835_spi_transfer(_data);
  }

  template <typename T, typename R> void transfernb(const T &tbuf, R &rbuf) {
    if (tbuf.size() == 1) {
      BCM2835::bcm2835()->bcm2835_spi_transfer(tbuf[0]);
    } else {
      BCM2835::bcm2835()->bcm2835_spi_transfernb(
          (char *)tbuf.data(), (char *)rbuf.data(),
          std::min(tbuf.size(), rbuf.size()));
    }
  }

  void chipSelect(int csn_pin) {
    BCM2835::bcm2835()->bcm2835_spi_chipSelect(csn_pin);
    std::this_thread::sleep_for(5us);
  }

  static void beginTransaction(SPISettings settings);

  static void endTransaction();

  void setBitOrder(uint8_t order) { bcm2835_spi_bit_order = order; }

  /*! Sets the SPI clock divider and therefore the
    SPI clock speed.
    \param[in] divider The desired SPI clock divider, one of
    BCM2835_SPI_CLOCK_DIVIDER_*, see \ref bcm2835SPIClockDivider
  */
  void setClockDivider(uint16_t divider) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_spi0 + BCM2835_SPI0_CLK / 4;
    BCM2835::bcm2835()->peri_write(paddr, divider);
  }

  /*! Sets the SPI clock divider by converting the speed parameter to
    the equivalent SPI clock divider. ( see \sa bcm2835_spi_setClockDivider)
    \param[in] speed_hz The desired SPI clock speed in Hz
  */
  void set_speed_hz(uint32_t speed_hz) {
    uint16_t divider = (uint16_t)((uint32_t)BCM2835_CORE_CLK_HZ / speed_hz);
    divider &= 0xFFFE;
    setClockDivider(divider);
  }

  /*! Sets the SPI data mode
    Sets the clock polariy and phase
    \param[in] mode The desired data mode, one of BCM2835_SPI_MODE*,
    see \ref bcm2835SPIMode
  */
  void setDataMode(uint8_t mode) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_spi0 + BCM2835_SPI0_CS / 4;
    /* Mask in the CPO and CPHA bits of CS */
    BCM2835::bcm2835()->peri_set_bits(
        paddr, mode << 2, BCM2835_SPI0_CS_CPOL | BCM2835_SPI0_CS_CPHA);
  }

  /*! Sets the chip select pin(s)
    When an bcm2835_spi_transfer() is made, the selected pin(s) will be
    asserted during the transfer. \param[in] cs Specifies the CS pins(s) that
    are used to activate the desired slave. One of BCM2835_SPI_CS*, see \ref
    bcm2835SPIChipSelect
  */
  void chipSelect(uint8_t cs) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_spi0 + BCM2835_SPI0_CS / 4;
    /* Mask in the CS bits of CS */
    BCM2835::bcm2835()->peri_set_bits(paddr, cs, BCM2835_SPI0_CS_CS);
  }

  void beginTransaction();
  void endTransaction();

private:
  uint8_t transfer_(const uint8_t value);
  void transfernb_(char *tbuf, char *rbuf, uint32_t len);

  uint8_t correct_order(const uint8_t b);
  /* SPI bit order. BCM2835 SPI0 only supports MSBFIRST, so we instead
   * have a software based bit reversal, based on a contribution by Damiano
   * Benedetti
   */
  uint32_t clock_speed_;
  uint8_t bit_order_;
  uint8_t data_mode_;
};
} // namespace bcm2835
} // namespace xiaxr
