/* bcm2835.c
// C and C++ support for Broadcom BCM 2835 as used in Raspberry Pi
// http://elinux.org/RPi_Low-level_peripherals
//
http://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
//
// Author: Mike McCauley
// Copyright (C) 2011-2013 Mike McCauley
// $Id: bcm2835.c,v 1.28 2020/01/11 05:07:13 mikem Exp mikem $
*/

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <sys/capability.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <system_error>
#include <time.h>
#include <unistd.h>

#include "bcm2835.h"
#include "compatibility.h"

BCM2835 *BCM2835::instance_ = 0;

/* Map 'size' bytes starting at 'off' in file 'fd' to memory.
// Return mapped address on success, MAP_FAILED otherwise.
// On error print message.
*/
static void *mapmem(const char *msg, size_t size, int fd, off_t off) {
  void *map = mmap(NULL, size, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, off);
  if (map == MAP_FAILED) {
    throw std::system_error(errno, std::system_category(),
                            std::string("bcm2835_init: Unable to memory map ") +
                                std::string(msg));
  }
  return map;
}

void unmapmem(void **pmem, size_t size) {
  if (*pmem == MAP_FAILED)
    return;
  munmap(*pmem, size);
  *pmem = MAP_FAILED;
}

BCM2835::BCM2835()
    : bcm2835_peripherals_base(BCM2835_PERI_BASE),
      bcm2835_peripherals_size(BCM2835_PERI_SIZE), pud_type_rpi4(0),
      pud_compat_setting(BCM2835_GPIO_PUD_OFF) {}

BCM2835::~BCM2835() {
  unmapmem((void **)&bcm2835_peripherals, bcm2835_peripherals_size);
}


bool BCM2835::has_capability(cap_value_t capability) {
  bool ok = false;
  cap_t cap = cap_get_proc();
  if (cap) {
    cap_flag_value_t value;
    if (cap_get_flag(cap, capability, CAP_EFFECTIVE, &value) == 0 &&
        value == CAP_SET)
      ok = true;
    cap_free(cap);
  }
  return ok;
}

/* Function to return the pointers to the hardware register bases */
// uint32_t *bcm2835_regbase(uint8_t regbase) {
//   switch (regbase) {
//   case BCM2835_REGBASE_ST:
//     return (uint32_t *)bcm2835_st;
//   case BCM2835_REGBASE_GPIO:
//     return (uint32_t *)bcm2835_gpio;
//   case BCM2835_REGBASE_PWM:
//     return (uint32_t *)bcm2835_pwm;
//   case BCM2835_REGBASE_CLK:
//     return (uint32_t *)bcm2835_clk;
//   case BCM2835_REGBASE_PADS:
//     return (uint32_t *)bcm2835_pads;
//   case BCM2835_REGBASE_SPI0:
//     return (uint32_t *)bcm2835_spi0;
//   case BCM2835_REGBASE_BSC0:
//     return (uint32_t *)bcm2835_bsc0;
//   case BCM2835_REGBASE_BSC1:
//     return (uint32_t *)bcm2835_st;
//   case BCM2835_REGBASE_AUX:
//     return (uint32_t *)bcm2835_aux;
//   case BCM2835_REGBASE_SPI1:
//     return (uint32_t *)bcm2835_spi1;
//   }
//   return (uint32_t *)MAP_FAILED;
// }

uint32_t BCM2835::peri_read(volatile uint32_t *paddr) {
  uint32_t ret;
  __sync_synchronize();
  ret = *paddr;
  __sync_synchronize();
  return ret;
}

void BCM2835::peri_write(volatile uint32_t *paddr, uint32_t value) {
  __sync_synchronize();
  *paddr = value;
  __sync_synchronize();
}

void BCM2835::peri_set_bits(volatile uint32_t *paddr, uint32_t value,
                            uint32_t mask) {
  uint32_t v = peri_read(paddr);
  v = (v & ~mask) | (value & mask);
  peri_write(paddr, v);
}

