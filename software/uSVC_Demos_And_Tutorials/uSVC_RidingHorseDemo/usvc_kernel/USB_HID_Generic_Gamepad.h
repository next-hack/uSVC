
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
*  USB_HID_Generic_Gamepad.c/h: Generic Gamepad enumeration and handling.
*
*  Note: instead of doing a full and accurate HID descriptor parsing, we just
*  look for specific patterns. This might not work on some fancy gamepads.
*
*/
#ifndef USB_HID_GENERIC_GAMEPAD_H_
#define USB_HID_GENERIC_GAMEPAD_H_
#include "usb_host.h"
#define GP_BUTTON_1 1
#define GP_BUTTON_2 2
#define GP_BUTTON_3 4
#define GP_BUTTON_4 8
#define GP_BUTTON_L1 16
#define GP_BUTTON_R1 32
#define GP_BUTTON_L2 64
#define GP_BUTTON_R2 128
#define GP_BUTTON_SELECT 256
#define GP_BUTTON_START 512
#define MAX_GAMEPADS 1
typedef struct
{
	uint32_t buttons;
	int16_t axes[4];
	int16_t XYZRxMinimum;
	int16_t XYZRxMaximum;
	uint8_t numberOfAxes;
	uint8_t numberOfButtons;
} gamePadState_t;
typedef struct
{
	__attribute__((__aligned__(4))) uint8_t reportBuffer[16];
	
//	__attribute__((__aligned__(4))) uint8_t prevState[8];
//	void (*OnControlKeysChanged)(uint8_t prev, uint8_t now);
//	void (*OnKeyDown)(uint8_t modifier, uint8_t key);
//	void (*OnKeyUp)(uint8_t modifier, uint8_t key);
	uint16_t time;
	uint16_t wReportDescriptorSize;
	uint8_t pipe_ep0_n;
	uint8_t pipe_n;
	uint8_t intrfc;
	uint8_t deviceInstalled;
	uint8_t pollInterval;
	uint8_t bitStartXYZRx;
	uint8_t bitStopXYZRx;
	uint8_t axeSize;
	gamePadState_t gamePadState;
	uint8_t bitStartButtons;
	uint8_t reportSize;
} usbHidGamepadData_t;
typedef struct
{
	usbHidGamepadData_t gamepadData[MAX_GAMEPADS];
	uint8_t numGamepads;
} usbGamepadStruct_t;
uint32_t usbHidGenericGamepadInstaller(uint32_t action, usbDevice_t* pdev, void *paramPtr);
uint8_t usbHidGenericGamepadIsInstalled();
uint32_t usbHidGenericGamepadPoll();
uint8_t getCurrentGamepadState(gamePadState_t *gps);
void setGenericGamepadInstallationCompleteCallback(void *callback);

#endif /* USB_HID_GENERIC_GAMEPAD_H_ */