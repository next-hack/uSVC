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
#include "usvc_kernel.h"
#if USE_BL_AUDIO
#define MAX_CHANNELS 4
#define SOUNDWAVE_SIZE 256
uint8_t ramSoundWave[SOUNDWAVE_SIZE];
typedef struct  
{
	uint8_t channelEnvVolume[MAX_CHANNELS];
	int8_t channelEnvDelta[MAX_CHANNELS];
	uint8_t nextTime;
	uint8_t time;
	const uint8_t *songPtr;	
	uint8_t songActive;
}bootloaderAudio_t;
bootloaderAudio_t blAudio;
void bootloaderSoundEngine(void)
{
	if (blAudio.songActive == 0)
		return;
	#if BL_AUDIO_HAS_4_CHANNELS
	for (uint8_t ch=0; ch < 4; ch++)
	{
	#else
 	{	
		uint8_t ch = 0;
	#endif
		// perform envelope variation
		int fltr = audioMixerData.filterValue;
		fltr += 8;
		if (fltr > 16383)
			fltr = 16383;
		audioMixerData.filterValue = fltr;
		int vol = audioMixerData.channels[ch].volume;
		vol += blAudio.channelEnvDelta[ch];
		if (vol > 255)
			vol = 255;
		else if (vol < 0)
			vol = 0;
		audioMixerData.channels[ch].volume = vol;
	}
	while (blAudio.nextTime == blAudio.time)
	{
		uint8_t effect = *blAudio.songPtr++;
		#if BL_AUDIO_HAS_4_CHANNELS
		uint8_t channel = effect & 3;
		#else
			uint8_t channel = 0;
		#endif
		uint8_t parameterLo = *blAudio.songPtr++;
		uint8_t parameterHi= *blAudio.songPtr++;
		uint8_t delta = *blAudio.songPtr++;
		blAudio.nextTime += delta;
		switch (effect & 0xFC)
		{
			case BL_SET_ENV_DATA:
				blAudio.channelEnvDelta[channel] = parameterHi;
				blAudio.channelEnvVolume[channel] = parameterLo;
				break;
			case BL_PLAY_NOTE:
				audioMixerData.channels[channel].volume = blAudio.channelEnvVolume[channel];	
//				audioMixerData.filterValue = 3400;
				audioMixerData.filterValue = 16384 * 3400 / 4096;			
				audioMixerData.channels[channel].increment =  (parameterHi << 8) | parameterLo;
				audioMixerData.channels[channel].length = SOUNDWAVE_SIZE << 8;
				audioMixerData.channels[channel].index = 0; // start from the beginning
				audioMixerData.channels[channel].delta = SOUNDWAVE_SIZE << 8;			
				audioMixerData.channels[channel].offset = (uint32_t) ramSoundWave;
				break;
		}
		if (delta == 255)
		{
			blAudio.songActive = 0;
			#if BL_AUDIO_HAS_4_CHANNELS
			for (uint8_t ch = 0; ch < MAX_CHANNELS; ch++)	// stop channels
				audioMixerData.channels[ch].increment = 0;
			#else
					audioMixerData.channels[0].increment = 0;
			#endif
		}
	}
	blAudio.time++;
}
void initBootloaderAudio(void)
{
	// to save on flash, we will use only one base wave, which will be then modified run-time
	for (int i = 0; i < SOUNDWAVE_SIZE; i++)
	{
		ramSoundWave[i] =  i * 256 / SOUNDWAVE_SIZE - 128;
	}	
		blAudio.nextTime = 0;
		blAudio.time = 0;
}
void bootloaderStartSong(const uint8_t *song)
{
	blAudio.nextTime = blAudio.time + *song;
	blAudio.songPtr = song + 1;
	blAudio.songActive = 1;
}
#endif