bool BCM2835::get_base_size_peripheral(off_t &peripherals_base,
                                       size_t &peripherals_size) {
  std::fstream fs;
  fs.open(BMC2835_RPI2_DT_FILENAME, std::fstream::in | std::fstream::binary);
  if (!fs.is_open()) {
    return false;
  }

  unsigned char buf[16];

  fs.read(reinterpret_cast<char *>(buf), sizeof(buf));
  if (fs.gcount() < 8) {
    return false;
  }

  uint32_t base_address =
      (buf[4] << 24) | (buf[5] << 16) | (buf[6] << 8) | (buf[7] << 0);

  uint32_t peri_size =
      (buf[8] << 24) | (buf[9] << 16) | (buf[10] << 8) | (buf[11] << 0);

  if (!base_address) {
    /* looks like RPI 4 */
    base_address =
        (buf[8] << 24) | (buf[9] << 16) | (buf[10] << 8) | (buf[11] << 0);

    peri_size =
        (buf[12] << 24) | (buf[13] << 16) | (buf[14] << 8) | (buf[15] << 0);
  }

  /* check for valid known range formats */
  if ((buf[0] == 0x7e) && (buf[1] == 0x00) && (buf[2] == 0x00) &&
      (buf[3] == 0x00) &&
      ((base_address == BCM2835_PERI_BASE) ||
       (base_address == BCM2835_RPI2_PERI_BASE) ||
       (base_address == BCM2835_RPI4_PERI_BASE))) {
    peripherals_base = (off_t)base_address;
    peripherals_size = (size_t)peri_size;
    if (base_address == BCM2835_RPI4_PERI_BASE) {
      pud_type_rpi4 = 1;
    }
    return true;
  }

  return false;
}

void BCM2835::init() {
  if (init_) {
    return;
  }

  init_ = true;

  /* Figure out the base and size of the peripheral address block
  // using the device-tree. Required for RPi2/3/4, optional for RPi 1
  */
  if (!get_base_size_peripheral(bcm2835_peripherals_base,
                                bcm2835_peripherals_size)) {
    /* else we are prob on RPi 1 with BCM2835, and use the hardwired defaults */
    bcm2835_peripherals_base = default_bcm2835_peripherals_base;
    bcm2835_peripherals_size = default_bcm2835_peripherals_size;
  }

  /* Now get ready to map the peripherals block
   * If we are not root, try for the new /dev/gpiomem interface and accept
   * the fact that we can only access GPIO
   * else try for the /dev/mem interface and get access to everything
   */
  if (geteuid() == 0 || has_capability(CAP_SYS_RAWIO)) {
    int memfd = -1;
    /* Open the master /dev/mem device */
    if ((memfd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
      throw std::system_error(errno, std::system_category(),
                              "bcm2835_init: Unable to open /dev/mem");
    }

    /* Base of the peripherals block is mapped to VM */
    bcm2835_peripherals = reinterpret_cast<uint32_t *>(mapmem(
        "gpio", bcm2835_peripherals_size, memfd, bcm2835_peripherals_base));

    close(memfd);

    if (bcm2835_peripherals == MAP_FAILED) {
      throw std::runtime_error(
          "bcm2835_init: Error memory mapping peripherals");
    }

    /* Now compute the base addresses of various peripherals,
    // which are at fixed offsets within the mapped peripherals block
    // Caution: bcm2835_peripherals is uint32_t*, so divide offsets by 4
    */
    bcm2835_gpio = bcm2835_peripherals + BCM2835_GPIO_BASE / 4;
    bcm2835_pwm = bcm2835_peripherals + BCM2835_GPIO_PWM / 4;
    bcm2835_clk = bcm2835_peripherals + BCM2835_CLOCK_BASE / 4;
    bcm2835_pads = bcm2835_peripherals + BCM2835_GPIO_PADS / 4;
    bcm2835_spi0 = bcm2835_peripherals + BCM2835_SPI0_BASE / 4;
    bcm2835_bsc0 = bcm2835_peripherals + BCM2835_BSC0_BASE / 4; /* I2C */
    bcm2835_bsc1 = bcm2835_peripherals + BCM2835_BSC1_BASE / 4; /* I2C */
    bcm2835_st = bcm2835_peripherals + BCM2835_ST_BASE / 4;
    bcm2835_aux = bcm2835_peripherals + BCM2835_AUX_BASE / 4;
    bcm2835_spi1 = bcm2835_peripherals + BCM2835_SPI1_BASE / 4;
  } else {
    /* Not root, try /dev/gpiomem */
    /* Open the master /dev/mem device */
    int memfd = -1;
    if ((memfd = open("/dev/gpiomem", O_RDWR | O_SYNC)) < 0) {
      throw std::system_error(errno, std::system_category(),
                              "bcm2835_init: Unable to open /dev/gpiomem");
    }

    /* Base of the peripherals block is mapped to VM */
    bcm2835_peripherals_base = 0;
    bcm2835_peripherals = reinterpret_cast<uint32_t *>(mapmem(
        "gpio", bcm2835_peripherals_size, memfd, bcm2835_peripherals_base));

    close(memfd);

    if (bcm2835_peripherals == MAP_FAILED) {
      throw std::runtime_error(
          "bcm2835_init: Error memory mapping peripherals");
    }
    bcm2835_gpio = bcm2835_peripherals;
  }
}