/*
 *  RedBalls (not in the UrbanDictionary definintion!): 
 *  A clone of the popular Worms game, runnin on uChip Simple VGA Console
 *  a simple Cortex M0+ console with 32 kB of RAM!
 *
 *  Copyright 2019-2020 Nicola Wrachien (next-hack.com)
 *
 *  This file is part of next-hack's RedBalls.
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
 *
 * usvc_config.h: this file allows you to config the video mode of USVC. 
 */
#ifndef USVC_CONFIG_H_
#define USVC_CONFIG_H_
/*
	FIXED SECTION
	TILEMODE allows for a fixed section (no-scroll) at the top or the bottom of the screen.
	To activate it: 
	define USE_SECTION as TOP_FIXED_SECTION or BOTTOM_FIXED_SECTION
	eg, for a bottom fixed section:
	#define USE_SECTION TOP_FIXED_SECTION
	Also set the section limit, i.e. the row at which the section starts (for bottom) or ends (for top). The number is in terms of tiles and it is not inclusive.
*/
// Graphics mode definition
#define GFX_MODE TILE_MODE_BOOT
// number of palettes (only for tile mode 2 or bitmapped mode)
#define MAX_NUMBER_OF_PALETTES 2	// each palette has 256 entries

// audio settings
#define AUDIO_ENABLED 0
#define USE_BL_AUDIO 1
#define AUDIO_USES_LPF 1
#define AUDIO_ENGINE_ON_VBLANK_INTERRUPT 1 // enables the execution of audio sound engine in a low-priority (high number) interrupt. This allows to have audio during SD accesses
#define USE_MIXER 1
#define USE_SYSTICK_TIMER 1			// USE ONLY IN BOOTLOADER! Systick requires an interrupt for ms counting, while the timer does not.
#define MUSIC_ENGINE NO_MUSIC_ENGINE
#define IS_BOOTLOADER 1
#define INCLUDE_DEFAULT_WAVES 0
// optional video mode features
#define ENABLE_TILE_PRIORITY 0
#define ENABLE_SPRITE_PRIORITY 0
#define ENABLE_PER_LINE_COLOR_REMAPPING 0
#define ENABLE_PALETTE_REMAPPING 0
#define USE_ROW_REMAPPING 0
#define PER_LINE_X_SCROLL 0
#define PER_TILE_X_SCROLL 0
// sprites
#define SPRITES_ENABLED 0
#define ENABLE_TRANSPARENT_SPRITES  0		// only for tile mode 1
#define MAX_TEMP_SPRITE_TILES 146
#define MAX_ONSCREEN_SPRITES 41
//
#define ENABLE_HIRES_PER_LINE_COLOR_REMAPPING 0
#define ENABLE_PALETTE_ROW_REMAPPING 0
// optional fixed section (for tile mode 1 and 2)
#define USE_SECTION NO_FIXED_SECTION
#define SECTION_LIMIT 22
#define USE_SEPARATE_FIXED_SECTION_PALETTE 1
#define FIXED_SECTION_PALETTE_INDEX 0
// Max number of ram tiles
#define MAX_TILES (210)  // adjust this according to the number of tiles
// USB CONFIGURATION: MAX 2 Pipes and 1 device. Modify according to your needs. Be aware that HUB does not support low speed devices!
#define USE_USB_HUB 0
#define USB_NUMDEVICES 1
#define NUMBER_OF_USB_PIPES 3
#define MAX_USB_INTERFACES 4
// PUT HERE THE INCLUDES TO ALL YOUR DATA (for instance audio patches, sounds and sprites)
#if AUDIO_ENABLED == 1
	#include "soundWaveList.h"
	#include "patches.h"
#endif
#if SPRITES_ENABLED == 1
	#include "VGAspriteData.h"
	#if ENABLE_TRANSPARENT_SPRITES == 1
		#include "colorTables.h"
	#endif
#endif
#if ENABLE_PALETTE_REMAPPING
	#include "paletteRemapData.h"
#endif
#if USE_ROW_REMAPPING
	#include "rowRemapTable.h"
#endif
#if USE_SECTION != NO_FIXED_SECTION
#include "fixedSectionTileData.h"
#include "fixedSectionMap.h"
#endif
#ifndef MAX_FIXED_SECTION_TILES
	#define MAX_FIXED_SECTION_TILES 38
#endif

#endif /* USVC_CONFIG_H_ */