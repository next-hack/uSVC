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
#ifndef LEVELS_H_
#define LEVELS_H_ 
#include "main.h"
level_t currentLevelData;
const customLevelData_t customLevelData[] =
{
	{
		//.pCustomDataHandler =  customDataHandler,
		.paletteRowRemap = level0_paletteRowRemap,
		.rowPaletteIndexes = level0_rowPaletteIndexes,
		.colorToChangeIndex = level0_colorToChangeIndex,
		.newColorTable = level0_newColorTable,
		.checkpointData = level0_checkpointData,
		.explosionColors = NULL,	
	},
};
const level_t levels[] =
{
	{
		.pGameMap = (uint16_t*) gameMap_level1,
		.pMetaTiles = metaTiles_level1,
		.mapSizeX = MAPSIZEX_LEVEL1,
		.mapSizeY = MAPSIZEY_LEVEL1,
		.pTiles = &tileData[0][0], // note change this if you are using a different tile set for each level!
		.numberOfTilesToCopyInRam = RAMTILES, // note change this if you are using a different tile set for each level!
		.pixelSizeX = TILE_SIZE_X * MAPSIZEX_LEVEL1 * 2,
		.pixelSizeY = TILE_SIZE_Y * MAPSIZEY_LEVEL1 * 2,
		.tileSizeX = TILE_SIZE_X * 2,
		.tileSizeY = TILE_SIZE_Y * 2,
		.useMetaTiles = 1,	
		.useMetaTiles = 1,
		.customData = &customLevelData[0],
	},	
};

void changeLevel(uint16_t levelNumber)
{
	// This operation might take a while. To avoid displaying rubbish we firstly clear a tile, then we clear VRAM
	memset(&tiles[0], 0, sizeof(tiles[0]));
	for (int i = 0; i < VRAMX * VRAMY; i++)
		vram[i] = (uint32_t) &tiles[0];
	// now the screen is black
	currentLevel = levelNumber;
	videoData.ramTiles = levels[levelNumber].numberOfTilesToCopyInRam;
	#if (GFX_MODE != TILE_MODE2) 
		memcpy(&tiles[0], levels[levelNumber].pTiles, TILE_SIZE_X * TILE_SIZE_Y * videoData.ramTiles );
	#else
		memcpy(&tiles[0], levels[levelNumber].pTiles, TILE_SIZE_X * TILE_SIZE_Y / 2 * videoData.ramTiles);
	#endif
#if LEVELS_CAN_SPECIFY_CUSTOM_ADDITIONAL_DATA && LEVELS_CAN_SPECIFY_CUSTOM_FUNCTION_HANDLER
	if (levels[levelNumber].customDataHasHandlerFunction && levels[levelNumber].customData != NULL)
	{
		((customDataHandler_t*)levels[levelNumber].customData)(levelNumber);		
	}
#endif
	memcpy (&currentLevelData, &levels[levelNumber],sizeof (currentLevelData));
}
void changeLevelEx(uint16_t levelNumber, uint32_t reservedWorkTiles)
{
	changeLevel(levelNumber);
	videoData.ramTiles += reservedWorkTiles;
}
int getNumberOfLevels()
{
	return sizeof(levels)/sizeof(level_t);
}
#endif /* LEVELS_H_ */