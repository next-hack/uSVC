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
*  audioBootloader.c/h. Very simple audio engine for the booloader.
*
* This reduced sound engine is used to play the initial logo sound, and the menu effects, therefore it is very primitive.
* To save on flash, the soundWave is created in RAM when this module is initialized, and it is modified using some
* params. In case of lack of space, in the future, the parameters might be hardcoded.
*/
#ifndef AUDIOBOOTLOADER_H_
#define AUDIOBOOTLOADER_H_
#include <stdint.h>
void bootloaderStartSong(const uint8_t *song);
void bootloaderSoundEngine(void);
void initBootloaderAudio(void);
#define BL_PLAY_NOTE (0 << 2)
#define BL_NOTE_A5 0x01E1 // Note 57 (A-5), frequency 220.000000, Step 1.878906
#define BL_NOTE_AS5	0x01FD // Note 58 (A#-5), frequency 233.081881, Step 1.988281
#define BL_NOTE_B5	0x021B // Note 59 (B-5), frequency 246.941651, Step 2.105469
#define BL_NOTE_C6	0x023C // Note 60 (C-6), frequency 261.625565, Step 2.234375
#define BL_NOTE_CS6	0x025E // Note 61 (C#-6), frequency 277.182631, Step 2.367188
#define BL_NOTE_D6	0x0282 // Note 62 (D-6), frequency 293.664768, Step 2.507813
#define BL_NOTE_C7 0x0477
#define BL_SET_ENV_DATA (1 << 2)
#define BL_NOTE(x) (x & 255 ), ((x >> 8) & 255 )

#endif /* AUDIOBOOTLOADER_H_ */