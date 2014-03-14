#ifndef LED_C_INCLUDED
#define LED_C_INCLUDED


#include <stdint.h>
#include "mk20dx128.h"


/* onboard led */

static inline void led_setup(void)
{
  /* interrupt or dma disabled */
  /* not locked */
  /* mux set to gpio mode */
  /* high drive strength */
  /* open drain disabled */
  /* passive filter disabled */
  /* slow slew rate enabled */
  /* internal pullup disabled */

  PORTC_PCR5 = (1 << 8) | (1 << 6) | (1 << 2);

  /* enable PORTC pin 5 write */
  /* PORTC_GPCLR = 1 << (16 + 5); */

  GPIOC_PDDR |= 1 << 5;
}

static inline void led_set_val(unsigned int x)
{
#if 0
  /* warning: GPIOx_PyOR already dereferenced */
  if (x) GPIOC_PSOR = 1 << 5;
  else GPIOC_PCOR = 1 << 5;
#else
  if (x) GPIOC_PDOR |= 1 << 5;
  else GPIOC_PDOR &= ~(1 << 5);
#endif
}


#endif /* LED_C_INCLUDED */
