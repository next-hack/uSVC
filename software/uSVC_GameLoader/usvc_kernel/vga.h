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
*/
#ifndef VGA_H_
#define VGA_H_
#include <stdint.h>
#include "vgaConstants.h"
#include "sprites.h"
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#include "../usvc_config.h"
//
void initVga();
#ifndef VERTICAL_SCREEN_POSITION
	#define VERTICAL_SCREEN_POSITION 40
#endif
#if GFX_MODE == BITMAPPED_MODE
	#define BITS_PER_PIXEL 2
	#define LORES_PIXELS_X (SCREEN_SIZE_X / 2)
	#define PIXELS_PER_BYTE (8 / BITS_PER_PIXEL)
	#define LORES_PIXEL_PER_BYTE (PIXELS_PER_BYTE / 2)
	#define BYTES_PER_LINE (SCREEN_SIZE_X / PIXELS_PER_BYTE)
	extern uint8_t pixels[SCREEN_SIZE_X/4 * MAX_Y_LINES / 2] __attribute__ ((aligned (4)));
	#define TILE_SIZE_X 8
	#define LOG2_TILE_SIZE_X 3
	#define TILE_SIZE_Y 8
	#define LOG2_TILE_SIZE_Y 3	
	#if PER_HORIZONTAL_BLOCK_PALETTE_REMAP
		extern uint32_t palette[MAX_NUMBER_OF_PALETTE_ENTRIES];
	#else
		extern uint32_t palette[16];
	#endif
	void setPaletteIndex(int bx, int y, uint8_t index);
	void initPaletteEntries(const uint32_t *paletteData, uint32_t paletteEntries, uint16_t startPalette);
	void printText(const uint8_t (*font)[128][8], char *text, uint8_t col, uint8_t row, uint16_t fgColor, uint16_t bkgColor);
	void addLineAtBottom(char *text, uint16_t fgColor, uint16_t bkgColor);
#else
	#define HIGH_ADDRESS 0x20000000UL		// used to create the full ram address
	#define TILE_SIZE_X 8
	#define LOG2_TILE_SIZE_X 3
	#define TILE_SIZE_Y 8
	#define LOG2_TILE_SIZE_Y 3
	#if GFX_MODE == TILE_MODE_BOOT
		#define VRAMX 40
		#define VRAMY 27			
		extern uint8_t vram[VRAMY * VRAMX] __attribute__ ((aligned (4)));
		extern uint16_t tiles[MAX_TILES][TILE_SIZE_X * TILE_SIZE_Y]  __attribute__ ((aligned (4)));
		extern uint8_t rowRemapTable[200];
	#elif GFX_MODE == TILE_MODE0
		#define VRAMX 41
		extern void* vram[VRAMY * VRAMX] __attribute__ ((aligned (4)));
	#else
			#ifndef VRAMX
				#define VRAMX 42
			#endif
			#ifndef VRAMY
				#define VRAMY 26
			#endif
		#if GFX_MODE == TILE_MODE1
			extern uint16_t vram[VRAMY * VRAMX] __attribute__ ((aligned (4)));
		#elif GFX_MODE == TILE_MODE2
			// 2020/05/08: The +2 entry is to avoid adding complexity to the code when dealing with the very last pixel of the line.
			// In TILE_MODE1, this has been dealt easily by software, adding a BEQ instruction (only 1 cycle penalty, many cycles saved in the other case)
			// Here we preferred to waste 4 bytes of ram, to avoid errors.
			extern uint16_t vram[VRAMY * VRAMX + 2] __attribute__ ((aligned (4)));
			extern uint32_t palette[MAX_NUMBER_OF_PALETTES * 256];
		#endif
		#if USE_SECTION == TOP_FIXED_SECTION
			#define LINELIMITSTR TOSTRING((SECTION_LIMIT * 8 - 1))
/*			Note: these must be implemented on your code
			extern uint8_t fixedSectionMap[VRAMX * SECTION_LIMIT] __attribute__ ((aligned (4)));
			#if GFX_MODE != TILE_MODE2
				extern uint32_t fixedSectionTiles[MAX_FIXED_SECTION_TILES][TILE_SIZE_X/4*TILE_SIZE_Y] __attribute__ ((aligned (4)));
			#else
				extern uint32_t fixedSectionTiles[MAX_FIXED_SECTION_TILES][TILE_SIZE_X/8*TILE_SIZE_Y] __attribute__ ((aligned (4)));
			#endif			
*/
		#elif USE_SECTION == BOTTOM_FIXED_SECTION
			#define LINELIMITSTR TOSTRING((SECTION_LIMIT * 8 ))
		
