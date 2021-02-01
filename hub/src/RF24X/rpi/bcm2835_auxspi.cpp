// TODO
// int BCM2835::bcm2835_aux_spi_begin(void) {
//   volatile uint32_t *enable = bcm2835_aux + BCM2835_AUX_ENABLE / 4;
//   volatile uint32_t *cntl0 = bcm2835_spi1 + BCM2835_AUX_SPI_CNTL0 / 4;
//   volatile uint32_t *cntl1 = bcm2835_spi1 + BCM2835_AUX_SPI_CNTL1 / 4;

//   if (bcm2835_spi1 == MAP_FAILED)
//     return 0; /* bcm2835_init() failed, or not root */

//   /* Set the SPI pins to the Alt 4 function to enable SPI1 access on them */
//   gpio_fsel(RPI_V2_GPIO_P1_36, BCM2835_GPIO_FSEL_ALT4); /* SPI1_CE2_N */
//   gpio_fsel(RPI_V2_GPIO_P1_35, BCM2835_GPIO_FSEL_ALT4); /* SPI1_MISO */
//   gpio_fsel(RPI_V2_GPIO_P1_38, BCM2835_GPIO_FSEL_ALT4); /* SPI1_MOSI */
//   gpio_fsel(RPI_V2_GPIO_P1_40, BCM2835_GPIO_FSEL_ALT4); /* SPI1_SCLK */

//   bcm2835_aux_spi_setClockDivider(
//       bcm2835_aux_spi_CalcClockDivider(1000000)); // Default 1MHz SPI

//   peri_write(enable, BCM2835_AUX_ENABLE_SPI0);
//   peri_write(cntl1, 0);
//   peri_write(cntl0, BCM2835_AUX_SPI_CNTL0_CLEARFIFO);

//   return 1; /* OK */
// }

// void BCM2835::bcm2835_aux_spi_end(void) {
//   /* Set all the SPI1 pins back to input */
//   gpio_fsel(RPI_V2_GPIO_P1_36, BCM2835_GPIO_FSEL_INPT); /* SPI1_CE2_N */
//   gpio_fsel(RPI_V2_GPIO_P1_35, BCM2835_GPIO_FSEL_INPT); /* SPI1_MISO */
//   gpio_fsel(RPI_V2_GPIO_P1_38, BCM2835_GPIO_FSEL_INPT); /* SPI1_MOSI */
//   gpio_fsel(RPI_V2_GPIO_P1_40, BCM2835_GPIO_FSEL_INPT); /* SPI1_SCLK */
// }

// #define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))

// uint16_t BCM2835::bcm2835_aux_spi_CalcClockDivider(uint32_t speed_hz) {
//   uint16_t divider;

//   if (speed_hz < (uint32_t)BCM2835_AUX_SPI_CLOCK_MIN) {
//     speed_hz = (uint32_t)BCM2835_AUX_SPI_CLOCK_MIN;
//   } else if (speed_hz > (uint32_t)BCM2835_AUX_SPI_CLOCK_MAX) {
//     speed_hz = (uint32_t)BCM2835_AUX_SPI_CLOCK_MAX;
//   }

//   divider = (uint16_t)DIV_ROUND_UP(BCM2835_CORE_CLK_HZ, 2 * speed_hz) - 1;

//   if (divider > (uint16_t)BCM2835_AUX_SPI_CNTL0_SPEED_MAX) {
//     return (uint16_t)BCM2835_AUX_SPI_CNTL0_SPEED_MAX;
//   }

//   return divider;
// }

// void BCM2835::bcm2835_aux_spi_write(uint16_t data) {
//   volatile uint32_t *cntl0 = bcm2835_spi1 + BCM2835_AUX_SPI_CNTL0 / 4;
//   volatile uint32_t *cntl1 = bcm2835_spi1 + BCM2835_AUX_SPI_CNTL1 / 4;
//   volatile uint32_t *stat = bcm2835_spi1 + BCM2835_AUX_SPI_STAT / 4;
//   volatile uint32_t *io = bcm2835_spi1 + BCM2835_AUX_SPI_IO / 4;

//   uint32_t _cntl0 = (spi1_speed << BCM2835_AUX_SPI_CNTL0_SPEED_SHIFT);
//   _cntl0 |= BCM2835_AUX_SPI_CNTL0_CS2_N;
//   _cntl0 |= BCM2835_AUX_SPI_CNTL0_ENABLE;
//   _cntl0 |= BCM2835_AUX_SPI_CNTL0_MSBF_OUT;
//   _cntl0 |= 16; // Shift length

