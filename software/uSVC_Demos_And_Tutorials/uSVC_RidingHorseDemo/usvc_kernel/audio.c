/*
*  Audio.c: a port of UZEBOX's audio system for uChip Simple VGA Console (USVC) Kernel.
*
*  Original Sound and Music Engine: Alec Bourque (Belogic Software, UzeBox project)
*
*  Audio Mixer (see audio.h): Nicola Wrachien (next-hack.com)
*  Additional patch commands:  Nicola Wrachien (next-hack.com)
*  USVC port and adaptation:   Nicola Wrachien (next-hack.com)
*  
*  uChip Simple VGA Console Kernel, a minimalistic console with VGA and USB support using uChip!
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
#include "usvc_kernel.h"
#define DO_NOT_USE_TRACK_ALLOCATION 1
#if DO_NOT_USE_TRACK_ALLOCATION
	#define isTrackAllocated(track) 1
#else
	#define isTrackAllocated(track) (0 != (track.flags & TRACK_FLAGS_ALLOCATED))
#endif
#if USE_MIXER
audioMixer_t audioMixerData;
#if USE_BL_AUDIO || USE_NVM_MODULE
	// this is mandatory during flash writes.
	volatile int8_t silenceValue = 0;	
#else
	const int8_t silenceValue = 0;
#endif
/* About track allocation: in the original code, actually TRACK_FLAGS_ALLOCATED is not used, as it is checked using | instead of &. This makes the 
   statement always true. Furthermore, by changing | with &, the code breaks, unless in triggerNote we change track->flags = 0 to
   track->flags  &= (~TRACK_FLAGS_PRIORITY);// priority=0;  (as it was originally coded, but then commented).
   However, setting to 0 the flags, is much stronger than only clearing the priority bit, as other flags might be still non-zero. Therefore, for this 
   reason, and since actually track allocation is not used, we override the track allocation, so the engine behaves like the UzeBox original code.
*/ 

void initAudioMixer()
{
	// init mixer structure
	memset(&audioMixerData, 0, sizeof(audioMixer_t));
	for (int i = 0; i < NUMBER_OF_AUDIO_CHANNELS; i++)
	{
		audioMixerData.channels[i].offset = (uint32_t) &silenceValue;
	}
	// MANDATORY!
	audioMixerData.dacrAddress = (uint32_t) (&DAC->DATA.reg);
	//audioMixerData.masterVolume = 0;		// zero volume
	audioMixerData.minus4 = -4;
	// Note: DAC clock already configured in init()
	setPortMux(2,PORT_PMUX_PMUXE_B);
	// reset DAC to a known state
	DAC->CTRLA.reg = DAC_CTRLA_SWRST;
	DAC->CTRLB.reg = DAC_CTRLB_REFSEL_INT1V | DAC_CTRLB_BDWP | DAC_CTRLB_EOEN;	// enable output bufer, internal reference, disable write protection, right adjusted
	DAC->CTRLA.reg = DAC_CTRLA_ENABLE | DAC_CTRLA_RUNSTDBY;
	//
	
}
#endif
#if AUDIO_ENABLED == 1
#include <string.h>
#define CONTROLER_VOL 7
#define CONTROLER_EXPRESSION 11
#define CONTROLER_TREMOLO 92
#define CONTROLER_TREMOLO_RATE 100

#define DEFAULT_MASTER_VOL 0xFF
#define DEFAULT_WAVE		7
#define DEFAULT_PATCH		0
#define DEFAULT_TRACK_VOL	0xff
#define DEFAULT_EXPRESSION_VOL 0xff
#define MIDI_NULL 0xfd
typedef void (*patchCommand)(Track* track, int16_t value);  // originally value was a char
void SetTriggerCommonValues(Track *track, uint8_t volume, uint8_t note);


// credit UZEBOX

Track tracks[NUMBER_OF_AUDIO_CHANNELS];
extern const uint16_t stepTable[128];

//Common player vars
uint8_t playSong = 0;
unsigned char lastStatus;
unsigned char masterVolume;
uint8_t songSpeed;
uint8_t step;

#if MUSIC_ENGINE == MIDI

uint16_t readVarLen(const int8_t **songPos);

const int8_t *songPos;
const int8_t *songStart;
const int8_t *loopStart;

uint16_t nextDeltaTime;
uint16_t currDeltaTime;

#elif MUSIC_ENGINE == STREAM

const char *songPos;
const char *loopStart;

uint16_t nextDeltaTime;

#else //MOD player

uint8_t currentTick;
uint8_t currentStep;
uint8_t modChannels;
uint8_t songLength;
const char *songPos;
const char *songStart;
const char *loopStart;
const uint16_t *patternOffsets;
const char *patterns;
#endif
const patch_t *patchPointers;

