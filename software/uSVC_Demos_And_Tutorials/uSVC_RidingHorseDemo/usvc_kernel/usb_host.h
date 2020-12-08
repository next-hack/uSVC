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
#ifndef USB_HOST_H_
#define USB_HOST_H_
#include "usvc_kernel.h" 
#ifndef NUMBER_OF_USB_PIPES
	#define NUMBER_OF_USB_PIPES 8
#endif
#ifndef USB_NUMDEVICES	
	#define USB_NUMDEVICES		4	//number of USB devices
#endif

#ifndef MAX_USB_INTERFACES
	#define MAX_USB_INTERFACES 15
#endif


#define WLANGID_ENGLISH_US 0x0409

/* Misc.USB constants */
#define DEV_DESCR_LEN   18      //device descriptor length
#define CONF_DESCR_LEN  9       //configuration descriptor length
#define INTR_DESCR_LEN  9       //interface descriptor length
#define EP_DESCR_LEN    7       //endpoint descriptor length

/* Standard Device Requests */

#define USB_REQUEST_GET_STATUS                  0       // Standard Device Request - GET STATUS
#define USB_REQUEST_CLEAR_FEATURE               1       // Standard Device Request - CLEAR FEATURE
#define USB_REQUEST_SET_FEATURE                 3       // Standard Device Request - SET FEATURE
#define USB_REQUEST_SET_ADDRESS                 5       // Standard Device Request - SET ADDRESS
#define USB_REQUEST_GET_DESCRIPTOR              6       // Standard Device Request - GET DESCRIPTOR
#define USB_REQUEST_SET_DESCRIPTOR              7       // Standard Device Request - SET DESCRIPTOR
#define USB_REQUEST_GET_CONFIGURATION           8       // Standard Device Request - GET CONFIGURATION
#define USB_REQUEST_SET_CONFIGURATION           9       // Standard Device Request - SET CONFIGURATION
#define USB_REQUEST_GET_INTERFACE               10      // Standard Device Request - GET INTERFACE
#define USB_REQUEST_SET_INTERFACE               11      // Standard Device Request - SET INTERFACE
#define USB_REQUEST_SYNCH_FRAME                 12      // Standard Device Request - SYNCH FRAME

#define USB_FEATURE_ENDPOINT_HALT               0       // CLEAR/SET FEATURE - Endpoint Halt
#define USB_FEATURE_DEVICE_REMOTE_WAKEUP        1       // CLEAR/SET FEATURE - Device remote wake-up
#define USB_FEATURE_TEST_MODE                   2       // CLEAR/SET FEATURE - Test mode

/* Setup Data Constants */

#define USB_SETUP_HOST_TO_DEVICE                0x00    // Device Request bmRequestType transfer direction - host to device transfer
#define USB_SETUP_DEVICE_TO_HOST                0x80    // Device Request bmRequestType transfer direction - device to host transfer
#define USB_SETUP_TYPE_STANDARD                 0x00    // Device Request bmRequestType type - standard
#define USB_SETUP_TYPE_CLASS                    0x20    // Device Request bmRequestType type - class
#define USB_SETUP_TYPE_VENDOR                   0x40    // Device Request bmRequestType type - vendor
#define USB_SETUP_RECIPIENT_DEVICE              0x00    // Device Request bmRequestType recipient - device
#define USB_SETUP_RECIPIENT_INTERFACE           0x01    // Device Request bmRequestType recipient - interface
#define USB_SETUP_RECIPIENT_ENDPOINT            0x02    // Device Request bmRequestType recipient - endpoint
#define USB_SETUP_RECIPIENT_OTHER               0x03    // Device Request bmRequestType recipient - other

/* USB descriptors  */

#define USB_DESCRIPTOR_DEVICE           0x01    // bDescriptorType for a Device Descriptor.
#define USB_DESCRIPTOR_CONFIGURATION    0x02    // bDescriptorType for a Configuration Descriptor.
#define USB_DESCRIPTOR_STRING           0x03    // bDescriptorType for a String Descriptor.
#define USB_DESCRIPTOR_INTERFACE        0x04    // bDescriptorType for an Interface Descriptor.
#define USB_DESCRIPTOR_ENDPOINT         0x05    // bDescriptorType for an Endpoint Descriptor.
#define USB_DESCRIPTOR_DEVICE_QUALIFIER 0x06    // bDescriptorType for a Device Qualifier.
#define USB_DESCRIPTOR_OTHER_SPEED      0x07    // bDescriptorType for a Other Speed Configuration.
#define USB_DESCRIPTOR_INTERFACE_POWER  0x08    // bDescriptorType for Interface Power.
#define USB_DESCRIPTOR_OTG              0x09    // bDescriptorType for an OTG Descriptor.

