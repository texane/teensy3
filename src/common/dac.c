#ifndef DAC_C_INCLUDED
#define DAC_C_INCLUDED

#include <stdint.h>
#include "mk20dx128.h"


#ifdef DAC_USE_VREF
#define DAC_VREF_MV 1200
#else
#define DAC_VREF_MV 3300
#endif /* DAC_USE_VREF */


static void dac_setup(void)
{
  /* enable dac0 module */
  SIM_SCGC2 |= 1 << 12;

#ifdef DAC_USE_VREF
  /* dac is disabled. internal vref selected. */
  DAC0_C0 = 0;
#else
  /* dac is disabled. 3.3v selected. */
  DAC0_C0 = 1 << 6;
#endif /* DAC_USE_VREF */

  /* single word buffer */
  DAC0_C1 = 0;
}

static void dac_set(uint16_t x)
{
  *((volatile uint16_t*)&DAC0_DAT0L) = x & 0xfff;
}

static void dac_enable(void)
{
  dac_set(0);
  DAC0_C0 |= 1 << 7;
}

static void dac_disable(void)
{
  DAC0_C0 &= ~(1 << 7);
}

static inline uint16_t dac_mv_to_val(uint32_t mv)
{
  /* DAC_VREF_MV * x / 4096 = mv */
  /* x = mv * 4096 / DAC_VREF_MV */
  return (mv * 4096) / DAC_VREF_MV;
}


#if 0 /* dac unit test */

static void finer_delay(void)
{
  volatile uint32_t x;
  for (x = 0; x != 100; ++x) ;
}

int main(void)
{
  uint16_t x;

  dac_setup();

  dac_enable();

  for (x = 0; 1; ++x)
  {
    finer_delay();
    dac_set(x);
  }

  return 0;
}

#endif /* dac unit test */


#endif /* DAC_C_INCLUDED */
