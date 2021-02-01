#pragma once

#include <cstddef>
#include <cstdint>

#include "utility/delay.h"

#include "bcm2835.h"
#include "bcm2835_defs.h"

namespace xiaxr {
namespace bcm2835 {
class gpio {
public:
  /*! Sets the Function Select register for the given pin, which configures
the pin as Input, Output or one of the 6 alternate functions.
\param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref RPiGPIOPin.
\param[in] mode Mode to set the pin to, one of BCM2835_GPIO_FSEL_* from \ref
bcm2835FunctionSelect
*/
  /* Function select
  // pin is a BCM2835 GPIO pin number NOT RPi pin number
  //      There are 6 control registers, each control the functions of a block
  //      of 10 pins.
  //      Each control register has 10 sets of 3 bits per GPIO pin:
  //
  //      000 = GPIO Pin X is an input
  //      001 = GPIO Pin X is an output
  //      100 = GPIO Pin X takes alternate function 0
  //      101 = GPIO Pin X takes alternate function 1
  //      110 = GPIO Pin X takes alternate function 2
  //      111 = GPIO Pin X takes alternate function 3
  //      011 = GPIO Pin X takes alternate function 4
  //      010 = GPIO Pin X takes alternate function 5
  //
  // So the 3 bits for port X are:
  //      X / 10 + ((X % 10) * 3)
  */
  static void fsel(uint8_t pin, uint8_t mode) {
    /* Function selects are 10 pins per 32 bit word, 3 bits per pin */
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPFSEL0 / 4 + (pin / 10);
    uint8_t shift = (pin % 10) * 3;
    uint32_t mask = BCM2835_GPIO_FSEL_MASK << shift;
    uint32_t value = mode << shift;
    BCM2835::bcm2835()->peri_set_bits(paddr, value, mask);
  }

  /*! Sets the specified pin output to
    HIGH.
    \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref RPiGPIOPin.
    \sa write()
  */
  static void set(uint8_t pin) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPSET0 / 4 + pin / 32;
    uint8_t shift = pin % 32;
    BCM2835::bcm2835()->peri_write(paddr, 1 << shift);
  }

  /*! Sets the specified pin output to
    LOW.
    \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref RPiGPIOPin.
    \sa write()
  */
  static void clr(uint8_t pin) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPCLR0 / 4 + pin / 32;
    uint8_t shift = pin % 32;
    BCM2835::bcm2835()->peri_write(paddr, 1 << shift);
  }