void setMixerWave(uint8_t channel, uint8_t waveNumber)
{
	if (waveNumber > (getNumberOfSoundWaves() - 1))
		waveNumber = (getNumberOfSoundWaves() - 1); // let's avoid crashes!
	if (channel > (NUMBER_OF_AUDIO_CHANNELS - 1))
		channel = NUMBER_OF_AUDIO_CHANNELS - 1;
	audioMixerData.channels[channel].index = 0;
	audioMixerData.channels[channel].offset = (uint32_t) soundWaves[waveNumber].wData;
	audioMixerData.channels[channel].length = soundWaves[waveNumber].length << 8;
	audioMixerData.channels[channel].delta = soundWaves[waveNumber].length << 8;		// by default waves are repeated
}
void setCorrectSingleShotOperation(Track * track)
{
	// When a sample is not played at the full 30 ksps (i.e. increment = 0x100, 1 sample per horizontal scanline), then the strategy of putting loopStart = sample LENGTH-1 
    // to have a single shot (i.e. no loop) does not work, as the increment, being not exactly 0x100, would always add to the sample pointer something more or less that 1 sample. 
	// In other words, the pointer would not actually stop at a fixed position.
	// here we check if the condition loopStart >= loopEnd -1, and if it's true, we set to increment the "delta" (the value that is subtracted from the sample pointer to wrap up).  
	// check if we need the patch or fxPatch
	uint8_t isFx = (track->flags & TRACK_FLAGS_PRIORITY);
	uint8_t patch = isFx ? track->fxPatchNo : track->patchNo;
	uint32_t loopStart = patchPointers[patch].loopStart;
	uint32_t loopEnd = patchPointers[patch].loopEnd;
	if ((loopStart + 1) >= loopEnd)
		audioMixerData.channels[track->channel].delta = audioMixerData.channels[track->channel].increment;
}
void setMixerNote(uint8_t channel, uint8_t note)
{ 
	if (channel > (NUMBER_OF_AUDIO_CHANNELS - 1))
		channel = NUMBER_OF_AUDIO_CHANNELS - 1;
	if (note == 0x80)	// 128 is reserved to achieve a 1:1 sample
		audioMixerData.channels[channel].increment = 0x100;
	else
	{
		audioMixerData.channels[channel].increment = stepTable[note];		
	}
	setCorrectSingleShotOperation(&tracks[channel]);
}
void setMixerPlaySpeed(uint8_t channel, uint16_t increment)
{
	if (channel > (NUMBER_OF_AUDIO_CHANNELS - 1))
		channel = NUMBER_OF_AUDIO_CHANNELS - 1;
	audioMixerData.channels[channel].increment = increment;
	setCorrectSingleShotOperation(&tracks[channel]);
}
/*
 * Command 00: Set envelope speed per frame +127/-128, 0=no enveloppe
 * Param:
 */
void patchCommand00(Track* track, int16_t param)
{
	track->envelopeStep = param;
}
/*
 * Command 01: Set noise channel params 
 * Param:
 */
void patchCommand01(Track* track, int16_t param)
{
	(void)track; //to remove unused warning
	#if 0	// TODO: implement noise
		#if MIXER_CHAN4_TYPE == 0
		mixer.channels.type.noise.barrel = 0x0101;
		mixer.channels.type.noise.params = param;
		#endif
	#endif
}
/*
 * Command 02: Set wave
 * Param:
 */
void patchCommand02(Track* track, int16_t param)
{
	setMixerWave(track->channel, param);
}
/*
 * Command 03: Note up * param
 * Param:
 */
void patchCommand03(Track* track, int16_t param)
{
	track->note += param;
	setMixerNote(track->channel,track->note);
}
/*
 * Command 04: Note down * param
 * Param:
 */
void patchCommand04(Track* track, int16_t param)
{
	track->note -= param;
	setMixerNote(track->channel, track->note);
}
/*
 * Command 05: End of note/fx
 * Param:
 */
void patchCommand05(Track* track, int16_t param)
{
	(void)param; //to remove unused warning
	track->flags &= ~ (TRACK_FLAGS_PLAYING + TRACK_FLAGS_PRIORITY);	//patchPlaying=false,priority=0	
}

/*
 * Command 06: Note hold
 * Param:
 */
void patchCommand06(Track* track, int16_t param)
{
	(void)param; //to remove unused warning
	track->flags |= TRACK_FLAGS_HOLD_ENV; //patchEnvelopeHold=true;
}

/*
 * Command 07: Set envelope volume
 * Param:
 */

void patchCommand07(Track* track, int16_t param)
{
	track->envelopeVol = param;
}

/*
 * Command 08: Set Note/Pitch
 * Param:
 */

void patchCommand08(Track* track, int16_t param)
{
	setMixerNote(track->channel,param);
	track->note = param;
	track->flags &= ~(TRACK_FLAGS_SLIDING);	
}

/*
 * Command 09: Set tremolo level
 * Param:
*/

void patchCommand09(Track* track, int16_t param)
{
	track->tremoloLevel = param;
}

/*
 * Command 10: Set tremolo rate
 * Param:
*/
void patchCommand10(Track* track, int16_t param)
{
	track->tremoloRate = param;
}


/*
 * Command 11: Pitch slide (linear) 
 * Param: (+/-) half steps to slide to
*/

void patchCommand11(Track* track, int16_t param)
{
	//slide to note from current note
	int16_t currentStep,targetStep,delta;	
	
	currentStep = stepTable[track->note];
	targetStep = stepTable[track->note + param];	
	delta = ((targetStep - currentStep) / track->slideSpeed);
	if (delta == 0)
		delta++;

	audioMixerData.channels[track->channel].increment += delta;
	setCorrectSingleShotOperation(track);
	track->slideStep = delta;
	track->flags |= TRACK_FLAGS_SLIDING;
	track->slideNote = track->note + param;
}


/*
 * Command 12: Pitch slide speed 
 * Param: slide speed (fixed 4:4)
 */
void patchCommand12(Track* track, int16_t param)
{
	track->slideSpeed = param;
}

/*
 *  Command 13: Loop start
 * Description: Defines the start of a loop. Works in conjunction with command 14 (PC_LOOP_END).
 *		 Param: loop count
 */
void patchCommand13(Track* track, int16_t param)
{
	track->loopCount=(uint8_t)param;
}

