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


#endif /* ADC_C_INCLUDED */
