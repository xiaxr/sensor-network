#pragma once

namespace xiaxr {
namespace bcm2835 {
class I2C {
public:
  /*! \defgroup i2c I2C access
These functions let you use I2C (The Broadcom Serial Control bus with the
Philips I2C bus/interface version 2.1 January 2000.) to interface with an
external I2C device.
@{
*/

  /*! Start I2C operations.
    Forces RPi I2C pins P1-03 (SDA) and P1-05 (SCL)
    to alternate function ALT0, which enables those pins for I2C interface.
    You should call bcm2835_i2c_end() when all I2C functions are complete to
    return the pins to their default functions \return 1 if successful, 0
    otherwise (perhaps because you are not running as root) \sa
    bcm2835_i2c_end()
  */
  int bcm2835_i2c_begin(void);

  /*! End I2C operations.
    I2C pins P1-03 (SDA) and P1-05 (SCL)
    are returned to their default INPUT behaviour.
  */
  void bcm2835_i2c_end(void);

  /*! Sets the I2C slave address.
    \param[in] addr The I2C slave address.
  */
  void bcm2835_i2c_setSlaveAddress(uint8_t addr);

  /*! Sets the I2C clock divider and therefore the I2C clock speed.
    \param[in] divider The desired I2C clock divider, one of
    BCM2835_I2C_CLOCK_DIVIDER_*, see \ref bcm2835I2CClockDivider
  */
  void bcm2835_i2c_setClockDivider(uint16_t divider);

  /*! Sets the I2C clock divider by converting the baudrate parameter to
    the equivalent I2C clock divider. ( see \sa bcm2835_i2c_setClockDivider)
    For the I2C standard 100khz you would set baudrate to 100000
    The use of baudrate corresponds to its use in the I2C kernel device
    driver. (Of course, bcm2835 has nothing to do with the kernel driver)
  */
  void bcm2835_i2c_set_baudrate(uint32_t baudrate);

  /*! Transfers any number of bytes to the currently selected I2C slave.
    (as previously set by \sa bcm2835_i2c_setSlaveAddress)
    \param[in] buf Buffer of bytes to send.
    \param[in] len Number of bytes in the buf buffer, and the number of bytes to
    send. \return reason see \ref bcm2835I2CReasonCodes
  */
  uint8_t bcm2835_i2c_write(const char *buf, uint32_t len);

  /*! Transfers any number of bytes from the currently selected I2C slave.
    (as previously set by \sa bcm2835_i2c_setSlaveAddress)
    \param[in] buf Buffer of bytes to receive.
    \param[in] len Number of bytes in the buf buffer, and the number of bytes to
    received. \return reason see \ref bcm2835I2CReasonCodes
  */
  uint8_t bcm2835_i2c_read(char *buf, uint32_t len);

  /*! Allows reading from I2C slaves that require a repeated start (without any
    prior stop) to read after the required slave register has been set. For
    example, the popular MPL3115A2 pressure and temperature sensor. Note that
    your device must support or require this mode. If your device does not
    require this mode then the standard combined: \sa bcm2835_i2c_write \sa
    bcm2835_i2c_read are a better choice. Will read from the slave previously
    set by \sa bcm2835_i2c_setSlaveAddress \param[in] regaddr Buffer containing
    the slave register you wish to read from. \param[in] buf Buffer of bytes to
    receive. \param[in] len Number of bytes in the buf buffer, and the number of
    bytes to received. \return reason see \ref bcm2835I2CReasonCodes
  */
  uint8_t bcm2835_i2c_read_register_rs(char *regaddr, char *buf, uint32_t len);

  /*! Allows sending an arbitrary number of bytes to I2C slaves before issuing a
    repeated start (with no prior stop) and reading a response. Necessary for
    devices that require such behavior, such as the MLX90620. Will write to and
    read from the slave previously set by \sa bcm2835_i2c_setSlaveAddress
    \param[in] cmds Buffer containing the bytes to send before the repeated
    start condition. \param[in] cmds_len Number of bytes to send from cmds
    buffer \param[in] buf Buffer of bytes to receive. \param[in] buf_len Number
    of bytes to receive in the buf buffer. \return reason see \ref
    bcm2835I2CReasonCodes
  */
  uint8_t bcm2835_i2c_write_read_rs(char *cmds, uint32_t cmds_len, char *buf,
                                    uint32_t buf_len);

private:
  /* I2C The time needed to transmit one byte. In microseconds.
   */
  int i2c_byte_wait_us; // i2c_byte_wait_us(0),
};
} // namespace bcm2835
} // namespace xiaxr