#define USB_DESCRIPTOR_HID			0x21

/* HID requests */
#define bmREQ_HIDOUT        (USB_SETUP_HOST_TO_DEVICE|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE)
#define bmREQ_HIDIN        (USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_CLASS|USB_SETUP_RECIPIENT_INTERFACE)
#define bmREQ_HIDREPORT     (USB_SETUP_DEVICE_TO_HOST|USB_SETUP_TYPE_STANDARD|USB_SETUP_RECIPIENT_INTERFACE)

/* HID constants. Not part of chapter 9 */
/* Class-Specific Requests */
#define HID_REQUEST_GET_REPORT      0x01
#define HID_REQUEST_GET_IDLE        0x02
#define HID_REQUEST_GET_PROTOCOL    0x03
#define HID_REQUEST_SET_REPORT      0x09
#define HID_REQUEST_SET_IDLE        0x0A
#define HID_REQUEST_SET_PROTOCOL    0x0B

/* Class Descriptor Types */
#define HID_DESCRIPTOR_HID			0x21
#define HID_DESCRIPTOR_REPORT		0x22
#define HID_DESRIPTOR_PHY			0x23

/* Protocol Selection */
#define HID_BOOT_PROTOCOL			0x00
#define HID_RPT_PROTOCOL			0x01

/* HID Interface Class Code */
#define HID_INTF                    0x03

/* HID Interface Class SubClass Codes */
#define HID_BOOT_INTF_SUBCLASS      0x01
#define HID_NO_SUBCLASS				0x00


/* HID Interface Class Protocol Codes */
#define HID_PROTOCOL_NONE           0x00
#define HID_PROTOCOL_KEYBOARD       0x01
#define HID_PROTOCOL_MOUSE          0x02
/* HID REPORT*/
#define HID_REPORT_BOOT 0
#define HID_REPORT_OUTPUT 2
/* OTG SET FEATURE Constants    */
#define OTG_FEATURE_B_HNP_ENABLE                3       // SET FEATURE OTG - Enable B device to perform HNP
#define OTG_FEATURE_A_HNP_SUPPORT               4       // SET FEATURE OTG - A device supports HNP
#define OTG_FEATURE_A_ALT_HNP_SUPPORT           5       // SET FEATURE OTG - Another port on the A device supports HNP

/* USB Endpoint Transfer Types  */
#define USB_TRANSFER_TYPE_CONTROL               0x00    // Endpoint is a control endpoint.
#define USB_TRANSFER_TYPE_ISOCHRONOUS           0x01    // Endpoint is an isochronous endpoint.
#define USB_TRANSFER_TYPE_BULK                  0x02    // Endpoint is a bulk endpoint.
#define USB_TRANSFER_TYPE_INTERRUPT             0x03    // Endpoint is an interrupt endpoint.


/* Standard Feature Selectors for CLEAR_FEATURE Requests    */
#define USB_FEATURE_ENDPOINT_STALL              0       // Endpoint recipient
#define USB_FEATURE_DEVICE_REMOTE_WAKEUP        1       // Device recipient
#define USB_FEATURE_TEST_MODE                   2       // Device recipient

// USB Device Classes
#define USB_CLASS_USE_CLASS_INFO	0x00	// Use Class Info in the Interface Descriptors
#define USB_CLASS_AUDIO			0x01	// Audio
#define USB_CLASS_COM_AND_CDC_CTRL	0x02	// Communications and CDC Control
#define USB_CLASS_HID			0x03	// HID
#define USB_CLASS_PHYSICAL		0x05	// Physical
#define USB_CLASS_IMAGE			0x06	// Image
#define USB_CLASS_PRINTER		0x07	// Printer
#define USB_CLASS_MASS_STORAGE		0x08	// Mass Storage
#define USB_CLASS_HUB			0x09	// Hub
#define USB_CLASS_CDC_DATA		0x0a	// CDC-Data
#define USB_CLASS_SMART_CARD		0x0b	// Smart-Card
#define USB_CLASS_CONTENT_SECURITY	0x0d	// Content Security
#define USB_CLASS_VIDEO			0x0e	// Video
#define USB_CLASS_PERSONAL_HEALTH	0x0f	// Personal Healthcare
#define USB_CLASS_DIAGNOSTIC_DEVICE	0xdc	// Diagnostic Device
#define USB_CLASS_WIRELESS_CTRL		0xe0	// Wireless Controller
#define USB_CLASS_MISC			0xef	// Miscellaneous
#define USB_CLASS_APP_SPECIFIC		0xfe	// Application Specific
#define USB_CLASS_VENDOR_SPECIFIC	0xff	// Vendor Specific