/*
 *  Command 14: Loop end
 * Description: Defines the end of a loop.
 *		 Param: If zero: scan back to find the command next to PC_LOOP_START. This is done 
 *				because we do not store the loop begin adress to save ram. To have maximum
 *				performance, use a non-zero value to explicitely define the number of 
 *				commands to go backwards, no including the LOOP_END.This must be in terms 
 *				of commands and not bytes.
 *	   Example:
 *				const char patch01[] PROGMEM ={	
 *					0,PC_WAVE,4,
 *					0,PC_LOOP_START,50,
 *					1,PC_NOTE_UP,3,
 *					1,PC_NOTE_DOWN,3,
 *					0,PC_LOOP_END,2,
 *					0,PATCH_END  
 *				};
 */
void patchCommand14(Track* track, int16_t param)
{
	if(track->loopCount > 0)
	{
		//track->patchCommandStreamPos=track->loopStart;
		if(param != 0)
		{
			track->patchCommandStreamPos -= ((param+1)*3);
		}
		else
		{
			uint8_t command;
			while(1)
			{
				track->patchCommandStreamPos-=3;
				command=*(track->patchCommandStreamPos -3 + 1);
				//if we found the loop point or somehow reached the previous patch, exit
				if(command == PC_LOOP_START || command == PATCH_END) 
					break;				
			}
		}
		track->loopCount--;
	}
}
void patchCommand15(Track* track, int16_t param)
{	// Adjust the increment so that the sample will play reproducing the required note
	(void) param;		// remove unused warning.
	uint8_t channel = track->channel;
	uint8_t note = track->note;
	// stepTable is calculated with a 256-sample wave. Using a different sample number requires a different stepValue to achieve the same note.
	// steptable is at most 0x6B0A. It means no overflow on a 32-bit uint for samples as large as 156k. These are unreasonably long so we won't put the uint64_t
	//uint64_t increment = stepTable[note] * audioMixerData.channels[channel].length;  //uint64_t to avoid overflows with unreasonably long waves
	//audioMixerData.channels[channel].increment = increment >> 8;		// divide back for 256.
	audioMixerData.channels[channel].increment = stepTable[note] * (audioMixerData.channels[channel].length >> 8) >> 8;		// This is good until 156k samples at the highest frequency.
	setCorrectSingleShotOperation(track); 
}
// PatchCommand16: // adjust the increment so that the sample will play nominally at the specified sample rate.
void patchCommand16(Track* track, int16_t param)
{
	(void) param;		// remove unused warning.
	uint8_t isFx = (track->flags & TRACK_FLAGS_PRIORITY);
	uint8_t patch = isFx ? track->fxPatchNo : track->patchNo;
	audioMixerData.channels[track->channel].increment = ((soundWaves[patchPointers[patch].soundWaveNumber].sps) << 8) / SAMPLE_RATE;	
	setCorrectSingleShotOperation(track);
}
void patchCommand17(Track * track, int16_t param)
{
	#if AUDIO_USES_LPF
		uint16_t filterValue = param;
		audioMixerData.filterValue = filterValue;
	#else
		(void) param;
	#endif
}
const patchCommand patchCommands[]  =
{
	patchCommand00, patchCommand01, patchCommand02, patchCommand03, 
	patchCommand04, patchCommand05, patchCommand06, patchCommand07, 
	patchCommand08, patchCommand09, patchCommand10, patchCommand11,
	patchCommand12, patchCommand13, patchCommand14, patchCommand15,
	patchCommand16, patchCommand17	
};
void setNoteVol(uint8_t channel, uint8_t vol)
{
	tracks[channel].noteVol = vol;
}
void initMusicPlayer(const patch_t *patchPointersParam)
{

	patchPointers = patchPointersParam;

	masterVolume = DEFAULT_MASTER_VOL;

	playSong = 0;

	//initialize default channels patches
	for (unsigned char t=0; t < NUMBER_OF_AUDIO_CHANNELS; t++ )
	{
		tracks[t].channel = t;
		tracks[t].flags = TRACK_FLAGS_ALLOCATED;	//allocated=true,priority=0
		tracks[t].noteVol = 0;
		tracks[t].trackVol = DEFAULT_TRACK_VOL;
		tracks[t].patchNo = DEFAULT_PATCH;
		setMixerWave(t, 0);
		tracks[t].tremoloRate = 24; //~6hz
		tracks[t].slideSpeed = 0x10;
	}
}
#if MUSIC_ENGINE == MIDI

void startSong(const uint8_t *song)
{
	for(unsigned char t=0; t < NUMBER_OF_AUDIO_CHANNELS; t++)
	{
		tracks[t].flags &= (~TRACK_FLAGS_PRIORITY); // priority=0;
		tracks[t].expressionVol = DEFAULT_EXPRESSION_VOL;
	}
	songPos = (const int8_t*) song + 1; //skip first delta-time
	songStart = (const int8_t*)song + 1;//skip first delta-time
	loopStart = (const int8_t*) song + 1;
	nextDeltaTime = 0;
	currDeltaTime = 0;
	songSpeed = 0;

	lastStatus = 0;
	playSong = 1;
}

#elif MUSIC_ENGINE == STREAM

void startSong(const uint8_t *song)
{
	for(unsigned char t=0; t < NUMBER_OF_AUDIO_CHANNELS; t++)	
	{
		tracks[t].flags &= (~TRACK_FLAGS_PRIORITY);
		tracks[t].expressionVol = DEFAULT_EXPRESSION_VOL;
	}
	songPos		= song;
	//songStart	= song;
	loopStart	= song;
	nextDeltaTime	= 0;
	songSpeed	= 0;

	lastStatus	= 0;
	playSong	= true;
}

