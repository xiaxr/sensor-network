// TODO
//   /*! @} */

//   /*! \defgroup st System Timer access
//     Allows access to and delays using the System Timer Counter.
//     @{
//   */

//   /*! Read the System Timer Counter register.
//     \return the value read from the System Timer Counter Lower 32 bits register
//   */
//   uint64_t bcm2835_st_read(void);

//   /*! Delays for the specified number of microseconds with offset.
//     \param[in] offset_micros Offset in microseconds
//     \param[in] micros Delay in microseconds
//   */
//   void bcm2835_st_delay(uint64_t offset_micros, uint64_t micros);

//   /*! @}  */

//   /*! \defgroup pwm Pulse Width Modulation
//     Allows control of 2 independent PWM channels. A limited subset of GPIO pins
//     can be connected to one of these 2 channels, allowing PWM control of GPIO
//     pins. You have to set the desired pin into a particular Alt Fun to PWM
//     output. See the PWM documentation on the Main Page.
//     @{
//   */

//   /*! Sets the PWM clock divisor,
//     to control the basic PWM pulse widths.
//     \param[in] divisor Divides the basic 19.2MHz PWM clock. You can use one of
//     the common values BCM2835_PWM_CLOCK_DIVIDER_* in \ref bcm2835PWMClockDivider
//   */
//   void bcm2835_pwm_set_clock(uint32_t divisor);

//   /*! Sets the mode of the given PWM channel,
//     allowing you to control the PWM mode and enable/disable that channel
//     \param[in] channel The PWM channel. 0 or 1.
//     \param[in] markspace Set true if you want Mark-Space mode. 0 for Balanced
//     mode. \param[in] enabled Set true to enable this channel and produce PWM
//     pulses.
//   */
//   void bcm2835_pwm_set_mode(uint8_t channel, uint8_t markspace,
//                             uint8_t enabled);

//   /*! Sets the maximum range of the PWM output.
//     The data value can vary between 0 and this range to control PWM output
//     \param[in] channel The PWM channel. 0 or 1.
//     \param[in] range The maximum value permitted for DATA.
//   */
//   void bcm2835_pwm_set_range(uint8_t channel, uint32_t range);

//   /*! Sets the PWM pulse ratio to emit to DATA/RANGE,
//     where RANGE is set by bcm2835_pwm_set_range().
//     \param[in] channel The PWM channel. 0 or 1.
//     \param[in] data Controls the PWM output ratio as a fraction of the range.
//     Can vary from 0 to RANGE.
//   */
//   void bcm2835_pwm_set_data(uint8_t channel, uint32_t data);

/* Read the System Timer Counter (64-bits) */
// uint64_t BCM2835::bcm2835_st_read(void) {
//   volatile uint32_t *paddr;
//   uint32_t hi, lo;
//   uint64_t st;

//   if (bcm2835_st == MAP_FAILED)
//     return 0;

//   paddr = bcm2835_st + BCM2835_ST_CHI / 4;
//   hi = peri_read(paddr);

//   paddr = bcm2835_st + BCM2835_ST_CLO / 4;
//   lo = peri_read(paddr);

//   paddr = bcm2835_st + BCM2835_ST_CHI / 4;
//   st = peri_read(paddr);

//   /* Test for overflow */
//   if (st == hi) {
//     st <<= 32;
//     st += lo;
//   } else {
//     st <<= 32;
//     paddr = bcm2835_st + BCM2835_ST_CLO / 4;
//     st += peri_read(paddr);
//   }
//   return st;
// }

// /* Delays for the specified number of microseconds with offset */
// void BCM2835::bcm2835_st_delay(uint64_t offset_micros, uint64_t micros) {
//   uint64_t compare = offset_micros + micros;

//   while (bcm2835_st_read() < compare)
//     ;
// }

// /* PWM */

// void BCM2835::bcm2835_pwm_set_clock(uint32_t divisor) {
//   if (bcm2835_clk == MAP_FAILED || bcm2835_pwm == MAP_FAILED)
//     return; /* bcm2835_init() failed or not root */

//   /* From Gerts code */
//   divisor &= 0xfff;
//   /* Stop PWM clock */
//   peri_write(bcm2835_clk + BCM2835_PWMCLK_CNTL, BCM2835_PWM_PASSWRD | 0x01);
//   delay(110); /* Prevents clock going slow */
//   /* Wait for the clock to be not busy */
//   while ((peri_read(bcm2835_clk + BCM2835_PWMCLK_CNTL) & 0x80) != 0) {
//     delay(1);
//   }
//   /* set the clock divider and enable PWM clock */
//   peri_write(bcm2835_clk + BCM2835_PWMCLK_DIV,
//              BCM2835_PWM_PASSWRD | (divisor << 12));
//   peri_write(bcm2835_clk + BCM2835_PWMCLK_CNTL,
//              BCM2835_PWM_PASSWRD | 0x11); /* Source=osc and enable */
// }

// void BCM2835::bcm2835_pwm_set_mode(uint8_t channel, uint8_t markspace,
//                                    uint8_t enabled) {
//   if (bcm2835_clk == MAP_FAILED || bcm2835_pwm == MAP_FAILED)
//     return; /* bcm2835_init() failed or not root */

//   uint32_t control = peri_read(bcm2835_pwm + BCM2835_PWM_CONTROL);

//   if (channel == 0) {
//     if (markspace)
//       control |= BCM2835_PWM0_MS_MODE;
//     else
//       control &= ~BCM2835_PWM0_MS_MODE;
//     if (enabled)
//       control |= BCM2835_PWM0_ENABLE;
//     else
//       control &= ~BCM2835_PWM0_ENABLE;
//   } else if (channel == 1) {
//     if (markspace)
//       control |= BCM2835_PWM1_MS_MODE;
//     else
//       control &= ~BCM2835_PWM1_MS_MODE;
//     if (enabled)
//       control |= BCM2835_PWM1_ENABLE;
//     else
//       control &= ~BCM2835_PWM1_ENABLE;
//   }

//   /* If you use the barrier here, wierd things happen, and the commands dont
//    * work */
//   peri_write_nb(bcm2835_pwm + BCM2835_PWM_CONTROL, control);
//   /*  peri_write_nb(bcm2835_pwm + BCM2835_PWM_CONTROL,
//    * BCM2835_PWM0_ENABLE | BCM2835_PWM1_ENABLE | BCM2835_PWM0_MS_MODE |
//    * BCM2835_PWM1_MS_MODE); */
// }

// void BCM2835::bcm2835_pwm_set_range(uint8_t channel, uint32_t range) {
//   if (bcm2835_clk == MAP_FAILED || bcm2835_pwm == MAP_FAILED)
//     return; /* bcm2835_init() failed or not root */

//   if (channel == 0)
//     peri_write_nb(bcm2835_pwm + BCM2835_PWM0_RANGE, range);
//   else if (channel == 1)
//     peri_write_nb(bcm2835_pwm + BCM2835_PWM1_RANGE, range);
// }

// void BCM2835::bcm2835_pwm_set_data(uint8_t channel, uint32_t data) {
//   if (bcm2835_clk == MAP_FAILED || bcm2835_pwm == MAP_FAILED)
//     return; /* bcm2835_init() failed or not root */

//   if (channel == 0)
//     peri_write_nb(bcm2835_pwm + BCM2835_PWM0_DATA, data);
//   else if (channel == 1)
//     peri_write_nb(bcm2835_pwm + BCM2835_PWM1_DATA, data);
// }
