#pragma once
// TODO
// spi1_speed(0)
//   /*! Start AUX SPI operations.
//     Forces RPi AUX SPI pins P1-38 (MOSI), P1-38 (MISO), P1-40 (CLK) and P1-36
//     (CE2) to alternate function ALT4, which enables those pins for SPI
//     interface. \return 1 if successful, 0 otherwise (perhaps because you are not
//     running as root)
//   */
//   int bcm2835_aux_spi_begin(void);

//   /*! End AUX SPI operations.
//      SPI1 pins P1-38 (MOSI), P1-38 (MISO), P1-40 (CLK) and P1-36 (CE2)
//      are returned to their default INPUT behaviour.
//    */
//   void bcm2835_aux_spi_end(void);

//   /*! Sets the AUX SPI clock divider and therefore the AUX SPI clock speed.
//     \param[in] divider The desired AUX SPI clock divider.
//   */
//   void bcm2835_aux_spi_setClockDivider(uint16_t divider) {
//     spi1_speed = (uint32_t)divider;
//   }

//   /*!
//    * Calculates the input for \sa bcm2835_aux_spi_setClockDivider
//    * @param speed_hz A value between \sa BCM2835_AUX_SPI_CLOCK_MIN and \sa
//    * BCM2835_AUX_SPI_CLOCK_MAX
//    * @return Input for \sa bcm2835_aux_spi_setClockDivider
//    */
//   uint16_t bcm2835_aux_spi_CalcClockDivider(uint32_t speed_hz);

//   /*! Transfers half-word to the AUX SPI slave.
//     Asserts the currently selected CS pins during the transfer.
//     \param[in] data The 8 bit data byte to write to MOSI
//     \return The 16 bit byte simultaneously read from  MISO
//     \sa bcm2835_spi_transfern()
//   */
//   void bcm2835_aux_spi_write(uint16_t data);

//   /*! Transfers any number of bytes to the AUX SPI slave.
//     Asserts the CE2 pin during the transfer.
//     \param[in] buf Buffer of bytes to send.
//     \param[in] len Number of bytes in the tbuf buffer, and the number of bytes
//     to send
//   */
//   void bcm2835_aux_spi_writenb(const char *buf, uint32_t len);

//   /*! Transfers any number of bytes to and from the AUX SPI slave
//     using bcm2835_aux_spi_transfernb.
//     The returned data from the slave replaces the transmitted data in the
//     buffer. \param[in,out] buf Buffer of bytes to send. Received bytes will
//     replace the contents \param[in] len Number of bytes in the buffer, and the
//     number of bytes to send/received \sa bcm2835_aux_spi_transfer()
//   */
//   void bcm2835_aux_spi_transfern(char *buf, uint32_t len) {
//     bcm2835_aux_spi_transfernb(buf, buf, len);
//   }

//   /*! Transfers any number of bytes to and from the AUX SPI slave.
//     Asserts the CE2 pin during the transfer.
//     Clocks the len 8 bit bytes out on MOSI, and simultaneously clocks in data
//     from MISO. The data read read from the slave is placed into rbuf. rbuf must
//     be at least len bytes long \param[in] tbuf Buffer of bytes to send.
//     \param[out] rbuf Received bytes will by put in this buffer \param[in] len
//     Number of bytes in the tbuf buffer, and the number of bytes to send/received
//   */
//   void bcm2835_aux_spi_transfernb(const char *tbuf, char *rbuf, uint32_t len);

//   /*! Transfers one byte to and from the AUX SPI slave.
//     Clocks the 8 bit value out on MOSI, and simultaneously clocks in data from
//     MISO. Returns the read data byte from the slave. \param[in] value The 8 bit
//     data byte to write to MOSI \return The 8 bit byte simultaneously read from
//     MISO \sa bcm2835_aux_spi_transfern()
//   */
//   uint8_t bcm2835_aux_spi_transfer(uint8_t value);

//   uint32_t spi1_speed;
