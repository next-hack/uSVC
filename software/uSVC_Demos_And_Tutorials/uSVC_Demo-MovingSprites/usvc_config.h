/*
 *  USVC Template Project
 *
 *  Change/remove this copyright part when you create your own program!
 * 
 *  Copyright 2019-2020 Nicola Wrachien (next-hack.com)
 *
 *  This file is part of next-hack's USCV Template project.
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
// Graphics mode definition. TILE_MODE1 = 8bpp (256 colors). TILE_MODE2 = 4bpp (16 colors). BITMAPPED_MODE: 2 bit per pixels
#define GFX_MODE TILE_MODE1
// For TILE_MODE2 and TILE_MODE1 you can redefine VRAMX and VRAMY. Minimum values are VRAM = 40 and VRAMY = 25
#if GFX_MODE == TILE_MODE2 || GFX_MODE == TILE_MODE1
// note: if using meta tiles, vramy and vramx size should be a multiple of 2.
	#define VRAMX 42
	#define VRAMY 26	
	// Max number of tiles. Note that this number must include both the number of Ram Tile, and the number of temporary sprite tiles. 
	#define MAX_TILES (304 + 40)  // adjust this according to the number of tiles
#endif
// sprites (only for TILE_MODE1 and TILE_MODE2). Adjust everything according to your requirements
#define SPRITES_ENABLED 1
#define ENABLE_TRANSPARENT_SPRITES  0	// only for TILE_MODE1. Require to include a file, which defines a variable transparentSpriteColorTable.
#define MAX_TEMP_SPRITE_TILES 304		// Be sure to set also MAX_TILES large enough!
#define MAX_ONSCREEN_SPRITES 76			// Number of actual sprites on screen
#define ENABLE_SPRITE_PRIORITY 0        // if 1, it is possible to put the sprite always on top of a tile (regardless its priority) or always under the tiles.
// Tile priority
#define ENABLE_TILE_PRIORITY 0			// if 1, it is possible to define the priority of a tile, so that it will appear on top of those sprites without top or bottom priority
#define TILES_ALL_OPAQUE 1				// (used only if ENABLE_TILE_PRIORITY is 1) if 1, it tells the engine that all the tiles are either completely transparent or completely opaque, so that, if they have priority, the sprite won't appear under them.
// Advanced video features:
// number of palettes (only for tile mode 2 or bitmapped mode)
#define MAX_NUMBER_OF_PALETTES 2	// each palette has 256 entries. Each entry is 4 bytes, so use it carefully!
// Color Change Engine (only for TILE_MODE2)
#define ENABLE_PALETTE_REMAPPING 0					// if 1, enables the color change engine, allowing to change palette on each row.
#define ENABLE_PER_LINE_COLOR_REMAPPING 0			// if 1 it also allows to change an arbitrary color index for one particular palette, on each row.
#define ENABLE_HIRES_PER_LINE_COLOR_REMAPPING 0		// if 1, each table entry is interpreted as a single hires row (400 rows). If 0, each entry is for 2 hires row.
#define ENABLE_PALETTE_ROW_REMAPPING 0				// Advanced: if 1, allows to add for each line, an offset, so that the actual table row accessed will be offset[Row] + row;
// ROW Remapping: allows to arbitrarily replace the vertical line position.
#define USE_ROW_REMAPPING 0							// if 1 enables vertical remapping.	
#define USE_HIRES_ROW_REMAPPING 1					// (only for TILE_MODE2) if 1 remapping occurs on hi-res basis. Note that in this case, the entry only point to low-res lines. 
// Horizontal scroll.
#define PER_LINE_X_SCROLL 0							// if 1, scroll can be specified on per line basis (screen relative! - before any rowRemapping)
#define PER_TILE_X_SCROLL 0							// if 1, scroll can be specified on per tile basis (vram relative! - after any rowRemapping!)
/*
	FIXED SECTION
	TILE_MODE1 and TILE_MODE2 allow for a fixed section (no-scroll) at the top or bottom of the screen.
	To activate it: 
	define USE_SECTION as TOP_FIXED_SECTION or BOTTOM_FIXED_SECTION
	eg, for a bottom fixed section:
	#define USE_SECTION TOP_FIXED_SECTION
	Also set the section limit, i.e. the row at which the section starts (for bottom) or ends (for top). The number is in terms of tiles and it is not inclusive.
*/
#define USE_SECTION NO_FIXED_SECTION
#define SECTION_LIMIT 2
#define USE_SEPARATE_FIXED_SECTION_PALETTE 0		// Only for TILE_MODE2: the fixed section has its own palette 
#define FIXED_SECTION_PALETTE_INDEX 0				// Only for TILE_MODE2 and if USE_SEPARATE_FIXED_SECTION_PALETTE is 1: palette of the fixed section
#define FIXED_SECTION_MAPSIZEX 40					// width of fixed section map. It must be 40!
#define FIXED_SECTION_MAPSIZEY 3					// height of fixed section map.
#define MAX_FIXED_SECTION_TILES 16					// number of separated tiles for the fixed section.
#if USE_SECTION != NO_FIXED_SECTION
	extern uint32_t fixedSectionTiles[MAX_FIXED_SECTION_TILES][GFX_MODE == TILE_MODE2 ? 8 : 16] __attribute__ ((aligned (4)));
	extern uint8_t fixedSectionMap[FIXED_SECTION_MAPSIZEX * FIXED_SECTION_MAPSIZEY]  __attribute__ ((aligned (4)));
