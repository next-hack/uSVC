/*
 * romTiles.h
 *
 * Created: 02/06/2020 10:54:33
 *  Author: PETN
 */ 


#ifndef ROMTILES_H_
#define ROMTILES_H_
#include <stdint.h>
#define MAXTILEINDEX 136
#define RAMTILES 40
#define TILESIZEX 8
#define TILESIZEY 8
extern const uint8_t tileData[MAXTILEINDEX][TILESIZEX * TILESIZEY ];




#endif /* ROMTILES_H_ */