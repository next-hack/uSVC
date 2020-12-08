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
#include "usvc_kernel.h" 
#include "USB_HID_Generic_Gamepad.h"
#include <stdio.h>
#include <string.h>

//#define USB_GAMEPAD_DEBUG
#ifdef USB_GAMEPAD_DEBUG
static volatile char usbDebugBuffer[40];
#define USB_GAMEPAD_TRACE(...) do { snprintf((char*)usbDebugBuffer,sizeof(usbDebugBuffer),__VA_ARGS__); addLineAtBottom( (char *) usbDebugBuffer, 1, 0);}while(0)
void usbTraceBytes(uint8_t *buffer)
{
	USB_GAMEPAD_TRACE("0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
	buffer[0],
	buffer[1],
	buffer[2],
	buffer[3],
	buffer[4],
	buffer[5],
	buffer[6],
	buffer[7]);
}
#else
#define USB_GAMEPAD_TRACE(...) do{}while(0)
#define usbTraceBytes(x) do{}while(0)
#endif

#define ITEM_USAGE_PAGE 0x04
#define ITEM_LOGICAL_MINIMUM 0x14
#define ITEM_LOGICAL_MAXIMUM 0x24
#define ITEM_REPORT_SIZE 0x74
#define ITEM_REPORT_ID 0x84
#define ITEM_REPORT_COUNT 0x94
#define ITEM_USAGE  0x08
#define ITEM_INPUT 0x80
#define ITEM_END_COLLECTION 0xC0
#define USAGE_X 0x30
#define USAGE_Y	0x31
#define USAGE_Z 0x32
#define USAGE_RX 0x33
#define USAGE_RY 0x34
#define USAGE_RZ 0x35
#define USAGE_PAGE_BUTTON 0x09
#define INPUT_CONSTANT 1
//
#if USB_GAMEPAD_DRIVER_SUPPORTS_MORE_GAMEPADS && MAX_GAMEPADS > 1
	#define CURRENT_GAMEPAD 
#else
	#define CURRENT_GAMEPAD 0
	#define OVERRIDE_INSTALLED 1
#endif
// 

//
#define VALUE_WITHIN(v,l,h) (((v)>=(l)) && ((v)<=(h)))

#if !(IS_BOOTLOADER || FORCE_INCLUDE_USB_GAMEPAD_MODULE)
uint32_t usbHidGenericGamepadInstaller(uint32_t action, usbDevice_t* pdev, void *paramPtr)
{
	uint32_t (*f)(uint32_t, usbDevice_t* , void *);
	f = (uint32_t (*)(uint32_t, usbDevice_t* , void *)) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_GENERIC_GAMEPAD_INSTALLER);
	return f(action, pdev, paramPtr);
}
uint8_t usbHidGenericGamepadIsInstalled()
{
	uint8_t (*f)();	
	f = (uint8_t (*)()) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_GENERIC_GAMEPAD_IS_INSTALLED);
	return f();	
}
uint32_t usbHidGenericGamepadPoll()
{
	uint32_t (*f)();	
	f = (uint32_t (*)()) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_GENERIC_GAMEPAD_POLL);
	return f();
}
uint8_t getCurrentGamepadState(gamePadState_t *gps)
{
	uint8_t (*f)(gamePadState_t *);	
	f = (uint8_t (*)(gamePadState_t *)) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_GENERIC_GAMEPAD_GET_STATE);
	return f(gps);
}
void setGenericGamepadInstallationCompleteCallback(void *callback)
{
	void (*f)(void *);	
	f = (void (*)(void *)) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_GENERIC_GAMEPAD_SET_INSTALLATION_COMPLETE_CALLBACK);
	f(callback);
}
#else





