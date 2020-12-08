/*	USVC Game Loader and libraries
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
 */
#ifndef LOGOTILES_H_
#define LOGOTILES_H_

#include <stdint.h>
#define MAXTILEINDEX 13
#define TILESIZEX 8
#define TILESIZEY 8
extern const uint8_t tilePalette[];
extern const uint8_t tileData[MAXTILEINDEX][TILESIZEX * TILESIZEY / 4 ];

#endif /* LOGOTILES_H_ */