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
*  usb_host.c/h: USB host handler, which works together with the USVC VGA library.
*
* Credits: even if this part is strongly modified, this is based on the original
* USB Host library for Arduino, Copyright (C) 2011 Circuits At Home, LTD. All rights reserved,
* which was released under the terms of the GNU General Public License version 2 (GPL2).
*/
#include "usvc_kernel.h"
#include <string.h>
#define USB_HOST_EN_PIN 28
#define BOOST_ENABLE_PIN 14
// timeouts for errors
#define USB_SOF_MISSING_ERROR_TIMEOUT 1000UL  //
#define USB_POWER_CYCLE_DURATION 1000UL
//
// NVM Software Calibration Area Mapping
// USB TRANSN calibration value. Should be written to the USB PADCAL register.
#define NVM_USB_PAD_TRANSN_POS     45
#define NVM_USB_PAD_TRANSN_SIZE    5
// USB TRANSP calibration value. Should be written to the USB PADCAL register.
#define NVM_USB_PAD_TRANSP_POS     50
#define NVM_USB_PAD_TRANSP_SIZE    5
// USB TRIM calibration value. Should be written to the USB PADCAL register.
#define NVM_USB_PAD_TRIM_POS       55
#define NVM_USB_PAD_TRIM_SIZE      3
#define USB_PORT_MUX 6
#define USB_DM_PIN 24
#define USB_DP_PIN 25
//
// In some vide modes, we cannot direclty control PIN14. Therefore must use this trick to control the PIN...
#define setBoostPin(value) setClock0OutputValue(value)
#ifndef MAX_USB_DRIVER_MEMORY
	#define MAX_USB_DRIVER_MEMORY sizeof(USB_Keyboard_struct_t)
	#warning USB driver memory defined as sizeof (USB_Keyboard_struct_t)
#endif
__attribute__((__aligned__(4))) uint8_t staticUSBDriverMemory[MAX_USB_DRIVER_MEMORY];
void *usbStandardSingleDeviceAllocateDriverMemory(uint16_t memory, uint8_t deviceType)
{
	(void) deviceType;
	if (memory > sizeof(staticUSBDriverMemory))
		return NULL;
	else
		return staticUSBDriverMemory;
}
void initBoostEnablePin(void)
{
	setPortMux(USB_HOST_EN_PIN, PORT_PMUX_PMUXE_H_Val );  // workaround to avoid issues on some video modes.
	setPortMux(BOOST_ENABLE_PIN, PORT_PMUX_PMUXE_H_Val);  
}

//#define USB_HOST_DEBUG
#ifdef USB_HOST_DEBUG
	static volatile char usbDebugBuffer[40];
	#define USB_TRACE(...) do { snprintf((char*)usbDebugBuffer,sizeof(usbDebugBuffer),__VA_ARGS__); addLineAtBottom( (char *) usbDebugBuffer, 1, 0);}while(0)
#else
	#define USB_TRACE(...) do{}while(0)
#endif


usbModuleData_t * getUSBData()
{
	return (usbModuleData_t *) GET_RAM_POINTER(POINTER_MODULE_USB);
}
// THESE VARIABLES MUST BE DECLARED BOTH WHEN USING THE BOOTLOADER, AND WHEN USING THE APPLICATION (GAME)
usbPipe_t USB_pipe_array[NUMBER_OF_USB_PIPES];
usbModuleData_t USB_data;
usbDevice_t USB_devices[USB_NUMDEVICES];
 __attribute__((__aligned__(4))) volatile UsbHostDescriptor usbPipeTable[NUMBER_OF_USB_PIPES];
 __attribute__((__aligned__(4))) uint8_t usbEndpoint0Buffer[256];		
 __attribute__((__aligned__(4))) uint8_t usbEndpoint0ControlBuffer[8];
  // enumeration is performed only one device at once. Each device has at most 15 interfaces (unless MAX_USB_INTERFACES is defined in usvc_config.h).
  // instead of creating many instances, we just use pointers to the temporary configuration descriptor.
  // these pointers are just temporary: once the device has been configured/installed (i.e. pipe allocated)
  // this variable can be later used to install other devices. 
  usbInterfaceEndpointDescriptor_t *tmpUSBInterfaces[MAX_USB_INTERFACES];	// 
// forward function declarations
void usbHostPowerOn();
void usbTransferHandler();
// these functions must be implemented both in BOOTLOADER and application
void initUSBModule(const usbDeviceInstaller_t *pUsbInstallerList, void * (* memoryAllocationFunction)(uint16_t, uint8_t))
{
	USB_data.maxNumberOfPipes = NUMBER_OF_USB_PIPES;
	USB_data.maxNumberOfUSBdevices = USB_NUMDEVICES;
	USB_data.currentUSBdeviceNumber = 0;
	USB_data.pUsbDeviceInstallers = (usbDeviceInstaller_t *) pUsbInstallerList;
	USB_data.usbTaskState = USB_DETACHED_SUBSTATE_INITIALIZE;
	USB_data.pUsbPipes = USB_pipe_array;
	USB_data.pTmpUsbInterfaces = tmpUSBInterfaces;
	USB_data.pUsbDevices = USB_devices;
	USB_data.pUsbEndpoint0Buffer = usbEndpoint0Buffer;
	USB_data.usbEndpoint0BufferSize = sizeof usbEndpoint0Buffer;
	USB_data.pUsbEndpoint0ControlBuffer = usbEndpoint0ControlBuffer;
	USB_data.pUsbPipeTable = usbPipeTable;
	USB_data.usbDebounceTime = 0;
	USB_data.enumeratingDeviceType = -1;
	USB_data.usbHostTaskLastTime = 0;
	USB_data.pAllocateDriverMemory = memoryAllocationFunction;
	SET_RAM_POINTER(POINTER_MODULE_USB, &USB_data);
}
void usbSetUnsupportedDeviceCallBack(void *callback)
{
	usbModuleData_t *pUSB = getUSBData();
	pUSB->pUnsupportedDeviceCallback = callback;
}
void usbSetProductNameFoundCallBack(void *callback)
{
	usbModuleData_t *pUSB = getUSBData();
	pUSB->pProductNameFoundCallBack = callback;
}
void usbSetHubSupport(void *resetHubPort, void *getResetHubPortStatus)
{
	usbModuleData_t *pUSB = getUSBData();
	pUSB->pResetHubPort = resetHubPort;	
	pUSB->pGetResetHubPortStatus = getResetHubPortStatus;
}
uint32_t usbStartHubEnumeration (uint8_t *pNewAddress, uint8_t port)
{
	usbModuleData_t *pUSB = getUSBData(); 	
	// Hub enumerations starts from device 1 (which will receive later address 2). Device 0 (Address 1) is the hub itself! 
	for (int i = 1; i < pUSB->maxNumberOfUSBdevices; i++)
	{
		if (pUSB->pUsbDevices[i].enumerationState == 0)
		{
			// found one free device. Use this.
			USB_TRACE("Found new device %d", i);
			pUSB->currentUSBdeviceNumber = i;
			pUSB->usbTaskState = USB_STATE_CONFIGURING;
			if (pNewAddress)
				*pNewAddress = pUSB->currentUSBdeviceNumber + 1;
			pUSB->pUsbDevices[i].hubPort = port;
			return 0;
		}
	}
	return ERROR_TOO_MANY_DEVICES;
}
//#define FORCE_INCLUDE_USB_MODULE 1
#if !(IS_BOOTLOADER || FORCE_INCLUDE_USB_MODULE)
void usbHostInit()
{
	void (*f)(void);
	f = (void (*)(void)) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_HOST_INIT);
	f();
}
void usbHostTask()
{
	void (*f)(void);
	f = (void (*)(void)) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_HOST_TASK);
	f();
}
uint32_t uhdPipe0Alloc(uint8_t address, uint8_t ep_size)
{
	uint32_t (*f)(uint8_t, uint8_t);
	f = (uint32_t (*)(uint8_t, uint8_t)) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_PIPE0_ALLOC);
	return f(address, ep_size);
}
uint32_t uhdPipeAlloc(uint32_t ul_dev_addr, uint32_t ul_dev_ep, uint32_t ul_type, uint32_t ul_dir, uint32_t ul_ep_size, uint32_t ul_interval, uint32_t ul_nb_bank)
{
	uint32_t (*f)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
	f = (uint32_t (*)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_PIPE_ALLOC);
	return f(ul_dev_addr, ul_dev_ep, ul_type, ul_dir, ul_ep_size, ul_interval, ul_nb_bank);
}
void uhdPipeFree(uint32_t ul_pipe)
{
	void (*f)(uint32_t);
	f = (void (*)(uint32_t)) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_PIPE_FREE);
	f(ul_pipe);
}
void usbCreateStandardRequest ( volatile uint8_t *buffer, uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength)
{
	void (*f)(volatile uint8_t *, uint8_t, uint8_t, uint16_t, uint16_t, uint16_t);
	f = (void (*)(volatile uint8_t *, uint8_t, uint8_t, uint16_t, uint16_t, uint16_t)) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_CREATE_STANDARD_REQUEST);
	f(buffer, bmRequestType, bRequest, wValue, wIndex, wLength);
}
uint32_t usbAddTransaction(uint32_t pipe_n, uint32_t bufferSize, uint8_t *setupPacket, uint8_t* buffer, uint8_t direction, uint32_t type, uint32_t timeout, void *errorCallback,  void *transferCompletedCallback, void *signal)
{
	uint32_t (*f)(uint32_t, uint32_t, uint8_t *, uint8_t*, uint8_t, uint32_t, uint32_t, void *,  void *, void *);
	f = (uint32_t (*)(uint32_t, uint32_t, uint8_t *, uint8_t*, uint8_t, uint32_t, uint32_t, void *,  void *, void *)) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_ADD_TRANSACTION);
	return f(pipe_n, bufferSize, setupPacket, buffer, direction, type, timeout, errorCallback,  transferCompletedCallback, signal);
}
void usbReleaseDevice(uint8_t address)
{
	void (*f)(uint8_t address);
	f = (void (*)(uint8_t address)) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_RELEASE_DEVICE);
	f(address);
}
uint32_t usbGetTaskState ()
{
	uint32_t (*f)(void);
	f = (uint32_t (*)(void)) GET_BOOTLOADER_LIBRARY_ELEMENT(USB_GET_STATE);
	return f();
}
int usbStringDescriptor2Char (uint8_t *destBuffer, usbStringDescriptor_t *sd, int bufferSize)
{
	int (*f)(uint8_t *, usbStringDescriptor_t *, int);
	f = (int (*)(uint8_t *, usbStringDescriptor_t *, int))GET_BOOTLOADER_LIBRARY_ELEMENT(USB_STRING_DESCRIPTOR_TO_CHAR);
	return f(destBuffer, sd, bufferSize);
}
uint8_t usbFindHidInterfaceAndEndpoint(uint8_t interfacesFound, usbInterfaceEndpointDescriptor_t** interfaceEndpointList, uint8_t bInterfaceSubClass, uint8_t bInterfaceProtocol, uint8_t *foundInterfaceNumber, uint8_t *foundEndpointNumber)
{
	uint8_t (*f)(uint8_t, usbInterfaceEndpointDescriptor_t**, uint8_t, uint8_t, uint8_t *, uint8_t *);	
	f = (uint8_t (*)(uint8_t, usbInterfaceEndpointDescriptor_t**, uint8_t, uint8_t, uint8_t *, uint8_t *))GET_BOOTLOADER_LIBRARY_ELEMENT(USB_FIND_HID_INTERFACE_AND_ENDPOINT);
	return f(interfacesFound,interfaceEndpointList, bInterfaceSubClass, bInterfaceProtocol, foundInterfaceNumber,foundEndpointNumber);
}

