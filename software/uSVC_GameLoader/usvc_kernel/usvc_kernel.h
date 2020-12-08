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
*  usvc_kernel.h: includes the configuration header (which must be on the parent directory) and all the headers of the modules.
*
*/
#ifndef USVC_KERNEL_H_
#define USVC_KERNEL_H_
#include "include/sam.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "vgaConstants.h"
#include "sprites.h"
#include "../usvc_config.h"
#if PRINTF_ENABLE_SUPPORT_FLOAT == 0
	#define PRINTF_DISABLE_SUPPORT_FLOAT
#endif
#if PRINTF_ENABLE_SUPPORT_EXPONENTIAL == 0
	#define PRINTF_DISABLE_SUPPORT_EXPONENTIAL
#endif
#if PRINTF_ENABLE_SUPPORT_LONG_LONG == 0
	#define PRINTF_DISABLE_SUPPORT_LONG_LONG
#endif
#include "usvcUtils.h"
#if USE_STANDARD_PRINTF == 0
	#include "printf.h"		// for optimized printf - avoid dynamic memory allocation
#endif
#include "audioMixer.h"
#include "audioBootloader.h"
#include "audio.h"
#include "system.h"
#include "font8x8.h"
#include "bootFont8x8reduced.h"
#include "usb_host.h"
#include "usb_supported_devices.h"
#include "vga.h"
#include "defaultSounds.h"
#include "pffconf.h"
#include "pff.h"
#include "diskio.h"
#endif /* USVC_KERNEL_H_ */