#define USB_TIMEOUT        5000UL    // (5000) USB transfer timeout in milliseconds, per section 9.2.6.1 of USB 2.0 spec
#define USB_RETRY_LIMIT		3       // 3 retry limit for a transfer
#define USB_SETTLE_DELAY	20UL     //settle delay in milliseconds

#define HUB_PORT_RESET_DELAY	20	// hub port reset delay 10 ms recomended, can be up to 20 ms

/* USB state machine states */
#define USB_STATE_MASK                                      0xf0
#define USB_STATE_DETACHED                                  0x10
#define USB_DETACHED_SUBSTATE_INITIALIZE                    0x11
#define USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE               0x12
#define USB_DETACHED_SUBSTATE_ILLEGAL                       0x13
#define USB_ATTACHED_SUBSTATE_SETTLE_GET_TIME				0x20
#define USB_ATTACHED_SUBSTATE_SETTLE                        0x30
#define USB_ATTACHED_SUBSTATE_RESET_DEVICE                  0x40
#define USB_ATTACHED_SUBSTATE_WAIT_RESET_COMPLETE           0x50
#define USB_ATTACHED_SUBSTATE_WAIT_SOF                      0x60
#define USB_ATTACHED_SUBSTATE_WAIT_RESET                    0x61
#define USB_ATTACHED_SUBSTATE_SOF_MISSING					0x62
#define USB_ATTACHED_SUBSTATE_GET_DEVICE_DESCRIPTOR_SIZE    0x70
#define USB_STATE_ADDRESSING                                0x80
#define USB_STATE_CONFIGURING                               0x90
#define USB_STATE_RUNNING                                   0xa0
#define USB_STATE_ERROR                                     0xb0
// USB HOST
#define  USB_EP_DIR_IN        0x80  // USB_SETUP_DEVICE_TO_HOST
#define  USB_EP_DIR_OUT       0x00  // USB_SETUP_HOST_TO_DEVICE

#define USB_HOST_PTYPE_DIS_val     (0x0) // Pipe is disabled
#define USB_HOST_PTYPE_CTRL_val    (0x1) // Pipe is enabled and configured as CONTROL
#define USB_HOST_PTYPE_ISO_val     (0x2) // Pipe is enabled and configured as ISO
#define USB_HOST_PTYPE_BULK_val    (0x3) // Pipe is enabled and configured as BULK
#define USB_HOST_PTYPE_INT_val     (0x4) // Pipe is enabled and configured as INTERRUPT
#define USB_HOST_PTYPE_EXT_val     (0x5) // Pipe is enabled and configured as EXTENDED

#define USB_HOST_NB_BK_1		1

#define USB_HOST_PCFG_PTOKEN_SETUP  USB_HOST_PCFG_PTOKEN(0x0)
#define USB_HOST_PCFG_PTOKEN_IN     USB_HOST_PCFG_PTOKEN(0x1)
#define USB_HOST_PCFG_PTOKEN_OUT    USB_HOST_PCFG_PTOKEN(0x2)


#define USB_PCKSIZE_SIZE_8_BYTES        0
#define USB_PCKSIZE_SIZE_16_BYTES       1
#define USB_PCKSIZE_SIZE_32_BYTES       2
#define USB_PCKSIZE_SIZE_64_BYTES       3
#define USB_PCKSIZE_SIZE_128_BYTES      4
#define USB_PCKSIZE_SIZE_256_BYTES      5
#define USB_PCKSIZE_SIZE_512_BYTES      6
#define USB_PCKSIZE_SIZE_1023_BYTES_FS  7
#define USB_PCKSIZE_SIZE_1024_BYTES_HS  7

