#ifndef VREF_C_INCLUDED
#define VREF_C_INCLUDED


#include <stdint.h>
#include "mk20dx128.h"


static void vref_setup(void)
{
  /* voltage reference, chapter 34 */

  /* enable VREF module */
  SIM_SCGC4 |= SIM_SCGC4_VREF;

  /* TRIM: minimum value, 1.2V */
  /* CHOPEN: enabled */
  VREF_TRM = 1 << 6;

  /* VREFEN: internal voltage reference enabled */
  /* VREGEN: 1.75v internal regulator enabled */
  /* ICOMPEN: compensation enabled */
  /* MODE_LV: low power buffer mode */
  VREF_SC = (1 << 7) | (1 << 6) | (1 << 5) | (2 << 0);

  /* wait for VREF stable */
  while ((VREF_SC & (1 << 2)) == 0) ;
}


#endif /* VREF_C_INCLUDED */
