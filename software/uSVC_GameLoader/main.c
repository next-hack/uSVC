/*	USVC Game Loader and libraries
*
 *  Copyright 2019-2020 Nicola Wrachien (next-hack.com)
 *
 *  This file is part of next-hack's RedBalls.
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
 * main.c/h for bootloader. Compile this with "optimize for size" switch! 
 *
 */
#include "main.h"
#define GAME_START_ADDRESS 0x6000
#define PREVIEW_SIZE_X 12
#define PREVIEW_SIZE_Y 9
#define PREVIEW_START_Y 6
#define PREVIEW_START_X (40 - 14)
#define NUMBER_OF_SECTORS_PER_IMAGE ((PREVIEW_SIZE_X * PREVIEW_SIZE_Y * TILE_SIZE_X * TILE_SIZE_Y + 511)/512)
#define VERSION "0.1"
extern uint32_t *_sfixed;
#define NUMBER_OF_CHARACTERS (128 + 6 - ' ')
uint8_t selectedFileName[13];  // 8.3+null
uint16_t keyBuffer[6] = {0, 0, 0, 0, 0, 0};
#define NO_USB 0			// 1 for testing without USB
#include <stdlib.h>
// we need a vector table in ram, because during erase and programming, any access to flash would stall the CMO+ so video would not  be generated.
void* ramVectorTable[16+44] __attribute__((__aligned__(256)));
const uint8_t colors[] = 
{
	COLOR_TORGB332(7, 5, 1),  // regions that won't be programmed
	COLOR_TORGB332(7, 5, 1),  // regions that won't be programmed
	COLOR_TORGB332( 0, 5, 3), // erased region
	COLOR_TORGB332( 0, 5, 3), // erased region
	COLOR_TORGB332( 1, 5, 1), //   programmed region
	COLOR_TORGB332( 1, 5, 1)
};  //  programmed region

