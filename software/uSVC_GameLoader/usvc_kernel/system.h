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
*  audioMixer.h Here the audio samples are calculated for each channel, and the summed, during the screen blank.
*/
#ifndef SYSTEM_H_
#define SYSTEM_H_
#include <stdint.h>
//#define RAMFUNC __attribute__ ((section(".ramfunc")))
#define RAMFUNC __attribute__ ((section(".data,\"aw\" // ")))		// NOTE: if we do not put the ram function in .data, then the "RunOutputFileVerifyTask" will under report RAM usage.
#define min(a,b) (a < b ? a : b)
#define max(a,b) (a > b ? a : b)
#define GET_BOOTLOADER_LIBRARY_ELEMENT(x) (*((uint32_t **)(0x00002000 + 44*4)))[x]
#define GET_RAM_POINTER(x) ((*((uint32_t**)0x20000000))[x])
#define SET_RAM_POINTER(x,v) do{(*((uint32_t**)0x20000000))[x] =(uint32_t) v; }while(0)
#define GET_MODULE_ADDRESS(x) &((*((uint32_t**)0x20000000))[x])
enum
{
	FAT_FS_PF_MOUNT = 0,
	FAT_FS_PF_OPEN,
	FAT_FS_PF_READ,
	FAT_FS_PF_WRITE,
	FAT_FS_PF_LSEEK,
	FAT_FS_PF_OPENDIR,
	FAT_FS_PF_READDIR,
	USB_HOST_INIT,
	USB_HOST_TASK,
	USB_PIPE0_ALLOC,
	USB_PIPE_ALLOC,
	USB_PIPE_FREE,
	USB_CREATE_STANDARD_REQUEST,
	USB_ADD_TRANSACTION,
	USB_RELEASE_DEVICE,
	USB_GET_STATE,	
	USB_STRING_DESCRIPTOR_TO_CHAR,
	USB_FIND_HID_INTERFACE_AND_ENDPOINT,
	USB_KEYBOARD_BOOT_INSTALLER,
	USB_KEYBOARD_IS_INSTALLED,
	USB_KEYBOARD_GET_KEY,
	USB_KEYBOARD_POLL,
	USB_KEYBOARD_GET_STATE,
	USB_KEYBOARD_GET_ASCII,
	USB_KEYBOARD_GET_ASCII_EX,	
	USB_KEYBOARD_SET_INSTALLATION_COMPLETE_CALLBACK,
	USB_GENERIC_GAMEPAD_INSTALLER,
	USB_GENERIC_GAMEPAD_IS_INSTALLED,
	USB_GENERIC_GAMEPAD_POLL,
	USB_GENERIC_GAMEPAD_GET_STATE,
	USB_GENERIC_GAMEPAD_SET_INSTALLATION_COMPLETE_CALLBACK,	

};
enum
{
	P_F_MILLIS16,
	P_F_WAIT_TILL_ENOUGH_TIME,
	P_F_WAIT_FOR_VERTICAL_BLANK	
};
enum 
{
	POINTER_MODULE_FATFS = 0,
	POINTER_MODULE_USB,
	POINTER_MODULE_USB_KEYBOARD,
	POINTER_KEYBOARD_INSTALLATION_CALLBACK,
	POINTER_CONTEXT_SPECIFIC_FUNCTIONS,		// those functions that need to access a global variable which is defined by the current application (bootloader or game)
	POINTER_MODULE_USB_GAMEPAD,
	POINTER_GAMEPAD_INSTALLATION_CALLBACK,
	MAX_MODULE
};
#include "audio.h"
#define LEDPIN  PORT_PA07
//#define FORCE_USB_DFLL48MHZ
#ifdef FORCE_USB_DFLL48MHZ
#warning FORCE_USB_DFLL48MHZ defined.
#endif

/** Frequency of the board main oscillator */
#define VARIANT_MAINOSC		(32768ul)

/** Master clock frequency */
#define M_CLK			  (48000000ul)
/* Generic Clock Multiplexer IDs */
/*
#define GCM_DFLL48M_REF           (0x00U)
#define GCM_FDPLL96M_INPUT        (0x01U)
#define GCM_FDPLL96M_32K          (0x02U)
#define GCM_WDT                   (0x03U)
#define GCM_RTC                   (0x04U)
#define GCM_EIC                   (0x05U)
#define GCM_USB                   (0x06U)*/
#define GCM_EVSYS_CHANNEL_0       (0x07U)
#define GCM_EVSYS_CHANNEL_1       (0x08U)
#define GCM_EVSYS_CHANNEL_2       (0x09U)
#define GCM_EVSYS_CHANNEL_3       (0x0AU)
#define GCM_EVSYS_CHANNEL_4       (0x0BU)
#define GCM_EVSYS_CHANNEL_5       (0x0CU)
#define GCM_EVSYS_CHANNEL_6       (0x0DU)
#define GCM_EVSYS_CHANNEL_7       (0x0EU)
#define GCM_EVSYS_CHANNEL_8       (0x0FU)
#define GCM_EVSYS_CHANNEL_9       (0x10U)
#define GCM_EVSYS_CHANNEL_10      (0x11U)
#define GCM_EVSYS_CHANNEL_11      (0x12U)
#define GCM_SERCOMx_SLOW          (0x13U)
#define GCM_SERCOM0_CORE          (0x14U)
#define GCM_SERCOM1_CORE          (0x15U)
#define GCM_SERCOM2_CORE          (0x16U)
#define GCM_SERCOM3_CORE          (0x17U)
#define GCM_SERCOM4_CORE          (0x18U)
#define GCM_SERCOM5_CORE          (0x19U)
#define GCM_TCC0_TCC1             (0x1AU)
#define GCM_TCC2_TC3              (0x1BU)
#define GCM_TC4_TC5               (0x1CU)
#define GCM_TC6_TC7               (0x1DU)
#define GCM_ADC                   (0x1EU)
#define GCM_AC_DIG                (0x1FU)
#define GCM_AC_ANA                (0x20U)
#define GCM_DAC                   (0x21U)

// Constants for Clock generators
#define GENERIC_CLOCK_GENERATOR_MAIN      (0u)
#define GENERIC_CLOCK_GENERATOR_XOSC32K   (1u)
#define GENERIC_CLOCK_GENERATOR_OSC32K    (1u)
#define GENERIC_CLOCK_GENERATOR_OSCULP32K (2u) /* Initialized at reset for WDT */
#define GENERIC_CLOCK_GENERATOR_OSC8M     (3u)
// Constants for Clock multiplexers
#define GENERIC_CLOCK_MULTIPLEXER_DFLL48M (0u)
#define setVextVoltage5V(voltageIs5V) setClock1OutputValue(voltageIs5V)
#define setDisablePinValue(v) setClock4OutputValue(v)
void setPortMux(uint8_t pinNumber, uint8_t peripheral);
void millisTimerInit(void);
void setLed(uint8_t state);
uint32_t millis(void);
uint16_t millis16(void);  // 16 bit version to save ram an for easy implementation with the systick timer.
void initLed(void);
void initUsvc(const patch_t *patchPointerParams);
void setClock0OutputValue(uint8_t value);
void setClock1OutputValue(uint8_t value);
inline void setClock4OutputValue(uint8_t value)		// this is for the DISABLE (74AHC245 nOE)
{
	GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(4) | (GCLK_GENCTRL_OOV *  (value != 0));
}
void cs_waitTillHasEnoughTime(uint16_t micros);		// context specific wait till has enough time
void cs_waitForVerticalBlank(void);			// context specific wait for vertical blank
#endif /* SYSTEM_H_ */