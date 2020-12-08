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
*  vga.c/h. This is the main core of USVC. This file will provide the suport
*  for all the video modes. Do not change the assembly lines, as each line (especially on the 4BPP mode) 
*  has been carefully tweaked. Even changing the order of some unrelated instructions might break the code,
*  due to some bugs we discovered on the ATSAMD21 memory bus system.
*
*  Special note on the assembly. For some obscure reason the GCC suite treats CM0 assembly differently, and the assembler will complain when 
*  some instructions are used, claiming that they are not available in CM0 instruction set. 
*  In particular, we must write many instructions without the -S suffix, which tells the processor to set the flags accordingly to the result, but,
*  despite this, the instructions will be actually coded with the -s suffix! 
*  Let me tell you, what a dumb decision, as this makes code less portable between different cores (which should be code compatible at least!).
*  Yes, the -masm-syntax-unified might be used (but not without any risk), but when we figured out about this **extremely** dumb decision by those 
*  who programmed the GAS, it was too late, as too much code was already written. By the way by default the option is disabled! Facepalm.
*/
#include "usvc_kernel.h" 
#if (VRAMX < 40 || VRAMY < 25) && GFX_MODE != BITMAPPED_MODE
	#error VRAMX and VRAMY should be at least 40 and 25, respectively!
#endif
// NOTE: In ARM/GNU Linker -> General: Check Use size optimized library --specs=nano.specs
videoData_t videoData;
#if GFX_MODE == BITMAPPED_MODE
	// actual pixel buffer.
	uint8_t pixels[SCREEN_SIZE_X/4 * MAX_Y_LINES / 2] __attribute__ ((aligned (4)));
	#if PER_HORIZONTAL_BLOCK_PALETTE_REMAP
		/* The palette index table defines which palette has to be used for the 8x1 pixels block.
		   As for now, there are 256 palettes, but they are overlapped, to improve in some cases memory usage.
		   In particular, palette 0 point to paletteIndexTable[0], and palette n points to paletteIndexTable[4*n],
		   i.e. two contiguous palettes share 12 2-pixel entries.
		   This limits the actual color entry table to only 1024 2-pixel values. In future we might add a define switch, to allow
		   a larger palette separation  (8), to allow for a larger number of entries. This of course will have a cost in terms of memory 
		   (furthemore take into consideration that the pixel buffer takes 16k, the indexTable 8k, so very little memory is left. We could
		   also add another switch to consider larger blocks.*/
		uint8_t paletteIndexTable[SCREEN_SIZE_X / 8 * MAX_Y_LINES / 2];
		uint32_t palette[MAX_NUMBER_OF_PALETTE_ENTRIES] __attribute__ ((aligned(64)));		
	#else
		// if no color remapping is used the palette consists of 16 2-pixel color entries. Beware that which entry is used is determined by 2 pixel values!
		uint32_t palette[16] __attribute__ ((aligned(64)));
	#endif
#else
	/* Tile modes. In tile modes, the screen is drawn by tiles. There is a video ram (vram), which stores the lower part of the tile address or the tile number 
	(depending on the particular mode), and the actual tiles, which must reside in RAM, as we need single cycle access. As now, all the mode support 8x8 pixel tiles only.
	Tile modes available:
	- Tile mode boot is optimized for code size and it is used for the bootloader. It allows to show 256-color tiles, but it does not support any
	  scrolling, sprites, etc. The tiles store the actual raw pixel signal values, therefore each tile is 128-bytes large. Vertical row remapping is implemented too.
	- Tile Mode 0: no more used, it has been deprecated.
	- Tile Mode 1: It is an improvement of tile mode 0, because the video ram only stores the lower part of the tile addresses, halving therefore the
	  vram usage. Each tiles is 64 bytes large (8x8 bytes). It features:
	  - 256 onscreen colors.
	  - x and y screen scrolling.
	  - tiles and sprite priorities.
	  - vertical row remapping to create some nice effects, such as mirrors, split screen, etc.
	  - per tile and per row x scrolling.
	  - size independent sprites, with horizontal and vertical flip, mirror x-y, and 90-180-270 ° sprite rotation.
	  - "transparent" sprites (the pixel drawn depends both on the sprite pixel and the destination pixel values, allowing for nice transparency effects).
	  - Optional fixed section (see later).
	- Tile Mode 2: this is a 4 bpp (16 color) version of the tile mode 1. Each tile is only 32 bytes large (8x8/2), allowing for a larger number of tiles.
	   With respect to Tile Mode 1, there are the following difference:
	   - No transparent sprites.
	   - No 90 and 270° sprite rotations (only horizontal and vertical flips, or 180° rotation).
	   - Per line palette remapping. Each line defines which palette can be used. This remap can occur every high-res line (i.e. 400 lines). Beware that
	     each palette takes 1kB of RAM!
	   - Per line color change. On each high-res (400) or low-res (200) line, you can change the RGB value one of the 16 colors of any palette.
	   Finally, since the cortex M0 does not have nibble handling instructions, to improve the speed, sprites are stored in two version, one being the 1-pixel
	   horizontally translated version. This allows to deal with 2 pixels at once.
	
	Fixed section.
	This is still work in progress, as - see later- some improvements will be added (without breaking the existing code!). 
	It allows to define a top or bottom horizontal section, where you can place some information, like score, etc. It uses a separate ram-tile region and
	also a separate video ram, which stores the 8-bit index (not the address) of the fixed section tile to be drawn at that particular coordinates.
	Since each fixed section vram is an 8-bit index, only 256 fixed section tiles can be defined.
	Because of the method with which we draw sprites, the fixed section must be redrawn calling drawFixedSection(). This will be fixed later.
	You must define and declare the following extern variables, and include the header in which they are declared in usvc_config.h !
	For both modes: extern uint8_t fixedSectionMap[VRAMX * SECTION_LIMIT] __attribute__ ((aligned (4)));
	For tile mode 1: extern uint32_t fixedSectionTiles[MAX_FIXED_SECTION_TILES][TILE_SIZE_X/4*TILE_SIZE_Y] __attribute__ ((aligned (4)));
	For tile mode 2: extern uint32_t fixedSectionTiles[MAX_FIXED_SECTION_TILES][TILE_SIZE_X/8*TILE_SIZE_Y] __attribute__ ((aligned (4)));
	In usvc_config.h you must also select the type of section (TOP, BOTTOM, NONE) and decide where the section begin or ends (SECTION_LIMIT). 
	See the examples for more information!
	*/
	#if GFX_MODE != TILE_MODE_BOOT && ENABLE_TILE_PRIORITY != 0
		#ifndef TILES_ALL_OPAQUE
			#define TILES_ALL_OPAQUE 0
			#warning TILES_ALL_OPAQUE should be defined to 0 or 1. Defaulting to 0.
		#endif
	#endif
	#if GFX_MODE != TILE_MODE_BOOT && USE_ROW_REMAPPING != 0
		#ifndef USE_HIRES_ROW_REMAPPING
			#define USE_HIRES_ROW_REMAPPING 0
			#warning USE_HIRES_ROW_REMAPPING should be defined to 0 or 1. Defaulting to 0.
		#endif			
	#endif
	#if GFX_MODE == TILE_MODE_BOOT
		uint8_t vram[VRAMY * VRAMX] __attribute__ ((aligned (4)));
		uint16_t tiles[MAX_TILES][TILE_SIZE_X * TILE_SIZE_Y]  __attribute__ ((aligned (4)));
		uint8_t rowRemapTable[200];
	#elif GFX_MODE == TILE_MODE0
		void* vram[VRAMY * VRAMX] __attribute__ ((aligned (4)));
		uint32_t tiles[MAX_TILES][TILE_SIZE_X/4*TILE_SIZE_Y] __attribute__ ((aligned (4)));
	#elif GFX_MODE == TILE_MODE1
		uint16_t vram[VRAMY * VRAMX] __attribute__ ((aligned (4)));
		uint32_t tiles[MAX_TILES][TILE_SIZE_X/4*TILE_SIZE_Y] __attribute__ ((aligned (4)));
		// the actual code for tile drawing. Sorry for the comments if they actually do not correspond! A lot of reworking has been done!
		#define DRAW_TILE(n, rvram, offset) \
			"scroll0_tile" TOSTRING(n) "_start:\n\t" \
			"UXTB r7_temp3, r5_temp1\n\t"	/* get the first byte by zero extending the least significant byte of r5_temp1*/ \
			"MUL r7_temp3,  r2_mult_c\n\t" 		 /* get pixel signals by multiplying them for the magic constant :) */  \
			"REV16 r4_temp0,r5_temp1\n\t"			/* reverse bytes in halfwords, so that the 2nd and 3rd pixel bytes will be in "easy to separate" positions.*/ \
			"STRH r7_temp3,[r3_port]\n\t"		/* write pixel #0 signals to the port.*/ \
			\
			"scroll1_tile" TOSTRING(n) "_start:\n\t" \
			"UXTB r7_temp3, r4_temp0\n\t"			/* Get the byte of the second pixel (pixel #1) */ \
			"MUL r7_temp3, r2_mult_c\n\t"      	/* multiply it. Now in r7_temp3 we have the second pixel (pixel #1)*/	\
			"LSR r4_temp0, r4_temp0,#24\n\t"		 /* we have time for one more instruction. Let's extract pixel #2, which is the MSB of r5_temp1, which now is the last byte of r4_temp0*/ \
			"STRH r7_temp3,[r3_port]\n\t"		/* outut pixel #1*/ \
			\
			"scroll2_tile" TOSTRING(n) "_start:\n\t"  \
			"MUL r4_temp0,  r2_mult_c\n\t" 		 /* convert the byte value to the signals for pixel #2. */ \
			/* now r5_temp1 does not have to hold the entire qPixels, we can use it at wish!*/ \
			"LSR r5_temp1, r5_temp1, #24\n\t"		/* get the byte of the last pixel of the quad pixel*/\
			"MUL r5_temp1, r2_mult_c\n\t"			/* multiplication for pixel #3 */ \
			"STRH r4_temp0,[r3_port]\n\t"		/* write third pixel (pixel #2) */ \
			\
			"scroll3_tile" TOSTRING(n) "_start:\n\t" \
			"UXTB r7_temp3, r6_temp2\n\t"			/* get the first byte of r6_temp2 (pixel 4) by zero extending the least significant byte */ \
			"MUL r7_temp3,  r2_mult_c\n\t" 	 	/* get pixel signals by multiplying them for the magic constant :)*/ \
			"REV16 r4_temp0, r6_temp2\n\t"	 		/* reverse bytes in halfwords of the second quad pixels.*/\
			"STRH r5_temp1,[r3_port]\n\t"   /* write fourth pixel (pixel #3) */\			
			\
			/* second quad pixel. */ \
			"scroll4_tile" TOSTRING(n) "_start:\n\t"  \
			"UXTB r5_temp1, r4_temp0\n\t"		       /* Get the byte of the second pixel of Qpixel 2 (pixel #5), which is the first of r4_temp0 */ \
			"MUL r5_temp1, r2_mult_c\n\t"      	       /* multiply it. Now in r5_temp1 we have the sixth pixel (pixel #5)*/ \
			"LSR r4_temp0, r4_temp0,#24\n\t"		       /* we have time for one more instruction. Let's extract pixel #6, which was the MSB of r6_temp2.*/ \
			"STRH r7_temp3,[r3_port]\n\t"	           /* write  pixel #4 signals to the port.*/ \
			\
			"scroll5_tile" TOSTRING(n) "_start:\n\t"  \
			"MUL r4_temp0,  r2_mult_c\n\t" 		        /* convert the byte to the signals for pixel #6.*/ \
			"LSR r7_temp3, r6_temp2, #24\n\t"		 /* get the byte of the last pixel (pixel #7)*/ \
			"MUL r7_temp3, r2_mult_c\n\t"			 /* multiplication for pixel #7*/ \
			"STRH r5_temp1,[r3_port]\n\t"		/* outut pixel #5*/ \
			\
			"scroll6_tile" TOSTRING(n) "_start:\n\t"  \
			/* we need now to get the new Tile Address */   \
			"LDRH r5_temp1,[" rvram ",#" TOSTRING(offset) "]\n\t"			/* get the address of next tile.*/ \
			"ADD r5_temp1, r10_offset\n\t"                /* add the offset which contains the higher address + offset of the first pixel to be drawn, relative to the tile*/ \
			"STRH r4_temp0,[r3_port]\n\t"		          /* write pixel 6*/ \
			\
			"scroll7_tile" TOSTRING(n) "_start:\n\t"  \
			/* already load 8 pixels for the next tile!*/ \
			"LDM r5_temp1, {r5_temp1, r6_temp2}\n\t"            /* this takes 3 cycles!*/ \
			"STRH r7_temp3,[r3_port]\n\t"                     /* write pixel 7*/ 
		
		#define DRAW_LAST_COMPLETE_TILE(n, rvram, offset) \
			"scroll0_tile" TOSTRING(n) "_start:\n\t" \
			"UXTB r7_temp3, r5_temp1\n\t"	/* get the first byte by zero extending the least significant byte of r5_temp1*/ \
			"MUL r7_temp3,  r2_mult_c\n\t" 		 /* get pixel signals by multiplying them for the magic constant :) */  \
			"REV16 r4_temp0,r5_temp1\n\t"			/* reverse bytes in halfwords, so that the 2nd and 3rd pixel bytes will be in "easy to separate" positions.*/ \
			"STRH r7_temp3,[r3_port]\n\t"		/* write pixel #0 signals to the port.*/ \
			\
			"scroll1_tile" TOSTRING(n) "_start:\n\t" \
			"UXTB r7_temp3, r4_temp0\n\t"			/* Get the byte of the second pixel (pixel #1) */ \
			"MUL r7_temp3, r2_mult_c\n\t"      	/* multiply it. Now in rQpixel3 we have the second pixel (pixel #1)*/	\
			"LSR r4_temp0, r4_temp0,#24\n\t"		 /* we have time for one more instruction. Let's extract pixel #2, which is the MSB of r5_temp1, which now is the last byte of rTemp*/ \
			"STRH r7_temp3,[r3_port]\n\t"		/* outut pixel #1*/ \
			\
			"scroll2_tile" TOSTRING(n) "_start:\n\t"  \
			"MUL r4_temp0,  r2_mult_c\n\t" 		 /* convert the byte value to the signals for pixel #2. */ \
			/* now r5_temp1 does not have to hold the entire qPixels, we can use it at wish!*/ \
			"LSR r5_temp1, r5_temp1, #24\n\t"		/* get the byte of the last pixel of the quad pixel*/\
			"MUL r5_temp1, r2_mult_c\n\t"			/* multiplication for pixel #3 */ \
			"STRH r4_temp0,[r3_port]\n\t"		/* write third pixel (pixel #2) */ \
			\
			"scroll3_tile" TOSTRING(n) "_start:\n\t" \
			"UXTB r7_temp3, r6_temp2\n\t"			/* get the first byte of r6_temp2 (pixel 4) by zero extending the least significant byte */ \
			"MUL r7_temp3,  r2_mult_c\n\t" 	 	/* get pixel signals by multiplying them for the magic constant :)*/ \
			"REV16 r4_temp0, r6_temp2\n\t"	 		/* reverse bytes in halfwords of the second quad pixels.*/\
			"STRH r5_temp1,[r3_port]\n\t"   /* write fourth pixel (pixel #3) */\			
			\
			/* second quad pixel. */ \
			"scroll4_tile" TOSTRING(n) "_start:\n\t"  \
			"UXTB r5_temp1, r4_temp0\n\t"		       /* Get the byte of the second pixel of rQpixel 2 (pixel #5), which is the first of r4_temp0 */ \
			"MUL r5_temp1, r2_mult_c\n\t"      	       /* multiply it. Now in r5_temp1 we have the sixth pixel (pixel #5)*/ \
			"LSR r4_temp0, r4_temp0,#24\n\t"		       /* we have time for one more instruction. Let's extract pixel #6, which was the MSB of r6_temp2.*/ \
			"STRH r7_temp3,[r3_port]\n\t"	           /* write  pixel #4 signals to the port.*/ \
			\
			"scroll5_tile" TOSTRING(n) "_start:\n\t"  \
			"MUL r4_temp0,  r2_mult_c\n\t" 		        /* convert the byte to the signals for pixel #6.*/ \
			"LSR r7_temp3, r6_temp2, #24\n\t"		 /* get the byte of the last pixel (pixel #7)*/ \
			"MUL r7_temp3, r2_mult_c\n\t"			 /* multiplication for pixel #7*/ \
			"STRH r5_temp1,[r3_port]\n\t"		/* outut pixel #5*/ \
			\
			"scroll6_tile" TOSTRING(n) "_start:\n\t"  \
			"NOP\n\t"  \
			"NOP\n\t"  \
			"NOP\n\t"  \
			"STRH r4_temp0,[r3_port]\n\t"		          /* write pixel 6*/ \
			\
			"scroll7_tile" TOSTRING(n) "_start:\n\t"  \
			"NOP\n\t"  \
			"MOV r5_temp1,r8_x_scroll\n\t"   /* get next 4 pixels */  \
			"MOV r6_temp2,r9_y_scroll\n\t" 	/* get last 4 pixels.*/ \
			"STRH r7_temp3,[r3_port]\n\t"                     /* write pixel 7*/ 
	#elif GFX_MODE == TILE_MODE2
		#if SPRITES_ENABLED
			inline void drawSpriteOnTopOfTile(int32_t y, uint8_t *pp, uint8_t* dest, int32_t vi, int32_t deltaX);
			inline void drawSpriteUnderTile(int32_t y, uint8_t *pp, uint8_t* dest, int32_t vi, int32_t deltaX);
			inline void drawMirrorSpriteOnTopOfTile(int32_t y, uint8_t *pp, uint8_t* dest, int32_t vi, int32_t deltaX);
			inline void drawMirrorSpriteUnderTile(int32_t y, uint8_t *pp, uint8_t* dest, int32_t vi, int32_t deltaX);	
			const uint8_t nibbleMask[256] =   // this lookup table quickly converts two 4-bit pixels to a mask, in order to quickly determine which pixels should be copied.
			{
				0xFF, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 
				0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,			
			};    		
		#endif		
		uint16_t vram[VRAMY * VRAMX + 2] __attribute__ ((aligned (4)));	
		uint32_t palette[MAX_NUMBER_OF_PALETTES * 256] __attribute__ ((aligned(8)));		
		// the actual code for tile drawing. Sorry for the comments if they actually do not correspond! A lot of reworking has been done!
		// Note: tile mode 2 is very code sensitive, due to the actual internal bus handling of the ATSAMD21.
		// The code has been tweaked to achieve the correct operation. Even swapping the position of a two instructions might lead to corrupted graphics!
		// The first two tiles are drawn as they are.
		#define DRAW_FIRST_TWO_TILES(n) \
		"scroll0_bitile" TOSTRING(n) "_start:\n\t" \
		"AND bPixel,AndMask\n\t"			/* get the two 4-bit indexes of p0 and p1 */ \
		"LDR bPixel,[pPalette, bPixel]\n\t"		/* load p0 and p1 RGB values */ \
		"STRH bPixel,[rPort, #2]\n\t"			/* output p0 */ \
		\
		"scroll1_bitile" TOSTRING(n) "_start:\n\t" \
		"LSR rTemp,Opixel,#6\n\t"			/* shift the 32-bit pixel data so that indexes of p2 and p3 in the correct positions */ \
		"NOP\n\t"		\
		"NOP\n\t" 	\
		"STR bPixel,[rPort]\n\t"			/* output p1 */ \
		\
		"scroll2_bitile" TOSTRING(n) "_start:\n\t" \
		"AND rTemp,AndMask\n\t"				/* get the two 4-bit indexes of p2 and p3 */ \
		"LDR rTemp,[pPalette, rTemp]\n\t"			/* load p2 and p3 RGB values */ \
		"STRH rTemp,[rPort, #2]\n\t"			/* output p2 */ \
		\
		"scroll3_bitile" TOSTRING(n) "_start:\n\t" \
		"LSR bPixel,Opixel,#14\n\t"			/* shift the 32-bit pixel data so that indexes of p4 and p5 are in the correct positions */ \
		"NOP\n\t"	\
		"NOP\n\t"	\
		"STR rTemp,[rPort]\n\t"				/* output p3 */ \
		\
		"scroll4_bitile" TOSTRING(n) "_start:\n\t" \
		"AND bPixel,AndMask\n\t"			/* get the two 4-bit indexes p4 and p5. */ \
		"LDR bPixel,[pPalette, bPixel]\n\t"		/* load p4 and p5 RGB values */ \
		"STRH bPixel,[rPort, #2]\n\t"			    /* output p4 */ \
		\
		"scroll5_bitile" TOSTRING(n) "_start:\n\t" \
		"LSR tileAddresses, #16\n\t"			/* extract now the new tile address (p8-p15). We do it here only because we have a spare cycle. */ \
		"LSR rTemp,Opixel,#22\n\t"			    /* now rTemp is free. We can shift Opixel so that the next two 4-bit indexes - p6 and p7 - are in the correct place */ \
		"NOP\n\t" \
		"STR bPixel,[rPort]\n\t"		/* ouput p5 */ \
		\
		"scroll6_bitile" TOSTRING(n) "_start:\n\t" \
		"AND rTemp,AndMask\n\t"			/* get the two 4-bit indexes of p6 and p7 */ \
		"LDR rTemp,[pPalette,rTemp]\n\t"		/* load p6 and p7 RGB values */ \
		"STRH rTemp,[rPort, #2]\n\t"		/* output p6 */ \
		\
		"scroll7_bitile" TOSTRING(n) "_start:\n\t" \
		"LDR tileAddresses,[tileAddresses, Offset]\n\t"     /* tileaddress is no more used now. it will be used as Opixel for the next 8 pixels. */ \
		"LSL bPixel,tileAddresses,#2\n\t"	/* shift tileAddresses - now used as Opixel - in bPixel, so that p8 and p9 are in the right positions */ \
		"STR rTemp, [rPort]\n\t"			/* output p7 */ \
		\
		"scroll8_bitile" TOSTRING(n) "_start:\n\t" \
		"AND bPixel,AndMask\n\t"	        /* get the two 4-bit indexes of p8 and p9 */ \
		"LDR bPixel,[pPalette, bPixel]\n\t"	/* load p8 and p9 RGB values */ \
		"STRH bPixel,[rPort, #2]\n\t"			      		/* output p8*/ \
		\
		"scroll9_bitile" TOSTRING(n) "_start:\n\t" \
		"NOP\n\t"  \
		"NOP\n\t"  \
		"LSR Opixel,tileAddresses,#6\n\t"       /* shift tileAddresses - now used as Opixel - in bPixel, so that p10 and p11 are in the right positions */ \
		"STR bPixel,[rPort]\n\t"			/* output p9*/ \
		\
		"scroll10_bitile" TOSTRING(n) "_start:\n\t" \
		"AND Opixel,AndMask\n\t"	   		/* some instructions above, we prepared in Opixel the tile pixel index data so that p10 and p11 were in the right position for this and operation. */ \
		"LDR rTemp,[pPalette, Opixel]\n\t"		/* load the RGB values of p10 and p11 */ \
		"STRH rTemp,[rPort, #2]\n\t"			/* output p10 */ \
		\
		"scroll11_bitile" TOSTRING(n) "_start:\n\t" \
		"LSR Opixel,tileAddresses,#22\n\t"		/* move p14 and p15 pixel indexes in the right positions */ \
		"LSR bPixel,tileAddresses,#14\n\t"		/* move p12 and p13 pixel indexes in the right positions */ \
		"NOP\n\t" \
		"STR rTemp,[rPort]\n\t"				/* output p11 */ \
		\
		"scroll12_bitile" TOSTRING(n) "_start:\n\t" \
		"AND bPixel,AndMask\n\t"			/* extract p12 and p13 indexes */ \
		"LDR bPixel,[pPalette, bPixel]\n\t"		/* load p12 and p13 RGB values */ \
		"STRH bPixel,[rPort, #2]\n\t"			/* output p12 */ \
		\
		"scroll13_bitile" TOSTRING(n) "_start:\n\t" \
		"POP {tileAddresses}\n\t"			/* get the lower parts of the next two tile addresses - the higher is always 0x2000 */ \
		"AND Opixel,AndMask\n\t"			/* extract p14 and p15 indexes*/ \
		"STR bPixel,[rPort]\n\t"			/* output p13 */ \
		\
		"scroll14_bitile" TOSTRING(n) "_start:\n\t" \
		"LDR rTemp,[pPalette, Opixel]\n\t"             /* load p14 and p15 rgb values */ \
		"UXTH bPixel, tileAddresses\n\t"	      /* extract the lower part of first tile address */ \
		"STRH rTemp,[rPort, #2]\n\t"		      /* output p14 */ \
		\
		"scroll15_bitile" TOSTRING(n) "_start:\n\t" \
		"LDR Opixel,[bPixel, Offset]\n\t"	     /* get the 8 4-bit indexes of the first tile in the pair */ \
		"LSL bPixel,Opixel,#2\n\t"		     /* shift left the indexes so that those of p0 and p1 are in the right place - for word alignment */ \
		"STR rTemp,[rPort]\n\t"			     /* finally output p15! */
		// Here comes the most interesting part. We can insert some instructions in the spare cycles, to allow color changing operations.
		// There are several versions of this macro, to allow for different code snippet usages (as LDR/STR take 2 cycles).
		#define DRAW_TWO_TILES(n, first_cycle, second_to_fourth_cycles,  fifth_to_seventh_cycles, eigth_cycle) \
		"scroll0_bitile" TOSTRING(n) "_start:\n\t" \
		"AND bPixel,AndMask\n\t"			/* get the two 4-bit indexes of p0 and p1 */ \
		"LDR bPixel,[pPalette, bPixel]\n\t"		/* load p0 and p1 RGB values */ \
		"STRH bPixel,[rPort, #2]\n\t"			/* output p0 */ \
		\
		"scroll1_bitile" TOSTRING(n) "_start:\n\t" \
		"LSR rTemp,Opixel,#6\n\t"			/* shift the 32-bit pixel data so that indexes of p2 and p3 in the correct positions */ \
		"AND rTemp,AndMask\n\t"				/* get the two 4-bit indexes of p2 and p3 */ \
		"LSR tileAddresses, #16\n\t"			/* extract now the new tile address (p8-p15). We do it here only because we have a spare cycle. */ \
		"STR bPixel,[rPort]\n\t"			/* output p1 */ \
		\
		"scroll2_bitile" TOSTRING(n) "_start:\n\t" \
		"LDR rTemp,[pPalette, rTemp]\n\t"			/* load p2 and p3 RGB values */ \
		"LSR bPixel,Opixel,#14\n\t"			/* shift the 32-bit pixel data so that indexes of p4 and p5 are in the correct positions */ \
		"STRH rTemp,[rPort, #2]\n\t"			/* output p2 */ \
		\
		"scroll3_bitile" TOSTRING(n) "_start:\n\t" \
		"AND bPixel,AndMask\n\t"			/* get the two 4-bit indexes p4 and p5. */ \
		"LDR bPixel,[pPalette, bPixel]\n\t"		/* load p4 and p5 RGB values */ \
		"STR rTemp,[rPort]\n\t"				/* output p3 */ \
		\
		"scroll4_bitile" TOSTRING(n) "_start:\n\t" \
		"LDR tileAddresses,[tileAddresses, Offset]\n\t"     /* tileaddress is no more used now. it will be used as Opixel for the next 8 pixels. */ \
		"LSR rTemp,Opixel,#22\n\t"			    /* now rTemp is free. We can shift Opixel so that the next two 4-bit indexes - p6 and p7 - are in the correct place */ \
		"STRH bPixel,[rPort, #2]\n\t"			    /* output p4 */ \
		\
		"scroll5_bitile" TOSTRING(n) "_start:\n\t" \
		"AND rTemp,AndMask\n\t"			/* get the two 4-bit indexes of p6 and p7 */ \
		"LDR rTemp,[pPalette,rTemp]\n\t"		/* load p6 and p7 RGB values */ \
		"STR bPixel,[rPort]\n\t"		/* ouput p5 */ \
		\
		"scroll6_bitile" TOSTRING(n) "_start:\n\t" \
		"LSL bPixel,tileAddresses,#2\n\t"	/* shift tileAddresses - now used as Opixel - in bPixel, so that p8 and p9 are in the right positions */ \
		"AND bPixel,AndMask\n\t"	        /* get the two 4-bit indexes of p8 and p9 */ \
		"LSR Opixel,tileAddresses,#6\n\t"       /* shift tileAddresses - now used as Opixel - in bPixel, so that p10 and p11 are in the right positions */ \
		"STRH rTemp,[rPort, #2]\n\t"		/* output p6 */ \
		\
		"scroll7_bitile" TOSTRING(n) "_start:\n\t" \
		"LDR bPixel,[pPalette, bPixel]\n\t"	/* load p8 and p9 RGB values */ \
		first_cycle \
		"STR rTemp, [rPort]\n\t"			/* output p7 */ \
		\
		"scroll8_bitile" TOSTRING(n) "_start:\n\t" \
		second_to_fourth_cycles \
		"STRH bPixel,[rPort, #2]\n\t"			      		/* output p8*/ \
		\
		"scroll9_bitile" TOSTRING(n) "_start:\n\t" \
		fifth_to_seventh_cycles \
	   "STR bPixel,[rPort]\n\t"			/* output p9*/ \
		\
		"scroll10_bitile" TOSTRING(n) "_start:\n\t" \
		"AND Opixel,AndMask\n\t"	   		/* 11 instructions above, we prepared in Opixel the tile pixel index data so that p10 and p11 were in the right position for this and operation. */ \
		"LDR rTemp,[pPalette, Opixel]\n\t"		/* load the RGB values of p10 and p11 */ \
		"STRH rTemp,[rPort, #2]\n\t"			/* output p10 */ \
		\
		"scroll11_bitile" TOSTRING(n) "_start:\n\t" \
		"LSR Opixel,tileAddresses,#22\n\t"		/* move p14 and p15 pixel indexes in the right positions */ \
		"LSR bPixel,tileAddresses,#14\n\t"		/* move p12 and p13 pixel indexes in the right positions */ \
		"AND bPixel,AndMask\n\t"			/* extract p12 and p13 indexes */ \
		"STR rTemp,[rPort]\n\t"				/* output p11 */ \
		\
		"scroll12_bitile" TOSTRING(n) "_start:\n\t" \
		"LDR bPixel,[pPalette, bPixel]\n\t"		/* load p12 and p13 RGB values */ \
		"AND Opixel,AndMask\n\t"			/* extract p14 and p15 indexes*/ \
		"STRH bPixel,[rPort, #2]\n\t"			/* output p12 */ \
		\
		"scroll13_bitile" TOSTRING(n) "_start:\n\t" \
		"POP {tileAddresses}\n\t"			/* get the lower parts of the next two tile addresses - the higher is always 0x2000 */ \
		eigth_cycle \
		"STR bPixel,[rPort]\n\t"			/* output p13 */ \
		\
		"scroll14_bitile" TOSTRING(n) "_start:\n\t" \
		"LDR rTemp,[pPalette, Opixel]\n\t"             /* load p14 and p15 rgb values */ \
		"UXTH bPixel, tileAddresses\n\t"	      /* extract the lower part of first tile address */ \
		"STRH rTemp,[rPort, #2]\n\t"		      /* output p14 */ \
		\
		"scroll15_bitile" TOSTRING(n) "_start:\n\t" \
		"LDR Opixel,[bPixel, Offset]\n\t"	     /* get the 8 4-bit indexes of the first tile in the pair */ \
		"LSL bPixel,Opixel,#2\n\t"		     /* shift left the indexes so that those of p0 and p1 are in the right place - for word alignment */ \
		"STR rTemp,[rPort]\n\t"			     /* finally output p15! */		
		//  This is slightly modified with respect the previous, to allow for a different instruction sequence.		
		#define DRAW_TWO_TILES2(n, first_cycle, second_cycle, third_to_fifth_cycles,  sixth_cycle, seventh_to_eigth_cycles) \
		"scroll0_bitile" TOSTRING(n) "_start:\n\t" \
		"AND bPixel,AndMask\n\t"			/* get the two 4-bit indexes of p0 and p1 */ \
		"LDR bPixel,[pPalette, bPixel]\n\t"		/* load p0 and p1 RGB values */ \
		"STRH bPixel,[rPort, #2]\n\t"			/* output p0 */ \
		\
		"scroll1_bitile" TOSTRING(n) "_start:\n\t" \
		"LSR rTemp,Opixel,#6\n\t"			/* shift the 32-bit pixel data so that indexes of p2 and p3 in the correct positions */ \
		"AND rTemp,AndMask\n\t"				/* get the two 4-bit indexes of p2 and p3 */ \
		"LSR tileAddresses, #16\n\t"			/* extract now the new tile address (p8-p15). We do it here only because we have a spare cycle. */ \
		"STR bPixel,[rPort]\n\t"			/* output p1 */ \
		\
		"scroll2_bitile" TOSTRING(n) "_start:\n\t" \
		"LDR rTemp,[pPalette, rTemp]\n\t"			/* load p2 and p3 RGB values */ \
		"LSR bPixel,Opixel,#14\n\t"			/* shift the 32-bit pixel data so that indexes of p4 and p5 are in the correct positions */ \
		"STRH rTemp,[rPort, #2]\n\t"			/* output p2 */ \
		\
		"scroll3_bitile" TOSTRING(n) "_start:\n\t" \
		"AND bPixel,AndMask\n\t"			/* get the two 4-bit indexes p4 and p5. */ \
		"LDR bPixel,[pPalette, bPixel]\n\t"		/* load p4 and p5 RGB values */ \
		"STR rTemp,[rPort]\n\t"				/* output p3 */ \
		\
		"scroll4_bitile" TOSTRING(n) "_start:\n\t" \
		"LSR rTemp,Opixel,#22\n\t"			    /* now rTemp is free. We can shift Opixel so that the next two 4-bit indexes - p6 and p7 - are in the correct place */ \
		"LDR tileAddresses,[tileAddresses, Offset]\n\t"     /* tileaddress is no more used now. it will be used as Opixel for the next 8 pixels. */ \
		"STRH bPixel,[rPort, #2]\n\t"			    /* output p4 */ \
		\
		"scroll5_bitile" TOSTRING(n) "_start:\n\t" \
		"AND rTemp,AndMask\n\t"			/* get the two 4-bit indexes of p6 and p7 */ \
		"LDR Opixel,[pPalette,rTemp]\n\t"		/* load p6 and p7 RGB values */ \
		"STR bPixel,[rPort]\n\t"		/* ouput p5 */ \
		\
		"scroll6_bitile" TOSTRING(n) "_start:\n\t" \
		"LSL bPixel,tileAddresses,#2\n\t"	/* shift tileAddresses - now used as Opixel - in bPixel, so that p8 and p9 are in the right positions */ \
		"AND bPixel,AndMask\n\t"	        /* get the two 4-bit indexes of p8 and p9 */ \
		first_cycle \
		"STRH Opixel,[rPort, #2]\n\t"		/* output p6 */ \
		\
		"scroll7_bitile" TOSTRING(n) "_start:\n\t" \
		"LDR bPixel,[pPalette, bPixel]\n\t"	/* load p8 and p9 RGB values */ \
		second_cycle \
		"STR Opixel, [rPort]\n\t"			/* output p7 */ \
		\
		"scroll8_bitile" TOSTRING(n) "_start:\n\t" \
		third_to_fifth_cycles \
		"STRH bPixel,[rPort, #2]\n\t"			      		/* output p8*/ \
		\
		"scroll9_bitile" TOSTRING(n) "_start:\n\t" \
		"LSR Opixel,tileAddresses,#6\n\t"       /* shift tileAddresses - now used as Opixel - in bPixel, so that p10 and p11 are in the right positions */ \
		"AND Opixel,AndMask\n\t"	   		/* 11 instructions above, we prepared in Opixel the tile pixel index data so that p10 and p11 were in the right position for this and operation. */ \
		"LSR tileAddresses,tileAddresses,#14\n\t"		/* move p12 and p13 pixel indexes in the right positions */ \
		"STR bPixel,[rPort]\n\t"			/* output p9*/ \
		\
		"scroll10_bitile" TOSTRING(n) "_start:\n\t" \
		"LDR Opixel,[pPalette, Opixel]\n\t"		/* load the RGB values of p10 and p11 */ \
		sixth_cycle \
		"STRH Opixel,[rPort, #2]\n\t"			/* output p10 */ \
		\
		"scroll11_bitile" TOSTRING(n) "_start:\n\t" \
		seventh_to_eigth_cycles \
		"LSR rTemp,tileAddresses,#(22 - 14)\n\t"		/* move p14 and p15 pixel indexes in the right positions */ \
		"STR Opixel,[rPort]\n\t"				/* output p11 */ \
		\
		"scroll12_bitile" TOSTRING(n) "_start:\n\t" \
		"AND tileAddresses,AndMask\n\t"			/* extract p12 and p13 indexes */ \
		"LDR bPixel,[pPalette, tileAddresses]\n\t"		/* load p12 and p13 RGB values */ \
		"STRH bPixel,[rPort, #2]\n\t"			/* output p12 */ \
		\
		"scroll13_bitile" TOSTRING(n) "_start:\n\t" \
		"AND rTemp,AndMask\n\t"			/* extract p14 and p15 indexes*/ \
		"POP {tileAddresses}\n\t"			/* get the lower parts of the next two tile addresses - the higher is always 0x2000 */ \
		"STR bPixel,[rPort]\n\t"			/* output p13 */ \
		\
		"scroll14_bitile" TOSTRING(n) "_start:\n\t" \
		"LDR rTemp,[pPalette, rTemp]\n\t"             /* load p14 and p15 rgb values */ \
		"UXTH bPixel, tileAddresses\n\t"	      /* extract the lower part of first tile address */ \
		"STRH rTemp,[rPort, #2]\n\t"		      /* output p14 */ \
		\
		"scroll15_bitile" TOSTRING(n) "_start:\n\t" \
		"LDR Opixel,[bPixel, Offset]\n\t"	     /* get the 8 4-bit indexes of the first tile in the pair */ \
		"LSL bPixel,Opixel,#2\n\t"		     /* shift left the indexes so that those of p0 and p1 are in the right place - for word alignment */ \
		"STR rTemp,[rPort]\n\t"			     /* finally output p15! */
		//  This is slightly modified with respect the previous, in preparation for the last macro, that draws partial tiles.		
		#define DRAW_LAST_TWO_TILES() \
		"AND bPixel,AndMask\n\t"			/* get the two 4-bit indexes of p0 and p1 */ \
		"LDR bPixel,[pPalette, bPixel]\n\t"		/* load p0 and p1 RGB values */ \
		"STRH bPixel,[rPort, #2]\n\t"			/* output p0 */ \
		\
		"LSR rTemp,Opixel,#6\n\t"			/* shift the 32-bit pixel data so that indexes of p2 and p3 in the correct positions */ \
		"AND rTemp,AndMask\n\t"				/* get the two 4-bit indexes of p2 and p3 */ \
		"LSR tileAddresses, #16\n\t"			/* extract now the new tile address (p8-p15). We do it here only because we have a spare cycle. */ \
		"STR bPixel,[rPort]\n\t"			/* output p1 */ \
		\
		"LDR rTemp,[pPalette, rTemp]\n\t"			/* load p2 and p3 RGB values */ \
		"LSR bPixel,Opixel,#14\n\t"			/* shift the 32-bit pixel data so that indexes of p4 and p5 are in the correct positions */ \
		"STRH rTemp,[rPort, #2]\n\t"			/* output p2 */ \
		\
		"AND bPixel,AndMask\n\t"			/* get the two 4-bit indexes p4 and p5. */ \
		"LDR bPixel,[pPalette, bPixel]\n\t"		/* load p4 and p5 RGB values */ \
		"STR rTemp,[rPort]\n\t"				/* output p3 */ \
		\
		"LDR tileAddresses,[tileAddresses, Offset]\n\t"     /* tileaddress is no more used now. it will be used as Opixel for the next 8 pixels. */ \
		"LSR rTemp,Opixel,#22\n\t"			    /* now rTemp is free. We can shift Opixel so that the next two 4-bit indexes - p6 and p7 - are in the correct place */ \
		"STRH bPixel,[rPort, #2]\n\t"			    /* output p4 */ \
		\
		"AND rTemp,AndMask\n\t"			/* get the two 4-bit indexes of p6 and p7 */ \
		"LDR rTemp,[pPalette,rTemp]\n\t"		/* load p6 and p7 RGB values */ \
		"STR bPixel,[rPort]\n\t"		/* ouput p5 */ \
		\
		"MOV SP,rStackPtrBackup\n\t" \
		"LDR Opixel,[SP, #8]\n\t" 			/* MODIFIED: load from the stack the first 8 4-bit indexes of the last partial bi-tile */\
		"STRH rTemp,[rPort, #2]\n\t"		/* output p6 */ \
		\
		"LSL bPixel,tileAddresses,#2\n\t"	/* shift tileAddresses - now used as Opixel - in bPixel, so that p8 and p9 are in the right positions */ \
		"AND bPixel,AndMask\n\t"	        /* get the two 4-bit indexes of p8 and p9 */ \
		"UXTB Opixel,Opixel\n\t"			/* MODIFIED. Instead of a shift + AND, we perform a UXTB */ \
		"STR rTemp, [rPort]\n\t"			/* output p7 */ \
		\
		"LSL rTemp, Opixel, #2\n\t"			/* SHIFT LEFT By two */ \
		"LDR bPixel,[pPalette, bPixel]\n\t"	/* load p8 and p9 RGB values */ \
		"STRH bPixel,[rPort, #2]\n\t"			      		/* output p8*/ \
		\
		"LDR rTemp2, [pPalette, rTemp]\n\t"		/*NB: rTemp2 = Offset, no more used! MODIFIED get the RGB signals of p0 and p1 of the last partial bi-tile */ \
		"LSR Opixel,tileAddresses,#6\n\t"       /* shift tileAddresses - now used as Opixel - in bPixel, so that p10 and p11 are in the right positions */ \
		\
		"STR bPixel,[rPort]\n\t"			/* output p9*/ \
		\
		"AND Opixel,AndMask\n\t"	   		/*  some instructions above, we prepared in Opixel the tile pixel index data so that p10 and p11 were in the right position for this and operation. */ \
		"LDR rTemp,[pPalette, Opixel]\n\t"		/* load the RGB values of p10 and p11 */ \
		"STRH rTemp,[rPort, #2]\n\t"			/* output p10 */ \
		\
		"LSR bPixel,tileAddresses,#14\n\t"		/* move p12 and p13 pixel indexes in the right positions */ \
		"LSR Opixel,tileAddresses,#22\n\t"		/* move p14 and p15 pixel indexes in the right positions.*/ \
		"AND bPixel,AndMask\n\t"			/* extract p12 and p13 indexes */ \
		"STR rTemp,[rPort]\n\t"				/* output p11 */ \
		\
		"AND Opixel, AndMask\n\t"			/* extract p14 and p15 indexes. */ \
		"LDR rTemp,[pPalette, bPixel]\n\t"		/* load p12 and p13 RGB values */ \
		"STRH rTemp,[rPort, #2]\n\t"			/* output p12 */ \
		\
		"POP {rShift, Opixel2}\n\t"			/*  MODIFIED: Get rShift and rWhereTojump*/ \
		"STR rTemp,[rPort]\n\t"			/* output p13 */ \
		\
		"MOV rWhereToJump, Opixel2\n\t"			  \
		"LDR rTemp,[pPalette, Opixel]\n\t"             /* load p14 and p15 RGB values */ \
		"STRH rTemp,[rPort, #2]\n\t"		      /* output p14 */ \
		\
		"POP {Opixel, Opixel2}\n\t"			/*  MODIFIED: Get Opixel and Opixel2*/ \
		"STR rTemp,[rPort]\n\t"			     /* finally output p15! */ \
		"CMP rShift, #1\n\t" 					/* used in case of odd shift" */ \
		"BX rWhereToJump\n\t"	
		// this draws the last partial tiles, due to x scrolling.
		#define DRAW_PARTIAL_TILES() \
		"evenShift:\n\r" \
		/* p0 */ \
		"STRH rTemp2, [rPort,#2]\n\r" \
		"LSR  rTemp,Opixel, #6\n\r" \
		"AND  rTemp, AndMask\n\r" \
		"CMP rShift, #2\n\r" \
		/* p1 */ \
		"STR  rTemp2,[rPort]\n\r" \
		"BEQ endDraw\n\r" \
		"LDR  rTemp2, [pPalette, rTemp]\n\r" \
		/* p2 */ \
		"STRH rTemp2, [rPort,#2]\n\r" \
		"LSR  rTemp,Opixel, #14\n\r" \
		"AND  rTemp, AndMask\n\r" \
		"CMP rShift, #4\n\r" \
		/* p3 */ \
		"STR  rTemp2, [rPort]\n\r" \
		"BEQ endDraw\n\r" \
		"LDR  rTemp2, [pPalette, rTemp]\n\r" \
		/* p4 */ \
		"STRH rTemp2, [rPort,#2]\n\r" \
		"LSR  rTemp,Opixel, #22\n\r" \
		"AND  rTemp, AndMask\n\r" \
		"CMP rShift, #6\n\r" \
		/* p5 */ \
		"STR  rTemp2,[rPort]\n\r" \
		"BEQ endDraw\n\r" \
		"LDR  rTemp2, [pPalette, rTemp]\n\r" \
		/* p6 */ \
		"STRH rTemp2, [rPort,#2]\n\r" \
		"LSL  rTemp,Opixel2, #2\n\r" \
		"AND  rTemp, AndMask\n\r" \
		"CMP rShift, #8\n\r" \
		/* p7 */ \
		"STR  rTemp2, [rPort]\n\r" \
		"BEQ endDraw\n\r" \
		"LDR  rTemp2, [pPalette, rTemp]\n\r" \
		/* p8 */ \
		"STRH rTemp2, [rPort,#2]\n\r" \
		"LSR  rTemp,Opixel2, #6\n\r" \
		"AND  rTemp, AndMask\n\r" \
		"CMP rShift, #10\n\r" \
		/* p9 */ \
		"STR  rTemp2,[rPort]\n\r" \
		"BEQ endDraw\n\r" \
		"LDR  rTemp2, [pPalette, rTemp]\n\r" \
		/* p10 */ \
		"STRH rTemp2, [rPort,#2]\n\r" \
		"LSR  rTemp,Opixel2, #14\n\r" \
		"AND  rTemp, AndMask\n\r" \
		"CMP rShift, #12\n\r" \
		/* p11 */ \
		"STR  rTemp2, [rPort]\n\r" \
		"BEQ endDraw\n\r" \
		"LDR  rTemp2, [pPalette, rTemp]\n\r" \
		/* p12 */ \
		"STRH rTemp2, [rPort,#2]\n\r" \
		"LSR  rTemp,Opixel2, #22\n\r" \
		"AND  rTemp, AndMask\n\r" \
		"CMP rShift, #14\n\r" \
		/* p13 */ \
		"STR  rTemp2,[rPort]\n\r" \
		"BEQ endDraw\n\r" \
		"LDR  rTemp2, [pPalette, rTemp]\n\r" \
		/* p14 */ \
		"STRH rTemp2, [rPort,#2]\n\r" \
		"B endDraw\n\r" \
		\
		"oddShift:\n\r" \
		/* p0 */ \
		"STRH rTemp2, [rPort,#2]\n\r" \
		"BLE endDraw\n\r" \
		"LSR rTemp, Opixel, #6\n\r" \
		"AND rTemp, AndMask\n\r" \
		/* p1 */ \
		"STR rTemp2, [rPort]\n\r" \
		"LDR rTemp2, [pPalette, rTemp]\n\r" \
		"CMP rShift, #3\n\r" \
		/* p2 */ \
		"STRH rTemp2, [rPort,#2]\n\r" \
		"BLE endDraw\n\r" \
		"LSR rTemp, Opixel, #14\n\r" \
		"AND rTemp, AndMask\n\r" \
		/* p3 */ \
		"STR rTemp2, [rPort]\n\r" \
		"LDR rTemp2, [pPalette, rTemp]\n\r" \
		"CMP rShift, #5\n\r" \
		/* p4 */ \
		"STRH rTemp2, [rPort,#2]\n\r" \
		"BLE endDraw\n\r" \
		"LSR rTemp, Opixel, #22\n\r" \
		"AND rTemp, AndMask\n\r" \
		/* p5 */ \
		"STR rTemp2, [rPort]\n\r" \
		"LDR rTemp2, [pPalette, rTemp]\n\r" \
		"CMP rShift, #7\n\r" \
		/* p6 */ \
		"STRH rTemp2, [rPort,#2]\n\r" \
		"BLE endDraw\n\r" \
		"LSL rTemp, Opixel2, #2\n\r" \
		"AND rTemp, AndMask\n\r" \
		/* p7 */ \
		"STR rTemp2, [rPort]\n\r" \
		"LDR rTemp2, [pPalette, rTemp]\n\r" \
		"CMP rShift, #9\n\r" \
		/* p8 */ \
		"STRH rTemp2, [rPort,#2]\n\r" \
		"BLE endDraw\n\r" \
		"LSR rTemp, Opixel2, #6\n\r" \
		"AND rTemp, AndMask\n\r" \
		/* p9 */ \
		"STR rTemp2, [rPort]\n\r" \
		"LDR rTemp2, [pPalette, rTemp]\n\r" \
		"CMP rShift, #11\n\r" \
		/* p10 */ \
		"STRH rTemp2, [rPort,#2]\n\r" \
		"BLE endDraw\n\r" \
		"LSR rTemp, Opixel2, #14\n\r" \
		"AND rTemp, AndMask\n\r" \
		/* p11 */ \
		"STR rTemp2, [rPort]\n\r" \
		"LDR rTemp2, [pPalette, rTemp]\n\r" \
		"CMP rShift, #13\n\r" \
		/* p12 */ \
		"STRH rTemp2, [rPort,#2]\n\r" \
		"BLE endDraw\n\r" \
		"LSR rTemp, Opixel2, #22\n\r" \
		"AND rTemp, AndMask\n\r" \
		/* p13 */ \
		"STR rTemp2, [rPort]\n\r" \
		"LDR rTemp2, [pPalette, rTemp]\n\r" \
		"NOP\n\r" \
		/* p14 */ \
		"STRH rTemp2, [rPort]\n\r" \
		"NOP\n\r" \
		"NOP\n\r" \
		"endDraw:\n\r" \
		"MOV rTemp2, #0\n\r" \
		"STR  rTemp2, [rPort]\n\r" 		
		uint32_t tiles[MAX_TILES][TILE_SIZE_X/8*TILE_SIZE_Y] __attribute__ ((aligned (4)));
	#endif
	// These should be declared and defined in your code. These definitions were removed these from here!
