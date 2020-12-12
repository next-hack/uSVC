/*
*  Audio.c: a port of UZEBOX's audio system for uChip Simple VGA Console (USVC) Kernel.
*
*  Original Sound and Music Engine: Alec Bourque (Belogic Software, UzeBox project)
*
*  Audio Mixer (see audio.h): Nicola Wrachien (next-hack.com)
*  Additional patch commands:  Nicola Wrachien (next-hack.com)
*  USVC port and adaptation:   Nicola Wrachien (next-hack.com)
*
*  uChip Simple VGA Console Kernel,
*
*  Copyright 2019-2020 Nicola Wrachien (next-hack.com)
*  Original sound and Music Engine: Copyright 2008-2020: Alec Bourque
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
*/
#ifndef AUDIO_H_
#define AUDIO_H_
#define NUMBER_OF_AUDIO_CHANNELS 4
#define SAMPLE_RATE 30000
#include <stdint.h>
void initAudioMixer();
// Credits: UzeBox 
#define MIDI	0
#define MOD	1
#define STREAM	2
#define NO_MUSIC_ENGINE 3
//Patch commands
#define PC_ENV_SPEED	 0
#define PC_NOISE_PARAMS	 1
#define PC_WAVE			 2
#define PC_NOTE_UP		 3
#define PC_NOTE_DOWN	 4
#define PC_NOTE_CUT		 5
#define PC_NOTE_HOLD 	 6
#define PC_ENV_VOL		 7
#define PC_PITCH		 8
#define PC_TREMOLO_LEVEL 9
#define PC_TREMOLO_RATE	10
#define PC_SLIDE		11
#define PC_SLIDE_SPEED	12
#define PC_LOOP_START	13
#define PC_LOOP_END		14
#define PC_SAMPLE_RATE_NORMALIZE 15			// adjust the increment so that the sample will play exactly the specified note
#define PC_SAMPLE_RATE_SPECIFY 16			// adjust the increment so that the sample will play nominally at the specified sample rate.
#define PC_SET_FILTER_VALUE 17
#define PATCH_END		0xff
#define TRACK_FLAGS_SLIDING		8
#define TRACK_FLAGS_ALLOCATED	16
#define TRACK_FLAGS_PLAYING		32
#define TRACK_FLAGS_HOLD_ENV	64
#define TRACK_FLAGS_PRIORITY	128
//
#define FX_FLAGS_RETRIG 1
#define FX_FLAGS_SPECIFY_SAMPLE_FREQUENCY 2
#define FX_FLAGS_SPECIFY_CHANNEL 4
#define FX_FLAGS_SPECIFY_CHANNEL_POS 3
#define FX_FLAGS_SPECIFY_CHANNEL_MASK (0x3 << FX_FLAGS_SPECIFY_CHANNEL_POS) 
#define FX_FLAGS_SPECIFY_CHANNEL_N (FX_FLAGS_SPECIFY_CHANNEL | ((n & 3) << FX_FLAGS_SPECIFY_CHANNEL_POS))
#define FX_FLAGS_SPECIFY_CHANNEL_0 FX_FLAGS_SPECIFY_CHANNEL_N(0)
#define FX_FLAGS_SPECIFY_CHANNEL_1 FX_FLAGS_SPECIFY_CHANNEL_N(1)
#define FX_FLAGS_SPECIFY_CHANNEL_2 FX_FLAGS_SPECIFY_CHANNEL_N(2)
#define FX_FLAGS_SPECIFY_CHANNEL_3 FX_FLAGS_SPECIFY_CHANNEL_N(3)

typedef struct 
{
	uint32_t length;
	int8_t *wData;
	uint16_t sps;
} soundWave_t;			// all waves are PCM type.
typedef struct 
{
		uint8_t flags;		//b0-b2: reserved
		//b3: pitch slide		: 1=sliding to note, 0=slide off
		//b4: allocated 		: 1=used by music player, 0=voice can be controlled by main program
		//b5: patch playing 	: 1=playing, 0=stopped
		//b6: hold envelope		: 1=hold volume envelope, i.e: don't increae/decrease, 0=don't hold
		//b7: priority			: 1=Hi/Fx, 0=low/regular note

		uint16_t note;
		uint8_t channel;

		#if MUSIC_ENGINE == MOD
			const char *patternPos;
		#else
			unsigned char expressionVol;
		#endif
		
		uint8_t loopCount;
		
		int16_t slideStep;		//used to slide to note
		uint8_t  slideNote;		//target note
		uint8_t	slideSpeed;		//fixed point 4:4, 1:0= 1/16 half note per frame

		uint8_t tremoloPos;
		uint8_t tremoloLevel;
		uint8_t tremoloRate;

		uint8_t trackVol;
		uint8_t noteVol;
		uint8_t envelopeVol;		//(0-255)
		int8_t envelopeStep;				//signed, amount of envelope change each frame +127/-128
		
		uint8_t patchNo;
		uint8_t fxPatchNo;
		// modified to uint16_t
		uint16_t patchNextDeltaTime;
		uint16_t patchCurrDeltaTime;
		uint16_t patchPlayingTime;	//used by fx to steal oldest voice
		uint8_t *patchCommandStreamPos;
} Track;
extern Track tracks[NUMBER_OF_AUDIO_CHANNELS];
typedef struct  
{
	unsigned char type;
//	const char *pcmData;
	const soundWave_t * pSoundWaveData;
	const char *cmdStream;
	unsigned int loopStart;
	unsigned int loopEnd;
} PatchStruct;
typedef struct  
{
	uint8_t type;
//	int8_t *pcmData;
	int16_t soundWaveNumber;	
	uint8_t *cmdStream;
	uint32_t loopStart;
	uint32_t loopEnd;
} patch_t;
void triggerNote (unsigned char channel, int16_t patch, unsigned char note, unsigned char volume);
//void TriggerFx (int16_t patch,unsigned char volume, uint8_t retrig);
uint8_t triggerFx (int16_t patch,unsigned char volume, uint8_t flags, uint32_t detuning);
void stopLoopingFx(uint8_t channel, uint8_t abruptStop);
void soundEngine(void);
void initMusicPlayer(const patch_t *patchPointersParam);
void stopSong();
void resumeSong();
void setSongSpeed(uint8_t speed);
uint8_t getSongSpeed();
void setNoteVol(uint8_t channel, uint8_t vol);
void setMixerPlaySpeed(uint8_t channel, uint16_t increment);
#if MUSIC_ENGINE == MIDI
	void startSong(const uint8_t *song);
#elif MUSIC_ENGINE == STREAM
	void startSong(const uint8_t *song);
#elif MUSIC_ENGINE == MOD // MOD
	void startSong (const uint8_t *song, uint16_t startPos, uint8_t loop);
#endif
#endif /* AUDIO_H_ */