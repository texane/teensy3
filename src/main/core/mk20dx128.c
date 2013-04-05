#include "mk20dx128.h"


extern unsigned long _stext;
extern unsigned long _etext;
extern unsigned long _sdata;
extern unsigned long _edata;
extern unsigned long _sbss;
extern unsigned long _ebss;
extern unsigned long _estack;
//extern void __init_array_start(void);
//extern void __init_array_end(void);
extern int main (void);
void ResetHandler(void);

void usb_isr(void);
void systick_isr(void);
void pdb_isr(void);
void pit3_isr(void);

void porta_isr(void);
void portb_isr(void);
void portc_isr(void);
void portd_isr(void);
void porte_isr(void);

void uart0_status_isr(void);
void uart1_status_isr(void);
void uart2_status_isr(void);

void fault_isr(void);
void unused_isr(void);


// TODO: create AVR-stype ISR() macro, with default linkage to undefined handler
//
__attribute__ ((section(".vectors"), used))
void (* const gVectors[])(void) =
{
        (void (*)(void))((unsigned long)&_estack),	//  0 ARM: Initial Stack Pointer
        ResetHandler,					//  1 ARM: Initial Program Counter
	fault_isr,					//  2 ARM: Non-maskable Interrupt (NMI)
	fault_isr,					//  3 ARM: Hard Fault
	fault_isr,					//  4 ARM: MemManage Fault
	fault_isr,					//  5 ARM: Bus Fault
	fault_isr,					//  6 ARM: Usage Fault
	fault_isr,					//  7 --
	fault_isr,					//  8 --
	fault_isr,					//  9 --
	fault_isr,					// 10 --
	fault_isr,					// 11 ARM: Supervisor call (SVCall)
	fault_isr,					// 12 ARM: Debug Monitor
	fault_isr,					// 13 --
	fault_isr,					// 14 ARM: Pendable req serv(PendableSrvReq)
	systick_isr,					// 15 ARM: System tick timer (SysTick)
	unused_isr,					// 16 DMA channel 0 transfer complete
	unused_isr,					// 17 DMA channel 1 transfer complete
	unused_isr,					// 18 DMA channel 2 transfer complete
	unused_isr,					// 19 DMA channel 3 transfer complete
	unused_isr,					// 20 DMA error interrupt channel
	unused_isr,					// 21 DMA --
	unused_isr,					// 22 Flash Memory Command complete
	unused_isr,					// 23 Read collision
	unused_isr,					// 24 Low-voltage detect/warning
	unused_isr,					// 25 Low Leakage Wakeup
	unused_isr,					// 26 Both EWM and WDOG interrupt
	unused_isr,					// 27 I2C
	unused_isr,					// 28 SPI
	unused_isr,					// 29 I2S Transmit
	unused_isr,					// 30 I2S Receive
	unused_isr,					// 31 UART0 CEA709.1-B (LON) status
	uart0_status_isr,				// 32 UART0 status
	unused_isr,					// 33 UART0 error
	uart1_status_isr,				// 34 UART1 status
	unused_isr,					// 35 UART1 error
	uart2_status_isr,				// 36 UART2 status
	unused_isr,					// 37 UART2 error
	unused_isr,					// 38 ADC0
	unused_isr,					// 39 CMP0
	unused_isr,					// 40 CMP1
	unused_isr,					// 41 FTM0
	unused_isr,					// 42 FTM1
	unused_isr,					// 43 CMT
	unused_isr,					// 44 RTC Alarm interrupt
	unused_isr,					// 45 RTC Seconds interrupt
	unused_isr,					// 46 PIT Channel 0
	unused_isr,					// 47 PIT Channel 1
	unused_isr,					// 48 PIT Channel 2
	pit3_isr,					// 49 PIT Channel 3
	pdb_isr,					// 50 PDB Programmable Delay Block
	usb_isr,					// 51 USB OTG
	unused_isr,					// 52 USB Charger Detect
	unused_isr,					// 53 TSI
	unused_isr,					// 54 MCG
	unused_isr,					// 55 Low Power Timer
	porta_isr,					// 56 Pin detect (Port A)
	portb_isr,					// 57 Pin detect (Port B)
	portc_isr,					// 58 Pin detect (Port C)
	portd_isr,					// 59 Pin detect (Port D)
	porte_isr,					// 60 Pin detect (Port E)
	unused_isr,					// 61 Software interrupt
};

void fault_isr(void)
{
        while (1); // die
}

void unused_isr(void)
{
        while (1); // die
}

//void usb_isr(void)
//{
//}

__attribute__ ((section(".flashconfig"), used))
const uint8_t flashconfigbytes[16] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF
};


// Automatically initialize the RTC.  When the build defines the compile
// time, and the user has added a crystal, the RTC will automatically
// begin at the time of the first upload.
#ifndef TIME_T
#define TIME_T 1349049600 // default 1 Oct 2012
#endif
extern void rtc_set(unsigned long t);