/*			Note: these must be implemented in your code!
			#if GFX_MODE != TILE_MODE2
				extern uint32_t fixedSectionTiles[MAX_FIXED_SECTION_TILES][TILE_SIZE_X/4*TILE_SIZE_Y] __attribute__ ((aligned (4)));
			#else
				extern uint32_t fixedSectionTiles[MAX_FIXED_SECTION_TILES][TILE_SIZE_X/8*TILE_SIZE_Y] __attribute__ ((aligned (4)));
			#endif
			extern uint8_t fixedSectionMap[VRAMX * (VRAMY - SECTION_LIMIT - 1)] __attribute__ ((aligned (4))); 
*/
		#endif 
	#endif
	#if GFX_MODE == TILE_MODE_0 || GFX_MODE == TILE_MODE1
		extern uint32_t tiles[MAX_TILES][TILE_SIZE_X/4*TILE_SIZE_Y] __attribute__ ((aligned (4)));
	#elif GFX_MODE == TILE_MODE2
		extern uint32_t tiles[MAX_TILES][TILE_SIZE_X/8*TILE_SIZE_Y] __attribute__ ((aligned (4)));
	#endif
	//	
	uint32_t putSprite(uint16_t num, int32_t x, int32_t y, uint16_t flags, uint16_t frameNum);	// put a sprite in the list of sprites to be drawn. NOTE! this does not actually draw the sprites! drawSprites must be called.
	void putCharInTile(const uint8_t (*font)[128][8], uint8_t c, uint8_t color, uint8_t backGroundColor, uint8_t flags ,uint8_t *pTile, int yOffset);		// put one char in the specified ram tile
	void drawSprites();																										// draw the sprites in the list
	void removeAllSprites(uint8_t redrawScreen);	
	void freeSpriteTiles(void);																		// remove all the sprites in the list, and optionally restores the VRAM (useful if the vram is not later updated by other functions).
	void drawFixedSection();																								// copy the fixed section map to the bottom or top part of the vram.
	void restoreBackgroundTiles(void);																						// restores the background tiles, i.e cancelling the sprites. This does not clear the sprite list.