#define usbHostDtgl(p)               (USB->HOST.HostPipe[p].PSTATUS.reg & USB_HOST_PSTATUS_DTGL)>>USB_HOST_PSTATUS_DTGL_Pos

// USB host connection/disconnection monitoring
#define uhdEnableConnectionInt()           USB->HOST.INTENSET.reg = USB_HOST_INTENSET_DCONN
#define uhdDisableConnectionInt()          USB->HOST.INTENCLR.reg = USB_HOST_INTENCLR_DCONN
#define uhdAckConnection()                  USB->HOST.INTFLAG.reg = USB_HOST_INTFLAG_DCONN

#define uhdEnableDisconnectionInt()        USB->HOST.INTENSET.reg = USB_HOST_INTENSET_DDISC
#define uhdDisableDisconnectionInt()       USB->HOST.INTENCLR.reg = USB_HOST_INTENCLR_DDISC
#define uhdAckDisconnection()               USB->HOST.INTFLAG.reg = USB_HOST_INTFLAG_DDISC

// Initiates a USB register reset
#define uhdStartUsbRegReset()             USB->HOST.CTRLA.bit.SWRST = 1;

// Bus Reset
#define isUhdStartingReset()             (USB->HOST.CTRLB.bit.BUSRESET == 1)
#define uhdBusReset()                      USB->HOST.CTRLB.bit.BUSRESET = 1
#define uhdStopReset()                    // nothing to do

#define uhdAckResetSent()                         USB->HOST.INTFLAG.reg = USB_HOST_INTFLAG_RST
#define isUhdResetSent()                          (USB->HOST.INTFLAG.reg & USB_HOST_INTFLAG_RST)

// Initiates a SOF events
#define uhdEnableSof()                             USB->HOST.CTRLB.bit.SOFE = 1
#define uhdDisableSof()                            USB->HOST.CTRLB.bit.SOFE = 0
#define isUhdSofEnabled()                         (USB->HOST.CTRLB & USB_HOST_CTRLB_SOFE)
#define isUhdSof()                                 (USB->HOST.INTFLAG.reg & USB_HOST_INTFLAG_HSOF)

// USB address of pipes
#define uhdConfigureAddress(pipe_num, addr) usb_pipe_table[pipe_num].HostDescBank[0].CTRL_PIPE.bit.PDADDR = addr
#define uhdGetConfiguredAddress(pipe_num)  usb_pipe_table[pipe_num].HostDescBank[0].CTRL_PIPE.bit.PDADDR

// Pipes
#define uhdFreezePipe(p)                       USB->HOST.HostPipe[p].PSTATUSSET.reg = USB_HOST_PSTATUSSET_PFREEZE
#define uhdUnfreezePipe(p)                     USB->HOST.HostPipe[p].PSTATUSCLR.reg = USB_HOST_PSTATUSCLR_PFREEZE
#define isUhdPipeFrozen(p)                    ((USB->HOST.HostPipe[p].PSTATUS.reg&USB_HOST_PSTATUS_PFREEZE)==USB_HOST_PSTATUS_PFREEZE)

// Pipe configuration
#define uhdConfigurePipeToken(p, token)       USB->HOST.HostPipe[p].PCFG.bit.PTOKEN = token

// Pipe data management
#define uhdByteCount(p)                        usb_pipe_table[p].HostDescBank[0].PCKSIZE.bit.BYTE_COUNT
#define uhdAckSetupReady(p)                   USB->HOST.HostPipe[p].PINTFLAG.reg = USB_HOST_PINTFLAG_TXSTP
#define isUhdSetupReady(p)                    ((USB->HOST.HostPipe[p].PINTFLAG.reg&USB_HOST_PINTFLAG_TXSTP) == USB_HOST_PINTFLAG_TXSTP)
#define uhdAckInReceived(p)                   USB->HOST.HostPipe[p].PINTFLAG.reg = USB_HOST_PINTFLAG_TRCPT(1)
#define isUhdInReceived(p)                    ((USB->HOST.HostPipe[p].PINTFLAG.reg&USB_HOST_PINTFLAG_TRCPT(1)) == USB_HOST_PINTFLAG_TRCPT(1))
#define uhdAckOutReady(p)                     USB->HOST.HostPipe[p].PINTFLAG.reg = USB_HOST_PINTFLAG_TRCPT(1)
#define isUhdOutReady(p)                      ((USB->HOST.HostPipe[p].PINTFLAG.reg&USB_HOST_PINTFLAG_TRCPT(1)) == USB_HOST_PINTFLAG_TRCPT(1))
#define uhdAckNakReceived(p)                  usb_pipe_table[p].HostDescBank[1].STATUS_BK.reg &= ~USB_HOST_STATUS_BK_ERRORFLOW
#define isUhdNakReceived(p)                   (usb_pipe_table[p].HostDescBank[1].STATUS_BK.reg & USB_HOST_STATUS_BK_ERRORFLOW)

