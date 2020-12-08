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
*  USB_HID_Boot_Keyboard.c/h: keyboard enumeration and handling.
*
* Credits: even if this part is strongly modified, this is based on the original
* USB Host library for Arduino, Copyright (C) 2011 Circuits At Home, LTD. All rights reserved,
* which was released under the terms of the GNU General Public License version 2 (GPL2).
*/
#ifndef USB_HID_BOOT_KEYBOARD_H_
#define USB_HID_BOOT_KEYBOARD_H_
#include "usb_host.h"
// Modifiers

#define USB_KEY_MOD_LCTRL  0x01
#define USB_KEY_MOD_LSHIFT 0x02
#define USB_KEY_MOD_LALT   0x04
#define USB_KEY_MOD_LGUI  0x08
#define USB_KEY_MOD_RCTRL  0x10
#define USB_KEY_MOD_RSHIFT 0x20
#define USB_KEY_MOD_RALT   0x40
#define USB_KEY_MOD_RGUI  0x80

/**
 * Scan codes - last N slots in the HID report (usually 6).
 * 0x00 if no key pressed.
 * 
 * If more than N keys are pressed, the HID reports 
 * KEY_ERR_OVF in all slots to indicate this condition.
 */

#define USB_KEY_NONE 0x00 // No key pressed
#define USB_KEY_ERR_OVF 0x01 //  Keyboard Error Roll Over - used for all slots if too many keys are pressed ("Phantom key")
#define USB_KEYBOARD_POST_FAIL 0x02  //  Keyboard POST Fail
#define USB_ERROR_UNDEFINED 0x03 //  Keyboard Error Undefined
#define USB_KEY_A 0x04 // Keyboard a and A
#define USB_KEY_B 0x05 // Keyboard b and B
#define USB_KEY_C 0x06 // Keyboard c and C
#define USB_KEY_D 0x07 // Keyboard d and D
#define USB_KEY_E 0x08 // Keyboard e and E
#define USB_KEY_F 0x09 // Keyboard f and F
#define USB_KEY_G 0x0a // Keyboard g and G
#define USB_KEY_H 0x0b // Keyboard h and H
#define USB_KEY_I 0x0c // Keyboard i and I
#define USB_KEY_J 0x0d // Keyboard j and J
#define USB_KEY_K 0x0e // Keyboard k and K
#define USB_KEY_L 0x0f // Keyboard l and L
#define USB_KEY_M 0x10 // Keyboard m and M
#define USB_KEY_N 0x11 // Keyboard n and N
#define USB_KEY_O 0x12 // Keyboard o and O
#define USB_KEY_P 0x13 // Keyboard p and P
#define USB_KEY_Q 0x14 // Keyboard q and Q
#define USB_KEY_R 0x15 // Keyboard r and R
#define USB_KEY_S 0x16 // Keyboard s and S
#define USB_KEY_T 0x17 // Keyboard t and T
#define USB_KEY_U 0x18 // Keyboard u and U
#define USB_KEY_V 0x19 // Keyboard v and V
#define USB_KEY_W 0x1a // Keyboard w and W
#define USB_KEY_X 0x1b // Keyboard x and X
#define USB_KEY_Y 0x1c // Keyboard y and Y
#define USB_KEY_Z 0x1d // Keyboard z and Z
// numbers
#define USB_KEY_1 0x1e // Keyboard 1 and !
#define USB_KEY_2 0x1f // Keyboard 2 and @
#define USB_KEY_3 0x20 // Keyboard 3 and #
#define USB_KEY_4 0x21 // Keyboard 4 and $
#define USB_KEY_5 0x22 // Keyboard 5 and %
#define USB_KEY_6 0x23 // Keyboard 6 and ^
#define USB_KEY_7 0x24 // Keyboard 7 and &
#define USB_KEY_8 0x25 // Keyboard 8 and *
#define USB_KEY_9 0x26 // Keyboard 9 and (
#define USB_KEY_0 0x27 // Keyboard 0 and )
//
#define USB_KEY_ENTER 0x28 // Keyboard Return (ENTER)
#define USB_KEY_ESCAPE 0x29 // Keyboard ESCAPE
#define USB_KEY_DELETE 0x2a // Keyboard DELETE (Backspace)
#define USB_KEY_TAB 0x2b // Keyboard Tab
#define USB_KEY_SPACE 0x2c // Keyboard Spacebar
#define USB_KEY_MINUS 0x2d // Keyboard - and _
#define USB_KEY_EQUAL 0x2e // Keyboard = and +
#define USB_KEY_LEFTBRACE 0x2f // Keyboard [ and {
#define USB_KEY_RIGHTBRACE 0x30 // Keyboard ] and }
#define USB_KEY_BACKSLASH 0x31 // Keyboard \ and |
#define USB_KEY_HASHTILDE 0x32 // Keyboard Non-US # and ~
#define USB_KEY_SEMICOLON 0x33 // Keyboard ; and :
#define USB_KEY_APOSTROPHE 0x34 // Keyboard ' and "
#define USB_KEY_GRAVE 0x35 // Keyboard ` and ~
#define USB_KEY_COMMA 0x36 // Keyboard , and <
#define USB_KEY_DOT 0x37 // Keyboard . and >
#define USB_KEY_SLASH 0x38 // Keyboard / and ?
#define USB_KEY_CAPS_LOCK 0x39 // Keyboard Caps Lock
// Function keys
#define USB_KEY_F1 0x3a // Keyboard F1
#define USB_KEY_F2 0x3b // Keyboard F2
#define USB_KEY_F3 0x3c // Keyboard F3
#define USB_KEY_F4 0x3d // Keyboard F4
#define USB_KEY_F5 0x3e // Keyboard F5
#define USB_KEY_F6 0x3f // Keyboard F6
#define USB_KEY_F7 0x40 // Keyboard F7
#define USB_KEY_F8 0x41 // Keyboard F8
#define USB_KEY_F9 0x42 // Keyboard F9
#define USB_KEY_F10 0x43 // Keyboard F10
#define USB_KEY_F11 0x44 // Keyboard F11
#define USB_KEY_F12 0x45 // Keyboard F12
// 
#define USB_KEY_SYSRQ 0x46 // Keyboard Print Screen
#define USB_KEY_SCROLL_LOCK 0x47 // Keyboard Scroll Lock
#define USB_KEY_PAUSE 0x48 // Keyboard Pause
#define USB_KEY_INSERT 0x49 // Keyboard Insert
#define USB_KEY_HOME 0x4a // Keyboard Home
#define USB_KEY_PAGEUP 0x4b // Keyboard Page Up
#define USB_KEY_DELETE_FORWARD 0x4c // Keyboard Delete Forward
#define USB_KEY_END 0x4d // Keyboard End
#define USB_KEY_PAGEDOWN 0x4e // Keyboard Page Down
#define USB_KEY_RIGHT 0x4f // Keyboard Right Arrow
#define USB_KEY_LEFT 0x50 // Keyboard Left Arrow
#define USB_KEY_DOWN 0x51 // Keyboard Down Arrow
#define USB_KEY_UP 0x52 // Keyboard Up Arrow
//
#define USB_KEY_NUM_LOCK 0x53 // Keyboard Num Lock and Clear
#define USB_KEY_KPSLASH 0x54 // Keypad /
#define USB_KEY_KPASTERISK 0x55 // Keypad *
#define USB_KEY_KPMINUS 0x56 // Keypad -
#define USB_KEY_KPPLUS 0x57 // Keypad +
#define USB_KEY_KPENTER 0x58 // Keypad ENTER
#define USB_KEY_KP1 0x59 // Keypad 1 and End
#define USB_KEY_KP2 0x5a // Keypad 2 and Down Arrow
#define USB_KEY_KP3 0x5b // Keypad 3 and PageDn
#define USB_KEY_KP4 0x5c // Keypad 4 and Left Arrow
#define USB_KEY_KP5 0x5d // Keypad 5
#define USB_KEY_KP6 0x5e // Keypad 6 and Right Arrow
#define USB_KEY_KP7 0x5f // Keypad 7 and Home
#define USB_KEY_KP8 0x60 // Keypad 8 and Up Arrow
#define USB_KEY_KP9 0x61 // Keypad 9 and Page Up
#define USB_KEY_KP0 0x62 // Keypad 0 and Insert
#define USB_KEY_KPDOT 0x63 // Keypad . and Delete

