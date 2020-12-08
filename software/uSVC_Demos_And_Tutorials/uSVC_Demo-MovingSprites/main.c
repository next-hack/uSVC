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
#define MAX_SPRITES 66
#include "main.h"
typedef struct  
{
	int16_t cx;	// 10.6
	int16_t cy;	// 10.6
	int8_t vx; // 2.6
	int8_t vy;	// 2.6
	uint8_t flags;
} sprites_t;
sprites_t sprites[MAX_SPRITES];
void drawMap(uint16_t xOffset, uint16_t yOffset, uint8_t forceRedraw)
{
	static uint16_t m_old_xStart = 0xFFFF;
	static uint16_t m_old_yStart = 0xFFFF;
	// first, restore the background tiles. The sprites will be redrawn later.
	restoreBackgroundTiles();
	// then copy the tiles required.
	// we just need to copy part of the map in vram.
	setXScroll(xOffset & (2 * TILE_SIZE_X - 1));
	setYScroll(yOffset & (TILE_SIZE_Y - 1));
	if (xOffset >=  (MAPSIZEX * TILE_SIZE_X - SCREEN_SIZE_X ))
	{	// this is the last condition for the very last pixel.
		setXScroll(16);
		xOffset = MAPSIZEX * TILE_SIZE_X - VRAMX * TILE_SIZE_X;
	}
	else if (xOffset >= MAPSIZEX * TILE_SIZE_X - VRAMX * TILE_SIZE_X)
	{
		xOffset = MAPSIZEX * TILE_SIZE_X - VRAMX * TILE_SIZE_X;
	}
	if (yOffset >= (MAPSIZEY * TILE_SIZE_Y - SCREEN_SIZE_Y ))
	{
		setYScroll(8);
		yOffset = MAPSIZEY * TILE_SIZE_Y - VRAMY * TILE_SIZE_Y;
	}
	else if (yOffset >= MAPSIZEY * TILE_SIZE_Y - VRAMY * TILE_SIZE_Y)
	{
		yOffset = MAPSIZEY * TILE_SIZE_Y - VRAMY * TILE_SIZE_Y;
	}
	uint16_t xStart = (xOffset >> (LOG2_TILE_SIZE_X + 1)) * 2;
	uint16_t yStart = yOffset >> LOG2_TILE_SIZE_Y;
	if (m_old_xStart != xStart || m_old_yStart != yStart || forceRedraw)
	{
		uint16_t *dst = &vram[0];
		uint16_t tileAddr =  (uint32_t) &tiles[0];
		const uint16_t *src = &demoMap[xStart + yStart * MAPSIZEX];
		for (int y = 0; y < VRAMY; y++)
		{
			for (int x = 0; x < VRAMX; x+=2)
			{
				// this generates a faster code.
				*dst++ = tileAddr + (*src++) * 64; 
				*dst++ = tileAddr + (*src++) * 64 ;
// 				placeTile(x, y, demoMap[x + xStart + (y + yStart) * MAPSIZEX]);
// 				placeTile(x + 1, y, demoMap[x + 1 + xStart + (y + yStart) * MAPSIZEX]);
			}
			src += MAPSIZEX - VRAMX;
		}
		m_old_xStart = xStart;
		m_old_yStart = yStart;
	}
}
int main(void) 
{	
	// NOTE! The videoData structure must be correctly initialized BEFORE Calling initUsvc!
	// Note: if audio is enabled, replace NULL with the patches array!
	initUsvc(NULL);	  	// Init uSVC Hardware
	uint32_t lastTime = millis();
	int testLed = 0;
	/************************************************************************/
	/*  SETUP                                                               */
	/************************************************************************/
	setNumberOfRamTiles(RAMTILES);
	memcpy (tiles, tileData, 64 * RAMTILES);  // copy tiles
	// setup initial sprite positions
	for (int n = 0; n < MAX_SPRITES; n++)
	{
		sprites[n].cx = ( 66 + (n % 10) * 24) << 6;
		sprites[n].cy =  (17 +(n / 10) * 24) << 6;
		
	}
	uint32_t frame = 0;
//	drawMap( 0, 0, 1);
	while (1)   // Game main loop.
	{
		frame++;
		uint32_t timeNow = millis();
		waitForVerticalBlank();
		// optional, but recommended, led blinker to show that the program is alive!
		if (timeNow - lastTime > 1000UL)
		{
			// row below commented because we use the LED to indicate frame time overflow.
//			setLed(testLed);
			testLed = 1 - testLed;
			lastTime = timeNow;
		}
		// remove all sprites
		SysTick->CTRL = 0;
		SysTick->LOAD = 0xFFFFFF;
		SysTick->VAL = 0;
		SysTick->CTRL = 1;
		removeAllSprites(0);
		//  animate tiles!
		uint32_t *src = (uint32_t*)(&tileData[8 + (frame % 4) * 32]);
		uint32_t *dst = (uint32_t*) & tiles[8];
		fastAlignedMemCpy32(dst, src, 64);
		// end animate tiles
		int16_t angle = (2 * frame) % NUMBER_OF_SINTABLE_ENTRIES;
		// move the camera as a circle.
		int16_t cameraX = 160 + (( 160 * sinTable[angle]) >> 15); 
		int16_t cameraY = 160 + (( 160 * sinTable[(angle + NUMBER_OF_SINTABLE_ENTRIES / 4) % NUMBER_OF_SINTABLE_ENTRIES]) >> 15);

		drawMap( cameraX, cameraY, 0);
		
		for (int i = 0; i < MAX_SPRITES; i++)
		{
			// each 64 frames (about 1 second) change speed and direction
			if (frame % 128 == 127)	
			{
				sprites[i].vx =  rand() % 64 - 32;
				sprites[i].vy =  rand() % 64 - 32;
				sprites[i].flags = rand() & 0x7;

			}
			// each frame update position
			sprites[i].cx += sprites[i].vx;
			sprites[i].cy += sprites[i].vy;
			// boundary check
			if (sprites[i].cx > (320 - 16) * 64)
			{
				sprites[i].vx = -sprites[i].vx;
				sprites[i].cx = (320 - 16) * 64;
			}
			else if (sprites[i].cx < 0)
			{
				sprites[i].vx = -sprites[i].vx;
				sprites[i].cx = 0;
			}

			if (sprites[i].cy > (200 - 16) * 64)
			{
				sprites[i].vy = -sprites[i].vy;
				sprites[i].cy = (200 - 16) * 64;
			}
			else if (sprites[i].cy < 0)
			{
				sprites[i].vy = -sprites[i].vy;
				sprites[i].cy = 0;
			}
			putSprite(i, sprites[i].cx >> 6, sprites[i].cy >> 6, SPRITE_FLAGS_HANDLE_CENTER | sprites[i].flags, 0);
		}
		drawSprites();
		// did we exceed our time?
		uint32_t time = 0xFFFFFF - SysTick->VAL;
		if (time > 124 * 1600)	// each line is 1600 clock cycles.
			setLed(1);
		else
			setLed(0);		
	}
}