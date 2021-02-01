// TODO
// int BCM2835::bcm2835_i2c_begin(void) {
//   uint16_t cdiv;

//   if (bcm2835_bsc0 == MAP_FAILED || bcm2835_bsc1 == MAP_FAILED)
//     return 0; /* bcm2835_init() failed, or not root */

// #ifdef I2C_V1
//   volatile uint32_t *paddr = bcm2835_bsc0 + BCM2835_BSC_DIV / 4;
//   /* Set the I2C/BSC0 pins to the Alt 0 function to enable I2C access on them */
//   gpio_fsel(RPI_GPIO_P1_03, BCM2835_GPIO_FSEL_ALT0); /* SDA */
//   gpio_fsel(RPI_GPIO_P1_05, BCM2835_GPIO_FSEL_ALT0); /* SCL */
// #else
//   volatile uint32_t *paddr = bcm2835_bsc1 + BCM2835_BSC_DIV / 4;
//   /* Set the I2C/BSC1 pins to the Alt 0 function to enable I2C access on them */
//   gpio_fsel(RPI_V2_GPIO_P1_03, BCM2835_GPIO_FSEL_ALT0); /* SDA */
//   gpio_fsel(RPI_V2_GPIO_P1_05, BCM2835_GPIO_FSEL_ALT0); /* SCL */
// #endif

//   /* Read the clock divider register */
//   cdiv = peri_read(paddr);
//   /* Calculate time for transmitting one byte
//   // 1000000 = micros seconds in a second
//   // 9 = Clocks per byte : 8 bits + ACK
//   */
//   i2c_byte_wait_us = ((float)cdiv / BCM2835_CORE_CLK_HZ) * 1000000 * 9;

//   return 1;
// }

// void BCM2835::bcm2835_i2c_end(void) {
// #ifdef I2C_V1
//   /* Set all the I2C/BSC0 pins back to input */
//   gpio_fsel(RPI_GPIO_P1_03, BCM2835_GPIO_FSEL_INPT); /* SDA */
//   gpio_fsel(RPI_GPIO_P1_05, BCM2835_GPIO_FSEL_INPT); /* SCL */
// #else
//   /* Set all the I2C/BSC1 pins back to input */
//   gpio_fsel(RPI_V2_GPIO_P1_03, BCM2835_GPIO_FSEL_INPT); /* SDA */
//   gpio_fsel(RPI_V2_GPIO_P1_05, BCM2835_GPIO_FSEL_INPT); /* SCL */
// #endif
// }

// void BCM2835::bcm2835_i2c_setSlaveAddress(uint8_t addr) {
//   /* Set I2C Device Address */
// #ifdef I2C_V1
//   volatile uint32_t *paddr = bcm2835_bsc0 + BCM2835_BSC_A / 4;
// #else
//   volatile uint32_t *paddr = bcm2835_bsc1 + BCM2835_BSC_A / 4;
// #endif
//   peri_write(paddr, addr);
// }

// /* defaults to 0x5dc, should result in a 166.666 kHz I2C clock frequency.
// // The divisor must be a power of 2. Odd numbers
// // rounded down.
// */
// void BCM2835::bcm2835_i2c_setClockDivider(uint16_t divider) {
// #ifdef I2C_V1
//   volatile uint32_t *paddr = bcm2835_bsc0 + BCM2835_BSC_DIV / 4;
// #else
//   volatile uint32_t *paddr = bcm2835_bsc1 + BCM2835_BSC_DIV / 4;
// #endif
//   peri_write(paddr, divider);
//   /* Calculate time for transmitting one byte
//   // 1000000 = micros seconds in a second
//   // 9 = Clocks per byte : 8 bits + ACK
//   */
//   i2c_byte_wait_us = ((float)divider / BCM2835_CORE_CLK_HZ) * 1000000 * 9;
// }

// /* set I2C clock divider by means of a baudrate number */
// void BCM2835::bcm2835_i2c_set_baudrate(uint32_t baudrate) {
//   uint32_t divider;
//   /* use 0xFFFE mask to limit a max value and round down any odd number */
//   divider = (BCM2835_CORE_CLK_HZ / baudrate) & 0xFFFE;
//   bcm2835_i2c_setClockDivider((uint16_t)divider);
// }

