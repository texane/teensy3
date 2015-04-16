#ifndef SPI_C_INCLUDED
#define SPI_C_INCLUDED


#include <stdint.h>
#include "mk20dx128.h"


#define CONFIG_USE_DSPI 0

#if (CONFIG_USE_DSPI == 1)

/* dspi, chapter 43 */

static inline void spi_halt(void)
{
  SPI0_MCR |= SPI_MCR_HALT;
}

static inline void spi_start(void)
{
  SPI0_MCR &= ~SPI_MCR_HALT;
}

static inline unsigned int spi_is_running(void)
{
#ifndef SPI_SR_TXRXS
#define SPI_SR_TXRXS ((uint32_t)0x40000000)
#endif
  return SPI0_SR & SPI_SR_TXRXS;
}

static void spi_setup_master(void)
{
  /* configure inout ports */
  /* chapters 10, 11, 12 */

  /* enable module clocking */
  if ((SIM_SCGC6 & SIM_SCGC6_SPI0) == 0) SIM_SCGC6 |= SIM_SCGC6_SPI0;

  /* PORTD, slave select pin */
#if 0
  /* PTD0, SPI0_CS0, ALT2 */
  PORTD_PCR0 = 2 << 8;
#else
  PORTD_PCR0 = (1 << 8) | (1 << 6) | (1 << 2);
  GPIOD_PDDR |= 1 << 0;
  GPIOD_PDOR |= 1 << 0;
#endif
  /* PTD1, SPI0_SCK, ALT2 */
  PORTD_PCR1 = 2 << 8;
  /* PTD2, SPI0_SOUT, ALT2, internall pullup */
  PORTD_PCR2 = (2 << 8) | 3;
  /* PTD3, SPI0_SIN, ALT2 */
  PORTD_PCR3 = 2 << 8;

  /* SPI0_MCR, module configuration register */
  /* MSTR: enable master */
  /* CONT_SCKE: continuous SCK disabled */
  /* DCONF: SPI mode */
  /* FRZ: freeze disabled */
  /* MTFE: modified SPI transfer format disabled */
  /* ROOE: ignore incoming data on overflow */
  /* PCSIS: active low */
  /* DOZE: disabled */
  /* MDIS: DSPI clocks disabled */
  /* DIS_TXF: transmit fifo disabled */
  /* DIS_RXF: receive fifo disabled */

  /* disable MDIS first to disable FIFOs */
  SPI0_MCR = SPI_MCR_MDIS;

  SPI0_MCR = SPI_MCR_MSTR | SPI_MCR_MDIS | SPI_MCR_DIS_TXF | SPI_MCR_DIS_RXF | SPI_MCR_HALT;
  while (spi_is_running()) ;

  /* SPI0_CTAR0, clock and transfer attributes register */
  /* DBR: double baud rate, 50 / 50 */
  /* FMSZ: frame size set to 7 + 1 */
  /* CPOL: inactive when sck low */
  /* CPHA: data sampled on leading edge */
  /* LSBFE: MSB first */
  /* PCSSCK: PCS activation prescaler set to 1 */
  /* PASC: PCS deactivation prescaler set to 1 */
  /* PDT: delay after transfer prescaler set to 1 */
  /* PBR: baud rate prescaler set to 1 */
  /* CSSCK: PCS to SCK delay scaler set to 2 */
  /* ASC: after delay scaler set to 0 */
  /* DT: delay after transfer scaler set to 0 */
  /* BR: baud rate scaler set to 128 (ie. 375kHz) */
  SPI0_CTAR0 = (7 << 27) | 8;

  /* SPI0_RSER, DMA and interrupt request select and enable register */
  /* all disabled */
  SPI0_RSER = 0;

  /* enable DSPI clocks */
  SPI0_MCR &= ~(SPI_MCR_MDIS | SPI_MCR_HALT);
}

static inline void spi_write_uint8(uint8_t x)
{
  /* tx fifo disabled */

  /* SPI0_PUSHR */
  /* CONT: PCS to inactive between transfers */
  /* CTAR: TAR0 selected */
  /* EOQ: last data */
  /* CTCNT: do not clear counter */
  /* PCS: select cs0 (not applicable if hand controlled) */
  /* TXDATA: data */

  SPI0_PUSHR = x;
}

static inline void spi_write_uint16(uint16_t x)
{
  spi_write_uint8((x >> 8) & 0xff);
  spi_write_uint8((x >> 0) & 0xff);
}

static inline uint8_t spi_read_uint8(void)
{
  /* rx fifo disabled */
  spi_write_uint8(0xff);
  while (((SPI0_SR >> 4) & 7) == 0) ;
  return SPI0_POPR;
}

#else /* CONFIG_USE_SOFTSPI */

/* PTD0, SPI_CS_BAR */
/* PTD1, SPI_SCK */
/* PTD2, SPI_MISO */
/* PTD3, SPI_MOSI */

#define SOFTSPI_DIR_REG GPIOD_PDDR
#define SOFTSPI_INPUT_REG GPIOD_PDIR
#define SOFTSPI_OUTPUT_REG GPIOD_PDOR
#define SOFTSPI_CS_MASK (1 << 0)
#define SOFTSPI_SCK_MASK (1 << 1)
#define SOFTSPI_MISO_MASK (1 << 2)
#define SOFTSPI_MOSI_MASK (1 << 3)

