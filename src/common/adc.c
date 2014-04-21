#ifndef ADC_C_INCLUDED
#define ADC_C_INCLUDED


/* compatibility */


#include "adc0.c"


static void adc_set_chan(uint32_t chan)
{
  adc0_set_chan(chan);
}

__attribute__((unused))
static void adc_disable(void)
{
  adc0_disable();
}

static void adc_enable(uint32_t chan)
{
  adc0_enable(chan);
}

static uint32_t adc_read_16bits(uint32_t chan)
{
  return adc0_read_16bits(chan);
}

static uint32_t adc_read(uint32_t chan)
{
  return adc0_read(chan);
}

static void adc_calibrate(void)
{
  adc0_calibrate();
}

static void adc_setup(void)
{
  adc0_setup();
}


#if 0 /* adc unit test */

int main(void)
{
  /* mcu aref is 3v */

  /* shunt current measure */
  /* ina210, connected to adc 0, channel 0 */
  /* adc0_dp0, mcu package pin 9, teensy pin a10 */
  /* rshunt = 0.025 ohms */
  /* gain = 200 */
  /* i = u / (200 * 0.025) */
  /* i = 3 * x / (2^16 * 200 * 0.025) */
  /* test: 3V @ 1.2K ohms is 2.5mA */

  /* vload measure */
  /* rdiv_4, connected to adc 1, channel 0 */
  /* adc1_dp0, mcu package pin 11, teensy pin a12 */
  /* gain = 1/4 */
  /* u = 3 * 4 * x / (2^16) */

  uint32_t i = 0;
  uint32_t j = 0;
  uint16_t dac_val = 0;
  uint32_t sum[2] = { 0, 0 };
  uint16_t x[2];

  serial_setup();
  adc0_setup();
  adc1_setup();
  dac_setup();

  dac_enable();

  adc0_start_continuous(0);
  adc1_start_continuous(0);

  while (1)
  {
    sum[0] += adc0_read_continuous(0);
    sum[1] += adc1_read_continuous(0);

    if ((++i) == 1000)
    {
      x[0] = (uint16_t)(sum[0] / i);
      x[1] = (uint16_t)(sum[1] / i);

#if 0 /* ascii format */
      serial_write(uint16_to_string(x[0]), 4);
      SERIAL_WRITE_STRING(" ");
      serial_write(uint16_to_string(x[1]), 4);
      SERIAL_WRITE_STRING("\r\n");
#else /* binary format */
      serial_write((const uint8_t*)x, sizeof(x));
#endif

      sum[0] = 0;
      sum[1] = 0;
      i = 0;

      if ((++j) == 5)
      {
	dac_set(dac_val++);
	j = 0;
      }
    }
  }

  return 0;
}

#endif /* adc unit test */


#endif /* ADC_C_INCLUDED */