//   peri_write(cntl0, _cntl0);
//   peri_write(cntl1, BCM2835_AUX_SPI_CNTL1_MSBF_IN);

//   while (peri_read(stat) & BCM2835_AUX_SPI_STAT_TX_FULL) {
//   };

//   peri_write(io, (uint32_t)data << 16);
// }

// void BCM2835::bcm2835_aux_spi_writenb(const char *tbuf, uint32_t len) {
//   volatile uint32_t *cntl0 = bcm2835_spi1 + BCM2835_AUX_SPI_CNTL0 / 4;
//   volatile uint32_t *cntl1 = bcm2835_spi1 + BCM2835_AUX_SPI_CNTL1 / 4;
//   volatile uint32_t *stat = bcm2835_spi1 + BCM2835_AUX_SPI_STAT / 4;
//   volatile uint32_t *txhold = bcm2835_spi1 + BCM2835_AUX_SPI_TXHOLD / 4;
//   volatile uint32_t *io = bcm2835_spi1 + BCM2835_AUX_SPI_IO / 4;

//   char *tx = (char *)tbuf;
//   uint32_t tx_len = len;
//   uint32_t count;
//   uint32_t data;
//   uint32_t i;
//   uint8_t byte;

//   uint32_t _cntl0 = (spi1_speed << BCM2835_AUX_SPI_CNTL0_SPEED_SHIFT);
//   _cntl0 |= BCM2835_AUX_SPI_CNTL0_CS2_N;
//   _cntl0 |= BCM2835_AUX_SPI_CNTL0_ENABLE;
//   _cntl0 |= BCM2835_AUX_SPI_CNTL0_MSBF_OUT;
//   _cntl0 |= BCM2835_AUX_SPI_CNTL0_VAR_WIDTH;

//   peri_write(cntl0, _cntl0);
//   peri_write(cntl1, BCM2835_AUX_SPI_CNTL1_MSBF_IN);

//   while (tx_len > 0) {

//     while (peri_read(stat) & BCM2835_AUX_SPI_STAT_TX_FULL)
//       ;

//     count = MIN(tx_len, 3);
//     data = 0;

//     for (i = 0; i < count; i++) {
//       byte = (tx != NULL) ? (uint8_t)*tx++ : (uint8_t)0;
//       data |= byte << (8 * (2 - i));
//     }

//     data |= (count * 8) << 24;
//     tx_len -= count;

//     if (tx_len != 0) {
//       peri_write(txhold, data);
//     } else {
//       peri_write(io, data);
//     }

//     while (peri_read(stat) & BCM2835_AUX_SPI_STAT_BUSY)
//       ;

//     (void)peri_read(io);
//   }
// }

// void BCM2835::bcm2835_aux_spi_transfernb(const char *tbuf, char *rbuf,
//                                          uint32_t len) {
//   volatile uint32_t *cntl0 = bcm2835_spi1 + BCM2835_AUX_SPI_CNTL0 / 4;
//   volatile uint32_t *cntl1 = bcm2835_spi1 + BCM2835_AUX_SPI_CNTL1 / 4;
//   volatile uint32_t *stat = bcm2835_spi1 + BCM2835_AUX_SPI_STAT / 4;
//   volatile uint32_t *txhold = bcm2835_spi1 + BCM2835_AUX_SPI_TXHOLD / 4;
//   volatile uint32_t *io = bcm2835_spi1 + BCM2835_AUX_SPI_IO / 4;

//   char *tx = (char *)tbuf;
//   char *rx = (char *)rbuf;
//   uint32_t tx_len = len;
//   uint32_t rx_len = len;
//   uint32_t count;
//   uint32_t data;
//   uint32_t i;
//   uint8_t byte;

//   uint32_t _cntl0 = (spi1_speed << BCM2835_AUX_SPI_CNTL0_SPEED_SHIFT);
//   _cntl0 |= BCM2835_AUX_SPI_CNTL0_CS2_N;
//   _cntl0 |= BCM2835_AUX_SPI_CNTL0_ENABLE;
//   _cntl0 |= BCM2835_AUX_SPI_CNTL0_MSBF_OUT;
//   _cntl0 |= BCM2835_AUX_SPI_CNTL0_VAR_WIDTH;