/*	#if USE_SECTION == TOP_FIXED_SECTION
		uint8_t fixedSectionMap[VRAMX * SECTION_LIMIT] __attribute__ ((aligned (4)));
		#if GFX_MODE != TILE_MODE2
		uint32_t fixedSectionTiles[MAX_FIXED_SECTION_TILES][TILE_SIZE_X/4*TILE_SIZE_Y] __attribute__ ((aligned (4)));
		#else
			uint32_t fixedSectionTiles[MAX_FIXED_SECTION_TILES][TILE_SIZE_X/8*TILE_SIZE_Y] __attribute__ ((aligned (4)));
		#endif
	#elif USE_SECTION == BOTTOM_FIXED_SECTION
		uint8_t fixedSectionMap[VRAMX * (VRAMY - SECTION_LIMIT - 1)] __attribute__ ((aligned (4)));
		#if GFX_MODE != TILE_MODE2
		uint32_t fixedSectionTiles[MAX_FIXED_SECTION_TILES][TILE_SIZE_X/4*TILE_SIZE_Y] __attribute__ ((aligned (4)));
		#else
			uint32_t fixedSectionTiles[MAX_FIXED_SECTION_TILES][TILE_SIZE_X/8*TILE_SIZE_Y] __attribute__ ((aligned (4)));
	#endif
	#endif*/
    // private data structures.
	typedef struct  
	{
		uint16_t originalVramIndex;			// where the original tile was (0xFFFF = invalid).		
		uint16_t originalTileLoAddr;		// lower address of the original tile it was pointing to.	
	} spriteTile_t;
	#if SPRITES_ENABLED
	spriteTile_t spriteTiles[MAX_TEMP_SPRITE_TILES] __attribute__ ((aligned (4)));
	#endif
	typedef struct  
	{
		int16_t x;
		int16_t y;
		uint16_t frameNum;		// frame number in rom.
		uint16_t flags;			// if x and y are relative to vram or to the visible screen. And if the sprite should be x or y flipped
	} usvcSprite_t;
	#if SPRITES_ENABLED
	usvcSprite_t usvcSprites[MAX_ONSCREEN_SPRITES];
	#endif
//
#endif
#if GFX_MODE == BITMAPPED_MODE && PER_HORIZONTAL_BLOCK_PALETTE_REMAP == 1
/* setPaletteIndex()
* small utility function to set the palette index on a certain block. 
* y is the row, bx is the 8-byte block number. (bx < 40). index is the palette index we must set for the 8x1 pixel block at (bx,y).
* This is not recommended when speed is required. You might want to access the array directly.
*/
void setPaletteIndex(int bx, int y, uint8_t index)
{
	paletteIndexTable[bx + y * (SCREEN_SIZE_X >> 3) ] = index;
}
/* initPaletteEntries()
 * This copies the palette data to the internal palette array, adding also an offset value. This allows the coexistence of multiple items with different 
 * palettes. 
 * paletteData is the array (usually stored in flash) which we need to copy. 
 * numberOfPaletteEntries is... well no need of explanation.
 * startPalette determines the offset at which the palette data is copied to.
*/ 
void initPaletteEntries(const uint32_t *paletteData, uint32_t numberOfPaletteEntries, uint16_t startPalette)
{
	const uint32_t *s;
	uint32_t *d;
	int n = min(numberOfPaletteEntries, sizeof(palette) / 4 - startPalette);
	s = paletteData;
	d = (uint32_t*) palette + startPalette;
	for (int i = 0; i < n; i++)
		*d++ = *s++;
}
#endif
#if GFX_MODE == TILE_MODE1 || GFX_MODE == TILE_MODE2
inline void copyTileFast(uint32_t *dst, const uint32_t *src)
{
#if GFX_MODE == TILE_MODE1
	asm volatile
	(
		"LDMIA %[src]!,{r4, r5, r6, r7}\n\t"
		"STMIA %[dst]!,{r4, r5, r6, r7}\n\t"
		"LDMIA %[src]!,{r4, r5, r6, r7}\n\t"
		"STMIA %[dst]!,{r4, r5, r6, r7}\n\t"
		"LDMIA %[src]!,{r4, r5, r6, r7}\n\t"
		"STMIA %[dst]!,{r4, r5, r6, r7}\n\t"
		"LDMIA %[src]!,{r4, r5, r6, r7}\n\t"
		"STMIA %[dst]!,{r4, r5, r6, r7}\n\t"
	:  [dst] "+l" (dst), [src] "+l" (src)
	:
	: "r4", "r5", "r6", "r7"
	);
#else
	// for only 32 bytes, it is faster this one, which is translated in 8 loads and 8 stores!
	for (int i = 0; i < 8; i++)
		*dst++ = *src++;
#endif
}
#endif