#endif
/* AUDIO.
	Audio is made of two modules. The sound Engine, and the Audio Mixer.
	The sound Engine is a port of Alec Borque's UZEBox sound engine, with minor changes, and it provides support for samples and songs.
	The audio mixer actually mixes the four channels producing one single mono 10 bit signal.
	To actually produce any audio output you must:
	1) define AUDIO_ENABLED as 1
	2) define USE_MIXER as 1
	3) Create one file, which defines an array of soundWave_t, containing the parmeters of all the sound waves you are using (including the default waves, if enabled!).
	   In other words: sondWave_t soundWaves[];
	   That file must also define the function "int getNumberOfSoundWaves()", which returns how many sound waves you have defined (i.e. it must return
	   sizeof (soundWaves) / sizeof(soundWave_t);  
	4) Create some sound patches, and define a function "int getNumberOfPatches()", which returns the number of patches defined by you. In other words:
		int getNumberOfPatches() {return sizeof(patches) / sizeof(patch_t);}
	5) Include at the bottom of this file, the headers of the soundWaves and patches. 
*/
#define AUDIO_ENABLED 0				// if 1, audio engine is enabled. To actually produce audio, you must also enable USE_MIXER. You also need to provide 
#define USE_MIXER (AUDIO_ENABLED)	// note: it must be 1 if AUDIO_ENABLED is 1... It can be 1 even if AUDIO_ENABLED is 0, but you will have to feed it with data!
#define INCLUDE_DEFAULT_WAVES 1	// if 1, the default sondwaves are included. 
// USB CONFIGURATION: MAX 2 Pipes and 1 device. Modify according to your needs. Be aware that HUB does not support low speed devices!
#define USE_USB_HUB 0
#define USB_NUMDEVICES 1
#define NUMBER_OF_USB_PIPES 3
#define MAX_USB_INTERFACES 4
// INCLUDE HERE THE HEADERS OF ALL YOUR DATA (for instance audio patches, sounds and sprites)
#include "VGASpriteData.h"
#if GFX_MODE == TILE_MODE1 && ENABLE_TRANSPARENT_SPRITES && SPRITES_ENABLED
	#include "transparentSpriteColorTable.h"
#endif
#if AUDIO_ENABLED
	#include "soundWaveList.h"
	#include "patches.h"
#endif
// do not modify the following lines if you don't know what you are doing!
#ifndef USE_BOOTLOADER
	#define FORCE_INCLUDE_USB_GAMEPAD_MODULE 1 // 0: use bootloader libraries and save space
	#define FORCE_INCLUDE_USB_MODULE 1			// 0: use bootloader libraries and save space
	#define FORCE_INCLUDE_USB_KEYBOARD_MODULE 1	// 0: use bootloader libraries and save space
#endif
#endif /* USVC_CONFIG_H_ */