#elif MUSIC_ENGINE==MOD

void startSong (const uint8_t *song, u16 startPos, uint8_t loop)
{
	for(unsigned char t=0; t < NUMBER_OF_AUDIO_CHANNELS; t++)
	{
		tracks[t].flags &= (~TRACK_FLAGS_PRIORITY); // priority=0;
	}

	uint8_t headerSize;
	uint8_t patternsCount;
	uint8_t restartPosition;

	//MOD setup
	headerSize= *song;
	modChannels = *(song+1);
	patternsCount = *(song+2);
	step = *(song+3);
	songSpeed = *(song+4);
	songLength = *(song+5);
	restartPosition = *(song+6);

	songPos = song + headerSize; //poinst to orders
	songStart = song + headerSize; //poinst to orders
	if (loop)
	{
		loopStart = song + headerSize + (restartPosition * modChannels);
	}
	else
	{
		loopStart = NULL;
	}
	patternOffsets = song+headerSize + (songLength *modChannels);
	pattern s= song + headerSize + (songLength * modChannels) + (patternsCount * 2);

	songPos += (startPos * modChannels);

	currentTick = 0;
	currentStep = 0;

	lastStatus = 0;
	playSong = 1;

}


#endif




void stopSong()
{

	for(uint8_t i = 0; i < NUMBER_OF_AUDIO_CHANNELS; i++)
	{
		tracks[i].envelopeStep=-6;
	}
	playSong = 0;
}
void resumeSong()
{
	playSong = 1;
}
void setSongSpeed(uint8_t speed)
{
	songSpeed = speed;
}

