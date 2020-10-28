/*
*  uChip Simple VGA Console Kernel, a minimalistic console with VGA and USB support using uChip!
*
*  Copyright 2019-2020 Nicola Wrachien (next-hack.com)
*
*  This file is part of uChip Simple VGA Console Kernel Library.
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program  is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*  tl;dr
*  Do whatever you want, this program is free! Though we won't
*  reject donations https://next-hack.com/index.php/donate/ :)
*
*  system.c/h: all the hardware related initializations and functions are here.
*/
/* Note on SyncBusy... Implementing a "while(Register.SyncBusy);" each time we access a write-synch'ed register
  is NOT required! Writing on a register while syncchronization is ongoing, will make the bus stall, until the previous
  write operation has been synch'ed. That's it!
*/
#include "usvc_kernel.h"
#include "pff.h"
#include <string.h>
//#define EXT_CLK 7372800
#if USE_SYSTICK_TIMER 		// in the lite version, we use the SysTick
	static uint32_t	 m_millis = 0;
#endif
#define EXT_CLK 16000000

#if EXT_CLK == 7372800
	#define GCLK_DIVIDE_FACTOR 6
	#define CLOCK_PLL_MUL   (39U - 1)

#else // other integer values
	#define GCLK_DIVIDE_FACTOR (EXT_CLK / 1000000)
	#define 	CLOCK_PLL_MUL   (M_CLK / 1000000 - 1)
#endif
#define 	CLOCK_PLL_DIV   (1U) 
uint16_t f_millis16(void);
void* ramPointers[MAX_MODULE] __attribute__((used))  ={};
#if IS_BOOTLOADER
const uint32_t functionVector[]= 
{
	(uint32_t) &pf_mount, (uint32_t) &pf_open, (uint32_t) &pf_read, (uint32_t) &pf_write, (uint32_t) &pf_lseek, 
	(uint32_t) &pf_opendir, (uint32_t) &pf_readdir,		// FATFS module functions
	(uint32_t) &usbHostInit, (uint32_t)&usbHostTask, (uint32_t)&uhdPipe0Alloc, (uint32_t)&uhdPipeAlloc, 
	(uint32_t) &uhdPipeFree, (uint32_t) &usbCreateStandardRequest, (uint32_t)&usbAddTransaction, (uint32_t)&usbReleaseDevice, 
	(uint32_t) &usbGetTaskState,	(uint32_t)&usbStringDescriptor2Char, (uint32_t) &usbFindHidInterfaceAndEndpoint, // USB module functions
	(uint32_t) &usbHidBootKeyboardInstaller, (uint32_t) &usbHidBootKeyboardIsInstalled, (uint32_t) &usbGetKey,
	(uint32_t) &usbKeyboardPoll, (uint32_t) &usbGetCurrentKeyboardState, (uint32_t) &usbGetCurrentAsciiKeyboardState,
	(uint32_t) &usbGetCurrentAsciiKeyboardStateEx, (uint32_t) &usbSetKeyboardInstallationCompleteCallback, // USB Keyboard functions
	(uint32_t) &usbHidGenericGamepadInstaller, (uint32_t) &usbHidGenericGamepadIsInstalled, (uint32_t) &usbHidGenericGamepadPoll,
	(uint32_t) &getCurrentGamepadState, (uint32_t) &setGenericGamepadInstallationCompleteCallback	// USB gamepad functions
};
#else
	const uint32_t functionVector = 0;
