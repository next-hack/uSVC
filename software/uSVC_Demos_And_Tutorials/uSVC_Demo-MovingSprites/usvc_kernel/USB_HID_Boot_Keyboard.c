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
#include "usvc_kernel.h" 
#include "USB_HID_boot_Keyboard.h"
#include <stdio.h>
#include <string.h>

//
#if USB_KEYBOARD_DRIVER_SUPPORTS_MORE_KEYBOARDS && MAX_KEYBOARDS > 1
	#define CURRENT_KEYBOARD pKeyboardData->numKeyboards
#else
	#define CURRENT_KEYBOARD 0
	#define OVERRIDE_INSTALLED 1
#endif
#define NUMLOCK_bm 1
#define CAPSLOCK_bm 2
#define SCROLLLOCK_bm 4
// 

//
#define VALUE_WITHIN(v,l,h) (((v)>=(l)) && ((v)<=(h)))

//#define FORCE_INCLUDE_USB_KEYBOARD_MODULE 1
#if !(IS_BOOTLOADER || FORCE_INCLUDE_USB_KEYBOARD_MODULE)
uint32_t usbHidBootKeyboardInstaller(uint32_t action, usbDevice_t* pdev, void *paramPtr)
{
	uint32_t (*f)(uint32_t, usbDevice_t* , void *);
	f = (uint32_t (*)(uint32_t, usbDevice_t* , void *)) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_KEYBOARD_BOOT_INSTALLER);
	return f(action, pdev, paramPtr);
}
uint8_t usbHidBootKeyboardIsInstalled()
{
	uint8_t (*f)();	
	f = (uint8_t (*)()) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_KEYBOARD_IS_INSTALLED);
	return f();
}
int16_t usbGetKey()
{
	int16_t (*f)();
	f = (int16_t (*)()) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_KEYBOARD_GET_KEY);
	return f();	
}
uint32_t usbKeyboardPoll()
{
	uint32_t (*f)();	
	f = (uint32_t (*)()) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_KEYBOARD_POLL);
	return f();
}
void usbGetCurrentKeyboardState(uint8_t *keyBuffer, uint8_t *modifier) 
{
	void (*f)(uint8_t *, uint8_t *);	
	f = (void (*)(uint8_t*, uint8_t*)) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_KEYBOARD_GET_STATE);
	return f(keyBuffer, modifier);
}
void usbGetCurrentAsciiKeyboardState(uint8_t *buffer)
{
	void (*f)(uint8_t *buffer);
	f = (void (*)(uint8_t *)) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_KEYBOARD_GET_ASCII);	
	f(buffer);
}
void usbGetCurrentAsciiKeyboardStateEx(uint16_t *buffer)
{
	void (*f)(uint16_t *buffer);
	f = (void (*)(uint16_t *)) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_KEYBOARD_GET_ASCII_EX);	
	f(buffer);
}
void usbSetKeyboardInstallationCompleteCallback(void *callback)
{
	void (*f)(void *);	
	f = (void (*)(void *)) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_KEYBOARD_SET_INSTALLATION_COMPLETE_CALLBACK);
	f(callback);
}
#else
static const uint8_t numKeys[10] = { '!', '@', '#', '$', '%', '^', '&', '*', '(', ')'};
static const uint8_t symKeysUp[12] = { '_', '+', '{', '}', '|', '~', ':', '"', '~', '<', '>', '?'};
static const uint8_t symKeysLo[12] = { '-', '=', '[', ']', '\\', ' ', ';', '\'', '`', ',', '.', '/'};
static const uint8_t padKeys[5] = { '/', '*', '-', '+', '\r'};

// forward declaration of local functions
static uint16_t OemToAscii(usbHidBootKeyboardData_t *p_kbd, uint8_t mod, uint8_t key);
static uint32_t HandleLockingKeys(usbHidBootKeyboardData_t * p_kbd, uint8_t key);
static uint8_t putKey(usbHidBootKeyboardData_t * p_kbd, uint16_t key);



