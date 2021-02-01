#pragma once

#define RF24_LINUX

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "bcm2835.h"
#include "compatibility.h"
#include "spi.h"

#define _SPI spi

#define RF24_SPI_TRANSACTIONS

// GCC a Arduino Missing
#define _BV(x) (1 << (x))
#define pgm_read_word(p) (*(p))
#define pgm_read_byte(p) (*(p))
#define pgm_read_ptr(p) (*(p))

// typedef uint16_t prog_uint16_t;
#define PSTR(x) (x)
#define printf_P printf
#define strlen_P strlen
#define PROGMEM
#define PRIPSTR "%s"

#ifdef SERIAL_DEBUG
#define IF_SERIAL_DEBUG(x) ({ x; })
#else
#define IF_SERIAL_DEBUG(x)
#endif

#define digitalWrite(pin, value) BCM2835::bcm2835()->gpio_write(pin, value)
#define pinMode(pin, value) BCM2835::bcm2835()->gpio_fsel(pin, value)
#define OUTPUT BCM2835_GPIO_FSEL_OUTP
#define INPUT BCM2835_GPIO_FSEL_INPT