#else
int usbStringDescriptor2Char (uint8_t *destBuffer, usbStringDescriptor_t *sd, int bufferSize)
{
	int i;
	if (sd->bDescriptorType != 3)
	return 0;
	if (bufferSize == 0)
	return 0;
	if (sd->bLength <= 2)
	{
		// if the length is 0 or it is bad, then put just the null terminator.
		destBuffer[0] = 0;
		return 0;
	}
	for (i = 0 ; (i < (sd->bLength - 2) && ((i >> 1) < (bufferSize - 1) )) ; i += 2)
	{
		destBuffer[i>>1] = sd->String[i];			// unicode :/
	}
	// add null terminator
	destBuffer[i >> 1] = 0;
	return i >> 1;
}
static void UHD_Pipe_Write(uint32_t ul_pipe, uint32_t ul_size, uint8_t *buf);
static uint32_t UHD_Pipe_Read(uint32_t pipe_num, uint32_t buf_size, uint8_t *buf);
static void UHD_Pipe_Send(uint32_t ul_pipe, uint32_t ul_token_type, uint32_t autoZLP);


uint32_t usbGetTaskState ()
{
	return getUSBData()->usbTaskState;	
}

void enumerationErrorCallback(void *signal)
{
	USB_TRACE("USB ERROR");
	if (signal)
	{
		uint32_t *p = (uint32_t*) signal;
		*p = ENUMERATION_ERROR;
	}	
}
#ifdef USB_HOST_DEBUG
void usbTraceBytes(uint8_t *buffer)
{
	USB_TRACE("0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
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
	#define usbTraceBytes(x)
#endif
void enumTransactionCompleteCallback(void *signal)
{
	USB_TRACE("USB TRANSFER COMPLETE!");
	usbTraceBytes(usbEndpoint0Buffer);
	if (signal)
	{
		uint32_t *p = (uint32_t*) signal;
		*p = *p + 1;
	}	
}
void usbReleaseDevice(uint8_t address)
{
	usbModuleData_t *pUSB = getUSBData(); 
	if (address)
	{
		memset((void*)&pUSB->pUsbDevices[address - 1], 0, sizeof (usbDevice_t));
	}
}
uint8_t parseConfigurationDescriptor(uint8_t *p_cd_buffer)
{
	usbModuleData_t *pUSB = getUSBData(); 
	uint8_t interfacesFound = 0;
	int n = 0;
	usbDescriptor_t *descriptor;
	uint16_t wTotalLength = ((usbConfigurationDescriptor_t*) p_cd_buffer)->wTotalLength;
	// now we must walk-through and validate the configuration.
	while ( n < wTotalLength)
	{
		descriptor = (usbDescriptor_t*) &p_cd_buffer[n];
		switch (descriptor->bDescriptorType)
		{
			case USB_DESCRIPTOR_CONFIGURATION:		// config descriptor. This should be the first one
			// do nothing :)
				break;
			case USB_DESCRIPTOR_HID:
				// this is a "HID" descriptor.
				// do nothing
				break;
			case 4:		// new interface found! Set the address at the corresponding buffer position!
				if (interfacesFound < MAX_USB_INTERFACES)
					pUSB->pTmpUsbInterfaces[interfacesFound] = (usbInterfaceEndpointDescriptor_t*) &p_cd_buffer[n];
				interfacesFound++;
				break;
			case 5:		// new endpoint found! Well, do nothing :)
				
			break;
		}
		n += descriptor->bLength;
	}
	return interfacesFound;
}		


uint32_t usbEnumeration(uint8_t device)
{
	usbModuleData_t *pUSB = getUSBData(); 	
	uint32_t r = USB_STATE_CONFIGURING;
	// This is to avoid warnings due to a bug in GCC...
	#if USB_NUMDEVICES == 1
		usbDevice_t * pUSBdev = (usbDevice_t *) &pUSB->pUsbDevices[0];
	#else
		usbDevice_t * pUSBdev = (usbDevice_t *) &pUSB->pUsbDevices[device];
	#endif
	// phase 1: get descriptor size
	switch (pUSBdev->enumerationState)
	{
		case FIRST_GET_DD:
			usbCreateGetDeviceDescriptorRequest8(pUSB->pUsbEndpoint0ControlBuffer);
			uhdPipe0Alloc(0, 8);		// regardless the speed, we must ask for a 8-bytes get DD the first time.
			usbAddTransaction(0, 8, pUSB->pUsbEndpoint0ControlBuffer, pUSB->pUsbEndpoint0Buffer, DIRECTION_IN, USB_TRANSFER_TYPE_CONTROL, USB_TIMEOUT, enumerationErrorCallback, enumTransactionCompleteCallback, &pUSBdev->enumerationState);
			USB_TRACE(" +SENT GET DD");
			pUSBdev->enumerationState = WAIT_FOR_FIRST_GET_DD;
			break;
		case WAIT_FOR_FIRST_GET_DD:
			// do nothing
			break;
		case FIRST_GET_DD_RECEIVED:
			{
				usbDeviceDescriptor_t *pD = (usbDeviceDescriptor_t *) pUSB->pUsbEndpoint0Buffer;
				pUSBdev->ep0Size = pD->bMaxPacketSize0;
				// wait 20 ms
				pUSBdev->time = millis16();
				pUSBdev->enumerationState = WAIT_20_MS;
			}
			break;
		case WAIT_20_MS:
			if ( (uint16_t) (millis16() - pUSBdev->time) > 20UL)
			{
				// now reset the device. Again.	
				
				if (device  == 0)			
				{
					uhdBusReset();  //issue bus reset
					USB_TRACE(" +SEND RESET");
					pUSBdev->enumerationState = WAIT_FOR_RESET_SENT;

				}
				else
				{
					USB_TRACE("HUB PORT RESET SKIPPED");
					pUSBdev->enumerationState = WAIT_FOR_RESET_SENT;
					pUSBdev->time = millis16();
					pUSBdev->enumerationState = WAIT_100MS;

				}
			}
			break;
		case WAIT_FOR_RESET_SENT:
			if (device == 0)
			{
				if (isUhdResetSent())
				{
					// wait for 100 ms
					USB_TRACE(" + RESET SENT. WAITING 100 ms");
					// Clear Bus Reset flag
					uhdAckResetSent();
					// Enable Start Of Frame generation
					uhdEnableSof();
					pUSBdev->time = millis16();
					pUSBdev->enumerationState = WAIT_100MS;

				}	
				else
				{
					if (pUSB->pGetResetHubPortStatus)
					{
						if (pUSB->pGetResetHubPortStatus())
						{
							pUSBdev->time = millis16();
							pUSBdev->enumerationState = WAIT_100MS;
						}
					}
				}
			}
			break;
		case WAIT_100MS:
			if ( (uint16_t) (millis16() - pUSBdev->time) > 100UL)
			{
				// now send a set address command. However now use the correct ep0 size.
				uhdPipe0Alloc(0, pUSBdev->ep0Size);				
				pUSBdev->address = pUSB->currentUSBdeviceNumber + 1;
				usbCreateSetAddress(pUSB->pUsbEndpoint0ControlBuffer,pUSBdev->address);
				pUSBdev->enumerationState = WAIT_FOR_SET_ADDRESS_SENT;
				usbAddTransaction(0, 0, pUSB->pUsbEndpoint0ControlBuffer, pUSB->pUsbEndpoint0Buffer, DIRECTION_OUT, USB_TRANSFER_TYPE_CONTROL, USB_TIMEOUT, enumerationErrorCallback, enumTransactionCompleteCallback, &pUSBdev->enumerationState);				
			}
			break;
		case WAIT_FOR_SET_ADDRESS_SENT:
			break;
		case SET_ADDRESS_SENT:
			pUSBdev->time = millis16();
			pUSBdev->enumerationState = WAIT_20_MS_2;
			USB_TRACE(" +ADDRESS SET, WAIT 20 ms");
			break;
		case WAIT_20_MS_2:
			if ((uint16_t)(millis16() - pUSBdev->time) > 20UL)
			{
				usbCreateGetDeviceDescriptorRequest(pUSB->pUsbEndpoint0ControlBuffer);
				uhdPipe0Alloc(pUSBdev->address, pUSBdev->ep0Size);
				pUSBdev->enumerationState = WAIT_FOR_SECOND_GET_DD;
				memset(pUSB->pUsbEndpoint0Buffer, 0, pUSB->usbEndpoint0BufferSize);
				USB_TRACE(" +SECOND GET_DD. Ep sz %d", pUSBdev->ep0Size);
				usbAddTransaction(0, pUSB->usbEndpoint0BufferSize, pUSB->pUsbEndpoint0ControlBuffer, pUSB->pUsbEndpoint0Buffer, DIRECTION_IN, USB_TRANSFER_TYPE_CONTROL, USB_TIMEOUT, enumerationErrorCallback, enumTransactionCompleteCallback, &pUSBdev->enumerationState);
			}
			break;		
		case WAIT_FOR_SECOND_GET_DD:
			break;
		case PARSE_DEVICE_DESCRIPTOR:
			{
				usbDeviceDescriptor_t *dd = (usbDeviceDescriptor_t *) pUSB->pUsbEndpoint0Buffer;
				pUSBdev->bDeviceClass = dd->bDeviceClass;
				pUSBdev->bDeviceSubClass = dd->bDeviceSubClass ;
				pUSBdev->idVendor = dd->idVendor;
				pUSBdev->idProduct = dd->idProduct;
				pUSBdev->iProduct = dd->iProduct;
				pUSBdev->enumerationState++;		
			}
			break;
		case REQUEST_PRODUCT_NAME:
			usbCreateGetStringDescriptorRequest(pUSB->pUsbEndpoint0ControlBuffer, pUSBdev->iProduct, WLANGID_ENGLISH_US, pUSB->usbEndpoint0BufferSize);
//			uhdPipe0Alloc(pUSBdev->address, pUSBdev->ep0Size);
			memset(pUSB->pUsbEndpoint0Buffer, 0, pUSB->usbEndpoint0BufferSize);
			USB_TRACE(" +GET SD");
			usbAddTransaction(0, pUSB->usbEndpoint0BufferSize, pUSB->pUsbEndpoint0ControlBuffer, pUSB->pUsbEndpoint0Buffer, DIRECTION_IN, USB_TRANSFER_TYPE_CONTROL, USB_TIMEOUT, enumerationErrorCallback, enumTransactionCompleteCallback, &pUSBdev->enumerationState);
			pUSBdev->enumerationState = WAIT_FOR_PRODUCT_NAME;
			break;
		case WAIT_FOR_PRODUCT_NAME:
			break;
		case PRINT_PRODUCT_NAME:
			pUSBdev->enumerationState++;
			{
				uint8_t tmpString[32];
				if (usbStringDescriptor2Char (tmpString, (usbStringDescriptor_t *) pUSB->pUsbEndpoint0Buffer, sizeof(tmpString) ))
				{
					if (pUSB->pProductNameFoundCallBack)
						pUSB->pProductNameFoundCallBack(tmpString);
				}
				else
				{
					if (pUSB->pProductNameFoundCallBack)
						pUSB->pProductNameFoundCallBack(NULL);
				}
									
			}
			break;
		case REQUEST_CONFIGURATION_DESCRIPTOR:
				usbCreateGetConfigurationDescriptorRequest(pUSB->pUsbEndpoint0ControlBuffer, 0 , pUSB->usbEndpoint0BufferSize);
//				uhdPipe0Alloc(pUSBdev->address, pUSBdev->ep0Size);
				pUSBdev->enumerationState = WAIT_FOR_REQUEST_CONFIGURATION_DESCRIPTOR;
				memset(pUSB->pUsbEndpoint0Buffer, 0, pUSB->usbEndpoint0BufferSize);
				USB_TRACE(" +GET CD");
				usbAddTransaction(0, pUSB->usbEndpoint0BufferSize, pUSB->pUsbEndpoint0ControlBuffer, pUSB->pUsbEndpoint0Buffer, DIRECTION_IN, USB_TRANSFER_TYPE_CONTROL, USB_TIMEOUT, enumerationErrorCallback, enumTransactionCompleteCallback, &pUSBdev->enumerationState);
			break;
		case WAIT_FOR_REQUEST_CONFIGURATION_DESCRIPTOR:
			break;
		case PARSE_CONFIGURATION_DESCRIPTOR:
			{
				#ifdef USB_HOST_DEBUG
				USB_TRACE("CD Lenght: %d", USB_pipe[0].bytesReceived);
				usbTraceBytes(&usbEndpoint0Buffer[0]);
				usbTraceBytes(&usbEndpoint0Buffer[8]);
				usbTraceBytes(&usbEndpoint0Buffer[16]);
				usbTraceBytes(&usbEndpoint0Buffer[24]);
				usbTraceBytes(&usbEndpoint0Buffer[32]);
				usbTraceBytes(&usbEndpoint0Buffer[40]);
				usbTraceBytes(&usbEndpoint0Buffer[48]);
				usbTraceBytes(&usbEndpoint0Buffer[64]);
				#endif
				pUSBdev->enumerationState++;
				pUSBdev->interfacesFound = parseConfigurationDescriptor(pUSB->pUsbEndpoint0Buffer);				
				uint8_t deviceSupported = INSTALLER_NO_DRIVERS_FOUND;
				int i = 0;
				while (pUSB->pUsbDeviceInstallers[i] != 0)
				{
					deviceSupported = pUSB->pUsbDeviceInstallers[i](CHECK_DRIVER_COMPATIBILITY, pUSBdev, pUSB->pTmpUsbInterfaces);
					if (deviceSupported == USB_DRIVER_FOUND)
					{
						pUSB->enumeratingDeviceType = i;
						pUSBdev->installationStep = 0;
						// now let's find if we can allocate enough memory
						uint16_t memory = pUSB->pUsbDeviceInstallers[i](GET_MEMORY_REQUIREMENTS, pUSBdev, pUSB->pTmpUsbInterfaces);
						// TODO: handle "no enough memory"
						if (memory)
						{
							if (pUSB->pAllocateDriverMemory)
							{
								void * address = pUSB->pAllocateDriverMemory(memory, pUSB->enumeratingDeviceType);
								pUSB->pUsbDeviceInstallers[i](SET_MEMORY, pUSBdev, address);
								if (address)
								{	// successfully allocated memory
									pUSBdev->enumerationState = CONFIGURE_DEVICE;
									memset(address, 0, memory);		// clear memory
									break;
								}								
							}
						}
						else  // no memory required (that's strange, but whatever). Good.
						{
						pUSBdev->enumerationState = CONFIGURE_DEVICE;
							pUSBdev->modulePointer = 0;
						break;
					}	
					}	
					i++;
				}
				if (deviceSupported != USB_DRIVER_FOUND)			// no device drivers found
					pUSBdev->enumerationState = DEVICE_NOT_SUPPORTED;
			}
			break;
		case CONFIGURE_DEVICE:
			pUSB->pUsbDeviceInstallers[pUSB->enumeratingDeviceType](INSTALL_DEVICE, pUSBdev, pUSB->pTmpUsbInterfaces);
			break;
		case ENUMERATION_COMPLETE:
			USB_TRACE("ENUMERATION COMPLETE!");
			pUSB->pUsbDeviceInstallers[pUSB->enumeratingDeviceType](DEVICE_INSTALLED_CALLBACK, pUSBdev, pUSB->pTmpUsbInterfaces);
			r = USB_STATE_RUNNING;
			break;
		case DEVICE_NOT_SUPPORTED:
			USB_TRACE("NO driver for the attached device!");
			if (pUSB->pUnsupportedDeviceCallback)
				pUSB->pUnsupportedDeviceCallback(pUSBdev);
			pUSBdev->enumerationState = ENUMERATION_ERROR;
			break;
// 		case ENUMERATION_RESET_REQUIRED:
// 			pUSBdev->enumerationState = FIRST_GET_DD;
// 			r = USB_ATTACHED_SUBSTATE_SETTLE_GET_TIME;
// 			usbHostInit();
// 			break;
		case ENUMERATION_ERROR:
			usbHostInit();
			r = USB_STATE_DETACHED;	
			break;
		
	}
	return r;
}
uint8_t usbFindHidInterfaceAndEndpoint(uint8_t interfacesFound, usbInterfaceEndpointDescriptor_t** interfaceEndpointList, uint8_t bInterfaceSubClass, uint8_t bInterfaceProtocol, uint8_t *foundInterfaceNumber, uint8_t *foundEndpointNumber)
{
	for (int i = 0; i < interfacesFound; i++)
	{
		if (interfaceEndpointList[i]->interfaceDescriptor.bInterfaceClass == USB_CLASS_HID &&
		interfaceEndpointList[i]->interfaceDescriptor.bInterfaceSubClass == bInterfaceSubClass &&
		interfaceEndpointList[i]->interfaceDescriptor.bInterfaceProtocol == bInterfaceProtocol)
		{
			for (int j = 0; j < interfaceEndpointList[i]->interfaceDescriptor.bNumEndpoints; j++)
			{
				if ( (interfaceEndpointList[i]->data.hid_endpoints.endpoint[j].bEndpointAddress & 0x80) &&
				(interfaceEndpointList[i]->data.hid_endpoints.endpoint[j].bmAttributes & 0x3) ==USB_TRANSFER_TYPE_INTERRUPT)
				{
					*foundInterfaceNumber = i;
					*foundEndpointNumber = j;
					return USB_DRIVER_FOUND;
				}
			}
		}
	}
	return USB_DRIVER_NOT_COMPATIBLE;
}

uint32_t usbAddTransaction(uint32_t pipe_n, uint32_t bufferSize, uint8_t *setupPacket, uint8_t* buffer, uint8_t direction, uint32_t type, uint32_t timeout, void *errorCallback,  void *transferCompletedCallback, void *signal)
{
	usbModuleData_t *pUSB = getUSBData(); 
	if (pipe_n >= pUSB->maxNumberOfPipes)
		return ERROR_INVALID_PIPE;
	usbPipe_t *pPipe = &pUSB->pUsbPipes[pipe_n];
	if (!pPipe->isAssociated)
		return ERROR_PIPE_NOT_ASSOCIATED;
	if (pPipe->state != STATE_IDLE)
		return ERROR_PIPE_BUSY;
	//if (USB->HOST.HostPipe[pipe_n].PCFG.bit.PTYPE != USB_HOST_PTYPE_DIS_val && pipe_n != 0)
	// the pipe is free here.
	pPipe->bufferSize = bufferSize;
	pPipe->pBuffer = buffer;
	pPipe->direction = direction;
	pPipe->timeout = timeout;
	pPipe->time = millis16();
	pPipe->bytesReceived = 0;
	pPipe->errorCallback = errorCallback;
	pPipe->pSetupPacket = setupPacket;
	pPipe->transferCompletedCallback = transferCompletedCallback;
	pPipe->signal = signal;
	switch (type)
	{
		case USB_TRANSFER_TYPE_CONTROL:
			UHD_Pipe_Write(pipe_n, 8, pPipe->pSetupPacket);
			pPipe->state = CONTROL_REQUEST_SETUP_STAGE_TRANSMISSION;
			break;
		case USB_TRANSFER_TYPE_INTERRUPT:
		case USB_TRANSFER_TYPE_BULK:
			if (direction == DIRECTION_IN)
			{
				pPipe->state = WAITING_FOR_IN_TRANSMISSION_COMPLETE;
				UHD_Pipe_Read(pipe_n, bufferSize, buffer);
			}
			else
			{
				pPipe->state = WAITING_FOR_OUT_TRANSMISSION_COMPLETE;
				UHD_Pipe_Write(pipe_n, bufferSize, buffer);
				UHD_Pipe_Send(pipe_n, USB_HOST_PCFG_PTOKEN_OUT, 1);	// ZLP set to 1
			}
			break;
	}
	return 0;	// no error
}


// init the USB host. 
void usbHostInit()
{
	
	usbModuleData_t *pUSB = getUSBData(); 	
	// We check if there was a device that has been disconnected, and if there was some memory allocated, unallocate it.
	for (int i = 0; i <= pUSB->currentUSBdeviceNumber; i++)
	{
		if (pUSB->pUsbDevices[i].modulePointer)
		{
			// we should call a free-like function, but we use static allocation.
			*pUSB->pUsbDevices[i].modulePointer = NULL;
			pUSB->pUsbDevices[i].modulePointer  = NULL;
		}
	}
	uint32_t pad_transn;
	uint32_t pad_transp;
	uint32_t pad_trim;
	initBoostEnablePin();
	/* Enable USB clock */
	PM->APBBMASK.reg |= PM_APBBMASK_USB;

	/* Set up the USB DP/DM pins */
	setPortMux(USB_DM_PIN, USB_PORT_MUX);
	setPortMux(USB_DP_PIN, USB_PORT_MUX );
	// CPU and USB share the same 48 MHz clock.
	/* ----------------------------------------------------------------------------------------------
	* Put Generic Clock Generator 0 as source for Generic Clock Multiplexer 6 (USB reference)
	*/
	#ifdef FORCE_USB_DFLL48MHZ
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(6) |        // Generic Clock Multiplexer 6
		GCLK_CLKCTRL_ID_DFLL48 |    // Generic Clock Generator 0 is source
		GCLK_CLKCTRL_CLKEN;
	#else
		GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(6) |        // Generic Clock Multiplexer 6
		GCLK_CLKCTRL_GEN_GCLK0 |    // Generic Clock Generator 0 is source
		GCLK_CLKCTRL_CLKEN;

	#endif
	
	/* Reset */
	USB->HOST.CTRLA.reg = USB_CTRLA_SWRST;
	// NOTE: THE FOLLOWING SYNC WAIT IS MANDATORY!!!!
	while (USB->HOST.SYNCBUSY.bit.SWRST);

	/* Load Pad Calibration */
	pad_transn = (*((uint32_t *)(NVMCTRL_OTP4)       // Non-Volatile Memory Controller
					+ (NVM_USB_PAD_TRANSN_POS / 32))
					>> (NVM_USB_PAD_TRANSN_POS % 32))
				& ((1 << NVM_USB_PAD_TRANSN_SIZE) - 1);

	if (pad_transn == 0x1F)         // maximum value (31)
	{
		pad_transn = 5;
	}


	pad_transp = (*((uint32_t *)(NVMCTRL_OTP4)
					+ (NVM_USB_PAD_TRANSP_POS / 32))
					>> (NVM_USB_PAD_TRANSP_POS % 32))
				& ((1 << NVM_USB_PAD_TRANSP_SIZE) - 1);

	if (pad_transp == 0x1F)         // maximum value (31)
	{
		pad_transp = 29;
	}


	pad_trim = (*((uint32_t *)(NVMCTRL_OTP4)
					+ (NVM_USB_PAD_TRIM_POS / 32))
				>> (NVM_USB_PAD_TRIM_POS % 32))
				& ((1 << NVM_USB_PAD_TRIM_SIZE) - 1);

	if (pad_trim == 0x7)         // maximum value (7)
	{
		pad_trim = 3;
	}
//  Don't do this
// 	USB->HOST.PADCAL.bit.TRANSN = pad_transn;
// 	USB->HOST.PADCAL.bit.TRANSP = pad_transp;
// 	USB->HOST.PADCAL.bit.TRIM = pad_trim;
//  Do this instead! (and you'll save 40 bytes!)
	USB->HOST.PADCAL.reg = USB_PADCAL_TRANSP(pad_transp) | USB_PADCAL_TRANSN(pad_transn) | USB_PADCAL_TRIM(pad_trim);

	/* Set the configuration */
	// enable USB
	USB->HOST.CTRLA.reg |= USB_CTRLA_ENABLE | USB_CTRLA_MODE | USB_CTRLA_RUNSTDBY;
//	uhd_force_host_mode();
//	USB->HOST.CTRLA.reg |= USB_CTRLA_RUNSTDBY;
	memset((uint8_t*)pUSB->pUsbPipeTable, 0 , sizeof(UsbHostDescriptor) * pUSB->maxNumberOfPipes);
	memset((uint8_t*)pUSB->pUsbDevices, 0, sizeof(usbDevice_t) * pUSB->maxNumberOfUSBdevices);
	memset(pUSB->pUsbPipes, 0, sizeof(usbPipe_t) * pUSB->maxNumberOfPipes);
	// Set address of USB SRAM
	USB->HOST.DESCADD.reg = (uint32_t)(&pUSB->pUsbPipeTable[0]);
	// For USB_SPEED_FULL
	uhdForceFullSpeed();
	// note: we should delete all the ram pointers here... for all the devices
	pUSB->currentUSBdeviceNumber = 0;
	// Put VBUS on USB port
	usbHostPowerOn();
	#ifdef USB_USE_INTERRUPT
		USB->HOST.INTENSET.reg = USB_HOST_INTENSET_RAMACER;	
		USB->HOST.INTENSET.reg = USB_HOST_INTENSET_DCONN;
		USB->HOST.INTENSET.reg = USB_HOST_INTENSET_WAKEUP;
		USB->HOST.INTENSET.reg = USB_HOST_INTENSET_DDISC;	
		// Configure interrupts
		NVIC_SetPriority((IRQn_Type)USB_IRQn, 2UL);		// set at priority 2 as VGA has pri 1 and 0.
		NVIC_EnableIRQ((IRQn_Type)USB_IRQn);	
	#endif		
	USB_TRACE("Usb initialized");
}
void usbHostPowerOn()
{
	setBoostPin(1);
	USB->HOST.CTRLB.bit.VBUSOK = 1;	
}
void usb_host_power_off()
{
	USB->HOST.CTRLB.bit.VBUSOK = 0;
	setBoostPin(0);
}

uint32_t uhdPipe0Alloc(uint8_t address, uint8_t ep_size)
{
	uint8_t pkt_size_code;
	usbModuleData_t *pUSB = getUSBData();	
	if( USB->HOST.STATUS.reg & USB_HOST_STATUS_SPEED(1))
	{
		pkt_size_code = USB_PCKSIZE_SIZE_8_BYTES;  // Low Speed
		USB_TRACE("LOW SPEED: EPSZ 8");
	}
	else
		{
		for (pkt_size_code = 0; pkt_size_code < 8; pkt_size_code++)
		{
			if ((1 << (pkt_size_code + 3)) == ep_size)
				break;
		}	
		}	
	USB->HOST.HostPipe[0].PCFG.bit.PTYPE = USB_HOST_PTYPE_CTRL_val;
	// Don't do this.
	//pUSB->p_usb_pipe_table[0].HostDescBank[0].CTRL_PIPE.bit.PEPNUM = 0;
	//pUSB->p_usb_pipe_table[0].HostDescBank[0].CTRL_PIPE.bit.PDADDR = address;
	//pUSB->p_usb_pipe_table[0].HostDescBank[0].CTRL_PIPE.bit.PERMAX = MAX_USB_ERRORS;
	// Do this instead and save 24 bytes!
	pUSB->pUsbPipeTable[0].HostDescBank[0].CTRL_PIPE.reg = USB_HOST_CTRL_PIPE_PERMAX(MAX_USB_ERRORS) | USB_HOST_CTRL_PIPE_PDADDR(address) | USB_HOST_CTRL_PIPE_PEPNUM(0);
	//
	pUSB->pUsbPipeTable[0].HostDescBank[0].PCKSIZE.bit.SIZE = pkt_size_code;

	pUSB->pUsbPipes[0].isAssociated = 1;
	return 0;
}



/**
 * \brief Allocate a new pipe.

 * \param ul_dev_addr Address of remote device.
 * \param ul_dev_ep Targeted endpoint of remote device.
 * \param ul_type Pipe type.
 * \param ul_dir Pipe direction.
 * \param ul_ep_size Pipe size.
 * \param ul_interval Polling interval (if applicable to pipe type).
 * \param ul_nb_bank Number of banks associated with this pipe.
 *
 * \return the pipe number allocated.
 */
uint32_t uhdPipeAlloc(uint32_t ul_dev_addr, uint32_t ul_dev_ep, uint32_t ul_type, uint32_t ul_dir, uint32_t ul_ep_size , uint32_t ul_interval, uint32_t ul_nb_bank)
{
	uint8_t pkt_size_code;
	usbModuleData_t *pUSB = getUSBData();
	/* set pipe config */
	uint8_t pipeNumber = 0;
	for (int i = 1; i < NUMBER_OF_USB_PIPES; i++)	// pipe 0 is always used for enumeration.
	{
			if (!pUSB->pUsbPipes[i].isAssociated)
			{
				pipeNumber = i;
				break;	
			}
	}
	if (!pipeNumber)	// no pipe free available
		return 0;	//i.e. return invalid pipe (0 is only for enumeration)

	USB->HOST.HostPipe[pipeNumber].BINTERVAL.reg  = ul_interval;
	uint8_t token;
	if (ul_dir & USB_EP_DIR_IN)
	{
		//USB->HOST.HostPipe[pipeNumber].PCFG.bit.PTOKEN = USB_HOST_PCFG_PTOKEN_IN;
		token = USB_HOST_PCFG_PTOKEN_IN;
		USB->HOST.HostPipe[pipeNumber].PSTATUSSET.reg  = USB_HOST_PSTATUSSET_BK0RDY;
	}
	else
	{
		//USB->HOST.HostPipe[pipeNumber].PCFG.bit.PTOKEN = USB_HOST_PCFG_PTOKEN_OUT;
		token = USB_HOST_PCFG_PTOKEN_OUT;
		USB->HOST.HostPipe[pipeNumber].PSTATUSCLR.reg  = USB_HOST_PSTATUSCLR_BK0RDY;
	}
	// Don't do this....
	//USB->HOST.HostPipe[pipeNumber].PCFG.bit.BK    = ul_nb_bank;
	//USB->HOST.HostPipe[pipeNumber].PCFG.bit.PTYPE = ul_type;
	//USB->HOST.HostPipe[pipeNumber].PCFG.bit.PTOKEN = token;		// actually it was a bit different.
	// Do this instead and save 44 bytes!
	USB->HOST.HostPipe[pipeNumber].PCFG.reg = USB_HOST_PCFG_BK * (0 != ul_nb_bank) | USB_HOST_PCFG_PTOKEN(token) | USB_HOST_PCFG_PTYPE(ul_type);
	if( USB->HOST.STATUS.reg & USB_HOST_STATUS_SPEED(1) )
	   pkt_size_code = USB_PCKSIZE_SIZE_8_BYTES;  // Low Speed
	else
	{
		for (pkt_size_code = 0; pkt_size_code < 8; pkt_size_code++)
		{
			if ((1 << (pkt_size_code + 3)) == ul_ep_size)
				break;
		}
	}
	memset((uint8_t *)&pUSB->pUsbPipeTable[pipeNumber], 0, sizeof(UsbHostDescriptor));
	//
	pUSB->pUsbPipeTable[pipeNumber].HostDescBank[0].PCKSIZE.bit.SIZE     = pkt_size_code;
	// Don't do this!
// 	pUSB->p_usb_pipe_table[pipeNumber].HostDescBank[0].CTRL_PIPE.bit.PDADDR = ul_dev_addr;
// 	pUSB->p_usb_pipe_table[pipeNumber].HostDescBank[0].CTRL_PIPE.bit.PEPNUM = ul_dev_ep;
// 	pUSB->p_usb_pipe_table[pipeNumber].HostDescBank[0].CTRL_PIPE.bit.PERMAX = MAX_USB_ERRORS;
	// Do this instead and save 24 bytes!
	pUSB->pUsbPipeTable[pipeNumber].HostDescBank[0].CTRL_PIPE.reg = USB_HOST_CTRL_PIPE_PERMAX(MAX_USB_ERRORS) | USB_HOST_CTRL_PIPE_PDADDR(ul_dev_addr) | USB_HOST_CTRL_PIPE_PEPNUM(ul_dev_ep);
	//
	pUSB->pUsbPipes[pipeNumber].isAssociated = 1;
	return pipeNumber;
}

/**
 * \brief Free a pipe.
 *
 * \param ul_pipe Pipe number to free.
 */
void uhdPipeFree(uint32_t ul_pipe)
{
	usbModuleData_t *pUSB = getUSBData();
	if (ul_pipe < NUMBER_OF_USB_PIPES)
	{
		// The Pipe is frozen and no additional requests will be sent to the device on this pipe address.
		usbPipe_t *pPipe = &pUSB->pUsbPipes[ul_pipe];
		USB->HOST.HostPipe[ul_pipe].PSTATUSSET.reg = USB_HOST_PSTATUSSET_PFREEZE;
		pPipe->isAssociated = 0;
		pPipe->state = STATE_IDLE;		
	}
}


/**
 * \brief Read from a pipe.
 *
 * \param ul_pipe Pipe number.
 * \param ul_size Maximum number of data to read.
 * \param data Buffer to store the data.
 *
 * \return number of data read.
 */
static uint32_t UHD_Pipe_Read(uint32_t pipe_num, uint32_t buf_size, uint8_t *buf)
{
	usbModuleData_t *pUSB = getUSBData();
   if (USB->HOST.HostPipe[pipe_num].PCFG.bit.PTYPE == USB_HOST_PTYPE_DIS_val)
   {
      return 0;
   }

   /* get pipe config from setting register */
   pUSB->pUsbPipeTable[pipe_num].HostDescBank[0].ADDR.reg = (uint32_t)buf;
	// don't do this
// 	pUSB->p_usb_pipe_table[pipe_num].HostDescBank[0].PCKSIZE.bit.BYTE_COUNT        = 0;
// 	pUSB->p_usb_pipe_table[pipe_num].HostDescBank[0].PCKSIZE.bit.MULTI_PACKET_SIZE = buf_size;
// 	pUSB->p_usb_pipe_table[pipe_num].HostDescBank[0].PCKSIZE.bit.AUTO_ZLP = 0;
	// do this instead and save 12 bytes. (note: we explicitly wrote USB_HOST_PCKSIZE_AUTO_ZLP * 0 to remark NO auto zlp.
	pUSB->pUsbPipeTable[pipe_num].HostDescBank[0].PCKSIZE.reg = (pUSB->pUsbPipeTable[pipe_num].HostDescBank[0].PCKSIZE.reg & USB_HOST_PCKSIZE_SIZE_Msk) | // preserve old SIZE
					USB_HOST_PCKSIZE_BYTE_COUNT(0) | USB_HOST_PCKSIZE_MULTI_PACKET_SIZE(buf_size) | USB_HOST_PCKSIZE_AUTO_ZLP * 0;
   USB->HOST.HostPipe[pipe_num].PCFG.bit.PTOKEN = USB_HOST_PCFG_PTOKEN_IN;

   /* Start transfer */
	//  USB->HOST.HostPipe[pipe_num].PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_BK0RDY;
   // Unfreeze pipe
	// USB->HOST.HostPipe[pipe_num].PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_PFREEZE;
	// (we joined together these two operations)
	USB->HOST.HostPipe[pipe_num].PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_PFREEZE | USB_HOST_PSTATUSCLR_BK0RDY; 
   return buf_size;
}


/**
 * \brief Write into a pipe.
 *
 * \param ul_pipe Pipe number.
 * \param ul_size Maximum number of data to read.
 * \param data Buffer containing data to write.
 */
static void UHD_Pipe_Write(uint32_t ul_pipe, uint32_t ul_size, uint8_t *buf)
{
	usbModuleData_t *pUSB = getUSBData();
   /* get pipe config from setting register */
   pUSB->pUsbPipeTable[ul_pipe].HostDescBank[0].ADDR.reg = (uint32_t)buf;
   // Don't do this
//    pUSB->p_usb_pipe_table[ul_pipe].HostDescBank[0].PCKSIZE.bit.BYTE_COUNT = ul_size;
//    pUSB->p_usb_pipe_table[ul_pipe].HostDescBank[0].PCKSIZE.bit.MULTI_PACKET_SIZE = 0;
	// Do this instead and save 8 bytes
	pUSB->pUsbPipeTable[ul_pipe].HostDescBank[0].PCKSIZE.reg = (pUSB->pUsbPipeTable[ul_pipe].HostDescBank[0].PCKSIZE.reg & (USB_HOST_PCKSIZE_SIZE_Msk | ( 1 << USB_HOST_PCKSIZE_AUTO_ZLP_Pos))) | // preserve old SIZE and auto zlp
		USB_HOST_PCKSIZE_BYTE_COUNT(ul_size) | USB_HOST_PCKSIZE_MULTI_PACKET_SIZE(0);
	
}

/**
 * \brief Send a pipe content.
 *
 * \param ul_pipe Pipe number.
 * \param ul_token_type Token type.
 */


static void UHD_Pipe_Send(uint32_t ul_pipe, uint32_t ul_token_type, uint32_t autoZLP)
{
	usbModuleData_t *pUSB = getUSBData();
	USB->HOST.HostPipe[ul_pipe].PCFG.bit.PTOKEN = ul_token_type;
   /* Start transfer */
	if(ul_token_type == USB_HOST_PCFG_PTOKEN_SETUP )
	{
		pUSB->pUsbPipeTable[ul_pipe].HostDescBank[0].PCKSIZE.bit.AUTO_ZLP = 0;
		USB->HOST.HostPipe[ul_pipe].PINTFLAG.reg = USB_HOST_PINTFLAG_TXSTP;
		USB->HOST.HostPipe[ul_pipe].PSTATUSSET.reg = USB_HOST_PSTATUSSET_BK0RDY;
		// don't do this:
		//USB->HOST.HostPipe[ul_pipe].PSTATUSCLR.bit.DTGL = 1;	// each setup phase begins with data toggle 0.
		// do this instead, and save 8 bytes!
		USB->HOST.HostPipe[ul_pipe].PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_DTGL;
	}
	else if(ul_token_type == USB_HOST_PCFG_PTOKEN_IN )
	{
		pUSB->pUsbPipeTable[ul_pipe].HostDescBank[0].PCKSIZE.bit.AUTO_ZLP = autoZLP;
		USB->HOST.HostPipe[ul_pipe].PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_BK0RDY;
		//
	}
	else
	{
		pUSB->pUsbPipeTable[ul_pipe].HostDescBank[0].PCKSIZE.bit.AUTO_ZLP = autoZLP;
		USB->HOST.HostPipe[ul_pipe].PINTFLAG.reg = USB_HOST_PINTFLAG_TRCPT(1);  // Transfer Complete 0
		USB->HOST.HostPipe[ul_pipe].PSTATUSSET.reg = USB_HOST_PSTATUSSET_BK0RDY;
	}
   
	// Unfreeze pipe
    uhdUnfreezePipe(ul_pipe);
}

/**
 * \brief Check for pipe transfer completion.
 *
 * \param ul_pipe Pipe number.
 * \param ul_token_type Token type.
 *
 * \retval 0 transfer is not complete.
 * \retval 1 transfer is complete.
 */
uint32_t UHD_Pipe_Is_Transfer_Complete(uint32_t ul_pipe, uint32_t ul_token_type)
{
   // Check for transfer completion depending on token type
   switch (ul_token_type)
   {
      case USB_HOST_PCFG_PTOKEN_SETUP:
         if (isUhdSetupReady(ul_pipe))
         {
            uhdAckSetupReady(ul_pipe);
            uhdFreezePipe(ul_pipe);
            return 1;
         }
		 break;

      case USB_HOST_PCFG_PTOKEN_IN:
         if (isUhdInReceived(ul_pipe))
         {
            // IN packet received
            uhdAckInReceived(ul_pipe);
			// Freeze will stop after the transfer
			uhdFreezePipe(ul_pipe);
            return 1;
         }
		 break;
 
      case USB_HOST_PCFG_PTOKEN_OUT:
         if (isUhdOutReady(ul_pipe))
         {
            // OUT packet sent
            uhdAckOutReady(ul_pipe);
            uhdFreezePipe(ul_pipe);
            return 1;
         }
		 break;
   }

   return 0;
}



void usbCreateStandardRequest ( volatile uint8_t *buffer, uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength)
{
	buffer[0] = bmRequestType;
	buffer[1] = bRequest;
	buffer[2] = wValue & 0xFF;
	buffer[3] = wValue >> 8;
	buffer[4] = wIndex & 0xFF;
	buffer[5] = wIndex >> 8;
	buffer[6] = wLength & 0xFF;
	buffer[7] = wLength >> 8;
}
void usbHostTask()
{
	usbModuleData_t *pUSB = getUSBData(); 
	// This function is called in polling. The reason is that the VGA might be busy and we want to handle things during the vertical
	// blank, i.e. when we have a lot of time.
	uint8_t tmpdata = 0;
	if (USB->HOST.STATUS.bit.LINESTATE && (pUSB->usbTaskState & USB_STATE_MASK) == USB_STATE_DETACHED)
	{
		if ((uint16_t)(millis16() - pUSB->usbDebounceTime) >= 100UL)
		{
			tmpdata = UHD_STATE_CONNECTED;
		}
	}
	else
	{  // reset counter
		if ((pUSB->usbTaskState & USB_STATE_MASK) == USB_STATE_DETACHED)
			pUSB->usbDebounceTime = millis16();
	}
	if (USB->HOST.STATUS.bit.LINESTATE == 0 && (pUSB->usbTaskState & USB_STATE_MASK) != USB_STATE_DETACHED)
	{
		if ((uint32_t)(millis16() - pUSB->usbDebounceTime) >= 100UL)
		{
			tmpdata = UHD_STATE_DISCONNECTED;
		}
	}
	else
	{	// reset counter.
		if ((pUSB->usbTaskState & USB_STATE_MASK) != USB_STATE_DETACHED)
			pUSB->usbDebounceTime = millis16();
	}
	if (!USB->HOST.CTRLB.bit.VBUSOK)		// don't do anything if VBUS is not ok
	{
		tmpdata = 0;
	}
	//modify USB task state if Vbus changed 
	if (tmpdata == UHD_STATE_DISCONNECTED)
	{
		if ((pUSB->usbTaskState & USB_STATE_MASK) != USB_STATE_DETACHED)
		{
			pUSB->usbTaskState = USB_DETACHED_SUBSTATE_INITIALIZE;
		}
	}
	else if (tmpdata == UHD_STATE_CONNECTED)
	{		
		if ((pUSB->usbTaskState & USB_STATE_MASK) == USB_STATE_DETACHED) 
		{
			pUSB->usbTaskState = USB_ATTACHED_SUBSTATE_SETTLE_GET_TIME;
		}
	}
	// Perform USB enumeration stage and clean up
	switch (pUSB->usbTaskState) 
	{
		case USB_DETACHED_SUBSTATE_INITIALIZE:
			// Init USB stack and driver
			usbHostInit();
			pUSB->usbTaskState = USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE;
			break;
		case USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE:  //just sit here
			break;
// 		case USB_DETACHED_SUBSTATE_ILLEGAL: 
// 			usbHostInit();			
// 			usb_task_state = USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE;
// 			break;
		case USB_ATTACHED_SUBSTATE_SETTLE_GET_TIME:
			pUSB->usbHostTaskLastTime = millis16();				
			pUSB->usbTaskState = USB_ATTACHED_SUBSTATE_SETTLE;
		break;
		case USB_ATTACHED_SUBSTATE_SETTLE: // Settle time for just attached device
			if ((uint16_t )(millis16()  - pUSB->usbHostTaskLastTime) >=  USB_SETTLE_DELAY)
				pUSB->usbTaskState = USB_ATTACHED_SUBSTATE_RESET_DEVICE;
			break;
		case USB_ATTACHED_SUBSTATE_RESET_DEVICE:
			uhdBusReset();  //issue bus reset
			pUSB->usbTaskState = USB_ATTACHED_SUBSTATE_WAIT_RESET_COMPLETE;
			break;
		case USB_ATTACHED_SUBSTATE_WAIT_RESET_COMPLETE:
			if (isUhdResetSent())
			{
				// Clear Bus Reset flag
				uhdAckResetSent();

				// Enable Start Of Frame generation
				uhdEnableSof();

				pUSB->usbTaskState = USB_ATTACHED_SUBSTATE_WAIT_SOF;

				// Wait 20ms after Bus Reset (USB spec)
				pUSB->usbHostTaskLastTime = millis16();
			}
		break;			
		case USB_ATTACHED_SUBSTATE_WAIT_SOF:
			// Wait for SOF received first
			if (isUhdSof())
			{
				
				if ((uint16_t )(millis16()  - pUSB->usbHostTaskLastTime) >=  USB_SETTLE_DELAY)
				{
					// 20ms waiting elapsed
					pUSB->usbTaskState = USB_STATE_CONFIGURING;
					pUSB->pUsbDevices[pUSB->currentUSBdeviceNumber].enumerationState = FIRST_GET_DD;
				}
			}
			else
			{
				// if sof is not coming between 1 sec, then there is something screwed. Let's have a full reset.
				if ((uint16_t) (millis16() - pUSB->usbHostTaskLastTime) >= USB_SOF_MISSING_ERROR_TIMEOUT)
				{
					usb_host_power_off();
					pUSB->usbTaskState = USB_ATTACHED_SUBSTATE_SOF_MISSING;
					pUSB->usbHostTaskLastTime = millis16();
				}
			}
			break;
		case USB_ATTACHED_SUBSTATE_SOF_MISSING:
			if ((uint16_t) (millis16() - pUSB->usbHostTaskLastTime) >= USB_POWER_CYCLE_DURATION)
			{
				pUSB->usbTaskState = USB_DETACHED_SUBSTATE_INITIALIZE;
			}
			break;
		case USB_STATE_CONFIGURING:
			//USB_TRACE(" + USB_STATE_CONFIGURING");
			pUSB->usbTaskState = usbEnumeration(pUSB->currentUSBdeviceNumber);
			if (pUSB->usbTaskState == USB_STATE_CONFIGURING)
				usbTransferHandler();
			break;
		case USB_STATE_RUNNING:
			usbTransferHandler();
			break;
		case USB_STATE_ERROR:
		
			pUSB->usbTaskState = USB_DETACHED_SUBSTATE_INITIALIZE;
			break;	
	} // switch( usb_task_state )

}
/*
	usbTransferHandler
	For each pipe, this function checks the status. Based on this, it performs actions. 
*/

void usbTransferHandler()
{
	usbModuleData_t *pUSB = getUSBData();
	for (int pipe_n = 0; pipe_n < NUMBER_OF_USB_PIPES; pipe_n++)
	{
		// check if pipe is allocated first.
		if (USB->HOST.HostPipe[pipe_n].PCFG.bit.PTYPE != USB_HOST_PTYPE_DIS_val)
		{
			usbPipe_t *pPipe =  &pUSB->pUsbPipes[pipe_n];
			//uint32_t pintflag = USB->HOST.HostPipe[pipe_n].PINTFLAG.reg;
			//uint32_t status_pipe =  usb_pipe_table[pipe_n].HostDescBank[0].STATUS_PIPE.reg;
			pipeState_t state = pPipe->state;
			if (( (uint16_t) (millis16() - pPipe->time) >pPipe->timeout) && state != STATE_IDLE && pPipe->timeout != 0)
			{
				pPipe->state = STATE_ERROR;
				state = STATE_ERROR;
			}
			// get pipe status
			switch (state)
			{
				// control request state machine
				default:
					break;
				case CONTROL_REQUEST_SETUP_STAGE_TRANSMISSION:
					// the user already created the control request. Then we need to send it.
					pPipe->state = CONTROL_WAITING_FOR_SETUP_STAGE_COMPLETE;
					usbTraceBytes(pPipe->pSetupPacket);
					UHD_Pipe_Send(pipe_n, USB_HOST_PCFG_PTOKEN_SETUP, 0);
					pPipe->time = millis16();
					break;
				case CONTROL_WAITING_FOR_SETUP_STAGE_COMPLETE:
					if (UHD_Pipe_Is_Transfer_Complete(pipe_n, USB_HOST_PCFG_PTOKEN_SETUP))
					{
						// yes. Then check what we need to do.
						if (pPipe->direction == DIRECTION_IN)
						{					
							pPipe->state = CONTROL_WAITING_FOR_DATA_IN_STAGE_RECEPITON_COMPLETE;
							UHD_Pipe_Read(pipe_n, pPipe->bufferSize, pPipe->pBuffer);
							USB_TRACE("WAIT FOR DSRC");
						}
						else
						{  // out transfer
							if (pPipe->bufferSize == 0)	// this was a control without any data stage. We expect a ZLP packet.
							{
								pPipe->state = CONTROL_WAITING_FOR_ZLP_RECEPTION;	// we skip CONTROL_WAITING_FOR_DATA_OUT_STAGE_TRASMISSION_COMPLETE
								USB->HOST.HostPipe[pipe_n].PSTATUSSET.reg = USB_HOST_PSTATUSSET_DTGL;
								pUSB->pUsbPipeTable[pipe_n].HostDescBank[0].PCKSIZE.bit.BYTE_COUNT = 0;
								UHD_Pipe_Send(pipe_n,USB_HOST_PCFG_PTOKEN_IN, 0);
							}
							else
							{
								pPipe->state = CONTROL_WAITING_FOR_DATA_OUT_STAGE_TRASMISSION_COMPLETE ;
								UHD_Pipe_Write(pipe_n, pPipe->bufferSize, pPipe->pBuffer);
								UHD_Pipe_Send(pipe_n, USB_HOST_PCFG_PTOKEN_OUT, 0);	// autozlp must be 0!
							}
							
						}
						pPipe->time = millis16();
					}
					break;	
				case CONTROL_WAITING_FOR_DATA_OUT_STAGE_TRASMISSION_COMPLETE:
					// 
					if (UHD_Pipe_Is_Transfer_Complete(pipe_n, USB_HOST_PCFG_PTOKEN_OUT))
					{
						// now we must get the zero lenght packet.	
						pPipe->state = CONTROL_WAITING_FOR_ZLP_RECEPTION;	// we skip CONTROL_WAITING_FOR_DATA_OUT_STAGE_TRASMISSION_COMPLETE
						USB->HOST.HostPipe[pipe_n].PSTATUSSET.reg = USB_HOST_PSTATUSSET_DTGL;
						pUSB->pUsbPipeTable[pipe_n].HostDescBank[0].PCKSIZE.bit.BYTE_COUNT = 0;
						UHD_Pipe_Send(pipe_n,USB_HOST_PCFG_PTOKEN_IN, 0);						
					}
					break;
				case CONTROL_WAITING_FOR_DATA_IN_STAGE_RECEPITON_COMPLETE:
					if (UHD_Pipe_Is_Transfer_Complete(pipe_n, USB_HOST_PCFG_PTOKEN_IN))
					{	
						USB_TRACE("WFDIS");
						// send a ZLP to confirm the correct reception of the in packets.
						pPipe->bytesReceived = pUSB->pUsbPipeTable[pipe_n].HostDescBank[0].PCKSIZE.bit.BYTE_COUNT;
						USB->HOST.HostPipe[pipe_n].PSTATUSSET.reg = USB_HOST_PSTATUSSET_DTGL;
						UHD_Pipe_Write(pipe_n, 0, pPipe->pSetupPacket);							
						UHD_Pipe_Send(pipe_n, USB_HOST_PCFG_PTOKEN_OUT, 0);						
						pPipe->time = millis16();
						pPipe->state = CONTROL_WAITING_FOR_ZLP_TRANSMISSION_COMPLETE;	
					}
					break;
				case WAITING_FOR_IN_TRANSMISSION_COMPLETE:					
				case CONTROL_WAITING_FOR_ZLP_RECEPTION:
					if (UHD_Pipe_Is_Transfer_Complete(pipe_n, USB_HOST_PCFG_PTOKEN_IN))
					{
						pPipe->bytesReceived = pUSB->pUsbPipeTable[pipe_n].HostDescBank[0].PCKSIZE.bit.BYTE_COUNT;						
						pPipe->state = STATE_IDLE;
						if (pPipe->transferCompletedCallback)
							((void (*)(void*, int))pPipe->transferCompletedCallback)(pPipe->signal, pPipe->bytesReceived);
						USB_TRACE("Pipe %d In transfer complete",pipe_n);
					}
					break;
				case WAITING_FOR_OUT_TRANSMISSION_COMPLETE:	
				case CONTROL_WAITING_FOR_ZLP_TRANSMISSION_COMPLETE:
					if (UHD_Pipe_Is_Transfer_Complete(pipe_n, USB_HOST_PCFG_PTOKEN_OUT))
					{
						pPipe->state = STATE_IDLE;
						if (pPipe->transferCompletedCallback)
							((void (*)(void*, int))pPipe->transferCompletedCallback)(pPipe->signal, pPipe->bytesReceived);
						USB_TRACE("Pipe %d Out transfer complete",pipe_n);
					}
					break;
				case STATE_ERROR:
					USB->HOST.HostPipe[pipe_n].PCFG.bit.PTYPE = USB_HOST_PTYPE_DIS_val;
					if (pPipe->errorCallback)
						((void (*)(void*))pPipe->errorCallback)(pPipe->signal);
					break;
			}
		}

	}
}
#endif  // if !(IS_BOOTLOADER || FORCE_INCLUDE_USB_MODULE)