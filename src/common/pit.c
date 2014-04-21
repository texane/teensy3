#ifndef PIT_C_INCLUDED
#define PIT_C_INCLUDED


#include <stdint.h>
#include "mk20dx128.h"


static void pit_start(unsigned int i, uint32_t n)
{
  /* i the timer identifier, in 0:3 */
  /* n the period */

  volatile uint32_t* const PIT_LDVAL = (&PIT_LDVAL0) + i * 4;
  volatile uint32_t* const PIT_TCTRL = (&PIT_TCTRL0) + i * 4;
  volatile uint32_t* const PIT_TFLG = (&PIT_TFLG0) + i * 4;
  const uint32_t IRQ_PIT_CH = IRQ_PIT_CH0 + i;

  /* enable pit clock */
  SIM_SCGC6 |= 1 << 23;

  /* section 37.3 */

  /* enable the module clock */
  PIT_MCR = 0;

  /* timer load value */
  /* countdown to 0 */
  /* loaded after the timer expires */
  *PIT_LDVAL = n;

  /* clear pending interrupt */
  *PIT_TFLG = 1 << 0;

  /* enable interrupt vector */
  NVIC_ENABLE_IRQ(IRQ_PIT_CH);

  /* enable timer */
  /* interrupt enabled */
  *PIT_TCTRL = (1 << 1) | (1 << 0);
}

static void pit_stop(unsigned int i)
{
  volatile uint32_t* const PIT_TCTRL = (&PIT_TCTRL0) + i * 4;
  const uint32_t IRQ_PIT_CH = IRQ_PIT_CH0 + i;
  PIT_MCR = 1;
  *PIT_TCTRL = 0;
  NVIC_DISABLE_IRQ(IRQ_PIT_CH);
}

static inline void pit_clear_int(unsigned int i)
{
  volatile uint32_t* const PIT_TFLG = (&PIT_TFLG0) + i * 4;
  *PIT_TFLG |= 1 << 0;
}

static inline unsigned int pit_is_int(unsigned int i)
{
  volatile uint32_t* const PIT_TFLG = (&PIT_TFLG0) + i * 4;
  return *PIT_TFLG & (1 << 0);
}

static inline uint32_t pit_get_val(unsigned int i)
{
  volatile uint32_t* const PIT_CVAL = (&PIT_CVAL0) + i * 4;
  return *PIT_CVAL;
}


#if 0 /* unit */

static uint32_t led_val = 0;

void pit1_isr(void)
{
  led_val ^= 1;
  led_set_val(led_val & 1);

  pit_clear_int(1);
}

int main(void)
{
  led_setup();
  pit_start(1, 24000000);
  while (1) ;
}

#endif /* unit */


#endif /* PIT_C_INCLUDED */
