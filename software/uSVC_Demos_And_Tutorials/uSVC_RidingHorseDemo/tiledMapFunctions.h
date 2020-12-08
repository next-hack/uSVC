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
#ifndef TILEDMAPFUNCTIONS_H_
#define TILEDMAPFUNCTIONS_H_
#include <stdint.h>
#include "level.h"
// uint16_t getMapTileFromCoordinates(uint8_t level, int16_t coordX, int16_t coordY);
// uint16_t getMapTileFromTileCoordinates(uint8_t levelN,uint16_t xTile,  uint16_t yTile);
int32_t getMapNumTileX();
int32_t getMapNumTileY();
void drawMap(uint16_t xOffset, uint16_t yOffset, uint8_t forceRedraw);
static inline uint16_t getMapTileFromTileCoordinates(uint16_t xTile,  uint16_t yTile)
{
	if (currentLevelData.useMetaTiles)
	{
		uint16_t metaNumber = currentLevelData.pGameMap[(xTile >> 1) + (yTile >> 1) * currentLevelData.mapSizeX ];
		return currentLevelData.pMetaTiles[metaNumber * 4 + (xTile & 1) + 2 * (yTile & 1)];
	}
	else
	return currentLevelData.pGameMap[xTile + yTile * currentLevelData.mapSizeX];
}
static inline uint16_t getMapTileFromTileCoordinatesNoMetaTiles(uint16_t xTile,  uint16_t yTile)
{
	return currentLevelData.pGameMap[xTile + yTile * currentLevelData.mapSizeX];
}
static inline uint16_t getMapTileFromTileCoordinatesWithMetaTiles(uint16_t xTile,  uint16_t yTile)
{
	uint16_t metaNumber = currentLevelData.pGameMap[(xTile >> 1) + (yTile >> 1) * currentLevelData.mapSizeX ];
	return currentLevelData.pMetaTiles[metaNumber * 4 + (xTile & 1) + 2 * (yTile & 1)];
}


#endif /* TILEDMAPFUNCTIONS_H_ */