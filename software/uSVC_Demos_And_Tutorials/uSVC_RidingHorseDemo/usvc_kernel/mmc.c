/*-------------------------------------------------------------------------*/
/* PFF - Low level disk control module for AVR            (C)ChaN, 2010    */
/*-------------------------------------------------------------------------*/
#include "usvc_kernel.h"

/*-------------------------------------------------------------------------*/
/* Platform dependent macros and functions needed to be modified           */
/*-------------------------------------------------------------------------*/

#define SERCOM_FCK 48000000
#define F_SPI 12000000
#define F_SPI_SLOW 200000

#define MOSI_PIN_NUMBER 16
#define SCK_PIN_NUMBER 17
#define MISO_PIN_NUMBER 31
#define SELECT()	do{REG_PORT_DIRCLR0 = PORT_PA31; REG_PORT_OUTCLR0 = PORT_PA09; REG_PORT_DIRSET0 = PORT_PA09; }while(0) 			/* MISO INPUT, CS=low */
#define	DESELECT()	do{REG_PORT_OUTSET0 = PORT_PA09;  REG_PORT_DIRCLR0 = PORT_PA09;}while(0)			/* CS=high, MISO_OUTPUT */
#define	MMC_SEL		(!(REG_PORT_IN0 & PORT_PA09))			/* CS status (true:CS == L) */
#define SPI_FULL_SPEED()  do{  SERCOM1->SPI.CTRLA.bit.ENABLE = 0; while(SERCOM1->SPI.SYNCBUSY.bit.ENABLE); REG_SERCOM1_SPI_BAUD = SERCOM_FCK / (2 * F_SPI) - 1;  SERCOM1->SPI.CTRLA.bit.ENABLE = 1; while(SERCOM1->SPI.SYNCBUSY.bit.ENABLE); }while(0)
/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* Definitions for MMC/SDC command */
#define CMD0	(0x40+0)	/* GO_IDLE_STATE */
#define CMD1	(0x40+1)	/* SEND_OP_COND (MMC) */
#define	ACMD41	(0xC0+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(0x40+8)	/* SEND_IF_COND */
#define CMD16	(0x40+16)	/* SET_BLOCKLEN */
#define CMD17	(0x40+17)	/* READ_SINGLE_BLOCK */
#define CMD24	(0x40+24)	/* WRITE_BLOCK */
#define CMD55	(0x40+55)	/* APP_CMD */
#define CMD58	(0x40+58)	/* READ_OCR */


/* Card type flags (CardType) */
#define CT_MMC				0x01	/* MMC ver 3 */
#define CT_SD1				0x02	/* SD ver 1 */
#define CT_SD2				0x04	/* SD ver 2 */
#define CT_BLOCK			0x08	/* Block addressing */

#define loop_until_bit_is_set(a,b) do{}while (!(a & b))
	
// SPI get/release are required to relinquish/release the color signals, which are used also as SD I/O and CLK signals.
void spi_release()
{
 	PORT->Group[0].PINCFG[MOSI_PIN_NUMBER].reg =  PORT_PINCFG_INEN | PORT_PINCFG_DRVSTR;
 	PORT->Group[0].PINCFG[SCK_PIN_NUMBER].reg =  PORT_PINCFG_INEN | PORT_PINCFG_DRVSTR ;
 	PORT->Group[0].PINCFG[MISO_PIN_NUMBER].reg =  PORT_PINCFG_INEN | PORT_PINCFG_DRVSTR ;
	REG_PORT_DIRSET0 = PORT_PA31;
	setDisablePinValue(0);
}
void spi_attach()
{
	// disable screen output.
	setDisablePinValue(1);
	REG_PORT_DIRCLR0 = PORT_PA31;
	PORT->Group[0].PINCFG[MOSI_PIN_NUMBER].reg =  PORT_PINCFG_PMUXEN | PORT_PINCFG_INEN | PORT_PINCFG_DRVSTR ;
	PORT->Group[0].PINCFG[SCK_PIN_NUMBER].reg =  PORT_PINCFG_PMUXEN | PORT_PINCFG_INEN | PORT_PINCFG_DRVSTR;
	PORT->Group[0].PINCFG[MISO_PIN_NUMBER].reg =  PORT_PINCFG_PMUXEN | PORT_PINCFG_INEN | PORT_PINCFG_DRVSTR ;
}
/* Delay 100 microseconds*/
static void dly_100us (void)
{
	asm volatile
	(	
		"LDR r0,value%=\n\t"
		"B MMCdelayLoop%=\n\t"
		".align(4)\n\t"
		"value%=:\n\t"
		".word 1200\n\t"
		"MMCdelayLoop%=:\n\t"
		"SUB r0,#1\n\t"
		"CMP r0,#0\n\t"
		"BNE MMCdelayLoop%=\n\t"
		// cycles (excluding preamble): 2 LDR, 2 B,  4 * valueInR0 - 1. To achieve 100us (circa) we need: (48E6 *100E-6 - 3)/4 = 199
	:
	:
	: "r0"
	);
}