static void softspi_setup_master(void)
{
  SOFTSPI_DIR_REG |= SOFTSPI_CS_MASK;
  PORTD_PCR0 = (1 << 8) | (1 << 6) | (1 << 2);
  SOFTSPI_OUTPUT_REG |= SOFTSPI_CS_MASK;

  /* sck */
  SOFTSPI_DIR_REG |= SOFTSPI_SCK_MASK;
  PORTD_PCR1 = (1 << 8) | (1 << 6) | (1 << 2);
  SOFTSPI_OUTPUT_REG &= ~SOFTSPI_SCK_MASK;

  /* mosi */
  SOFTSPI_DIR_REG |= SOFTSPI_MOSI_MASK;
  PORTD_PCR3 = (1 << 8) | (1 << 6) | (1 << 2);
  SOFTSPI_OUTPUT_REG &= ~SOFTSPI_MOSI_MASK;

  /* miso, internal pullup enabled */
  SOFTSPI_OUTPUT_REG &= ~SOFTSPI_MISO_MASK;
  SOFTSPI_DIR_REG &= ~SOFTSPI_MISO_MASK;
  PORTD_PCR2 = (1 << 8) | 3;
}

static inline void softspi_clk_low(void)
{
  SOFTSPI_OUTPUT_REG &= ~SOFTSPI_SCK_MASK;
}

static inline void softspi_clk_high(void)
{
  SOFTSPI_OUTPUT_REG |= SOFTSPI_SCK_MASK;
}

static inline void softspi_mosi_low(void)
{
  SOFTSPI_OUTPUT_REG &= ~SOFTSPI_MOSI_MASK;
}

static inline void softspi_mosi_high(void)
{
  SOFTSPI_OUTPUT_REG |= SOFTSPI_MOSI_MASK;
}


static void softspi_wait_clk(void)
{
  volatile unsigned int i;
  for (i = 0; i < 100; ++i) __asm__ __volatile__ ("nop\r\n");
}

static void softspi_wait_after_clk(void)
{
  volatile unsigned int i;
  for (i = 0; i < 100; ++i) __asm__ __volatile__ ("nop\r\n");
}

static inline void softspi_write_bit(uint8_t x, uint8_t m)
{
  /* 5 insns per bit */

  if (x & m) softspi_mosi_high(); else softspi_mosi_low();

  /* mode0: data sampled at rising edge, propagated at falling edge */

  softspi_clk_low();
  softspi_wait_clk();
  softspi_clk_high();

  softspi_wait_after_clk();
}

static inline void softspi_write_uint8(uint8_t x)
{
  /* transmit msb first, sample at clock falling edge */

  softspi_write_bit(x, 1 << 7);
  softspi_write_bit(x, 1 << 6);
  softspi_write_bit(x, 1 << 5);
  softspi_write_bit(x, 1 << 4);
  softspi_write_bit(x, 1 << 3);
  softspi_write_bit(x, 1 << 2);
  softspi_write_bit(x, 1 << 1);
  softspi_write_bit(x, 1 << 0);
}

static inline void softspi_write_uint16(uint16_t x)
{
  softspi_write_uint8((x >> 8) & 0xff);
  softspi_write_uint8((x >> 0) & 0xff);
}

static inline void softspi_read_bit(uint8_t* x, uint8_t m)
{
  /* mode0: data sampled at rising edge, propagated at falling edge */

  softspi_clk_low();
  softspi_wait_clk();
  softspi_clk_high();

  softspi_wait_after_clk();
  if (SOFTSPI_INPUT_REG & SOFTSPI_MISO_MASK) *x |= m;
}

static inline uint8_t softspi_read_uint8(void)
{
  /* receive msb first, sample at clock falling edge */

  /* must be initialized to 0 */
  uint8_t x = 0;

  softspi_read_bit(&x, 1 << 7);
  softspi_read_bit(&x, 1 << 6);
  softspi_read_bit(&x, 1 << 5);
  softspi_read_bit(&x, 1 << 4);
  softspi_read_bit(&x, 1 << 3);
  softspi_read_bit(&x, 1 << 2);
  softspi_read_bit(&x, 1 << 1);
  softspi_read_bit(&x, 1 << 0);

  return x;
}

static void spi_setup_master(void)
{
  softspi_setup_master();
}

static inline void spi_write_uint8(uint8_t x)
{
  softspi_write_uint8(x);
}

static inline void spi_write_uint16(uint16_t x)
{
  softspi_write_uint16(x);
}

static inline uint8_t spi_read_uint8(void)
{
  return softspi_read_uint8();
}

#endif /* CONFIG_USE_DSPI == 1 */

static void spi_write(const uint8_t* s, unsigned int len)
{
  for (; len; --len, ++s) spi_write_uint8(*s);
}

static void spi_read(uint8_t* s, unsigned int len)
{
  for (; len; --len, ++s) *s = spi_read_uint8();
}

static inline void spi_read_512(uint8_t* s)
{
  return spi_read(s, 512);
}

static inline void spi_write_512(const uint8_t* s)
{
  return spi_write(s, 512);
}


#endif /* SPI_C_INCLUDED */