#endif
typedef struct
{
	#if GFX_MODE == TILE_MODE_BOOT
		// important! For TILE_MODE_BOOT, the ordering is important. That's why we explicitly write everything in a separate clause
		uint32_t * pPort;
		uint32_t  blackPixel;
		uint8_t *pVram;
		uint16_t * pTiles;
		volatile uint32_t currentLineNumber;
		uint8_t *ptrRowRemapTable;
	#else
		volatile uint32_t currentLineNumber;
		#if	USE_ROW_REMAPPING == 1
			uint8_t *ptrRowRemapTable;
		#endif
		#if PER_LINE_X_SCROLL == 1
			uint8_t *ptrRowXScroll;
		#endif
		#if PER_TILE_X_SCROLL == 1
			uint8_t *ptrTileXScroll;
		#endif
		#if PER_TILE_X_SCROLL == 0 || PER_LINE_X_SCROLL == 0
			uint32_t xScroll;
		#endif
		uint32_t yScroll;
		#if GFX_MODE == BITMAPPED_MODE
			uint32_t * pPixel;
			uint32_t * pPalette;
			uint32_t * pPort;
			#if PER_HORIZONTAL_BLOCK_PALETTE_REMAP
				uint8_t * pPaletteIndexTable;
			#endif
		#elif GFX_MODE == TILE_MODE2
			uint32_t * pPalette;							// pointer to a contiguous array of palettes (at least one palette -1kB)
			uint32_t * pPort;								// pointer with the fast port register address preloaded 
		#elif GFX_MODE == TILE_MODE1
			uint16_t * pPort;								// pointer with the fast port register address preloaded	
		#endif
		#if (GFX_MODE == TILE_MODE2)
			#if ENABLE_PALETTE_REMAPPING 
				uint8_t *pPaletteIdx;						// pointer to an array with at least 200 bytes (200 nibbes + 200 nibbles) containing the palette to change (4MSB) and the palette to show (4LSB) in the curret line. If highres 400 bytes are required.
				#if ENABLE_PER_LINE_COLOR_REMAPPING
					uint8_t *pNewColorChangeTable;			// pointer to a 200-byte array (at least) containing the new 8-bit color value. In high res mode, at least 400 bytes are required.
					uint8_t *pNewColorChangeIndexTable;		// depending on the mode, this might be a 100-byte array (200 nibbles) up to 400-byte array.
				#endif
				#if ENABLE_PALETTE_ROW_REMAPPING
					int16_t *pPaletteRemappingRowOffsets;		// pointer to an array, which allows to add offsets (positive or negative) to the currentLine number, for per_line_color_remapping. This allows to use ROM (instead of RAM) arrays for pPaletteIdx, pNewColorChangeTable and  pNewColorChangeIndexTable
				#endif			
			#endif
		#endif
		#if (GFX_MODE == TILE_MODE1 || GFX_MODE == TILE_MODE2) && ENABLE_TILE_PRIORITY 
			#if ((VRAMX * VRAMY) & 31)
					__attribute__((__aligned__(4))) uint8_t tilePriority[(VRAMX * VRAMY / 32) * 4 + 4];
			#else
					__attribute__((__aligned__(4))) uint8_t tilePriority[(VRAMX * VRAMY / 8)];
			#endif
		#endif
		#if GFX_MODE != BITMAPPED_MODE
			uint16_t ramTiles;
			uint8_t spriteTilesRemoved;
		#endif
		volatile uint8_t currentFrame;	
	#endif
} videoData_t;
extern videoData_t videoData;
inline uint32_t getCurrentScanLineNumber() {return videoData.currentLineNumber; }
#if GFX_MODE != BITMAPPED_MODE
	inline void placeTile(int x, int y, uint32_t n) {vram[x + y * VRAMX] = (uint32_t) &tiles[n];}
	#if USE_ROW_REMAPPING
		inline void setRowRemapTablePtr(const uint8_t* rowRemapTable) { videoData.ptrRowRemapTable = (uint8_t*) rowRemapTable; };
	#endif		
	#if GFX_MODE != TILE_MODE_BOOT
		inline void setNumberOfRamTiles(uint16_t ramTiles) {videoData.ramTiles = ramTiles;}
		inline uint16_t getNumberOfRamTiles() { return videoData.ramTiles;}
		#if PER_LINE_X_SCROLL == 0 || PER_TILE_X_SCROLL == 0
			inline void setXScroll(uint8_t scroll) { videoData.xScroll = scroll;}
		#endif
		#if PER_TILE_X_SCROLL
			inline void setPerTileXScrollPtr(const uint8_t *ptr) {videoData.ptrTileXScroll = (uint8_t*) ptr;}
			inline void setPerTileXScroll(uint8_t tileRow, uint8_t scroll) {videoData.ptrTileXScroll[tileRow] = scroll;}
		#endif
		#if PER_LINE_X_SCROLL
			inline void setPerLineXScrollPtr(const uint8_t *ptr) {videoData.ptrRowXScroll = (uint8_t*) ptr;}		
			inline void setPerLineXScroll(uint8_t line, uint8_t scroll) {videoData.ptrRowXScroll[line] = scroll;}	
		#endif
		
		#if (GFX_MODE == TILE_MODE2)
			#if ENABLE_PALETTE_REMAPPING 
				inline void setPaletteIndexPtr(const uint8_t* paletteIndexPtr) {videoData.pPaletteIdx = (uint8_t*) paletteIndexPtr; }
				#if ENABLE_PER_LINE_COLOR_REMAPPING
					inline void setNewColorChangeTablePtr(const uint8_t* ptr) { videoData.pNewColorChangeTable = (uint8_t*) ptr; }
					inline void setNewColorChangeIndexTablePtr(const uint8_t* ptr) { videoData.pNewColorChangeIndexTable = (uint8_t*) ptr; }
				#endif
				#if ENABLE_PALETTE_ROW_REMAPPING
					inline void setPaletteRemappingRowOffsetsPtr(const int16_t* ptr) { videoData.pPaletteRemappingRowOffsets = (int16_t*) ptr; }
				#endif			
			#endif
		#endif		
		inline void setYScroll(uint8_t scroll) { videoData.yScroll = scroll;}
	#endif
#endif
uint8_t checkForNewFrame(void);
void waitForVerticalBlank(void);
//
#endif /* VGA_H_ */