typedef enum
{
	GP_CREATE_PIPES,
	GP_SET_CONFIGURATION,
	GP_WAIT_FOR_SET_CONFIGURATION,
	GP_CONFIGURATION_SET,
	GP_SET_INTERFACE_PROTOCOL,
	GP_WAIT_FOR_SET_INTERFACE_PROTOCOL,
	GP_INTERFACE_PROTOCOL_SET,
	GP_REQUEST_GET_REPORT_DESCRIPTOR,
	GP_WAIT_FOR_GP_REQUEST_GET_REPORT_DESCRIPTOR,
	GP_REQUEST_GET_REPORT_DESCRIPTOR_COMPLETE,
	GP_INSTALLATION_COMPLETE,
	GP_INSTALLATION_ERROR = 0xFFFF		
} gamepad_installation_state;

usbGamepadStruct_t *getGamepadData()
{
	return (usbGamepadStruct_t *) GET_RAM_POINTER(POINTER_MODULE_USB_GAMEPAD);
}
void gamepadInstallationErrorCallback(void* signal)
{
	if (signal)
	{
		uint32_t *p = (uint32_t*) signal;
		*p = GP_INSTALLATION_ERROR;
	}
	USB_GAMEPAD_TRACE("USB KB ERROR");
}
void gamepadInstallationTransactionCompleteCallback(void* signal, int bytesReceived)
{
	(void) bytesReceived;		// remove warning
	if (signal)
	{
		uint32_t *p = (uint32_t*) signal;
		*p = *p + 1;
	}	
	
	USB_GAMEPAD_TRACE("USB KB TRANSFER COMPLETE!");
}
void parseGamepadHIDreportDescriptor(uint8_t * buffer,usbHidGamepadData_t* gamePadData)
{
	/*
		simple parser. It is not intended to be exhaustive, due to memory requirements.

	*/
	uint16_t length = gamePadData->wReportDescriptorSize;
	int32_t logicalMinimum = 0;
	int32_t logicalMaximum = 0;
	uint16_t usagePage = 0;
	uint16_t reportSize = 0;  // in bits
	uint16_t reportCount = 0;
	uint32_t usage = 0 ;
	const uint8_t bytesToRead[4] = {0, 1, 2, 4};
	uint8_t bitStartButtons = 0;
	uint8_t bitStopButtons = 0;
	uint8_t bitPosition = 0;
	uint8_t oldBitPosition = 0;
	uint8_t offsetBecauseOfReportID = 0;
	for (int i = 0; i < length; i++)
	{
		uint8_t item = buffer[i] & 0xFC;
		uint8_t numberOfBytesToRead = bytesToRead[buffer[i] & 3];
		int32_t value = 0;
		uint8_t j = 0;

		for (j = 0; j < numberOfBytesToRead; j++)
		{
			uint8_t b = buffer[i + j + 1];
			value = value | (b << (j * 8));
		}
//		USB_GAMEPAD_TRACE("i %d, j %d, item %02X, value %02X",i, j, item, value);
		// in value we have the parameter now! But what was it for?
		switch (item)
		{
			// global items of interest
			case ITEM_USAGE_PAGE:
				// reset usage too.
				usage = 0;
				usagePage = value;
				break;
			case ITEM_LOGICAL_MINIMUM:
				logicalMinimum = value;
				break;
			case ITEM_LOGICAL_MAXIMUM:
				logicalMaximum = value;
				break;
			case ITEM_REPORT_SIZE:
				reportSize  = value;
				break;
			case ITEM_REPORT_ID:
				// note: we should check that we are registering the correct report...
				offsetBecauseOfReportID = 8;		// if there is the report ID in the descriptor, then the first byte will be the report ID.
				break;
			case ITEM_REPORT_COUNT:
				reportCount = value;
				break;
			case ITEM_USAGE:
				usage = value;
				break;
			case ITEM_END_COLLECTION:
				i = length;
				break;
			case ITEM_INPUT:
				oldBitPosition = bitPosition;
				bitPosition  += reportCount  *  reportSize;
				if (!(value & INPUT_CONSTANT))
				{
					switch (usage)
					{
						case USAGE_X:
						case USAGE_Y:
						case USAGE_Z:	// x2
						case USAGE_RX:  // y2
						case USAGE_RY:  // y2 (sometimes)
						case USAGE_RZ:  // y2 (sometimes)
							USB_GAMEPAD_TRACE("Found usage XYZRX");
							gamePadData->axeSize = reportSize >> 3;
							// todo: in usage case, put numberOfAxes ++;
							if (usage - USAGE_X + 1 > 4)
								usage = USAGE_RX;
							gamePadData->gamePadState.numberOfAxes = usage - USAGE_X + 1;
							//	
							gamePadData->gamePadState.XYZRxMinimum = logicalMinimum;
							gamePadData->gamePadState.XYZRxMaximum = logicalMaximum;
							gamePadData->bitStartXYZRx = oldBitPosition +  offsetBecauseOfReportID;
							gamePadData->bitStopXYZRx = bitPosition + offsetBecauseOfReportID;
							break;
						default:	// buttons
							if (usagePage == USAGE_PAGE_BUTTON)
							{
								if (bitStopButtons == 0)		// this is the first time we encounter the buttons. We modify the start bits too.
								{
									bitStartButtons = oldBitPosition;
								}
								bitStopButtons = bitPosition;
							}
							break;
					}
				}
				break;
			//
		}
		i += numberOfBytesToRead;		// skip the data bytes;
	}	
	gamePadData->bitStartButtons = bitStartButtons + offsetBecauseOfReportID;
	gamePadData->gamePadState.numberOfButtons = bitStopButtons - bitStartButtons;
	gamePadData->reportSize = (bitPosition + offsetBecauseOfReportID + 7) >> 3;
 	USB_GAMEPAD_TRACE("Logicals: %d : %d",gamePadData->gamePadState.XYZRxMinimum, gamePadData->gamePadState.XYZRxMaximum);	
 	USB_GAMEPAD_TRACE("StartStopXY1 %d %d nAxes: %d",gamePadData->bitStartXYZRx, gamePadData->bitStopXYZRx, gamePadData->gamePadState.numberOfAxes);
 	USB_GAMEPAD_TRACE("StartStopBtn, total %d %d %d", bitStartButtons, gamePadData->gamePadState.numberOfButtons, bitPosition);
}