//   peri_write(cntl0, _cntl0);
//   peri_write(cntl1, BCM2835_AUX_SPI_CNTL1_MSBF_IN);

//   while ((tx_len > 0) || (rx_len > 0)) {

//     while (!(peri_read(stat) & BCM2835_AUX_SPI_STAT_TX_FULL) && (tx_len > 0)) {
//       count = MIN(tx_len, 3);
//       data = 0;

//       for (i = 0; i < count; i++) {
//         byte = (tx != NULL) ? (uint8_t)*tx++ : (uint8_t)0;
//         data |= byte << (8 * (2 - i));
//       }

//       data |= (count * 8) << 24;
//       tx_len -= count;

//       if (tx_len != 0) {
//         peri_write(txhold, data);
//       } else {
//         peri_write(io, data);
//       }
//     }

//     while (!(peri_read(stat) & BCM2835_AUX_SPI_STAT_RX_EMPTY) && (rx_len > 0)) {
//       count = MIN(rx_len, 3);
//       data = peri_read(io);

//       if (rbuf != NULL) {
//         switch (count) {
//         case 3:
//           *rx++ = (char)((data >> 16) & 0xFF);
//           /*@fallthrough@*/
//           /* no break */
//         case 2:
//           *rx++ = (char)((data >> 8) & 0xFF);
//           /*@fallthrough@*/
//           /* no break */
//         case 1:
//           *rx++ = (char)((data >> 0) & 0xFF);
//         }
//       }

//       rx_len -= count;
//     }

//     while (!(peri_read(stat) & BCM2835_AUX_SPI_STAT_BUSY) && (rx_len > 0)) {
//       count = MIN(rx_len, 3);
//       data = peri_read(io);

//       if (rbuf != NULL) {
//         switch (count) {
//         case 3:
//           *rx++ = (char)((data >> 16) & 0xFF);
//           /*@fallthrough@*/
//           /* no break */
//         case 2:
//           *rx++ = (char)((data >> 8) & 0xFF);
//           /*@fallthrough@*/
//           /* no break */
//         case 1:
//           *rx++ = (char)((data >> 0) & 0xFF);
//         }
//       }

//       rx_len -= count;
//     }
//   }
// }

// /* Writes (and reads) a single byte to AUX SPI */
// uint8_t BCM2835::bcm2835_aux_spi_transfer(uint8_t value) {
//   volatile uint32_t *cntl0 = bcm2835_spi1 + BCM2835_AUX_SPI_CNTL0 / 4;
//   volatile uint32_t *cntl1 = bcm2835_spi1 + BCM2835_AUX_SPI_CNTL1 / 4;
//   volatile uint32_t *stat = bcm2835_spi1 + BCM2835_AUX_SPI_STAT / 4;
//   volatile uint32_t *io = bcm2835_spi1 + BCM2835_AUX_SPI_IO / 4;

//   uint32_t data;

//   uint32_t _cntl0 = (spi1_speed << BCM2835_AUX_SPI_CNTL0_SPEED_SHIFT);
//   _cntl0 |= BCM2835_AUX_SPI_CNTL0_CS2_N;
//   _cntl0 |= BCM2835_AUX_SPI_CNTL0_ENABLE;
//   _cntl0 |= BCM2835_AUX_SPI_CNTL0_MSBF_OUT;
//   _cntl0 |= BCM2835_AUX_SPI_CNTL0_CPHA_IN;
//   _cntl0 |= 8; // Shift length.

//   uint32_t _cntl1 = BCM2835_AUX_SPI_CNTL1_MSBF_IN;

//   peri_write(cntl1, _cntl1);
//   peri_write(cntl0, _cntl0);

//   peri_write(io, (uint32_t)correct_order(value) << 24);

//   while (peri_read(stat) & BCM2835_AUX_SPI_STAT_BUSY)
//     ;

//   data = correct_order(peri_read(io) & 0xff);

//   bcm2835_aux_spi_reset();

//   return data;
// }

//   void bcm2835_aux_spi_reset() {
//     volatile uint32_t *cntl0 = bcm2835_spi1 + BCM2835_AUX_SPI_CNTL0 / 4;
//     volatile uint32_t *cntl1 = bcm2835_spi1 + BCM2835_AUX_SPI_CNTL1 / 4;

//     peri_write(cntl1, 0);
//     peri_write(cntl0, BCM2835_AUX_SPI_CNTL0_CLEARFIFO);
//   }

