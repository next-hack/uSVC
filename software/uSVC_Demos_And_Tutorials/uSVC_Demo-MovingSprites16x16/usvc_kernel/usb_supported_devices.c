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
*  usb_supported_devices.c/h: list of devices (for which you should also provide the driver) supported.
*
* Credits: even if this part is strongly modified, this is based on the original
* USB Host library for Arduino, Copyright (C) 2011 Circuits At Home, LTD. All rights reserved,
* which was released under the terms of the GNU General Public License version 2 (GPL2).
*/
#include "usb_supported_devices.h" 
// Put in the array below the installer functions for all the devices you want to support. WARNING! LAST DEVICE MUST BE 0!
const usbDeviceInstaller_t USB_device_Installers[] = {usbHidBootKeyboardInstaller, usbHidGenericGamepadInstaller, 0};