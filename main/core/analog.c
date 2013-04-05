#include "core_pins.h"
//#include "HardwareSerial.h"

static uint8_t calibrating;

void analog_init(void)
{
	VREF_TRM = 0x60;
	VREF_SC = 0xE1;		// enable 1.2 volt ref

	// SIM_SCGC6 |= SIM_SCGC6_ADC0;
#if F_BUS == 48000000
	ADC0_CFG1 = ADC_CFG1_ADIV(3) + ADC_CFG1_ADLSMP + ADC_CFG1_MODE(3) + ADC_CFG1_ADICLK(1);
#elif F_BUS == 24000000
	ADC0_CFG1 = ADC_CFG1_ADIV(3) + ADC_CFG1_ADLSMP + ADC_CFG1_MODE(3) + ADC_CFG1_ADICLK(0);
#else
#error
#endif
	//return;
	ADC0_CFG2 = ADC_CFG2_MUXSEL + ADC_CFG2_ADLSTS(1);

	ADC0_SC2 = ADC_SC2_REFSEL(0); // vcc/ext ref

	//ADC0_SC2 = ADC_SC2_REFSEL(1); // 1.2V ref

	ADC0_SC3 = ADC_SC3_AVGE + ADC_SC3_AVGS(3); // avg 32 samples
	ADC0_SC3 = ADC_SC3_AVGE + ADC_SC3_AVGS(0); // avg 4 samples
	ADC0_SC3 = 0;  // no averaging
	//calibrating = 0;
	//return;
	ADC0_SC3 = ADC_SC3_CAL + ADC_SC3_AVGE + ADC_SC3_AVGS(3); // begin cal
	calibrating = 1;
}

static void wait_for_cal(void)
{
	uint16_t sum;

	//serial_print("wait_for_cal\n");
	while (ADC0_SC3 & ADC_SC3_CAL) {
		// wait
		//serial_print(".");
	}
	//serial_print("\n");
	sum = ADC0_CLPS + ADC0_CLP4 + ADC0_CLP3 + ADC0_CLP2 + ADC0_CLP1 + ADC0_CLP0;
	sum = (sum / 2) | 0x8000;
	ADC0_PG = sum;
	//serial_print("ADC0_PG = ");
	//serial_phex16(sum);
	//serial_print("\n");
	sum = ADC0_CLMS + ADC0_CLM4 + ADC0_CLM3 + ADC0_CLM2 + ADC0_CLM1 + ADC0_CLM0;
	sum = (sum / 2) | 0x8000;
	ADC0_MG = sum;
	//serial_print("ADC0_MG = ");
	//serial_phex16(sum);
	//serial_print("\n");
	calibrating = 0;
}

// ADCx_SC2[REFSEL] bit selects the voltage reference sources for ADC.
//   VREFH/VREFL - connected as the primary reference option
//   1.2 V VREF_OUT - connected as the VALT reference option

// the alternate clock is connected to OSCERCLK (16 MHz).

// datasheet says ADC clock should be 2 to 12 MHz

void analogReference(uint8_t type)
{
	if (calibrating) wait_for_cal();

	// TODO: implement this
}

static uint8_t analog_right_shift = 6;

void analogReadRes(unsigned int bits)
{
	if (bits > 16) bits = 16;
	analog_right_shift = 16 - bits;
	// TODO: actually reconfigure A/D for desired resolution
}

// The SC1A register is used for both software and hardware trigger modes of operation.


static const uint8_t channel2sc1a[] = {
	5, 14, 8, 9, 13, 12, 6, 7, 15, 4,
	0, 19, 3, 21, 26, 22
};

int analogRead(uint8_t pin)
{
	int result;

	if (pin >= 14) {
		if (pin <= 23) {
			pin -= 14;  // 14-23 are A0-A9
		} else if (pin >= 34 && pin <= 39) {
			pin -= 24;  // 34-37 are A10-A13, 38 is temp sensor, 39 is vref
		} else {
			return 0;   // all others are invalid
		}
	}
	//serial_print("analogRead");
	//return 0;
	if (calibrating) wait_for_cal();
	//pin = 5; // PTD1/SE5b, pin 14, analog 0

	ADC0_SC1A = channel2sc1a[pin];
	while ((ADC0_SC1A & ADC_SC1_COCO) == 0) {
		// wait
		//serial_print(".");
	}
	//serial_print("\n");
	result = ADC0_RA >> analog_right_shift;
	//serial_phex16(result >> 3);
	//serial_print("\n");
	return result;
}






