// /* Writes an number of bytes to I2C */
// uint8_t BCM2835::bcm2835_i2c_write(const char *buf, uint32_t len) {
// #ifdef I2C_V1
//   volatile uint32_t *dlen = bcm2835_bsc0 + BCM2835_BSC_DLEN / 4;
//   volatile uint32_t *fifo = bcm2835_bsc0 + BCM2835_BSC_FIFO / 4;
//   volatile uint32_t *status = bcm2835_bsc0 + BCM2835_BSC_S / 4;
//   volatile uint32_t *control = bcm2835_bsc0 + BCM2835_BSC_C / 4;
// #else
//   volatile uint32_t *dlen = bcm2835_bsc1 + BCM2835_BSC_DLEN / 4;
//   volatile uint32_t *fifo = bcm2835_bsc1 + BCM2835_BSC_FIFO / 4;
//   volatile uint32_t *status = bcm2835_bsc1 + BCM2835_BSC_S / 4;
//   volatile uint32_t *control = bcm2835_bsc1 + BCM2835_BSC_C / 4;
// #endif

//   uint32_t remaining = len;
//   uint32_t i = 0;
//   uint8_t reason = BCM2835_I2C_REASON_OK;

//   /* Clear FIFO */
//   peri_set_bits(control, BCM2835_BSC_C_CLEAR_1, BCM2835_BSC_C_CLEAR_1);
//   /* Clear Status */
//   peri_write(status,
//              BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR | BCM2835_BSC_S_DONE);
//   /* Set Data Length */
//   peri_write(dlen, len);
//   /* pre populate FIFO with max buffer */
//   while (remaining && (i < BCM2835_BSC_FIFO_SIZE)) {
//     peri_write_nb(fifo, buf[i]);
//     i++;
//     remaining--;
//   }

//   /* Enable device and start transfer */
//   peri_write(control, BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST);

//   /* Transfer is over when BCM2835_BSC_S_DONE */
//   while (!(peri_read(status) & BCM2835_BSC_S_DONE)) {
//     while (remaining && (peri_read(status) & BCM2835_BSC_S_TXD)) {
//       /* Write to FIFO */
//       peri_write(fifo, buf[i]);
//       i++;
//       remaining--;
//     }
//   }

//   /* Received a NACK */
//   if (peri_read(status) & BCM2835_BSC_S_ERR) {
//     reason = BCM2835_I2C_REASON_ERROR_NACK;
//   }

//   /* Received Clock Stretch Timeout */
//   else if (peri_read(status) & BCM2835_BSC_S_CLKT) {
//     reason = BCM2835_I2C_REASON_ERROR_CLKT;
//   }

//   /* Not all data is sent */
//   else if (remaining) {
//     reason = BCM2835_I2C_REASON_ERROR_DATA;
//   }

//   peri_set_bits(control, BCM2835_BSC_S_DONE, BCM2835_BSC_S_DONE);

//   return reason;
// }

// /* Read an number of bytes from I2C */
// uint8_t BCM2835::bcm2835_i2c_read(char *buf, uint32_t len) {
// #ifdef I2C_V1
//   volatile uint32_t *dlen = bcm2835_bsc0 + BCM2835_BSC_DLEN / 4;
//   volatile uint32_t *fifo = bcm2835_bsc0 + BCM2835_BSC_FIFO / 4;
//   volatile uint32_t *status = bcm2835_bsc0 + BCM2835_BSC_S / 4;
//   volatile uint32_t *control = bcm2835_bsc0 + BCM2835_BSC_C / 4;
// #else
//   volatile uint32_t *dlen = bcm2835_bsc1 + BCM2835_BSC_DLEN / 4;
//   volatile uint32_t *fifo = bcm2835_bsc1 + BCM2835_BSC_FIFO / 4;
//   volatile uint32_t *status = bcm2835_bsc1 + BCM2835_BSC_S / 4;
//   volatile uint32_t *control = bcm2835_bsc1 + BCM2835_BSC_C / 4;
// #endif

//   uint32_t remaining = len;
//   uint32_t i = 0;
//   uint8_t reason = BCM2835_I2C_REASON_OK;

//   /* Clear FIFO */
//   peri_set_bits(control, BCM2835_BSC_C_CLEAR_1, BCM2835_BSC_C_CLEAR_1);
//   /* Clear Status */
//   peri_write_nb(status,
//                 BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR | BCM2835_BSC_S_DONE);
//   /* Set Data Length */
//   peri_write_nb(dlen, len);
//   /* Start read */
//   peri_write_nb(control,
//                 BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST | BCM2835_BSC_C_READ);

//   /* wait for transfer to complete */
//   while (!(peri_read_nb(status) & BCM2835_BSC_S_DONE)) {
//     /* we must empty the FIFO as it is populated and not use any delay */
//     while (remaining && peri_read_nb(status) & BCM2835_BSC_S_RXD) {
//       /* Read from FIFO, no barrier */
//       buf[i] = peri_read_nb(fifo);
//       i++;
//       remaining--;
//     }
//   }