/* Exchange a byte */
void xmit_spi (		/* Returns received data */
	BYTE dat		/* Data to be sent */
)
{
	REG_SERCOM1_SPI_DATA = dat;
	loop_until_bit_is_set(REG_SERCOM1_SPI_INTFLAG, SERCOM_SPI_INTFLAG_TXC);
	uint8_t dummy = REG_SERCOM1_SPI_DATA;
	(void) dummy;
}

/* Send a 0xFF to the MMC and get the received byte (usi.S) */

BYTE rcv_spi (void)	/* Returns received data */
{
	REG_SERCOM1_SPI_DATA = 0xFF;
	loop_until_bit_is_set(REG_SERCOM1_SPI_INTFLAG, SERCOM_SPI_INTFLAG_TXC);
	return REG_SERCOM1_SPI_DATA;
}


static void init_spi (void)
{
	//Setting clock
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID( GCLK_CLKCTRL_ID_SERCOM1_CORE_Val ) | // Generic Clock 0 (SERCOMx)
	GCLK_CLKCTRL_GEN_GCLK0 | // Generic Clock Generator 0 is source
	GCLK_CLKCTRL_CLKEN ;
	//
	while ( GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY );
	//
	// Enable correct mux
	//
	REG_PORT_DIRSET0 = (1 << MOSI_PIN_NUMBER) | (1<<SCK_PIN_NUMBER);
	setPortMux(MOSI_PIN_NUMBER, PORT_PMUX_PMUXE_C_Val) ;        // Note: O_Val or E_Val : does not change.
	setPortMux(SCK_PIN_NUMBER, PORT_PMUX_PMUXE_C_Val) ;         // Note: O_Val or E_Val : does not change.
	setPortMux(MISO_PIN_NUMBER, PORT_PMUX_PMUXE_D_Val);         // Note: O_Val or E_Val : does not change.
	//
	// SPI MODE 0, MSB FIRST,
	REG_SERCOM1_SPI_CTRLA = 0; //SERCOM_SPI_CTRLA_DOPO(0) | SERCOM_SPI_CTRLA_DIPO(3) | SERCOM_SPI_CTRLA_MODE_SPI_MASTER ;
	REG_SERCOM1_SPI_CTRLB =  SERCOM_SPI_CTRLB_CHSIZE(8) | SERCOM_SPI_CTRLB_RXEN;
	// 200kHz
	REG_SERCOM1_SPI_BAUD = SERCOM_FCK / (2 * F_SPI_SLOW) - 1;
	REG_SERCOM1_SPI_CTRLA = SERCOM_SPI_CTRLA_ENABLE | SERCOM_SPI_CTRLA_DOPO(0) | SERCOM_SPI_CTRLA_DIPO(3) | SERCOM_SPI_CTRLA_MODE_SPI_MASTER;
	while(SERCOM1->SPI.SYNCBUSY.bit.ENABLE);
}


//static BYTE CardType;

//
static void deselectAndSPIReleaseAndWaitMicrosBeforeSPIAttach(uint16_t micros , uint8_t selectAfter)
{
	DESELECT();
	spi_release();
	cs_waitTillHasEnoughTime(micros);
	spi_attach();	
	if (selectAfter)
		SELECT();
}
/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/


