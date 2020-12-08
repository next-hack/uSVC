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
 * tiledMapFunctions.h/c: some utility functions used to deal with tiles.
 * Note: non all the functions here are used, and the compiler will strip off those not used.
 * Actually this file can (and is) used in other USVC games.
 */
#ifndef TILEDMAPFUNCTIONS_H_
#define TILEDMAPFUNCTIONS_H_
#include <stdint.h>
uint16_t getMapTileFromCoordinates(uint8_t level, int16_t coordX, int16_t coordY);
uint16_t getMapTileFromTileCoordinates(uint8_t levelN,uint16_t xTile,  uint16_t yTile);
int32_t getMapNumTileX();
int32_t getMapNumTileY();
void drawMap(uint16_t xOffset, uint16_t yOffset, uint8_t forceRedraw);



#endif /* TILEDMAPFUNCTIONS_H_ */