// Endpoint Interrupt Summary
#define uhdEndpointInterrupt()            USB->HOST.PINTSMRY.reg

// Run in Standby
#define uhdRunInStandby()                USB->HOST.CTRLA.reg |= USB_CTRLA_RUNSTDBY
// Force host mode
#define uhdForceHostMode()               USB->HOST.CTRLA.reg |= USB_CTRLA_MODE

// Enable USB macro
#define uhdEnable()                        USB->HOST.CTRLA.reg |= USB_CTRLA_ENABLE
// Disable USB macro
#define uhdDisable()                       USB->HOST.CTRLA.reg &= ~USB_CTRLA_ENABLE

// Force full speed mode
#define uhdForceFullSpeed()              USB->HOST.CTRLB.reg &= ~USB_HOST_CTRLB_SPDCONF_Msk

//

/* descriptor data structures */

/* Device descriptor structure */
typedef struct 
{
	uint8_t		bLength;               // Length of this descriptor.
	uint8_t		bDescriptorType;       // DEVICE descriptor type (USB_DESCRIPTOR_DEVICE).
	uint16_t	bcdUSB;				   // USB Spec Release Number (BCD).
	uint8_t		bDeviceClass;          // Class code (assigned by the USB-IF). 0xFF-Vendor specific.
	uint8_t		bDeviceSubClass;       // Subclass code (assigned by the USB-IF).
	uint8_t		bDeviceProtocol;       // Protocol code (assigned by the USB-IF). 0xFF-Vendor specific.
	uint8_t		bMaxPacketSize0;       // Maximum packet size for endpoint 0.
	uint16_t	idVendor;			   // Vendor ID (assigned by the USB-IF).
	uint16_t	idProduct;			   // Product ID (assigned by the manufacturer).
	uint16_t	bcdDevice;			   // Device release number (BCD).
	uint8_t		iManufacturer;         // Index of String Descriptor describing the manufacturer.
	uint8_t		iProduct;              // Index of String Descriptor describing the product.
	uint8_t		iSerialNumber;         // Index of String Descriptor with the device's serial number.
	uint8_t		bNumConfigurations;    // Number of possible configurations.
} __attribute__ ((packed)) usbDeviceDescriptor_t;
typedef struct 
{
	uint8_t bLength;						//
	uint8_t bDescriptorType;	
}	usbDescriptor_t;
/* Configuration descriptor structure */
typedef struct {
	uint8_t bLength;               // Length of this descriptor.
	uint8_t bDescriptorType;       // CONFIGURATION descriptor type (USB_DESCRIPTOR_CONFIGURATION).
	uint16_t wTotalLength;          // Total length of all descriptors for this configuration.
	uint8_t bNumInterfaces;        // Number of interfaces in this configuration.
	uint8_t bConfigurationValue;   // Value of this configuration (1 based).
	uint8_t iConfiguration;        // Index of String Descriptor describing the configuration.
	uint8_t bmAttributes;          // Configuration characteristics.
	uint8_t bMaxPower;             // Maximum power consumed by this configuration.
} __attribute__ ((packed)) usbConfigurationDescriptor_t;

/* Interface descriptor structure */
typedef struct {
	uint8_t bLength;               // Length of this descriptor.
	uint8_t bDescriptorType;       // INTERFACE descriptor type (USB_DESCRIPTOR_INTERFACE).
	uint8_t bInterfaceNumber;      // Number of this interface (0 based).
	uint8_t bAlternateSetting;     // Value of this alternate interface setting.
	uint8_t bNumEndpoints;         // Number of endpoints in this interface.
	uint8_t bInterfaceClass;       // Class code (assigned by the USB-IF).  0xFF-Vendor specific.
	uint8_t bInterfaceSubClass;    // Subclass code (assigned by the USB-IF).
	uint8_t bInterfaceProtocol;    // Protocol code (assigned by the USB-IF).  0xFF-Vendor specific.
	uint8_t iInterface;            // Index of String Descriptor describing the interface.
} __attribute__ ((packed)) usbInterfaceDescriptor_t;

