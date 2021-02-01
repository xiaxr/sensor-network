/* bcm2835.h

   C and C++ support for Broadcom BCM 2835 as used in Raspberry Pi

   Author: Mike McCauley
   Copyright (C) 2011-2013 Mike McCauley
   $Id: bcm2835.h,v 1.26 2020/01/11 05:07:13 mikem Exp mikem $
*/

#pragma once

#include "bcm2835_defs.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <sys/capability.h>
#include <thread>

class BCM2835 {
  const static off_t default_bcm2835_peripherals_base = BCM2835_PERI_BASE;
  const static size_t default_bcm2835_peripherals_size = BCM2835_PERI_SIZE;

public:
  static BCM2835 *bcm2835() {
    if (!instance_) {
      instance_ = new BCM2835;
      instance_->init();
    }
    return instance_;
  }

  /*! Reads 32 bit value from a peripheral address WITH a memory barrier before
    and after each read. This is safe, but slow.  The MB before protects this
    read from any in-flight reads that didn't use a MB.  The MB after protects
    subsequent reads from another peripheral.

    \param[in] paddr Physical address to read from. See BCM2835_GPIO_BASE etc.
    \return the value read from the 32 bit register
    \sa Physical Addresses
  */
  uint32_t peri_read(volatile uint32_t *paddr);

  /*! Reads 32 bit value from a peripheral address WITHOUT the read barriers
    You should only use this when:
    o your code has previously called peri_read() for a register
    within the same peripheral, and no read or write to another peripheral has
    occurred since. o your code has called bcm2835_memory_barrier() since the
    last access to ANOTHER peripheral.

    \param[in] paddr Physical address to read from. See BCM2835_GPIO_BASE etc.
    \return the value read from the 32 bit register
    \sa Physical Addresses
  */
  uint32_t peri_read_nb(volatile uint32_t *paddr) { return *paddr; }

  /*! Writes 32 bit value from a peripheral address WITH a memory barrier before
  and after each write This is safe, but slow.  The MB before ensures that any
  in-flight write to another peripheral completes before this write is issued.
  The MB after ensures that subsequent reads and writes to another peripheral
  will see the effect of this write.

  This is a tricky optimization; if you aren't sure, use the barrier version.

  \param[in] paddr Physical address to read from. See BCM2835_GPIO_BASE etc.
  \param[in] value The 32 bit value to write
  \sa Physical Addresses
*/
  void peri_write(volatile uint32_t *paddr, uint32_t value);

  /*! Writes 32 bit value from a peripheral address without the write barrier
    You should only use this when:
    o your code has previously called peri_write() for a register
    within the same peripheral, and no other peripheral access has occurred
    since. o your code has called bcm2835_memory_barrier() since the last access
    to ANOTHER peripheral.

    This is a tricky optimization; if you aren't sure, use the barrier version.

    \param[in] paddr Physical address to read from. See BCM2835_GPIO_BASE etc.
    \param[in] value The 32 bit value to write
    \sa Physical Addresses
  */
  void peri_write_nb(volatile uint32_t *paddr, uint32_t value) {
    *paddr = value;
  }

  /*! Alters a number of bits in a 32 peripheral regsiter.
  It reads the current valu and then alters the bits defines as 1 in mask,
  according to the bit value in value.
  All other bits that are 0 in the mask are unaffected.
  Use this to alter a subset of the bits in a register.
  Memory barriers are used.  Note that this is not atomic; an interrupt
  routine can cause unexpected results.
  \param[in] paddr Physical address to read from. See BCM2835_GPIO_BASE etc.
  \param[in] value The 32 bit value to write, masked in by mask.
  \param[in] mask Bitmask that defines the bits that will be altered in the
  register. \sa Physical Addresses
*/
  void peri_set_bits(volatile uint32_t *paddr, uint32_t value, uint32_t mask);

  volatile uint32_t *bcm2835_gpio;
  volatile uint32_t *bcm2835_pads;
  volatile uint32_t *bcm2835_pwm;
  volatile uint32_t *bcm2835_clk;
  volatile uint32_t *bcm2835_spi0;
  volatile uint32_t *bcm2835_bsc0;
  volatile uint32_t *bcm2835_bsc1;
  volatile uint32_t *bcm2835_st;
  volatile uint32_t *bcm2835_aux;
  volatile uint32_t *bcm2835_spi1;

  /* RPI 4 has different pullup registers - we need to know if we have that type
   */
  uint8_t pud_type_rpi4;

  /* RPI 4 has different pullup operation - make backwards compat */
  uint8_t pud_compat_setting;

private:
  BCM2835();
  ~BCM2835();

  BCM2835(BCM2835 const &) = delete;
  BCM2835 &operator=(BCM2835 const &) = delete;

  void init();

  bool has_capability(cap_value_t capability);

  bool get_base_size_peripheral(off_t &peripherals_base,
                                size_t &peripherals_size);
  bool init_ = false;

  off_t bcm2835_peripherals_base;
  size_t bcm2835_peripherals_size;

  /* Virtual memory address of the mapped peripherals block
   */
  uint32_t *bcm2835_peripherals;

  /* And the register bases within the peripherals block
   */

  static BCM2835 *instance_;
};