BYTE send_cmd (
	BYTE cmd,		/* 1st byte (Start + Index) */
	DWORD arg		/* Argument (32 bits) */
)
{
	BYTE n, res;
// 	DESELECT();
// 	spi_release();
// 	cs_waitTillHasEnoughTime(100);
// 	spi_attach();
	deselectAndSPIReleaseAndWaitMicrosBeforeSPIAttach(300, 1);
//	SELECT();
	if (cmd & 0x80) {	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);

		if (res > 1) return res;
	}

	/* Select the card */
// 	DESELECT();
// 	spi_release();
// 	cs_waitTillHasEnoughTime(100);
// 	spi_attach();
	deselectAndSPIReleaseAndWaitMicrosBeforeSPIAttach(300, 0);
	rcv_spi();
	SELECT();
	rcv_spi();

	/* Send a command packet */
	xmit_spi(cmd);						/* Start + Command index */
// 	xmit_spi((BYTE)(arg >> 24));		/* Argument[31..24] */
// 	xmit_spi((BYTE)(arg >> 16));		/* Argument[23..16] */
// 	xmit_spi((BYTE)(arg >> 8));			/* Argument[15..8] */
// 	xmit_spi((BYTE)arg);				/* Argument[7..0] */
 	for (int i = 24; i >= 0; i-=8)
 	{

 		xmit_spi(arg >> i);
 	}
	n = 0x01;							/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;			/* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;			/* Valid CRC for CMD8(0x1AA) */
	xmit_spi(n);

	/* Receive a command response */
	n = 10;								/* Wait for a valid response in timeout of 10 attempts */
	do {
		res = rcv_spi();
	} while ((res & 0x80) && --n);
	return res;			/* Return with the response value */
}




/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (FATFS *fs)
{
	BYTE n, cmd, ty, ocr[4];
	UINT tmr;
#if _PF_USE_WRITE

	if (CardType && MMC_SEL) 
	{
		disk_writep(0, 0);	/* Finalize write process if it is in progress */
	}
#endif
	init_spi();		/* Initialize ports to control MMC */
	DESELECT();
	cs_waitForVerticalBlank();
	REG_PORT_DIRCLR0 = PORT_PA31;
	for (n = 10; n; n--) rcv_spi();	/* 80*10 dummy clocks with CS=H */

	ty = 0;
	if (send_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDv2 */

			for (n = 0; n < 4; n++) ocr[n] = rcv_spi();		/* Get trailing return value of R7 resp */
		
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) 
			{			/* The card can work at vdd range of 2.7-3.6V */
				for (tmr = 10000; tmr && send_cmd(ACMD41, 1UL << 30); tmr--) 
					{
						DESELECT();
						dly_100us();	/* Wait for leaving idle state (ACMD41 with HCS bit) */
						cs_waitTillHasEnoughTime(2500);
					}
			
				if (tmr && send_cmd(CMD58, 0) == 0) {		/* Check CCS bit in the OCR */


					for (n = 0; n < 4; n++) ocr[n] = rcv_spi();

					ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;	/* SDv2 (HC or SC) */
				}
			}
		} else {							/* SDv1 or MMCv3 */
			if (send_cmd(ACMD41, 0) <= 1) 	{
				ty = CT_SD1; cmd = ACMD41;	/* SDv1 */
			} else {
				ty = CT_MMC; cmd = CMD1;	/* MMCv3 */
			}
			for (tmr = 10000; tmr && send_cmd(cmd, 0); tmr--) dly_100us();	/* Wait for leaving idle state */
			if (!tmr || send_cmd(CMD16, 512) != 0)			/* Set R/W block length to 512 */
				ty = 0;
		}

	}
	fs->cardType = ty;
	DESELECT();
	rcv_spi();
	//full speed SPI
	SPI_FULL_SPEED();
	spi_release();
	return ty ? 0 : STA_NOINIT;
}