uint8_t getSongSpeed()
{
	return songSpeed;
}
void soundEngine(void)
{
	uint8_t c1, c2;
	uint32_t uVol, tVol;
	
	uint8_t tmp, trackVol;
	int16_t  vol;
	uint8_t channel;
	Track* track;


	//process patches envelopes & pitch slides
	for(unsigned char trackNo = 0; trackNo < NUMBER_OF_AUDIO_CHANNELS; trackNo++)
	{
		track = &tracks[trackNo];
		//update envelope
		if (track->envelopeStep != 0)
		{
			vol = track->envelopeVol+track->envelopeStep;		
			if (vol<0)
			{
				vol=0;			
			}
			else if (vol>0xff)
			{
				vol=0xff;						
			}
			track->envelopeVol=vol;
		}

		//if volumes reaches zero and no more patch command, explicitly end playing on track
		//if(vol==0 && track->patchCommandStreamPos==NULL) track->flags&=~(TRACK_FLAGS_PLAYING);

		if (track->flags & TRACK_FLAGS_SLIDING)
		{

			audioMixerData.channels[trackNo].increment += track->slideStep;
			uint16_t tStep = stepTable[track->slideNote];
			if ((track->slideStep > 0 && audioMixerData.channels[trackNo].increment >= tStep) || (track->slideStep < 0 && audioMixerData.channels[trackNo].increment <= tStep))
			{					
				audioMixerData.channels[trackNo].increment = tStep;					
				track->flags &= ~(TRACK_FLAGS_SLIDING);	
			}
			setCorrectSingleShotOperation(track);
		}
	}



	//Process song MIDI notes
	if (playSong)
	{
		#if MUSIC_ENGINE == MIDI		
			//process all simultaneous events

			while (currDeltaTime == nextDeltaTime)
			{
				c1 = *songPos++;
			
				if (c1 == 0xff)
				{
					//META data type event
					c1= *songPos++;
					
					if (c1 == 0x2f)
					{ //end of song
						playSong = 0;
						break;	
					}
					else if (c1 == 0x6)
					{ //marker
						c1 = *songPos++; //read len
						c2 = *songPos++; //read data
						if (c2 == 'S')
						{ //loop start
							loopStart=songPos;
						}
						else if (c2 == 'E')
						{//loop end
							songPos = loopStart;
						}
					}
				}
				else
				{
					if (c1 & 0x80) 
						lastStatus = c1;					
					channel = lastStatus & 0x03;		// uchipVGA has only 4 channels
					//get next data byte		
					if (c1 & 0x80) 
						c1 = *songPos++; 
					switch (lastStatus & 0xf0 )
					{
						//note-on
						case 0x90:
							//c1 = note						
							c2 = (*songPos++) << 1; //get volume
							//if (tracks[channel].flags | TRACK_FLAGS_ALLOCATED) <--- this is the old code. It's a bug, & should be used, as this clause is always true
							if (isTrackAllocated(tracks[channel]))
							{ //allocated==true
								triggerNote(channel, tracks[channel].patchNo, c1, c2);
							}
							break;
						//controllers
						case 0xb0:
							///c1 = controller #
							c2 = *songPos++; //get controller value
						
							if (c1 == CONTROLER_VOL)
							{
								tracks[channel].trackVol= c2 << 1;
							}
							else if(c1 == CONTROLER_EXPRESSION)
							{
								tracks[channel].expressionVol = c2 << 1;
							}
							else if (c1 == CONTROLER_TREMOLO)
							{
								tracks[channel].tremoloLevel= c2 << 1;
							}
							else if (c1 == CONTROLER_TREMOLO_RATE)
							{
								tracks[channel].tremoloRate= c2 << 1;
							}
							break;

						//program change
						case 0xc0:
							// c1 = patch #		
							if (c1 >= getNumberOfPatches())
								c1 = DEFAULT_PATCH;
							tracks[channel].patchNo = c1;
						
							break;

					}//end switch(c1&0xf0)


				}//end if(c1==0xff)

				//read next delta time
				nextDeltaTime = readVarLen(&songPos);			
				currDeltaTime = 0;		
				#if SONG_SPEED == 1
					if (songSpeed != 0)
					{
						uint32_t l  = (uint32_t) (nextDeltaTime << 8);

						if (songSpeed < 0)
						{//slower
							(uint32_t)(l += (uint32_t)(-songSpeed * (nextDeltaTime << 1)));
							(uint32_t)(l >>= 8);
						}
						else//faster
							(uint32_t)(l /= (uint32_t)((1 << 8) + (songSpeed << 1)));
						nextDeltaTime = l;
					}
				#endif

			}//end while

			currDeltaTime++;
		#elif MUSIC_ENGINE == STREAM		
			//process all simultaneous events
			//everything about this format is designed to minimize the size of the most common events
			if (nextDeltaTime)//eat last frames delay
				nextDeltaTime--;

			while (!nextDeltaTime)
			{
				c1 = songBufRead();

				if (c1 == 0xFF)
				{//future expansion
					playSong = false;//we do not yet know the number of argument bytes for these, so can't continue the stream
					break;
				}
				else if ((c1 & 0b11100000) < 0b10100000) 
				{//0bCCCXXXXX, if (CCC>>5) is 0-4, then it is a Note On
					//Note On: 0bCCCVVVVV, VNNNNNNN = CCC = channel, VVVVV V = volume, NNNNNNN = note
					channel = (c1>>5);//get channel
					c1 = (c1 & 0b00011111);//get volume 5 LSBits
					c2 = songBufRead();//get packed note and MSBit of volume
					c1 |= (c2 & 0b10000000)>>2;//add 1 MSBit to previous LSBits for 6 bits total volume
					c1 <<= 2;//convert our 6 bits to: 7 bit converted to 8 bit volume of original format
					c2 &= 0b01111111;//mask out the MSbit of volume, leaving just the note
					//c2 = note, c1 = volume
//	The following line original is buggy, it's always positive and it does not actually check for allocation status
//					if (tracks[channel].flags | TRACK_FLAGS_ALLOCATED)//allocated==true
					if (isTrackAllocated(tracks[channel]))
						triggerNote(channel, tracks[channel].patchNo, c2, c1);
				
				}
				else
				{ //"channel" is not actually the channel, but an indicator of the command
					c2 = (c1 &	0b11100000);//determine the actual command type by the "channel" signal

					if (c2 == 0b10100000)
					{ //"channel" == 5<<5 indicates Program Change
						channel = (c1 & 0b00000111);//extract actual channel(not the 5 used for signal)
						c2 = songBufRead();//get patch
						tracks[channel].patchNo = c2;
					
					}
					else if (c2 == 0b11000000)
					{ //"channel" == 6<<5 indicates Marker						
						c2 = (c1 & 0b00000011);
						
						if (c2 == 0b00000000)
						{ //Loop End(0b11000000)
							songPos = loopStart;
						}
						else if (c2 == 0b00000001){//Loop Start(0b11000001)
							loopStart = songPos;
						}
						else if (c2 == 0b00000010){//Song End(0b11000010)
							playSong = false;
							break;
						}
						else
						{ //Tick End(0b110XXX11)
							nextDeltaTime = ((c1 & 0b00011100) >> 2) + 1;//possibly short version 1-7 frames, 1 byte
							if (nextDeltaTime == (0b00000111) + 1) //longer version, 2 bytes
								nextDeltaTime = songBufRead();
						}
					}
					else
					{ //c2 = 0b11100000 "channel" = 7<<5 indicates Controller Event
						channel = (c1 & 0b00000111); //get the actual channel
						c2 = (c1 & 0b00011000); //mask the controller type
						c1 = songBufRead(); //get controller value
	
						if (c2 == 0b00000000)//Channel Volume
							tracks[channel].trackVol = c1 << 1;
						else if (c2 == 0b00001000)//Expression
							tracks[channel].expressionVol = c1 << 1;
						else if(c2 == 0b00010000)//Tremolo Volume
							tracks[channel].tremoloLevel = c1 << 1;
						else//c2 = 0b00011000//Tremolo Rate
							tracks[channel].tremoloRate = c1 << 1;
					}
				}
				#if SONG_SPEED == 1
					if (!nextDeltaTime)
						continue;

					if (songSpeed != 0)
					{
						uint32_t l  = (uint32_t)(nextDeltaTime << 8);

						if (songSpeed < 0)
						{//slower
							(uint32_t)(l += (uint32_t) (-songSpeed * (nextDeltaTime << 1)));
							(uint32_t)(l >>= 8);
						}
						else//faster
							(uint32_t)(l /= (uint32_t)((1 << 8) + (songSpeed << 1)));

						nextDeltaTime = l;
					}
				#endif

			}//end while
		
		#elif MUSIC_ENGINE == MOD //MOD
			

			uint8_t patternNo, data, note, data2, flags;
			uint16_t tmp1;

			if (currentTick == 0)
			{
				//process next MOD row
				for (uint8_t trackNo = 0; trackNo < modChannels; trackNo++)
				{
					track = &tracks[trackNo];
					const char* patPos;
					
					if (currentStep == 0)
					{
						//get pattern order
						patternNo = *songPos+trackNo;
						//get pattern address
						tmp1 = patternOffsets[patternNo]);
						patPos = patterns + tmp1;
					}
					else
					{
						patPos = track->patternPos;
					}
					data = *patPos++;
					note = data&0x7f;
					if (note!=0) 
						track->note=note;

					/*Pack format:
					 *
					 * byte1: [msb,6-0]  msb->inst,vol or fx follows, 6-0: note. 0=no note
					 * byte2: [7-5,4-0] 7-5=001 -> 4-0=instrument
					 *                     =010 -> 4-0=volume
					 *                     =011 -> 4-0=instrument, next byte is volume
					 *                     =100 -> 4-0=FX type, next byte is fx param
					 *                     =101 -> 4-0=instr, next 2 bytes fx type & fx param
					 *                     =110 -> 4-0=vol,  next 2 bytes fx type & fx param
					 *                     =111 -> 4-0=instr, next 3 bytes vol, fx type & fx param
					 * Notes:
					 *       volumes are stored as 0x00-0x1f (0-31)
					 */
					if ((data & 0x80) != 0)
					{
						data2 = *patPos++;
						flags = data2 >> 5;
						data2 &= 0x1f;
						track->noteVol = 255; //default vol
						switch (flags)
						{
							case 0x01:
								//instrument byte
								track->patchNo = data2;
								break;
							case 0x02:
								track->noteVol = (data2 << 3);
								break;
							case 0x03:
								track->patchNo = data2;
								track->noteVol = (*patPos++) << 3;
								break;
							case 0x04:
								patPos += 2; //TODO: skip 2 effects bytes
								break;
							case 0x05:
								track->patchNo = data2;
								patPos += 2; //TODO: skip 2 effects bytes
								break;
							case 0x06:
								track->noteVol = (data2 << 3);
								patPos += 2; //TODO: skip 2 effects bytes
								break;
							case 0x07:
								track->patchNo = data2;
								track->noteVol = (*patPos++) << 3;
								patPos+=2; //TODO: skip 2 effects bytes
								break;
						
						}
					}
					if (note != 0)
					{
						// this line is buggy, and has been replaced (& should have been used instead)
						//if (track->flags | TRACK_FLAGS_ALLOCATED)			// 
						if (isTrackAllocated(track))
						{ //allocated==true
							if (trackNo==3)
							{
								triggerNote(trackNo, 0, track->patchNo, track->noteVol); //on noise channel, note actually selects the patch
							}
							else
							{
								triggerNote(trackNo, track->patchNo, note, track->noteVol);
							}
						}
						
					}

					track->patternPos = patPos;										
				}
			}
			currentTick++;
			if (currentTick > songSpeed) 
			{
				currentTick = 0;
				currentStep++;
				if (currentStep == step)
				{
					currentStep=0;
					songPos+=modChannels;

					//handle loop
					if(songPos >= songStart + (songLength *modChannels))
					{
						if(loopStart != NULL)
						{
							songPos=loopStart;
						}
						else
						{
							stopSong();
						}
					}

				}
			}

			
	
		#endif

	}//end if(playSong)

	
	// here we perform all the stuff required to play music and/or sound effects
	for(unsigned char trackNo = 0; trackNo < NUMBER_OF_AUDIO_CHANNELS; trackNo++)
	{
		track= &tracks[trackNo];

		//process patch command stream
		if ((track->flags & TRACK_FLAGS_PLAYING) && (track->patchCommandStreamPos != NULL) && ( (track->flags & TRACK_FLAGS_HOLD_ENV) == 0))
		{
			//process all simultaneous events
			while(track->patchCurrDeltaTime == track->patchNextDeltaTime)
			{
				c1 = *track->patchCommandStreamPos++;
				if(c1 == PATCH_END)
				{
					//end of stream!
					track->flags &= (~TRACK_FLAGS_PRIORITY);// priority=0;
					track->patchCommandStreamPos=NULL;
					break;
				}
#if AUDIO_USES_LPF
				else if (c1 == PC_SET_FILTER_VALUE)
				{
					// this command has 2 parameters!
					uint8_t cl = (uint8_t) *track->patchCommandStreamPos++;
					uint8_t ch = (uint8_t) *track->patchCommandStreamPos++;
					int16_t clh = (int16_t) ((ch << 8) | cl);
					//invoke patch command function
					patchCommands[c1](track, clh);
				}
#endif
				else
				{
					c2 = *track->patchCommandStreamPos++;
					//invoke patch command function
					patchCommands[c1](track, c2);
				}
				//read next delta time
				track->patchNextDeltaTime= *track->patchCommandStreamPos++;
				track->patchCurrDeltaTime = 0;
			}
			track->patchCurrDeltaTime++;
		}
		if (track->flags & TRACK_FLAGS_PLAYING)
		{
			if(track->patchPlayingTime < 0xffff)		// modified to uint16
			{
				track->patchPlayingTime++;
			}
			// BEGIN: 2020/12/07 Bug fix: triggerFx on samples without actual associated patch (i.e. NULL pointer) would leave the channel as reserved to FX (priority flag set)
			// to avoid this, and to avoid to have to create a patch for each sample as a workaround, we can check if the playing time exceeded the sample playing time by
			// at least one frame
			if (track->patchCommandStreamPos == NULL && (track->flags & TRACK_FLAGS_PRIORITY))	//this issue is present only if we do not provide a patch
			{ 
				uint8_t patch = track->fxPatchNo;
				uint32_t loopStart = patchPointers[patch].loopStart;
				uint32_t loopEnd = patchPointers[patch].loopEnd;
				if ((loopStart + 1) >= loopEnd) // is this a non-looping sample? Then check since when it's playing.
				{
					// how long would the fx play for? (loopEnd << 8) / Increment / HORIZONTAL_FREQ  (in seconds).
					// In frames: nFrames = VERTICAL_FREQ * (loopEnd << 8) / Increment /  HORIZONTAL_FREQ. 
					// note that VERTICAL_FREQ / HORIZONTAL_FREQ = 1/VERTICAL LINES, i.e. 525. 
					// nFrames = (loopEnd << 8) / (Increment * 525). If nFrames + 1 > track->patchPlayingTime, then we can assume the fx has ended.
					// instead of a division, we multiply both members.				
					if ((loopEnd << 8) <=  (track->patchPlayingTime - 1) * 525 * audioMixerData.channels[trackNo].increment)
					{ // note, left term might overflow for large increments or playing times. Though, it is very unlikely it will overflow Before we reach loopEnd << 8.
				      // in fact, due to flash size, loopend at most would be something like 220k o even less! (just for one sample!)
						track->flags &= ~(TRACK_FLAGS_PLAYING | TRACK_FLAGS_PRIORITY); // time expired: turn off.			
					}
				}				
			}
			// END 2020/12/07
			//compute final frame volume
			if( track->flags & TRACK_FLAGS_PRIORITY)
			{
				//if an FX, use full track volume.
				trackVol = 0xff;
			}
			else
			{
				//if regular note, apply MIDI track volume
				trackVol= track->trackVol;
			}
			if (track->noteVol != 0 && track->envelopeVol != 0 && trackVol != 0 && masterVolume != 0)
			{
				uVol= (track->noteVol * trackVol) + 0x100;
				uVol >>= 8;
				
				uVol = ( uVol * track->envelopeVol) + 0x100;
				uVol >>= 8;
				
				#if MUSIC_ENGINE == MIDI
					uVol= (uVol * track->expressionVol) + 0x100;
					uVol >>= 8;
				#endif
				
				uVol = (uVol * masterVolume) + 0x100;
				uVol >>= 8;

				if (track->tremoloLevel > 0)
				{
					#if (INCLUDE_DEFAULT_WAVES != 0)
						tmp =  sineWave[track->tremoloPos];
					#else
						tmp = 0;
					#endif
					tmp -= 128; //convert to unsigned

					tVol = ( ((uint32_t) track->tremoloLevel) * tmp ) + 0x100;
					tVol >>= 8;
					
					uVol=	(uVol* (0xff - tVol)) + 0x100;
					uVol >>= 8;
				}

				
			}
			else
			{
				uVol = 0;
			}
			track->tremoloPos += track->tremoloRate;
		}
		else
		{
			uVol = 0;
		}
		
		audioMixerData.channels[trackNo].volume = (uVol & 0xff);
	}
}