__attribute__ ((section(".startup")))
void ResetHandler(void)
{
        uint32_t *src = &_etext;
        uint32_t *dest = &_sdata;
	//void (* ptr)(void);

	WDOG_UNLOCK = WDOG_UNLOCK_SEQ1;
	WDOG_UNLOCK = WDOG_UNLOCK_SEQ2;
	WDOG_STCTRLH = WDOG_STCTRLH_ALLOWUPDATE;

	// enable clocks to always-used peripherals
	SIM_SCGC5 = 0x00043F82;		// clocks active to all GPIO
	SIM_SCGC6 = SIM_SCGC6_RTC | SIM_SCGC6_FTM0 | SIM_SCGC6_FTM1 | SIM_SCGC6_ADC0 | SIM_SCGC6_FTFL;
	// if the RTC oscillator isn't enabled, get it started early
	if (!(RTC_CR & RTC_CR_OSCE)) {
		RTC_SR = 0;
		RTC_CR = RTC_CR_SC16P | RTC_CR_SC4P | RTC_CR_OSCE;
	}

	// TODO: do this while the PLL is waiting to lock....
        while (dest < &_edata) *dest++ = *src++;
        dest = &_sbss;
        while (dest < &_ebss) *dest++ = 0;
	SCB_VTOR = 0;	// use vector table in flash

        // start in FEI mode
        // enable capacitors for crystal
        OSC0_CR = OSC_SC8P | OSC_SC2P;
        // enable osc, 8-32 MHz range, low power mode
        MCG_C2 = MCG_C2_RANGE0(2) | MCG_C2_EREFS;
        // switch to crystal as clock source, FLL input = 16 MHz / 512
        MCG_C1 =  MCG_C1_CLKS(2) | MCG_C1_FRDIV(4);
        // wait for crystal oscillator to begin
        while ((MCG_S & MCG_S_OSCINIT0) == 0) ;
        // wait for FLL to use oscillator
        while ((MCG_S & MCG_S_IREFST) != 0) ;
        // wait for MCGOUT to use oscillator
        while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST(2)) ;
        // now we're in FBE mode
        // config PLL input for 16 MHz Crystal / 4 = 4 MHz
        MCG_C5 = MCG_C5_PRDIV0(3);
        // config PLL for 96 MHz output
        MCG_C6 = MCG_C6_PLLS | MCG_C6_VDIV0(0);
        // wait for PLL to start using xtal as its input
        while (!(MCG_S & MCG_S_PLLST)) ;
        // wait for PLL to lock
        while (!(MCG_S & MCG_S_LOCK0)) ;
        // now we're in PBE mode
#if F_CPU == 96000000
        // config divisors: 96 MHz core, 48 MHz bus, 24 MHz flash
        SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(0) | SIM_CLKDIV1_OUTDIV2(1) |  SIM_CLKDIV1_OUTDIV4(3);
#elif F_CPU == 48000000
        // config divisors: 48 MHz core, 48 MHz bus, 24 MHz flash
        SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(1) | SIM_CLKDIV1_OUTDIV2(1) |  SIM_CLKDIV1_OUTDIV4(3);
#elif F_CPU == 24000000
        // config divisors: 24 MHz core, 24 MHz bus, 24 MHz flash
        SIM_CLKDIV1 = SIM_CLKDIV1_OUTDIV1(3) | SIM_CLKDIV1_OUTDIV2(3) |  SIM_CLKDIV1_OUTDIV4(3);
#else
#error "Error, F_CPU must be 96000000, 48000000, or 24000000"
#endif
        // switch to PLL as clock source, FLL input = 16 MHz / 512
        MCG_C1 = MCG_C1_CLKS(0) | MCG_C1_FRDIV(4);
        // wait for PLL clock to be used
        while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST(3)) ;
        // now we're in PEE mode
        // configure USB for 48 MHz clock
        SIM_CLKDIV2 = SIM_CLKDIV2_USBDIV(1); // USB = 96 MHz PLL / 2
        // USB uses PLL clock, trace is CPU clock, CLKOUT=OSCERCLK0
        SIM_SOPT2 = SIM_SOPT2_USBSRC | SIM_SOPT2_PLLFLLSEL | SIM_SOPT2_TRACECLKSEL | SIM_SOPT2_CLKOUTSEL(6);

        // initialize the SysTick counter
        SYST_RVR = (F_CPU / 1000) - 1;
        SYST_CSR = SYST_CSR_CLKSOURCE | SYST_CSR_TICKINT | SYST_CSR_ENABLE;

	//init_pins();
	__enable_irq();

	_init_Teensyduino_internal_();
	if (RTC_SR & RTC_SR_TIF) rtc_set(TIME_T);

	__libc_init_array();

/*
	for (ptr = &__init_array_start; ptr < &__init_array_end; ptr++) {
		(*ptr)();
	}
*/

        main();
        while (1) ;
}

// TODO: is this needed for c++ and where does it come from?
/*
void _init(void)
{
}
*/


void * _sbrk(int incr)
{
        static char *heap_end = (char *)&_ebss;
	char *prev = heap_end;

	heap_end += incr;
	return prev;
}

int _read(int file, char *ptr, int len)
{
	return 0;
}

int _write(int file, char *ptr, int len)
{
	return 0;
}

void __cxa_pure_virtual()
{
	while (1);
}