void diskReadMult(BYTE*buff, WORD ofs, WORD cnt)
{
	uint8_t data;		
	REG_SERCOM1_SPI_DATA = 0xFF;
	loop_until_bit_is_set(REG_SERCOM1_SPI_INTFLAG, SERCOM_SPI_INTFLAG_DRE);	
	REG_SERCOM1_SPI_DATA = 0xFF;
	for (int i = 0; i < 514; i++)
	{
		loop_until_bit_is_set(REG_SERCOM1_SPI_INTFLAG, SERCOM_SPI_INTFLAG_RXC);
		data = REG_SERCOM1_SPI_DATA;
		if (i < 512) 
			REG_SERCOM1_SPI_DATA = 0xFF;	
		if (i >= ofs && i < ofs + cnt)
		{
			*buff++ = data;
		}	
	}
// 	loop_until_bit_is_set(REG_SERCOM1_SPI_INTFLAG, SERCOM_SPI_INTFLAG_RXC);
// 	data = REG_SERCOM1_SPI_DATA;
// 	loop_until_bit_is_set(REG_SERCOM1_SPI_INTFLAG, SERCOM_SPI_INTFLAG_RXC);
// 	data = REG_SERCOM1_SPI_DATA;
}

/*-----------------------------------------------------------------------*/
/* Read partial sector                                                   */
/*-----------------------------------------------------------------------*/

DRESULT disk_readp (
	BYTE *buff,		/* Pointer to the read buffer (NULL:Read bytes are forwarded to the stream) */
	DWORD lba,		/* Sector number (LBA) */
	WORD ofs,		/* Byte offset to read from (0..511) */
	WORD cnt,		/* Number of bytes to read (ofs + cnt mus be <= 512) */
	FATFS *fs		// pointer to FATFS object, required for cardType
)
{
	DRESULT res;
	BYTE rc;
	WORD bc;											
	if (!(fs->cardType & CT_BLOCK)) lba *= 512;		/* Convert to byte address if needed */
	   res = RES_ERROR;
	deselectAndSPIReleaseAndWaitMicrosBeforeSPIAttach(1200, 1);
	if (send_cmd(CMD17, lba) == 0) 
	{		/* READ_SINGLE_BLOCK */


		bc = 40000;
		do {							/* Wait for data packet */
// 			DESELECT();
// 			spi_release();
// 			cs_waitTillHasEnoughTime(1200);
// 			spi_attach();
			deselectAndSPIReleaseAndWaitMicrosBeforeSPIAttach(1800, 1);
//			SELECT();
			rc = rcv_spi();

		} while (rc == 0xFF && --bc);


		if (rc == 0xFE) 
		{				/* A data packet arrived */
			diskReadMult(buff, ofs, cnt);
			res = RES_OK;
		}
	}
	DESELECT();
	rcv_spi();
	spi_release();
	return res;
}



/*-----------------------------------------------------------------------*/
/* Write partial sector                                                  */
/*-----------------------------------------------------------------------*/

#if PF_USE_WRITE
DRESULT disk_writep (
	const BYTE *buff,	/* Pointer to the bytes to be written (NULL:Initiate/Finalize sector write) */
	DWORD sa,			/* Number of bytes to send, Sector number (LBA) or zero */
	FATFS *fs
)
{
	DRESULT res;
	WORD bc;
	static WORD wc;
	deselectAndSPIReleaseAndWaitMicrosBeforeSPIAttach(2000, 0);
	res = RES_ERROR;

	if (buff) {		/* Send data bytes */
		bc = (WORD)sa;
		while (bc && wc) {		/* Send data bytes to the card */
			xmit_spi(*buff++);
			wc--; bc--;
		}
		res = RES_OK;
	} else {
		if (sa) {	/* Initiate sector write process */
			if (!(fs->cardType & CT_BLOCK)) sa *= 512;	/* Convert to byte address if needed */
			if (send_cmd(CMD24, sa) == 0) {			/* WRITE_SINGLE_BLOCK */
				xmit_spi(0xFF); xmit_spi(0xFE);		/* Data block header */
				wc = 512;							/* Set byte counter */
				res = RES_OK;
			}
		} else {	/* Finalize sector write process */
			bc = wc + 2;
			while (bc--) xmit_spi(0);	/* Fill left bytes and CRC with zeros */
			if ((rcv_spi() & 0x1F) == 0x05) {	/* Receive data resp and wait for end of write process in timeout of 500ms */
				for (bc = 5000; rcv_spi() != 0xFF && bc; bc--) dly_100us();	/* Wait ready */
				if (bc) res = RES_OK;
			}
			DESELECT();
			rcv_spi();
		}
	}
	spi_release();
	return res;
}
#endif