//#define USB_KB_DEBUG
#if USB_KB_DEBUG
	static volatile char usbDebugBuffer[40];
	#define USB_KB_TRACE(...) do { snprintf((char*)usbDebugBuffer,sizeof(usbDebugBuffer),__VA_ARGS__); addLineAtBottom( (char *) usbDebugBuffer, 1, 0);}while(0)
#else
	#define USB_KB_TRACE(...) do{}while(0)
#endif


//static uint8_t numKeyboards = 0;
//usbHidBootKeyboardData_t keyboard_data[MAX_KEYBOARDS];

typedef enum
{
	KB_CREATE_PIPES,
	KB_SET_CONFIGURATION,
	KB_WAIT_FOR_SET_CONFIGURATION,
	KB_CONFIGURATION_SET,
	KB_SET_INTERFACE_PROTOCOL,
	KB_WAIT_FOR_SET_INTERFACE_PROTOCOL,
	KB_INTERFACE_PROTOCOL_SET,
	KB_REQUEST_SET_CAPSLOCK,
	KB_WAIT_FOR_SET_LED_CAPSLOCK,
	KB_CAPSLOCK_SET,
	KB_WAIT_1000MS,
	KB_SET_CAPSLOCK_OFF,
	KB_WAIT_FOR_SET_CAPSLOCK_OFF,
	KB_CAPSLOCK_OFF,
	KB_WAIT_1000MS_2,
	KB_INSTALLATION_COMPLETE,
	KB_INSTALLATION_ERROR = 0xFFFF		
} kb_installation_state;

