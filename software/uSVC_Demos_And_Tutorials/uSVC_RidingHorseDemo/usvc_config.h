/*
 *  Riding Horse demo. 
 *  Copyright 2019-2020 Nicola Wrachien (next-hack.com)
 *  A simple demo to show the most exciting features of the TILE_MODE22
 *  1) High res per line x scroll to create water deformation
 *  2) High res row remapping to create water mirror
 *  3) High res Palette change for water color distortion
 *  4) Per line color change to create sky shades
 *  5) Tile prioirity
 *  6) Sprite prioirity
 *
 *  Credits: the graphics of this game (namely the riding horse, the clouds and the tiles) have been taken from opengameart.org, and recoloured to better
 *  fit a 16-color palette.
 *  In particular:
 *  Original assassin character riding a horse: Author Stagnation. Original URL: https://opengameart.org/content/assassin-character-on-horse (License GPL3)
 *  Original classical ruin tiles: Author surt. Original URL: https://opengameart.org/content/classical-ruin-tiles (License: CC0 - public domain)
 *  The graphics is copyright of their respective authors. 
 *
 *  Note: this demo is not optimized for game, just to show some of the most important features all at once. 
 *  Better optimization (especially by using rom vectors) can be achieved fo save tons of RAM!
 *
 *  Warning: the code needs a clean-up, and there might be some unused additional files and functions.
 *
 *  This file is part of next-hack's Riding Horse demo.
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
 */
#ifndef USVC_CONFIG_H_
#define USVC_CONFIG_H_
#include <stdint.h>
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
#define GFX_MODE TILE_MODE2
// number of palettes (only for tile mode 2 or bitmapped mode)
#define MAX_NUMBER_OF_PALETTES 2	// each palette has 256 entries
#if GFX_MODE == TILE_MODE2
	#define VRAMX 44
	#define VRAMY 26	// note: if using meta tiles, vramy and vramx size should be a multiple of 2, and the map size should be larger than VRAMX and VRAMY.
#endif
// audio settings
#define AUDIO_ENABLED 1
#define USE_MIXER 1				// note: it must be 1 if AUDIO_ENABLED is 1... It can be 1 even if AUDIO_ENABLED is 0, but you will have to feed it with data!
#define INCLUDE_DEFAULT_WAVES 1
// optional video mode features
#define ENABLE_TILE_PRIORITY 1
#define ENABLE_SPRITE_PRIORITY 1
#define ENABLE_PER_LINE_COLOR_REMAPPING 1
#define ENABLE_PALETTE_REMAPPING 1
#define USE_ROW_REMAPPING 1
#define USE_HIRES_ROW_REMAPPING 1
#define PER_LINE_X_SCROLL 1
#define PER_TILE_X_SCROLL 0
#define TILES_ALL_OPAQUE 0
// sprites
#define SPRITES_ENABLED 1
#define ENABLE_TRANSPARENT_SPRITES  0		// only for tile mode 1
#define MAX_TEMP_SPRITE_TILES 170
#define MAX_ONSCREEN_SPRITES 10
//
#define ENABLE_HIRES_PER_LINE_COLOR_REMAPPING 1
#define ENABLE_PALETTE_ROW_REMAPPING 1
#ifndef USE_BOOTLOADER
	#define FORCE_INCLUDE_USB_GAMEPAD_MODULE 1 // 0: use bootloader libraries and save space 
	#define FORCE_INCLUDE_USB_MODULE 1			// 0: use bootloader libraries and save space 
	#define FORCE_INCLUDE_USB_KEYBOARD_MODULE 1	// 0: use bootloader libraries and save space 
#endif
// optional fixed section (for tile mode 1 and 2)
#define USE_SECTION NO_FIXED_SECTION
#define SECTION_LIMIT 2
#define USE_SEPARATE_FIXED_SECTION_PALETTE 1
#define FIXED_SECTION_PALETTE_INDEX 0
#define FIXED_SECTION_MAPSIZEX 40
#define FIXED_SECTION_MAPSIZEY 3
#define MAX_FIXED_SECTION_TILES 16
#if USE_SECTION != NO_FIXED_SECTION
extern uint32_t fixedSectionTiles[MAX_FIXED_SECTION_TILES][16] __attribute__ ((aligned (4)));
extern uint8_t fixedSectionMap[FIXED_SECTION_MAPSIZEX * FIXED_SECTION_MAPSIZEY]  __attribute__ ((aligned (4)));
#endif
// Max number of ram tiles
#define MAX_TILES (449 + MAX_TEMP_SPRITE_TILES)  // adjust this according to the number of tiles
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
// #if USE_ROW_REMAPPING
// 	#include "rowRemapTable.h"
// #endif
#ifndef MAX_FIXED_SECTION_TILES
	#define MAX_FIXED_SECTION_TILES 38
#endif

#endif /* USVC_CONFIG_H_ */