#ifndef LCD_C_INCLUDED
#define LCD_C_INCLUDED


#include <stdint.h>
#include "mk20dx128.h"


/* lcd */

/* note: lcd model MC21605A6W */
/* note: DB0:3 and RW must be grounded */
/* db4: ptb0 */
/* db5: ptb1 */
/* db6: ptb3 */
/* db7: ptb2 */
/* en: ptd5 */
/* rs: ptd6 */

#define LCD_POS_DB 0x00
#define LCD_PORT_DB GPIOB_PDOR
#define LCD_DIR_DB GPIOB_PDDR
#define LCD_MASK_DB (0x0f << LCD_POS_DB)

#define LCD_POS_EN 0x05
#define LCD_PORT_EN GPIOD_PDOR
#define LCD_DIR_EN GPIOD_PDDR
#define LCD_MASK_EN (0x01 << LCD_POS_EN)

#define LCD_POS_RS 0x06
#define LCD_PORT_RS GPIOD_PDOR
#define LCD_DIR_RS GPIOD_PDDR
#define LCD_MASK_RS (0x01 << LCD_POS_RS)

#define CONFIG_CORTEX_M4 1
#if (CONFIG_CORTEX_M4 == 1)
#define LCD_PCR_DB4 PORTB_PCR0
#define LCD_PCR_DB5 PORTB_PCR1
#define LCD_PCR_DB6 PORTB_PCR3
#define LCD_PCR_DB7 PORTB_PCR2
#define LCD_PCR_EN PORTD_PCR5
#define LCD_PCR_RS PORTD_PCR6
#endif /* CONFIG_CORTEX_M4 */


#if (CONFIG_CORTEX_M4 == 1)

static inline void wait_50_ns(void)
{
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
}

static inline void wait_500_ns(void)
{
  /* 24 cycles at 48mhz */
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
}

static inline void wait_50_us(void)
{
  uint8_t x;
  for (x = 0; x < 100; ++x) wait_500_ns();
}

#else /* CONFIG_ATMEGA328P */

static inline void wait_50_ns(void)
{
  __asm__ __volatile__ ("nop\n\t");
}

static inline void wait_500_ns(void)
{
  /* 8 cycles at 16mhz */
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
  __asm__ __volatile__ ("nop\n\t");
}

static inline void wait_50_us(void)
{
  /* 800 cycles at 16mhz */
  uint8_t x;
  for (x = 0; x < 100; ++x) wait_500_ns();
}

#endif /* CONFIG_CORTEX_M4 */

static inline void wait_2_ms(void)
{
  wait_50_us();
  wait_50_us();
  wait_50_us();
  wait_50_us();
}

static inline void wait_50_ms(void)
{
  /* FIXME: was _delay_ms(50), but not working */
  uint8_t x;
  for (x = 0; x < 25; ++x) wait_2_ms();
}

static inline void lcd_pulse_en(void)
{
  /* assume EN low */
  LCD_PORT_EN |= LCD_MASK_EN;
  wait_50_us();
  LCD_PORT_EN &= ~LCD_MASK_EN;
  wait_2_ms();
}

#if (CONFIG_CORTEX_M4 == 1)
/* swap the last nibble 2 last bits because of error in routing */
static uint8_t swap_nibble_last_2_bits(uint8_t x)
{
  const uint8_t mask = ((x >> 1) & 4) | ((x << 1) & 8);
  return (x & 3) | mask;
}
#else
static inline uint8_t swap_nibble_last_2_bits(uint8_t x)
{
  return x;
}
#endif /* CONFIG_CORTEX_M4 */

static inline void lcd_write_db4(uint8_t x)
{
  /* configured in 4 bits mode */

  LCD_PORT_DB &= ~LCD_MASK_DB;
  LCD_PORT_DB |= swap_nibble_last_2_bits(x >> 4) << LCD_POS_DB;
  lcd_pulse_en();

  LCD_PORT_DB &= ~LCD_MASK_DB;
  LCD_PORT_DB |= swap_nibble_last_2_bits(x & 0xf) << LCD_POS_DB;
  lcd_pulse_en();
}

static inline void lcd_write_db8(uint8_t x)
{
  /* configured in 8 bits mode */

  /* only hi nibble transmitted, (0:3) grounded */
  LCD_PORT_DB &= ~LCD_MASK_DB;
  LCD_PORT_DB |= swap_nibble_last_2_bits(x >> 4) << LCD_POS_DB;
  lcd_pulse_en();
}

static inline void lcd_setup(void)
{
#if (CONFIG_CORTEX_M4 == 1)
  LCD_PCR_DB4 = (1 << 8) | (1 << 6) | (1 << 2);
  LCD_PCR_DB5 = (1 << 8) | (1 << 6) | (1 << 2);
  LCD_PCR_DB6 = (1 << 8) | (1 << 6) | (1 << 2);
  LCD_PCR_DB7 = (1 << 8) | (1 << 6) | (1 << 2);
  LCD_PCR_RS = (1 << 8) | (1 << 6) | (1 << 2);
  LCD_PCR_EN = (1 << 8) | (1 << 6) | (1 << 2);
#endif /* CONFIG_CORTEX_M4 */

  LCD_DIR_DB |= LCD_MASK_DB;
  LCD_DIR_RS |= LCD_MASK_RS;
  LCD_DIR_EN |= LCD_MASK_EN;

  LCD_PORT_DB &= ~LCD_MASK_DB;
  LCD_PORT_RS &= ~LCD_MASK_RS;
  LCD_PORT_EN &= ~LCD_MASK_EN;

  /* small delay for the lcd to boot */
  wait_50_ms();

  /* datasheet init sequence */

#define LCD_MODE_BLINK (1 << 0)
#define LCD_MODE_CURSOR (1 << 1)
#define LCD_MODE_DISPLAY (1 << 2)

  lcd_write_db8(0x30);
  wait_2_ms();
  wait_2_ms();
  wait_500_ns();

  lcd_write_db8(0x30);
  wait_2_ms();

  lcd_write_db4(0x32);
  wait_2_ms();

  lcd_write_db4(0x28);
  wait_2_ms();

  lcd_write_db4((1 << 3) | LCD_MODE_DISPLAY);
  wait_2_ms();

  lcd_write_db4(0x01);
  wait_2_ms();

  lcd_write_db4(0x0f);
  wait_2_ms();
}

static inline void lcd_clear(void)
{
  /* clear lcd */
  lcd_write_db4(0x01);
  wait_2_ms();
}

static inline void lcd_home(void)
{
  /* set cursor to home */
  lcd_write_db4(0x02);
  wait_2_ms();
}

static inline void lcd_set_ddram(uint8_t addr)
{
  lcd_write_db4((1 << 7) | addr);
  wait_2_ms();
}

static inline void lcd_goto_xy(uint8_t x, uint8_t y)
{
  /* assume 0 <= x < 8 */
  /* assume 0 <= y < 2 */

  /* from datasheet: */
  /* first line is 0x00 to 0x27 */
  /* second line is 0x40 to 0x67 */
  static const uint8_t row[] = { 0x00, 0x40 };
  lcd_set_ddram(row[y] | x);
}

static void lcd_write(const uint8_t* s, unsigned int n)
{
  wait_50_ns();

  LCD_PORT_RS |= LCD_MASK_RS;
  for (; n; --n, ++s)
  {
    lcd_write_db4(*s);
    wait_2_ms();
  }
  LCD_PORT_RS &= ~LCD_MASK_RS;
}


#endif /* LCD_C_INCLUDED */
