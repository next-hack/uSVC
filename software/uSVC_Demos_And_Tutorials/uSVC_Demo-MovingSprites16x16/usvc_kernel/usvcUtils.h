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

#ifndef USVCUTILS_H_
#define USVCUTILS_H_
#include <stdint.h>
uint32_t divide(uint32_t dividend, uint32_t divisor);
inline void fastAlignedMemCpy32(uint32_t *dst, uint32_t *src, uint16_t size)
{
	
	asm volatile 
	(
		"MOV r5,%[dst]\n\t"		// Move destination to r5
		"LSL r6,%[size],#5\n\t"		// multiply size by 32 and put it into r6.
		"ADD r6,r5\n\t"				// add destination address to size
		"MOV r0,%[src]\n\t"			// move source to r0
		"copyLoop%=:"				// copy 32 bytes each loop (about 23 cycles without wait, more with...)
			"LDMIA r0!,{r1, r2, r3, r4}\n\t"
			"STMIA r5!,{r1, r2, r3, r4}\n\t"
			"LDMIA r0!,{r1, r2, r3, r4}\n\t"
			"STMIA r5!,{r1, r2, r3, r4}\n\t"
			"CMP r5,r6\n\t"
		"BNE copyLoop%=\n\t"
		: 
		: [dst] "h" (dst), [src] "h" (src), [size] "l" (size)
		: "r0", "r1", "r2", "r3", "r4", "r5", "r6"
	);	
}
#endif /* USVCUTILS_H_ */