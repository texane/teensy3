#include <stdint.h>
#include "mk20dx128.h"
#include "serial.c"
#include "led.c"


extern void delay(uint32_t);


int main(void)
{
  serial_setup();
  led_setup();

  while (1)
  {
    SERIAL_WRITE_STRING("x\r\n");
    led_set_val(1);
    delay(250);

    SERIAL_WRITE_STRING("y\r\n");
    led_set_val(0);
    delay(250);
  }

  return 0;
}
