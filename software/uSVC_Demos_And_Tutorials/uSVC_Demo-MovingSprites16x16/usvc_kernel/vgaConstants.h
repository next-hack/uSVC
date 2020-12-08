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
*  vgaConstants.h. useful defines for the VGA.
*/
#ifndef VGACONSTANTS_H_
#define VGACONSTANTS_H_

#define NO_FIXED_SECTION 0
#define TOP_FIXED_SECTION 1
#define BOTTOM_FIXED_SECTION 2
#define HORIZONTAL_FREQ 30000  //31469 is the right VGA value. 
#define HSYNC_PULSE (96*2) // since the clock is 48MHz, each pixel counts as 2
#define HSYNC_BACKPORCH (48*2)
#define MAX_Y_LINES (2 * SCREEN_SIZE_Y)		//
#define BITMAPPED_MODE 0
#define TILE_MODE0 1		// OBSOLETE, REMOVED!
#define TILE_MODE1 2		// 8bpp tile
#define TILE_MODE2 3		// 4bpp tile
#define TILE_MODE_BOOT 4	// do not use this mode, it is very limited and just for the bootloader!
//
#define SCREEN_SIZE_Y 200		// MUST NOT BE UL!
#define SCREEN_SIZE_X 320		// MUST NOT BE UL!
#define MAXCOLS  (SCREEN_SIZE_X / 8)
#define MAXROWS (SCREEN_SIZE_Y / 8)
#define COLOR(r,g,b) ((r & 0x7) | ((b & 0x1) << (19-16)) | ((g & 0x6) << (21-16)) | ((b & 0x02) << (30-16)) | ((g & 0x1) << (30-16)))
#define BICOLOR( r1, g1, b1, r2, g2, b2) ((COLOR(r1, g1, b1) << 16) | COLOR(r2, g2,b2))
#define LORES_COLOR(r, g, b) BICOLOR(r, g, b, r, g, b)
#define COLOR_TORGB332(red, green, blue)  ( (red & 1) | ((red & 4) >> 1) | ((red & 2) << 1)  | ((blue & 1) << 3) | ((green & 1) << 4) | ((blue & 2) << 4) | ((green & 2) << 5) | ((green & 4) << 5))
//
#define TILEMASK 0x3FFF			// Mask to exclude tile priority or semi transparency
// sprite flags
#define SPRITE_FLAGS_FLIP_HORIZONTAL 1
#define SPRITE_FLAGS_FLIP_VERTICAL 2
#define SPRITE_FLAGS_INVERTXY 4
#define SPRITE_FLAGS_INVERTXY180 (SPRITE_FLAGS_INVERTXY | SPRITE_FLAGS_FLIP_HORIZONTAL | SPRITE_FLAGS_FLIP_VERTICAL)
#define SPRITE_FLAGS_ROTATE_90 (SPRITE_FLAGS_INVERTXY | SPRITE_FLAGS_FLIP_HORIZONTAL )
#define SPRITE_FLAGS_ROTATE_270 (SPRITE_FLAGS_INVERTXY | SPRITE_FLAGS_FLIP_VERTICAL)
#define SPRITE_FLAGS_ROTATE_180 (SPRITE_FLAGS_FLIP_HORIZONTAL | SPRITE_FLAGS_FLIP_VERTICAL)
#define SPRITE_FLAGS_HANDLE_TOPLEFT 0x0000
#define SPRITE_FLAGS_HANDLE_TOPRIGHT 0x1000
#define SPRITE_FLAGS_HANDLE_BOTTOMLEFT 0x2000
#define SPRITE_FLAGS_HANDLE_BOTTOMRIGHT 0x3000
#define SPRITE_FLAGS_HANDLE_CENTER 0x4000
#define SPRITE_FLAGS_HANDLE_CENTER_TOP 0x5000
#define SPRITE_FLAGS_HANDLE_CENTER_BOTTOM 0x6000
#define SPRITE_FLAGS_ROTATION_MASK 0x0F
#define SPRITE_FLAGS_NO_X_SCROLL_CORRECTION 0x040  // the sprite will be printed at absolute logical x value, without x scroll 
#define SPRITE_FLAGS_NO_Y_SCROLL_CORRECTION 0x080 // the sprite will be printed at absolute logical y value, without y scroll 			
#define SPRITE_FLAGS_TRANSPARENT_SPRITE 0x0100				
#define SPRITE_FLAGS_PER_TILE_X_SCROLL_DEFORMATION 0x0200
#define SPRITE_FLAGS_PRIORITY_ALWAYS_TOP 0x0400
#define SPRITE_FLAGS_PRIORITY_ALWAYS_BOTTOM 0x0800
#define SPRITE_FLAGS_PRIORITY_MASK (SPRITE_FLAGS_PRIORITY_ALWAYS_TOP | SPRITE_FLAGS_PRIORITY_ALWAYS_BOTTOM)




#endif /* VGACONSTANTS_H_ */