usbKeyboardStruct_t *getKeyboardData()
{
	return (usbKeyboardStruct_t *) GET_RAM_POINTER(POINTER_MODULE_USB_KEYBOARD);
}
void kbInstallationErrorCallback(void* signal)
{
	if (signal)
	{
		uint32_t *p = (uint32_t*) signal;
		*p = KB_INSTALLATION_ERROR;
	}
	USB_KB_TRACE("USB KB ERROR");
}
void kbInstallationTransactionCompleteCallback(void* signal, int bytesReceived)
{
	(void) bytesReceived;		// remove warning
	if (signal)
	{
		uint32_t *p = (uint32_t*) signal;
		*p = *p + 1;
	}	
	
	USB_KB_TRACE("USB KB TRANSFER COMPLETE!");
}
uint32_t usbHidBootKeyboardInstaller(uint32_t action, usbDevice_t* pdev, void *paramPtr)
{
	usbInterfaceEndpointDescriptor_t** interfaceEndpointList = paramPtr;
	usbModuleData_t *pUSB = getUSBData();	
	usbKeyboardStruct_t *pKeyboardData = getKeyboardData();
	if (action == CHECK_DRIVER_COMPATIBILITY)
	{
		// we need to parse the interfaces found
		uint8_t dummy;
		return usbFindHidInterfaceAndEndpoint(pdev->interfacesFound, interfaceEndpointList, HID_BOOT_INTF_SUBCLASS, HID_PROTOCOL_KEYBOARD, &dummy, &dummy);
	}
	else if (action == INSTALL_DEVICE)
	{
		uint16_t millisNow = millis16();
		switch (pdev->installationStep)
		{
				// here we are still in the enumeration, so we can use the common endpoint buffer.
				case KB_CREATE_PIPES:
							{
						uint8_t i;
						uint8_t e;
						usbFindHidInterfaceAndEndpoint(pdev->interfacesFound, interfaceEndpointList, HID_BOOT_INTF_SUBCLASS, HID_PROTOCOL_KEYBOARD, &i, &e);
									pdev->userDataPtr = &pKeyboardData->keyboardData[CURRENT_KEYBOARD];
									memset(&pKeyboardData->keyboardData[CURRENT_KEYBOARD], 0, sizeof(usbHidBootKeyboardData_t));
									// good! We found a keyboard!
									// Let's grab the endpoints
									#if OVERRIDE_INSTALLED
										int pipe_n;
										// allocate interrupt ep
										pipe_n = uhdPipeAlloc(pdev->address,  
							interfaceEndpointList[i]->data.hid_endpoints.endpoint[e].bEndpointAddress & 0xF,
																	USB_HOST_PTYPE_INT_val, 
																	USB_EP_DIR_IN,
																	8, 
							interfaceEndpointList[i]->data.hid_endpoints.endpoint[e].bInterval ,
																	0);
										if (pipe_n == 0)
										{
											USB_KB_TRACE("NO ENOUGH PIPES");
								pdev->installationStep = KB_INSTALLATION_ERROR;
											return USB_NOT_ENOUGH_PIPES;
										}
										USB_KB_TRACE("Pipe %x");
							pKeyboardData->keyboardData[CURRENT_KEYBOARD].pollInterval = interfaceEndpointList[i]->data.hid_endpoints.endpoint[e].bInterval;
													pKeyboardData->keyboardData[CURRENT_KEYBOARD].pipeNumber = pipe_n;
										// allocate control ep
										pipe_n = uhdPipeAlloc(pdev->address,
																0,
																USB_HOST_PTYPE_CTRL_val, 
																USB_EP_DIR_IN,					// this is actually overridden as it is ctrl endpoint.			
																8,
																0,
																0);
										if (pipe_n == 0)
										{
											// free previous allocated ep
											uhdPipeFree(pKeyboardData->keyboardData[CURRENT_KEYBOARD].pipeNumber);
								pdev->installationStep = KB_INSTALLATION_ERROR;
											return USB_NOT_ENOUGH_PIPES;		
										}
										pKeyboardData->keyboardData[CURRENT_KEYBOARD].pipe_ep0_n = pipe_n;
										USB_KB_TRACE("KB Endpoints allocated!");
										pKeyboardData->keyboardData[CURRENT_KEYBOARD].intrfc = i;
										// initialize circular buffer
										pKeyboardData->keyboardData[CURRENT_KEYBOARD].start = 0;
										pKeyboardData->keyboardData[CURRENT_KEYBOARD].end = 0;
							pdev->installationStep = KB_SET_CONFIGURATION;
									#else
													#error multi keyboard not implemented
									#endif
								}
					break;
				case KB_SET_CONFIGURATION:
					USB_KB_TRACE("KB SET CONFIG");
					usbCreateSetConfiguration(pUSB->pUsbEndpoint0ControlBuffer, 1);		// set the first config.
					usbAddTransaction(0, 0, pUSB->pUsbEndpoint0ControlBuffer, pUSB->pUsbEndpoint0Buffer, DIRECTION_OUT, USB_TRANSFER_TYPE_CONTROL, USB_TIMEOUT, kbInstallationErrorCallback, kbInstallationTransactionCompleteCallback, &pdev->installationStep);
					pdev->installationStep = KB_WAIT_FOR_SET_CONFIGURATION;
					break;
				case KB_WAIT_FOR_SET_CONFIGURATION:
					// do nothing
					break;
				case KB_CONFIGURATION_SET:
					USB_KB_TRACE("KB SET CONFIG SET");
					pdev->installationStep = KB_SET_INTERFACE_PROTOCOL;
					break;
				case KB_SET_INTERFACE_PROTOCOL:
					USB_KB_TRACE("KB SET IF PROTOCOL");
					usbCreateSetInterfaceProtocol(pUSB->pUsbEndpoint0ControlBuffer, pKeyboardData->keyboardData[CURRENT_KEYBOARD].intrfc, HID_BOOT_PROTOCOL);
					usbAddTransaction(0, 0, pUSB->pUsbEndpoint0ControlBuffer, pUSB->pUsbEndpoint0Buffer, DIRECTION_OUT, USB_TRANSFER_TYPE_CONTROL, USB_TIMEOUT, kbInstallationErrorCallback, kbInstallationTransactionCompleteCallback, &pdev->installationStep);					
					pdev->installationStep = KB_WAIT_FOR_SET_INTERFACE_PROTOCOL;
					break;
				case KB_WAIT_FOR_SET_INTERFACE_PROTOCOL:
					break;
				case KB_INTERFACE_PROTOCOL_SET:
					USB_KB_TRACE("KB IF PROTOCOL SET");
					pdev->installationStep = KB_REQUEST_SET_CAPSLOCK;
					break;
				case KB_REQUEST_SET_CAPSLOCK:
					pUSB->pUsbEndpoint0Buffer[0] = 0x07;
					usbCreateSetReport(pUSB->pUsbEndpoint0ControlBuffer, pKeyboardData->keyboardData[CURRENT_KEYBOARD].intrfc, HID_REPORT_OUTPUT, HID_REPORT_BOOT,  1);
					USB_KB_TRACE("SET CAPSLOCK");
					usbAddTransaction(0, 1, pUSB->pUsbEndpoint0ControlBuffer, pUSB->pUsbEndpoint0Buffer, DIRECTION_OUT, USB_TRANSFER_TYPE_CONTROL, USB_TIMEOUT, kbInstallationErrorCallback, kbInstallationTransactionCompleteCallback, &pdev->installationStep);
					pdev->time = millisNow;				
					pdev->installationStep = KB_WAIT_FOR_SET_LED_CAPSLOCK;
					break;
				case KB_WAIT_FOR_SET_LED_CAPSLOCK:
					// do nothing
					break;
				case KB_CAPSLOCK_SET:
					pdev->installationStep = KB_WAIT_1000MS;				
					break;
				case KB_WAIT_1000MS:
					if ( (uint16_t)(millisNow - pdev->time) > 1000UL)
						pdev->installationStep = KB_SET_CAPSLOCK_OFF;	
					break;
				case KB_SET_CAPSLOCK_OFF:
					pUSB->pUsbEndpoint0Buffer[0] = 0x00;
					usbCreateSetReport(pUSB->pUsbEndpoint0ControlBuffer, pKeyboardData->keyboardData[CURRENT_KEYBOARD].intrfc, HID_REPORT_OUTPUT, HID_REPORT_BOOT,  1);
					USB_KB_TRACE("CLR CAPSLOCK");
					usbAddTransaction(0, 1, pUSB->pUsbEndpoint0ControlBuffer, pUSB->pUsbEndpoint0Buffer, DIRECTION_OUT, USB_TRANSFER_TYPE_CONTROL, USB_TIMEOUT, kbInstallationErrorCallback, kbInstallationTransactionCompleteCallback, &pdev->installationStep);
					pdev->time = millisNow;			
					pdev->installationStep = KB_WAIT_FOR_SET_CAPSLOCK_OFF;
					break;
				case KB_WAIT_FOR_SET_CAPSLOCK_OFF:
						// do nothing
					break;
				case KB_CAPSLOCK_OFF:
				{
					pdev->installationStep = KB_INSTALLATION_COMPLETE; 
					pdev->enumerationState = ENUMERATION_COMPLETE;
					usbHidBootKeyboardData_t * p_kbd = (usbHidBootKeyboardData_t*) pdev->userDataPtr;
					p_kbd->deviceInstalled = 1;
					p_kbd->time = millisNow;
					break;
				}
				case KB_INSTALLATION_COMPLETE:
					// we should not reach this.
					break;
				case KB_INSTALLATION_ERROR:
					pdev->enumerationState = ENUMERATION_ERROR;  // corrected 2019/10/03
					break;
		}
		return 0;
	}
	else if (action == DEVICE_INSTALLED_CALLBACK)
	{
		void (*kbInstallationCompleteCallback)(void) = (void (*)(void)) GET_RAM_POINTER(POINTER_KEYBOARD_INSTALLATION_CALLBACK);
		if (kbInstallationCompleteCallback != NULL)
			kbInstallationCompleteCallback();
	}
	else if (action == GET_MEMORY_REQUIREMENTS)
	{	
		return sizeof(usbKeyboardStruct_t);
	}
	else if (action == SET_MEMORY)
	{
		int numKeyboards = 0;
		if (pKeyboardData) // is this the second time we initialize the driver?
		{
			numKeyboards = CURRENT_KEYBOARD;
			// this 
		}
		if (paramPtr)
			SET_RAM_POINTER(POINTER_MODULE_USB_KEYBOARD, paramPtr);
		pKeyboardData = getKeyboardData();
		pKeyboardData->numKeyboards = numKeyboards;		// copy - or initialize - data
		pdev->modulePointer = (void**) GET_MODULE_ADDRESS(POINTER_MODULE_USB_KEYBOARD);
	}
	return 0;
}
void usbSetKeyboardInstallationCompleteCallback(void *callback)
{
	SET_RAM_POINTER(POINTER_KEYBOARD_INSTALLATION_CALLBACK, callback);
}
void kbPollSuccessCallback(void* signal, int bytesReceived)
{
	(void) bytesReceived;	// remove unused variable warning	
	usbHidBootKeyboardData_t * p_kbd = (usbHidBootKeyboardData_t*) signal ;
	// signal in this case is p_kbd
	uint8_t *buf = p_kbd->reportBuffer;
	uint8_t *prevstate =  p_kbd->prevState;
	if (buf[2] == 1)
		return;
	// provide event for changed control key state
	if (prevstate[0x00] != buf[0x00]) 
	{
		if (p_kbd->onControlKeysChanged)
			p_kbd->onControlKeysChanged(prevstate[0x00], buf[0x00]);
		
	}

	for (uint32_t i = 2; i < 8; i++) 
	{
		uint8_t down = 1;
		uint8_t up	 = 1;

		for (uint8_t j = 2; j < 8; j++) 
		{
			if (buf[i] ==  prevstate[j] && buf[i] != 1)
				down = 0;
			if (buf[j] == prevstate[i] && prevstate[i] != 1)
				up = 0;
		}
		if (down) 
		{
			USB_KB_TRACE("Lock %x ep0 %x", p_kbd->kbdLockingKeys, p_kbd->pipe_ep0_n);
			HandleLockingKeys(p_kbd, buf[i]);
			if (p_kbd->onKeyDown) 
				p_kbd->onKeyDown(buf[0], buf[i]);
			//
			if (putKey(p_kbd, OemToAscii(p_kbd, buf[0], buf[i])))
			{
				if (p_kbd->bufferFull)
					p_kbd->bufferFull();
			}
		}
		if (up)
			if (p_kbd->onKeyUp)
				p_kbd->onKeyUp(prevstate[0], prevstate[i]);
	}
// 	for(uint32_t i = 0; i < 8; i++)
// 		 prevstate[i] = buf[i];
	memcpy(prevstate, buf, 8);	// less bytes :)
}
void usbGetCurrentKeyboardState(uint8_t *keyBuffer, uint8_t *modifier)  // note! It is up to the user to check if keyboard is installed!
{
	usbKeyboardStruct_t *pKeyboardData = getKeyboardData();	
	*modifier = pKeyboardData->keyboardData[CURRENT_KEYBOARD].prevState[0];
	memcpy(keyBuffer, &pKeyboardData->keyboardData[CURRENT_KEYBOARD].prevState[2], 6);
}  
void usbGetCurrentAsciiKeyboardState(uint8_t *buffer)	// note! It is up to the user to check if keyboard is installed!
{
	usbKeyboardStruct_t *pKeyboardData = getKeyboardData();	
	for (int i = 0; i < 6; i++)
		buffer[i] =  OemToAscii(&pKeyboardData->keyboardData[CURRENT_KEYBOARD], pKeyboardData->keyboardData[CURRENT_KEYBOARD].prevState[0], pKeyboardData->keyboardData[CURRENT_KEYBOARD].prevState[i + 2]);
}
void usbGetCurrentAsciiKeyboardStateEx(uint16_t *buffer)	// Similar to the previous one, but for non-ascii characters, we set in the higher byte, the scancode.
{
	usbKeyboardStruct_t *pKeyboardData = getKeyboardData();
	for (int i = 0; i < 6; i++)
		buffer[i] =  OemToAscii(&pKeyboardData->keyboardData[CURRENT_KEYBOARD], pKeyboardData->keyboardData[CURRENT_KEYBOARD].prevState[0], pKeyboardData->keyboardData[CURRENT_KEYBOARD].prevState[i + 2]);
}
static uint32_t HandleLockingKeys(usbHidBootKeyboardData_t * p_kbd, uint8_t key) 
{
	uint8_t mask = 0;
	switch (key) 
	{
		case USB_KEY_NUM_LOCK:
			mask = NUMLOCK_bm;
			break;
		case USB_KEY_CAPS_LOCK:
			mask = CAPSLOCK_bm;
			break;
		case USB_KEY_SCROLL_LOCK:
			mask = SCROLLLOCK_bm;
			break;
	}
	if (mask)
	{
		USB_KB_TRACE("Data: 0x%x",p_kbd->kbdLockingKeys);
		p_kbd->kbdLockingKeys = p_kbd->kbdLockingKeys ^ mask;
		p_kbd->kbdLedReportData = p_kbd->kbdLockingKeys;
		usbCreateSetReport(p_kbd->setReportBuffer, p_kbd->intrfc, HID_REPORT_OUTPUT, HID_REPORT_BOOT,  1);
		USB_KB_TRACE("Data: 0x%x",p_kbd->kbdLockingKeys);
		usbAddTransaction(p_kbd->pipe_ep0_n, 1, p_kbd->setReportBuffer, (uint8_t *) &p_kbd->kbdLedReportData, DIRECTION_OUT, USB_TRANSFER_TYPE_CONTROL, USB_TIMEOUT, NULL, NULL, NULL);	
		return 0;
	}
	return 0;
}

