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
*  audioMixer.h Here the audio samples are calculated for each channel, and the summed, during the screen blank.
*/
#ifndef AUDIOMIXER_H_
#define AUDIOMIXER_H_
#include "system.h"
typedef struct  __attribute__((packed, aligned(4)))
{
	uint32_t offset;			// origin of the sample
	uint32_t length;			// PCM wave length
	uint32_t delta;				// once we reached the "sample lengh" index, how many bytes we must go back (useful for loops, partial loops and silence).
	uint32_t increment;			// where will be the next sample to be played (relative to the current one)
	uint32_t volume;
	uint32_t index;				// current sample position to be played.
} audioChannel_t;
typedef struct __attribute__((packed, aligned(4)))
{
	uint32_t minus4;
	audioChannel_t channels[4];
	uint32_t dacrAddress;
	#if AUDIO_USES_LPF
	int32_t filterValue;			// this is a .16 value. Please leave all the upper 16 bit at 0!
	int32_t oldSampleValue;
	#endif
//	uint32_t masterVolume;		// for a quicker access we use a 32 bit!
} audioMixer_t;
extern audioMixer_t audioMixerData;

RAMFUNC inline static void audioMixer()
{
	#if IS_BOOTLOADER
		// single channel audio.
		asm volatile
		(
			// Prologue: we need a -4
			// channel 0
			"LDR r0,audioDataAddress\n\t"
			//	"ADD r0,%[audioDataOffset]\n\t"
			"LDMIA r0!,{r1,r2,r3,r4,r5,r6, r7}\n\t"		// (8 cycles). R1 = -4; R2 = offset. R3 = length; R4 = delta;  R5 = increment ; R6= volume; R7 =  index
			"ADD r5,r7\n\t"			// (1 cycle) add increment to index and place the result to the no more used increment
			"CMP r5,r3\n\t"			// (1 cycle) r5 - r3 (inew ndex - length)
			"BLT saveindex0\n\t"			// (2 cycles if taken, 1 otherwise) if r5 < r3, then just save the index.
			"SUB r5,r4\n\t"			// (1 cycle) remove the delta
			"saveindex0:\n\t"
			"STR r5,[r0, r1]\n\t"			// (2 cycle) save it
			// now we have these registers free: r4, r3, r2
			"LSR r7,r7,#8\n\t"			// (1 cycle) r7 = index >> 8
			"LDRSB r7,[r2, r7]\n\t"			// (2 cycles) r7 = channel_sample
			"MUL r7,r6\n\t"			// (1 cycle) channel_sample = channel_sample * volume
//			"MOV r8,r7\n\t"			// (1 cycle) sample is r8; The channel 0 needs to initialize r8 to r6! Next channels instead adds it.
			//
			"ADD r0,#(18*4)\n\t" 
// 			// channel 1
// 			"LDMIA r0!,{r2,r3,r4,r5,r6, r7}\n\t"		// (8 cycles). R1 = -4; R2 = offset. R3 = length; R4 = delta;  R5 = increment ; R6= volume; R7 =  index
// 			"ADD r5,r7\n\t"			// (1 cycle) add increment to index and place the result to the no more used increment
// 			"CMP r5,r3\n\t"			// (1 cycle) r5 - r3 (inew ndex - length)
// 			"BLT saveindex1\n\t"			// (2 cycles if taken, 1 otherwise) if r5 < r3, then just save the index.
// 			"SUB r5,r4\n\t"			// (1 cycle) remove the delta
// 			"saveindex1:\n\t"
// 			"STR r5,[r0, r1]\n\t"			// (2 cycle) save it
// 			// now we have these registers free: r4, r3, r2
// 			"LSR r7,r7,#8\n\t"			// (1 cycle) r7 = index >> 8
// 			"LDRSB r7,[r2, r7]\n\t"			// (2 cycles) r7 = channel_sample
// 			"MUL r7,r6\n\t"			// (1 cycle) channel_sample = channel_sample * volume
// 			"ADD r8,r7\n\t"			// (1 cycle) sample is r8; The channel 0 needs to initialize r8 to r6! Next channels instead adds it.
// 			// channel 2
// 			"LDMIA r0!,{r2,r3,r4,r5,r6, r7}\n\t"		// (8 cycles). R1 = -4; R2 = offset. R3 = length; R4 = delta;  R5 = increment ; R6= volume; R7 =  index
// 			"ADD r5,r7\n\t"			// (1 cycle) add increment to index and place the result to the no more used increment
// 			"CMP r5,r3\n\t"			// (1 cycle) r5 - r3 (inew ndex - length)
// 			"BLT saveindex2\n\t"			// (2 cycles if taken, 1 otherwise) if r5 < r3, then just save the index.
// 			"SUB r5,r4\n\t"			// (1 cycle) remove the delta
// 			"saveindex2:\n\t"
// 			"STR r5,[r0, r1]\n\t"			// (2 cycle) save it
// 			// now we have these registers free: r4, r3, r2
// 			"LSR r7,r7,#8\n\t"			// (1 cycle) r7 = index >> 8
// 			"LDRSB r7,[r2, r7]\n\t"			// (2 cycles) r7 = channel_sample
// 			"MUL r7,r6\n\t"			// (1 cycle) channel_sample = channel_sample * volume
// 			"ADD r8,r7\n\t"			// (1 cycle) sample is r8; The channel 0 needs to initialize r8 to r6! Next channels instead adds it.
// 			// channel 3
// 			// this is a bit modified to optimize!
// 			"LDMIA r0!,{r2,r3,r4,r5,r6,r7}\n\t"		// (8 cycles). R1 = -4; R2 = offset. R3 = length; R4 = delta;  R5 = increment ; R6= volume; R7 =  index
// 			"ADD r5,r7\n\t"			// (1 cycle) add increment to index and put the result to incremnent, which is no more used!
// 			"CMP r5,r3\n\t"			// (1 cycle) r4 - r2 (newIndex - length)
// 			"BLT saveindex3\n\t"			// (2 cycles if taken, 1 otherwise) if r4 < r2, then just save the index.
// 			"SUB r5,r4\n\t"			// (1 cycle) remove the delta
// 			"saveindex3:\n\t"
// 			"STR r5,[r0, r1]\n\t"			// (2 cycle) save it
// 			"LSR r7,r7,#8\n\t"			// (1 cycle) r0 = index >> 8
// 			"LDRSB r7,[r2, r7]\n\t"			// (2 cycles) r0 = channel_sample
// 			"MUL r7,r6\n\t"			// (1 cycle) channel_sample = channel_sample * volume
// 			"ADD r7,r8\n\t"			// (1 cycle) sample is r8; The channel 0 needs to initialize r8 to r6! Next channels instead adds it.
			#if AUDIO_USES_LPF
			// FILTER: it performs sample(t) = sample(t-1) * filterCoeff + newValue(t) * ( 1 - filterCoeff)
			// this is also equal to:  (sample(t-1) - newValue(t)) * filterCoeff + newValue(t)
			"LDMIA r0!,{r2,r3, r4}\n\t" // r1 = -4; r2 = DAC register, r3 = filter coefficient, r4 = oldvalue (already shifted and multiplied)
			"SUB r5, r4, r7\n\t"		// r5 = oldValue - newValue
			"MUL r5, r3\n\t"			// r5 = r5 * filterCoeff
			"LSL r7, #14\n\t"			// newValue = newValue *4096
			"ADD r7, r7, r5\n\t"		// newValue = newValue + r5
			"ASR r7, #14\n\t"			// newValue / 4096
			"STR r7, [r0, r1]\n\t"		// store the premultipied value of the sample for the next cycle
			// END FILTER
			#else
			"LDR r2,[r0]\n\t" // r1 = -4; r2 = DAC register, r3 = filter coefficient, r4 = oldvalue (already shifted and multiplied)
			#endif
			"ASR r7,#9\n\t"			// (1 cycle)
			"ADD r7,#128\n\t"
			"ADD r7,#128\n\t"
			"STR r7, [r2]\n\t"			// (2 cycles) write to dac
			"B endAudio\n\t"
			".align(4)\n\t"
			"audioDataAddress:\n\t"
			".word audioMixerData\n\t"
			"endAudio:\n\t"
			:
			:
			:  "r0", "r1", "r2","r3","r4","r5","r6","r7","r8"
		);	
	#else
	asm volatile
	(
	// Prologue: we need a -4
	// channel 0
	"LDR r0,audioDataAddress\n\t"
	//	"ADD r0,%[audioDataOffset]\n\t"
	"LDMIA r0!,{r1,r2,r3,r4,r5,r6, r7}\n\t"		// (8 cycles). R1 = -4; R2 = offset. R3 = length; R4 = delta;  R5 = increment ; R6= volume; R7 =  index
	"ADD r5,r7\n\t"			// (1 cycle) add increment to index and place the result to the no more used increment
	"CMP r5,r3\n\t"			// (1 cycle) r5 - r3 (inew ndex - length)
	"BLT saveindex0\n\t"			// (2 cycles if taken, 1 otherwise) if r5 < r3, then just save the index.
	"SUB r5,r4\n\t"			// (1 cycle) remove the delta
	"saveindex0:\n\t"
	"STR r5,[r0, r1]\n\t"			// (2 cycle) save it
	// now we have these registers free: r4, r3, r2
	"LSR r7,r7,#8\n\t"			// (1 cycle) r7 = index >> 8
	"LDRSB r7,[r2, r7]\n\t"			// (2 cycles) r7 = channel_sample
	"MUL r7,r6\n\t"			// (1 cycle) channel_sample = channel_sample * volume
	"MOV r8,r7\n\t"			// (1 cycle) sample is r8; The channel 0 needs to initialize r8 to r6! Next channels instead adds it.
	// channel 1
	"LDMIA r0!,{r2,r3,r4,r5,r6, r7}\n\t"		// (8 cycles). R1 = -4; R2 = offset. R3 = length; R4 = delta;  R5 = increment ; R6= volume; R7 =  index
	"ADD r5,r7\n\t"			// (1 cycle) add increment to index and place the result to the no more used increment
	"CMP r5,r3\n\t"			// (1 cycle) r5 - r3 (inew ndex - length)
	"BLT saveindex1\n\t"			// (2 cycles if taken, 1 otherwise) if r5 < r3, then just save the index.
	"SUB r5,r4\n\t"			// (1 cycle) remove the delta
	"saveindex1:\n\t"
	"STR r5,[r0, r1]\n\t"			// (2 cycle) save it
	// now we have these registers free: r4, r3, r2
	"LSR r7,r7,#8\n\t"			// (1 cycle) r7 = index >> 8
	"LDRSB r7,[r2, r7]\n\t"			// (2 cycles) r7 = channel_sample
	"MUL r7,r6\n\t"			// (1 cycle) channel_sample = channel_sample * volume
	"ADD r8,r7\n\t"			// (1 cycle) sample is r8; The channel 0 needs to initialize r8 to r6! Next channels instead adds it.
	// channel 2
	"LDMIA r0!,{r2,r3,r4,r5,r6, r7}\n\t"		// (8 cycles). R1 = -4; R2 = offset. R3 = length; R4 = delta;  R5 = increment ; R6= volume; R7 =  index
	"ADD r5,r7\n\t"			// (1 cycle) add increment to index and place the result to the no more used increment
	"CMP r5,r3\n\t"			// (1 cycle) r5 - r3 (inew ndex - length)
	"BLT saveindex2\n\t"			// (2 cycles if taken, 1 otherwise) if r5 < r3, then just save the index.
	"SUB r5,r4\n\t"			// (1 cycle) remove the delta
	"saveindex2:\n\t"
	"STR r5,[r0, r1]\n\t"			// (2 cycle) save it
	// now we have these registers free: r4, r3, r2
	"LSR r7,r7,#8\n\t"			// (1 cycle) r7 = index >> 8
	"LDRSB r7,[r2, r7]\n\t"			// (2 cycles) r7 = channel_sample
	"MUL r7,r6\n\t"			// (1 cycle) channel_sample = channel_sample * volume
	"ADD r8,r7\n\t"			// (1 cycle) sample is r8; The channel 0 needs to initialize r8 to r6! Next channels instead adds it.
	// channel 3
	// this is a bit modified to optimize!
	"LDMIA r0!,{r2,r3,r4,r5,r6,r7}\n\t"		// (8 cycles). R1 = -4; R2 = offset. R3 = length; R4 = delta;  R5 = increment ; R6= volume; R7 =  index
	"ADD r5,r7\n\t"			// (1 cycle) add increment to index and put the result to incremnent, which is no more used!
	"CMP r5,r3\n\t"			// (1 cycle) r4 - r2 (newIndex - length)
	"BLT saveindex3\n\t"			// (2 cycles if taken, 1 otherwise) if r4 < r2, then just save the index.
	"SUB r5,r4\n\t"			// (1 cycle) remove the delta
	"saveindex3:\n\t"
	"STR r5,[r0, r1]\n\t"			// (2 cycle) save it
	"LSR r7,r7,#8\n\t"			// (1 cycle) r0 = index >> 8
	"LDRSB r7,[r2, r7]\n\t"			// (2 cycles) r0 = channel_sample
	"MUL r7,r6\n\t"			// (1 cycle) channel_sample = channel_sample * volume
	"ADD r7,r8\n\t"			// (1 cycle) sample is r8; The channel 0 needs to initialize r8 to r6! Next channels instead adds it.
	//	"SUB r0,#8\n\t"			// (1 cycle) point back to index
	#if AUDIO_USES_LPF
	// FILTER: it performs sample(t) = sample(t-1) * filterCoeff + newValue(t) * ( 1 - filterCoeff)
	// this is also equal to:  (sample(t-1) - newValue(t)) * filterCoeff + newValue(t)
	"LDMIA r0!,{r2,r3, r4}\n\t" // r1 = -4; r2 = DAC register, r3 = filter coefficient, r4 = oldvalue (already shifted and multiplied)
	"SUB r5, r4, r7\n\t"		// r5 = oldValue - newValue
	"MUL r5, r3\n\t"			// r5 = r5 * filterCoeff
	"LSL r7, #14\n\t"			// newValue = newValue *4096
	"ADD r7, r7, r5\n\t"		// newValue = newValue + r5
	"ASR r7, #14\n\t"			// newValue / 4096
	"STR r7, [r0, r1]\n\t"		// store the premultipied value of the sample for the next cycle
	// END FILTER
	#else
	"LDR r2,[r0]\n\t" // r1 = -4; r2 = DAC register, r3 = filter coefficient, r4 = oldvalue (already shifted and multiplied)
	#endif
	"ASR r7,#9\n\t"			// (1 cycle)
	"ADD r7,#128\n\t"
	"ADD r7,#128\n\t"
	"STR r7, [r2]\n\t"			// (2 cycles) write to dac
	"B endAudio\n\t"
	".align(4)\n\t"
	"audioDataAddress:\n\t"
	".word audioMixerData\n\t"
	"endAudio:\n\t"
	:
	:
	:  "r0", "r1", "r2","r3","r4","r5","r6","r7","r8"
	);
	#endif
}



#endif /* AUDIOMIXER_H_ */