/* 
*  initVga(): Initializes the vga hardware.
*  Prerequisites: 
*  VideoData structure must be properly initialzed. In particular, these parameters (depending on your configuration) must be initialized:
*		Tile MODE 2 Only:
*		- if ENABLE_PALETTE_REMAPPING videoData.pPaletteIdx
*		- if ENABLE_PER_LINE_COLOR_REMAPPING  &&  ENABLE_PALETTE_REMAPPING videoData.pNewColorChangeTable  and videoData.pNewColorChangeIndexTable 
*		- if ENABLE_PALETTE_ROW_REMAPPING && ENABLE_PER_LINE_COLOR_REMAPPING  &&  ENABLE_PALETTE_REMAPPING: videoData.pPaletteRemappingRowOffsets 
*		Tile MODE 2 and TILE MODE 1:
*		if USE_ROW_REMAPPING videoData.ptrRowRemapTable
*		if PER_LINE_X_SCROLL videoData.ptrRowXScroll
*		if PER_TILE_X_SCROLL videoData.ptrTileXScroll
*	NOTE: The arrays must be properly initialized!
*/
void initVga()
{
	#if GFX_MODE == TILE_MODE_BOOT
		memset(tiles, 512, sizeof(tiles));  // set SDCS = high. This will set also PA25 but it is configured as input on reset, until the control is taken over by the USB.
		videoData.ptrRowRemapTable = rowRemapTable;
	#endif
	#if (GFX_MODE == TILE_MODE1 || GFX_MODE == TILE_MODE2)
		#if SPRITES_ENABLED
			memset (usvcSprites, 0xFF, sizeof(usvcSprites));
			memset (spriteTiles, 0xFF, sizeof(spriteTiles));
		#endif
		for (int i = 0; i < sizeof(vram) / sizeof (vram[0]); i++)
		{
			vram[i] = (uint32_t) &tiles[0];
		}
		// Clear vram, so that each entry points to a known tile.
	#endif	
	// give more priority to the CPU.
	*(( volatile uint32_t *)0x41007110) = 0b11;	
	//
	#if GFX_MODE == BITMAPPED_MODE
		videoData.pPixel = (uint32_t *) pixels;
		videoData.pPort = (uint32_t *) (0x10 + 0x60000000);
		videoData.pPalette = (uint32_t*) palette;
		#if PER_HORIZONTAL_BLOCK_PALETTE_REMAP
			videoData.pPaletteIndexTable = paletteIndexTable;
		#endif
	#elif GFX_MODE == TILE_MODE2
		videoData.pPort = (uint32_t *) (0x10 + 0x60000000);
		videoData.pPalette = (uint32_t*) &palette[0];
		videoData.yScroll = 0;
		#if !(PER_TILE_X_SCROLL && PER_LINE_X_SCROLL)
			videoData.xScroll = 0;
		#endif		
	#elif GFX_MODE == TILE_MODE_BOOT
		videoData.pPort = (uint32_t *) (0x10 + 0x60000000);
		videoData.blackPixel = 0b0001000000000000 | (1 << 9);
		videoData.pVram = vram;
		videoData.pTiles = (uint16_t*) tiles;	
	#elif GFX_MODE == TILE_MODE1
		videoData.pPort =  (uint16_t*)	(0x12 + 0x60000000);
		videoData.yScroll = 0;
		#if !(PER_TILE_X_SCROLL && PER_LINE_X_SCROLL)
			videoData.xScroll = 0;
		#endif
	#endif	
	#if GFX_MODE != BITMAPPED_MODE && GFX_MODE != TILE_MODE_BOOT
		#if SPRITES_ENABLED
			videoData.spriteTilesRemoved = 1;
		#endif
		videoData.ramTiles = 0;
	#endif
	// Set the PORT direction for the sync signals and  for color signals.
	REG_PORT_OUTSET0 = PORT_PA09;
	REG_PORT_DIR0 = PORT_PA04 | PORT_PA08 | 0xC0CF0000 | PORT_PA10;
	setPortMux(10, PORT_PMUX_PMUXE_H); 	// This will override the PA10 output. This is mandatory for 4BPP mode, otherwise, if the color has r0 high, the screen would be black on odd pixels!
	setDisablePinValue(0);	// enable 74AHC245.
	//
	PORT->Group[0].PINCFG[30].reg =  PORT_PINCFG_INEN | 0 * PORT_PINCFG_PMUXEN;  // 0*PMUXEN explicitly written to remind that we disable the mux (SWD function)
	PORT->Group[0].PINCFG[31].reg =  PORT_PINCFG_INEN | 0 * PORT_PINCFG_PMUXEN;	// 0*PMUXEN explicitly written to remind that we disable the mux (SWD function)	
	// initialize horizontal timer
	// first enable APB clocks
	//REG_PM_APBCMASK |= PM_APBCMASK_TCC0 | PM_APBCMASK_TCC1 | PM_APBCMASK_TCC2 | PM_APBCMASK_EVSYS; already done in the initUSVC()
	// EVENT
	REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_EVSYS_0 );
	// Setup event for entering the drawing routine
	REG_EVSYS_CTRL = EVSYS_CTRL_GCLKREQ;
	REG_EVSYS_CHANNEL = EVSYS_CHANNEL_EDGSEL_RISING_EDGE | EVSYS_CHANNEL_PATH_SYNCHRONOUS | EVSYS_CHANNEL_CHANNEL(0) | EVSYS_CHANNEL_EVGEN(EVSYS_ID_GEN_TCC2_MCX_0);
	REG_EVSYS_INTENSET = EVSYS_INTENSET_EVD0;
	NVIC_SetPriority(EVSYS_IRQn, 0);
	NVIC_EnableIRQ( EVSYS_IRQn );
	// enable clocks for timers
	REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC0_TCC1)  ;
	//
	REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TCC2_TC3)  ;
	// Actually the CTRLA will be set once the timer is enabled. We must be sure it is disabled (this might not be the case if we are coming from the bootloader)
	#if !IS_BOOTLOADER
		// These MUST be reset!
		TCC1->CTRLA.reg = TCC_CTRLA_SWRST; //TCC_CTRLA_PRESCALER_DIV1 | TCC_CTRLA_PRESCSYNC_GCLK | TCC_CTRLA_RUNSTDBY;
		TCC2->CTRLA.reg = TCC_CTRLA_SWRST; // TCC_CTRLA_PRESCALER_DIV1 | TCC_CTRLA_PRESCSYNC_GCLK | TCC_CTRLA_RUNSTDBY;
		TCC0->CTRLA.reg = TCC_CTRLA_SWRST; //TCC_CTRLA_PRESCALER_DIV1 | TCC_CTRLA_PRESCSYNC_GCLK | TCC_CTRLA_RUNSTDBY;
	#endif
	//
	TCC1->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;		// Normal PWM
	//
	TCC1->COUNT.reg = 0;
	
	// this sets when the TCC1 handler is entered. Higher values => later. TILE_MODE_BOOT has few precalculations, therefore it can be entered later, leaving time for other things.
	#if GFX_MODE == TILE_MODE_BOOT
			REG_TCC1_CC1 = 80;
	#else
	#ifndef AUDIO_ENABLED
		REG_TCC1_CC1 = HSYNC_PULSE +  HSYNC_BACKPORCH - 1 - 180;		
	#else
		REG_TCC1_CC1 =   1; 	// 7 is the limit
	#endif
	#endif
	TCC1->CC[0].reg = HSYNC_PULSE - 1;	
	TCC1->PER.reg = M_CLK / HORIZONTAL_FREQ - 1;
	// Low pulse
	TCC1->DRVCTRL.reg = TCC_DRVCTRL_INVEN2; 
	//
	TCC1->INTENSET.reg = TCC_INTENSET_MC1;
	//	Set waveform out pin
	setPortMux(8,PORT_PMUX_PMUXE_F);
	
	// Timer 2: event generator 
	TCC2->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;		// Normal PWM
	//
	TCC2->COUNT.reg = 0;
	//
	#if GFX_MODE == TILE_MODE_BOOT
		REG_TCC2_CC0 =  HSYNC_PULSE +  HSYNC_BACKPORCH - 1 - 15 -4;		// this regulates the horizontal position
	#else
		REG_TCC2_CC0 =  HSYNC_PULSE +  HSYNC_BACKPORCH - 1 - 15 - 4 ;
	#endif
	TCC2->PER.reg = M_CLK / HORIZONTAL_FREQ - 1;
	//	Setup event for triggering the counter 2 delay, for actually starting the draw operations
	REG_EVSYS_CHANNEL = EVSYS_CHANNEL_EDGSEL_RISING_EDGE | EVSYS_CHANNEL_PATH_ASYNCHRONOUS | EVSYS_CHANNEL_CHANNEL(2) | EVSYS_CHANNEL_EVGEN(EVSYS_ID_GEN_TCC1_OVF);	
	REG_EVSYS_USER = EVSYS_USER_CHANNEL(2 + 1) | EVSYS_USER_USER(0x0E);	// 0x0E = TCC2 EV0
	// Timer 0 also uses the same event to count 
	REG_EVSYS_USER = EVSYS_USER_CHANNEL(2 + 1) | EVSYS_USER_USER(0x04);	// 0x04 = TCC0 EV0
	//	Now setup timer 0 for creating vertical sync signal.
	//
	TCC0->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;		// Normal PWM
	//
	videoData.currentLineNumber = 0;
	TCC0->PER.reg = 525 - 1;		// 525 horizontal lines
	//
	TCC0->COUNT.reg = TCC0->PER.reg - (480 + 10 - 2) + VERTICAL_SCREEN_POSITION;		// this regulates the vertical position on the screen
	REG_TCC0_CC0 = 2;
	// Low pulse
	TCC0->DRVCTRL.reg = TCC_DRVCTRL_INVEN0; 
	// Set Event input so that on each event TCC0 is incremented
	TCC0->EVCTRL.reg = TCC_EVCTRL_EVACT0_INC | TCC_EVCTRL_TCEI0;
	setPortMux(4,PORT_PMUX_PMUXE_E);
	//
	TCC2->EVCTRL.reg = TCC_EVCTRL_EVACT0_RETRIGGER | TCC_EVCTRL_TCEI0;	
	// enable output events
	TCC2->EVCTRL.reg = TCC_EVCTRL_MCEO0; 
	TCC1->EVCTRL.reg = TCC_EVCTRL_OVFEO; 	
	//
	TCC0->CTRLA.reg =  TCC_CTRLA_PRESCALER_DIV1 | TCC_CTRLA_PRESCSYNC_GCLK | TCC_CTRLA_RUNSTDBY | TCC_CTRLA_ENABLE; 
	TCC1->CTRLA.reg = TCC_CTRLA_PRESCALER_DIV1 | TCC_CTRLA_PRESCSYNC_GCLK | TCC_CTRLA_RUNSTDBY | TCC_CTRLA_ENABLE;
	TCC2->CTRLA.reg =  TCC_CTRLA_PRESCALER_DIV1 | TCC_CTRLA_PRESCSYNC_GCLK | TCC_CTRLA_RUNSTDBY | TCC_CTRLA_ENABLE; 
	//
	#if AUDIO_ENGINE_ON_VBLANK_INTERRUPT && (AUDIO_ENABLED || USE_BL_AUDIO)
		TCC0->INTENSET.reg = TCC_INTENSET_OVF;
		NVIC_SetPriority(TCC0_IRQn, 3);	// lowest pri
		NVIC_EnableIRQ( TCC0_IRQn ) ;
	#endif
	//	
	NVIC_SetPriority(TCC1_IRQn, 1);
	NVIC_EnableIRQ( TCC1_IRQn ) ;
}
// If using the NVM_MODULE, the EVSYS_Handler must be stored in RAM, otherwise glitches will occur!
#if GFX_MODE == TILE_MODE_BOOT || USE_NVM_MODULE
RAMFUNC void EVSYS_Handler(void)
#else
void EVSYS_Handler(void)
#endif
{
	// do nothing it's just for the first line ever
	REG_EVSYS_INTFLAG |= EVSYS_INTFLAG_EVD0;
}
#if AUDIO_ENGINE_ON_VBLANK_INTERRUPT && (AUDIO_ENABLED || USE_BL_AUDIO)
void TCC0_Handler(void)
{
	TCC0->INTFLAG.reg = TCC_INTFLAG_OVF;
	#if AUDIO_ENABLED
		soundEngine();
	#elif USE_BL_AUDIO
		bootloaderSoundEngine();
	#endif
}
#endif
// All the magic happens here!
RAMFUNC void TCC1_Handler (void)
{
	asm ("cpsid i\n\t");
#if USE_MIXER == 1
	// this will call the bootloader or standard audio mixer.
	audioMixer();
#endif
	*(( (uint8_t*) &TCC1->INTFLAG.reg) + 2)  = 2; //   This line will translate into only two asm instructions. Using TCC1->INTFLAG.reg = TCC_INTFLAG_MC1; will add another one.
	uint32_t lineNumber = videoData.currentLineNumber;
	if (lineNumber < (uint16_t) MAX_Y_LINES )
	{
	#if (GFX_MODE == BITMAPPED_MODE)	
	#if 1  // just put to be able to fold/unfold
		asm volatile 
		(

			/*
				Register usage in the main loop:
				r0 = rMask = 0x03C AND mask 
				r1 = rTemp (temporary register)
				r2 = rTemp2 (second temporary register)
				r3 = rTemp3 (third temporary register)
				r4 = r16pixel palettized pixels of the current 16-pixel block.
				r5 = rPalette (address value of the current palette)
				r6 = rPort (address of the 32-bit port)
				r7 = rPaletteTableIndex (address of next palette index in the table)
				r8 = rStackPtrBackup stack pointer backup
				r9 = rPaletteAddrBase (address of the first palette)
			*/
			//	Prologue. Here we can take our time until the first STRH.
			"rMask .req r0\n\t"
			"rTemp .req r1\n\t"
			"rTemp2 .req r2\n\t"
			"rTemp3 .req r3\n\t"
			"r16pixel .req r4\n\t"
			"rPalette .req r5\n\t"
			"rPort .req r6\n\t"
			"rPaletteIndexTable .req r7\n\t"
			"rStackPtrBackup .req r8\n\t"
			"rPaletteAddrBase .req r9\n\t"
			"rLoopCount .req r10\n\t"
			// First backup the stack pointer 
			"MOV rStackPtrBackup, SP\n\t"
			// Then load all the useful data from the videoData in the structure					
			"LDR rTemp,videoDataLabel\n\t"
			// load the current line number
			"LDR rTemp3, [rTemp, %[currentLineNumberOffset]]\n\t" 
			// In rTemp3: current LineNumber
			"LSR rTemp3, #1\n\t"
			"MOV rTemp2, %[BytesPerLine]\n\t"			
			"MUL rTemp2,rTemp3\n\t"
			#if PER_HORIZONTAL_BLOCK_PALETTE_REMAP 
				"ADD rTemp,%[pPixelOffset]\n\t"
				"LDMIA rTemp!, {r16pixel, rPalette, rPort, rPaletteIndexTable}\n\t"
				// move the palette base address to the correct register
				"MOV rPaletteAddrBase,rPalette\n\t"
			#else
				"ADD rTemp,%[pPixelOffset]\n\t"
				"LDMIA rTemp!, {r16pixel, rPalette, rPort}\n\t"
			#endif
				// point to the correct position
			"ADD r16pixel,rTemp2\n\t"
			#if PER_HORIZONTAL_BLOCK_PALETTE_REMAP
				"MOV rTemp2, %[INDEX_TABLE_COLUMNS]\n\t"
				"MUL rTemp2, rTemp3\n\t"
				"ADD rPaletteIndexTable, rTemp2\n\t"
			#endif
			"MOV SP, r16pixel\n\t"		// put into stack the data buffer position!
			"MOV rMask,  #0x3C\n\t"				// save the 4-bit mask which will be used as AND operand, to get the 2-pixels palette.
			// Now let's set the counter. We want to use high-register only. These can only support non-immediate ADD and CMP
			// However the only constant we have is the AND rMask (0x3C). We do not want to waste another register for constant, therefore we
			// count in terms of 0x3C increments, and we always compare the resutl to 0x3C.
			// Therefore, if we put a 0, we will have just one iteration. If we put -0x3C we will have two... To get 20 iterations we need -19*0x3C
			"LDR rTemp, LineIterationCount\n\t"			// we need 320 pix. = 20 16-pixel iterations.
			"MOV rLoopCount, rTemp\n\t"
			"B startDraw\n\t"
			".align(2)\n\t"
			"videoDataLabel:\n\t"
			".word videoData\n\t"
			"LineIterationCount:\n\t"
			".word (-19*0x3C)\n\t"
			"startDraw:\n\t"
			// now with patience we can fill the preliminary data...
			#if PER_HORIZONTAL_BLOCK_PALETTE_REMAP 
				"LDRB rPalette, [rPaletteIndexTable]\n\t"
				"LSL rPalette, #4\n\t"
				"ADD rPalette, rPaletteAddrBase\n\t"
			#endif
			"POP {r16pixel}\n\t"
			"LSL rTemp,r16pixel,#2\n\t"
			"AND rTemp,rMask\n\t"
			"LDR rTemp, [rPalette, rTemp]\n\t"
			"WFI\n\t"
			"vgaDrawLoop:\n\t"
			"STRH rTemp,[rPort,#2]\n\t"
			"LSR rTemp2,r16pixel,#2\n\t" // Second pixel couple
			"AND rTemp2,rMask\n\t"
			"LSR rTemp3, r16pixel, #14\n\t"
			"STR rTemp, [rPort]\n\t"
			"LDR rTemp2, [rPalette, rTemp2 ]\n\t"
			"LSR rTemp,r16pixel,#6\n\t"
			"STRH rTemp2,[rPort,#2]\n\t"
			"AND rTemp, rMask\n\t"
			"LDR rTemp, [rPalette, rTemp]\n\t"
			"STR rTemp2, [rPort]\n\t"
			"LSR rTemp2, r16pixel, #10\n\t"
			"AND rTemp2, rMask\n\t"
			"AND rTemp3, rMask\n\t"
			"STRH rTemp,[rPort,#2]\n\t"
			"LDR rTemp2, [rPalette, rTemp2]\n\t"
			"NOP\n\t"			
			"STR rTemp, [rPort]\n\t"
#if PER_HORIZONTAL_BLOCK_PALETTE_REMAP
			"LDRB rPalette, [rPaletteIndexTable, #1]\n\t"
			"LSL rPalette, #4\n\t"
#else
			"NOP\n\t"
			"NOP\n\t"
			"NOP\n\t"
#endif
			"STRH rTemp2,[rPort,#2]\n\t"
#if PER_HORIZONTAL_BLOCK_PALETTE_REMAP
			"ADD rPalette, rPaletteAddrBase\n\t"
#else
			"NOP\n\t"
#endif
			"LDR rTemp3, [rPalette, rTemp3]\n\t"	// first two pixels of the next 8-pixel block (p8-9)
			"STR rTemp2, [rPort]\n\t"
			"LSR rTemp, r16pixel, #18\n\t"
			"AND rTemp, rMask\n\t"
			"LSR rTemp2,r16pixel,#22\n\t"
			//--- second 8-pixel block
			"STRH rTemp3,[rPort,#2]\n\t"
			"LDR rTemp, [rPalette, rTemp]\n\t"			// pix (p10-11)
			"AND rTemp2,rMask\n\t"
			"STR rTemp3, [rPort]\n\t"
			"LDR rTemp2, [rPalette, rTemp2 ]\n\t"	// pix (p12-13)
			"LSR rTemp3,r16pixel, #26\n\t"			// pix 14-15
			"STRH rTemp,[rPort,#2]\n\t"
			"AND rTemp3, rMask\n\t"
			"LDR rTemp3, [rPalette, rTemp3]\n\t"
			"STR rTemp, [rPort]\n\t"
			"POP {r16pixel}\n\t"
#if PER_HORIZONTAL_BLOCK_PALETTE_REMAP
			"ADD rPaletteIndexTable, #2\n\t"
#else
			"NOP\n\t"
#endif
			"STRH rTemp2,[rPort,#2]\n\t"
#if PER_HORIZONTAL_BLOCK_PALETTE_REMAP
			"LDRB rPalette, [rPaletteIndexTable]\n\t"
			"LSL rPalette, #4\n\t"
#else
			"NOP\n\t"
			"NOP\n\t"
			"NOP\n\t"
#endif
			"STR rTemp2, [rPort]\n\t"
#if PER_HORIZONTAL_BLOCK_PALETTE_REMAP
			"ADD rPalette, rPaletteAddrBase\n\t"
#else
			"NOP\n\t"
#endif
			"LSL rTemp,r16pixel,#2\n\t"
			"AND rTemp,rMask\n\t"
			"STRH rTemp3,[rPort,#2]\n\t"
			"LDR rTemp, [rPalette, rTemp]\n\t"
			"ADD rLoopCount, rMask\n\t"
			"STR rTemp3, [rPort]\n\t"
			"CMP rMask,rLoopCount\n\t"
			"BNE vgaDrawLoop\n\t"
			// put a black pixel
			"EOR rMask,rMask\n\t"
			"STRH rMask, [rPort,#2]\n\t"
			// restore the correct stack pointer.
			"MOV SP, rStackPtrBackup\n\t"			 
			:
			: 			
			 	[pPixelOffset] "I" ((uint8_t) offsetof(videoData_t, pPixel)),			
			 	[currentLineNumberOffset] "I" ((uint8_t) offsetof(videoData_t, currentLineNumber)),
			 	[BytesPerLine] "I" ((uint8_t) BYTES_PER_LINE),
				[INDEX_TABLE_COLUMNS] "I" ((uint8_t) (SCREEN_SIZE_X /8))
			: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10"			
		);		
#endif
	#elif (GFX_MODE == TILE_MODE1)
		asm volatile
		(
			// Registers used only in setup
			"r0_videodata_ptr .req r0\n\t"
			"r1_rOffset .req r1\n\t"		// offset in a low register
			// registers used during signal generation
			"r0_rvram_first20Tiles_ptr .req r0\n\t"
			"r1_rvram_second20Tiles_ptr .req r1\n\t"			// we have two pointers because the maximum offset is 62 for word loads...
			"r2_mult_c .req r2\n\t"							// multiplicative constant to achieve the video signals
			"r3_port .req r3\n\t"
			"r4_temp0 .req r4\n\t"
			"r5_temp1 .req r5\n\t"							// generally used to store the first quad pixel.
			"r6_temp2 .req r6\n\t"							// generally used to store the second quad pixel.
			"r7_temp3 .req r7\n\t"
			"r8_x_scroll .req r8\n\t"						// x and y scroll values. Here we store also the last partial qPixel values.
			"r9_y_scroll .req r9\n\t"
			"r10_offset .req r10\n\t"  // same as offset but in r9, since luckily we can add high registers 
			// prologue
			".align(2)\n\t"	
			//
			// calculate the correct offsets
			// get the y offset on the tile
			"LDR r0_videodata_ptr,videoDataAddr\n\t"					// get the videoData structure address
			"LDR r3_port, [r0_videodata_ptr, %[pPortOffset]]\n\t"		// get the fast port address
			"LDR r5_temp1, [r0_videodata_ptr, %[currentLineNumberOffset]]\n\t"
			"LSR r5_temp1, #1\n\t"							// divide the currentLine number by 2, so that we have the low res.
			"LDRB r4_temp0, [r0_videodata_ptr, %[yScrollOffset]]\n\t"	// get Y scroll
			"MOV r9_y_scroll, r4_temp0\n\t"
			// Load xScroll of this line
			#if PER_LINE_X_SCROLL
				"LDR r4_temp0,[r0_videodata_ptr, %[ptrRowXScrollOffset]]\n\t"		// get the pointer to the row-remapping array.
				"LDRB r4_temp0,[r4_temp0, r5_temp1]\n\t"				// load the per row offset
				"MOV r8_x_scroll, r4_temp0\n\t"
				#if PER_TILE_X_SCROLL == 0
					// Add the global xScroll if the PER TILE XSCROLL feature is disabled.
					"LDRB r4_temp0, [r0_videodata_ptr, %[xScrollOffset]]\n\t"
					"ADD r8_x_scroll, r4_temp0\n\t"
				#endif
			#endif
			// remap Rows.
			#if USE_ROW_REMAPPING
				"LDR r4_temp0,[r0_videodata_ptr, %[rowRemapPtrOffset]]\n\t"		// get the pointer to the row-remapping array.
				"LDRB r5_temp1,[r4_temp0, r5_temp1]\n\t"				// load the correct remapped row
			#endif
			#if PER_TILE_X_SCROLL
				// scroll based on the remapped row.
				"MOV r6_temp2, r5_temp1\n\t"
				"ADD r6_temp2, r9_y_scroll\n\t"
				"LSR r6_temp2, r6_temp2, #3\n\t"						// get the tile row number
				"LDR r4_temp0,[r0_videodata_ptr, %[ptrTileXScrollOffset]]\n\t"		// get the pointer to the row-remapping array.
				"LDRB r4_temp0,[r4_temp0, r6_temp2]\n\t"				// load the per tile offset
				#if PER_LINE_X_SCROLL == 0
					"MOV r8_x_scroll, r4_temp0\n\t"
					// Add the global xScroll if the PER TILE XSCROLL feature is disabled.
					"LDRB r4_temp0, [r0_videodata_ptr, %[xScrollOffset]]\n\t"
					"ADD r8_x_scroll, r4_temp0\n\t"
				#else
					"ADD r8_x_scroll, r4_temp0\n\t"
				#endif
			#endif
			#if PER_TILE_X_SCROLL == 0 && PER_LINE_X_SCROLL == 0
				"LDRB r4_temp0, [r0_videodata_ptr, %[xScrollOffset]]\n\t"
				"MOV r8_x_scroll, r4_temp0\n\t"
			#endif
			#if USE_SECTION == TOP_FIXED_SECTION
				"CMP r5_temp1,#" LINELIMITSTR "\n\t"
				"BGT afterSection\n\t"
				"MOV r4_temp0, #0\n\t"
				"MOV r9_y_scroll,r4_temp0\n\t"
				"MOV r8_x_scroll,r4_temp0\n\t"
				"afterSection:"
				"ADD r5_temp1,r9_y_scroll\n\t"
			#elif USE_SECTION == BOTTOM_FIXED_SECTION
				"CMP r5_temp1,#" LINELIMITSTR "\n\t"
				"BLT afterSection\n\t"
				"MOV r4_temp0, #8\n\t"
				"MOV r9_y_scroll,r4_temp0\n\t"
				"MOV r4_temp0, #0\n\t"
				"MOV r8_x_scroll,r4_temp0\n\t"
				"afterSection:"
				"ADD r5_temp1,r9_y_scroll\n\t"	
			#elif USE_SECTION == NO_FIXED_SECTION
					"ADD r5_temp1,r9_y_scroll\n\t"
			#else
				#error "You must define USE_SECTION either as NO_FIXED_SECTION or TOP_FIXED_SECTION or BOTTOM_FIXED_SECTION! OR DEFINE USE_ROW_REMAPPING AS 1"
			#endif			
			// note: from now on, r9_y_scroll is free and can be used as a general purpose register.
			
			"MOV r1_rOffset,#0x07\n\t"
			"AND r1_rOffset,r5_temp1\n\t"		// now in rOffset we have (currentLineNumber >> 1) & 0x7, i.e the y position on the tile
			"LSL r1_rOffset,r1_rOffset,#3\n\t"		// Multiply by 8 the current y position in the tile. This is the displacement to point to the correct row in the tile.
			"MOV r4_temp0, #0x20\n\t"
			"LSL r4_temp0, r4_temp0,#24\n\t"		    // add the ram starting address value 0x20000000
			"ADD r1_rOffset,r4_temp0\n\t"
			//
			// now let's compute the the  address we should point to in the vram
			//
			"LSR r5_temp1,#3\n\t"				// divide the current line number by 8. This will get the y position on vram.
			//
			"MOV r0_rvram_first20Tiles_ptr,%[vramx] * 2\n\t"			// get the number of bytes per row
			"MUL r0_rvram_first20Tiles_ptr, r5_temp1\n\t"		// multiply the row number by the number of bytes per row
			"ADD r0_rvram_first20Tiles_ptr, %[vram]\n\t"
			// r0_rvram_first20Tiles_ptr must now be adjusted by how many complete tiles xScroll is horizontally scrolling the screen.
			"MOV r7_temp3, r8_x_scroll\n\t"  // get the scroll amount
			"LSR r7_temp3, #3\n\t"		// divide by 8
			"LSL r7_temp3, #1\n\t"		// multiply by 2 (each tile index is a two bytes entry)
			"ADD r0_rvram_first20Tiles_ptr, r7_temp3\n\t"	// add the offset to r0_rvram_first20Tiles_ptr
			//
			// now get the last tile address of the row (= first position + 2 * (numberOfHorizontalTilesPerScreen)] 
			"MOV r5_temp1,%[numberOfHorizontalTilesPerScreen]*2\n\t"			// get  2 * (numberOfHorizontalTilesPerScreen)
			"ADD r5_temp1, r0_rvram_first20Tiles_ptr\n\t"			// now in r5_temp1 we have the address of the last tile.
			// scroll: based on the scroll value, we must calculate where we should jump.
			"ADR r4_temp0,scrollTable\n\t"	// get jump table address
			"MOV r7_temp3, r8_x_scroll\n\t"  // get the scroll amount again
			"LSL r7_temp3,#(32-3)\n\t"	 // shift all the way left, so that only the 3 LSB will remain as MSB
			"LSR r7_temp3,#(32 -5)\n\t"	 // shift back right, but two position less. In this way we achieved r7_temp3 = (r8_x_scroll & 7)*4		
			// 2020/05/08: Warning! Do not add other flags modifying instructions before the next conditional branch! By the way LSR is actually
			// coded (due to the ASM divided syntax, see the assembler note on top of this file) as a LSRS (LSR does not even exist on CM0!)
			"LDR r4_temp0,[r4_temp0, r7_temp3]\n\t"		// get the correct vector				
			// now get the multiplication constant
			"LDR r2_mult_c, maskValue\n\t"
			// before we finally jump, we need to get the values of the last pixels
			// Fix 2020/05/08: however we must not do this when the offset is 0. (this to prevent accessing undefined addresses when we are showing the very last pixel)
			"BEQ doNotPreloadLastPixels\n\t"
			// endfix
			"LDRH r5_temp1,[r5_temp1]\n\t"
			"LDR r7_temp3, [r5_temp1, r1_rOffset]\n\t"
			"MOV r8_x_scroll,r7_temp3\n\t"
			"ADD r5_temp1, #4\n\t" // point to next 4 Qpixels

			"LDR r7_temp3,[r5_temp1, r1_rOffset]\n\t"
			"MOV r9_y_scroll,r7_temp3\n\t"
			"doNotPreloadLastPixels:\n\t"
			// now that r1_rOffset has been used, we can copy it to r10_offset and use r1_rvram_second20Tiles_ptr
			"MOV r10_offset, r1_rOffset\n\t"				// copy the offset register to its high-register counterpart.
			"MOV r1_rvram_second20Tiles_ptr, r0_rvram_first20Tiles_ptr\n\t"
			"ADD r1_rvram_second20Tiles_ptr, #40\n\t"
			// now need to load the first tile address and the first 8 pixels.
			"LDRH r5_temp1,[r0_rvram_first20Tiles_ptr,#0]\n\t"			// get the address of next tile
			"ADD r5_temp1, r10_offset\n\t" 
			"LDM r5_temp1, {r5_temp1, r6_temp2}\n\t"            /* this takes 3 cycles!*/ 
			//
			"BX r4_temp0\n\t"
			//
			".align(2)\n\t"
			"maskValue:"
			".word 0x401\n\t"
			"videoDataAddr:\n\t"
			".word videoData\n\t"

			"scrollTable:\n\t"
			".word scroll0 + 1\n\t"
			".word scroll1 + 1\n\t"
			".word scroll2 + 1\n\t"
			".word scroll3 + 1\n\t"
			".word scroll4 + 1\n\t"
			".word scroll5 + 1\n\t"
			".word scroll6 + 1\n\t"
			".word scroll7 + 1\n\t"
			// for each x scroll value, we need to do some of the work we will be skipping. We put in r8_x_scroll and r9_y_scroll the qPixels of the last partial tile 
			"scroll0:\n\t"
				// clear both r9_y_scroll and r8_x_scroll
				"MOV r7_temp3,#0\n\t"
				"MOV r9_y_scroll,r7_temp3\n\t"
				"MOV r8_x_scroll,r7_temp3\n\t"
				//
				"WFI\n\t"    // Wait for hsync
				"B scroll0_tile0_start\r\n"
			"scroll1:\n\t"
				// clear r9_y_scroll
				"MOV r7_temp3,#0\n\t"
				"MOV r9_y_scroll,r7_temp3\n\t"
				// mask all but the first pixel on r8_x_scroll
				"MOV r7_temp3, r8_x_scroll\n\t"
				"UXTB r7_temp3, r7_temp3\n\t"
				"MOV r8_x_scroll,r7_temp3\n\t"
				//
				// prepare register for the first visible pixels of tile 0
				"REV16 r4_temp0,r5_temp1\n\t"			// reverse bytes in halfwords, so that the 2nd and 3rd pixel bytes will be in "easy to separate" positions.
				"WFI\n\t"    // Wait for hsync
				"B scroll1_tile0_start\r\n"
				//
			"scroll2:\n\t"
				// clear r9_y_scroll
				"MOV r7_temp3,#0\n\t"
				"MOV r9_y_scroll,r7_temp3\n\t"
				// mask all but the first 2 pixels on xScroll
				"MOV r7_temp3, r8_x_scroll\n\t"
				"UXTH r7_temp3, r7_temp3\n\t"
				"MOV r8_x_scroll,r7_temp3\n\t"
				//
				// prepare register for the first visible pixels of tile 0
				"REV16 r4_temp0,r5_temp1\n\t"			// reverse bytes in halfwords, so that the 2nd and 3rd pixel bytes will be in "easy to separate" positions.
				"LSR r4_temp0, r4_temp0,#24\n\t"			
				"WFI\n\t"    // Wait for hsync
				"B scroll2_tile0_start\r\n"
				//
			"scroll3:\n\t"
				"MOV r7_temp3,#0\n\t"
				"MOV r9_y_scroll,r7_temp3\n\t"
				// clear the last pixel on xScroll
				"MOV r7_temp3, r8_x_scroll\n\t"
				"LSL r7_temp3,#8\n\t"
				"LSR r7_temp3,#8\n\t"
				"MOV r8_x_scroll,r7_temp3\n\t"
				//

				// prepare register for the first visible pixels of tile 0
				"LSR r5_temp1, r5_temp1, #24\n\t"		// get the byte of the last pixel
				"MUL r5_temp1, r2_mult_c\n\t"			// multiplication for pixel 3
				"WFI\n\t"    // Wait for hsync
				"B scroll3_tile0_start\r\n"
				//
			"scroll4:\n\t"
				// xScroll unchanged, but r9_y_scroll cleared
				"MOV r7_temp3,#0\n\t"
				"MOV r9_y_scroll,r7_temp3\n\t"
				// prepare register for the first visible pixels of tile 0
				"UXTB r7_temp3, r6_temp2\n\t"
				"MUL r7_temp3,  r2_mult_c\n\t" 
				"REV16 r4_temp0, r6_temp2\n\t"
				"WFI\n\t"    // Wait for hsync
				"B scroll4_tile0_start\r\n"
				//
			"scroll5:\n\t"
				// xScroll unchanged, and r9_y_scroll retains only the LSB
				"MOV r7_temp3,r9_y_scroll\n\t"
				"UXTB r7_temp3, r7_temp3\n\t"
				"MOV r9_y_scroll,r7_temp3\n\t"
				// prepare register for the first visible pixels of tile 0
				"REV16 r4_temp0, r6_temp2\n\t"
				"UXTB r5_temp1, r4_temp0\n\t"
				"MUL r5_temp1, r2_mult_c\n\t"   
				"LSR r4_temp0, r4_temp0,#24\n\t"
				"WFI\n\t"    // Wait for hsync
				"B scroll5_tile0_start\r\n"
				//
			"scroll6:\n\t"
				// xScroll unchanged, and r9_y_scroll retains only the LS Hword
				"MOV r7_temp3,r9_y_scroll\n\t"
				"UXTH r7_temp3, r7_temp3\n\t"
				"MOV r9_y_scroll,r7_temp3\n\t"
				// prepare register for the first visible pixels of tile 0
				"REV16 r4_temp0, r6_temp2\n\t"	 		/* reverse bytes in halfwords of the second quad pixels.*/
				"LSR r4_temp0, r4_temp0,#24\n\t"		       /* we have time for one more instruction. Let's extract pixel 2, which is the MSB of r6_temp2.*/ 
				"MUL r4_temp0,  r2_mult_c\n\t" 		        /* convert the byte to the signals for pixel 2.*/ 
				"LSR r7_temp3, r6_temp2, #24\n\t"		 /* get the byte of the last pixel*/ 
				"MUL r7_temp3, r2_mult_c\n\t"			 /* multiplication for pixel 3*/ 
				"WFI\n\t"    // Wait for hsync
				"B scroll6_tile0_start\r\n"
				//
			"scroll7:\n\t"
				// r8_x_scroll unchanged. r9_y_scroll can still be unchanged as it won't be displayed at all  (maximum scroll size is 7) :)
				// prepare register for the first visible pixels of tile 0
				"LSR r7_temp3, r6_temp2, #24\n\t"		 /* get the byte of the last pixel*/
				"MUL r7_temp3, r2_mult_c\n\t"			 /* multiplication for pixel 3*/
				"LDRH r5_temp1,[r0_rvram_first20Tiles_ptr,#2]\n\t"			/* get the address of next tile.*/ \
				"ADD r5_temp1, r10_offset\n\t"
				"WFI\n\t"    // Wait for hsync
				"B scroll7_tile0_start\r\n"
			#if 1
			".align(2)\n\t"
			DRAW_TILE(0, "r0_rvram_first20Tiles_ptr", 2)
			DRAW_TILE(1, "r0_rvram_first20Tiles_ptr", 4)
			DRAW_TILE(2, "r0_rvram_first20Tiles_ptr", 6)
			DRAW_TILE(3, "r0_rvram_first20Tiles_ptr", 8)
			DRAW_TILE(4, "r0_rvram_first20Tiles_ptr", 10)
			DRAW_TILE(5, "r0_rvram_first20Tiles_ptr", 12)
			DRAW_TILE(6, "r0_rvram_first20Tiles_ptr", 14)
			DRAW_TILE(7, "r0_rvram_first20Tiles_ptr", 16)
			DRAW_TILE(8, "r0_rvram_first20Tiles_ptr", 18)
			DRAW_TILE(9, "r0_rvram_first20Tiles_ptr", 20)
			DRAW_TILE(10, "r0_rvram_first20Tiles_ptr", 22)
			DRAW_TILE(11, "r0_rvram_first20Tiles_ptr", 24)
			DRAW_TILE(12, "r0_rvram_first20Tiles_ptr", 26)
			DRAW_TILE(13, "r0_rvram_first20Tiles_ptr", 28)
			DRAW_TILE(14, "r0_rvram_first20Tiles_ptr", 30)
			DRAW_TILE(15, "r0_rvram_first20Tiles_ptr", 32)
			DRAW_TILE(16, "r0_rvram_first20Tiles_ptr", 34)
			DRAW_TILE(17, "r0_rvram_first20Tiles_ptr", 36)
			DRAW_TILE(18, "r0_rvram_first20Tiles_ptr", 38)
			DRAW_TILE(19, "r0_rvram_first20Tiles_ptr", 40)
			DRAW_TILE(20, "r1_rvram_second20Tiles_ptr", 2)
			DRAW_TILE(21, "r1_rvram_second20Tiles_ptr", 4)
			DRAW_TILE(22, "r1_rvram_second20Tiles_ptr", 6)
			DRAW_TILE(23, "r1_rvram_second20Tiles_ptr", 8)
			DRAW_TILE(24, "r1_rvram_second20Tiles_ptr", 10)
			DRAW_TILE(25, "r1_rvram_second20Tiles_ptr", 12)
			DRAW_TILE(26, "r1_rvram_second20Tiles_ptr", 14)
			DRAW_TILE(27, "r1_rvram_second20Tiles_ptr", 16)
			DRAW_TILE(28, "r1_rvram_second20Tiles_ptr", 18)
			DRAW_TILE(29, "r1_rvram_second20Tiles_ptr", 20)
			DRAW_TILE(30, "r1_rvram_second20Tiles_ptr", 22)
			DRAW_TILE(31, "r1_rvram_second20Tiles_ptr", 24)
			DRAW_TILE(32, "r1_rvram_second20Tiles_ptr", 26)
			DRAW_TILE(33, "r1_rvram_second20Tiles_ptr", 28)
			DRAW_TILE(34, "r1_rvram_second20Tiles_ptr", 30)
			DRAW_TILE(35, "r1_rvram_second20Tiles_ptr", 32)
			DRAW_TILE(36, "r1_rvram_second20Tiles_ptr", 34)
			DRAW_TILE(37, "r1_rvram_second20Tiles_ptr", 36)
			DRAW_TILE(38, "r1_rvram_second20Tiles_ptr", 38)
			DRAW_LAST_COMPLETE_TILE(39, "r1_rvram_second20Tiles_ptr", 40)		// this is different as we load here the last pixel		
			// The last tile is different, and we write it here explicitly.
			// pix 0
			"UXTB r7_temp3, r5_temp1\n\t"	/* get the first byte by zero extending the least significant byte of r5_temp1*/ 
			"MUL r7_temp3,  r2_mult_c\n\t" 		 /* get pixel signals by multiplying them for the magic constant :) */  
			"REV16 r4_temp0,r5_temp1\n\t"			/* reverse bytes in halfwords, so that the 2nd and 3rd pixel bytes will be in "easy to separate" positions.*/ 
			"STRH r7_temp3,[r3_port]\n\t"		/* write pixel #0 signals to the port.*/ 
			// pix 1
			"UXTB r7_temp3, r4_temp0\n\t"			/* Get the byte of the second pixel (pixel #1) */ 
			"MUL r7_temp3, r2_mult_c\n\t"      	/* multiply it. Now in r7_temp3 we have the second pixel (pixel #1)*/	
			"LSR r4_temp0, r4_temp0,#24\n\t"		 /* we have time for one more instruction. Let's extract pixel #2, which is the MSB of r5_temp1, which now is the last byte of rTemp*/ 
			"STRH r7_temp3,[r3_port]\n\t"		/* outut pixel #1*/ 
			// pix 2
			"MUL r4_temp0,  r2_mult_c\n\t" 		 /* convert the byte value to the signals for pixel #2. */ 
			/* now r5_temp1 does not have to hold the entire qPixels, we can use it at wish!*/ 
			"LSR r5_temp1, r5_temp1, #24\n\t"		/* get the byte of the last pixel of the quad pixel*/
			"MUL r5_temp1, r2_mult_c\n\t"			/* multiplication for pixel #3 */ 
			"STRH r4_temp0,[r3_port]\n\t"		/* write third pixel (pixel #2) */ 
			// pix 3
			"UXTB r7_temp3, r6_temp2\n\t"			/* get the first byte of r6_temp2 (pixel 4) by zero extending the least significant byte */ 
			"MUL r7_temp3,  r2_mult_c\n\t" 	 	/* get pixel signals by multiplying them for the magic constant :)*/ 
			"REV16 r4_temp0, r6_temp2\n\t"	 		/* reverse bytes in halfwords of the second quad pixels.*/
			"STRH r5_temp1,[r3_port]\n\t"   /* write fourth pixel (pixel #3) */
			// pix 4
			"UXTB r5_temp1, r4_temp0\n\t"		       /* Get the byte of the second pixel of Qpixel 2 (pixel #5), which is the first of r4_temp0 */ 
			"MUL r5_temp1, r2_mult_c\n\t"      	       /* multiply it. Now in r5_temp1 we have the sixth pixel (pixel #5)*/ 
			"LSR r6_temp2, r4_temp0,#24\n\t"		       /* we have time for one more instruction. Let's extract pixel #6, which was the MSB of r6_temp2.*/
			"STRH r7_temp3,[r3_port]\n\t"	           /* write  pixel #4 signals to the port.*/ 
			// pix 5		
			// let's try and skip black pixels.
			"ADD r7_temp3,r6_temp2, r5_temp1\n\t"		// (note: this is compiled as an ADDS, but writing ADDS won't let you compile... eh gcc...) longshot! are pixel 6 and 5 both black? Jump to pixel 7 which is always black, and save some cycles!
			"BEQ blackpixel\n\t"				   //
			// what a pity, there was a non black pixel, either rQpixel 2 or r5_temp1. Well, try again! 
			"LSR r4_temp0, r4_temp0,#24\n\t"		       /* we have time for one more instruction. Let's extract pixel #6, which was the MSB of r6_temp2.*/			
			"STRH r5_temp1,[r3_port]\n\t"		/* outut pixel #5*/ 
			// pix 6
			"MUL r4_temp0,  r2_mult_c\n\t" 		        /* convert the byte to the signals for pixel #6.*/
			"BEQ blackpixel\n\t"				// note: if r4_temp0*r2_mult_c = 0, then r4_temp0 = 0, therfore r6_temp2 is 0 too!
			"MOV r6_temp2,#0\n\t"				// if it wasn't then now it will be!
			"STRH r4_temp0,[r3_port]\n\t"		          /* write pixel 6*/ 
			// pix 7
			"NOP\n\t"  
			"NOP\n\t"
			"NOP\n\t"
			"blackpixel:\n\t"			
			"STRH r6_temp2,[r3_port]\n\t"                     /* write pixel 7*/ 
			#else
				"WFI\n\t"
			#endif
			:
				: 	[vram] "h" (&vram[0]),
			#if USE_ROW_REMAPPING
				 [rowRemapPtrOffset] "I" ((uint8_t) offsetof(videoData_t, ptrRowRemapTable)), 
			#endif
				[vramx] "I" ( (uint8_t) VRAMX),
				[numberOfHorizontalTilesPerScreen] "I" ((uint8_t) (SCREEN_SIZE_X / TILE_SIZE_X)),				
				[pPortOffset] "I" ((uint8_t) offsetof(videoData_t, pPort)),
				[currentLineNumberOffset] "I" ((uint8_t) offsetof(videoData_t, currentLineNumber)), 
				[yScrollOffset] "I"  ((uint8_t) offsetof(videoData_t, yScroll)) 
			#if PER_LINE_X_SCROLL
				,[ptrRowXScrollOffset] "I" ((uint8_t) offsetof(videoData_t, ptrRowXScroll))
			#endif
			#if PER_TILE_X_SCROLL
				,[ptrTileXScrollOffset] "I" ((uint8_t) offsetof(videoData_t, ptrTileXScroll))		
			#endif
			#if PER_TILE_X_SCROLL == 0 || PER_LINE_X_SCROLL == 0
				,[xScrollOffset] "I" ((uint8_t) offsetof(videoData_t, xScroll))		 			 
			#endif
				 //, [currentLineNumber] "h" (lineNumber >> 1) , [xScroll] "h" (g_xScroll ), [yScroll] "h" (g_yScroll)
				: "r0", "r1", "r2", "r3","r4", "r5", "r6", "r7", "r8", "r9", "r10"
			);
		#elif GFX_MODE == TILE_MODE2
			asm volatile
			(
			// fixed registers		
			"rPort .req r0\n\t"
			"Offset .req r1\n\t"
			"pPalette .req r2\n\t"
			// registers changed but restored too
			"AndMask .req r3\n\t"
			// temporary registers: mostly used for their function name, but also used as temp
			"Opixel .req r4\n\t"
			"tileAddresses .req r5\n\t"
			"bPixel .req r6\n\t"
			"rTemp .req r7\n\t"
			// backup high registers. Used to hold a copy of a low register.
			"rStackPtrBackup .req r8\n\t"
			"OffsetValueBkp .req r9\n\t"
			// high registers used to change palette (until at least bi-tile 18 )
			"rNewPalette .req r10\n\t"
			"rNewColor .req r11\n\t"
			"rColorIndex .req r12\n\t"
			"rNewPalette2 .req r14\n\t"
			// high registers calculated at the beginning of the draw partial bi-tile code, used when the horizontal scroll is != 0
			"rWhereToJump .req r14\n\t" 
			// low registers (renamed!) used for the partial bi-tile, (i.e. when the scroll is != 0)
			"rTemp2 .req r1\n\t"  // same as Offset!
			"rShift .req r5\n\t"  // same as tileAddress!
			"Opixel2 .req r6\n\t" // same as bPixel!
			//			
			// prologue
			//
			// calculate the correct offsets
			// Note! For these pre-calculations, we use numeric registers!
			// get the y offset on the tile
			"LDR r7,videoDataAddr\n\t"					// get the structure address
			// get the current line number
			"LDR r0, [r7, %[currentLineNumberOffset]]\n\t"
			#if ENABLE_PALETTE_REMAPPING && ENABLE_PER_LINE_COLOR_REMAPPING && ENABLE_HIRES_PER_LINE_COLOR_REMAPPING
				// Palette
				"LDR pPalette,[r7,%[pPaletteOffset]]\n\t"  // get the palette base address
				#if ENABLE_PALETTE_REMAPPING
					#if ENABLE_PALETTE_ROW_REMAPPING
						"LDR r6, [r7, %[pPaletteRemappingRowOffsetsOffset]]\n\t"
						"ADD r6,r0\n\t"		// we need to access a 16 bit word. To do this we need to access to R6 + 2*r0. Hence we need a r0 offset here too...
						"LDRSH r6,[r6,r0]\n\t"  
						"ADD r6,r0\n\t"
					#else
						"MOV r6, r0\n\t"
					#endif
					"MOV rNewPalette,pPalette\n\t"				// copy the palette base address to rNewPalette
					"LDR r1,[r7, %[pPaletteIdxOffset]]\n\t"					// get the palette index table
					"LDRB r3,[r1, r6]\n\t"								// get the palette index of the current line
					// in r3 now we have: indexOfPaletteToBeChanged (4MSB) and indexOfCurrentalette (4LSB)
					"LSL r4, r3, #28\n\t"					// clear the upper 4 bits
					#if !USE_SEPARATE_FIXED_SECTION_PALETTE || USE_SECTION == NO_FIXED_SECTION

						"LSR r4, #18\n\t"						// each palette is a 1-kByte table, therefore we need to multiply the palette index by 1024... We multiplied by 2^28, now we divide by 2^18.
						"ADD pPalette, r4\n\t"					// add the offset to the palette	
					#else	
						// NOTE: DO NOT MODIFY r4 util fixed section! There will be used!!!
					#endif
					#if ENABLE_PER_LINE_COLOR_REMAPPING
						// Per line color remapping allows to change one single color entry (0 to 15) of one of the palettes, each line. Note, this has to be done by writing on 32 entries on the palette!
						// Get the index of the next row's palette
						"LSR r3, #4\n\t"						// clear the 4 LSB
						"LSL r3, #10\n\t"						// each palette is a 1-kByte table, therefore we need to multiply the palette index by 1024...
						"ADD rNewPalette, r3\n\t"					// add the offset to the palette
						// get the value of the new color for this line
						"LDR r1,[r7, %[pNewColorChangeTableOffset]]\n\t"		    // get the color change table
						"LDRB r1,[r1, r6]\n\t"										// get the actual value
						"MOV rNewColor,r1\n\t"										// save it
//#define PACK_TWO_PIXELS 1
#if PACK_TWO_PIXELS
						// finally get the index of which color must be changed
						"LSR r5,r6,#1\n\t"												// shift the lowres line, as two color indexes fit in a single byte
						"LDR r1,[r7, %[pNewColorChangeIndexTableOffset]]\n\t"		    // get the color change Index table
						"LDRB r1,[r1, r5]\n\t"										// get the actual value: note: there will be a high and a low nibble here...
						"BCC evenRow\n\t"
						"LSR r1,#4\n\t"
						"evenRow:\n\t"
						"MOV r3, #0xF\n\t"
						"AND r1,r3\n\t"
						"MOV rColorIndex, r1\n\t"
#else
						"LDR r1,[r7, %[pNewColorChangeIndexTableOffset]]\n\t"		    // get the color change Index table
						"LDRB r1,[r1, r6]\n\t"										// get the actual value: note: there will be a high and a low nibble here in hires mode...
						// now in R1 we have either the 4 LSB of the original value (evenRows), or the 4 MSB (odd rows)
						"MOV rColorIndex, r1\n\t"
#endif
					#endif
				#endif
			#else					
				// divide by two
				"LSR r1, r0,#1\n\t"    // r1 = currentLineNumber/2  ( r1 now is low res currentLineNumber)
				// Palette
				"LDR pPalette,[r7,%[pPaletteOffset]]\n\t"  // get the palette base address
				#if ENABLE_PALETTE_REMAPPING
					#if ENABLE_PALETTE_ROW_REMAPPING
						"LDR r6, [r7, %[pPaletteRemappingRowOffsetsOffset]]\n\t"
						"ADD r6,r1\n\t"		// we need to access a 16 bit word. To do this we need to access to R6 + 2*r1. Hence we need a r1 offset here too...
						"LDRSH r6,[r6,r1]\n\t"
						"ADD r6,r1\n\t"
					#else
						"MOV r6, r1\n\t"
					#endif				
					"MOV rNewPalette,pPalette\n\t"				// copy the palette base address to rNewPalette
					"LDR r1,[r7, %[pPaletteIdxOffset]]\n\t"					// get the palette index table
					"LDRB r3,[r1, r6]\n\t"								// get the palette index of the current line
					// in r3 now we have: indexOfPaletteToBeChanged (4MSB) and indexOfCurrentalette (4LSB)
					"LSL r4, r3, #28\n\t"					// clear the upper 4 bits
					#if !USE_SEPARATE_FIXED_SECTION_PALETTE || USE_SECTION == NO_FIXED_SECTION
				
						"LSR r4, #18\n\t"						// each palette is a 1-kByte table, therefore we need to multiply the palette index by 1024... We multiplied by 2^28, now we divide by 2^18.
						"ADD pPalette, r4\n\t"					// add the offset to the palette	
					#else	
						// NOTE: DO NOT MODIFY r4 util fixed section! There will be used!!!
					#endif
					#if ENABLE_PER_LINE_COLOR_REMAPPING
						// Per line color remapping allows to change one single color entry (0 to 15) of one of the palettes, each line. Note, this has to be done by writing on 32 entries on the palette!
						// Get the index of the next row's palette
						"LSR r3, #4\n\t"						// clear the 4 LSB
						"LSL r3, #10\n\t"						// each palette is a 1-kByte table, therefore we need to multiply the palette index by 1024...
						"ADD rNewPalette, r3\n\t"					// add the offset to the palette
						// get the value of the new color for this line
						"LDR r1,[r7, %[pNewColorChangeTableOffset]]\n\t"		    // get the color change table
						"LDRB r1,[r1, r6]\n\t"										// get the actual value
						"MOV rNewColor,r1\n\t"										// save it
						// finally get the index of which color must be changed
						"LDR r1,[r7, %[pNewColorChangeIndexTableOffset]]\n\t"		    // get the color change Index table
						"LDRB r1,[r1, r6]\n\t"										// get the actual value: note: there will be a high and a low nibble here in hires mode...
						// now in R1 we have either the 4 LSB of the original value (evenRows), or the 4 MSB (odd rows)
						"MOV rColorIndex, r1\n\t"
					#endif
				#endif
			#endif
			//	
			//
			#if 1
			#if USE_SECTION == TOP_FIXED_SECTION
				"LSR Offset,r0,#1\n\t"  // y = currentLine(HighRes)/2				
				"CMP Offset,#" LINELIMITSTR  "\n\t"
				"BGT afterSection\n\t"
				"LSR r3,Offset,#3\n\t"	// r3 = y / 8
				"MOV r0,#0\n\t"		// clear xscroll	(r0 will have that function after label "inSection")			
				#if ENABLE_PER_LINE_COLOR_REMAPPING && ENABLE_PALETTE_REMAPPING && USE_SEPARATE_FIXED_SECTION_PALETTE
					"MOV r4,%[fixedSectionPalette]\n\t"
					"LSL r4, #10\n\t"
					"ADD pPalette, r4\n\t"
				#endif				
				"B inSection\n\t"			
				"afterSection:\n\t"	
				#if ENABLE_PALETTE_REMAPPING && USE_SEPARATE_FIXED_SECTION_PALETTE
					"LSR r4, #18\n\t"						// each palette is a 1-kByte table, therefore we need to multiply the palette index by 1024... We multiplied by 2^28, now we divide by 2^18.
					"ADD pPalette, r4\n\t"					// add the offset to the palette
				#endif
			#elif USE_SECTION == BOTTOM_FIXED_SECTION
				"LSR Offset,r0,#1\n\t"  // y = currentLine(HighRes)/2
				"CMP Offset,#" LINELIMITSTR  "\n\t"
				"BLT afterSection\n\t"
				"ADD Offset,#8\n\t"		// add 8 to the line, because when the tile section is at the bottom, we need one tile of space to allow for scroll.
				"LSR r3,Offset,#3\n\t"	// r3 = y / 8
				"MOV r0,#0\n\t"		// clear xscroll (r0 will have that function after label "inSection")
				#if ENABLE_PALETTE_REMAPPING && USE_SEPARATE_FIXED_SECTION_PALETTE
					"MOV r4,%[fixedSectionPalette]\n\t"
					"LSL r4, #10\n\t"
					"ADD pPalette, r4\n\t"
				#endif				
				"B inSection\n\t"			
				"afterSection:\n\t"	
				#if ENABLE_PER_LINE_COLOR_REMAPPING && ENABLE_PALETTE_REMAPPING && USE_SEPARATE_FIXED_SECTION_PALETTE
					"LSR r4, #18\n\t"						// each palette is a 1-kByte table, therefore we need to multiply the palette index by 1024... We multiplied by 2^28, now we divide by 2^18.
					"ADD pPalette, r4\n\t"					// add the offset to the palette
				#endif
			#elif USE_SECTION != NO_FIXED_SECTION
				#error "You must define USE_SECTION either as NO_FIXED_SECTION or TOP_FIXED_SECTION or BOTTOM_FIXED_SECTION!"
			#endif
				#endif 
			#if USE_ROW_REMAPPING
				"LDR Offset,[r7, %[rowRemapPtrOffset]]\n\t"		// get the pointer to the row-remapping array.
				#if !USE_HIRES_ROW_REMAPPING
					"LSR r0,#1\n\t"				// low res row remapping
				#endif
				"LDRB Offset, [Offset, r0]\n\t"  // now rOffset (r1) is the remapped line
				"LDRB R3, [r7, %[yScrollOffset]]\n\t"
				"ADD Offset,r3\n\t"  // now rOffset (r1) is the vertical line (r0) + scroll.	
			#else
					"LSR r0,#1\n\t"  // if no fixed section is used and no row remapping, then we need to shift R0.
					"LDRB Offset, [r7, %[yScrollOffset]]\n\t"			
					"ADD Offset,r0\n\t"  // now rOffset (r1) is the vertical line (r0) + scroll.		
			#endif
			// Load xScroll of this line
			"LSR r3,Offset,#3\n\t"				// divide the current line number (offset) by 8. This will get the y position on vram.
			/* Regisgter status in this position
				r0 = current physical low res (QVGA) line
				r1 = Offset = remapped low res line
				r3 = which tile row the remapped line is pointing to.
			*/
			// scroll: based on the scroll value, we must calculate where we should jump.
			/* How is the horizontal scroll calculated:
				PER_LINE_X_SCROLL == 0 and PER_TLE_XSCROLL == 0
					xScroll = videoData.xScroll
				PER_LINE_X_SCROLL == 1 and PER_TLE_XSCROLL == 0
					xScroll = videoData.xScrollPtr[physicalLine] + videoData.xScroll
				PER_LINE_X_SCROLL == 1 and PER_TLE_XSCROLL == 1
					xScroll = videoData.ptrRowXScroll[physicalLine] + videoData.ptrTileXScroll[remappedLineTile]   // (remapped line = r3, physical line = r0)							
				PER_LINE_X_SCROLL == 0 and PER_TLE_XSCROLL == 1
					xScroll = videoData.xScroll + videoData.ptrTileXScroll[remappedLineTile]   // (remapped line = r3, physical line = r0)

				Fixed section: if Row remapping && (per Line || per Tile) then it is disabled, as it is assumed that these features already allow the creation of a fixed section.
			*/
			#if  PER_LINE_X_SCROLL == 0 && PER_TILE_X_SCROLL == 0
				"LDR r0,[r7, %[xScrollOffset]]\n\t"			// load in r0 the screen-wide scroll.
			#endif
			#if PER_LINE_X_SCROLL 
				"LDR r5,[r7, %[ptrRowXScrollOffset]]\n\t"		// get the pointer to the scroll-table
				#if PER_TILE_X_SCROLL == 0
					"LDRB r5,[r5, r0]\n\t"						// load the per line offset
					"LDR r0,[r7, %[xScrollOffset]]\n\t"			// load in r0 the screen-wide scroll.
					"ADD r0,r5,r0\n\t"							// sum this per-line scroll value to the screeen-wide scroll value
				#else
					"LDRB r0,[r5, r0]\n\t"					// load the per line offset
				#endif
			#endif
			#if PER_TILE_X_SCROLL 
				"LDR r5,[r7, %[ptrTileXScrollOffset]]\n\t"		// get the pointer to the scroll-table
				"LDRB r5,[r5, r3]\n\t"						// load the per row offset
				#if PER_LINE_X_SCROLL == 0
					"LDR r0,[r7, %[xScrollOffset]]\n\t"			// load in r0 the screen-wide scroll.
				#endif
				"ADD r0,r5,r0\n\t"							// sum this per-line scroll value to the screeen-wide scroll value
			#endif
			#if USE_SECTION != NO_FIXED_SECTION
				"inSection:\n\t"			
			#endif
			"LSL Offset,#29\n\t"
			"LSR Offset, #27\n\t"
			"MOV r5, #0x20\n\t"
			"LSL r5, r5,#24\n\t"		    // add the ram starting address value 0x20000000
			"ADD Offset,r5\n\t"	
			// save offset to its backup high register
			"MOV OffsetValueBkp,Offset\n\t"		
			//
			// now let's compute the the  address we should point to in the vram
			//
			// registers at this point:
			// r0: xScroll offset
			/* we must now set the start tile x position. 
			   In TILE_MODE2, the total xscroll can be arbitrary between 0 and (8 * (VRAMX-42) -1). 
			   The start bitile address is (r0 >> 4) << 2		
			*/
			"LSR r5, r0, #4\n\t"			// divide by 16
			"LSL r5, r5, #2\n\t"			// multiply by 4
			"MOV r4,%[vramx] * 2\n\t"			// get the number of bytes per row in r4
			"MUL r4, r3\n\t"		// multiply the row number (r3) by the number of bytes per row (r4)
			"ADD r4,r5\n\t"				// add to the offset
			"MOV r5, #0xF\n\t"
			"AND r0, r5\n\t"				// x scroll is the lower part of r5
			"LDR r5, vramAddr\n\t"  // get the vram address (r5)
			"ADD r4, r5\n\t"				// r4 = r4 + r5 = pointer to the first tile address of the current row.
			// warning DO NOT MODIFY r4 until we copy it to the SP!!! 
			// now get the last tile address of the row (= first position + 2 * numberOfHorizontalTilesPerScreen]
			"MOV r5,%[numberOfHorizontalTilesPerScreen]*2\n\t"			// r5 =  2 * (numberOfHorizontalTilesPerScreen)
			"ADD r5, r4\n\t"			// r5 the address pointing to the of the last two tile addresses.
			"LDR r5, [r5]\n\t"			// in r5 we have now the last two addresses...
			"UXTH r3, r5\n\t"			// get the first low address
			// 2020/05/08: note. Here we should - like the 8bpp case - avoid to read the last pixels when the offset is 0.
			// However, instead of adding complexity, we just added another location in VRAM, so that there will always be 
			// a "valid" address (or better, an address, which might contain rubbish - and in that case won't be used- but
			// won't create any hard fault. 
			"LDR r3,[r3, Offset]\n\t"   // in r4 the 8 4-bit color indexes of the first part of "last" (partial) bi-tile.
			"LSR r5, #16\n\t"			// in r5 the lower part of the second address
			"LDR r5, [r5, Offset]\n\t"  // load the 8 4-bit color indexes of the last part of the "last" (partial) bi-tile
			"PUSH {r3,r5}\n\t"			// save them into stack for later use! Save also the shift register!
			// Now get the vectors where we will have to jump to later.
			"LSL r3,r0,#3\n\t"				// multiply the scroll amount by 8
			"ADR r5,scrollTable\n\t"    // get the jump table address
			"ADD r3,r5\n\t"				// sum it
			"LDMIA r3, {r3, r6}\n\t"		// get the jump vectors
			"PUSH {r0,r3}\n\t"			// push the scroll and "wheretojump" values.*/				
			"LDR rPort,[r7, %[pPortOffset]]\n\t"		// set port register
			//
			"MOV rStackPtrBackup,SP\n\t"				// copy the stack pointer to its designated backup register
			"MOV SP, r4\n\t" 				// copy the list of tile addresses to the stack pointer
			//
			// now get the and constant
			"MOV AndMask, #0xFF\n\t"
			"LSL AndMask,#2\n\t"
			// get tile addresses (r5)
			"POP {tileAddresses}\n\t"
			"BX r6\n\t" 
			".align(2)\n\t"
			"videoDataAddr:\n\t"
				".word videoData\n\t"
			"vramAddr:\n\t"
				".word vram\n\t"
			// this table contains the jump address to where the first bi-tile should begin, and which last bi-tile draw routine should point to.
			"scrollTable:\n\t"
				".word endDraw + 1\n\t"
				".word scroll0+1\n\t"
				".word oddShift + 1 \n\t"
				".word scroll1+1\n\t"
				".word evenShift + 1 \n\t"
				".word scroll2+1\n\t"
				".word oddShift + 1 \n\t"
				".word scroll3+1\n\t"
				".word evenShift + 1 \n\t"
				".word scroll4+1\n\t"
				".word oddShift + 1 \n\t"
				".word scroll5+1\n\t"
				".word evenShift + 1 \n\t"
				".word scroll6+1\n\t"
				".word oddShift + 1 \n\t"
				".word scroll7+1\n\t"
				".word evenShift + 1 \n\t"
				".word scroll8+1\n\t"
				".word oddShift + 1 \n\t"
				".word scroll9+1\n\t"
				".word evenShift + 1 \n\t"
				".word scroll10+1\n\t"
				".word oddShift + 1 \n\t"
				".word scroll11+1\n\t"
				".word evenShift + 1 \n\t"
				".word scroll12+1\n\t"
				".word oddShift + 1 \n\t"
				".word scroll13+1\n\t"
				".word evenShift + 1 \n\t"
				".word scroll14+1\n\t"
				".word oddShift + 1 \n\t"
				".word scroll15+1\n\t"
			"scroll0:\n\t"
				"UXTH bPixel, tileAddresses\n\t"	      /* extract the lower part of first tile address */
				"LDR Opixel,[bPixel, Offset]\n\t"	     /* get the 8 4-bit indexes of the first tile in the pair */
				"LSL bPixel,Opixel,#2\n\t"		     /* shift left the indexes so that those of p0 and p1 are in the right place - for word alignment */
				"WFI\n\t"
				"B scroll0_bitile0_start\r\n"
				//
			"scroll1:\n\t"
				"UXTH bPixel, tileAddresses\n\t"	      /* extract the lower part of first tile address */
				"LDR Opixel,[bPixel, Offset]\n\t"	     /* get the 8 4-bit indexes of the first tile in the pair */
				"LSL bPixel,Opixel,#2\n\t"		     /* shift left the indexes so that those of p0 and p1 are in the right place - for word alignment */
				"AND bPixel,AndMask\n\t"			/* get the two 4-bit indexes of p0 and p1 */
				"LDR bPixel,[pPalette, bPixel]\n\t"		/* load p0 and p1 RGB values */
				"WFI\n\t"
				"B scroll1_bitile0_start\r\n"
			"scroll2:\n\t"
				"UXTH bPixel, tileAddresses\n\t"	      /* extract the lower part of first tile address */
				"LDR Opixel,[bPixel, Offset]\n\t"	     /* get the 8 4-bit indexes of the first tile in the pair */
				"LSR rTemp,Opixel,#6\n\t"			/* shift the 32-bit pixel data so that indexes of p2 and p3 in the correct positions */
				"WFI\n\t"
				"B scroll2_bitile0_start\r\n"
			"scroll3:\n\t"
				"UXTH bPixel, tileAddresses\n\t"	      /* extract the lower part of first tile address */
				"LDR Opixel,[bPixel, Offset]\n\t"	     /* get the 8 4-bit indexes of the first tile in the pair */
				"LSR rTemp,Opixel,#6\n\t"			/* shift the 32-bit pixel data so that indexes of p2 and p3 in the correct positions */
				"AND rTemp,AndMask\n\t"				/* get the two 4-bit indexes of p2 and p3 */
				"LDR rTemp,[pPalette, rTemp]\n\t"			/* load p2 and p3 RGB values */
				"WFI\n\t"
				"B scroll3_bitile0_start\r\n"
			"scroll4:\n\t"
				"UXTH bPixel, tileAddresses\n\t"	      /* extract the lower part of first tile address */
				"LDR Opixel,[bPixel, Offset]\n\t"	     /* get the 8 4-bit indexes of the first tile in the pair */
				"LSR bPixel,Opixel,#14\n\t"			/* shift the 32-bit pixel data so that indexes of p4 and p5 are in the correct positions */
				"WFI\n\t"
				"B scroll4_bitile0_start\r\n"
			"scroll5:\n\t"
				"UXTH bPixel, tileAddresses\n\t"	      /* extract the lower part of first tile address */
				"LDR Opixel,[bPixel, Offset]\n\t"	     /* get the 8 4-bit indexes of the first tile in the pair */
				"LSR bPixel,Opixel,#14\n\t"			/* shift the 32-bit pixel data so that indexes of p4 and p5 are in the correct positions */
				"AND bPixel,AndMask\n\t"			/* get the two 4-bit indexes p4 and p5. */
				"LDR bPixel,[pPalette, bPixel]\n\t"		/* load p4 and p5 RGB values */
				"WFI\n\t"
				"B scroll5_bitile0_start\r\n"
			"scroll6:\n\t"
				"UXTH bPixel, tileAddresses\n\t"	      /* extract the lower part of first tile address */
				"LDR Opixel,[bPixel, Offset]\n\t"	     /* get the 8 4-bit indexes of the first tile in the pair */
				"LSR tileAddresses, #16\n\t"			/* extract now the new tile address (p8-p15). We do it here only because we have a spare cycle. */
				"LSR rTemp,Opixel,#22\n\t"			    /* now rTemp is free. We can shift Opixel so that the next two 4-bit indexes - p6 and p7 - are in the correct place */
				"WFI\n\t"
				"B scroll6_bitile0_start\r\n"
			"scroll7:\n\t"
				"UXTH bPixel, tileAddresses\n\t"	      /* extract the lower part of first tile address */
				"LDR Opixel,[bPixel, Offset]\n\t"	     /* get the 8 4-bit indexes of the first tile in the pair */
				"LSR tileAddresses, #16\n\t"			/* extract now the new tile address (p8-p15). We do it here only because we have a spare cycle. */
				"LSR rTemp,Opixel,#22\n\t"			    /* now rTemp is free. We can shift Opixel so that the next two 4-bit indexes - p6 and p7 - are in the correct place */
				"AND rTemp,AndMask\n\t"			/* get the two 4-bit indexes of p6 and p7 */
				"LDR rTemp,[pPalette,rTemp]\n\t"		/* load p6 and p7 RGB values */
				"WFI\n\t"
				"B scroll7_bitile0_start\r\n"
			"scroll8:\n\t"
				"LSR tileAddresses, #16\n\t"			/* extract now the new tile address (p8-p15). We do it here only because we have a spare cycle. */
				"LDR tileAddresses,[tileAddresses, Offset]\n\t"     /* tileaddress is no more used now. it will be used as Opixel for the next 8 pixels. */
				"LSL bPixel,tileAddresses,#2\n\t"	/* shift tileAddresses - now used as Opixel - in bPixel, so that p8 and p9 are in the right positions */
				"WFI\n\t"
				"B scroll8_bitile0_start\r\n"
			"scroll9:\n\t"
				"LSR tileAddresses, #16\n\t"			/* extract now the new tile address (p8-p15). We do it here only because we have a spare cycle. */
				"LDR tileAddresses,[tileAddresses, Offset]\n\t"     /* tileaddress is no more used now. it will be used as Opixel for the next 8 pixels. */
				"LSL bPixel,tileAddresses,#2\n\t"	/* shift tileAddresses - now used as Opixel - in bPixel, so that p8 and p9 are in the right positions */
				"AND bPixel,AndMask\n\t"	        /* get the two 4-bit indexes of p8 and p9 */
				"LSR Opixel,tileAddresses,#6\n\t"       /* shift tileAddresses - now used as Opixel - in bPixel, so that p10 and p11 are in the right positions */
				"LDR bPixel,[pPalette, bPixel]\n\t"	/* load p8 and p9 RGB values */
				"WFI\n\t"
				"B scroll9_bitile0_start\r\n"
			"scroll10:\n\t"
				"LSR tileAddresses, #16\n\t"			/* extract now the new tile address (p8-p15). We do it here only because we have a spare cycle. */
				"LDR tileAddresses,[tileAddresses, Offset]\n\t"     /* tileaddress is no more used now. it will be used as Opixel for the next 8 pixels. */
				"LSR Opixel,tileAddresses,#6\n\t"       /* shift tileAddresses - now used as Opixel - in bPixel, so that p10 and p11 are in the right positions */
				"WFI\n\t"
				"B scroll10_bitile0_start\r\n"
			"scroll11:\n\t"
				"LSR tileAddresses, #16\n\t"			/* extract now the new tile address (p8-p15). We do it here only because we have a spare cycle. */
				"LDR tileAddresses,[tileAddresses, Offset]\n\t"     /* tileaddress is no more used now. it will be used as Opixel for the next 8 pixels. */
				"LSR Opixel,tileAddresses,#6\n\t"       /* shift tileAddresses - now used as Opixel - in bPixel, so that p10 and p11 are in the right positions */
				"AND Opixel,AndMask\n\t"	   		/* in the instruction above, we prepared in Opixel the tile pixel index data so that p10 and p11 were in the right position for this and operation. */
				"LDR rTemp,[pPalette, Opixel]\n\t"		/* load the RGB values of p10 and p11 */
				"WFI\n\t"
				"B scroll11_bitile0_start\r\n"
			"scroll12:\n\t"
				"LSR tileAddresses, #16\n\t"			/* extract now the new tile address (p8-p15). We do it here only because we have a spare cycle. */
				"LDR tileAddresses,[tileAddresses, Offset]\n\t"     /* tileaddress is no more used now. it will be used as Opixel for the next 8 pixels. */
				"LSR Opixel,tileAddresses,#22\n\t"		/* move p14 and p15 pixel indexes in the right positions */ 
				"LSR bPixel,tileAddresses,#14\n\t"		/* move p12 and p13 pixel indexes in the right positions */
				"WFI\n\t"
				"B scroll12_bitile0_start\r\n"
			"scroll13:\n\t"
				"LSR tileAddresses, #16\n\t"			/* extract now the new tile address (p8-p15). We do it here only because we have a spare cycle. */
				"LDR tileAddresses,[tileAddresses, Offset]\n\t"     /* tileaddress is no more used now. it will be used as Opixel for the next 8 pixels. */
				"LSR Opixel,tileAddresses,#22\n\t"		/* move p14 and p15 pixel indexes in the right positions */
				"LSR bPixel,tileAddresses,#14\n\t"		/* move p12 and p13 pixel indexes in the right positions */
				"AND bPixel,AndMask\n\t"			/* extract p12 and p13 indexes */
				"LDR bPixel,[pPalette, bPixel]\n\t"		/* load p12 and p13 RGB values */
				"WFI\n\t"
				"B scroll13_bitile0_start\r\n"
			"scroll14:\n\t"
				"LSR tileAddresses, #16\n\t"			/* extract now the new tile address (p8-p15). We do it here only because we have a spare cycle. */
				"LDR tileAddresses,[tileAddresses, Offset]\n\t"     /* tileaddress is no more used now. it will be used as Opixel for the next 8 pixels. */
				"LSR Opixel,tileAddresses,#22\n\t"		/* move p14 and p15 pixel indexes in the right positions */
				"AND Opixel,AndMask\n\t"			/* extract p14 and p15 indexes*/
				"POP {tileAddresses}\n\t"			/* get the lower parts of the next two tile addresses - the higher is always 0x2000 */
				"WFI\n\t"
				"B scroll14_bitile0_start\r\n"
			"scroll15:\n\t"
				"LSR tileAddresses, #16\n\t"			/* extract now the new tile address (p8-p15). We do it here only because we have a spare cycle. */
				"LDR tileAddresses,[tileAddresses, Offset]\n\t"     /* tileaddress is no more used now. it will be used as Opixel for the next 8 pixels. */
				"LSR Opixel,tileAddresses,#22\n\t"		/* move p14 and p15 pixel indexes in the right positions */
				"AND Opixel,AndMask\n\t"			/* extract p14 and p15 indexes*/
				"POP {tileAddresses}\n\t"			/* get the lower parts of the next two tile addresses - the higher is always 0x2000 */
				"LDR rTemp,[pPalette, Opixel]\n\t"             /* load p14 and p15 rgb values */
				"UXTH bPixel, tileAddresses\n\t"	      /* extract the lower part of first tile address */
				"WFI\n\t"
				"B scroll15_bitile0_start\r\n"
			//
			// NOTE: THE FOLLOWING MUST BE ALIGN 4 !!!
			".align(4)\n\t"			
			".byte 0\n\t"	
			".byte 0\n\t"
			"testDraw:\n\t"	
	DRAW_FIRST_TWO_TILES(0)
#if ENABLE_PER_LINE_COLOR_REMAPPING	&& ENABLE_PALETTE_REMAPPING
/* first, we use 8+8 spare cycles to compute the correct values of rNewPalette, rNewPalette2 and rColorIndex.	
	//bitile1
	1 MOV rNewPalette2, rNewPalette  <--- copy rNewPalette to rNewPalette2.
	2 MOV rTemp,rColorIndex			 <--- get the index of the color we want to modify.
	3 LSL rTemp,#6					 <---multiply it by 4 * 16 (shift left 6)
	4: ADD rNewPalette,Temp			 <---Now rNewPalette points to the words that contains the first entry (on its upper halfword) of the odd pixel color value identified by rColorIndex.
	5: LSR rTemp,#4					 <--- shift back color index by 4 position. We need to point now to the first even-pixel color entry.
	6: ADD rNewPalette2, rTemp	     <--- and add it to the rNewPalette2 register.
	7: NOP
	8: NOP

	//bitile 2
	1: MOV Offset,rNewColor				<--- get the new color value
	2 MOV rTemp,#64				<--- save to color index 64...
	3 MOV rColorIndex,rTemp				<--- ...so that we can point each time to the next two entries
	4 LSL Offset,#10				<--- Now, we must multiply rNewColor by 1025, to get the rgb signals
	5 LSL rTemp,#3					<--- rTemp = 512  (so that the SD is disabled)
	6 ORR rOffset,rTemp 				<--- Set the disable SD pin (PA09)
	7 ADD rNewColor,Offset				<--- rNewcolor = (1024*rNewColor | 512) + rNewColor
	8: MOV Offset,OffsetValueBkp
*/

// Warning! Do not modify this. The order of memory access has been carefully optimized not to produce any bus stall.
/*3-4*/		DRAW_TWO_TILES(1, "MOV rNewPalette2,rNewPalette\n\t" , "MOV rTemp, rColorIndex\n\tLSL rTemp,#6\n\tADD rNewPalette,rTemp\n\t",  "LSR rTemp,#4\n\tADD rNewPalette2,rTemp\n\tNOP\n\t", "NOP\n\t")
/*5-6*/		DRAW_TWO_TILES(2, "MOV Offset,rNewColor\n\t" , "MOV rTemp, #64\n\tMOV rColorIndex,rTemp\n\tLSL Offset,#10\n\t",   "LSL rTemp,#3\n\tORR Offset,rTemp\n\tADD rNewColor,Offset\n\t", "MOV Offset,OffsetValueBkp\n\t")
/*7-8*/		DRAW_TWO_TILES2(3, "NOP\n\t", "MOV rTemp,rNewColor\n\t","MOV Opixel,rNewPalette2\n\tSTRH rTemp,[Opixel]\n\t","MOV bPixel,rNewPalette\n\t","STRH rTemp,[bPixel,#2]\n\t")
/*9-10*/	DRAW_TWO_TILES2(4, "ADD rNewPalette2,rColorIndex\n\t", "MOV rTemp,rNewColor\n\t","MOV Opixel,rNewPalette\n\tSTRH rTemp,[Opixel,#6]\n\t","MOV bPixel,rNewPalette2\n\t","STRH rTemp,[bPixel]\n\t")
/*11-12*/	DRAW_TWO_TILES2(5, "ADD rNewPalette2,rColorIndex\n\t", "MOV rTemp,rNewColor\n\t","MOV Opixel,rNewPalette2\n\tSTRH rTemp,[Opixel]\n\t","MOV bPixel,rNewPalette\n\t","STRH rTemp,[bPixel,#14]\n\t") // 14 instead of 10!!!
/*13-14*/	DRAW_TWO_TILES2(6, "ADD rNewPalette2,rColorIndex\n\t", "MOV rTemp,rNewColor\n\t","MOV Opixel,rNewPalette\n\tSTRH rTemp,[Opixel,#10]\n\t","MOV bPixel,rNewPalette2\n\t","STRH rTemp,[bPixel]\n\t") // 10 insetad of 14!
/*15-16*/	DRAW_TWO_TILES2(7, "ADD rNewPalette2,rColorIndex\n\t", "MOV rTemp,rNewColor\n\t","MOV Opixel,rNewPalette2\n\tSTRH rTemp,[Opixel]\n\t","MOV bPixel,rNewPalette\n\t","STRH rTemp,[bPixel,#18]\n\t") // 
/*17-18*/	DRAW_TWO_TILES2(8, "ADD rNewPalette2,rColorIndex\n\t", "MOV rTemp,rNewColor\n\t","MOV Opixel,rNewPalette\n\tSTRH rTemp,[Opixel,#22]\n\t","MOV bPixel,rNewPalette2\n\t","STRH rTemp,[bPixel]\n\t") // 
/*19-20*/	DRAW_TWO_TILES2(9, "ADD rNewPalette2,rColorIndex\n\t", "MOV rTemp,rNewColor\n\t","MOV Opixel,rNewPalette2\n\tSTRH rTemp,[Opixel]\n\t","MOV bPixel,rNewPalette\n\t","STRH rTemp,[bPixel,#30]\n\t") // 30 instead of 26!
/*21-22*/	DRAW_TWO_TILES2(10, "ADD rNewPalette2,rColorIndex\n\t", "MOV rTemp,rNewColor\n\t","MOV Opixel,rNewPalette\n\tSTRH rTemp,[Opixel,#26]\n\t","MOV bPixel,rNewPalette2\n\t","STRH rTemp,[bPixel]\n\t") // 26 instead of 30
/*23-24*/	DRAW_TWO_TILES2(11, "ADD rNewPalette2,rColorIndex\n\t", "MOV rTemp,rNewColor\n\t","MOV Opixel,rNewPalette2\n\tSTRH rTemp,[Opixel]\n\t","MOV bPixel,rNewPalette\n\t","STRH rTemp,[bPixel,#34]\n\t") //
/*25-26*/	DRAW_TWO_TILES2(12, "ADD rNewPalette2,rColorIndex\n\t", "MOV rTemp,rNewColor\n\t","MOV Opixel,rNewPalette\n\tSTRH rTemp,[Opixel,#38]\n\t","MOV bPixel,rNewPalette2\n\t","STRH rTemp,[bPixel]\n\t") //
/*27-28*/	DRAW_TWO_TILES2(13, "ADD rNewPalette2,rColorIndex\n\t", "MOV rTemp,rNewColor\n\t","MOV Opixel,rNewPalette2\n\tSTRH rTemp,[Opixel]\n\t","MOV bPixel,rNewPalette\n\t","STRH rTemp,[bPixel,#46]\n\t") // 46 instead of 42!
/*29-30*/	DRAW_TWO_TILES2(14, "ADD rNewPalette2,rColorIndex\n\t", "MOV rTemp,rNewColor\n\t","MOV Opixel,rNewPalette\n\tSTRH rTemp,[Opixel,#42]\n\t","MOV bPixel,rNewPalette2\n\t","STRH rTemp,[bPixel]\n\t") // 42 instead of 46!
/*31-32*/	DRAW_TWO_TILES2(15, "ADD rNewPalette2,rColorIndex\n\t", "MOV rTemp,rNewColor\n\t","MOV Opixel,rNewPalette2\n\tSTRH rTemp,[Opixel]\n\t","MOV bPixel,rNewPalette\n\t","STRH rTemp,[bPixel,#50]\n\t") //
/*33-34*/	DRAW_TWO_TILES2(16, "ADD rNewPalette2,rColorIndex\n\t", "MOV rTemp,rNewColor\n\t","MOV Opixel,rNewPalette\n\tSTRH rTemp,[Opixel,#54]\n\t","MOV bPixel,rNewPalette2\n\t","STRH rTemp,[bPixel]\n\t") //
/*35-36*/	DRAW_TWO_TILES2(17, "ADD rNewPalette2,rColorIndex\n\t", "MOV rTemp,rNewColor\n\t","MOV Opixel,rNewPalette2\n\tSTRH rTemp,[Opixel]\n\t","MOV bPixel,rNewPalette\n\t","STRH rTemp,[bPixel,#62]\n\t") // 62 instead of 58!
/*37-38*/	DRAW_TWO_TILES2(18, "ADD rNewPalette2,rColorIndex\n\t", "MOV rTemp,rNewColor\n\t","MOV Opixel,rNewPalette\n\tSTRH rTemp,[Opixel,#58]\n\t","MOV bPixel,rNewPalette2\n\t","STRH rTemp,[bPixel]\n\t") // 58 instead of 62!

			
#else
			DRAW_TWO_TILES(1, "NOP\n\t" , "NOP\n\tNOP\n\tNOP\n\t",  "NOP\n\tNOP\n\tNOP\n\t", "NOP\n\t")	 
			DRAW_TWO_TILES(2, "NOP\n\t" , "NOP\n\tNOP\n\tNOP\n\t",  "NOP\n\tNOP\n\tNOP\n\t", "NOP\n\t")
			DRAW_TWO_TILES(3, "NOP\n\t" , "NOP\n\tNOP\n\tNOP\n\t",  "NOP\n\tNOP\n\tNOP\n\t", "NOP\n\t")
			DRAW_TWO_TILES(4, "NOP\n\t" , "NOP\n\tNOP\n\tNOP\n\t",  "NOP\n\tNOP\n\tNOP\n\t", "NOP\n\t")
			DRAW_TWO_TILES(5, "NOP\n\t" , "NOP\n\tNOP\n\tNOP\n\t",  "NOP\n\tNOP\n\tNOP\n\t", "NOP\n\t")
			DRAW_TWO_TILES(6, "NOP\n\t" , "NOP\n\tNOP\n\tNOP\n\t",  "NOP\n\tNOP\n\tNOP\n\t", "NOP\n\t")
			DRAW_TWO_TILES(7, "NOP\n\t" , "NOP\n\tNOP\n\tNOP\n\t",  "NOP\n\tNOP\n\tNOP\n\t", "NOP\n\t")
			DRAW_TWO_TILES(8, "NOP\n\t" , "NOP\n\tNOP\n\tNOP\n\t",  "NOP\n\tNOP\n\tNOP\n\t", "NOP\n\t")
			DRAW_TWO_TILES(9, "NOP\n\t" , "NOP\n\tNOP\n\tNOP\n\t",  "NOP\n\tNOP\n\tNOP\n\t", "NOP\n\t")
			DRAW_TWO_TILES(10, "NOP\n\t" , "NOP\n\tNOP\n\tNOP\n\t",  "NOP\n\tNOP\n\tNOP\n\t", "NOP\n\t")
			DRAW_TWO_TILES(11, "NOP\n\t" , "NOP\n\tNOP\n\tNOP\n\t",  "NOP\n\tNOP\n\tNOP\n\t", "NOP\n\t")
			DRAW_TWO_TILES(12, "NOP\n\t" , "NOP\n\tNOP\n\tNOP\n\t",  "NOP\n\tNOP\n\tNOP\n\t", "NOP\n\t")
			DRAW_TWO_TILES(13, "NOP\n\t" , "NOP\n\tNOP\n\tNOP\n\t",  "NOP\n\tNOP\n\tNOP\n\t", "NOP\n\t")
			DRAW_TWO_TILES(14, "NOP\n\t" , "NOP\n\tNOP\n\tNOP\n\t",  "NOP\n\tNOP\n\tNOP\n\t", "NOP\n\t")
			DRAW_TWO_TILES(15, "NOP\n\t" , "NOP\n\tNOP\n\tNOP\n\t",  "NOP\n\tNOP\n\tNOP\n\t", "NOP\n\t")
			DRAW_TWO_TILES(16, "NOP\n\t" , "NOP\n\tNOP\n\tNOP\n\t",  "NOP\n\tNOP\n\tNOP\n\t", "NOP\n\t")
			DRAW_TWO_TILES(17, "NOP\n\t" , "NOP\n\tNOP\n\tNOP\n\t",  "NOP\n\tNOP\n\tNOP\n\t", "NOP\n\t")
			DRAW_TWO_TILES(18, "NOP\n\t" , "NOP\n\tNOP\n\tNOP\n\t",  "NOP\n\tNOP\n\tNOP\n\t", "NOP\n\t")
#endif
			DRAW_LAST_TWO_TILES()		// these are always drawn
			DRAW_PARTIAL_TILES()		// these are drawn only if shift != 0		
			:
			:
				#if USE_ROW_REMAPPING
					[rowRemapPtrOffset] "I" ((uint8_t) offsetof(videoData_t, ptrRowRemapTable)),
				#endif
					[vramx] "I" ( (uint8_t) VRAMX),
					[numberOfHorizontalTilesPerScreen] "I" ((uint8_t) (SCREEN_SIZE_X / TILE_SIZE_X)),
					[currentLineNumberOffset] "I" ((uint8_t) offsetof(videoData_t, currentLineNumber)),
					[yScrollOffset] "I"  ((uint8_t) offsetof(videoData_t, yScroll)),
					[pPortOffset] "I"  ((uint8_t) offsetof(videoData_t, pPort)),
					[pPaletteOffset] "I"  ((uint8_t) offsetof(videoData_t, pPalette))
				#if PER_LINE_X_SCROLL
					,[ptrRowXScrollOffset] "I" ((uint8_t) offsetof(videoData_t, ptrRowXScroll))
				#endif
				#if ENABLE_PALETTE_REMAPPING
					,[pPaletteIdxOffset] "I" ((uint8_t) offsetof(videoData_t, pPaletteIdx))
					#if ENABLE_PER_LINE_COLOR_REMAPPING 
						,[pNewColorChangeTableOffset] "I" ((uint8_t) offsetof(videoData_t, pNewColorChangeTable))
						,[pNewColorChangeIndexTableOffset] "I" ((uint8_t) offsetof(videoData_t, pNewColorChangeIndexTable))
					#endif	
					#if ENABLE_PALETTE_ROW_REMAPPING
						,[pPaletteRemappingRowOffsetsOffset] "I" ((uint8_t) offsetof(videoData_t, pPaletteRemappingRowOffsets))					
					#endif
				#endif			
				#if PER_TILE_X_SCROLL
					,[ptrTileXScrollOffset] "I" ((uint8_t) offsetof(videoData_t, ptrTileXScroll))
				#endif
				#if ENABLE_PALETTE_REMAPPING && USE_SEPARATE_FIXED_SECTION_PALETTE
				, [fixedSectionPalette] "I" ((uint8_t) FIXED_SECTION_PALETTE_INDEX)
				#endif				
				#if PER_LINE_X_SCROLL == 0 || PER_TILE_X_SCROLL == 0
					,[xScrollOffset] "I" ((uint8_t) offsetof(videoData_t, xScroll))
				#endif	
			: "r0" , "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r14"
		);
		#elif GFX_MODE == TILE_MODE_BOOT
		/*
			This mode is for the bootloader. It's designed so that it will occupy the smallest amount of ram, whilst still being able to show 256 color graphics, for the game title preview.
			The downside is that each tile occupies a lot of ram (128 bytes/tile) as all the signals are stored in the uncompressed format.
			Register usage:
			- LOW:
			rPort				: the usual pPort address
			rBlackPixel		    : The value corresponding to black (it has also the other signals set to their correct value, so it is not 0)
			rVramRowStart          : This determines the first tile in the current row. It is ((int)currentLine / (2 * TILESIZEY)) * VRAMX + &vram[0], i.e. &vram[0 + y*VRAMX]
			rHighTileOffset		: this is &tiles[0] + ((currentLine / 2) % TILE_SIZE_Y) * TILE_SIZE_X * 2
			rBiPixel			: two pixel already in their uncompressed format.  It is initially loaded with the current line, to perform some calculations
			//
			rBiPixel2			: other two pixel already in their uncompressed format

			//
			rTileXPos			: the current horizontal tile we are processing.
			rTileAddr			: The address of the current tile
			- HIGH:
		*/
		asm volatile 
		(
			//		Prologue:
		"rPort .req r0\n\t"
		"rBlackPixel .req r1\n\t"
		"rVramRowStart .req r2\n\t"
		"rHighTileOffset .req r3\n\t"
		"rBiPixel .req r4\n\t"
		"rBiPixel2 .req r5\n\t"
		"rTileXPos .req r6\n\t"
		"rTileAddr .req r7\n\t"
		// First, big load of data
		"LDR rPort, videoDataAddr\n\t"
		"LDM rPort, {rPort, rBlackPixel, rVramRowStart, rHighTileOffset, rBiPixel, rBiPixel2}\n\t"
		// now, port is read, and also rBlackPixel. We need to correct rVramRowStart and rHighTileOffset, based on currentLine, which is rBiPixel
		"LSR rBiPixel,#1\n\t" // divide by two the current number of line.
		"LDRB rBiPixel,[rBiPixel2, rBiPixel]\n\t"    // remap the current line
		"LSR rBiPixel2, rBiPixel,#3\n\t"			// rBiPixel2 = currentLineHires/8 (i.e. currentLowResLine / ( TILESIZEY))				
		"MOV rTileXPos, %[vramx]\n\t"				// multiply it by VRAMX
		"MUL rBiPixel2, rTileXPos\n\t"				// done
		"ADD rVramRowStart, rBiPixel2\n\t"				// add this to the rVramRowStart. In this way we have the address of VRAM[y * VRAMX], where y is the y of the tile currently being drawn.
		// to perform ((currentLine / 2) % TILE_SIZE_Y), i.e. (currentLine >> 1) & 7, we can use two shifts.
		"LSL rBiPixel,#(32-3)\n\t"					// this will leave 0bWXYZ000000....0
		"LSR rBiPixel,#(32-3)\n\t"					// this will leave 0b00000..00WXY
		"LSL rBiPixel,#4\n\t"						// now we get 0b00...00WXY0000
		"ADD rHighTileOffset, rBiPixel\n\t"		
		//
		"MOV rTileXPos, #0\n\t"						// rTileXPos = 0
		//
		"LDRB rTileAddr, [rVramRowStart, rTileXPos]\n\t" //load the current tile number. 
		"LSL rTileAddr,#7\n\t"			     // tiles are 128 bytes in size, so to get the address we need to multiply the number by 128.
		"WFI\n\t"                            // wait for hsync
		"ADD rTileAddr, rHighTileOffset\n\t" // Let's add the offset, i.e. the address of the first tile at the currently displayed row.
		"LDMIA rTileAddr!, {rBiPixel}\n\t"		 // load the first couple of uncompressed pixels. Note, this autoincrements.
		"B Loop\n\t"
		".align(2)\n\t"
		"videoDataAddr:\n\t"
		".word videoData\n\t"
		"Loop:\n\t"
		"STRH rBiPixel, [rPort,#2]\n\t"		// output the first pixel
		"NOP\n\t"
		"NOP\n\t"
		"NOP\n\t"
		"STR rBiPixel, [rPort]\n\t"			// output the second pixel
		"NOP\n\t"							
		"LDMIA rTileAddr!, {rBiPixel}\n\t"	// load the third and fourth pixel signals.
		"STRH rBiPixel, [rPort,#2]\n\t"		// output the third
		"NOP\n\t"
		"NOP\n\t"
		"NOP\n\t"
		"STR rBiPixel, [rPort]\n\t"			// output the fourth pixel
		"NOP\n\t"
		"LDMIA rTileAddr!, {rBiPixel}\n\t"	// load the fifth and sixth pixel signals.
		"STRH rBiPixel, [rPort,#2]\n\t"		// output the fifth
		"ADD rTileXPos,#1\n\t"				// point to next x tile position
		"LDMIA rTileAddr!, {rBiPixel2}\n\t"    // load the seventh and eigthth pixel signals. We must do it already because we will have to load the next tile data soon. We need to use another register though.
		"STR rBiPixel, [rPort]\n\t"			// output the sitxth
		"LDRB rTileAddr, [rVramRowStart, rTileXPos]\n\t"	// Load the next tile number
		"LSL rTileAddr,#7\n\t"			     // tiles are 128 bytes in size, so to get the address we need to multiply the number by 128.
		"STRH rBiPixel2, [rPort,#2]\n\t"			// output the seventh
		"ADD rTileAddr, rHighTileOffset\n\t"	    // compute the new full tile address (at the correct y position on that tile)
		"LDMIA rTileAddr!, {rBiPixel}\n\t"			// load the first and second pixel signals of the next tile.
		"STR rBiPixel2, [rPort]\n\t"				// store last tile
		"CMP rTileXPos, %[vramx]\n\t"
		"BNE Loop\n\t"
		"STR rBlackPixel, [rPort]\n\t"				// black pixel
		:
		:
		[vramx] "I" ( (uint8_t) VRAMX)
		: "r0" , "r1", "r2", "r3", "r4", "r5", "r6", "r7"	
		);

		#else
			#error no valid GFX_MODE found
		#endif
	}
	#if GFX_MODE != TILE_MODE_BOOT
	else if (lineNumber == 400) //
	{ 
			videoData.currentFrame++;
	}
	#endif		
	else if (lineNumber == 524)
	{
		lineNumber = -1;
	}	
	lineNumber++;
	videoData.currentLineNumber = lineNumber;
	REG_EVSYS_INTFLAG = EVSYS_INTFLAG_EVD0;
	NVIC_ClearPendingIRQ(EVSYS_IRQn);	
	asm ("cpsie i\n\t");
}
/*
	waitForVerticalBlank().
	It halts the system (except interrupt) until the screen drawing has been completed. Notice that the screen drawing ends when the line is set to 400,
	but when this occurs, after very few cycles the TCC0_handler is called again for the next line. Since next line is black, the current line number
	is incremented immediately to 401, and there it remains for more than 30 us.
*/
void waitForVerticalBlank(void)
{
	while (videoData.currentLineNumber != 401);	
}
#if GFX_MODE != TILE_MODE_BOOT
inline uint8_t checkForNewFrame(void)
{
	/* Note! We *could* have used this code:
		uint8_t vbl;
		__set_PRIMASK(1);
		vbl = videoData.verticalBlank;
		videoData.verticalBlank = 0;
		__set_PRIMASK(0);
		return vbl;
		//
		However, if, by chance it is executed just before the beginning of the timer interrupt, just when we should draw the frame, then the interrupt would be delayed of at least 8 clock cycles.
		This delay is not predictable so we should delay the event by at least this amount of time, for reliably produce a frame.    
	*/
	static uint8_t oldFrame = 0;
	uint8_t currentFrame = videoData.currentFrame;
	if (oldFrame != currentFrame)
	{
		oldFrame = currentFrame;
		return 1;
	}
	return 0;
}
#endif
#if GFX_MODE != BITMAPPED_MODE && SPRITES_ENABLED
/*
	putSprite(). Reserves a sprite in the sprite array for the next drawSprites() request.
	num is the sprite index we want to draw. Note that:
		- for sprites with flags SPRITE_FLAGS_PRIORITY_ALWAYS_BOTTOM, higher sprite number means lower priority, therefore they are drawn under other the other sprites with lower index.
		- All the other sprite have opposite priority, i.e. larger index sprites will be drawn on top of lower indexed sprites.
	x and y coordinate behavior depend on which static switches are active and which flags have the sprites.
		- if PER_LINE_X_SCROLL == 0 or PER_TILE_X_SCROLL == 0, then videodata.xScroll is available (otherwise is implicitly considered as if it were 0).
		Here is a table that summarizes what happens depending on the flags.
		SPRITE_FLAGS_PER_TILE_X_SCROLL_DEFORMATION		SPRITE_FLAGS_NO_X_SCROLL_CORRECTION			Result
							0										0								X value of the sprite represents the physical screen coordinate  (i.e. x = 0 => leftmost pixel, x = 319 => rightmost)
							0										1								Different values on videoData.ptrTileXScroll[line] won't produce shape deformation, but the xScroll is not corrected. For those lines for which values on videoData.ptrTileXScroll[line] == 0, the x represents the logical (not physical) coordinate.
							1										0								Different values on videoData.ptrTileXScroll[line] will produce shape deformation, but the xScroll value is corrected. For those lines for which values on videoData.ptrTileXScroll[line] == 0, the x represents the physical screen coordinate.
							1										1								The x value represents the logical coordinate in the map.
		 When PER_LINE_X_SCROLL switch is set to 1, deformation always occur if videoData.ptrRowXScroll[] contains different values. 
		For the y value, only the flag SPRITE_FLAGS_NO_Y_SCROLL_CORRECTION is available. When set, the y represents the logical coordinate, when not set, it represents the physical (screen) value.
	flags: Flags can be used to determine the handle point of the sprite (i.e. where is the 0,0 pixel of the sprite), the transparency, the rotation and the priority.
	return value: if the sprite was within the visible area and there were enough sprite slots available 1, otherwise 0.
*/
uint32_t putSprite(uint16_t num, int32_t x, int32_t y, uint16_t flags, uint16_t frameNum)
{
	#if GFX_MODE == TILE_MODE0 || GFX_MODE == TILE_MODE1 || GFX_MODE == TILE_MODE2
	if (num >= MAX_ONSCREEN_SPRITES)
		return 0;
	#if GFX_MODE == TILE_MODE2 
		// to increase drawing speed, we do no actually perform single pixel blit. Instead we store 2 copies of each sprite, one shifted by 1 pixel.
		// furthermore sprites should always have a width multiple of two, in every case.
		frameNum = (frameNum << 1);
	#endif
	int32_t handlex, handley;
	uint8_t flagH =   flags & SPRITE_FLAGS_FLIP_HORIZONTAL;
	uint8_t flagV =   flags & SPRITE_FLAGS_FLIP_VERTICAL ;
	const frame_t *pFrame = &frameData[frameNum];
	switch (flags & 0xF000)
	{
		case SPRITE_FLAGS_HANDLE_TOPLEFT:
			handlex = - (flagH ? pFrame->w : 0);
			handley = - (flagV ? pFrame->h : 0);
			break;
		case SPRITE_FLAGS_HANDLE_TOPRIGHT:
			handlex = (flagH ? 0 : (- pFrame->w + 1)) ;
			handley = - (flagV ? pFrame->h : 0);
			break;
		case SPRITE_FLAGS_HANDLE_BOTTOMLEFT:
			handlex = - (flagH ? pFrame->w : 0);
			handley = (flagV ? 0 : (- pFrame->h + 1));
			break;
		case SPRITE_FLAGS_HANDLE_BOTTOMRIGHT:
			handlex = (flagH ? 0 : (- pFrame->w + 1)) ;
			handley = (flagV ? 0 : (- pFrame->h + 1));
			break;
		default:
		case SPRITE_FLAGS_HANDLE_CENTER:
			handlex  = (flagH ? (pFrame->ox - pFrame->w) : -pFrame->ox);
			handley = (flagV ? (pFrame->oy - pFrame->h) : -pFrame->oy);		
			break;							
		case SPRITE_FLAGS_HANDLE_CENTER_TOP:
			handlex = (flagH ? (pFrame->ox - pFrame->w) : -pFrame->ox);
			handley = - (flagV ? pFrame->h : 0);
			break;
		case SPRITE_FLAGS_HANDLE_CENTER_BOTTOM:
			handlex = (flagH ? (pFrame->ox - pFrame->w) : -pFrame->ox);
			handley = (flagV ? 0 : (- pFrame->h + 1));
			break;
		// TODO: center left - center right		
	}
#if GFX_MODE != TILE_MODE2
	if (flags & SPRITE_FLAGS_INVERTXY) 
	{
		x += handley;
		y += handlex;
		if ( (x  >= VRAMX * TILE_SIZE_X) || (y >= VRAMY * TILE_SIZE_Y) || ((x + pFrame->h) <= 0) || (y + pFrame->w) <= 0) // do not include sprites that are completely outside the draw region.
			return 0;
		usvcSprites[num].x = x;
		usvcSprites[num].y = y;		
	}
	else
#endif
	{
		x += handlex;
		y += handley;
			if ( (x  >= VRAMX * TILE_SIZE_X) || (y >= VRAMY * TILE_SIZE_Y) || ((x + pFrame->w) <= 0) || (y + pFrame->h) <= 0) // do not include sprites that are completely outside the draw region.
			return 0;
		usvcSprites[num].x = x;
		usvcSprites[num].y = y;		
	}
	usvcSprites[num].flags = flags & 0xFFF;
	usvcSprites[num].frameNum = frameNum;
	return 1;
		// Note: actually in the sprite drawing routine, the sprite could be still shifted. So we just send the even sprite number and let the routine to decide which sprite we should actually write
		// (this is true when there is a per-tile scroll and the sprite has FLAG_ABSOLUTE_X_VALUE set.
#endif
}
// removeSprites()
// removes the indicated sprite index from the array so that it won't be drawn.
void removeSprite(uint16_t num)
{
	usvcSprites[num].frameNum = 0xFFFF;
}
/* removeAllSprites()
*	removes all the sprites from the sprite array, and optionally, if the parameters is non 0, it redraws the map, by restoring the original tile indexes.
*/
void removeAllSprites(uint8_t redrawScreen)
{
	for (int i = 0; i < MAX_ONSCREEN_SPRITES; i++)
	{
		usvcSprites[i].frameNum = 0xFFFF;	
	}
	if (redrawScreen && !videoData.spriteTilesRemoved)		// if we want to redraw the screen after the sprites have been removed, without actually redraw the map on vram.
	{
		restoreBackgroundTiles();
	}		
}
/* restoreBackgroundTiles()
*	redraws the map, but it does not remove sprites from the array.	
*/
void restoreBackgroundTiles(void)
{
	for (int i = 0; i < MAX_TEMP_SPRITE_TILES; i++)
	{
		uint16_t idx = spriteTiles[i].originalVramIndex;
		if ( idx != 0xFFFF)
		{
			*((uint16_t*)(&vram[idx])) = spriteTiles[i].originalTileLoAddr;
			spriteTiles[i].originalVramIndex = 0xFFFF;	// free tile
		}
	}	
	videoData.spriteTilesRemoved = 1;			// we set spriteTilesRemoved, so that if the user calls drawSprites, then it does not try to restore the tiles.
}
/* freeSpriteTiles()
* quickly deallocateAllSpriteTiles by writing 0x0000FFFF on them
*/
void freeSpriteTiles()
{
	uint32_t *p = (uint32_t*) spriteTiles;
	for (int i = 0; i < sizeof (spriteTiles) / sizeof (spriteTile_t); i++)
	{
		*p++ = 0x0000FFFF;	// this will reset both originalTileLoAddr and  originalVramIndex
	}
	videoData.spriteTilesRemoved = 1;				
}
/* drawFixedSection()
*  Redraws the fixed section by translating the fixedSectionMap to the vram, at the right position.
*/
void drawFixedSection()
{
	#if USE_SECTION == TOP_FIXED_SECTION
		for (int y = 0; y < SECTION_LIMIT; y++)
		{
			for (int x = 0; x <SCREEN_SIZE_X / TILE_SIZE_X; x++)		
			{
				*((uint16_t*)(&vram[x + y * VRAMX])) = (uint32_t) &fixedSectionTiles[fixedSectionMap[(SCREEN_SIZE_X / TILE_SIZE_X) * y + x]][0];
			}
		}
	#elif USE_SECTION == BOTTOM_FIXED_SECTION
		for (int y = SECTION_LIMIT + 1; y < VRAMY ; y++)
		{
			for (int x = 0; x < SCREEN_SIZE_X / TILE_SIZE_X; x++)
			{
				*((uint16_t*)(&vram[x + y * VRAMX])) = (uint32_t) &fixedSectionTiles[fixedSectionMap[(SCREEN_SIZE_X / TILE_SIZE_X) * (y  - (SECTION_LIMIT + 1)) + x]][0];
			}
		}
	#endif
}
#if (GFX_MODE == TILE_MODE1 || GFX_MODE == TILE_MODE2) && SPRITES_ENABLED
#if (GFX_MODE == TILE_MODE1)
// private fast drawing routines for tile mode 1.
inline void drawSpriteOnTopOfTile(int32_t y, uint8_t *pp, uint8_t* dest, int32_t vi, int32_t deltaX, int32_t horizontalIncrement)
{
	uint32_t rt0, rt1, rt2;	// temporary registers
	asm volatile
	(
		// First, get the address which determines how many pixels need to be drawn).
		"MOV %[rt0], %[rDrawNextSpriteLine]\n\t"		// get deltaX
		"LSL %[rt2],%[rt0],#2\n\t"						// compute the correct offset (addresses are 4-byte aligned)
		"ADR %[rt1],drawTable%=\n\t"					// get the table address
		"LDR %[rt2],[%[rt1], %[rt2]]\n\t"				// get the correct entry 
		"MOV %[rDrawNextSpriteLine],%[rt2]\n\t"			// save the entry to a high register
		// now we must compute the horizontal increment.
		"MOV  %[rt2],%[horizontalIncrement]\n\t"	// get horizontal increment
		"MUL %[rt2],%[rt0]\n\t"						// multiply the horizontal increment by deltaX
		"SUB %[rt2], %[rt2],%[horizontalIncrement]\n\t"	// subtract the horizontal increment
		"MOV %[rt1], %[rt2]\n\t"					// this is a copy of the computed offset, as it must be restored at the end of each line.
		"BX %[rDrawNextSpriteLine]\n\t"
		".align(2)\n\t"
		"drawTable%=:\n\t"
			".word byte0%= + 1\n\t"		// deltaX can' be 0 actually...
			".word byte0%= + 1\n\t"
			".word byte1%= + 1\n\t"
			".word byte2%= + 1\n\t"
			".word byte3%= + 1\n\t"
			".word byte4%= + 1\n\t"
			".word byte5%= + 1\n\t"
			".word byte6%= + 1\n\t"
			".word byte7%= + 1\n\t"
		"byte7%=:\n\t"
			"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
			"SUB %[rt2], %[horizontalIncrement]\n\t"	// subtract the horizontal increment
			"TST %[rt0],%[rt0]\n\t"							// test if source pix is non zero
			"BEQ byte6%=\n\t"								// if yes, go to next pixel
			"STRB %[rt0], [%[dest], #7]\n\t"				// otherwise write it
		"byte6%=:\n\t"
			"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
			"SUB %[rt2], %[horizontalIncrement]\n\t"	// subtract the horizontal increment
			"TST %[rt0],%[rt0]\n\t"							// test if source pix is non zero
			"BEQ byte5%=\n\t"								// if yes, go to next pixel
			"STRB %[rt0], [%[dest], #6]\n\t"				// otherwise write it
		"byte5%=:\n\t"
			"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
			"SUB %[rt2], %[horizontalIncrement]\n\t"	// subtract the horizontal increment
			"TST %[rt0],%[rt0]\n\t"							// test if source pix is non zero
			"BEQ byte4%=\n\t"								// if yes, go to next pixel
			"STRB %[rt0], [%[dest], #5]\n\t"				// otherwise write it
		"byte4%=:\n\t"
			"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
			"SUB %[rt2], %[horizontalIncrement]\n\t"	// subtract the horizontal increment
			"TST %[rt0],%[rt0]\n\t"							// test if source pix is non zero
			"BEQ byte3%=\n\t"								// if yes, go to next pixel
			"STRB %[rt0], [%[dest], #4]\n\t"				// otherwise write it
		"byte3%=:\n\t"
			"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
			"SUB %[rt2], %[horizontalIncrement]\n\t"	// subtract the horizontal increment
			"TST %[rt0],%[rt0]\n\t"							// test if source pix is non zero
			"BEQ byte2%=\n\t"								// if yes, go to next pixel
			"STRB %[rt0], [%[dest], #3]\n\t"				// otherwise write it
		"byte2%=:\n\t"
			"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
			"SUB %[rt2], %[horizontalIncrement]\n\t"	// subtract the horizontal increment
			"TST %[rt0],%[rt0]\n\t"							// test if source pix is non zero
			"BEQ byte1%=\n\t"								// if yes, go to next pixel
			"STRB %[rt0], [%[dest], #2]\n\t"				// otherwise write it
		"byte1%=:\n\t"
			"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
			"SUB %[rt2], %[horizontalIncrement]\n\t"	// subtract the horizontal increment
			"TST %[rt0],%[rt0]\n\t"							// test if source pix is non zero
			"BEQ byte0%=\n\t"								// if yes, go to next pixel
			"STRB %[rt0], [%[dest], #1]\n\t"				// otherwise write it
		"byte0%=:\n\t"
			"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
			"TST %[rt0],%[rt0]\n\t"							// test if source pix is non zero
			"BEQ endPixels%=\n\t"								// if yes, go to next pixel
			"STRB %[rt0], [%[dest], #0]\n\t"				// otherwise write it
		// we have drawn one line.
		"endPixels%=:\n\t"
		// now we need to restore rt2
		"MOV %[rt2],%[rt1]\n\t"
		// change y
		"ADD %[dest],#8\n\t"
		"ADD %[pp],%[vi]\n\t"
		"SUB %[y],#1\n\t"
		"BEQ end%=\n\t"
		"BX %[rDrawNextSpriteLine]\n\t"
		"end%=:\n\t"
		: [rt0] "=&l" (rt0) , [rt1] "=&l" (rt1), [rt2] "=&l" (rt2), [rDrawNextSpriteLine] "+h" (deltaX)
		: [y] "l" (y), [dest] "l" (dest), [pp] "l" (pp),  [vi] "h" (vi), [horizontalIncrement] "l" (horizontalIncrement)     // inputs
		:  // clobber list
	);	
}
#if ENABLE_TRANSPARENT_SPRITES
inline void drawTransparentSpriteOnTopOfTile(int32_t y, uint8_t *pp, uint8_t* dest, int32_t vi, int32_t deltaX, int32_t horizontalIncrement)
{
	uint32_t rt0, rt1, rt2, rth;	// temporary registers
	asm volatile
	(
	// First, get the address which determines how many pixels need to be drawn).
	"MOV %[rt0], %[rDrawNextSpriteLine]\n\t"		// get deltaX
	"LSL %[rt2],%[rt0],#2\n\t"						// compute the correct offset (addresses are 4-byte aligned)
	"ADR %[rt1],drawTable%=\n\t"					// get the table address
	"LDR %[rt2],[%[rt1], %[rt2]]\n\t"				// get the correct entry
	"MOV %[rDrawNextSpriteLine],%[rt2]\n\t"			// save the entry to a high register
	// now we must compute the horizontal increment.
	"MOV  %[rt2],%[horizontalIncrement]\n\t"	// get horizontal increment
	"MUL %[rt2],%[rt0]\n\t"						// multiply the horizontal increment by deltaX
	"MOV %[rth], %[rt2]\n\t"					// this is a copy of the computed offset, as it must be restored at the end of each line.
	"BX %[rDrawNextSpriteLine]\n\t"
	".align(2)\n\t"
	"drawTable%=:\n\t"
	".word byte0%= + 1\n\t"		// deltaX can' be 0 actually...
	".word byte0%= + 1\n\t"
	".word byte1%= + 1\n\t"
	".word byte2%= + 1\n\t"
	".word byte3%= + 1\n\t"
	".word byte4%= + 1\n\t"
	".word byte5%= + 1\n\t"
	".word byte6%= + 1\n\t"
	".word byte7%= + 1\n\t"
	"byte7%=:\n\t"
		"SUB %[rt2], %[rt2],%[horizontalIncrement]\n\t"	// subtract the horizontal increment
		"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
		"TST %[rt0],%[rt0]\n\t"							// test if source pix is non zero
		"BEQ byte6%=\n\t"								// if yes, go to next pixel
		// we need to get transparentSpriteColorTable[pix][*dest], where pix is rt0
		"LSL %[rt0], #8\n\t"
		"LDRB %[rt1], [%[dest], #7]\n\t"
		"ADD %[rt0], %[rt1]\n\t"
		"LDRB %[rt0],[%[transparentSpriteColorTable], %[rt0]]\n\t"
		"STRB %[rt0], [%[dest], #7]\n\t"				// otherwise write it
	"byte6%=:\n\t"
		"SUB %[rt2], %[rt2],%[horizontalIncrement]\n\t"	// subtract the horizontal increment
		"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
		"TST %[rt0],%[rt0]\n\t"							// test if source pix is non zero
		"BEQ byte5%=\n\t"								// if yes, go to next pixel
		// we need to get transparentSpriteColorTable[pix][*dest], where pix is rt0
		"LSL %[rt0], #8\n\t"
		"LDRB %[rt1], [%[dest], #6]\n\t"
		"ADD %[rt0], %[rt1]\n\t"
		"LDRB %[rt0],[%[transparentSpriteColorTable], %[rt0]]\n\t"
		"STRB %[rt0], [%[dest], #6]\n\t"				// otherwise write it
	"byte5%=:\n\t"
		"SUB %[rt2], %[rt2],%[horizontalIncrement]\n\t"	// subtract the horizontal increment
		"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
		"TST %[rt0],%[rt0]\n\t"							// test if source pix is non zero
		"BEQ byte4%=\n\t"								// if yes, go to next pixel
		// we need to get transparentSpriteColorTable[pix][*dest], where pix is rt0
		"LSL %[rt0], #8\n\t"
		"LDRB %[rt1], [%[dest], #5]\n\t"
		"ADD %[rt0], %[rt1]\n\t"
		"LDRB %[rt0],[%[transparentSpriteColorTable], %[rt0]]\n\t"
		"STRB %[rt0], [%[dest], #5]\n\t"				// otherwise write it
	"byte4%=:\n\t"
		"SUB %[rt2], %[rt2],%[horizontalIncrement]\n\t"	// subtract the horizontal increment
		"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
		"TST %[rt0],%[rt0]\n\t"							// test if source pix is non zero
		"BEQ byte3%=\n\t"								// if yes, go to next pixel
		// we need to get transparentSpriteColorTable[pix][*dest], where pix is rt0
		"LSL %[rt0], #8\n\t"
		"LDRB %[rt1], [%[dest], #4]\n\t"
		"ADD %[rt0], %[rt1]\n\t"
		"LDRB %[rt0],[%[transparentSpriteColorTable], %[rt0]]\n\t"
		"STRB %[rt0], [%[dest], #4]\n\t"				// otherwise write it
	"byte3%=:\n\t"
		"SUB %[rt2], %[rt2],%[horizontalIncrement]\n\t"	// subtract the horizontal increment
		"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
		"TST %[rt0],%[rt0]\n\t"							// test if source pix is non zero
		"BEQ byte2%=\n\t"								// if yes, go to next pixel
		// we need to get transparentSpriteColorTable[pix][*dest], where pix is rt0
		"LSL %[rt0], #8\n\t"
		"LDRB %[rt1], [%[dest], #3]\n\t"
		"ADD %[rt0], %[rt1]\n\t"
		"LDRB %[rt0],[%[transparentSpriteColorTable], %[rt0]]\n\t"
		"STRB %[rt0], [%[dest], #3]\n\t"				// otherwise write it
	"byte2%=:\n\t"
		"SUB %[rt2], %[rt2],%[horizontalIncrement]\n\t"	// subtract the horizontal increment
		"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
		"TST %[rt0],%[rt0]\n\t"							// test if source pix is non zero
		"BEQ byte1%=\n\t"								// if yes, go to next pixel
		// we need to get transparentSpriteColorTable[pix][*dest], where pix is rt0
		"LSL %[rt0], #8\n\t"
		"LDRB %[rt1], [%[dest], #2]\n\t"
		"ADD %[rt0], %[rt1]\n\t"
		"LDRB %[rt0],[%[transparentSpriteColorTable], %[rt0]]\n\t"
		"STRB %[rt0], [%[dest], #2]\n\t"				// otherwise write it
	"byte1%=:\n\t"
		"SUB %[rt2], %[rt2],%[horizontalIncrement]\n\t"	// subtract the horizontal increment
		"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
		"TST %[rt0],%[rt0]\n\t"							// test if source pix is non zero
		"BEQ byte0%=\n\t"								// if yes, go to next pixel
		// we need to get transparentSpriteColorTable[pix][*dest], where pix is rt0
		"LSL %[rt0], #8\n\t"
		"LDRB %[rt1], [%[dest], #1]\n\t"
		"ADD %[rt0], %[rt1]\n\t"
		"LDRB %[rt0],[%[transparentSpriteColorTable], %[rt0]]\n\t"
		"STRB %[rt0], [%[dest], #1]\n\t"				// otherwise write it
	"byte0%=:\n\t"
		"SUB %[rt2], %[rt2],%[horizontalIncrement]\n\t"	// subtract the horizontal increment
		"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
		"TST %[rt0],%[rt0]\n\t"							// test if source pix is non zero
		"BEQ endPixels%=\n\t"								// if yes, go to next pixel
		// we need to get transparentSpriteColorTable[pix][*dest], where pix is rt0
		"LSL %[rt0], #8\n\t"
		"LDRB %[rt1], [%[dest], #0]\n\t"
		"ADD %[rt0], %[rt1]\n\t"
		"LDRB %[rt0],[%[transparentSpriteColorTable], %[rt0]]\n\t"
		"STRB %[rt0], [%[dest], #0]\n\t"				// otherwise write it
	// we have drawn one line.
	"endPixels%=:\n\t"
	// now we need to restore rt2
	"MOV %[rt2],%[rth]\n\t"
	// change y
	"ADD %[dest],#8\n\t"
	"ADD %[pp],%[vi]\n\t"
	"SUB %[y],#1\n\t"
	"BEQ end%=\n\t"
	"BX %[rDrawNextSpriteLine]\n\t"
	"end%=:\n\t"
	: [rt0] "=&l" (rt0) , [rt1] "=&l" (rt1), [rt2] "=&l" (rt2), [rth] "=&h" (rth), [rDrawNextSpriteLine] "+h" (deltaX)
	: [y] "l" (y), [dest] "l" (dest), [pp] "l" (pp),  [vi] "h" (vi), 
		[horizontalIncrement] "l" (horizontalIncrement), [transparentSpriteColorTable] "l" (transparentSpriteColorTable)    // inputs
	:  // clobber list
	);
}
inline void drawTransparentSpriteUnderTile(int32_t y, uint8_t *pp, uint8_t* dest, int32_t vi, int32_t deltaX, int32_t horizontalIncrement)
{
	uint32_t rt0, rt1, rt2, rth;	// temporary registers
	asm volatile
	(
	// First, get the address which determines how many pixels need to be drawn).
	"MOV %[rt0], %[rDrawNextSpriteLine]\n\t"		// get deltaX
	"LSL %[rt2],%[rt0],#2\n\t"						// compute the correct offset (addresses are 4-byte aligned)
	"ADR %[rt1],drawTable%=\n\t"					// get the table address
	"LDR %[rt2],[%[rt1], %[rt2]]\n\t"				// get the correct entry
	"MOV %[rDrawNextSpriteLine],%[rt2]\n\t"			// save the entry to a high register
	// now we must compute the horizontal increment.
	"MOV  %[rt2],%[horizontalIncrement]\n\t"	// get horizontal increment
	"MUL %[rt2],%[rt0]\n\t"						// multiply the horizontal increment by deltaX
	"MOV %[rth], %[rt2]\n\t"					// this is a copy of the computed offset, as it must be restored at the end of each line.
	"BX %[rDrawNextSpriteLine]\n\t"
	".align(2)\n\t"
	"drawTable%=:\n\t"
	".word byte0%= + 1\n\t"		// deltaX can' be 0 actually...
	".word byte0%= + 1\n\t"
	".word byte1%= + 1\n\t"
	".word byte2%= + 1\n\t"
	".word byte3%= + 1\n\t"
	".word byte4%= + 1\n\t"
	".word byte5%= + 1\n\t"
	".word byte6%= + 1\n\t"
	".word byte7%= + 1\n\t"
	"byte7%=:\n\t"
		"SUB %[rt2], %[rt2],%[horizontalIncrement]\n\t"	// subtract the horizontal increment
		"LDRB %[rt0], [%[dest], #7]\n\t"	// get destination pixel 
		"TST %[rt0],%[rt0]\n\t"							// test if dest pix is zero
		"BNE byte6%=\n\t"								// if non zero, go to next pixel
		// we need transparentSpriteColorTable[pix][0], where pix is rt0
		"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
		"LSL %[rt0], #8\n\t"
		"LDRB %[rt0],[%[transparentSpriteColorTable], %[rt0]]\n\t"
		"STRB %[rt0], [%[dest], #7]\n\t"				// otherwise write it
	"byte6%=:\n\t"
		"SUB %[rt2], %[rt2],%[horizontalIncrement]\n\t"	// subtract the horizontal increment
		"LDRB %[rt0], [%[dest], #6]\n\t"	// get destination pixel
		"TST %[rt0],%[rt0]\n\t"							// test if dest pix is zero
		"BNE byte5%=\n\t"								// if non zero, go to next pixel
		// we need transparentSpriteColorTable[pix][0], where pix is rt0
		"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
		"LSL %[rt0], #8\n\t"
		"LDRB %[rt0],[%[transparentSpriteColorTable], %[rt0]]\n\t"
		"STRB %[rt0], [%[dest], #6]\n\t"				// otherwise write it
	"byte5%=:\n\t"
		"SUB %[rt2], %[rt2],%[horizontalIncrement]\n\t"	// subtract the horizontal increment
		"LDRB %[rt0], [%[dest], #5]\n\t"	// get destination pixel
		"TST %[rt0],%[rt0]\n\t"							// test if dest pix is zero
		"BNE byte4%=\n\t"								// if non zero, go to next pixel
		// we need transparentSpriteColorTable[pix][0], where pix is rt0
		"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
		"LSL %[rt0], #8\n\t"
		"LDRB %[rt0],[%[transparentSpriteColorTable], %[rt0]]\n\t"
		"STRB %[rt0], [%[dest], #5]\n\t"				// otherwise write it
	"byte4%=:\n\t"
		"SUB %[rt2], %[rt2],%[horizontalIncrement]\n\t"	// subtract the horizontal increment
		"LDRB %[rt0], [%[dest], #4]\n\t"	// get destination pixel
		"TST %[rt0],%[rt0]\n\t"							// test if dest pix is zero
		"BNE byte3%=\n\t"								// if non zero, go to next pixel
		// we need transparentSpriteColorTable[pix][0], where pix is rt0
		"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
		"LSL %[rt0], #8\n\t"
		"LDRB %[rt0],[%[transparentSpriteColorTable], %[rt0]]\n\t"
		"STRB %[rt0], [%[dest], #4]\n\t"				// otherwise write it
	"byte3%=:\n\t"
		"SUB %[rt2], %[rt2],%[horizontalIncrement]\n\t"	// subtract the horizontal increment
		"LDRB %[rt0], [%[dest], #3]\n\t"	// get destination pixel
		"TST %[rt0],%[rt0]\n\t"							// test if dest pix is zero
		"BNE byte2%=\n\t"								// if non zero, go to next pixel
		// we need transparentSpriteColorTable[pix][0], where pix is rt0
		"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
		"LSL %[rt0], #8\n\t"
		"LDRB %[rt0],[%[transparentSpriteColorTable], %[rt0]]\n\t"
		"STRB %[rt0], [%[dest], #3]\n\t"				// otherwise write it
	"byte2%=:\n\t"
		"SUB %[rt2], %[rt2],%[horizontalIncrement]\n\t"	// subtract the horizontal increment
		"LDRB %[rt0], [%[dest], #2]\n\t"	// get destination pixel
		"TST %[rt0],%[rt0]\n\t"							// test if dest pix is zero
		"BNE byte1%=\n\t"								// if non zero, go to next pixel
		// we need transparentSpriteColorTable[pix][0], where pix is rt0
		"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
		"LSL %[rt0], #8\n\t"
		"LDRB %[rt0],[%[transparentSpriteColorTable], %[rt0]]\n\t"
		"STRB %[rt0], [%[dest], #2]\n\t"				// otherwise write it
	"byte1%=:\n\t"
		"SUB %[rt2], %[rt2],%[horizontalIncrement]\n\t"	// subtract the horizontal increment
		"LDRB %[rt0], [%[dest], #1]\n\t"	// get destination pixel
		"TST %[rt0],%[rt0]\n\t"							// test if dest pix is zero
		"BNE byte0%=\n\t"								// if non zero, go to next pixel
		// we need transparentSpriteColorTable[pix][0], where pix is rt0
		"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
		"LSL %[rt0], #8\n\t"
		"LDRB %[rt0],[%[transparentSpriteColorTable], %[rt0]]\n\t"
		"STRB %[rt0], [%[dest], #1]\n\t"				// otherwise write it
	"byte0%=:\n\t"
		"SUB %[rt2], %[rt2],%[horizontalIncrement]\n\t"	// subtract the horizontal increment
		"LDRB %[rt0], [%[dest], #0]\n\t"	// get destination pixel
		"TST %[rt0],%[rt0]\n\t"							// test if dest pix is zero
		"BNE endPixels%=\n\t"								// if non zero, go to next pixel
		// we need transparentSpriteColorTable[pix][0], where pix is rt0
		"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
		"LSL %[rt0], #8\n\t"
		"LDRB %[rt0],[%[transparentSpriteColorTable], %[rt0]]\n\t"
		"STRB %[rt0], [%[dest], #0]\n\t"				// otherwise write it
	// we have drawn one line.
	"endPixels%=:\n\t"
	// now we need to restore rt2
	"MOV %[rt2],%[rth]\n\t"
	// change y
	"ADD %[dest],#8\n\t"
	"ADD %[pp],%[vi]\n\t"
	"SUB %[y],#1\n\t"
	"BEQ end%=\n\t"
	"BX %[rDrawNextSpriteLine]\n\t"
	"end%=:\n\t"
	: [rt0] "=&l" (rt0) , [rt1] "=&l" (rt1), [rt2] "=&l" (rt2), [rth] "=&h" (rth), [rDrawNextSpriteLine] "+h" (deltaX)
	: [y] "l" (y), [dest] "l" (dest), [pp] "l" (pp),  [vi] "h" (vi), 
		[horizontalIncrement] "l" (horizontalIncrement), [transparentSpriteColorTable] "l" (transparentSpriteColorTable)    // inputs
	:  // clobber list
	);
}
#endif
inline void drawSpriteUnderTile(int32_t y, uint8_t *pp, uint8_t* dest, int32_t vi, int32_t deltaX, int32_t horizontalIncrement)
{
	uint32_t rt0, rt1, rt2;	// temporary registers
	asm volatile
	(
		// First, get the address which determines how many pixels need to be drawn).
		"MOV %[rt0], %[rDrawNextSpriteLine]\n\t"		// get deltaX
		"LSL %[rt2],%[rt0],#2\n\t"						// compute the correct offset (addresses are 4-byte aligned)
		"ADR %[rt1],drawTable%=\n\t"					// get the table address
		"LDR %[rt2],[%[rt1], %[rt2]]\n\t"				// get the correct entry 
		"MOV %[rDrawNextSpriteLine],%[rt2]\n\t"			// save the entry to a high register
		// now we must compute the horizontal increment.
		"MOV  %[rt2],%[horizontalIncrement]\n\t"	// get horizontal increment
		"MUL %[rt2],%[rt0]\n\t"						// multiply the horizontal increment by deltaX
//		"SUB %[rt2], %[rt2],%[horizontalIncrement]\n\t"	// subtract the horizontal increment
		"MOV %[rt1], %[rt2]\n\t"					// this is a copy of the computed offset, as it must be restored at the end of each line.
		"BX %[rDrawNextSpriteLine]\n\t"
		".align(2)\n\t"
		"drawTable%=:\n\t"
			".word byte0%= + 1\n\t"		// deltaX can' be 0 actually...
			".word byte0%= + 1\n\t"
			".word byte1%= + 1\n\t"
			".word byte2%= + 1\n\t"
			".word byte3%= + 1\n\t"
			".word byte4%= + 1\n\t"
			".word byte5%= + 1\n\t"
			".word byte6%= + 1\n\t"
			".word byte7%= + 1\n\t"
		"byte7%=:\n\t"
			"SUB %[rt2], %[horizontalIncrement]\n\t"	// subtract the horizontal increment
			"LDRB %[rt0], [%[dest], #7]\n\t"				// get  destination pixel.
			"TST %[rt0],%[rt0]\n\t"							// test if dest is non zero
			"BNE byte6%=\n\t"								// if it is non zero, go to next pixel
			"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
			"STRB %[rt0], [%[dest], #7]\n\t"				// otherwise write it
		"byte6%=:\n\t"
			"SUB %[rt2], %[horizontalIncrement]\n\t"	// subtract the horizontal increment
			"LDRB %[rt0], [%[dest], #6]\n\t"				// get  destination pixel.
			"TST %[rt0],%[rt0]\n\t"							// test if dest is non zero
			"BNE byte5%=\n\t"								// if it is non zero, go to next pixel
			"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
			"STRB %[rt0], [%[dest], #6]\n\t"				// otherwise write it
		"byte5%=:\n\t"
			"SUB %[rt2], %[horizontalIncrement]\n\t"	// subtract the horizontal increment
			"LDRB %[rt0], [%[dest], #5]\n\t"				// get  destination pixel.
			"TST %[rt0],%[rt0]\n\t"							// test if dest is non zero
			"BNE byte4%=\n\t"								// if it is non zero, go to next pixel
			"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
			"STRB %[rt0], [%[dest], #5]\n\t"				// otherwise write it
		"byte4%=:\n\t"
			"SUB %[rt2], %[horizontalIncrement]\n\t"	// subtract the horizontal increment
			"LDRB %[rt0], [%[dest], #4]\n\t"				// get  destination pixel.
			"TST %[rt0],%[rt0]\n\t"							// test if dest is non zero
			"BNE byte3%=\n\t"								// if it is non zero, go to next pixel
			"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
			"STRB %[rt0], [%[dest], #4]\n\t"				// otherwise write it
		"byte3%=:\n\t"
			"SUB %[rt2], %[horizontalIncrement]\n\t"	// subtract the horizontal increment
			"LDRB %[rt0], [%[dest], #3]\n\t"				// get  destination pixel.
			"TST %[rt0],%[rt0]\n\t"							// test if dest is non zero
			"BNE byte2%=\n\t"								// if it is non zero, go to next pixel
			"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
			"STRB %[rt0], [%[dest], #3]\n\t"				// otherwise write it
		"byte2%=:\n\t"
			"SUB %[rt2], %[horizontalIncrement]\n\t"	// subtract the horizontal increment
			"LDRB %[rt0], [%[dest], #2]\n\t"				// get  destination pixel.
			"TST %[rt0],%[rt0]\n\t"							// test if dest is non zero
			"BNE byte1%=\n\t"								// if it is non zero, go to next pixel
			"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
			"STRB %[rt0], [%[dest], #2]\n\t"				// otherwise write it
		"byte1%=:\n\t"
			"SUB %[rt2], %[horizontalIncrement]\n\t"	// subtract the horizontal increment
			"LDRB %[rt0], [%[dest], #1]\n\t"				// get  destination pixel.
			"TST %[rt0],%[rt0]\n\t"							// test if dest is non zero
			"BNE byte0%=\n\t"								// if it is non zero, go to next pixel
			"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
			"STRB %[rt0], [%[dest], #1]\n\t"				// otherwise write it
		"byte0%=:\n\t"
			"SUB %[rt2], %[horizontalIncrement]\n\t"	// subtract the horizontal increment
			"LDRB %[rt0], [%[dest], #0]\n\t"				// get  destination pixel.
			"TST %[rt0],%[rt0]\n\t"							// test if dest is non zero
			"BNE endPixels%=\n\t"								// if it is non zero, go to next pixel
			"LDRB %[rt0], [%[pp], %[rt2]]\n\t"  // get pixel data
			"STRB %[rt0], [%[dest], #0]\n\t"				// otherwise write it
		// we have drawn one line.
		"endPixels%=:\n\t"
		// now we need to restore rt2
		"MOV %[rt2],%[rt1]\n\t"
		// change y
		"ADD %[dest],#8\n\t"
		"ADD %[pp],%[vi]\n\t"
		"SUB %[y],#1\n\t"
		"BEQ end%=\n\t"
		"BX %[rDrawNextSpriteLine]\n\t"
		"end%=:\n\t"
		: [rt0] "=&l" (rt0) , [rt1] "=&l" (rt1), [rt2] "=&l" (rt2), [rDrawNextSpriteLine] "+h" (deltaX)
		: [y] "l" (y), [dest] "l" (dest), [pp] "l" (pp),  [vi] "h" (vi), [horizontalIncrement] "l" (horizontalIncrement)     // inputs
		:  // clobber list
	);
	
}