/* Endpoint descriptor structure */
typedef struct {
	uint8_t bLength;               // Length of this descriptor.
	uint8_t bDescriptorType;       // ENDPOINT descriptor type (USB_DESCRIPTOR_ENDPOINT).
	uint8_t bEndpointAddress;      // Endpoint address. Bit 7 indicates direction (0=OUT, 1=IN).
	uint8_t bmAttributes;          // Endpoint transfer type.
	uint16_t wMaxPacketSize;       // Maximum packet size.
	uint8_t bInterval;             // Polling interval in frames.
} __attribute__ ((packed)) usbEndpointDescriptor_t;


/* HID descriptor */
typedef struct {
	uint8_t		bLength;
	uint8_t		bDescriptorType;
	uint16_t	bcdHID;					// HID class specification release
	uint8_t		bCountryCode;
	uint8_t		bNumDescriptors;		// Number of additional class specific descriptors
	uint8_t		bDescrType;				// Type of class descriptor
	uint16_t	wDescriptorLength;		// Total size of the Report descriptor
} __attribute__ ((packed)) usbHidDescriptor_t ;

typedef struct {
	uint8_t		bDescrType;				// Type of class descriptor
	uint16_t	wDescriptorLength;		// Total size of the Report descriptor
} __attribute__ ((packed)) usbHidClassDescriptorLenAndType_t ;
typedef struct
{
	uint8_t bLength;						//
	uint8_t bDescriptorType;		// 3 for string descriptor
	uint16_t *wLANGID;					//
	
} __attribute__ ((packed)) usbStringDescriptorZero_t;
typedef struct
{
	uint8_t bLength;						//
	uint8_t bDescriptorType;		// 3 for string descriptor
	char String[];					//
	
} __attribute__ ((packed)) usbStringDescriptor_t;
typedef union
{
	struct // when we have no hid descriptors
	{
		usbEndpointDescriptor_t endpoint[15];
		usbHidDescriptor_t dummy;	// dummy to get the same size
	} endpoints;
	struct // when we have an interface with hid descriptor
	{
		usbHidDescriptor_t hid_descriptor;	
		usbEndpointDescriptor_t endpoint[15];			
	} hid_endpoints;
} __attribute__ ((packed))  usbMixedHidEndpointDescriptors_t;
typedef struct
{
	usbInterfaceDescriptor_t interfaceDescriptor;
	usbMixedHidEndpointDescriptors_t data;
} __attribute__ ((packed)) usbInterfaceEndpointDescriptor_t;
//! States of USBB interface
typedef enum {
	UHD_STATE_NO_VBUS = 0,
	UHD_STATE_DISCONNECTED = 1,
	UHD_STATE_CONNECTED = 2,
	UHD_STATE_ERROR = 3,
} uhdVbusState_t;
typedef enum
{
	FIRST_GET_DD = 0,
	WAIT_FOR_FIRST_GET_DD,
	FIRST_GET_DD_RECEIVED,
	WAIT_20_MS,
	WAIT_FOR_RESET_SENT,
	WAIT_100MS,
	WAIT_FOR_SET_ADDRESS_SENT,
	SET_ADDRESS_SENT,
	WAIT_20_MS_2,
	WAIT_FOR_SECOND_GET_DD,
	PARSE_DEVICE_DESCRIPTOR,
	REQUEST_PRODUCT_NAME,
	WAIT_FOR_PRODUCT_NAME,
	PRINT_PRODUCT_NAME,
	REQUEST_CONFIGURATION_DESCRIPTOR,
	WAIT_FOR_REQUEST_CONFIGURATION_DESCRIPTOR,
	PARSE_CONFIGURATION_DESCRIPTOR,
	CONFIGURE_DEVICE,
	ENUMERATION_COMPLETE,
//	ENUMERATION_RESET_REQUIRED = -3,
	DEVICE_NOT_SUPPORTED = -2,
	ENUMERATION_ERROR = -1
	
} enumerationState_t;
typedef struct
{
	uint32_t enumerationState;
	uint32_t installationStep;
	uint16_t time;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t iProduct;
	void * userDataPtr;
	void ** modulePointer;	
	uint8_t address;
	uint8_t ep0Size;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t interfacesFound;
	uint8_t hubPort;		// hub port the device is connected to.
} usbDevice_t;
typedef enum
{
	STATE_IDLE = 0,
	CONTROL_REQUEST_SETUP_STAGE_TRANSMISSION,
	CONTROL_WAITING_FOR_SETUP_STAGE_COMPLETE,
	CONTROL_WAITING_FOR_DATA_IN_STAGE_RECEPITON_COMPLETE,
	CONTROL_WAITING_FOR_DATA_OUT_STAGE_TRASMISSION_COMPLETE,
	CONTROL_WAITING_FOR_ZLP_RECEPTION,
	CONTROL_WAITING_FOR_ZLP_TRANSMISSION_COMPLETE,
	REQUEST_FOR_IN_TRANSMISSION,
	REQUEST_FOR_OUT_TRANSMISSION,
	WAITING_FOR_IN_TRANSMISSION_COMPLETE,
	WAITING_FOR_OUT_TRANSMISSION_COMPLETE,
	STATE_ERROR = 0xFFFFFFFF
	
} pipeState_t;
typedef struct
{
	uint16_t time;
	uint16_t timeout;
	void *errorCallback;
	void (*transferCompletedCallback)(void*, int);
	void *signal;
	uint8_t *pBuffer;	// pointer to what we need to send.
	uint8_t *pSetupPacket;
	uint16_t bufferSize;	// size of the buffer pointed by pBuffer.
	uint16_t bytesReceived;
	pipeState_t state;
	uint8_t direction;	// if the data stage must be IN or OUT
	uint8_t isAssociated;
} usbPipe_t;
typedef uint32_t (*usbDeviceInstaller_t)(uint32_t action, usbDevice_t* pdev, void *paramPtr);
typedef struct
{
	usbPipe_t *pUsbPipes;				// pointer to the pipe data
	volatile UsbHostDescriptor *pUsbPipeTable;		// pointer to the pipe table (accessed by the USB_HOST)
	uint8_t *pUsbEndpoint0Buffer;		// pointer to the endpoint 0 buffer
	uint8_t *pUsbEndpoint0ControlBuffer; // pointer to the control buffer of the endpoint 0. Fixed 8-byte size.
	usbDeviceInstaller_t *pUsbDeviceInstallers;	// pointer to a null terminated array of possible device drivers.
	usbDevice_t *pUsbDevices;			// pointer to USB_devices array
	usbInterfaceEndpointDescriptor_t **pTmpUsbInterfaces;		// pointe to the array of interface pointers
	uint8_t (*pResetHubPort)(uint8_t port);
	uint8_t (*pGetResetHubPortStatus)();
	void (*pProductNameFoundCallBack)(uint8_t *);
	void (*pUnsupportedDeviceCallback)(usbDevice_t *pdev);
	void* (* pAllocateDriverMemory)(uint16_t, uint8_t);
	uint32_t usbTaskState;
	uint16_t usbEndpoint0BufferSize;	// size of endpoint 0 buffer
	uint16_t usbHostTaskLastTime;
	uint16_t usbDebounceTime;
	uint8_t maxNumberOfPipes;		//
	uint8_t currentUSBdeviceNumber;
	uint8_t maxNumberOfUSBdevices;
	uint8_t enumeratingDeviceType;
} usbModuleData_t;
//
#define ERROR_PIPE_BUSY 1
#define ERROR_INVALID_PIPE 2
#define ERROR_PIPE_NOT_ASSOCIATED 3
#define ERROR_DEVICE_NOT_INSTALLED 4
#define ERROR_TOO_FREQUENT_REQUESTS 5
#define ERROR_BUFFER_TOO_LARGE 6
#define ERROR_READ_BUFFER_NOT_EMPTY 7
#define ERROR_TOO_MANY_DEVICES 8
#define ERROR_OTHER_DEVICE_CONFIGURING 9			// for HUB support
//
#define MAX_USB_ERRORS 0x0F
#define DIRECTION_IN 0x80
#define DIRECTION_OUT 0
#define usbCreateGetDeviceDescriptorRequest8(buffer) usbCreateStandardRequest(buffer, USB_SETUP_DEVICE_TO_HOST, USB_REQUEST_GET_DESCRIPTOR, (USB_DESCRIPTOR_DEVICE << 8), 0,8)
#define usbCreateGetDeviceDescriptorRequest(buffer) usbCreateStandardRequest(buffer, USB_SETUP_DEVICE_TO_HOST, USB_REQUEST_GET_DESCRIPTOR, (USB_DESCRIPTOR_DEVICE << 8), 0,DEV_DESCR_LEN)
#define usbCreateSetAddress(buffer,address) usbCreateStandardRequest(buffer, USB_SETUP_HOST_TO_DEVICE,USB_REQUEST_SET_ADDRESS, address, 0,0)
#define usbCreateGetConfigurationDescriptorRequest(buffer,number,length) usbCreateStandardRequest(buffer, USB_SETUP_DEVICE_TO_HOST,USB_REQUEST_GET_DESCRIPTOR, (USB_DESCRIPTOR_CONFIGURATION << 8) | number, 0,length)
#define usbCreateSetConfiguration(buffer,number) usbCreateStandardRequest(buffer, USB_SETUP_HOST_TO_DEVICE, USB_REQUEST_SET_CONFIGURATION, number, 0,0)
#define usbCreateGetStringDescriptorRequest(buffer, idString, langID, length) usbCreateStandardRequest(buffer, USB_SETUP_DEVICE_TO_HOST,USB_REQUEST_GET_DESCRIPTOR, (USB_DESCRIPTOR_STRING << 8) | idString, langID,length)
#define usbCreateSetInterfaceProtocol(buffer, intfc, prot) usbCreateStandardRequest(buffer, bmREQ_HIDOUT, HID_REQUEST_SET_PROTOCOL, prot, intfc, 0)
#define usbCreateSetReport(buffer, iface, report_type, report_id, nbytes) usbCreateStandardRequest(buffer, bmREQ_HIDOUT, HID_REQUEST_SET_REPORT, report_id | (report_type << 8), iface,  nbytes)
#define usbCreateGetReportDescriptor(buffer, interface, length) usbCreateStandardRequest(buffer, bmREQ_HIDREPORT, USB_REQUEST_GET_DESCRIPTOR, ((HID_DESCRIPTOR_REPORT << 8) | interface), interface, length)
#define usbInterfaceIsHID(intrfc) (intrfc.bInterfaceClass == USB_CLASS_HID)
void usbHostInit();
void usbHostTask();
uint32_t uhdPipe0Alloc(uint8_t address, uint8_t ep_size);
uint32_t uhdPipeAlloc(uint32_t ul_dev_addr, uint32_t ul_dev_ep, uint32_t ul_type, uint32_t ul_dir, uint32_t ul_epsize, uint32_t ul_interval, uint32_t ul_nb_bank);
void uhdPipeFree(uint32_t ul_pipe);
void usbCreateStandardRequest ( volatile uint8_t *buffer, uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength);
uint32_t usbAddTransaction(uint32_t pipe_n, uint32_t bufferSize, uint8_t *setupPacket, uint8_t* buffer, uint8_t direction, uint32_t type, uint32_t timeout, void *errorCallback,  void *transferCompletedCallback, void *signal);
void usbReleaseDevice(uint8_t address);
uint32_t usbGetTaskState ();
int usbStringDescriptor2Char (uint8_t *destBuffer, usbStringDescriptor_t *sd, int bufferSize);
uint8_t usbFindHidInterfaceAndEndpoint(uint8_t interfacesFound, usbInterfaceEndpointDescriptor_t** interfaceEndpointList, uint8_t bInterfaceSubClass, uint8_t bInterfaceProtocol, uint8_t *foundInterfaceNumber, uint8_t *foundEndpointNumber);

// These functions need to be present in the application too. start_hub_enumeration implementation is required only when actually dealing with
// hubs. In the bootloader, the linker will strip away this function, because it is not used. (the same applies for the other hub-related functions).
uint32_t usbStartHubEnumeration (uint8_t *pNewAddress, uint8_t port);
void usbSetHubSupport(void *resetHubPort, void *getResetHubPortStatus);
void initUSBModule(const usbDeviceInstaller_t *pUsbInstallerList, void* (* memoryAllocationFunction)(uint16_t, uint8_t));
usbModuleData_t * getUSBData();
void usbTraceBytes(uint8_t *buffer);
void usbSetUnsupportedDeviceCallBack(void *callback);
void usbSetProductNameFoundCallBack(void *callback);
void *usbStandardSingleDeviceAllocateDriverMemory(uint16_t memory, uint8_t deviceType);
//

#endif /* USB_HOST_H_ */