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
*  defaultSounds.h headers for the default sounds
*/
#ifndef DEFAULTSOUNDS_H_
#define DEFAULTSOUNDS_H_
#include "stdint.h"
#include "usvc_kernel.h"
#if INCLUDE_DEFAULT_WAVES
#define DEFAULT_SOUND_NUMBER 10
extern const int8_t squareWave50Filtered[];
extern const int8_t sineDistoWave3[];
extern const int8_t sineDistoWave2[];
extern const int8_t sineDistoWave1[];
extern const int8_t squareWave50[];
extern const int8_t squareWave25[];
extern const int8_t squareWave75[];
extern const int8_t triangleWave[];
extern const int8_t sawToothWave[];
extern const int8_t sineWave[];
#endif
#endif /* DEFAULTSOUNDS_H_ */