uint32_t usbHidGenericGamepadInstaller(uint32_t action, usbDevice_t* pdev, void *paramPtr)
{
	usbInterfaceEndpointDescriptor_t** interfaceEndpointList = paramPtr;
	usbModuleData_t *pUSB = getUSBData();	
	usbGamepadStruct_t *pGamepadData = getGamepadData();
	if (action == CHECK_DRIVER_COMPATIBILITY)
	{
		// we need to parse the interfaces found
		uint8_t dummy;
		return usbFindHidInterfaceAndEndpoint(pdev->interfacesFound, interfaceEndpointList, HID_NO_SUBCLASS, HID_PROTOCOL_NONE, &dummy, &dummy);
	}
	else if (action == INSTALL_DEVICE)
	{
		switch (pdev->installationStep)
		{
				// here we are still in the enumeration, so we can use the common endpoint buffer.
				case GP_CREATE_PIPES:
					{
						USB_GAMEPAD_TRACE("Create Pipes");
						uint8_t i;
						uint8_t e;
						usbFindHidInterfaceAndEndpoint(pdev->interfacesFound, interfaceEndpointList, HID_NO_SUBCLASS, HID_PROTOCOL_NONE, &i, &e);
						pdev->userDataPtr = &pGamepadData->gamepadData[CURRENT_GAMEPAD];
						memset(&pGamepadData->gamepadData[CURRENT_GAMEPAD], 0, sizeof(usbHidGamepadData_t));
						// good! We found a gamepad!
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
								USB_GAMEPAD_TRACE("NO ENOUGH PIPES");
								pdev->installationStep = GP_INSTALLATION_ERROR;
								return USB_NOT_ENOUGH_PIPES;
							}
							USB_GAMEPAD_TRACE("Pipe %x", pipe_n);
							pGamepadData->gamepadData[CURRENT_GAMEPAD].pollInterval = interfaceEndpointList[i]->data.hid_endpoints.endpoint[e].bInterval;
							pGamepadData->gamepadData[CURRENT_GAMEPAD].pipe_n = pipe_n;
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
								uhdPipeFree(pGamepadData->gamepadData[CURRENT_GAMEPAD].pipe_n);
								pdev->installationStep = GP_INSTALLATION_ERROR;
								return USB_NOT_ENOUGH_PIPES;
							}
							pGamepadData->gamepadData[CURRENT_GAMEPAD].pipe_ep0_n = pipe_n;
							USB_GAMEPAD_TRACE("GP Endpoints allocated!");
							pGamepadData->gamepadData[CURRENT_GAMEPAD].intrfc = i;
							pdev->installationStep = GP_SET_CONFIGURATION;
							// now get the report descriptor size. We need to access the hid descriptor, which is typically just after the interface.
							usbHidDescriptor_t * hidPtr = (usbHidDescriptor_t*) (&((uint8_t *) (interfaceEndpointList[i]))[9]);
							usbTraceBytes(hidPtr);
							if (hidPtr->bDescriptorType == HID_DESCRIPTOR_HID)
							{
								USB_GAMEPAD_TRACE("DescrSize %x", hidPtr->wDescriptorLength);	
								pGamepadData->gamepadData[CURRENT_GAMEPAD].wReportDescriptorSize = hidPtr->wDescriptorLength;
							}
						#else
							#error multi gamepad not implemented yet
						#endif					
					}
					break;
				case GP_SET_CONFIGURATION:
					USB_GAMEPAD_TRACE("GP SET CONFIG");
					usbCreateSetConfiguration(pUSB->pUsbEndpoint0ControlBuffer, 1);		// set the first config.
					usbAddTransaction(0, 0, pUSB->pUsbEndpoint0ControlBuffer, pUSB->pUsbEndpoint0Buffer, DIRECTION_OUT, USB_TRANSFER_TYPE_CONTROL, USB_TIMEOUT, gamepadInstallationErrorCallback, gamepadInstallationTransactionCompleteCallback, &pdev->installationStep);
					pdev->installationStep = GP_WAIT_FOR_SET_CONFIGURATION;
					break;
				case GP_WAIT_FOR_SET_CONFIGURATION:
					// do nothing
					break;
				case GP_CONFIGURATION_SET:
					USB_GAMEPAD_TRACE("GP SET CONFIG SET");
					pdev->installationStep = GP_SET_INTERFACE_PROTOCOL;
					break;
				case GP_SET_INTERFACE_PROTOCOL:
					USB_GAMEPAD_TRACE("GP SET IF PROTOCOL");
					usbCreateSetInterfaceProtocol(pUSB->pUsbEndpoint0ControlBuffer, pGamepadData->gamepadData[CURRENT_GAMEPAD].intrfc, HID_BOOT_PROTOCOL);
					usbAddTransaction(0, 0, pUSB->pUsbEndpoint0ControlBuffer, pUSB->pUsbEndpoint0Buffer, DIRECTION_OUT, USB_TRANSFER_TYPE_CONTROL, USB_TIMEOUT, gamepadInstallationErrorCallback, gamepadInstallationTransactionCompleteCallback, &pdev->installationStep);					
					pdev->installationStep = GP_WAIT_FOR_SET_INTERFACE_PROTOCOL;
					break;
				case GP_WAIT_FOR_SET_INTERFACE_PROTOCOL:
					break;
				case GP_INTERFACE_PROTOCOL_SET:
					USB_GAMEPAD_TRACE("KB IF PROTOCOL SET");
					pdev->installationStep = GP_REQUEST_GET_REPORT_DESCRIPTOR;
					break;
				case GP_REQUEST_GET_REPORT_DESCRIPTOR:
					memset (pUSB->pUsbEndpoint0Buffer, 0, pUSB->usbEndpoint0BufferSize);
					usbCreateGetReportDescriptor(pUSB->pUsbEndpoint0ControlBuffer, pGamepadData->gamepadData[CURRENT_GAMEPAD].intrfc, pGamepadData->gamepadData[CURRENT_GAMEPAD].wReportDescriptorSize);
					usbAddTransaction(0, pUSB->usbEndpoint0BufferSize, pUSB->pUsbEndpoint0ControlBuffer, pUSB->pUsbEndpoint0Buffer, DIRECTION_IN, USB_TRANSFER_TYPE_CONTROL, USB_TIMEOUT, gamepadInstallationErrorCallback, gamepadInstallationTransactionCompleteCallback, &pdev->installationStep);
					pdev->installationStep = GP_WAIT_FOR_GP_REQUEST_GET_REPORT_DESCRIPTOR;
					break;
				case  GP_WAIT_FOR_GP_REQUEST_GET_REPORT_DESCRIPTOR:
					break;
				case GP_REQUEST_GET_REPORT_DESCRIPTOR_COMPLETE:
					USB_GAMEPAD_TRACE("Report Descriptor L %d", pGamepadData->gamepadData[CURRENT_GAMEPAD].wReportDescriptorSize);
					usbTraceBytes(&pUSB->pUsbEndpoint0Buffer[0]);
					usbTraceBytes(&pUSB->pUsbEndpoint0Buffer[8]);
					usbTraceBytes(&pUSB->pUsbEndpoint0Buffer[16]);
					usbTraceBytes(&pUSB->pUsbEndpoint0Buffer[24]);
					usbTraceBytes(&pUSB->pUsbEndpoint0Buffer[32]);
					usbTraceBytes(&pUSB->pUsbEndpoint0Buffer[40]);					
					usbTraceBytes(&pUSB->pUsbEndpoint0Buffer[48]);
					usbTraceBytes(&pUSB->pUsbEndpoint0Buffer[56]);
					usbTraceBytes(&pUSB->pUsbEndpoint0Buffer[64]);
					usbTraceBytes(&pUSB->pUsbEndpoint0Buffer[72]);
					usbTraceBytes(&pUSB->pUsbEndpoint0Buffer[80]);
					usbTraceBytes(&pUSB->pUsbEndpoint0Buffer[88]);
					usbTraceBytes(&pUSB->pUsbEndpoint0Buffer[96]);
					usbTraceBytes(&pUSB->pUsbEndpoint0Buffer[104]);
					usbTraceBytes(&pUSB->pUsbEndpoint0Buffer[112]);
					usbTraceBytes(&pUSB->pUsbEndpoint0Buffer[120]);
					usbTraceBytes(&pUSB->pUsbEndpoint0Buffer[128]);
					usbTraceBytes(&pUSB->pUsbEndpoint0Buffer[136]);
					pdev->installationStep++;
					parseGamepadHIDreportDescriptor(&pUSB->pUsbEndpoint0Buffer[0], &pGamepadData->gamepadData[CURRENT_GAMEPAD]);
					pdev->installationStep = GP_INSTALLATION_COMPLETE;
					pdev->enumerationState = ENUMERATION_COMPLETE;
					pGamepadData->gamepadData[CURRENT_GAMEPAD].time = millis16();
					pGamepadData->gamepadData[CURRENT_GAMEPAD].deviceInstalled = 1;
					break;
				case GP_INSTALLATION_COMPLETE:
					// we should not reach this.
					break;
				case GP_INSTALLATION_ERROR:
					pdev->enumerationState = ENUMERATION_ERROR;  
					break;
		}
		return 0;
	}
	else if (action == DEVICE_INSTALLED_CALLBACK)
	{
		void (*gamepadInstallationCompleteCallback)(void) = (void (*)(void)) GET_RAM_POINTER(POINTER_GAMEPAD_INSTALLATION_CALLBACK);
		if (gamepadInstallationCompleteCallback != NULL)
			gamepadInstallationCompleteCallback();
	}
	else if (action == GET_MEMORY_REQUIREMENTS)
	{	
		return sizeof(usbGamepadStruct_t);
	}
	else if (action == SET_MEMORY)
	{
		int numGamepads = 0;
		if (pGamepadData) // is this the second time we initialize the driver?
		{
			pGamepadData = CURRENT_GAMEPAD;
			// this 
		}
		if (paramPtr)
			SET_RAM_POINTER(POINTER_MODULE_USB_GAMEPAD, paramPtr);
		pGamepadData = getGamepadData();
		pGamepadData->numGamepads = numGamepads;		// copy - or initialize - data
		pdev->modulePointer = (void**)  GET_MODULE_ADDRESS(POINTER_MODULE_USB_GAMEPAD);
	}

	return 0;
}
void setGenericGamepadInstallationCompleteCallback(void *callback)
{
	SET_RAM_POINTER(POINTER_GAMEPAD_INSTALLATION_CALLBACK, callback);
}
void genericGamepadPollSuccessCallback(void* signal, int bytesReceived)
{
	usbHidGamepadData_t * p_gp = (usbHidGamepadData_t *) signal;	
	// first set the axes gamepadState
	for (int i = 0; i < p_gp->gamePadState.numberOfAxes; i++)
	{
		p_gp->gamePadState.axes[i] = p_gp->reportBuffer[(p_gp->bitStartXYZRx >> 3) + i * p_gp->axeSize];
	}
	// next, button state.
	p_gp->gamePadState.buttons = 0;
	for (int i = 0; i < p_gp->gamePadState.numberOfButtons; i++)
	{
		uint8_t bitButton = (p_gp->bitStartButtons + i);
		uint8_t bit = (p_gp->reportBuffer[ bitButton / 8] >> (bitButton % 8)) & 1; 
		p_gp->gamePadState.buttons |= bit << i;
	}
// 	usbTraceBytes(&p_gp->reportBuffer[0]);
// 	usbTraceBytes(&p_gp->reportBuffer[8]);
	(void) bytesReceived;	// remove unused variable warning	
	// signal in this case is p_kbd
}

