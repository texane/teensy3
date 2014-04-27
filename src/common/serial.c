#ifndef SERIAL_C_INCLUDED
#define SERIAL_C_INCLUDED


#include <stdint.h>
#include "mk20dx128.h"


/* serial */

static inline uint8_t nibble(uint32_t x, uint8_t i)
{
  return (x >> (i * 4)) & 0xf;
}

static inline uint8_t hex(uint8_t x)
{
  return (x >= 0xa) ? 'a' + x - 0xa : '0' + x;
}

__attribute__((unused))
static uint8_t* uint8_to_string(uint8_t x)
{
  static uint8_t buf[2];

  buf[1] = hex(nibble(x, 0));
  buf[0] = hex(nibble(x, 1));

  return buf;
}

__attribute__((unused))
static uint8_t* uint16_to_string(uint16_t x)
{
  static uint8_t buf[4];

  buf[3] = hex(nibble(x, 0));
  buf[2] = hex(nibble(x, 1));
  buf[1] = hex(nibble(x, 2));
  buf[0] = hex(nibble(x, 3));

  return buf;
}

#ifdef USB_SERIAL
extern int usb_serial_putchar(uint8_t);
extern int usb_serial_getchar(void);
#else
extern void serial_begin(uint32_t);
extern void serial_putchar(uint8_t);
#endif

static void serial_setup(void)
{
#ifdef USB_SERIAL
#else
#define BAUD2DIV(baud) (((F_CPU * 2) + ((baud) >> 1)) / (baud))
  static uint32_t div = BAUD2DIV(9600);
  serial_begin(div);
#endif
}

static void serial_write(const uint8_t* s, unsigned int n)
{
  for (; n; --n, ++s)
  {
#ifdef USB_SERIAL
    usb_serial_putchar(*s);
#else
    serial_putchar(*s);
#endif
  }
}

static void serial_write_string(const char* s)
{
  unsigned int n;
  for (n = 0; s[n]; ++n) ;
  serial_write((const uint8_t*)s, n);
}

static void serial_write_hex(const uint8_t* s, unsigned int n)
{
  for (; n; --n, ++s) serial_write(uint8_to_string(*s), 2);
}

#define SERIAL_WRITE_STRING(__s) serial_write((void*)__s, sizeof(__s) - 1)

static uint8_t serial_read_uint8(void)
{
  return (uint8_t)usb_serial_getchar();
}

extern int usb_serial_available(void);
static uint32_t serial_get_rsize(void)
{
  return (uint32_t)usb_serial_available();
}

extern int usb_serial_read(void*, uint32_t);
static int serial_read(uint8_t* p, uint32_t n)
{
  return usb_serial_read(p, n);
}


#endif /* SERIAL_C_INCLUDED */
