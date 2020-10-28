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
*  usvc_utils.c/h: general purpose functions.
*
*/
#include "usvcUtils.h"
uint32_t divide(uint32_t dividend, uint32_t divisor)
{
	uint32_t q;
	// poor man's unsigned divide function.
	// this function does not deal with x/0. It returns 0xFFFFFFFF for n/0 (n!=0) and 1 for 0/0.
	// still, this occupies less space (about 50 bytes vs 2-300 bytes) than the more general (but likely faster) gcc division algorithm.
	asm volatile
	(
	"MOV r2,#0\n\t"
	"MOV r3,#0\n\t"
	"initial_shift:\n\t"
	"CMP %[divisor], %[dividend]\n\t"
	"BHS divideloop\n\t"
	"LSL %[divisor],#1\n\t"
	"ADD r2,#1\n\t"
	"CMP r2,#32\n\t"
	"BNE initial_shift\n\t"
	"divideloop:\n\t"
	"LSL r3,#1\n\t"
	"CMP %[dividend], %[divisor]\n\t"
	"BLO smaller\n\t"
	"ADD r3,#1\n\t"
	"SUB %[dividend],%[divisor]\n\t"
	"smaller:\n\t"
	"LSR %[divisor],#1\n\t"
	"SUB r2,#1\n\t"
	"CMP r2,#0\n\t"
	"BPL divideloop\n\t"
	"MOV %[quotient],r3\n\t"
	:[quotient] "=r" (q)
	:[dividend] "r" (dividend), [divisor] "r" (divisor)
	: "r2", "r3"
	);
	return q;
	// 	uint8_t n = 0;
	// 	uint32_t q = 0;
	// 	for (n = 0; n < 32; n++)
	// 	{
	// 		if (divisor >= dividend)
	// 			break;
	// 		divisor <<= 1;
	// 	}
	// 	for (int i = 0; i <= n; i++)
	// 	{
	// 		q <<= 1;
	// 		if (dividend >= divisor)
	// 		{
	// 			q |= 1;
	// 			dividend = dividend - divisor;
	// 		}
	// 		divisor >>= 1;
	// 	}
	// 	return q;
}
