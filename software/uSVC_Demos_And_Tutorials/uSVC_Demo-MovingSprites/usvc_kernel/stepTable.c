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
*  stepTable.c. Since the sampling frequency is fixed at 30kHz, and the
*  default sounds have 256 samples, for each note we need an increment
*  for decimation/integration.
*/
#include <stdint.h> 
const uint16_t stepTable[128] =
{
	0x0012, // Note 0 (C-1), frequency 8.175799, Step 0.070313
	0x0013, // Note 1 (C#-1), frequency 8.661957, Step 0.074219
	0x0014, // Note 2 (D-1), frequency 9.177024, Step 0.078125
	0x0015, // Note 3 (D#-1), frequency 9.722718, Step 0.082031
	0x0017, // Note 4 (E-1), frequency 10.300861, Step 0.089844
	0x0018, // Note 5 (F-1), frequency 10.913382, Step 0.093750
	0x0019, // Note 6 (F#-1), frequency 11.562326, Step 0.097656
	0x001B, // Note 7 (G-1), frequency 12.249857, Step 0.105469
	0x001C, // Note 8 (G#-1), frequency 12.978272, Step 0.109375
	0x001E, // Note 9 (A-1), frequency 13.750000, Step 0.117188
	0x0020, // Note 10 (A#-1), frequency 14.567618, Step 0.125000
	0x0022, // Note 11 (B-1), frequency 15.433853, Step 0.132813
	0x0024, // Note 12 (C-2), frequency 16.351598, Step 0.140625
	0x0026, // Note 13 (C#-2), frequency 17.323914, Step 0.148438
	0x0028, // Note 14 (D-2), frequency 18.354048, Step 0.156250
	0x002A, // Note 15 (D#-2), frequency 19.445436, Step 0.164063
	0x002D, // Note 16 (E-2), frequency 20.601722, Step 0.175781
	0x0030, // Note 17 (F-2), frequency 21.826764, Step 0.187500
	0x0033, // Note 18 (F#-2), frequency 23.124651, Step 0.199219
	0x0036, // Note 19 (G-2), frequency 24.499715, Step 0.210938
	0x0039, // Note 20 (G#-2), frequency 25.956544, Step 0.222656
	0x003C, // Note 21 (A-2), frequency 27.500000, Step 0.234375
	0x0040, // Note 22 (A#-2), frequency 29.135235, Step 0.250000
	0x0043, // Note 23 (B-2), frequency 30.867706, Step 0.261719
	0x0047, // Note 24 (C-3), frequency 32.703196, Step 0.277344
	0x004C, // Note 25 (C#-3), frequency 34.647829, Step 0.296875
	0x0050, // Note 26 (D-3), frequency 36.708096, Step 0.312500
	0x0055, // Note 27 (D#-3), frequency 38.890873, Step 0.332031
	0x005A, // Note 28 (E-3), frequency 41.203445, Step 0.351563
	0x005F, // Note 29 (F-3), frequency 43.653529, Step 0.371094
	0x0065, // Note 30 (F#-3), frequency 46.249303, Step 0.394531
	0x006B, // Note 31 (G-3), frequency 48.999429, Step 0.417969
	0x0071, // Note 32 (G#-3), frequency 51.913087, Step 0.441406
	0x0078, // Note 33 (A-3), frequency 55.000000, Step 0.468750
	0x007F, // Note 34 (A#-3), frequency 58.270470, Step 0.496094
	0x0087, // Note 35 (B-3), frequency 61.735413, Step 0.527344
	0x008F, // Note 36 (C-4), frequency 65.406391, Step 0.558594
	0x0097, // Note 37 (C#-4), frequency 69.295658, Step 0.589844
	0x00A0, // Note 38 (D-4), frequency 73.416192, Step 0.625000
	0x00AA, // Note 39 (D#-4), frequency 77.781746, Step 0.664063
	0x00B4, // Note 40 (E-4), frequency 82.406889, Step 0.703125
	0x00BF, // Note 41 (F-4), frequency 87.307058, Step 0.746094
	0x00CA, // Note 42 (F#-4), frequency 92.498606, Step 0.789063
	0x00D6, // Note 43 (G-4), frequency 97.998859, Step 0.835938
	0x00E3, // Note 44 (G#-4), frequency 103.826174, Step 0.886719
	0x00F0, // Note 45 (A-4), frequency 110.000000, Step 0.937500
	0x00FF, // Note 46 (A#-4), frequency 116.540940, Step 0.996094
	0x010E, // Note 47 (B-4), frequency 123.470825, Step 1.054688
	0x011E, // Note 48 (C-5), frequency 130.812783, Step 1.117188
	0x012F, // Note 49 (C#-5), frequency 138.591315, Step 1.183594
	0x0141, // Note 50 (D-5), frequency 146.832384, Step 1.253906
	0x0154, // Note 51 (D#-5), frequency 155.563492, Step 1.328125
	0x0168, // Note 52 (E-5), frequency 164.813778, Step 1.406250
	0x017D, // Note 53 (F-5), frequency 174.614116, Step 1.488281
	0x0194, // Note 54 (F#-5), frequency 184.997211, Step 1.578125
	0x01AC, // Note 55 (G-5), frequency 195.997718, Step 1.671875
	0x01C6, // Note 56 (G#-5), frequency 207.652349, Step 1.773438
	0x01E1, // Note 57 (A-5), frequency 220.000000, Step 1.878906
	0x01FD, // Note 58 (A#-5), frequency 233.081881, Step 1.988281
	0x021B, // Note 59 (B-5), frequency 246.941651, Step 2.105469
	0x023C, // Note 60 (C-6), frequency 261.625565, Step 2.234375
	0x025E, // Note 61 (C#-6), frequency 277.182631, Step 2.367188
	0x0282, // Note 62 (D-6), frequency 293.664768, Step 2.507813
	0x02A8, // Note 63 (D#-6), frequency 311.126984, Step 2.656250
	0x02D0, // Note 64 (E-6), frequency 329.627557, Step 2.812500
	0x02FB, // Note 65 (F-6), frequency 349.228231, Step 2.980469
	0x0328, // Note 66 (F#-6), frequency 369.994423, Step 3.156250
	0x0358, // Note 67 (G-6), frequency 391.995436, Step 3.343750
	0x038B, // Note 68 (G#-6), frequency 415.304698, Step 3.542969
	0x03C1, // Note 69 (A-6), frequency 440.000000, Step 3.753906
	0x03FA, // Note 70 (A#-6), frequency 466.163762, Step 3.976563
	0x0437, // Note 71 (B-6), frequency 493.883301, Step 4.214844
	0x0477, // Note 72 (C-7), frequency 523.251131, Step 4.464844
	0x04BB, // Note 73 (C#-7), frequency 554.365262, Step 4.730469
	0x0503, // Note 74 (D-7), frequency 587.329536, Step 5.011719
	0x054F, // Note 75 (D#-7), frequency 622.253967, Step 5.308594
	0x05A0, // Note 76 (E-7), frequency 659.255114, Step 5.625000
	0x05F6, // Note 77 (F-7), frequency 698.456463, Step 5.960938
	0x0651, // Note 78 (F#-7), frequency 739.988845, Step 6.316406
	0x06B1, // Note 79 (G-7), frequency 783.990872, Step 6.691406
	0x0716, // Note 80 (G#-7), frequency 830.609395, Step 7.085938
	0x0782, // Note 81 (A-7), frequency 880.000000, Step 7.507813
	0x07F5, // Note 82 (A#-7), frequency 932.327523, Step 7.957031
	0x086E, // Note 83 (B-7), frequency 987.766603, Step 8.429688
	0x08EE, // Note 84 (C-8), frequency 1046.502261, Step 8.929688
	0x0976, // Note 85 (C#-8), frequency 1108.730524, Step 9.460938
	0x0A06, // Note 86 (D-8), frequency 1174.659072, Step 10.023438
	0x0A9F, // Note 87 (D#-8), frequency 1244.507935, Step 10.621094
	0x0B40, // Note 88 (E-8), frequency 1318.510228, Step 11.250000
	0x0BEC, // Note 89 (F-8), frequency 1396.912926, Step 11.921875
	0x0CA1, // Note 90 (F#-8), frequency 1479.977691, Step 12.628906
	0x0D61, // Note 91 (G-8), frequency 1567.981744, Step 13.378906
	0x0E2D, // Note 92 (G#-8), frequency 1661.218790, Step 14.175781
	0x0F05, // Note 93 (A-8), frequency 1760.000000, Step 15.019531
	0x0FE9, // Note 94 (A#-8), frequency 1864.655046, Step 15.910156
	0x10DC, // Note 95 (B-8), frequency 1975.533205, Step 16.859375
	0x11DC, // Note 96 (C-9), frequency 2093.004522, Step 17.859375
	0x12EC, // Note 97 (C#-9), frequency 2217.461048, Step 18.921875
	0x140C, // Note 98 (D-9), frequency 2349.318143, Step 20.046875
	0x153D, // Note 99 (D#-9), frequency 2489.015870, Step 21.238281
	0x1681, // Note 100 (E-9), frequency 2637.020455, Step 22.503906
	0x17D7, // Note 101 (F-9), frequency 2793.825851, Step 23.839844
	0x1942, // Note 102 (F#-9), frequency 2959.955382, Step 25.257813
	0x1AC3, // Note 103 (G-9), frequency 3135.963488, Step 26.761719
	0x1C5A, // Note 104 (G#-9), frequency 3322.437581, Step 28.351563
	0x1E0A, // Note 105 (A-9), frequency 3520.000000, Step 30.039063
	0x1FD3, // Note 106 (A#-9), frequency 3729.310092, Step 31.824219
	0x21B7, // Note 107 (B-9), frequency 3951.066410, Step 33.714844
	0x23B8, // Note 108 (C-10), frequency 4186.009045, Step 35.718750
	0x25D8, // Note 109 (C#-10), frequency 4434.922096, Step 37.843750
	0x2818, // Note 110 (D-10), frequency 4698.636287, Step 40.093750
	0x2A7B, // Note 111 (D#-10), frequency 4978.031740, Step 42.480469
	0x2D01, // Note 112 (E-10), frequency 5274.040911, Step 45.003906
	0x2FAE, // Note 113 (F-10), frequency 5587.651703, Step 47.679688
	0x3284, // Note 114 (F#-10), frequency 5919.910763, Step 50.515625
	0x3585, // Note 115 (G-10), frequency 6271.926976, Step 53.519531
	0x38B4, // Note 116 (G#-10), frequency 6644.875161, Step 56.703125
	0x3C13, // Note 117 (A-10), frequency 7040.000000, Step 60.074219
	0x3FA6, // Note 118 (A#-10), frequency 7458.620184, Step 63.648438
	0x436E, // Note 119 (B-10), frequency 7902.132820, Step 67.429688
	0x4771, // Note 120 (C-11), frequency 8372.018090, Step 71.441406
	0x4BB0, // Note 121 (C#-11), frequency 8869.844191, Step 75.687500
	0x5031, // Note 122 (D-11), frequency 9397.272573, Step 80.191406
	0x54F5, // Note 123 (D#-11), frequency 9956.063479, Step 84.957031
	0x5A03, // Note 124 (E-11), frequency 10548.081821, Step 90.011719
	0x5F5D, // Note 125 (F-11), frequency 11175.303406, Step 95.363281
	0x6508, // Note 126 (F#-11), frequency 11839.821527, Step 101.031250
	0x6B0A // Note 127 (G-11), frequency 12543.853951, Step 107.039063
};