#endif
// private sprite drawing function.
inline uint32_t drawSpritesWithFlags(uint32_t usedSpriteTileNumber, uint32_t requiredSpriteFlags) __attribute__((always_inline));
inline uint32_t drawSpritesWithFlags(uint32_t usedSpriteTileNumber, uint32_t requiredSpriteFlags) 
{
#if GFX_MODE == TILE_MODE1
	for (int num = 0; num < MAX_ONSCREEN_SPRITES; num++)
	{
		usvcSprite_t *pSprite = &usvcSprites[num];
		if ((pSprite->flags & SPRITE_FLAGS_PRIORITY_MASK) != requiredSpriteFlags)
			continue;
		uint32_t nf = pSprite->frameNum;
		// is sprite active?
		if (nf != 0xFFFF)
		{
			// yes. Find out where it should go and reserve the tempTiles to draw on them.
			// we are going to determine a rectangle in the frame data, to be copied into tiles.
			int32_t xSpr;
#if PER_LINE_X_SCROLL == 0 && PER_TILE_X_SCROLL == 0
			if (pSprite->flags & SPRITE_FLAGS_NO_X_SCROLL_CORRECTION)
			{
				xSpr = pSprite->x;	
			}
			else
			{
				xSpr = pSprite->x + videoData.xScroll;
			}
#else
	#if PER_TILE_X_SCROLL == 0
			// if PER_TILE_X_SCROLL is 0 but PER_LINE_X_SCROLL is not, then we can still use xScroll
			if (pSprite->flags & SPRITE_FLAGS_NO_X_SCROLL_CORRECTION)
			{
				xSpr = pSprite->x;
			}
			else
			{
				xSpr = pSprite->x + videoData.xScroll;
			}
	#else
			xSpr = pSprite->x;			// optional correction will be applied later if PER_TILE_X_SCROLL is not 0. 
	#endif
#endif

			int32_t ySpr;
			if (pSprite->flags & SPRITE_FLAGS_NO_Y_SCROLL_CORRECTION)
			{
				ySpr = pSprite->y;
			}
			else
			{
				ySpr = pSprite->y + videoData.yScroll;
			}
			frame_t *pFrame = (frame_t*) &frameData[nf];
			int32_t wSpr = pFrame->w;
			int32_t hSpr = pFrame->h;
			//
			int32_t maxYsprite;
			int32_t minYsprite = ySpr;
			uint16_t flags = pSprite->flags & SPRITE_FLAGS_ROTATION_MASK;
			// boundary check
			int32_t maxXsprite;
			#if PER_TILE_X_SCROLL == 0
			int32_t minXsprite = xSpr;
			if (xSpr >= VRAMX * TILE_SIZE_X )
			{
				continue;		// sprite not visible
			}
			else if (xSpr < 0)
			{
				minXsprite = 0;
			}
			#endif
			if (ySpr >= VRAMY * TILE_SIZE_Y)
			{
				continue;		// sprite not visible
			}
			else if (ySpr < 0)
			{
				minYsprite = 0;
			}
			// in case we invert x and y (for 90 and 270° rotations), we must swap the boundaries.
			if (flags & SPRITE_FLAGS_INVERTXY)
			{
				maxXsprite = (xSpr + hSpr);
				maxYsprite = (ySpr + wSpr);
			}
			else
			{
				maxXsprite = (xSpr + wSpr);
				maxYsprite = (ySpr + hSpr);
			}
#if PER_TILE_X_SCROLL == 0
			if (maxXsprite < 0)
			{
				continue;		// sprite not visible
			}
			else if (maxXsprite > VRAMX * TILE_SIZE_X)
			{
				maxXsprite = VRAMX * TILE_SIZE_X;
			}
#endif
			if (maxYsprite < 0)
			{
				continue;		// sprite not visible
			}
			else if (maxYsprite > VRAMY * TILE_SIZE_Y)
			{
				// let's calculate how much the sprite is offscreen
				maxYsprite = VRAMY * TILE_SIZE_Y ;
			}
			// now, depending on the orientation, we must compute the increments we need to add to each pointer, as well as the pointer start location.
			const uint8_t *p =  0;				// these are initialized just to avoid warnings
			int horizontalIncrement = 0 ;	// these are initialized just to avoid warnings
			int verticalIncrement = 0;
			if (flags  == 0)
			{
				p = &pFrame->pData[0]; // point to the top left pixel.
				verticalIncrement  = wSpr;
				horizontalIncrement = 1;
			}
			else if  (  flags  == SPRITE_FLAGS_FLIP_VERTICAL)
			{
				p = &pFrame->pData[( hSpr - 1 ) * wSpr];
				verticalIncrement = -wSpr;
				horizontalIncrement = 1;
			}

			else if  (  flags  == SPRITE_FLAGS_FLIP_HORIZONTAL)
			{
				p = &pFrame->pData[wSpr - 1];
				verticalIncrement = wSpr;
				horizontalIncrement = -1;
			}
			else if  (  flags == (SPRITE_FLAGS_FLIP_HORIZONTAL | SPRITE_FLAGS_FLIP_VERTICAL ))
			{
				p = &pFrame->pData[( hSpr - 1) * wSpr + wSpr - 1];
				verticalIncrement = -wSpr;
				horizontalIncrement = -1;
			}
			else if (flags  == SPRITE_FLAGS_INVERTXY)
			{
				p = &pFrame->pData[0];
				verticalIncrement  =  1;
				horizontalIncrement = wSpr;
			}
			else if (flags  == SPRITE_FLAGS_ROTATE_90)
			{
				p = &pFrame->pData[wSpr - 1];
				verticalIncrement =  -1;
				horizontalIncrement = wSpr;
			}
			else if (flags  == SPRITE_FLAGS_INVERTXY180)
			{
				p = &pFrame->pData[( hSpr - 1 ) * wSpr + wSpr - 1 ];
				verticalIncrement =  -1;
				horizontalIncrement = -wSpr;
			}
			else if (flags  == SPRITE_FLAGS_ROTATE_270)
			{
				p = &pFrame->pData[( hSpr - 1 ) * wSpr];
				verticalIncrement =  1;
				horizontalIncrement = -wSpr;
			}
			// now, let's compute where, on the video screen, the sprite falls into
#if PER_TILE_X_SCROLL == 0
			int32_t tileXstart = minXsprite >> LOG2_TILE_SIZE_X;
			int32_t tileXstop = (maxXsprite - 1) >> LOG2_TILE_SIZE_X;
#endif
			int32_t tileYstart = minYsprite >> LOG2_TILE_SIZE_Y;
			int32_t tileYstop = (maxYsprite - 1) >> LOG2_TILE_SIZE_Y;
			uint32_t maxSpriteTilesAvailable = min ((MAX_TILES - videoData.ramTiles), MAX_TEMP_SPRITE_TILES);
			uint32_t *spriteTileStart = tiles[videoData.ramTiles];
			for (int yt = tileYstart; yt <= tileYstop; yt++)
			{
#if PER_TILE_X_SCROLL == 1
				uint8_t delta;
				int32_t maxXsprite2;
				if (pSprite->flags & SPRITE_FLAGS_PER_TILE_X_SCROLL_DEFORMATION)
				{
					#if PER_LINE_X_SCROLL == 0
						if (pSprite->flags & SPRITE_FLAGS_NO_X_SCROLL_CORRECTION)
						{
							delta = 0;
						}
						else
						{
							delta = videoData.xScroll;
						}
					#else
						// no xScroll available if both PER_TILE_X_SCROLL and PER_LINE_X_SCROLL are active.
						delta = 0;
					#endif
				}
				else
				{
					#if PER_LINE_X_SCROLL == 0
						if (pSprite->flags & SPRITE_FLAGS_NO_X_SCROLL_CORRECTION)
						{
							delta = videoData.ptrTileXScroll[yt];
						}
						else
						{
							delta = videoData.xScroll + videoData.ptrTileXScroll[yt];
						}					
					#else
						delta = videoData.ptrTileXScroll[yt];
					#endif
				}
				xSpr = pSprite->x + delta;
				maxXsprite2 = maxXsprite + delta;
				if (maxXsprite2 < 0)
				{
					continue;		// sprite not visible
				}
				else if (maxXsprite2 > VRAMX * TILE_SIZE_X)
				{
					maxXsprite2 = VRAMX * TILE_SIZE_X;
				}
				int32_t minXsprite = xSpr;
				if (xSpr >= VRAMX * TILE_SIZE_X )
				{
					continue;		// sprite not visible
				}
				else if (xSpr < 0)
				{
					minXsprite = 0;
				}
				int32_t tileXstart = minXsprite >> LOG2_TILE_SIZE_X;
				int32_t tileXstop = (maxXsprite2 - 1) >> LOG2_TILE_SIZE_X;
#endif
				int32_t minY = (yt * TILE_SIZE_Y < minYsprite) ? minYsprite : (yt * TILE_SIZE_Y);
				int32_t maxY = ((yt + 1) * TILE_SIZE_Y) > maxYsprite ? maxYsprite : ((yt + 1) * TILE_SIZE_Y);
				int32_t offsetY =   minY  - yt * TILE_SIZE_Y;
				for (int xt = tileXstart; xt <= tileXstop; xt++)
				{
					int32_t minX = (xt * TILE_SIZE_X < minXsprite) ? minXsprite : (xt * TILE_SIZE_X);		
#if PER_TILE_X_SCROLL == 0
					int32_t deltaX = ((xt + 1) * TILE_SIZE_X) > maxXsprite ? maxXsprite - minX : ((xt + 1) * TILE_SIZE_X) - minX;
#else
					int32_t deltaX = ((xt + 1) * TILE_SIZE_X) > maxXsprite2 ? maxXsprite2 - minX : ((xt + 1) * TILE_SIZE_X) - minX;
#endif
					int32_t offsetX =   minX - xt * TILE_SIZE_X;
					// now we are in one tile. Let's copy the sprite there the data (if any)
					int x, y;
					// let us first see if we are already writing on a sprite tile or if we need to reserve it first
					uint8_t *addr = (uint8_t*) ((vram[VRAMX * yt + xt]) | HIGH_ADDRESS);
					int32_t vi = verticalIncrement - (deltaX ) * horizontalIncrement;
#if ENABLE_TILE_PRIORITY
					uint8_t tilePri = videoData.tilePriority[(xt + VRAMX * yt) >> 3] & (1 << ((xt + VRAMX * yt) & 7) );
#endif
					if (!( ((uint32_t)addr) >= ((uint32_t) spriteTileStart )))
					{
						if (usedSpriteTileNumber < maxSpriteTilesAvailable)
						{
#if ENABLE_TILE_PRIORITY == 0
							uint8_t *dest = addr;
#else
							uint8_t * dest =   (uint8_t*) ((vram[VRAMX * yt + xt] | HIGH_ADDRESS) + offsetY * TILE_SIZE_X + offsetX) ;
#endif
							uint8_t tileFound = 0;
							uint8_t *pp = (uint8_t*) &p[(minY - ySpr) * verticalIncrement + (minX - xSpr) * horizontalIncrement];
							for (y = minY; y < maxY; y++)
							{
								for (x = 0; x < deltaX; x++)
								{
									if (requiredSpriteFlags == SPRITE_FLAGS_PRIORITY_ALWAYS_TOP)
									{
										if (*pp)
										{
											spriteTile_t *pSpriteTile = &spriteTiles[usedSpriteTileNumber];
											pSpriteTile->originalVramIndex = VRAMX * yt + xt;
											pSpriteTile->originalTileLoAddr = (uint32_t) addr;
											uint32_t *s = (uint32_t *) addr;
											uint32_t *d = spriteTileStart + 16 * usedSpriteTileNumber;
											vram[VRAMX * yt + xt] =  (uint32_t) d;	// upper 16 bit removed
											// copy old tile to temp sprite tile. This could be done in DMA. But for only 64 bytes the time required to setup the descriptors might exceed the time saved by the DMA.
											copyTileFast(d, s);
// 											for (int i = 0; i < TILE_SIZE_X * TILE_SIZE_Y / 4; i++)
// 											{
// 												*d++= *s++;
// 											}
											usedSpriteTileNumber++;
											tileFound = 1;
											break;
										}
										pp += horizontalIncrement;
									}
									else if (requiredSpriteFlags == SPRITE_FLAGS_PRIORITY_ALWAYS_BOTTOM)
									{
										if (*pp && *dest == 0 )
										{
											spriteTile_t *pSpriteTile = &spriteTiles[usedSpriteTileNumber];
											pSpriteTile->originalVramIndex = VRAMX * yt + xt;
											pSpriteTile->originalTileLoAddr = (uint32_t) addr;
											uint32_t *s = (uint32_t *) addr;
											uint32_t *d = spriteTileStart + 16 * usedSpriteTileNumber;
											vram[VRAMX * yt + xt] =  (uint32_t) d;	// upper 16 bit removed
											// copy old tile to temp sprite tile. This could be done in DMA. But for only 64 bytes the time required to setup the descriptors might exceed the time saved by the DMA.
											copyTileFast(d, s);
// 											for (int i = 0; i < TILE_SIZE_X * TILE_SIZE_Y / 4; i++)
// 											{
// 												*d++= *s++;
// 											}
											usedSpriteTileNumber++;
											tileFound = 1;
											break;
										}
										dest++;
										pp += horizontalIncrement;
									}
									else
									{
#if ENABLE_TILE_PRIORITY
										if (*pp && (tilePri == 0 || (tilePri && *dest == 0 && TILES_ALL_OPAQUE == 0)) )
										{
#else
										if (*pp)
										{
#endif
											spriteTile_t *pSpriteTile = &spriteTiles[usedSpriteTileNumber];
											pSpriteTile->originalVramIndex = VRAMX * yt + xt;
											pSpriteTile->originalTileLoAddr = (uint32_t) addr;
											uint32_t *s = (uint32_t *) addr;
											uint32_t *d = spriteTileStart + 16 * usedSpriteTileNumber;
											vram[VRAMX * yt + xt] =  (uint32_t) d;	// upper 16 bit removed
											// copy old tile to temp sprite tile. This could be done in DMA. But for only 64 bytes the time required to setup the descriptors might exceed the time saved by the DMA.
											copyTileFast(d, s);
// 											for (int i = 0; i < TILE_SIZE_X * TILE_SIZE_Y / 4; i++)
// 											{
// 												*d++= *s++;
// 											}
											usedSpriteTileNumber++;
											tileFound = 1;
											break;
										}
										dest++;
										pp += horizontalIncrement;
									}
								}
								if (tileFound)
									break;
								pp += vi ;//verticalIncrement - (maxX - minX) * horizontalIncrement;
		  						dest += TILE_SIZE_X - deltaX;
							}
							if (tileFound)
							{
								uint8_t *pp = (uint8_t *) &p[(minY - ySpr) * verticalIncrement + (minX - xSpr) * horizontalIncrement];
								dest =   (uint8_t*) ((vram[VRAMX * yt + xt] | HIGH_ADDRESS) + offsetY * TILE_SIZE_X + offsetX) ;
								if (requiredSpriteFlags == SPRITE_FLAGS_PRIORITY_ALWAYS_TOP)
								{
#if ENABLE_TRANSPARENT_SPRITES == 1
									if (pSprite->flags & SPRITE_FLAGS_TRANSPARENT_SPRITE)
									{
										drawTransparentSpriteOnTopOfTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);
									}
									else
#endif
									{
										drawSpriteOnTopOfTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);
									}
								}
								else if (requiredSpriteFlags == SPRITE_FLAGS_PRIORITY_ALWAYS_BOTTOM)
								{
#if ENABLE_TRANSPARENT_SPRITES == 1
									if (pSprite->flags & SPRITE_FLAGS_TRANSPARENT_SPRITE)
									{
										drawTransparentSpriteUnderTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);	
									}
									else
#endif
									{
										drawSpriteUnderTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);
									}
									
								}
								else
								{
#if ENABLE_TILE_PRIORITY
									if (tilePri)
									{
#if ENABLE_TRANSPARENT_SPRITES == 1
										if (pSprite->flags & SPRITE_FLAGS_TRANSPARENT_SPRITE)
										{
// Modified: tiles with priority are redrawn later
//											drawTransparentSpriteUnderTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);
											drawTransparentSpriteOnTopOfTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);

										}
										else
#endif
										{
// Modified: tiles with priority are redrawn later
//											drawSpriteUnderTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);
											drawSpriteOnTopOfTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);
										}										
									}
									else
#endif
									{
#if ENABLE_TRANSPARENT_SPRITES == 1
										if (pSprite->flags & SPRITE_FLAGS_TRANSPARENT_SPRITE)
										{
											drawTransparentSpriteOnTopOfTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);
										}
										else
#endif
										{
											drawSpriteOnTopOfTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);
										}
										
									}
									
								}
							}
						}
					}
					else
					{
						uint8_t *dest =   (uint8_t*) (((uint32_t)vram[VRAMX * yt + xt]) | HIGH_ADDRESS) + offsetY * TILE_SIZE_X + offsetX ;
						uint8_t *pp = (uint8_t*) &p[(minY - ySpr) * verticalIncrement + (minX - xSpr) * horizontalIncrement];
						// just copy the sprite
						if (requiredSpriteFlags == SPRITE_FLAGS_PRIORITY_ALWAYS_TOP)
						{
#if ENABLE_TRANSPARENT_SPRITES == 1
							if (pSprite->flags & SPRITE_FLAGS_TRANSPARENT_SPRITE)
							{
								drawTransparentSpriteOnTopOfTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);
							}
							else
#endif
							{
								drawSpriteOnTopOfTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);
							}
									
						}
						else if (requiredSpriteFlags == SPRITE_FLAGS_PRIORITY_ALWAYS_BOTTOM)
						{
#if ENABLE_TRANSPARENT_SPRITES == 1
							if (pSprite->flags & SPRITE_FLAGS_TRANSPARENT_SPRITE)
							{
								drawTransparentSpriteUnderTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);										
							}
							else
