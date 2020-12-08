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
*  sprites.h.  definition of frame_t and, in future other useful structures
*/

#ifndef SPRITES_H_
#define SPRITES_H_
#include <stdint.h>
typedef struct
{
	uint8_t w;  //width
	uint8_t h;  //height
	int8_t ox;  //offset x - for handle
	int8_t oy;  //offset y - for handle
	const uint8_t* pData;  //pointer to the topleft pixel
} frame_t;
#endif /* SPRITES_H_ */