  /*! Sets any of the first 32 GPIO output pins specified in the mask to
    HIGH.
    \param[in] mask Mask of pins to affect. Use eg: (1 << RPI_GPIO_P1_03) | (1
    << RPI_GPIO_P1_05) \sa write_multi()
  */
  static void set_multi(uint32_t mask) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPSET0 / 4;
    BCM2835::bcm2835()->peri_write(paddr, mask);
  }

  static void clr_multi(uint32_t mask) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPCLR0 / 4;
    BCM2835::bcm2835()->peri_write(paddr, mask);
  }

  /*! Sets any of the first 32 GPIO output pins specified in the mask to
    LOW.
    \param[in] mask Mask of pins to affect. Use eg: (1 << RPI_GPIO_P1_03) | (1
    << RPI_GPIO_P1_05) \sa write_multi()
  */
  static void multi(uint32_t mask) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPCLR0 / 4;
    BCM2835::bcm2835()->peri_write(paddr, mask);
  }

  /*! Reads the current level on the specified
    pin and returns either HIGH or LOW. Works whether or not the pin
    is an input or an output.
    \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref RPiGPIOPin.
    \return the current level  either HIGH or LOW
  */
  static uint8_t lev(uint8_t pin) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPLEV0 / 4 + pin / 32;
    uint8_t shift = pin % 32;
    uint32_t value = BCM2835::bcm2835()->peri_read(paddr);
    return (value & (1 << shift)) ? HIGH : LOW;
  }

  /*! Event Detect Status.
    Tests whether the specified pin has detected a level or edge
    as requested by ren(), fen(), hen(),
    len(), aren(), afen().
    Clear the flag for a given pin by calling set_eds(pin);
    \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref RPiGPIOPin.
    \return HIGH if the event detect status for the given pin is true.
  */
  static uint8_t eds(uint8_t pin) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPEDS0 / 4 + pin / 32;
    uint8_t shift = pin % 32;
    uint32_t value = BCM2835::bcm2835()->peri_read(paddr);
    return (value & (1 << shift)) ? HIGH : LOW;
  }

  /*! Same as eds() but checks if any of the pins specified in
    the mask have detected a level or edge.
    \param[in] mask Mask of pins to check. Use eg: (1 << RPI_GPIO_P1_03) | (1 <<
    RPI_GPIO_P1_05) \return Mask of pins HIGH if the event detect status for the
    given pin is true.
  */
  static uint32_t eds_multi(uint32_t mask) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPEDS0 / 4;
    uint32_t value = BCM2835::bcm2835()->peri_read(paddr);
    return (value & mask);
  }

  /*! Sets the Event Detect Status register for a given pin to 1,
    which has the effect of clearing the flag. Use this afer seeing
    an Event Detect Status on the pin.
    \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref RPiGPIOPin.
  */
  static void set_eds(uint8_t pin) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPEDS0 / 4 + pin / 32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    BCM2835::bcm2835()->peri_write(paddr, value);
  }

  /*! Same as set_eds() but clears the flag for any pin which
    is set in the mask.
    \param[in] mask Mask of pins to clear. Use eg: (1 << RPI_GPIO_P1_03) | (1 <<
    RPI_GPIO_P1_05)
  */
  static void set_eds_multi(uint32_t mask) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPEDS0 / 4;
    BCM2835::bcm2835()->peri_write(paddr, mask);
  }

  /*! Enable Rising Edge Detect Enable for the specified pin.
    When a rising edge is detected, sets the appropriate pin in Event Detect
    Status. The GPRENn registers use synchronous edge detection. This means the
    input signal is sampled using the system clock and then it is looking for a
    ?011? pattern on the sampled signal. This has the effect of suppressing
    glitches. \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref
    RPiGPIOPin.
  */
  static void ren(uint8_t pin) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPREN0 / 4 + pin / 32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    BCM2835::bcm2835()->peri_set_bits(paddr, value, value);
  }

  /*! Disable Rising Edge Detect Enable for the specified pin.
    \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref RPiGPIOPin.
  */
  static void clr_ren(uint8_t pin) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPREN0 / 4 + pin / 32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    BCM2835::bcm2835()->peri_set_bits(paddr, 0, value);
  }

  /*! Enable Falling Edge Detect Enable for the specified pin.
    When a falling edge is detected, sets the appropriate pin in Event Detect
    Status. The GPRENn registers use synchronous edge detection. This means the
    input signal is sampled using the system clock and then it is looking for a
    ?100? pattern on the sampled signal. This has the effect of suppressing
    glitches. \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref
    RPiGPIOPin.
  */
  static void fen(uint8_t pin) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPFEN0 / 4 + pin / 32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    BCM2835::bcm2835()->peri_set_bits(paddr, value, value);
  }

  /*! Disable Falling Edge Detect Enable for the specified pin.
    \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref RPiGPIOPin.
  */
  static void clr_fen(uint8_t pin) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPFEN0 / 4 + pin / 32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    BCM2835::bcm2835()->peri_set_bits(paddr, 0, value);
  }

  /*! Enable High Detect Enable for the specified pin.
    When a HIGH level is detected on the pin, sets the appropriate pin in Event
    Detect Status. \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref
    RPiGPIOPin.
  */
  static void hen(uint8_t pin) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPHEN0 / 4 + pin / 32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    BCM2835::bcm2835()->peri_set_bits(paddr, value, value);
  }

  /*! Disable High Detect Enable for the specified pin.
    \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref RPiGPIOPin.
  */
  static void clr_hen(uint8_t pin) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPHEN0 / 4 + pin / 32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    BCM2835::bcm2835()->peri_set_bits(paddr, 0, value);
  }

  /*! Enable Low Detect Enable for the specified pin.
    When a LOW level is detected on the pin, sets the appropriate pin in Event
    Detect Status. \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref
    RPiGPIOPin.
  */
  static void len(uint8_t pin) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPLEN0 / 4 + pin / 32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    BCM2835::bcm2835()->peri_set_bits(paddr, value, value);
  }

  /*! Disable Low Detect Enable for the specified pin.
    \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref RPiGPIOPin.
  */
  static void clr_len(uint8_t pin) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPLEN0 / 4 + pin / 32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    BCM2835::bcm2835()->peri_set_bits(paddr, 0, value);
  }

  /*! Enable Asynchronous Rising Edge Detect Enable for the specified pin.
    When a rising edge is detected, sets the appropriate pin in Event Detect
    Status. Asynchronous means the incoming signal is not sampled by the system
    clock. As such rising edges of very short duration can be detected.
    \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref RPiGPIOPin.
  */
  static void aren(uint8_t pin) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPAREN0 / 4 + pin / 32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    BCM2835::bcm2835()->peri_set_bits(paddr, value, value);
  }

  /*! Disable Asynchronous Rising Edge Detect Enable for the specified pin.
    \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref RPiGPIOPin.
  */
  static void clr_aren(uint8_t pin) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPAREN0 / 4 + pin / 32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    BCM2835::bcm2835()->peri_set_bits(paddr, 0, value);
  }

  /*! Enable Asynchronous Falling Edge Detect Enable for the specified pin.
    When a falling edge is detected, sets the appropriate pin in Event Detect
    Status. Asynchronous means the incoming signal is not sampled by the system
    clock. As such falling edges of very short duration can be detected.
    \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref RPiGPIOPin.
  */
  static void afen(uint8_t pin) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPAFEN0 / 4 + pin / 32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    BCM2835::bcm2835()->peri_set_bits(paddr, value, value);
  }

  /*! Disable Asynchronous Falling Edge Detect Enable for the specified pin.
    \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref RPiGPIOPin.
  */
  static void clr_afen(uint8_t pin) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPAFEN0 / 4 + pin / 32;
    uint8_t shift = pin % 32;
    uint32_t value = 1 << shift;
    BCM2835::bcm2835()->peri_set_bits(paddr, 0, value);
  }

  /*! Sets the Pull-up/down register for the given pin. This is
    used with pudclk() to set the  Pull-up/down resistor for the
    given pin. However, it is usually more convenient to use
    set_pud(). \param[in] pud The desired Pull-up/down mode. One of
    BCM2835_GPIO_PUD_* from bcm2835PUDControl On the RPI 4, although this
    function and pudclk() are supported for backward compatibility,
    new code should always use set_pud(). \sa
    set_pud()
  */
  static void pud(uint8_t pud) {
    if (BCM2835::bcm2835()->pud_type_rpi4) {
      BCM2835::bcm2835()->pud_compat_setting = pud;
    } else {
      volatile uint32_t *paddr =
          BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPPUD / 4;
      BCM2835::bcm2835()->peri_write(paddr, pud);
    }
  }

  /*! Clocks the Pull-up/down value set earlier by pud() into the
    pin. \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref
    RPiGPIOPin. \param[in] on HIGH to clock the value from pud()
    into the pin. LOW to remove the clock.

    On the RPI 4, although this function and pud() are supported
    for backward compatibility, new code should always use
    set_pud().

    \sa set_pud()
  */
  static void pudclk(uint8_t pin, uint8_t on) {
    if (BCM2835::bcm2835()->pud_type_rpi4) {
      if (on)
        set_pud(pin, BCM2835::bcm2835()->pud_compat_setting);
    } else {
      volatile uint32_t *paddr =
          BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPPUDCLK0 / 4 + pin / 32;
      uint8_t shift = pin % 32;
      BCM2835::bcm2835()->peri_write(paddr, (on ? 1 : 0) << shift);
    }
  }

  /*! Reads and returns the Pad Control for the given GPIO group.
    Caution: requires root access.
    \param[in] group The GPIO pad group number, one of BCM2835_PAD_GROUP_GPIO_*
    \return Mask of bits from BCM2835_PAD_* from \ref bcm2835PadGroup
  */
  static uint32_t pad(uint8_t group) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_pads + BCM2835_PADS_GPIO_0_27 / 4 + group;
    return BCM2835::bcm2835()->peri_read(paddr);
  }

  /*! Sets the Pad Control for the given GPIO group.
    Caution: requires root access.
    \param[in] group The GPIO pad group number, one of BCM2835_PAD_GROUP_GPIO_*
    \param[in] control Mask of bits from BCM2835_PAD_* from \ref
    bcm2835PadGroup. Note that it is not necessary to include
    BCM2835_PAD_PASSWRD in the mask as this is automatically included.
  */
  static void set_pad(uint8_t group, uint32_t control) {
    volatile uint32_t *paddr =
        BCM2835::bcm2835()->bcm2835_pads + BCM2835_PADS_GPIO_0_27 / 4 + group;
    BCM2835::bcm2835()->peri_write(paddr, control | BCM2835_PAD_PASSWRD);
  }

  /*! Sets the output state of the specified pin
    \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref RPiGPIOPin.
    \param[in] on HIGH sets the output to HIGH and LOW to LOW.
  */
  static void write(uint8_t pin, uint8_t on) {
    if (on)
      set(pin);
    else
      clr(pin);
  }

  /*! Sets any of the first 32 GPIO output pins specified in the mask to the
    state given by on \param[in] mask Mask of pins to affect. Use eg: (1 <<
    RPI_GPIO_P1_03) | (1 << RPI_GPIO_P1_05) \param[in] on HIGH sets the output
    to HIGH and LOW to LOW.
  */
  static void write_multi(uint32_t mask, uint8_t on) {
    if (on)
      set_multi(mask);
    else
      clr_multi(mask);
  }

  /*! Sets the first 32 GPIO output pins specified in the mask to the value
    given by value \param[in] value values required for each bit masked in by
    mask, eg: (1 << RPI_GPIO_P1_03) | (1 << RPI_GPIO_P1_05) \param[in] mask Mask
    of pins to affect. Use eg: (1 << RPI_GPIO_P1_03) | (1 << RPI_GPIO_P1_05)
  */
  static void write_mask(uint32_t value, uint32_t mask) {
    set_multi(value & mask);
    clr_multi((~value) & mask);
  }

  /*! Sets the Pull-up/down mode for the specified pin. This is more convenient
    than clocking the mode in with pud() and pudclk().
    \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref RPiGPIOPin.
    \param[in] pud The desired Pull-up/down mode. One of BCM2835_GPIO_PUD_* from
    bcm2835PUDControl
  */
  static void set_pud(uint8_t pin, uint8_t pud_) {
    if (BCM2835::bcm2835()->pud_type_rpi4) {
      int shiftbits = (pin & 0xf) << 1;
      uint32_t bits;
      uint32_t pull;

      switch (pud_) {
      case BCM2835_GPIO_PUD_OFF:
        pull = 0;
        break;
      case BCM2835_GPIO_PUD_UP:
        pull = 1;
        break;
      case BCM2835_GPIO_PUD_DOWN:
        pull = 2;
        break;
      default:
        return;
      }

      volatile uint32_t *paddr =
          BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPPUPPDN0 / 4 + (pin >> 4);

      bits = BCM2835::bcm2835()->peri_read_nb(paddr);
      bits &= ~(3 << shiftbits);
      bits |= (pull << shiftbits);

      BCM2835::bcm2835()->peri_write_nb(paddr, bits);

    } else {
      pud(pud_);
      delayMicroseconds(10);
      pudclk(pin, 1);
      delayMicroseconds(10);
      pud(BCM2835_GPIO_PUD_OFF);
      pudclk(pin, 0);
    }
  }

  /*! On the BCM2711 based RPI 4, gets the current Pull-up/down mode for the
    specified pin. Returns one of BCM2835_GPIO_PUD_* from bcm2835PUDControl. On
    earlier RPI versions not based on the BCM2711, returns
    BCM2835_GPIO_PUD_ERROR \param[in] pin GPIO number, or one of RPI_GPIO_P1_*
    from \ref RPiGPIOPin.
  */
  static uint8_t get_pud(uint8_t pin) {
    uint8_t ret = BCM2835_GPIO_PUD_ERROR;

    if (BCM2835::bcm2835()->pud_type_rpi4) {
      uint32_t bits;
      volatile uint32_t *paddr =
          BCM2835::bcm2835()->bcm2835_gpio + BCM2835_GPPUPPDN0 / 4 + (pin >> 4);
      bits =
          (BCM2835::bcm2835()->peri_read_nb(paddr) >> ((pin & 0xf) << 1)) & 0x3;

      switch (bits) {
      case 0:
        ret = BCM2835_GPIO_PUD_OFF;
        break;
      case 1:
        ret = BCM2835_GPIO_PUD_UP;
        break;
      case 2:
        ret = BCM2835_GPIO_PUD_DOWN;
        break;
      default:
        ret = BCM2835_GPIO_PUD_ERROR;
      }
    }

    return ret;
  }
};
} // namespace bcm2835
} // namespace xiaxr