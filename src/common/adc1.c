#ifndef ADC1_C_INCLUDED
#define ADC1_C_INCLUDED


#include <stdint.h>
#include "mk20dx128.h"


/* 16 bits SAR ADC, chapter 31 */

/* adc static configuration */
#define ADC1_RES_BITS 16
#define ADC1_VMAX (double)(1 << ADC1_RES_BITS)
#define ADC1_USE_INTERNAL_VREF 0

#if (ADC1_USE_INTERNAL_VREF == 1)
#define ADC1_VREF 1.2
#else /* external vref */
#define ADC1_VREF 3.3
#endif /* ADC1_USE_INTERNAL_VREF */

static void adc1_set_chan(uint32_t chan)
{
  ADC1_SC1A = (ADC1_SC1A & ~(0x1f)) | chan;
}

__attribute__((unused))
static void adc1_disable(void)
{
  /* the SAR logic is turned off when ADCH all ones */
  adc1_set_chan(0x1f);
}

static void adc1_enable(uint32_t chan)
{
  /* start a conversion on channel */
  adc1_set_chan(chan);
}

static uint32_t adc1_read_16bits(uint32_t chan)
{
  uint32_t x;

  adc1_enable(chan);

  /* conversion complete */
  while ((ADC1_SC1A & ADC_SC1_COCO) == 0) ;

  /* note: COCO cleared by result reading */
  x = ADC1_RA;

  /* the module is idle after conversion */
  /* with async clock disabled, idle is the */
  /* power lowest possible state */

  /* adc1_disable(); */

  /* msb reserved, always 0 */
  return x;
}

static uint32_t adc1_read(uint32_t chan)
{
  return adc1_read_16bits(chan) >> (16 - ADC1_RES_BITS);
}

static void adc1_calibrate(void)
{
  uint16_t x;

  /* self clear bit */
  ADC1_SC3 |= ADC_SC3_CAL;

  /* while ((ADC1_SC1A & ADC_SC1_COCO) == 0) ; */
  while (ADC1_SC3 & ADC_SC3_CAL) ;

  /* note ADC1_SC3 & ADC_SC3_CALF if calibration failed */

  /* set plus side gain, page 619 */
  x = 0;
  x += ADC1_CLP0;
  x += ADC1_CLP1;
  x += ADC1_CLP2;
  x += ADC1_CLP3;
  x += ADC1_CLP4;
  x += ADC1_CLPS;
  ADC1_PG = (x >> 1) | (1 << 15);

  /* set minus side gain, page 619 */
  x = 0;
  x += ADC1_CLM0;
  x += ADC1_CLM1;
  x += ADC1_CLM2;
  x += ADC1_CLM3;
  x += ADC1_CLM4;
  x += ADC1_CLMS;
  ADC1_MG = (x >> 1) | (1 << 15);
}

static void adc1_setup(void)
{
  /* 16 bits single ended mode SAR ADC */

#if (ADC1_USE_INTERNAL_VREF == 1)

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

#endif /* ADC1_USE_INTERNAL_VREF */

  /* enable ADC1 module */
  /* note: no SIM_SCGC6_ADC1 */
  SIM_SCGC6 |= SIM_SCGC6_ADC0;

  /* ADC1_SC1A, status control register */
  /* AIEN: interrupt disabled */
  /* DIFF: single ended mode */
  /* ADCH: selected channel, 0x1f module disabled */
  ADC1_SC1A = 0x1f;

  /* the alternate clock is connected to OSCERCLK, 16mhz */
  /* datasheet say that ADC clock should be 2 to 12MHz for 16 bit mode */
  /* ADC1_CFG1, configuration register */
  /* ADLPC: low power configuration */
  /* ADIV: CLOCK / 8 */
  /* ADLSMP: long sample time enabled */
  /* MODE: single ended 16 bits conversion */
  /* ADICLK: bus clock / 2 */
  ADC1_CFG1 = ADC_CFG1_ADLSMP | ADC_CFG1_MODE(3) | ADC_CFG1_ADIV(1) | ADC_CFG1_ADICLK(1);

  /* ADC1_CFG2, configuration register */
  /* MUXSEL: ADxxa channels selected */
  /* ADACKEN: async clock disabled */
  /* ADHSC: high speed disabled */
  /* ADLSTS: longest sample time (24 ADCK cycles) */
  ADC1_CFG2 = ADC_CFG2_ADLSTS(2);

  /* ADC1_SC2, conversion, triggering */
  /* ADCTRG: software triggered conversion */
  /* ACFE: compare function disabled */
  /* ACFGT: greater than compare function disabled */
  /* ACREN: range compare function disabled */
  /* DMAEN: DMA disabled */
#if (ADC1_USE_INTERNAL_VREF == 1)
  /* REFSEL: voltage ref on alternate pair (internal) */
  ADC1_SC2 = 1;
#else
  /* REFSEL: voltage on external pair */
  ADC1_SC2 = 0;
#endif

  /* ADC1_SC3, status control register 3 */
  /* ADCO: continuous conversion disabled */
  /* AVGE: hardware averaging disabled */
  /* AVGS: averaged sample count, not applicable */
  ADC1_SC3 = 0;

  adc1_calibrate();
}


#endif /* ADC1_C_INCLUDED */