//   /* transfer has finished - grab any remaining stuff in FIFO */
//   while (remaining && (peri_read_nb(status) & BCM2835_BSC_S_RXD)) {
//     /* Read from FIFO, no barrier */
//     buf[i] = peri_read_nb(fifo);
//     i++;
//     remaining--;
//   }

//   /* Received a NACK */
//   if (peri_read(status) & BCM2835_BSC_S_ERR) {
//     reason = BCM2835_I2C_REASON_ERROR_NACK;
//   }

//   /* Received Clock Stretch Timeout */
//   else if (peri_read(status) & BCM2835_BSC_S_CLKT) {
//     reason = BCM2835_I2C_REASON_ERROR_CLKT;
//   }

//   /* Not all data is received */
//   else if (remaining) {
//     reason = BCM2835_I2C_REASON_ERROR_DATA;
//   }

//   peri_set_bits(status, BCM2835_BSC_S_DONE, BCM2835_BSC_S_DONE);

//   return reason;
// }

// /* Read an number of bytes from I2C sending a repeated start after writing
// // the required register. Only works if your device supports this mode
// */
// uint8_t BCM2835::bcm2835_i2c_read_register_rs(char *regaddr, char *buf,
//                                               uint32_t len) {
// #ifdef I2C_V1
//   volatile uint32_t *dlen = bcm2835_bsc0 + BCM2835_BSC_DLEN / 4;
//   volatile uint32_t *fifo = bcm2835_bsc0 + BCM2835_BSC_FIFO / 4;
//   volatile uint32_t *status = bcm2835_bsc0 + BCM2835_BSC_S / 4;
//   volatile uint32_t *control = bcm2835_bsc0 + BCM2835_BSC_C / 4;
// #else
//   volatile uint32_t *dlen = bcm2835_bsc1 + BCM2835_BSC_DLEN / 4;
//   volatile uint32_t *fifo = bcm2835_bsc1 + BCM2835_BSC_FIFO / 4;
//   volatile uint32_t *status = bcm2835_bsc1 + BCM2835_BSC_S / 4;
//   volatile uint32_t *control = bcm2835_bsc1 + BCM2835_BSC_C / 4;
// #endif
//   uint32_t remaining = len;
//   uint32_t i = 0;
//   uint8_t reason = BCM2835_I2C_REASON_OK;

//   /* Clear FIFO */
//   peri_set_bits(control, BCM2835_BSC_C_CLEAR_1, BCM2835_BSC_C_CLEAR_1);
//   /* Clear Status */
//   peri_write(status,
//              BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR | BCM2835_BSC_S_DONE);
//   /* Set Data Length */
//   peri_write(dlen, 1);
//   /* Enable device and start transfer */
//   peri_write(control, BCM2835_BSC_C_I2CEN);
//   peri_write(fifo, regaddr[0]);
//   peri_write(control, BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST);

//   /* poll for transfer has started */
//   while (!(peri_read(status) & BCM2835_BSC_S_TA)) {
//     /* Linux may cause us to miss entire transfer stage */
//     if (peri_read(status) & BCM2835_BSC_S_DONE)
//       break;
//   }

//   /* Send a repeated start with read bit set in address */
//   peri_write(dlen, len);
//   peri_write(control,
//              BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST | BCM2835_BSC_C_READ);

//   /* Wait for write to complete and first byte back. */
//   delayMicroseconds(i2c_byte_wait_us * 3);

//   /* wait for transfer to complete */
//   while (!(peri_read(status) & BCM2835_BSC_S_DONE)) {
//     /* we must empty the FIFO as it is populated and not use any delay */
//     while (remaining && peri_read(status) & BCM2835_BSC_S_RXD) {
//       /* Read from FIFO */
//       buf[i] = peri_read(fifo);
//       i++;
//       remaining--;
//     }
//   }

//   /* transfer has finished - grab any remaining stuff in FIFO */
//   while (remaining && (peri_read(status) & BCM2835_BSC_S_RXD)) {
//     /* Read from FIFO */
//     buf[i] = peri_read(fifo);
//     i++;
//     remaining--;
//   }

//   /* Received a NACK */
//   if (peri_read(status) & BCM2835_BSC_S_ERR) {
//     reason = BCM2835_I2C_REASON_ERROR_NACK;
//   }

//   /* Received Clock Stretch Timeout */
//   else if (peri_read(status) & BCM2835_BSC_S_CLKT) {
//     reason = BCM2835_I2C_REASON_ERROR_CLKT;
//   }