// coordinates for printing the game info. 255 = do not print.
typedef struct
{
	uint8_t x;
	uint8_t y;
} xy_t;
const xy_t coordinates[] =
{
	{255, 0}, // short title
	{25, 1},  // title 1
	{25, 2},  // title 2
	{25, 3},  // title 3
	{25, 4},  // title 4
	{25, 16},  // desc 1
	{25, 17},  // desc 2
	{25, 18},  // desc 3
	{25, 19},  // desc 4
	{25 ,22},  // author 1
	{25 ,23},  // author 2
	{25, 21},  // date
	{25 + 9, 21},  // version
};
// 
#define FPB 5.5
const uint8_t startSound[] =
{
	0, BL_SET_ENV_DATA, 0, 1,
	57, BL_SET_ENV_DATA, 0xFF,	-4,
	//
	5,	BL_PLAY_NOTE,  BL_NOTE(BL_NOTE_B5),
	55, BL_PLAY_NOTE,   BL_NOTE(BL_NOTE_B5),
	22, BL_PLAY_NOTE,   BL_NOTE(BL_NOTE_B5),
	11, BL_PLAY_NOTE,   BL_NOTE(BL_NOTE_A5),
	22, BL_PLAY_NOTE,   BL_NOTE(BL_NOTE_A5),
	22, BL_PLAY_NOTE,   BL_NOTE(BL_NOTE_CS6),
	11, BL_PLAY_NOTE,   BL_NOTE(BL_NOTE_D6),
	11, BL_PLAY_NOTE,   BL_NOTE(BL_NOTE_CS6),
	11, BL_PLAY_NOTE,   BL_NOTE(BL_NOTE_A5),
	11, BL_PLAY_NOTE,   BL_NOTE(BL_NOTE_B5),
	55, BL_PLAY_NOTE,	0,		0,	
	255  // sound end
};		
const uint8_t menuSound[] =
{
	0, BL_SET_ENV_DATA, 255, 0,
	0,	BL_PLAY_NOTE,  BL_NOTE(BL_NOTE_C7),
	8, BL_SET_ENV_DATA, 0, 0,
	0, BL_SET_ENV_DATA, 255, 0,
	0,	BL_PLAY_NOTE,  BL_NOTE(BL_NOTE_C7),
	8, BL_SET_ENV_DATA, 0, 0,
	255  // sound end
};
FATFS fs;
uint8_t numberOfPages = 1;
static uint16_t g_frame = 0;	
#define UPPER_CASE_ONLY 0	
#define FILES_PER_PAGE 8
uint8_t fileNames[FILES_PER_PAGE][13];
uint16_t numberOfFiles;
uint16_t currentPage = 0;
int16_t selectedFile = 0 ;
uint32_t binChecksum = 0;
uint32_t binLength = 0;
const char usvcFileExt [] = "USC";
uint8_t fileBuffer[512];
#define PAGE_SIZE 64
void printLine (const char * text, int8_t col, uint8_t row)
{
	const uint8_t *pText = (uint8_t*) text;
	for (uint8_t c = col; c < 40 && *pText != 0; c++)
	{
		#if UPPER_CASE_ONLY
			uint8_t ch;
			ch = *pText++;
			if (ch >= 'a' && ch <= 'z')
				ch = ch - 'a' + 'A';
			vram[ row *  VRAMX + c] =  charTileSet + ch;

		#else
			int8_t character = *pText++ - ' ';
//			if (character < 0)
//				character = 0;
			vram[ row *  VRAMX + c] =  character;
		#endif
	}
}
#if GFX_MODE == TILE_MODE_BOOT && DEGUB_BOOTLOADER
void addLineAtBottom(char *text, uint16_t fgColor, uint16_t bkgColor)
{
	for (int y = 0; y < 24; y++)
	{
		for (int x = 0; x < 40; x++)
		{
			vram[y * 40 + x] = vram[(y + 1) * 40 + x];
		}	
	}
	for (int x = 0; x < 40; x++)
	{
		vram[24 * 40 + x] = 0;
	}
	printLine(text, 0, 24);
}
void debug(uint32_t i, uint8_t row)
{
	uint8_t buf[32];
	snprintf(buf,32,"VAL: 0X%08X",i);
	printLine(buf,0,row);
}
#endif
uint16_t listFiles (uint16_t filesToIgnore, uint8_t scanOnly)
{
	FRESULT res;
	DIR dir;
	FILINFO fno;  
	//uint8_t dirBuffer[512];
	//res = pf_opendir(&dir, "", dirBuffer, sizeof(dirBuffer) == 512); 
	// Updated 15/05/2020. Since we do not read files + list dirs at the same time, to prevent using too much stack space, we can use fileBuffer
	res = pf_opendir(&dir, "", fileBuffer, sizeof(fileBuffer) == 512);                     
	uint16_t u = 0;
	if (res == FR_OK) 
	{
		while (u < filesToIgnore + FILES_PER_PAGE || scanOnly) 
		{
//			res = pf_readdir(&dir, &fno, dirBuffer,  sizeof(dirBuffer) == 512);                   // Read a directory item 
			res = pf_readdir(&dir, &fno, fileBuffer,  sizeof(fileBuffer) == 512);                   // Read a directory item 
			if (res != FR_OK || fno.fname[0] == 0) 
				break;  // Break on error or end of dir 
			if (!(fno.fattrib & AM_DIR)) 
			{	
				int n = 0;
				uint8_t  match = 1;
				while (fno.fname[n] != '.' && fno.fname[n] != 0)
					n++;
				for (int m = 0; m < 3; m++)
				{
					uint8_t c = fno.fname[n + 1 + m];
					if (c >= 'a' && c <= 'z')
						c = c - ('a' - 'A');
					if (c != usvcFileExt[m])
						match = 0;
				}				
				if (match)
				{
					if (u >= filesToIgnore && !scanOnly)
					{
						// copy file name
						memcpy(fileNames[u - filesToIgnore], fno.fname, 13);
					}
					u++;					
				}
			}
		}
	}
	// now that all the files are scanned, try to open the selected one.
	return u;
}
void goToGame()
{
	// 	uint32_t app_start_address, tmpReg, zeroReg;
	// We need to disable all the interrupts we might have initialized.
	SysTick->CTRL = 0;
// 	NVIC_DisableIRQ( TCC1_IRQn ) ;
// 	NVIC_DisableIRQ( TCC0_IRQn ) ;
// 	NVIC_DisableIRQ( EVSYS_IRQn ) ;
    NVIC->ICER[0U] = ((1 << TCC1_IRQn) | (1 << TCC0_IRQn) | (1 << EVSYS_IRQn)) ;		// disable ALL the interrupts.
    __DSB();
    __ISB();	
//     NVIC->ICER[0U] = ((1 << TCC1_IRQn) | (1 << TCC0_IRQn) | (1 << EVSYS_IRQn)) ;		// disable ALL the interrupts.

	// WARNING! IT IS VERY IMPORTANT TO STOP THE USB BEFORE JUMPING TO THE APPLICATION!!!
	USB->HOST.CTRLA.reg = USB_CTRLA_SWRST;
	// NOTE: THE FOLLOWING SYNC WAIT IS MANDATORY!!!!
	while (USB->HOST.SYNCBUSY.bit.SWRST);
	uint32_t dummy;
	uint32_t address = GAME_START_ADDRESS;
	asm volatile
	( 
		"LDMIA %[address]!, {%[tmp]}\n\t"  // load stack pointer
		"MOV SP, %[tmp]\n\t"			   // set
		"LDMIA %[address]!, {%[tmp]}\n\t"  // load application address
		"BX %[tmp]\n\t"				   // jump
		: [tmp] "=&r" (dummy) 
		: [address] "r" (address)
	);
}
void unpackTiles(const uint8_t *buffer, uint16_t howManyBytes, uint8_t position)
{
	// tiles are stored in 8-bit format. Here we unpack them, having care also of choosing the correct values of the other signals.
	uint16_t *dest = tiles[position];
	for (int i = 0; i < howManyBytes; i++)
	{
		uint16_t colorSignal = ((*buffer++ * 1025) & 0b1100000011001111) | (1 << 12)  | (1 << 9);
		*dest++ = colorSignal;
	}
}
RAMFUNC void erase(uint32_t startAddr, uint32_t endAddr)
{
	// NOTE! in this function we must avoid touching the flash during erase, otherwise a stall will occur.
	#if AUDIO_ENGINE_ON_VBLANK_INTERRUPT
		NVIC_DisableIRQ( TCC0_IRQn );		// this will disable audio sound engine. Mandatory, to avoid accessing flash!
	#endif
	while (startAddr < endAddr)
	{	
		NVMCTRL->ADDR.reg = startAddr / 2;
		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
		while (NVMCTRL->INTFLAG.bit.READY == 0);
		uint8_t x = 3 + (startAddr >> 13);
		vram[x + 12 * VRAMX] = NUMBER_OF_CHARACTERS;
		vram[x + 13 * VRAMX] = NUMBER_OF_CHARACTERS + 1;		
		startAddr += PAGE_SIZE * 4; // point to next row
	}
	#if AUDIO_ENGINE_ON_VBLANK_INTERRUPT
		NVIC_EnableIRQ( TCC0_IRQn ) ;
	#endif
}
RAMFUNC void writeToFlash(uint8_t *buffer, uint32_t addr, uint16_t size)
{
	// NOTE! in this function we must avoid touching the flash during write, otherwise a stall will occur.
	#if AUDIO_ENGINE_ON_VBLANK_INTERRUPT
		NVIC_DisableIRQ( TCC0_IRQn );		// this will disable audio sound engine. Mandatory, to avoid accessing flash!	
	#endif
	uint32_t *src_addr = (uint32_t *) buffer;
	uint32_t *dst_addr = (uint32_t*)addr;
	// convert to word size
	size = size / 4;
	// Do writes in pages. Note manual page write is enabled at startup!
	while (size)
	{
		// Execute "PBC" Page Buffer Clear
		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
		while (NVMCTRL->INTFLAG.bit.READY == 0);
		uint32_t i;
		for (i=0; i < (PAGE_SIZE/4) && i < size; i++)
		{
			//dst_addr[i] = src_addr[i];
			*dst_addr++ = *src_addr++;
		}
		// Execute "WP" Write Page
		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
		while (NVMCTRL->INTFLAG.bit.READY == 0);
		// Advance to next page
// 		dst_addr += i;
// 		src_addr += i;
		size     -= i;
	}
	#if AUDIO_ENGINE_ON_VBLANK_INTERRUPT
		NVIC_EnableIRQ( TCC0_IRQn ) ;	
	#endif
}