#endif
							{
								drawSpriteUnderTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);
							}		
						}
						else
						{
#if ENABLE_TILE_PRIORITY
							if (tilePri)
							{
#if ENABLE_TRANSPARENT_SPRITES == 1
								if (pSprite->flags & SPRITE_FLAGS_TRANSPARENT_SPRITE)
								{
									// Modified: tiles with priority are redrawn later
									//											drawTransparentSpriteUnderTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);
									drawTransparentSpriteOnTopOfTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);

								}
								else
#endif
								{
									// Modified: tiles with priority are redrawn later
									//											drawSpriteUnderTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);
									drawSpriteOnTopOfTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);
								}
							}
							else
#endif
							{
#if ENABLE_TRANSPARENT_SPRITES == 1
								if (pSprite->flags & SPRITE_FLAGS_TRANSPARENT_SPRITE)
								{
									drawTransparentSpriteOnTopOfTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);											
								}
								else
#endif
								{
									drawSpriteOnTopOfTile(maxY - minY, pp, dest, verticalIncrement, deltaX, horizontalIncrement);
								}					
							}
									
						}
					}
				}
			}
		}
	}
	return usedSpriteTileNumber;	
#elif GFX_MODE == TILE_MODE2  
	for (int num = 0; num < MAX_ONSCREEN_SPRITES; num++)
	{
		usvcSprite_t *pSprite = &usvcSprites[num];
		if ((pSprite->flags & SPRITE_FLAGS_PRIORITY_MASK) != requiredSpriteFlags)
			continue;
		uint32_t nf = pSprite->frameNum;
		// is sprite active?
		if (nf != 0xFFFF)
		{
			// yes. Find out where it should go and reserve the tempTiles to draw on them.
			// we are going to determine a rectange in the frame data, to be copied into tiles.
			//
			// initial calculations are performed with the unshifted frame. 
			// the parameters of the sprite are expressed in pixels. Note that, due to byte-alignemnt, if the width is odd, the actual number of bytes per sprite line is (width + 1) / 2
			// i.e. the increment is (width + (width & 1)) >> 1
			//nf = nf << 1; <-- already performed on putSprite
			int32_t xSpr;
			#if PER_LINE_X_SCROLL == 0 && PER_TILE_X_SCROLL == 0
			if (pSprite->flags & SPRITE_FLAGS_NO_X_SCROLL_CORRECTION)
			{
				xSpr = pSprite->x;
			}
			else
			{
				xSpr = pSprite->x + videoData.xScroll;
			}
			#else
				#if PER_TILE_X_SCROLL == 0
					// if PER_TILE_X_SCROLL is 0 but PER_LINE_X_SCROLL is not, then we can still use xScroll
					if (pSprite->flags & SPRITE_FLAGS_NO_X_SCROLL_CORRECTION)
					{
			xSpr = pSprite->x;
					}
					else
					{
						xSpr = pSprite->x + videoData.xScroll;
					}
				#else
					xSpr = pSprite->x;			// optional correction will be applied later if PER_TILE_X_SCROLL is not 0. 
				#endif
			#endif
			uint16_t flags = pSprite->flags & SPRITE_FLAGS_ROTATION_MASK;
			int32_t ySpr;
			if (flags & SPRITE_FLAGS_NO_Y_SCROLL_CORRECTION)
			{
				ySpr = pSprite->y;
			}
			else
			{
				ySpr = pSprite->y + videoData.yScroll;
			}
			frame_t *pFrame = (frame_t*) &frameData[nf];
			int32_t wSpr = pFrame->w;
			int32_t hSpr = pFrame->h;
			//
			int32_t maxYsprite;
			int32_t minYsprite = ySpr;
			// boundary check
#if PER_TILE_X_SCROLL == 0
			int32_t maxXsprite;
			int32_t minXsprite = xSpr;
			if (xSpr >= VRAMX * TILE_SIZE_X )
			{
				continue;		// sprite not visible
			}
			else if (xSpr < 0)
			{
				minXsprite = 0;
			}
#endif
			if (ySpr >= VRAMY * TILE_SIZE_Y)
			{
				continue;		// sprite not visible
			}
			else if (ySpr < 0)
			{
				minYsprite = 0;
			}
			maxYsprite = (ySpr + hSpr);
#if PER_TILE_X_SCROLL == 0
			//
			if (flags & SPRITE_FLAGS_FLIP_HORIZONTAL)
			{

				pFrame = (frame_t*) &frameData[nf + ((xSpr & 1 ) ^ (frameData[nf].w & 1))];
			}
			else
			{
				pFrame = (frame_t*) &frameData[nf + (xSpr & 1 )];
			}								
			wSpr = pFrame->w;
			wSpr += wSpr & 1;
			xSpr = xSpr >> 1;
			wSpr = wSpr >> 1;
			maxXsprite = (xSpr + wSpr);
			if (maxXsprite < 0)
			{
				continue;		// sprite not visible in this tile number
			}
			else if (maxXsprite > VRAMX * TILE_SIZE_X / 2)
			{
				maxXsprite = VRAMX * TILE_SIZE_X / 2;
			}
			minXsprite = xSpr;
			if (xSpr >= VRAMX * TILE_SIZE_X / 2 )
			{
				continue;		// sprite not visible in this tile number
			}
			else if (xSpr < 0)
			{
				minXsprite = 0;
			}					
#endif
			if (maxYsprite < 0)
			{
				continue;		// sprite not visible
			}
			else if (maxYsprite > VRAMY * TILE_SIZE_Y)
			{
				// let's calculate how much the sprite is offscreen
				maxYsprite = VRAMY * TILE_SIZE_Y ;
			}
			const uint8_t *p =  0;				// these are initialized just to avoid warnings
			int horizontalIncrement = 0 ;	// these are initialized just to avoid warnings
			int verticalIncrement = 0;
			// if independent PER_TILE_X_SCROLL is not used, we can now calculate, depending on the orientation, the increments we need to add to each pointer, as well as the pointer start location.
#if PER_TILE_X_SCROLL == 0
			// NOTE: here wSpr si always even (i.e. byte rounded)
			if (flags  == 0)
			{
				p = &pFrame->pData[0]; // point to the top left pixel.
				verticalIncrement  = wSpr ;
				horizontalIncrement = 1;
			}
			else if (flags  == SPRITE_FLAGS_FLIP_VERTICAL)
			{
				p = &pFrame->pData[( hSpr - 1 ) * wSpr];
				verticalIncrement = -wSpr;
				horizontalIncrement = 1;
			}
			else if  (flags  == SPRITE_FLAGS_FLIP_HORIZONTAL)
			{
				p = &pFrame->pData[wSpr - 1];
				verticalIncrement = wSpr;
				horizontalIncrement = -1;
			}
			else if  (flags == (SPRITE_FLAGS_FLIP_HORIZONTAL | SPRITE_FLAGS_FLIP_VERTICAL ))
			{
				p = &pFrame->pData[hSpr * wSpr  - 1];
				verticalIncrement = -wSpr;
				horizontalIncrement = -1;
			}
			int32_t tileXstart = minXsprite >> (LOG2_TILE_SIZE_X - 1);
			int32_t tileXstop = (maxXsprite - 1) >> (LOG2_TILE_SIZE_X - 1);
#endif
			int32_t tileYstart = minYsprite >> LOG2_TILE_SIZE_Y;
			int32_t tileYstop = (maxYsprite - 1) >> LOG2_TILE_SIZE_Y;
			uint32_t maxSpriteTilesAvailable =  min ((MAX_TILES - videoData.ramTiles), MAX_TEMP_SPRITE_TILES);
			uint32_t *spriteTileStart = tiles[videoData.ramTiles];
			for (int yt = tileYstart; yt <= tileYstop; yt++)
			{
#if PER_TILE_X_SCROLL == 1
				// if PER_TILE_X_SCROLL is used, then we need first to check if the sprite will be drawn at an odd-pixel horizontal coordinate.
				// after we found out where the sprite will be drawn, we can adjust the frame number. Then, we can, based on horizontal and vertical
				// flags, calculate the horizontal and vertical increments. 
				uint8_t delta;
				int32_t maxXsprite2;
				if (pSprite->flags & SPRITE_FLAGS_PER_TILE_X_SCROLL_DEFORMATION)
				{
					#if PER_LINE_X_SCROLL == 0
						if (pSprite->flags & SPRITE_FLAGS_NO_X_SCROLL_CORRECTION)
						{
							delta = 0;
						}
						else
						{
							delta = videoData.xScroll;
						}
					#else
						// no xScroll available if both PER_TILE_X_SCROLL and PER_LINE_X_SCROLL are active.
						delta = 0;
					#endif
				}
				else
				{
					#if PER_LINE_X_SCROLL == 0
						if (pSprite->flags & SPRITE_FLAGS_NO_X_SCROLL_CORRECTION)
						{
							delta = videoData.ptrTileXScroll[yt];
						}
						else
						{
							delta = videoData.xScroll + videoData.ptrTileXScroll[yt];
						}
					#else
						delta = videoData.ptrTileXScroll[yt];
					#endif
				}
				xSpr = pSprite->x + delta;
				// note: for ODD-w sprites, the horizontal flip would introduce a spurious +1 shift because one nibble is 0 by default
				// We need therefore check which will be the frame and the xSpr.
												
				if (flags & SPRITE_FLAGS_FLIP_HORIZONTAL)
				{
					pFrame = (frame_t*) &frameData[nf + ((xSpr & 1 ) ^ (frameData[nf].w & 1))];
				}
				else
				{
					pFrame = (frame_t*) &frameData[nf + (xSpr & 1 )];
				}						
				wSpr = pFrame->w;
				xSpr = (xSpr >> 1);
				// based of the width in pixel, divide by two and round up to the next integer (to calculate the actual number of bytes)
				wSpr += wSpr & 1;
				wSpr = wSpr >> 1;
				maxXsprite2 = (xSpr + wSpr);
				if (maxXsprite2 < 0)
				{
					continue;		// sprite not visible in this tile number
				}
				else if (maxXsprite2 > VRAMX * TILE_SIZE_X / 2)
				{
					maxXsprite2 = VRAMX * TILE_SIZE_X / 2;
				}
				int32_t minXsprite = xSpr;
				if (xSpr >= VRAMX * TILE_SIZE_X / 2 )
				{
					continue;		// sprite not visible in this tile number
				}
				else if (xSpr < 0)
				{
					minXsprite = 0;
				}
				// NOTE: here wSpr si always even (i.e. byte rounded)
				if (flags  == 0)
				{
					p = &pFrame->pData[0]; // point to the top left pixel.
					verticalIncrement  = wSpr ;					
					horizontalIncrement = 1;
				}
				else if (flags  == SPRITE_FLAGS_FLIP_VERTICAL)
				{
					p = &pFrame->pData[( hSpr - 1 ) * wSpr];
					verticalIncrement = -wSpr;
					horizontalIncrement = 1;
				}
				else if  (flags  == SPRITE_FLAGS_FLIP_HORIZONTAL)
				{
				
					p = &pFrame->pData[wSpr - 1];
					verticalIncrement = wSpr;
					horizontalIncrement = -1;
				}
				else if  (flags == (SPRITE_FLAGS_FLIP_HORIZONTAL | SPRITE_FLAGS_FLIP_VERTICAL ))
				{
					p = &pFrame->pData[hSpr * wSpr  - 1];
					verticalIncrement = -wSpr;
					horizontalIncrement = -1;
				}
				int32_t tileXstart = minXsprite >> (LOG2_TILE_SIZE_X - 1);
				int32_t tileXstop = (maxXsprite2 - 1) >> (LOG2_TILE_SIZE_X - 1);
#endif
				int32_t minY = (yt * TILE_SIZE_Y < minYsprite) ? minYsprite : (yt * TILE_SIZE_Y);
				int32_t maxY = ((yt + 1) * TILE_SIZE_Y ) > maxYsprite ? maxYsprite : ((yt + 1) * TILE_SIZE_Y);
				int32_t offsetY =   minY  - yt * TILE_SIZE_Y;
				for (int xt = tileXstart; xt <= tileXstop ; xt++)
				{
					int32_t minX = (xt * (TILE_SIZE_X / 2) < minXsprite) ? minXsprite : (xt * (TILE_SIZE_X / 2));
#if PER_TILE_X_SCROLL == 0
					int32_t deltaX = (((xt + 1) * (TILE_SIZE_X / 2)) > maxXsprite ? maxXsprite - minX : ((xt + 1) * (TILE_SIZE_X / 2)) - minX);// >> 1;
#else
					int32_t deltaX = (((xt + 1) * (TILE_SIZE_X / 2)) > maxXsprite2 ? (maxXsprite2 - minX)  : ((xt + 1) * (TILE_SIZE_X / 2)) - minX);// >> 1;
#endif
					int32_t offsetX =   minX - xt * (TILE_SIZE_X / 2);
					// now we are in one tile. Let's copy the sprite there the data (if any)
					int x, y;
					// let us first see if we are already writing on a sprite tile or if we need to reserve it first
					uint8_t *addr = (uint8_t*) ((vram[VRAMX * yt + xt]) | HIGH_ADDRESS);
					int32_t vi = verticalIncrement  - deltaX  * horizontalIncrement;
#if ENABLE_TILE_PRIORITY
					uint8_t tilePri = videoData.tilePriority[(xt + VRAMX * yt) >> 3] & (1 << ((xt + VRAMX * yt) & 7) );
#endif
					uint8_t tileFound = ( ((uint32_t)addr) >= ((uint32_t) spriteTileStart ));
					if (!tileFound)
					{
						if (usedSpriteTileNumber < maxSpriteTilesAvailable)
						{
#if ENABLE_TILE_PRIORITY == 0
							uint8_t *dest = (uint8_t*) ((vram[VRAMX * yt + xt] | HIGH_ADDRESS) + ((offsetY * (TILE_SIZE_X / 2) + offsetX) ));
#else
							uint8_t *dest =   (uint8_t*) ((vram[VRAMX * yt + xt] | HIGH_ADDRESS) + ((offsetY * (TILE_SIZE_X / 2) + offsetX) ));
#endif
							uint8_t *pp = (uint8_t*) &p[(minY - ySpr) * verticalIncrement + (((minX - xSpr) * horizontalIncrement))];
							for (y = minY; y < maxY; y++)
							{
								for (x = 0; x < deltaX; x += 1)
								{
									if (requiredSpriteFlags == SPRITE_FLAGS_PRIORITY_ALWAYS_TOP)
									{
										if (*pp)
										{
											spriteTile_t *pSpriteTile = &spriteTiles[usedSpriteTileNumber];
											pSpriteTile->originalVramIndex = VRAMX * yt + xt;
											pSpriteTile->originalTileLoAddr = (uint32_t) addr;
											uint32_t *s = (uint32_t *) addr;
											uint32_t *d = spriteTileStart + 8 * usedSpriteTileNumber; 
											vram[VRAMX * yt + xt] =  (uint32_t) d;	// upper 16 bit removed
											// copy old tile to temp sprite tile. This could be done in DMA. But for only 64 bytes the time required to setup the descriptors might exceed the time saved by the DMA.
 											copyTileFast(d, s);
// 											for (int i = 0; i < TILE_SIZE_X * TILE_SIZE_Y / 8; i++) 
// 											{
// 												*d++= *s++;
// 											}
											usedSpriteTileNumber++;
											tileFound = 1;
											break;
										}
										// dest not used!
										pp += horizontalIncrement;
									}
									else if (requiredSpriteFlags == SPRITE_FLAGS_PRIORITY_ALWAYS_BOTTOM)
									{
										if (*pp && (nibbleMask[*dest] != 0))
										{
											spriteTile_t *pSpriteTile = &spriteTiles[usedSpriteTileNumber];
											pSpriteTile->originalVramIndex = VRAMX * yt + xt;
											pSpriteTile->originalTileLoAddr = (uint32_t) addr;
											uint32_t *s = (uint32_t *) addr;
											uint32_t *d = spriteTileStart + 8 * usedSpriteTileNumber; 
											vram[VRAMX * yt + xt] =  (uint32_t) d;	// upper 16 bit removed
											// copy old tile to temp sprite tile. This could be done in DMA. But for only 64 bytes the time required to setup the descriptors might exceed the time saved by the DMA.
											copyTileFast(d, s);
// 											for (int i = 0; i < TILE_SIZE_X * TILE_SIZE_Y / 8; i++) 
// 											{
// 												*d++= *s++;
// 											}
											usedSpriteTileNumber++;
											tileFound = 1;
											break;
										}
										dest++;
										pp += horizontalIncrement;
									}
									else
									{
#if ENABLE_TILE_PRIORITY
										if (*pp && (tilePri == 0 || (tilePri && nibbleMask[*dest] != 0 && TILES_ALL_OPAQUE == 0)) )
#else
										if (*pp)
#endif
										{
											spriteTile_t *pSpriteTile = &spriteTiles[usedSpriteTileNumber];
											pSpriteTile->originalVramIndex = VRAMX * yt + xt;
											pSpriteTile->originalTileLoAddr = (uint32_t) addr;
											uint32_t *s = (uint32_t *) addr;
											uint32_t *d = spriteTileStart + 8 * usedSpriteTileNumber; 
											vram[VRAMX * yt + xt] =  (uint32_t) d;	// upper 16 bit removed
											// copy old tile to temp sprite tile. This could be done in DMA. But for only 64 bytes the time required to setup the descriptors might exceed the time saved by the DMA.
 											copyTileFast(d, s);
// 											for (int i = 0; i < TILE_SIZE_X * TILE_SIZE_Y / 8; i++) 
// 											{
// 												*d++= *s++;
// 											}
											usedSpriteTileNumber++;
											tileFound = 1;
											break;
										}
										dest++;
										pp += horizontalIncrement;
									}
								}
								if (tileFound)
									break;
								pp +=  vi;//verticalIncrement - (maxX - minX) * horizontalIncrement;
								dest += TILE_SIZE_X / 2 - deltaX;
							}
						}
					}
					// now, either the tile has been allocated, or it was already allocated
					if (tileFound)
					{
						uint8_t *pp = (uint8_t*) &p[(minY - ySpr) * verticalIncrement + (((minX - xSpr) * horizontalIncrement) )];
						uint8_t *dest =   (uint8_t*) ((vram[VRAMX * yt + xt] | HIGH_ADDRESS) + ((offsetY * (TILE_SIZE_X / 2) + offsetX) )) ;
						if (requiredSpriteFlags == SPRITE_FLAGS_PRIORITY_ALWAYS_TOP)
						{
							if (horizontalIncrement > 0)
							{
								drawSpriteOnTopOfTile(maxY - minY,pp, dest,verticalIncrement,deltaX);
							}
							else
							{
								drawMirrorSpriteOnTopOfTile(maxY - minY,pp, dest,verticalIncrement,deltaX);
							}
						}
						else if (requiredSpriteFlags == SPRITE_FLAGS_PRIORITY_ALWAYS_BOTTOM)
						{
							if (horizontalIncrement > 0)
							{
								drawSpriteUnderTile(maxY - minY,pp, dest,verticalIncrement,deltaX);
							}
							else
							{
								drawMirrorSpriteUnderTile(maxY - minY,pp, dest,verticalIncrement,deltaX);
							}
						}
						else
						{
#if ENABLE_TILE_PRIORITY
							if (tilePri)
							{
								if (horizontalIncrement > 0)
								{
									drawSpriteUnderTile(maxY - minY,pp, dest,verticalIncrement,deltaX);
								}
								else
								{
									drawMirrorSpriteUnderTile(maxY - minY,pp, dest,verticalIncrement,deltaX);
								}
							}
							else
#endif
							{
								if (horizontalIncrement > 0)
								{
									drawSpriteOnTopOfTile(maxY - minY,pp, dest,verticalIncrement,deltaX);
								}
								else 
								{
									drawMirrorSpriteOnTopOfTile(maxY - minY,pp, dest,verticalIncrement,deltaX);										
								}
							}
						}
					}
				}
			}
		}
	}
	return usedSpriteTileNumber;	