//   /* Not all data is sent */
//   else if (remaining) {
//     reason = BCM2835_I2C_REASON_ERROR_DATA;
//   }

//   peri_set_bits(control, BCM2835_BSC_S_DONE, BCM2835_BSC_S_DONE);

//   return reason;
// }

// /* Sending an arbitrary number of bytes before issuing a repeated start
// // (with no prior stop) and reading a response. Some devices require this
// behavior.
// */
// uint8_t BCM2835::bcm2835_i2c_write_read_rs(char *cmds, uint32_t cmds_len,
//                                            char *buf, uint32_t buf_len) {
// #ifdef I2C_V1
//   volatile uint32_t *dlen = bcm2835_bsc0 + BCM2835_BSC_DLEN / 4;
//   volatile uint32_t *fifo = bcm2835_bsc0 + BCM2835_BSC_FIFO / 4;
//   volatile uint32_t *status = bcm2835_bsc0 + BCM2835_BSC_S / 4;
//   volatile uint32_t *control = bcm2835_bsc0 + BCM2835_BSC_C / 4;
// #else
//   volatile uint32_t *dlen = bcm2835_bsc1 + BCM2835_BSC_DLEN / 4;
//   volatile uint32_t *fifo = bcm2835_bsc1 + BCM2835_BSC_FIFO / 4;
//   volatile uint32_t *status = bcm2835_bsc1 + BCM2835_BSC_S / 4;
//   volatile uint32_t *control = bcm2835_bsc1 + BCM2835_BSC_C / 4;
// #endif

//   uint32_t remaining = cmds_len;
//   uint32_t i = 0;
//   uint8_t reason = BCM2835_I2C_REASON_OK;

//   /* Clear FIFO */
//   peri_set_bits(control, BCM2835_BSC_C_CLEAR_1, BCM2835_BSC_C_CLEAR_1);

//   /* Clear Status */
//   peri_write(status,
//              BCM2835_BSC_S_CLKT | BCM2835_BSC_S_ERR | BCM2835_BSC_S_DONE);

//   /* Set Data Length */
//   peri_write(dlen, cmds_len);

//   /* pre populate FIFO with max buffer */
//   while (remaining && (i < BCM2835_BSC_FIFO_SIZE)) {
//     peri_write_nb(fifo, cmds[i]);
//     i++;
//     remaining--;
//   }

//   /* Enable device and start transfer */
//   peri_write(control, BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST);

//   /* poll for transfer has started (way to do repeated start, from BCM2835
//    * datasheet) */
//   while (!(peri_read(status) & BCM2835_BSC_S_TA)) {
//     /* Linux may cause us to miss entire transfer stage */
//     if (peri_read_nb(status) & BCM2835_BSC_S_DONE)
//       break;
//   }

//   remaining = buf_len;
//   i = 0;

//   /* Send a repeated start with read bit set in address */
//   peri_write(dlen, buf_len);
//   peri_write(control,
//              BCM2835_BSC_C_I2CEN | BCM2835_BSC_C_ST | BCM2835_BSC_C_READ);

//   /* Wait for write to complete and first byte back. */
//   delayMicroseconds(i2c_byte_wait_us * (cmds_len + 1));

//   /* wait for transfer to complete */
//   while (!(peri_read_nb(status) & BCM2835_BSC_S_DONE)) {
//     /* we must empty the FIFO as it is populated and not use any delay */
//     while (remaining && peri_read(status) & BCM2835_BSC_S_RXD) {
//       /* Read from FIFO, no barrier */
//       buf[i] = peri_read_nb(fifo);
//       i++;
//       remaining--;
//     }
//   }

//   /* transfer has finished - grab any remaining stuff in FIFO */
//   while (remaining && (peri_read(status) & BCM2835_BSC_S_RXD)) {
//     /* Read from FIFO */
//     buf[i] = peri_read(fifo);
//     i++;
//     remaining--;
//   }

//   /* Received a NACK */
//   if (peri_read(status) & BCM2835_BSC_S_ERR) {
//     reason = BCM2835_I2C_REASON_ERROR_NACK;
//   }

//   /* Received Clock Stretch Timeout */
//   else if (peri_read(status) & BCM2835_BSC_S_CLKT) {
//     reason = BCM2835_I2C_REASON_ERROR_CLKT;
//   }

//   /* Not all data is sent */
//   else if (remaining) {
//     reason = BCM2835_I2C_REASON_ERROR_DATA;
//   }

//   peri_set_bits(control, BCM2835_BSC_S_DONE, BCM2835_BSC_S_DONE);

//   return reason;
// }