#if MUSIC_ENGINE == MIDI

uint16_t readVarLen(const int8_t **songPos)
{
    unsigned int value;
    unsigned char c;

    if ( (value = *(*songPos)++) & 0x80 )
    {
       value &= 0x7F;
       do
       {
         value = (value << 7) + ((c = *(*songPos)++) & 0x7F);
       } while (c & 0x80);
    }
    return value;
}

#elif MUSIC_ENGINE == STREAM

uint8_t songBufRead()
{//this name is a bit dubious for the flash only(no buffer) version, but it keep the code easier to read
		return *songPos++;
}
#endif




void triggerCommon(Track* track, uint8_t patch, uint8_t volume, int16_t note)  // note < 0 => we already set the speed, do not change
{
		
	uint8_t isFx = (track->flags & TRACK_FLAGS_PRIORITY);

	track->envelopeStep = 0; 
	track->envelopeVol = 0xff; 
	track->noteVol = volume;
	track->patchPlayingTime = 0;
	track->flags &= (~(TRACK_FLAGS_HOLD_ENV | TRACK_FLAGS_SLIDING));
	track->tremoloLevel = 0;
	track->tremoloPos = 0;
	track->note = note;
	track->loopCount = 0;

#if MUSIC_ENGINE == MIDI || MUSIC_ENGINE == STREAM
	track->expressionVol = DEFAULT_EXPRESSION_VOL;
#endif

	// Here all channels are PCM type.
	//PCM channel					
	audioMixerData.channels[track->channel].index = 0;
	if (patchPointers[patch].soundWaveNumber == -1)  // this is for compatibility with UZEBOX (by changing NULL with -1. Leaving null gives the same effect).  
	{
		setMixerWave(track->channel,0);//default wave
	}
	else
	{
		//audioMixerData.channels[track->channel].offset = (uint32_t) patchPointers[patch].pcmData;
		audioMixerData.channels[track->channel].offset = (uint32_t) soundWaves[patchPointers[patch].soundWaveNumber].wData;
		audioMixerData.channels[track->channel].length = (patchPointers[patch].loopEnd << 8);
		audioMixerData.channels[track->channel].delta = (patchPointers[patch].loopEnd - patchPointers[patch].loopStart) << 8;
	}
	if (isFx)
	{
		track->fxPatchNo = patch;
	}
	else
	{
		track->patchNo = patch;
	}
	if (note >= 0)
		setMixerNote(track->channel, note); // this must be called AFTER "if (isFx)", because the patch must be correctly set
	const char *pos = (const char*)(patchPointers[patch].cmdStream);
	if (pos == NULL) 
	{
		track->patchCommandStreamPos = NULL;
	}
	else
	{
		track->patchCurrDeltaTime = 0;
		track->patchNextDeltaTime = *pos++;
		track->patchCommandStreamPos = (uint8_t*) pos;
	}

}