#endif
}
#if GFX_MODE == TILE_MODE2
// fast drawing routines for tile mode 2
inline void drawMirrorSpriteUnderTile(int32_t y, uint8_t *pp, uint8_t* dest, int32_t vi, int32_t deltaX)
{
	uint32_t rt0, rt1, rt2,swapNibbleConstant = 0x10100000;	// temporary registers
	asm volatile
	(
		"MOV %[rt0], %[rDrawNextSpriteLine]\n\t"		// deltaX
		"SUB %[pp],#3\n\t"
		"LSL %[rt0],#2\n\t"
		"ADR %[rt1],drawTable%=\n\t"
		"LDR %[rt0],[%[rt1], %[rt0]]\n\t"
		"MOV %[rDrawNextSpriteLine],%[rt0]\n\t"
		"BX %[rDrawNextSpriteLine]\n\t"
		".align(2)\n\t"
		"drawTable%=:\n\t"
			".word end%= + 1\n\t"
			".word byte0%= + 1\n\t"
			".word byte1%= + 1\n\t"
			".word byte2%= + 1\n\t"
			".word byte3%= + 1\n\t"
		"byte3%=:\n\t"
			"LDRB %[rt0], [%[pp], #0]\n\t"
			"MUL %[rt0], %[swapNibbleConstant]\n\t"
			"LSR %[rt0],#24\n\t"
			"LDRB %[rt2], [%[dest], #3]\n\t"
			"LDRB %[rt1], [%[pNibbleMask], %[rt2]]\n\t"
			"AND %[rt0],%[rt1]\n\t"
			"ORR %[rt2],%[rt0]\n\t"
			"STRB %[rt2], [%[dest], #3]\n\t"
		"byte2%=:\n\t"
			"LDRB %[rt0], [%[pp], #1]\n\t"
			"MUL %[rt0], %[swapNibbleConstant]\n\t"
			"LSR %[rt0],#24\n\t"
			"LDRB %[rt2], [%[dest], #2]\n\t"
			"LDRB %[rt1], [%[pNibbleMask], %[rt2]]\n\t"
			"AND %[rt0],%[rt1]\n\t"
			"ORR %[rt2],%[rt0]\n\t"
			"STRB %[rt2], [%[dest], #2]\n\t"
		"byte1%=:\n\t"
			"LDRB %[rt0], [%[pp], #2]\n\t"
			"MUL %[rt0], %[swapNibbleConstant]\n\t"
			"LSR %[rt0],#24\n\t"
			"LDRB %[rt2], [%[dest], #1]\n\t"
			"LDRB %[rt1], [%[pNibbleMask], %[rt2]]\n\t"
			"AND %[rt0],%[rt1]\n\t"
			"ORR %[rt2],%[rt0]\n\t"
			"STRB %[rt2], [%[dest], #1]\n\t"
		"byte0%=:\n\t"
			"LDRB %[rt0], [%[pp], #3]\n\t"
			"MUL %[rt0], %[swapNibbleConstant]\n\t"
			"LSR %[rt0],#24\n\t"
			"LDRB %[rt2], [%[dest], #0]\n\t"
			"LDRB %[rt1], [%[pNibbleMask], %[rt2]]\n\t"
			"AND %[rt0],%[rt1]\n\t"
			"ORR %[rt2],%[rt0]\n\t"
			"STRB %[rt2], [%[dest]]\n\t"
		// change y
		"ADD %[dest],#4\n\t"
		"ADD %[pp],%[vi]\n\t"
		"SUB %[y],#1\n\t"
		"BEQ end%=\n\t"
		"BX %[rDrawNextSpriteLine]\n\t"
		"end%=:\n\t"
	: [rt0] "=&l" (rt0) , [rt1] "=&l" (rt1), [rt2] "=&l" (rt2), [rDrawNextSpriteLine] "+h" (deltaX)
	: [pNibbleMask] "l" (nibbleMask), [y] "l" (y), [dest] "l" (dest), [pp] "l" (pp),  [vi] "h" (vi), [swapNibbleConstant] "l" (swapNibbleConstant)     // inputs
	:  // clobber list
	);
	
}

