/*-----------------------------------------------------------------------
/  PFF - Low level disk interface modlue include file    (C)ChaN, 2014
/-----------------------------------------------------------------------*/

#ifndef _DISKIO_DEFINED
#define _DISKIO_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#include "pff.h"


/* Status of Disk Functions */
typedef BYTE	DSTATUS;


/* Results of Disk Functions */
typedef enum {
	RES_OK = 0,		/* 0: Function succeeded */
	RES_ERROR,		/* 1: Disk error */
	RES_NOTRDY,		/* 2: Not ready */
	RES_PARERR		/* 3: Invalid parameter */
} DRESULT;


/*---------------------------------------*/
/* Prototypes for disk control functions */

DSTATUS disk_initialize (FATFS *fs);
extern DRESULT disk_readp (
BYTE *buff,		/* Pointer to the read buffer (NULL:Read bytes are forwarded to the stream) */
DWORD lba,		/* Sector number (LBA) */
WORD ofs,		/* Byte offset to read from (0..511) */
WORD cnt,		/* Number of bytes to read (ofs + cnt mus be <= 512) */
FATFS *fs
);
DRESULT disk_writep (const BYTE* buff, DWORD sc, FATFS *fs);

#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */

#ifdef __cplusplus
}
#endif

#endif	/* _DISKIO_DEFINED */