#endif
void waitTillHasEnoughTime(uint16_t microSec)
{
	while (videoData.currentLineNumber < 401 || videoData.currentLineNumber >= 524 - (microSec >> 5));
}
// ContextSpecificFunctions: these functions depend on variables that are in ram. When from the game we call a bootloader library, the library must point to the functions declared in 
// the game app, and not in the bootloader, as the functions in the bootloader statically point to ram locations that are used for other purposes.
// In bootloader mode, the bootloader library should point to the functions declared in the bootloader.
const uint32_t contextSpecificFunctionVector[] = 
{
	(uint32_t) &f_millis16,
	(uint32_t) &waitTillHasEnoughTime,
	(uint32_t) &waitForVerticalBlank
};
static void externalClkInit(void)
{
	// enable clocks for the power, sysctrl and gclk modules 
	PM->APBAMASK.reg = (PM_APBAMASK_PM | PM_APBAMASK_SYSCTRL |PM_APBAMASK_GCLK);

	// setup generic clock 5 to feed DPLL. Use GCLK 5 input 
	setPortMux(11, PORT_PMUX_PMUXO_H_Val);

	// divide the input clock for the corresponding value		
	GCLK->GENDIV.reg = (GCLK_GENDIV_DIV(GCLK_DIVIDE_FACTOR) | 	GCLK_GENDIV_ID(5));
	GCLK->GENCTRL.reg = GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_GCLKIN | GCLK_GENCTRL_ID(5);
	GCLK->CLKCTRL.reg = (GCLK_CLKCTRL_GEN(5) | GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_FDPLL_Val) | GCLK_CLKCTRL_CLKEN);
	// enable PLL 
	SYSCTRL->DPLLRATIO.reg = (SYSCTRL_DPLLRATIO_LDR(CLOCK_PLL_MUL));
	SYSCTRL->DPLLCTRLB.reg = (SYSCTRL_DPLLCTRLB_REFCLK_GCLK) | SYSCTRL_DPLLCTRLB_FILTER(SYSCTRL_DPLLCTRLB_FILTER_DEFAULT_Val); 
	SYSCTRL->DPLLCTRLA.reg = (SYSCTRL_DPLLCTRLA_ENABLE);
	// The following wait is required, to be sure that the clock is ready
 	while(!(SYSCTRL->DPLLSTATUS.reg & (SYSCTRL_DPLLSTATUS_CLKRDY | SYSCTRL_DPLLSTATUS_LOCK)));
	//
	// select the PLL as source for clock generator 0 (CPU core clock) 
	GCLK->GENDIV.reg =  (GCLK_GENDIV_DIV(CLOCK_PLL_DIV) | GCLK_GENDIV_ID(0));
	GCLK->GENCTRL.reg = (GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_DPLL96M | GCLK_GENCTRL_ID(0)) | GCLK_GENCTRL_OOV;		
	//
}
// In game mode we do not use systick timer because it is 24 bit only, so we need to use an interrupt to count milliseconds.
// instead TC4 is coupled to form a 32 bit timer, so we just need to get its register, without having to call an interrupt each ms.
// however, setting up the timer counter requires a lot of instruction, therefore in bootloader we use systick.
// USB functions, however, use millis16 for the delay. We declare a millis16(), which simply gets from a ram pointer the function that should be used.
uint16_t f_millis16(void)
{
	#if !USE_SYSTICK_TIMER
		return TC4->COUNT32.COUNT.reg;
	#else
		return m_millis;
	#endif
}
/*
*	initUsvc - Initializes the USVC hardware.
*	Prerequisites: Note: see initVga in vga.c!!!
*   If AUDIO_ENABLED, then patchPointersParams must point to the array of patches. Otherwise it should be NULL. 
*/
void initUsvc(const patch_t *patchPointersParam)
{
	// kill the watch dog.
	WDT->CTRL.reg = 0;
	// Initialize ram pointers, to be able to use bootloader libraries
	void** ptr = ((void *)0x20000000);
	*ptr = &ramPointers;
	memset(ramPointers,0,sizeof(ramPointers));	
	//
	/* Set 1 Flash Wait State for 48MHz, cf tables 20.9 and 35.27 in SAMD21 Datasheet */
	NVMCTRL->CTRLB.bit.RWS = NVMCTRL_CTRLB_RWS_HALF_Val ;

	/* Turn on the digital interface clock */
	PM->APBAMASK.reg |= PM_APBAMASK_GCLK ;

	// reset the GCLK module so it is in a known state
	GCLK->CTRL.reg = GCLK_CTRL_SWRST;
	
	externalClkInit();

		// Now that all system clocks are configured, we can set CPU and APBx BUS clocks.There values are normally the one present after Reset.

		PM->CPUSEL.reg  = PM_CPUSEL_CPUDIV_DIV1 ;
		PM->APBASEL.reg = PM_APBASEL_APBADIV_DIV1_Val ;
		PM->APBBSEL.reg = PM_APBBSEL_APBBDIV_DIV1_Val ;
		PM->APBCSEL.reg = PM_APBCSEL_APBCDIV_DIV1_Val ;

		// Enable clocks. Since we are not implementing a low power device, let's activate everything. This will simplify the other initialization functions 
		PM->APBCMASK.reg = PM_APBCMASK_SERCOM0 | PM_APBCMASK_SERCOM1 | PM_APBCMASK_SERCOM2 | PM_APBCMASK_SERCOM3 | PM_APBCMASK_SERCOM4 | PM_APBCMASK_SERCOM5 | 
		PM_APBCMASK_TCC0 | PM_APBCMASK_TCC1 | PM_APBCMASK_TCC2 | PM_APBCMASK_TC3 | PM_APBCMASK_TC4 | PM_APBCMASK_TC5 |
		PM_APBCMASK_ADC | PM_APBCMASK_DAC | PM_APBCMASK_EVSYS;

		// Initialize DAC
		// Setting clock
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( GCM_DAC ) | // Generic Clock ADC
		GCLK_CLKCTRL_GEN_GCLK0     | // Generic Clock Generator 0 is source
		GCLK_CLKCTRL_CLKEN ;
		//
	initLed();
	#if USE_BL_AUDIO
		initBootloaderAudio();
	#endif
	#if USE_MIXER
		initAudioMixer();
	#endif
	#if AUDIO_ENABLED
		initMusicPlayer(patchPointersParam);
	#else
		(void) patchPointersParam;		// suppress warnings
	#endif
	initVga();	
 	millisTimerInit();	// must be called AFTER init_VGA();
 	SET_RAM_POINTER(POINTER_CONTEXT_SPECIFIC_FUNCTIONS, &contextSpecificFunctionVector);
 	initUSBModule(USB_device_Installers, usbStandardSingleDeviceAllocateDriverMemory);
 	usbHostInit();
	setVextVoltage5V(1);  // set output voltage from buck to be 5V. This allows the SD to work even without external cable (useful when debugging SD).
}

