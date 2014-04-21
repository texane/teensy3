#ifndef DAC_C_INCLUDED
#define DAC_C_INCLUDED

#include <stdint.h>
#include "mk20dx128.h"


static void dac_setup(void)
{
  /* enable dac0 module */
  SIM_SCGC2 |= 1 << 12;

  /* dac is disabled. 3.3v selected. */
  DAC0_C0 = 1 << 6;

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


#endif /* DAC_C_INCLUDED */
