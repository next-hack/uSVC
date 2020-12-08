/*
 * VGASpriteData.h
 *
 * Created: 02/06/2020 09:08:23
 *  Author: PETN
 */ 


#ifndef VGASPRITEDATA_H_
#define VGASPRITEDATA_H_
#include "main.h"
#define SPRITEDATA_NUMPIXELS 256
#define FRAMEDATA_NUMFRAMES 1
#define ANIMDATA_NUMANIM 1
#define ENTITYDATA_NUMENTITIES 1
#define ALLCOLORS 0
#define ALLCOLORS_DEFAULTANIM 0
#define ALLCOLORS_DEFAULTANIM_FRAMEINDEX 0x0000
#define ALLCOLORS_DEFAULTANIM_NUMFRAMES 0x0001
typedef struct
{
	uint16_t frameIndex;  //index to the first frame data
	uint8_t numFrames;  //number of frames
} anim_t;
extern const uint8_t spriteData[SPRITEDATA_NUMPIXELS];
extern const frame_t frameData[FRAMEDATA_NUMFRAMES];
extern const anim_t animData[ANIMDATA_NUMANIM];
extern const anim_t* entityData[ENTITYDATA_NUMENTITIES];
extern const uint16_t entityAnimStartIndex[ENTITYDATA_NUMENTITIES]; // this is redundant but by using this, the compiler will save a lot of RAM




#endif /* VGASPRITEDATA_H_ */