uint32_t millis(void)
{
	#if !USE_SYSTICK_TIMER
	return TC4->COUNT32.COUNT.reg;
	#else
		return m_millis;
	#endif
}
// This allows to use the systick timer on the bootloader (smaller size)
uint16_t millis16(void)
{
	uint32_t *fArray = (uint32_t*) GET_RAM_POINTER(POINTER_CONTEXT_SPECIFIC_FUNCTIONS);
	uint16_t (*f)(void) =  (uint16_t (*)(void)) fArray[P_F_MILLIS16];
	return f();
}
void cs_waitTillHasEnoughTime(uint16_t micros)
{
	uint32_t *fArray = (uint32_t*) GET_RAM_POINTER(POINTER_CONTEXT_SPECIFIC_FUNCTIONS);
	void (*f)(uint16_t) =  (void (*)(uint16_t)) fArray[P_F_WAIT_TILL_ENOUGH_TIME];
	return f(micros);
}
void cs_waitForVerticalBlank(void)
{
	uint32_t *fArray = (uint32_t*) GET_RAM_POINTER(POINTER_CONTEXT_SPECIFIC_FUNCTIONS);
	void (*f)(void) =  (void (*)(void)) fArray[P_F_WAIT_FOR_VERTICAL_BLANK];
	return f();
}
void setPortMux(uint8_t pinNumber, uint8_t peripheral)
{
	uint32_t temp ;
	if (pinNumber & 1)	// odd pin
	{
		// Get whole current setup for both odd and even pins and remove odd one
		temp = (PORT->Group[0].PMUX[pinNumber >> 1].reg) & PORT_PMUX_PMUXE( 0xF ) ;
		// Set new muxing
		PORT->Group[0].PMUX[pinNumber >> 1].reg = temp | PORT_PMUX_PMUXO( peripheral ) ;	 
	}
	else
	{	// even pin
		uint32_t temp ;
		temp = (PORT->Group[0].PMUX[pinNumber >> 1].reg) & PORT_PMUX_PMUXO( 0xF ) ;
		PORT->Group[0].PMUX[pinNumber >> 1].reg = temp  |PORT_PMUX_PMUXE( peripheral ) ;
	}
	PORT->Group[0].PINCFG[pinNumber].reg |=  PORT_PINCFG_PMUXEN ;	
}
// these functions are used because when the VGA is driven, in several video modes the whole port is written at once, for speed.
// this might alter the functionality. As a workaround, we use the particular functions of the various peripheral, such as serial, clock, etc.
void setClock0OutputValue(uint8_t value)   // this is for boost enable
{
	GCLK->GENCTRL.reg = (GCLK_GENCTRL_GENEN |
	GCLK_GENCTRL_SRC_DPLL96M |
	GCLK_GENCTRL_ID(0)) | (GCLK_GENCTRL_OOV *  (value != 0));
}
void setClock1OutputValue(uint8_t value)		// this is for 5V/3.3V selector
{
	setPortMux(15, PORT_PMUX_PMUXO_H_Val);
	GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(1) | (GCLK_GENCTRL_OOV *  (value != 0));
}
/*
	Set LED:
	In some modes (4bpp), we cannot use the output directly, as the whole port is overwritten. Therefore we use this nice trick to set the LED on or Off
*/
void initLed(void)
{
	//REG_PM_APBCMASK |= PM_APBCMASK_SERCOM0;  already done in initUsvc()
	//Setting clock
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( GCLK_CLKCTRL_ID_SERCOM0_CORE_Val ) | // Generic Clock 0 (SERCOMx)
	GCLK_CLKCTRL_GEN_GCLK0 | // Generic Clock Generator 0 is source
	GCLK_CLKCTRL_CLKEN ;
	REG_SERCOM0_SPI_CTRLA = SERCOM_SPI_CTRLA_MODE_SPI_MASTER |  (2 << SERCOM_SPI_CTRLA_DOPO_Pos) | SERCOM_SPI_CTRLA_ENABLE;
	REG_PORT_DIRCLR0 = LEDPIN;  // The LED is an input when in the off state!
}
void setLed(uint8_t state)
{	
	if (state)
		setPortMux(7, PORT_PMUX_PMUXO_D_Val);
	else
		setPortMux(7, 0);
}
#if USE_SYSTICK_TIMER
#if  !IS_BOOTLOADER
	#error DO NOT USE SystickTimer in game mode!
