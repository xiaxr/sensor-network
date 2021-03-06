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
namespace impl {
class spi_impl {
 public:
  spi_impl(uint8_t bit_order) noexcept : bit_order_(bit_order) {}
  ~spi_impl() noexcept {}

  void begin() {
    gpio::fsel(RPI_GPIO_P1_26, BCM2835_GPIO_FSEL_ALT0); /* CE1 */
    gpio::fsel(RPI_GPIO_P1_24, BCM2835_GPIO_FSEL_ALT0); /* CE0 */
    gpio::fsel(RPI_GPIO_P1_21, BCM2835_GPIO_FSEL_ALT0); /* MISO */
    gpio::fsel(RPI_GPIO_P1_19, BCM2835_GPIO_FSEL_ALT0); /* MOSI */
    gpio::fsel(RPI_GPIO_P1_23, BCM2835_GPIO_FSEL_ALT0); /* CLK */

    /* Set the SPI CS register to the some sensible defaults */
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_spi0 + BCM2835_SPI0_CS / 4;
    BCM2835::bcm2835()->peri_write(paddr, 0); /* All 0s */

    /* Clear TX and RX fifos */
    BCM2835::bcm2835()->peri_write_nb(paddr, BCM2835_SPI0_CS_CLEAR);
  }

  void end() {
    /* Set all the SPI0 pins back to input */
    gpio::fsel(RPI_GPIO_P1_26, BCM2835_GPIO_FSEL_INPT); /* CE1 */
    gpio::fsel(RPI_GPIO_P1_24, BCM2835_GPIO_FSEL_INPT); /* CE0 */
    gpio::fsel(RPI_GPIO_P1_21, BCM2835_GPIO_FSEL_INPT); /* MISO */
    gpio::fsel(RPI_GPIO_P1_19, BCM2835_GPIO_FSEL_INPT); /* MOSI */
    gpio::fsel(RPI_GPIO_P1_23, BCM2835_GPIO_FSEL_INPT); /* CLK */
  }

  void set_clock_divider(const uint16_t divider) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_spi0 + BCM2835_SPI0_CLK / 4;
    BCM2835::bcm2835()->peri_write(paddr, divider);
  }

  void set_speed_hz(const uint32_t speed_hz) {
    uint16_t divider = (uint16_t)((uint32_t)BCM2835_CORE_CLK_HZ / speed_hz);
    divider &= 0xFFFE;
    set_clock_divider(divider);
  }

  void set_data_mode(const uint8_t mode) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_spi0 + BCM2835_SPI0_CS / 4;
    /* Mask in the CPO and CPHA bits of CS */
    BCM2835::bcm2835()->peri_set_bits(
        paddr, mode << 2, BCM2835_SPI0_CS_CPOL | BCM2835_SPI0_CS_CPHA);
  }

  void chip_select(uint8_t cs) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_spi0 + BCM2835_SPI0_CS / 4;
    /* Mask in the CS bits of CS */
    BCM2835::bcm2835()->peri_set_bits(paddr, cs, BCM2835_SPI0_CS_CS);
    std::this_thread::sleep_for(5us);
  }

  uint8_t transfer(const uint8_t value);
  void transfernb(char *tbuf, char *rbuf, uint32_t len);

 private:
  uint8_t correct_order(const uint8_t b);
  uint8_t bit_order_;
};

constexpr auto verify_cs_pin(const uint16_t cs_pin) -> uint16_t {
  if (cs_pin == 0 || cs_pin == 1) {
    return cs_pin;
  }
  if (cs_pin == 7) {
    return 1;
  }
  if (cs_pin == 18) {
    return 10;
  }
  if (cs_pin == 17) {
    return 11;
  }
  if (cs_pin == 16) {
    return 12;
  }
  return 0;
}
}  // namespace impl

class spi {
 public:
  spi(uint16_t csn_pin, uint32_t clock_speed,
      uint8_t bit_order = BCM2835_SPI_BIT_ORDER_MSBFIRST,
      uint8_t data_mode = BCM2835_SPI_MODE0)
      : impl_(bit_order),
        csn_pin_(impl::verify_cs_pin(csn_pin)),
        clock_speed_(clock_speed),
        data_mode_(data_mode) {}
  ~spi() {}

  void begin() { impl_.begin(); }
  void end() { impl_.end(); }

  void beginTransaction();
  void endTransaction();

  uint8_t transfer(uint8_t _data) { return impl_.transfer(_data); }

  template <typename T, typename R>
  void transfernb(const T &tbuf, R &rbuf) {
    if (tbuf.size() == 1) {
      impl_.transfer(tbuf[0]);
    } else {
      impl_.transfernb((char *)tbuf.data(), (char *)rbuf.data(),
                       std::min(tbuf.size(), rbuf.size()));
    }
  }

 private:
  impl::spi_impl impl_;
  uint16_t csn_pin_;
  uint32_t clock_speed_;
  uint8_t data_mode_;
};
}  // namespace bcm2835
}  // namespace xiaxr