void unpackLogoTiles(uint8_t c, uint8_t pos)
{
	// c = color set
	uint16_t *d = &tiles[NUMBER_OF_CHARACTERS + 1 + (MAXTILEINDEX + 1) * pos][0];
	const uint8_t *p = &tileData[0][0];
	for (int i = 0; i < MAXTILEINDEX * 16; i++)
	{   // note! Tile 0 is empty, so we won't store it! It's already initialized to 0!
		uint8_t byte = *p++;
		for (int j = 0; j < 4; j++)
		{
			uint8_t color = tilePalette[((byte >> (2 * j)) & 0x3) + 4 * c];
			uint16_t colorSignal = ((color * 1025) & 0b1100000011001111) | (1 << 12)  | (1 << 9);
			*d++ = colorSignal;
		}
	}		
}
void drawLogo(uint8_t xs, uint8_t ys, uint8_t colorSet)
{
	for (uint16_t y = 0; y < MAPSIZEY_LOGO; y++)
	{
		for (uint16_t  x = 0; x < MAPSIZEX_LOGO/2; x++)
		{
			uint8_t biTile = logoMap[ x + MAPSIZEX_LOGO/2 * y];
			vram[xs + 2 * x + (y + ys) * VRAMX] = (0xF & biTile) + NUMBER_OF_CHARACTERS + (MAXTILEINDEX + 1) * colorSet;
			vram[xs + 2 * x + 1 + (y + ys) * VRAMX] = (biTile >> 4) + NUMBER_OF_CHARACTERS + (MAXTILEINDEX + 1) * colorSet;
		}		
	}
}
uint16_t reasonablyFastUint16ToArray(uint16_t  n, uint8_t *array /*, uint8_t showLeadingZeroes*/)
{
	int number = n;
	const int multArray[] = {10000, 1000, 100, 10, 1};
	uint8_t nonZero = 0 /*showLeadingZeroes*/;
	int mult;
	int digit;
	int digitNumber = 0;
	for (int i = 0; i < sizeof(multArray)/sizeof(multArray[0]); i++)
	{
		mult = multArray[i];
		for (digit = 0; digit < 10 && number >= 0; digit++)
		{
			number -= mult;
		}
		if (digit > 1 || i == sizeof(multArray)/sizeof(multArray[0]) - 1)
			nonZero = 1;
		number += mult;
		if (nonZero)
			array[digitNumber++] = digit - 1 + (nonZero ? '0' : ' ');
	}
	array[digitNumber] = 0;
	return digitNumber;
}
void drawMenu(uint8_t page, uint8_t selectedItem, uint8_t pageChanged)
{
	// If we were in a time-constrained environment (e.g. a game) we would not redraw everytime everything. 
	// In the bootloader, instead, space is a premium, therefore instead of choosing where to draw (to avoid time consuming operations)
	// we just crear everything and redraw.
	// clear vram 
	if (pageChanged)
	{
		memset(fileNames, 0, sizeof(fileNames));
		listFiles(page * FILES_PER_PAGE, 0);
	}
	// clear screen
	memset(vram,0, sizeof(vram));
	// draw menu bars
	for (int x = 0; x < 24; x++)
	{
		vram[x + 3 * VRAMX] = 127 - 32;
		vram[x + 21 * VRAMX] = 127 - 32;
	}
	for (int y = 0; y < 25; y++)
	{
		vram[23 + y * VRAMX] = 127 - 32;
	}	
	printLine("uSVC game loader v" VERSION, 1, 1);	
	// print file names and the selected one
	for (int y = 0; y < FILES_PER_PAGE; y++)
	{
		printLine((char*)fileNames[y],3,  5 + y * 2);
		if (y == selectedItem)
			vram[1 + (y * 2 + 5) * VRAMX] =  '>' - ' ';
	}
	uint8_t buffer[6];
	printLine("Page ",1, 23);
	int n = reasonablyFastUint16ToArray(page + 1, buffer);
	printLine((char*)buffer,6, 23);
	vram[23 * VRAMX + 6 + n] = '/' - ' ';
	n += reasonablyFastUint16ToArray(numberOfPages, buffer);
	printLine((char*) buffer,6 + n, 23);		
	// 15-05-2020. Since when we read the file, we do not need a dirbuffer, we will use the single global file buffer, to prevent stack overflow
	uint8_t res;
	res = pf_open((char*)fileNames[selectedItem], fileBuffer,sizeof(fileBuffer) == 512);
	// first read the first sector to print all the information
	UINT br;
	pf_read(fileBuffer, 512, &br);
	if (br == 512)
	{
		// get length and checksum. tHese are already stored in little endian format.
		binChecksum = *((uint32_t *) &fileBuffer[4]);
		binLength = *((uint32_t *) &fileBuffer[8]);
		// coordinates to quickly draw: 255 in x => do not print
		for (int i = 0; i < sizeof(coordinates) / sizeof(xy_t); i++)
		{
			if (coordinates[i].x != 255)
				printLine((const char*)&fileBuffer[(i + 1) * 32], coordinates[i].x, coordinates[i].y);
		}	
	}
	else 
		res = 1;
	// now read everything
//	int32_t read = 0;	
	for (int i = 0; !res && i < NUMBER_OF_SECTORS_PER_IMAGE; i++)
	{
		res |= pf_read(fileBuffer, 512, &br);
		if (br == 512)
		{
			// tiles for the preview are saved in a rgb USVC format. We need to unpack them
			unpackTiles(fileBuffer, i == 13 ? 256 : 512, NUMBER_OF_CHARACTERS + i * 8);
		}
//		read+=br;
	}
	if (res)
	{
		binLength = 0;
		printLine("File Error",12, 23);
	}	
	// restore the zone where preview should appear.
	uint8_t tileN = NUMBER_OF_CHARACTERS;
	for (int y = PREVIEW_START_Y; y < PREVIEW_START_Y + PREVIEW_SIZE_Y; y++)
	{
		for (int x = PREVIEW_START_X; x < PREVIEW_START_X + PREVIEW_SIZE_X; x++)
		{
			vram[y * VRAMX + x] = tileN;
			tileN++;
		}
	}	
}
 int main(void) 
{	
	// copy vector table in ram. Important! When erasing or writing is performed NO flash access is possible 
	// (otherwise the bus - and consequently, the CortexM0+ will stall for several hundreds of microseconds, and the video signal will get corrupted.)
	uint8_t * pSrc = (void *) &_sfixed;
	memcpy (ramVectorTable, pSrc, sizeof(ramVectorTable));
	SCB->VTOR = ((uint32_t) ramVectorTable & SCB_VTOR_TBLOFF_Msk);
	initUsvc(NULL);	
	//
	// create the characters
	for (int c = ' '; c < 128 + 6; c++)
		putCharInTile(NULL, c, c != 127 ? 0xFF : COLOR_TORGB332( 7, 5, 1), 0, 0, (uint8_t*) tiles[c - ' '], 0);
	// unpack the 2-bit logo tiles with two different color sets
	for (int i = 0; i < 2; i++)
		unpackLogoTiles(i, i);
	memset(rowRemapTable, 27 * TILE_SIZE_Y - 1, 200);		
	drawLogo(3, 0, 1);
	drawLogo(3, 13, 0);
	bootloaderStartSong(startSound);
	uint8_t logoAnim = 0;
	uint16_t oldKey = 0;
	// mount the fat file system and read the number of files.	
	pf_mount(&fs);
	numberOfFiles = listFiles(0,1);
	numberOfPages = (numberOfFiles + FILES_PER_PAGE - 1) / FILES_PER_PAGE;
	while (1)
	{
		waitForVerticalBlank();
#if !AUDIO_ENGINE_ON_VBLANK_INTERRUPT
	#if AUDIO_ENABLED
			soundEngine();			// called here to be sure it will be called at an almost constant 		
	#elif USE_BL_AUDIO
		bootloaderSoundEngine();
	#endif
#endif
		if (g_frame > 125) // wait till the monitor gets in synch
		{
			if (logoAnim < 13 * TILESIZEY)
			{
			    // we want the logo (which is between 0 and 13 * TILESIZEY to be placed about in the center, i.e. starting from 6 * TILESIZEY
				for (uint8_t y = 0; y < 13 * TILESIZEY - logoAnim; y++)
					rowRemapTable[6 * TILE_SIZE_Y + y] = 13 * TILESIZEY - 1 - logoAnim;
				rowRemapTable[6 * TILE_SIZE_Y + 13 * TILESIZEY - logoAnim] = 26 * TILESIZEY - logoAnim;				
			}
			else if (logoAnim ==  13 * TILESIZEY)
			{
				rowRemapTable[6 * TILE_SIZE_Y + 13 * TILESIZEY - logoAnim] = 26 * TILESIZEY - logoAnim;	
				printLine("Another madness from next-hack!", 6, 11 + 13);
			}
			else if (logoAnim < 13 * TILESIZEY + 4*13 )
			{ // fade in-out
				unpackLogoTiles( (logoAnim - 13 * TILESIZEY + 1) >> 2, 0);
			}
			else if (logoAnim == 13 * TILESIZEY + 4 * 13 + 57 )
			{
				// The following "goToGame();" line is uncommented only for game debug! Leave it commented!
				//goToGame();
				// clear logo tiles
				memset(&tiles[NUMBER_OF_CHARACTERS], 0, sizeof(tiles[0]) * (MAX_TILES - NUMBER_OF_CHARACTERS));
				memset(vram,0, sizeof(vram));
				// restore the 1:1 row remapping
				for (int i = 0; i < 200; i++)
					rowRemapTable[i] = i;
				drawMenu(0, 0, 1);
			}
			else if (logoAnim > 13 * TILESIZEY + 4 * 13 + 57 )
			{
				logoAnim = 253;		// put any number larger than the previous case, but smaller than 255.
				uint8_t oldPage = currentPage;
				uint16_t key = keyBuffer[0];
				uint8_t redrawRequired = 0;
				if (oldKey != key)
				 {
					if (EXBUFF_TO_OEM(key) == USB_KEY_DOWN)
					{			
						redrawRequired = 1;		
						selectedFile++;
						// let's see if there are more files.
						if (selectedFile >= numberOfFiles)
						{
							// go back to page 0
							selectedFile = 0;
						}
					}
					else if (EXBUFF_TO_OEM(key) == USB_KEY_UP)
					{
						redrawRequired = 1;
						if (selectedFile > 0)
						{
							selectedFile--;
						}
						else
						{
							selectedFile = max(0, numberOfFiles - 1);
						}
					}
					else if (key == '\r' && binLength)
					{
						redrawRequired = 1;
						// first do we need to program the file? calculate the checksum and see if there is something to do.
						uint32_t *p = (uint32_t*) GAME_START_ADDRESS;
						uint32_t checksum = 0;
						for (int i = 0; i < binLength/4; i++)
						{
							checksum += *p++;
						}
						if (checksum != binChecksum)
						{
							// different game loaded. Erase and write.
							memset(vram, 0, sizeof(vram));
							printLine ("Erasing...", 10,10);
							// 
							for (int i = 0; i < 6; i++)
							{
								uint8_t color = colors[i];
// 								if (i < 2)	// regions that won't be programmed
// 									color = COLOR_TORGB332(7, 5, 1);
// 								else if (i < 4)	// erased region
// 									color = COLOR_TORGB332( 0, 5, 3);
// 								else  //  programmed region
// 									color = COLOR_TORGB332( 1, 5, 1);
								putCharInTile(NULL,132 + (i & 1), color,0,0, (uint8_t*) &tiles[NUMBER_OF_CHARACTERS + i - 2], 0);	
							}
							// bottom and top border
							for (int x = 3; x < 35; x++)
							{
								const uint8_t tv [] = {130 - ' ', 132 - ' ' , 133 - ' ',  131 - ' '};
								for (int i = 11; i <= 14; i++)
								{
									vram[x + i * VRAMX] = tv[i - 11];
								}
							}
							for (int y = 12; y < 14; y++)
							{
								vram[2 + y * VRAMX] = 128 - ' ';
								vram[35 + y * VRAMX] = 129 - ' ';								
							}
							// 
							erase(GAME_START_ADDRESS, binLength + GAME_START_ADDRESS);
							printLine ("Programming...", 10,10);
 							// erased. Now write program. The file was already open and selected. Therefore 
							uint32_t address = GAME_START_ADDRESS;
							UINT br; 
							while(binLength)
							{
								uint32_t remainingLength = binLength;
								if (remainingLength > 512)
									remainingLength = 512;
								pf_read(fileBuffer, remainingLength, &br);
								writeToFlash(fileBuffer, address, remainingLength);
								int x = 3 + (address >> 13);
								vram[x + 12 * VRAMX] = NUMBER_OF_CHARACTERS + 2;
								vram[x + 13 * VRAMX] = NUMBER_OF_CHARACTERS + 3;
								address += remainingLength;
								binLength -= remainingLength;
							}
						}
						goToGame();
					}
				}
				currentPage = selectedFile / FILES_PER_PAGE;
				if (redrawRequired)
				{
					bootloaderStartSong(menuSound);
					drawMenu(currentPage, selectedFile % FILES_PER_PAGE, oldPage != currentPage);				
				}
				oldKey = key; 
			}
			logoAnim++;
		}
		setLed(g_frame & 32);
		g_frame++;
		do
		{
			// Note! In bootloader, to save us from checking twice if the keyboard or gamepad are installed, the poll command is queued AFTER the usbHostTask is executed
			// However, since we are performing a loop, except the very first cycle after enumeration, there always be good data.
			#if !NO_USB
			usbHostTask();
			if (usbHidBootKeyboardIsInstalled())
			{
				usbKeyboardPoll();
				usbGetCurrentAsciiKeyboardStateEx(keyBuffer);
			}			
			if (usbHidGenericGamepadIsInstalled())
			{
				gamePadState_t gps;
				usbHidGenericGamepadPoll();
				// gamepad to keyboard conversion
				getCurrentGamepadState(&gps);
				keyBuffer[0] = 0;
				if (gps.axes[1] == gps.XYZRxMinimum)
				{

					if (gps.buttons == (GP_BUTTON_R1 | GP_BUTTON_L1 | GP_BUTTON_L2 | GP_BUTTON_R2))
						keyBuffer[0] = USB_KEY_RESET;
					else
						keyBuffer[0] = USB_KEY_UP << 8;
				}
				else if (gps.axes[1] == gps.XYZRxMaximum)
					keyBuffer[0] = USB_KEY_DOWN << 8;
				if (gps.buttons & (GP_BUTTON_1 | GP_BUTTON_2 | GP_BUTTON_3 | GP_BUTTON_4))
					keyBuffer[0] = '\r';						
			}
			if (keyBuffer[0] == USB_KEY_RESET)
			{  //  reset.
				SCB->AIRCR  = ((0x5FAUL << SCB_AIRCR_VECTKEY_Pos) |  SCB_AIRCR_SYSRESETREQ_Msk); 
				while(1);
			}
			#endif
		} while (getCurrentScanLineNumber() < 523); 
	}
}