inline void drawSpriteUnderTile(int32_t y, uint8_t *pp, uint8_t* dest, int32_t vi, int32_t deltaX)
{
	//	return;
	uint32_t rt0, rt1, rt2;	// temporary registers
	asm volatile
	(
		"MOV %[rt0], %[rDrawNextSpriteLine]\n\t"		// deltaX
		"LSL %[rt0],#2\n\t"
		"ADR %[rt1],drawTable%=\n\t"
		"LDR %[rt0],[%[rt1], %[rt0]]\n\t"
		"MOV %[rDrawNextSpriteLine],%[rt0]\n\t"
		"BX %[rDrawNextSpriteLine]\n\t"
		".align(2)\n\t"
		"drawTable%=:\n\t"
			".word byte0%= + 1\n\t"
			".word byte0%= + 1\n\t"
			".word byte1%= + 1\n\t"
			".word byte2%= + 1\n\t"
			".word byte3%= + 1\n\t"
		"byte3%=:\n\t"
			"LDRB %[rt0], [%[pp], #3]\n\t"
			"LDRB %[rt2], [%[dest], #3]\n\t"
			"LDRB %[rt1], [%[pNibbleMask], %[rt2]]\n\t"
			"AND %[rt0],%[rt1]\n\t"
			"ORR %[rt2],%[rt0]\n\t"
			"STRB %[rt2], [%[dest], #3]\n\t"
		"byte2%=:\n\t"
			"LDRB %[rt0], [%[pp], #2]\n\t"
			"LDRB %[rt2], [%[dest], #2]\n\t"
			"LDRB %[rt1], [%[pNibbleMask], %[rt2]]\n\t"
			"AND %[rt0],%[rt1]\n\t"
			"ORR %[rt2],%[rt0]\n\t"
			"STRB %[rt2], [%[dest], #2]\n\t"
		"byte1%=:\n\t"
			"LDRB %[rt0], [%[pp], #1]\n\t"
			"LDRB %[rt2], [%[dest], #1]\n\t"
			"LDRB %[rt1], [%[pNibbleMask], %[rt2]]\n\t"
			"AND %[rt0],%[rt1]\n\t"
			"ORR %[rt2],%[rt0]\n\t"
			"STRB %[rt2], [%[dest], #1]\n\t"
		"byte0%=:\n\t"
			"LDRB %[rt0], [%[pp]]\n\t"
			"LDRB %[rt2], [%[dest], #0]\n\t"
			"LDRB %[rt1], [%[pNibbleMask], %[rt2]]\n\t"
			"AND %[rt0],%[rt1]\n\t"
			"ORR %[rt2],%[rt0]\n\t"
			"STRB %[rt2], [%[dest]]\n\t"
		// change y
		"ADD %[dest],#4\n\t"
		"ADD %[pp],%[vi]\n\t"
		"SUB %[y],#1\n\t"
		"BEQ end%=\n\t"
		"BX %[rDrawNextSpriteLine]\n\t"
		"end%=:\n\t"
	: [rt0] "=&l" (rt0) , [rt1] "=&l" (rt1), [rt2] "=&l" (rt2), [rDrawNextSpriteLine] "+h" (deltaX)
	: [pNibbleMask] "l" (nibbleMask), [y] "l" (y), [dest] "l" (dest), [pp] "l" (pp),  [vi] "h" (vi)     // inputs
	:  // clobber list
	);
}
inline void drawMirrorSpriteOnTopOfTile(int32_t y, uint8_t *pp, uint8_t* dest, int32_t vi, int32_t deltaX)
{
//	return;
	uint32_t rt0, rt1, rt2,swapNibbleConstant = 0x10100000;	// temporary registers
	asm volatile
	(
		"MOV %[rt0], %[rDrawNextSpriteLine]\n\t"		// deltaX 
		"SUB %[pp],#3\n\t"
		"LSL %[rt0],#2\n\t"
		"ADR %[rt1],drawTable%=\n\t"
		"LDR %[rt0],[%[rt1], %[rt0]]\n\t"
		"MOV %[rDrawNextSpriteLine],%[rt0]\n\t"
		"BX %[rDrawNextSpriteLine]\n\t"
		".align(2)\n\t"
		"drawTable%=:\n\t"
			".word end%= + 1\n\t"
			".word byte0%= + 1\n\t"
			".word byte1%= + 1\n\t"
			".word byte2%= + 1\n\t"
			".word byte3%= + 1\n\t"
		"byte3%=:\n\t"
			"LDRB %[rt0], [%[pp], #0]\n\t"
			"MUL %[rt0], %[swapNibbleConstant]\n\t"
			"LSR %[rt0],#24\n\t"
			"LDRB %[rt1], [%[pNibbleMask], %[rt0]]\n\t"
			"LDRB %[rt2], [%[dest], #3]\n\t"
			"AND %[rt2],%[rt1]\n\t"
			"ORR %[rt2],%[rt0]\n\t"
			"STRB %[rt2], [%[dest], #3]\n\t"
		"byte2%=:\n\t"
			"LDRB %[rt0], [%[pp], #1]\n\t"
			"MUL %[rt0], %[swapNibbleConstant]\n\t"
			"LSR %[rt0],#24\n\t"
			"LDRB %[rt1], [%[pNibbleMask], %[rt0]]\n\t"
			"LDRB %[rt2], [%[dest], #2]\n\t"
			"AND %[rt2],%[rt1]\n\t"
			"ORR %[rt2],%[rt0]\n\t"
			"STRB %[rt2], [%[dest], #2]\n\t"
		"byte1%=:\n\t"
			"LDRB %[rt0], [%[pp], #2]\n\t"
			"MUL %[rt0], %[swapNibbleConstant]\n\t"
			"LSR %[rt0],#24\n\t"
			"LDRB %[rt1], [%[pNibbleMask], %[rt0]]\n\t"
			"LDRB %[rt2], [%[dest], #1]\n\t"
			"AND %[rt2],%[rt1]\n\t"
			"ORR %[rt2],%[rt0]\n\t"
			"STRB %[rt2], [%[dest], #1]\n\t"
		"byte0%=:\n\t"
			"LDRB %[rt0], [%[pp], #3]\n\t"
			"MUL %[rt0], %[swapNibbleConstant]\n\t"
			"LSR %[rt0],#24\n\t"
			"LDRB %[rt1], [%[pNibbleMask], %[rt0]]\n\t"
			"LDRB %[rt2], [%[dest], #0]\n\t"
			"AND %[rt2],%[rt1]\n\t"
			"ORR %[rt2],%[rt0]\n\t"
			"STRB %[rt2], [%[dest]]\n\t"
		// change y
		"ADD %[dest],#4\n\t"
		"ADD %[pp],%[vi]\n\t"
		"SUB %[y],#1\n\t"
		"BEQ end%=\n\t"
		"BX %[rDrawNextSpriteLine]\n\t"
		"end%=:\n\t"
		: [rt0] "=&l" (rt0) , [rt1] "=&l" (rt1), [rt2] "=&l" (rt2), [rDrawNextSpriteLine] "+h" (deltaX)
		: [pNibbleMask] "l" (nibbleMask), [y] "l" (y), [dest] "l" (dest), [pp] "l" (pp),  [vi] "h" (vi), [swapNibbleConstant] "l" (swapNibbleConstant)     // inputs
		:  // clobber list
	);
	
}
inline void drawSpriteOnTopOfTile(int32_t y, uint8_t *pp, uint8_t* dest, int32_t vi, int32_t deltaX)
{
	//	return;
	uint32_t rt0, rt1, rt2;	// temporary registers
	asm volatile
	(
		"MOV %[rt0], %[rDrawNextSpriteLine]\n\t"		// deltaX 
		"LSL %[rt0],#2\n\t"
		"ADR %[rt1],drawTable%=\n\t"
		"LDR %[rt0],[%[rt1], %[rt0]]\n\t"
		"MOV %[rDrawNextSpriteLine],%[rt0]\n\t"
		"BX %[rDrawNextSpriteLine]\n\t"
		".align(2)\n\t"
		"drawTable%=:\n\t"
			".word byte0%= + 1\n\t"
			".word byte0%= + 1\n\t"
			".word byte1%= + 1\n\t"
			".word byte2%= + 1\n\t"
			".word byte3%= + 1\n\t"
		"byte3%=:\n\t"
			"LDRB %[rt0], [%[pp], #3]\n\t"
			"LDRB %[rt1], [%[pNibbleMask], %[rt0]]\n\t"
			"LDRB %[rt2], [%[dest], #3]\n\t"
			"AND %[rt2],%[rt1]\n\t"
			"ORR %[rt2],%[rt0]\n\t"
			"STRB %[rt2], [%[dest], #3]\n\t"
		"byte2%=:\n\t"
			"LDRB %[rt0], [%[pp], #2]\n\t"
			"LDRB %[rt1], [%[pNibbleMask], %[rt0]]\n\t"
			"LDRB %[rt2], [%[dest], #2]\n\t"
			"AND %[rt2],%[rt1]\n\t"
			"ORR %[rt2],%[rt0]\n\t"
			"STRB %[rt2], [%[dest], #2]\n\t"
		"byte1%=:\n\t"
			"LDRB %[rt0], [%[pp], #1]\n\t"
			"LDRB %[rt1], [%[pNibbleMask], %[rt0]]\n\t"
			"LDRB %[rt2], [%[dest], #1]\n\t"
			"AND %[rt2],%[rt1]\n\t"
			"ORR %[rt2],%[rt0]\n\t"
			"STRB %[rt2], [%[dest], #1]\n\t"
		"byte0%=:\n\t"
			"LDRB %[rt0], [%[pp]]\n\t"
			"LDRB %[rt1], [%[pNibbleMask], %[rt0]]\n\t"
			"LDRB %[rt2], [%[dest], #0]\n\t"
			"AND %[rt2],%[rt1]\n\t"
			"ORR %[rt2],%[rt0]\n\t"
			"STRB %[rt2], [%[dest]]\n\t"
		// change y
		"ADD %[dest],#4\n\t"
		"ADD %[pp],%[vi]\n\t"
		"SUB %[y],#1\n\t"
		"BEQ end%=\n\t"
		"BX %[rDrawNextSpriteLine]\n\t"
		"end%=:\n\t"
		: [rt0] "=&l" (rt0) , [rt1] "=&l" (rt1), [rt2] "=&l" (rt2), [rDrawNextSpriteLine] "+h" (deltaX)
		: [pNibbleMask] "l" (nibbleMask), [y] "l" (y), [dest] "l" (dest), [pp] "l" (pp),  [vi] "h" (vi)     // inputs
		:  // clobber list
	);
	
}
#endif
/*
*  drawSprites()
*  redraws all the sprites shown in the list. Before actually drawing, it also check and removes the sprites still present on the map. 
*/
void drawSprites()
{
	// previously the vram might have been changed in sprites were present. Therefore we must restore the vram with the original tiles.
	// this is to avoid redraw the map.
	// first restore the vram
	if (!videoData.spriteTilesRemoved)
	{
		restoreBackgroundTiles();
	}
	videoData.spriteTilesRemoved = 0; // signal that sprites have been drawn
 	// reserve the space for the sprites, and write.
	uint32_t usedSpriteTileNumber = 0;
	// if SPRITE_PRIORITY is not enabled, then only the TILE flags determine (if ENABLE_TILE_PRIORITY is not 0) if the sprite must be over or under the tile.
	usedSpriteTileNumber = drawSpritesWithFlags( usedSpriteTileNumber, 0);
	#if ENABLE_TILE_PRIORITY != 0 && TILES_ALL_OPAQUE == 0
		// if tile priority is enabled, and if the tiles are not all opaque (which would not result in any drawing operation), then we must
		// redraw the tiles with priority that were partially transparent and got some drawing over there.
		for (int n = 0; n < usedSpriteTileNumber; n++)
		{
			// get original index of tile in vram
			uint32_t originalIndex = spriteTiles[n].originalVramIndex;
			// check if the tile has priority
			if ((videoData.tilePriority[originalIndex / 32] >> (originalIndex % 32) ) & 1)
			{
				// this tile has priority. For sure it was not 100% opaque, otherwise no spriteTile would have been reserved.
				// we will use putSpriteOnTopOfTile with the sprite being the actual tile, and the tile being the reserved tile address
				uint8_t* originalTileAddress = (uint8_t*) (spriteTiles[n].originalTileLoAddr | HIGH_ADDRESS);
				uint8_t* reservedTileAddress = (uint8_t*) (vram[originalIndex] | HIGH_ADDRESS);
				#if GFX_MODE == TILE_MODE1
					drawSpriteOnTopOfTile(8, originalTileAddress, reservedTileAddress, 8, 8, 1);
				#else
						drawSpriteOnTopOfTile(8, originalTileAddress, reservedTileAddress, 8, 4);				
				#endif
			}
		}  
	#endif
#if ENABLE_SPRITE_PRIORITY != 0
	usedSpriteTileNumber = drawSpritesWithFlags(usedSpriteTileNumber, SPRITE_FLAGS_PRIORITY_ALWAYS_TOP);
	usedSpriteTileNumber = drawSpritesWithFlags(usedSpriteTileNumber, SPRITE_FLAGS_PRIORITY_ALWAYS_BOTTOM);

	// if SPRITE_PRIORITY is enabled, then the flags on the sprite determine where they are drawn: they can be either drawn on top of everything, or they can be drawn on bottom of everything, or the sprite will let decide the
	// tile priority.
	// To achieve a correct drawing, we must first print the sprites, which let the tiles to decide where they should go, then top, then bottom sprites.
#endif
}
#endif
#endif
// some utility functons
#if GFX_MODE == BITMAPPED_MODE
const uint16_t one2twoBits[256] =	{
	0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015, 0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055,
	0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115, 0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155,
	0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415, 0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455,
	0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515, 0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555,
	//
	0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015, 0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055,
	0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115, 0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155,
	0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415, 0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455,
	0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515, 0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555,
	//
	0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015, 0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055,
	0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115, 0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155,
	0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415, 0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455,
	0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515, 0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555,
	//
	0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015, 0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055,
	0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115, 0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155,
	0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415, 0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455,
	0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515, 0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555,
	//
};
#endif
#if GFX_MODE == BITMAPPED_MODE
void printText(const uint8_t (*font)[128][8], char *text, uint8_t col, uint8_t row, uint16_t fgColor, uint16_t bkgColor)
{
	int i = col;
	uint8_t c;
	c = *text++;
	int paletteIndex = fgColor >> 2;
	fgColor &= 0x3;
	bkgColor &= 0x3;
	if (row >= MAXROWS)
		return;
	if (font == NULL)
		font = &font8x8_basic;		// use default font if none is specified
	while (i < MAXCOLS && c != 0)
	{
		for (int y = 0; y < FONT_HEIGHT; y++)
		{
			uint8_t data = (*font)[c][y];
			uint16_t charLine = one2twoBits[data] * fgColor |  one2twoBits[(uint8_t)(~data)] * bkgColor ;
			uint16_t *pColumn = (uint16_t*) &pixels[ (row * FONT_HEIGHT + y) * BYTES_PER_LINE + i * FONT_WIDTH / PIXELS_PER_BYTE ];
			*pColumn = charLine;
			#if PER_HORIZONTAL_BLOCK_PALETTE_REMAP
				setPaletteIndex(i, (row * FONT_HEIGHT + y), paletteIndex);
	#endif
		}
		c = *text++;
		i++;
	}
}
void addLineAtBottom(char *text, uint16_t fgColor, uint16_t bkgColor)
{
	uint32_t *pSourceQuadPixel = (uint32_t *) &pixels[FONT_HEIGHT * BYTES_PER_LINE ];
	uint32_t *pDestQuadPixel = (uint32_t *) &pixels[0];

	for (int i = (FONT_HEIGHT * BYTES_PER_LINE / 4); i < sizeof(pixels) / 4; i++ )
	{
		*pDestQuadPixel++ = *pSourceQuadPixel++;
	}
	// clear last row with bkg;

	pDestQuadPixel = (uint32_t *) &pixels[sizeof(pixels) - (FONT_HEIGHT * BYTES_PER_LINE)];
	for (int i = 0; i < (FONT_HEIGHT * BYTES_PER_LINE / 4); i++ )
	{
		*pDestQuadPixel++ = bkgColor * 0b01010101010101010101010101010101;
	}
	printText(&font8x8_basic, text,0, MAXROWS - 1 , fgColor, bkgColor);
}
#endif
#if GFX_MODE != BITMAPPED_MODE
void putCharInTile(const uint8_t (*font)[128][8], uint8_t c, uint8_t color, uint8_t backGroundColor, uint8_t flags ,uint8_t *pTile, int yOffset)
{
	(void) (flags);		// just to remove warnings
#if GFX_MODE != TILE_MODE_BOOT
	if (font == NULL)
		font = &font8x8_basic;		// use default font if none is specified	
#else
	if (font == NULL)
		font = &font8x8_boot;		// use default font if none is specified
#endif
#if GFX_MODE == TILE_MODE1
	int maxH = (FONT_HEIGHT > TILE_SIZE_Y ? TILE_SIZE_Y : FONT_HEIGHT) - yOffset;
	int maxW = FONT_WIDTH > TILE_SIZE_X ? TILE_SIZE_X : FONT_WIDTH;
	pTile += yOffset * TILE_SIZE_X;
	for (int y = 0; y < maxH; y++)
	{
		uint8_t pixels = (*font)[c][y];
		for (int x = 0; x < maxW; x++)
		{
			*pTile++ = (pixels & 1)  ?  color : backGroundColor;
			pixels >>= 1;
		}
	}
#elif GFX_MODE == TILE_MODE2
	int maxH = (FONT_HEIGHT > TILE_SIZE_Y ? TILE_SIZE_Y : FONT_HEIGHT) - yOffset;
	int maxW = FONT_WIDTH > TILE_SIZE_X ? TILE_SIZE_X : FONT_WIDTH;
	pTile += yOffset * TILE_SIZE_X/2;
	uint8_t biPix[4] = {backGroundColor * 0x11, (backGroundColor << 4) | color, ( color << 4) | backGroundColor, color * 0x11};
	for (int y = 0; y < maxH ; y++)
	{
		uint8_t pixels = (*font)[c][y];
		for (int x = 0; x < maxW; x += 2)
		{
			*pTile++ = biPix[pixels & 3];
			pixels >>= 2;
		}
	}
#elif GFX_MODE == TILE_MODE_BOOT
	uint16_t * (pTileAddr) = (uint16_t*)pTile;
	uint16_t colorSignal = ((color * 1025) & 0b1100000011001111) | (1 << 12)  | (1 << 9);
	uint16_t backGroundColorSignal = ((backGroundColor * 1025) & 0b1100000011001111) | (1 << 12) | (1 << 9);		
	int maxH = (FONT_HEIGHT > TILE_SIZE_Y ? TILE_SIZE_Y : FONT_HEIGHT) - yOffset;
	int maxW = FONT_WIDTH > TILE_SIZE_X ? TILE_SIZE_X : FONT_WIDTH;
	pTileAddr += yOffset * TILE_SIZE_X * 2;
	for (int y = 0; y < maxH; y++)
	{
		uint8_t pixels = (*font)[c-' '][y];
		for (int x = 0; x < maxW; x++)
		{
			*pTileAddr++ = (pixels & 1)  ?  colorSignal : backGroundColorSignal;
			pixels >>= 1;
		}
	}
#endif
}
#endif