#endif
RAMFUNC void SysTick_Handler(void)			// it has to be in ram, otherwise during flash erase it won't work.
{
	m_millis++;
}
#endif
void millisTimerInit(void)
{
	#if USE_SYSTICK_TIMER
		NVIC_SetPriority(SysTick_IRQn, 3);
		SysTick->LOAD = 48000;
		SysTick->VAL = 0;
		SysTick->CTRL = 3;
	#else
	// It's stupid* to create a timer that each millisecond calls an interrupt to update a 32-bit value. We could instead just use the 32-bit counter value of a counter clocked at 1kHz.
	// Unluckily we cannot simply choose a 1kHz GCLK input clock for the timer. This would require a very slow synchronization for each readout, which would stall the system.
	// We will then use two timers. After all we do not have to do anything else with them... Both timers are clocked at 48MHz. The first one counts 48000, and generates one event. This event is used to increase 
	// the count on the second register. Actually we are using 3 times, as we want a 32-bit millisecond counter.
	// First configure TC3 as a 1kHz generator
	//
	// *beside being a stupid idea, it won't even work on our system, as during the video frame we use almost any instructrion for video + audio.
	REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3) ;
	//
	// REG_PM_APBCMASK |= PM_APBCMASK_TC3; already done in initUsvc()
	REG_TC3_CTRLA = TC_CTRLA_PRESCSYNC_GCLK | TC_CTRLA_RUNSTDBY | TC_CTRLA_PRESCALER_DIV1 |
	TC_CTRLA_WAVEGEN_MPWM | TC_CTRLA_MODE_COUNT16;
	REG_TC3_INTFLAG = TC_INTFLAG_MC0;		// clear flags on ch 0
	REG_TC3_COUNT16_CC0 = 48000 - 1;
	//REG_TC3_INTENSET = TC_INTENSET_MC0;
	REG_TC3_EVCTRL = TC_EVCTRL_MCEO0;		// enable Event generation on match
	TC3->COUNT16.CTRLC.bit.INVEN1 = 1;
	// finally enable timer
	REG_TC3_CTRLA |= TC_CTRLA_ENABLE;
	// start
	REG_TC3_CTRLBSET = TC_CTRLBCLR_CMD_RETRIGGER;
	// Let's configure the event sys
	REG_EVSYS_CTRL = EVSYS_CTRL_GCLKREQ;
	REG_EVSYS_CHANNEL = EVSYS_CHANNEL_EDGSEL_RISING_EDGE | EVSYS_CHANNEL_PATH_ASYNCHRONOUS | EVSYS_CHANNEL_CHANNEL(1) | EVSYS_CHANNEL_EVGEN(EVSYS_ID_GEN_TC3_MCX_0);
	REG_EVSYS_USER = EVSYS_USER_CHANNEL(1 + 1) | EVSYS_USER_USER(0x13);	// 0x13 = TC4 
	//
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |         // Enable the generic clock...
	GCLK_CLKCTRL_GEN_GCLK0 |     // ....on GCLK0
	GCLK_CLKCTRL_ID_TC4_TC5;     // Feed the GCLK5 to TC4 and TC5

	TC4->COUNT32.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1 |      // Set prescaler to 1, 1kHz/1 = 1kHz
	TC_CTRLA_MODE_COUNT32 | TC_CTRLA_RUNSTDBY;         // Set the TC4 timer to 32-bit mode in conjuction with timer TC5
	REG_TC4_EVCTRL = TC_EVCTRL_EVACT_COUNT | TC_EVCTRL_TCEI;
	TC4->COUNT32.CTRLA.bit.ENABLE = 1;               // Enable TC4

	TC4->COUNT32.READREQ.reg = TC_READREQ_RCONT |            // Enable a continuous read request
	TC_READREQ_ADDR(0x10);        // Offset of the 32-bit COUNT register
	#endif
}