static uint16_t OemToAscii(usbHidBootKeyboardData_t *p_kbd, uint8_t mod, uint8_t key) 
{
	uint8_t shift = (mod & 0x22);
	uint8_t ctrl  = (mod & 0x11);
	uint8_t alt  = (mod & 0x44);
	// [a-z]
	if (VALUE_WITHIN(key, USB_KEY_A, USB_KEY_Z)) 
	{
		// [^a-^z]
		if (ctrl) 
			return (key - 3);
		// Upper case letters
		if (( !(p_kbd->kbdLockingKeys & CAPSLOCK_bm) && shift) || ((p_kbd->kbdLockingKeys & CAPSLOCK_bm) && shift == 0))
			return (key - USB_KEY_A + 'A');
		// Lower case letters
		else
			return (key - USB_KEY_A + 'a');
	}// Numbers
	else if (VALUE_WITHIN(key, USB_KEY_1, USB_KEY_0)) 
	{
		if (ctrl && (key == USB_KEY_6)) 
			return (0x1E); /* RS ^^ */
		if (shift)
			return numKeys[key - USB_KEY_1];
		else
			return ((key == USB_KEY_0) ? '0' : key - USB_KEY_1 + '1');
	}// Keypad Numbers
	else if (VALUE_WITHIN(key, USB_KEY_KP1, USB_KEY_KP9)) 
	{
		if (p_kbd->kbdLockingKeys & NUMLOCK_bm)
			return (key - USB_KEY_KP1 + '1');
	} 
	else if(VALUE_WITHIN(key, USB_KEY_MINUS, USB_KEY_SLASH)) 
	{
		if (ctrl) 
		{
			switch (key) 
			{
				case USB_KEY_MINUS: return (0x1f); /* US  ^_ */
				case USB_KEY_LEFTBRACE: return (0x1b); /* ESC ^[ */
				case USB_KEY_RIGHTBRACE: return (0x1d); /* GS  ^] */
				case USB_KEY_BACKSLASH: return (0x1c); /* FS  ^\ */
				default:   return (0x00);
			}
		}
		return (shift) ? symKeysUp[key - USB_KEY_MINUS] : symKeysLo[key - USB_KEY_MINUS];
	}
	else if (VALUE_WITHIN(key, USB_KEY_KPSLASH, USB_KEY_KPENTER))
		return padKeys[key - USB_KEY_KPSLASH];
	else 
	{
		switch(key) 
		{
			case USB_KEY_SPACE: return (0x20);
			case USB_KEY_ENTER: return ('\r');
			case USB_KEY_ESCAPE: return (0x1b);
			case USB_KEY_DELETE: return (0x08);
			case USB_KEY_DELETE_FORWARD: 
				return (ctrl && alt ? USB_KEY_RESET : 0x7f);
			case USB_KEY_TAB:   return (0x09);
			case USB_KEY_KP0: return ((p_kbd->kbdLockingKeys & NUMLOCK_bm) ? '0': 0);
			case USB_KEY_KPDOT: return ((p_kbd->kbdLockingKeys & NUMLOCK_bm) ? '.': 0);
		}
	}
	return (key << 8);
}
// Error is not handled for poll (only for debug).
#if USB_KB_DEBUG
void kbPollErrorCallback(void* signal)
{
	//	usbHidBootKeyboardData_t * p_kbd = (usbHidBootKeyboardData_t*) signal;
	// signal in this case is p_kbd
	USB_KB_TRACE("Kb Poll Error");
}
#endif
uint8_t usbHidBootKeyboardIsInstalled()
{
	usbKeyboardStruct_t *pKeyboardData = getKeyboardData();
	if (pKeyboardData)
	{
		return pKeyboardData->keyboardData[CURRENT_KEYBOARD].deviceInstalled;
	}
	else
		return 0;
}
uint32_t usbKeyboardPoll()
{
	if (!usbHidBootKeyboardIsInstalled())
		return ERROR_DEVICE_NOT_INSTALLED;
	usbKeyboardStruct_t *pKeyboardData = getKeyboardData();
	usbHidBootKeyboardData_t * p_kbd = &pKeyboardData->keyboardData[CURRENT_KEYBOARD];
	uint8_t pipe_n = p_kbd->pipeNumber;
	if ( (uint16_t) (millis16() - p_kbd->time) > p_kbd->pollInterval)
	{
		 p_kbd->time = millis16();		
	}
	else
	{
		//
		return ERROR_TOO_FREQUENT_REQUESTS;
	}
	// pipe should be already allocated now
	return usbAddTransaction(  pipe_n, 
							sizeof(p_kbd->reportBuffer), 
							NULL, 
							p_kbd->reportBuffer, 
							DIRECTION_IN, 
							USB_TRANSFER_TYPE_INTERRUPT, 
							2000,
#if USB_KB_DEBUG == 1
						    kbPollErrorCallback, 
#else					
							NULL,   // no error handling. After all we could not do anything.
#endif
							kbPollSuccessCallback,							
							p_kbd);
}
static int16_t getKeyFromDevice(usbHidBootKeyboardData_t * p_kbd)
{
	uint8_t start = p_kbd->start;
	uint8_t end = p_kbd->end;	
	int16_t r = -1;
	if (start != end)
	{
		r =	p_kbd->fifo[start];
		start++;
		if (start >= MAX_KB_FIFO_SIZE)
			start = 0;
		 p_kbd->start = start;
	}
	return r;
}
void usbSetOnKeyDown(void * callBack)
{
	usbKeyboardStruct_t *pKeyboardData = getKeyboardData();
	pKeyboardData->keyboardData[CURRENT_KEYBOARD].onKeyDown = callBack;		
}
void usbSetOnKeyUp(void *callBack)
{
	usbKeyboardStruct_t *pKeyboardData = getKeyboardData();
	pKeyboardData->keyboardData[CURRENT_KEYBOARD].onKeyUp = callBack;	
}
void usbSetOnControlKeyStateChange(void *callBack)
{
	usbKeyboardStruct_t *pKeyboardData = getKeyboardData();
	pKeyboardData->keyboardData[CURRENT_KEYBOARD].onControlKeysChanged = callBack;
}
int16_t usbGetKey()
{
	usbKeyboardStruct_t *pKeyboardData = getKeyboardData();
	if (pKeyboardData)
		return getKeyFromDevice(&pKeyboardData->keyboardData[CURRENT_KEYBOARD]);
	else
		return -1;
}
static uint8_t putKey(usbHidBootKeyboardData_t * p_kbd, uint16_t key)
{
	uint8_t start = p_kbd->start;
	uint8_t end = p_kbd->end;
	if (!( (start == 0  && end == (MAX_KB_FIFO_SIZE - 1 )) || start == (end + 1) ))
	{
		p_kbd->fifo[end] = key;
		end++;
		if (end >= MAX_KB_FIFO_SIZE)	 
			end = 0;
		p_kbd->end = end;
		return 0;
	}
	else 
		return 1;		// buffer full
	
}
#endif