/* Trigger a sound effect.
 * Method allocates the channel based on priority.
 * Retrig flags: if this fx if already playing on a track, reuse same track.
 * return the channel number used. Useful to call later stopLoopingFx.
 */
uint8_t triggerFx (int16_t patch,unsigned char volume, uint8_t flags, uint32_t detuning)
{
	if (patch < 0)
		return 0;	// channel 0 will never be allocated, so it is safe to return this.
	unsigned char channel;
	uint8_t retrig = flags & FX_FLAGS_RETRIG;
	if (!(flags & FX_FLAGS_SPECIFY_CHANNEL))
	{
		//find the channel to play the fx
		//try to steal voice 3, then 2, then 1
		//never steal voice 0, reserve it for lead melodies
		if( (tracks[3].flags & TRACK_FLAGS_PRIORITY) == 0 || (tracks[3].fxPatchNo == patch && (tracks[3].flags & TRACK_FLAGS_PRIORITY) != 0 && retrig != 0))
		{
			// fx already playing
			channel = 3;
		}
		else if( (tracks[1].flags & TRACK_FLAGS_PRIORITY) == 0 || (tracks[1].fxPatchNo == patch && (tracks[1].flags & TRACK_FLAGS_PRIORITY) != 0 && retrig != 0))
		{ //fx already playing
			channel = 1;
		}
		else if ( (tracks[2].flags & TRACK_FLAGS_PRIORITY ) == 0 || (tracks[2].fxPatchNo == patch && (tracks[2].flags & TRACK_FLAGS_PRIORITY) != 0 && retrig != 0))
		{ //fx already playing
			channel = 2;
		}
		else
		{
			// all channels have fx playing, use the oldest one
			channel = 1;
			uint16_t oldestTime = tracks[1].patchPlayingTime ;
			for (int i = 2; i < 4; i++)
			{
				if (tracks[i].patchPlayingTime > oldestTime)
				{
					channel = i;
					oldestTime = tracks[i].patchPlayingTime;
				}
			}
		}	
	}
	else
	{
		channel = ((flags & FX_FLAGS_SPECIFY_CHANNEL_MASK) >> FX_FLAGS_SPECIFY_CHANNEL_POS);
	}
	Track* track= &tracks[channel];

	track->flags = TRACK_FLAGS_PRIORITY; //priority=1;
	track->patchCommandStreamPos = NULL;	
	if (flags & FX_FLAGS_SPECIFY_SAMPLE_FREQUENCY)
	{
		audioMixerData.channels[channel].increment = (((soundWaves[patchPointers[patch].soundWaveNumber].sps) * (detuning >> 4) ) / SAMPLE_RATE) >> 4;
		uint32_t loopStart = patchPointers[patch].loopStart;
		uint32_t loopEnd = patchPointers[patch].loopEnd;
		if ((loopStart + 1) >= loopEnd)
			audioMixerData.channels[channel].delta = audioMixerData.channels[channel].increment;
		triggerCommon(track, patch, volume , -1);	
	}
	else
	{
		triggerCommon(track, patch, volume , 80);	// TODO: check if this should be 80! Edit: at least this is good as a deault value. If one wants a 1:1 it must use a patch with a PC_PITCH 0x80	
	}	
	track->flags |= TRACK_FLAGS_PLAYING;
	return channel;
}
void stopLoopingFx(uint8_t channel, uint8_t abruptStop)
{
	if (abruptStop)
	{
		audioMixerData.channels[channel].increment = 0;	
		tracks[channel].flags &= ~( TRACK_FLAGS_PRIORITY | TRACK_FLAGS_PLAYING); // stop immediately.
	}
	else
	{  // stop at the end of the loop, however release the priority, so that the channel can be taken by a note (fixme)
		audioMixerData.channels[channel].delta = audioMixerData.channels[channel].increment;	
		tracks[channel].flags &= ~( TRACK_FLAGS_PRIORITY); // stop immediately.
	}
}