#define USB_KEY_102ND 0x64 // Keyboard Non-US \ and |
#define USB_KEY_COMPOSE 0x65 // Keyboard Application
#define USB_KEY_POWER 0x66 // Keyboard Power
#define USB_KEY_KPEQUAL 0x67 // Keypad =

#define USB_KEY_F13 0x68 // Keyboard F13
#define USB_KEY_F14 0x69 // Keyboard F14
#define USB_KEY_F15 0x6a // Keyboard F15
#define USB_KEY_F16 0x6b // Keyboard F16
#define USB_KEY_F17 0x6c // Keyboard F17
#define USB_KEY_F18 0x6d // Keyboard F18
#define USB_KEY_F19 0x6e // Keyboard F19
#define USB_KEY_F20 0x6f // Keyboard F20
#define USB_KEY_F21 0x70 // Keyboard F21
#define USB_KEY_F22 0x71 // Keyboard F22
#define USB_KEY_F23 0x72 // Keyboard F23
#define USB_KEY_F24 0x73 // Keyboard F24

#define USB_KEY_OPEN 0x74 // Keyboard Execute
#define USB_KEY_HELP 0x75 // Keyboard Help
#define USB_KEY_PROPS 0x76 // Keyboard Menu
#define USB_KEY_FRONT 0x77 // Keyboard Select
#define USB_KEY_STOP 0x78 // Keyboard Stop
#define USB_KEY_AGAIN 0x79 // Keyboard Again
#define USB_KEY_UNDO 0x7a // Keyboard Undo
#define USB_KEY_CUT 0x7b // Keyboard Cut
#define USB_KEY_COPY 0x7c // Keyboard Copy
#define USB_KEY_PASTE 0x7d // Keyboard Paste
#define USB_KEY_FIND 0x7e // Keyboard Find
#define USB_KEY_MUTE 0x7f // Keyboard Mute
#define USB_KEY_VOLUMEUP 0x80 // Keyboard Volume Up
#define USB_KEY_VOLUMEDOWN 0x81 // Keyboard Volume Down
#define USB_KEY_LOCKING_CAPS 0x82 // Keyboard Locking Caps Lock
#define USB_KEY_LOCKING_NUM 0x83 // Keyboard Locking Num Lock
#define USB_KEY_LOCKING_SCROLL  0x84 // Keyboard Locking Scroll Lock
#define USB_KEY_KPCOMMA 0x85 // Keypad Comma
#define USB_KP_EQUAL 0x86 // Keypad Equal Sign
//
#define USB_KEY_KP_LEFTPAREN 0xb6 // Keypad (
#define USB_KEY_KP_RIGHTPAREN 0xb7 // Keypad )
#define USB_KEY_KP_LEFTBRACE  0xb8 // Keypad {
#define USB_KEY_KP_RIGHTBRACE 0xb9 // Keypad }
//
#define USB_KEY_LEFTCTRL 0xe0 // Keyboard Left Control
#define USB_KEY_LEFTSHIFT 0xe1 // Keyboard Left Shift
#define USB_KEY_LEFTALT 0xe2 // Keyboard Left Alt
#define USB_KEY_LEFTGUI 0xe3 // Keyboard Left GUI
#define USB_KEY_RIGHTCTRL 0xe4 // Keyboard Right Control
#define USB_KEY_RIGHTSHIFT 0xe5 // Keyboard Right Shift
#define USB_KEY_RIGHTALT 0xe6 // Keyboard Right Alt
#define USB_KEY_RIGHTGUI 0xe7 // Keyboard Right GUI
//
#define USB_KEY_RESET			0xFF		// Ctrl alt del. Not from standard!
//
#define EXBUFF_TO_OEM(key) (key >> 8)
#define MAX_KEYBOARDS 1
#define MAX_KB_FIFO_SIZE 16
typedef struct
{
	__attribute__((__aligned__(4))) uint8_t reportBuffer[8];
	__attribute__((__aligned__(4))) uint8_t prevState[8];
	__attribute__((__aligned__(4))) uint8_t setReportBuffer[8];	//
	uint32_t kbdLedReportData;	// note: this MUST be a 32 bit integer
	void (*onControlKeysChanged)(uint8_t prev, uint8_t now);
	void (*onKeyDown)(uint8_t modifier, uint8_t key);
	void (*onKeyUp)(uint8_t modifier, uint8_t key);
	uint16_t time;
	uint8_t pipe_ep0_n;
	uint8_t pipeNumber;
	uint8_t intrfc;
	uint8_t deviceInstalled;
	uint8_t pollInterval;
	uint8_t (*bufferFull)();
	// circular buffer
	uint16_t fifo[MAX_KB_FIFO_SIZE];
	uint8_t start;
	uint8_t end;
	uint8_t kbdLockingKeys;
} usbHidBootKeyboardData_t;
typedef struct
{
	usbHidBootKeyboardData_t keyboardData[MAX_KEYBOARDS];
	uint8_t numKeyboards;
} usbKeyboardStruct_t;
uint32_t usbHidBootKeyboardInstaller(uint32_t action, usbDevice_t* pdev, void *paramPtr);
uint8_t usbHidBootKeyboardIsInstalled();
int16_t usbGetKey();
uint32_t usbKeyboardPoll();
void usbGetCurrentKeyboardState(uint8_t * keyBuffer, uint8_t *modifier);

void usbGetCurrentAsciiKeyboardState(uint8_t *buffer);
void usbGetCurrentAsciiKeyboardStateEx(uint16_t *buffer);
void usbSetKeyboardInstallationCompleteCallback(void *callback);
void usbSetOnKeyDown(void * callBack);
void usbSetOnKeyUp(void *callBack);
void usbSetOnControlKeyStateChange(void *callBack);
#endif /* USB_HID_BOOT_KEYBOARD_H_ */