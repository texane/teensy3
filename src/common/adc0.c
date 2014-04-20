#ifndef ADC0_C_INCLUDED
#define ADC0_C_INCLUDED


#include <stdint.h>
#include "mk20dx128.h"


/* 16 bits SAR ADC, chapter 31 */

/* adc static configuration */
#define ADC0_RES_BITS 16
#define ADC0_VMAX (double)(1 << ADC0_RES_BITS)
#define ADC0_USE_INTERNAL_VREF 0

#if (ADC0_USE_INTERNAL_VREF == 1)
#define ADC0_VREF 1.2
#else /* external vref */
#define ADC0_VREF 3.3
#endif /* ADC0_USE_INTERNAL_VREF */

static void adc0_set_chan(uint32_t chan)
{
  ADC0_SC1A = (ADC0_SC1A & ~(0x1f)) | chan;
}

__attribute__((unused))
static void adc0_disable(void)
{
  /* the SAR logic is turned off when ADCH all ones */
  adc0_set_chan(0x1f);
}

static void adc0_enable(uint32_t chan)
{
  /* start a conversion on channel */
  adc0_set_chan(chan);
}

static uint32_t adc0_read_16bits(uint32_t chan)
{
  uint32_t x;

  adc0_enable(chan);

  /* conversion complete */
  while ((ADC0_SC1A & ADC_SC1_COCO) == 0) ;

  /* note: COCO cleared by result reading */
  x = ADC0_RA;

  /* the module is idle after conversion */
  /* with async clock disabled, idle is the */
  /* power lowest possible state */

  /* adc0_disable(); */

  /* msb reserved, always 0 */
  return x;
}

static uint32_t adc0_read(uint32_t chan)
{
  return adc0_read_16bits(chan) >> (16 - ADC0_RES_BITS);
}

static void adc0_calibrate(void)
{
  uint16_t x;

  /* self clear bit */
  ADC0_SC3 |= ADC_SC3_CAL;

  /* while ((ADC0_SC1A & ADC0_SC1_COCO) == 0) ; */
  while (ADC0_SC3 & ADC_SC3_CAL) ;

  /* note ADC0_SC3 & ADC_SC3_CALF if calibration failed */

  /* set plus side gain, page 619 */
  x = 0;
  x += ADC0_CLP0;
  x += ADC0_CLP1;
  x += ADC0_CLP2;
  x += ADC0_CLP3;
  x += ADC0_CLP4;
  x += ADC0_CLPS;
  ADC0_PG = (x >> 1) | (1 << 15);

  /* set minus side gain, page 619 */
  x = 0;
  x += ADC0_CLM0;
  x += ADC0_CLM1;
  x += ADC0_CLM2;
  x += ADC0_CLM3;
  x += ADC0_CLM4;
  x += ADC0_CLMS;
  ADC0_MG = (x >> 1) | (1 << 15);
}

static void adc0_setup(void)
{
  /* 16 bits single ended mode SAR ADC */

  /* TODO: PORTC0 / pin15 as input */
  GPIOC_PDDR &= ~(1 << 0);

#if (ADC0_USE_INTERNAL_VREF == 1)

  /* voltage reference, chapter 33 */

  /* enable VREF module */
  SIM_SCGC4 |= SIM_SCGC4_VREF;

  /* TRIM: maximum value */
  /* CHOPEN: enabled */
  VREF_TRM = (1 << 6) | 0x3f;

  /* VREFEN: internal voltage reference enabled */
  /* VREGEN: 1.75v internal regulator enabled */
  /* ICOMPEN: compensation enabled */
  /* MODE_LV: low power buffer mode */
  VREF_SC = (1 << 7) | (1 << 6) | (1 << 5) | (2 << 0);

  /* wait for VREF stable */
  while ((VREF_SC & (1 << 2)) == 0) ;

#endif /* ADC0_USE_INTERNAL_VREF */

  /* enable ADC0 module */
  SIM_SCGC6 |= SIM_SCGC6_ADC0;

  /* ADC0_SC1A, status control register */
  /* AIEN: interrupt disabled */
  /* DIFF: single ended mode */
  /* ADCH: selected channel, 0x1f module disabled */
  ADC0_SC1A = 0x1f;

  /* the alternate clock is connected to OSCERCLK, 16mhz */
  /* datasheet say that ADC clock should be 2 to 12MHz for 16 bit mode */
  /* ADC0_CFG1, configuration register */
  /* ADLPC: low power configuration */
  /* ADIV: CLOCK / 8 */
  /* ADLSMP: long sample time enabled */
  /* MODE: single ended 16 bits conversion */
  /* ADICLK: bus clock / 2 */
  ADC0_CFG1 = ADC_CFG1_ADLSMP | ADC_CFG1_MODE(3) | ADC_CFG1_ADIV(1) | ADC_CFG1_ADICLK(1);

  /* ADC0_CFG2, configuration register */
  /* MUXSEL: ADxxa channels selected */
  /* ADACKEN: async clock disabled */
  /* ADHSC: high speed disabled */
  /* ADLSTS: longest sample time (24 ADCK cycles) */
  ADC0_CFG2 = ADC_CFG2_ADLSTS(2);

  /* ADC0_SC2, conversion, triggering */
  /* ADCTRG: software triggered conversion */
  /* ACFE: compare function disabled */
  /* ACFGT: greater than compare function disabled */
  /* ACREN: range compare function disabled */
  /* DMAEN: DMA disabled */
#if (ADC0_USE_INTERNAL_VREF == 1)
  /* REFSEL: voltage ref on alternate pair (internal) */
  ADC0_SC2 = 1;
#else
  /* REFSEL: voltage on external pair */
  ADC0_SC2 = 0;
#endif

  /* ADC0_SC3, status control register 3 */
  /* ADCO: continuous conversion disabled */
  /* AVGE: hardware averaging disabled */
  /* AVGS: averaged sample count, not applicable */
  ADC0_SC3 = 0;

  adc0_calibrate();
}


#endif /* ADC0_C_INCLUDED */