uint8_t getCurrentGamepadState(gamePadState_t *gps)
{
	usbGamepadStruct_t *pGamepadData = getGamepadData();
	if (!pGamepadData)
		return 0;
	memcpy(gps, &pGamepadData->gamepadData[CURRENT_GAMEPAD].gamePadState, sizeof(gamePadState_t));
	return 1;
}  


void genericGamepadPollErrorCallback(void* signal)
{
	USB_GAMEPAD_TRACE("Poll Error");
}
uint8_t usbHidGenericGamepadIsInstalled()
{
	usbGamepadStruct_t *pGamepadData = getGamepadData();
	if (pGamepadData)
		return pGamepadData->gamepadData[CURRENT_GAMEPAD].deviceInstalled;
	else
		return 0;
}
uint32_t usbHidGenericGamepadPoll()
{
	usbGamepadStruct_t *pGamepadData = getGamepadData();
	usbHidGamepadData_t * p_gp = &pGamepadData->gamepadData[CURRENT_GAMEPAD];
	if (!usbHidGenericGamepadIsInstalled())
		return ERROR_DEVICE_NOT_INSTALLED;
	uint8_t pipe_n = p_gp->pipe_n;
	if ( (uint16_t) (millis16() - p_gp->time) > p_gp->pollInterval)
	{
		 p_gp->time = millis16();		
	}
	else
	{
		//
		return ERROR_TOO_FREQUENT_REQUESTS;
	}
	// pipe should be already allocated now
	return usbAddTransaction(  pipe_n, 
							sizeof(p_gp->reportBuffer), 
							NULL, 
							p_gp->reportBuffer, 
							DIRECTION_IN, 
							USB_TRANSFER_TYPE_INTERRUPT, 
							2000,
							genericGamepadPollErrorCallback,
							genericGamepadPollSuccessCallback,							
							p_gp);
}
#endif