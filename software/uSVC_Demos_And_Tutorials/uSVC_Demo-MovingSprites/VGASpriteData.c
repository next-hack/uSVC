
#include "VGASpriteData.h"
const uint8_t spriteData[SPRITEDATA_NUMPIXELS] =
{
	0x00, 0x00, 0xC7, 0xC7, 0xC7, 0xC7, 0x00, 0x00, 0x00, 0xC7, 0xDF, 0xDF, 0xDF, 0xC7, 0xC7, 0x00,
	0xC7, 0xDF, 0x01, 0xC7, 0xC7, 0x01, 0xC7, 0x97, 0xC7, 0xDF, 0x01, 0xC7, 0xC7, 0x01, 0x97, 0x97,
	0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0xC7, 0x97, 0x97, 0x97, 0xC7, 0x01, 0xC7, 0xC7, 0x01, 0x97, 0x87,
	0x00, 0x97, 0xC7, 0x01, 0x01, 0x97, 0x87, 0x00, 0x00, 0x00, 0x97, 0x97, 0x97, 0x87, 0x00, 0x00
};
const frame_t frameData[FRAMEDATA_NUMFRAMES] =
{
	{ .w = 0x08,  .h = 0x08,  .ox = 0x04,  .oy = 0x04,  .pData = &spriteData[0x00000] }
};
const anim_t animData[ANIMDATA_NUMANIM] =
{
	{ .frameIndex = 0x0000,  .numFrames = 0x01 }
};
const anim_t* entityData[ENTITYDATA_NUMENTITIES] =
{
	&animData[0x00]
};
const uint16_t entityAnimStartIndex[ENTITYDATA_NUMENTITIES] =
{
	0x00
};
