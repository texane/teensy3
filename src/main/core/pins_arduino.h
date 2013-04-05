#ifndef pins_macros_for_arduino_compatibility_h
#define pins_macros_for_arduino_compatibility_h

#include <stdint.h>

#define A0	14
#define A1	15
#define A2	16
#define A3	17
#define A4	18
#define A5	19
#define A6	20
#define A7	21
#define A8	22
#define A9	23

#define A10	34
#define A11	35
#define A12	36
#define A13	37

#define SS	10
#define MOSI	11
#define MISO	12
#define SCK	13
#define LED_BUILTIN 13
#define SDA	18
#define SCL	19

#define NUM_DIGITAL_PINS 34
#define NUM_ANALOG_INPUTS 14

#define analogInputToDigitalPin(p) (((p) < 10) ? (p) + 14 : -1)
#define digitalPinHasPWM(p) (((p) >= 3 && (p) <= 6) || (p) == 9 || (p) == 10 || ((p) >= 20 && (p) <= 23))


struct digital_pin_bitband_and_config_table_struct {
        volatile uint32_t *reg;
        volatile uint32_t *config;
};
extern const struct digital_pin_bitband_and_config_table_struct digital_pin_to_info_PGM[];

// compatibility macros
#define digitalPinToPort(pin) (pin)
#define digitalPinToBitMask(pin) (1)
#define portOutputRegister(pin) ((volatile uint8_t *)(digital_pin_to_info_PGM[(pin)].reg + 0))
#define portSetRegister(pin)    ((volatile uint8_t *)(digital_pin_to_info_PGM[(pin)].reg + 32))
#define portClearRegister(pin)  ((volatile uint8_t *)(digital_pin_to_info_PGM[(pin)].reg + 64))
#define portToggleRegister(pin) ((volatile uint8_t *)(digital_pin_to_info_PGM[(pin)].reg + 96))
#define portInputRegister(pin)  ((volatile uint8_t *)(digital_pin_to_info_PGM[(pin)].reg + 128))
#define portModeRegister(pin)   ((volatile uint8_t *)(digital_pin_to_info_PGM[(pin)].reg + 160))
#define portConfigRegister(pin) ((volatile uint32_t *)(digital_pin_to_info_PGM[(pin)].config))

#define NOT_ON_TIMER 0
static inline uint8_t digitalPinToTimer(uint8_t) __attribute__((always_inline, unused));
static inline uint8_t digitalPinToTimer(uint8_t pin)
{
	if (pin >= 3 && pin <= 6) return pin - 2;
	if (pin >= 9 && pin <= 10) return pin - 4;
	if (pin >= 20 && pin <= 23) return pin - 13;
	return NOT_ON_TIMER;
}




#endif
