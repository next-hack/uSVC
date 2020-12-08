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
#include "usvc_kernel/usvc_kernel.h" 
#include "tiledMapFunctions.h"
#include "level.h"
int32_t getMapNumTileX()
{
	#if LEVELS_CAN_USE_META_TILE
		return levels[currentLevel].mapSizeX * (1 + (levels[currentLevel].useMetaTiles != 0));
	#else
		return levels[currentLevel].mapSizeX;
	#endif
}
int32_t getMapNumTileY()
{
#if LEVELS_CAN_USE_META_TILE
	return levels[currentLevel].mapSizeY * (1 + (levels[currentLevel].useMetaTiles != 0));
#else
	return levels[currentLevel].mapSizeY;
#endif
}
// uint16_t getMapTileFromTileCoordinates(uint8_t levelN,uint16_t xTile,  uint16_t yTile)
// {
// 	const level_t *pLevel = &levels[levelN];
// #if LEVELS_CAN_USE_META_TILE
// 	if (pLevel->useMetaTiles)
// 	{
// 		uint16_t metaNumber = pLevel->pGameMap[(xTile >> 1) + (yTile >> 1) * pLevel->mapSizeX ];
// 		return pLevel->pMetaTiles[metaNumber * 4 + (xTile & 1) + 2 * (yTile & 1)];
// 	}
// 	else
// #endif
// 		return pLevel->pGameMap[xTile + yTile * pLevel->mapSizeX];
// }
// uint16_t getMapTileFromCoordinates(uint8_t level, int16_t coordX, int16_t coordY)
// {
// 	return getMapTileFromTileCoordinates(level, coordX >> LOG2_TILE_SIZE_X, coordY >> LOG2_TILE_SIZE_Y);
// }
void drawMap(uint16_t xOffset, uint16_t yOffset, uint8_t forceRedraw)
{
	#if GFX_MODE == TILE_MODE1 || GFX_MODE == TILE_MODE2
	static uint16_t m_old_xStart = 0xFFFF;
	static uint16_t m_old_yStart = 0xFFFF;
	// first, restore the background tiles. The sprites will be redrawn later.
	restoreBackgroundTiles();
	// then copy the tiles required.
	// we just need to copy part of the map in vram.
	videoData.xScroll = xOffset & (2 * TILE_SIZE_X - 1);
	videoData.yScroll= yOffset & (TILE_SIZE_Y - 1);
	if (xOffset >=  (currentLevelData.pixelSizeX - SCREEN_SIZE_X ))
	{	// this is the last condition for the very last pixel.
		videoData.xScroll = 16;
		xOffset = currentLevelData.pixelSizeX - VRAMX * TILE_SIZE_X;
	}
	else if (xOffset >= (currentLevelData.pixelSizeX) - VRAMX * TILE_SIZE_X)
	{
		xOffset = currentLevelData.pixelSizeX - VRAMX * TILE_SIZE_X;
	}
	if (yOffset >= (currentLevelData.pixelSizeY - SCREEN_SIZE_Y ))
	{
		videoData.yScroll = 8;
		yOffset = currentLevelData.pixelSizeY - VRAMY * TILE_SIZE_Y;
	}
	else if (yOffset >= (currentLevelData.pixelSizeY ) - VRAMY * TILE_SIZE_Y)
	{
		yOffset = currentLevelData.pixelSizeY - VRAMY * TILE_SIZE_Y;
	}
	uint16_t xStart = (xOffset >> (LOG2_TILE_SIZE_X + 1)) * 2;
	uint16_t yStart = yOffset >> LOG2_TILE_SIZE_Y;
	if (m_old_xStart != xStart || m_old_yStart != yStart || forceRedraw)
	{
		#if	(GFX_MODE == TILE_MODE1 || GFX_MODE == TILE_MODE2) && ENABLE_TILE_PRIORITY
		int tilePos = 0;
		uint32_t *pTilePriority =  (uint32_t*) &videoData.tilePriority[0];
		uint32_t tmpTilePri = 0;
		#endif
		if (currentLevelData.useMetaTiles)
		{
			uint16_t* pVram = &vram[0];
			for (uint32_t y = 0; y < VRAMY; y++)
			{
				uint32_t yPos =  y + yStart;
				uint32_t gameMapPos =  (yPos >> 1) * currentLevelData.mapSizeX;
				for (uint32_t x = 0; x < VRAMX; )
				{
					#if	(GFX_MODE == TILE_MODE1 || GFX_MODE == TILE_MODE2) && ENABLE_TILE_PRIORITY

					int16_t metaNumber = currentLevelData.pGameMap[gameMapPos + ((x + xStart) >> 1)];
					uint16_t tile =  currentLevelData.pMetaTiles[metaNumber * 4 + ((x + xStart) & 1) + 2 * (yPos & 1)];
					// note: since vrram only holds a 16-bit number, and since each tile si 64 bytes long, even if there is the tile priority bit (bit 14)
					// set at 1, this would add a 1048576, which will be discarded later. Therefore we do not perform tile & TILEMASK.
					*pVram++ = (uint32_t) &tiles[tile][0];
					// NOTE!!! In this game there are no "Transparent tiles" so we can save one &1
					tmpTilePri |= (( tile >> 14) << tilePos);
					tilePos++;  // This is the odd pixel. For sure we won't overflow!
					x++;
					tile =  currentLevelData.pMetaTiles[metaNumber * 4 + ((x + xStart) & 1) + 2 * (yPos & 1)];
					// note: since vrram only holds a 16-bit number, and since each tile si 64 bytes long, even if there is the tile priority bit (bit 14)
					// set at 1, this would add a 1048576, which will be discarded later. Therefore we do not perform tile & TILEMASK.
					*pVram++ = (uint32_t) &tiles[tile][0];
					// NOTE!!! In this game there are no "Transparent tiles" so we can save one &1
					tmpTilePri |= (( tile >> 14) << tilePos);
					tilePos++;  // This is the odd pixel. For sure we won't overflow!
					x++;
					if (tilePos == 32)
					{
						tilePos = 0;
						*pTilePriority++ = tmpTilePri;
						tmpTilePri = 0;
					}
					#else
					int16_t metaNumber = currentLevelData.pGameMap[gameMapPos + ((x + xStart) >> 1)];
					uint16_t tile =  currentLevelData.pMetaTiles[metaNumber * 4 + ((x + xStart) & 1) + 2 * (yPos & 1)];
					// note: since vrram only holds a 16-bit number, and since each tile si 64 bytes long, even if there is the tile priority bit (bit 14)
					// set at 1, this would add a 1048576, which will be discarded later. Therefore we do not perform tile & TILEMASK.
					*pVram++ = (uint32_t) &tiles[tile][0];
					// NOTE!!! In this game there are no "Transparent tiles" so we can save one &1
					x++;
					tile =  currentLevelData.pMetaTiles[metaNumber * 4 + ((x + xStart) & 1) + 2 * (yPos & 1)];
					// note: since vrram only holds a 16-bit number, and since each tile si 64 bytes long, even if there is the tile priority bit (bit 14)
					// set at 1, this would add a 1048576, which will be discarded later. Therefore we do not perform tile & TILEMASK.
					*pVram++ = (uint32_t) &tiles[tile][0];
					// NOTE!!! In this game there are no "Transparent tiles" so we can save one &1
					x++;
					#endif
				}
			}
			
		}
		else
		{
			for (int y = 0; y < VRAMY; y++)
			{
				for (int x = 0; x < VRAMX; x++)
				{
					#if	(GFX_MODE == TILE_MODE1 || GFX_MODE == TILE_MODE2) && ENABLE_TILE_PRIORITY
					uint16_t tile =  getMapTileFromTileCoordinatesNoMetaTiles(x + xStart, y + yStart);
					*((uint16_t*)(&vram[x + y * VRAMX])) = (uint32_t) &tiles[tile & TILEMASK][0];
					//
					tmpTilePri |= ((( tile >> 14) & 1) << tilePos);
					tilePos++;
					if (tilePos == 8)
					{
						tilePos = 0;
						*pTilePriority++ = tmpTilePri;
						tmpTilePri = 0;
					}
					#else
					*((uint16_t*)(&vram[x + y * VRAMX])) = (uint32_t) &tiles[getMapTileFromTileCoordinatesNoMetaTiles(x + xStart, y + yStart)][0];
					#endif
				}
			}
		}
		#if	(GFX_MODE == TILE_MODE1 || GFX_MODE == TILE_MODE2) && ENABLE_TILE_PRIORITY
		// store last tilePri word if it was not complete
		if (tilePos != 0)
		{
			*pTilePriority++ = tmpTilePri;
		}
		#endif
		m_old_xStart = xStart;
		m_old_yStart = yStart;
	}
	#endif
	videoData.spriteTilesRemoved = 1;
}