void triggerNote (unsigned char channel, int16_t patch, unsigned char note, unsigned char volume)
{
	if (patch < 0)
		return;
	Track* track = &tracks[channel];

	//allow only other music notes 
	if ((track->flags & TRACK_FLAGS_PLAYING) == 0 || (track->flags & TRACK_FLAGS_PRIORITY) == 0)
	{
			
		if (volume == 0)
		{ //note-off received
		
			//cut note if there's no envelope & no note hold
			if(track->envelopeStep == 0 && !(track->flags & TRACK_FLAGS_HOLD_ENV))
			{
				track->noteVol=0;
			}

			track->flags &= (~TRACK_FLAGS_HOLD_ENV);//patchEnvelopeHold=false;
		}
		else
		{
			// the following line works with the original code, because actually ALLOCATED is not checked, as | is used instead of &.
			// However, by fixing the code, this line then breaks everything.  The original, uncommented value should be used instead
			// track->flags = 0;//&=(~TRACK_FLAGS_PRIORITY);// priority=0;
			// Still, actually TRACK_ALLOCATED is never changed throughout this source. Therefore, to save some minor operations, we have introduced
			// the switch.
			#if DO_NOT_USE_TRACK_ALLOCATION
				track->flags = 0; // (~TRACK_FLAGS_PRIORITY);// priority=0;
			#else
				track->flags &= (~TRACK_FLAGS_PRIORITY);// priority=0;
			#endif
			track->patchCommandStreamPos = NULL;
			triggerCommon(track, patch, volume, note);
			track->flags |= TRACK_FLAGS_PLAYING;
		}
	}
}



void setMasterVolume(unsigned char vol)
{
	masterVolume=vol;
}

uint8_t getMasterVolume()
{
	return masterVolume;
}

uint8_t isSongPlaying()
{
	return playSong;
}
#endif