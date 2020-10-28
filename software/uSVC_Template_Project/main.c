/*
 *  USVC Template Project
 *
 *  Change/remove this copyright part when you create your own program!
 * 
 *  Copyright 2019-2020 Nicola Wrachien (next-hack.com)
 *
 *  This file is part of next-hack's USCV Template project.
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
 */
#include "main.h"
int main(void) 
{	
	// NOTE! The videoData structure must be correctly initialized BEFORE Calling initUsvc!
	// Note: if audio is enabled, replace NULL with the patches array!
	initUsvc(NULL);	  	// Init uSVC Hardware
	uint32_t lastTime = millis();
	int testLed = 0;
	while (1)   // Game main loop.
	{
		uint32_t timeNow = millis();
		waitForVerticalBlank();
		// optional, but recommended, led blinker to show that the program is alive!
		if (timeNow - lastTime > 1000UL)
		{
			setLed(testLed);
			testLed = 1 - testLed;
			lastTime = timeNow;
		}
	}
}