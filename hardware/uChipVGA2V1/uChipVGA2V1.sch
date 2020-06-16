EESchema Schematic File Version 4
LIBS:uChipVGA2V1-cache
EELAYER 26 0
EELAYER END
$Descr A3 16535 11693
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L uChipProjects:uChip1V5 U3
U 1 1 5D98678B
P 6950 2050
F 0 "U3" H 8125 2615 50  0000 C CNN
F 1 "uChip1V5" H 8125 2524 50  0000 C CNN
F 2 "uChipProjects:uChip1V5_LargePads" H 7900 2650 50  0001 C CNN
F 3 "" H 7900 2650 50  0001 C CNN
	1    6950 2050
	1    0    0    -1  
$EndComp
$Comp
L Connector:DB15_Female_HighDensity_MountingHoles J4
U 1 1 5D98687B
P 12350 6100
F 0 "J4" H 12350 6967 50  0000 C CNN
F 1 "DB15_Female_HighDensity_MountingHoles" H 12350 6876 50  0000 C CNN
F 2 "uChipProjects:DSUB-15-HD_Female_Horizontal_P2.29x2.54mm_EdgePinOffset8.55mm_Housed_MountingHolesOffset11.00mm-Pads1.8mm" H 11400 6500 50  0001 C CNN
F 3 " ~" H 11400 6500 50  0001 C CNN
	1    12350 6100
	1    0    0    -1  
$EndComp
$Comp
L Oscillator:CXO_DIP8 X1
U 1 1 5D986A70
P 1450 2350
F 0 "X1" H 1700 2650 50  0000 L CNN
F 1 "CXO_DIP8 16 MHz" H 1700 2550 50  0000 L CNN
F 2 "uChipProjects:Oscillator_DIP-8" H 1900 2000 50  0001 C CNN
F 3 "http://cdn-reichelt.de/documents/datenblatt/B400/OSZI.pdf" H 1350 2350 50  0001 C CNN
	1    1450 2350
	1    0    0    -1  
$EndComp
$Comp
L 74xx:74HC245 U2
U 1 1 5D986C46
P 7100 5400
F 0 "U2" H 7200 6200 50  0000 C CNN
F 1 "74AHC245" H 7350 6100 50  0000 C CNN
F 2 "uChipProjects:DIP-20_W7.62mm_Socket_LongPads" H 7100 5400 50  0001 C CNN
F 3 "http://www.ti.com/lit/gpn/sn74HC245" H 7100 5400 50  0001 C CNN
	1    7100 5400
	1    0    0    -1  
$EndComp
$Comp
L Connector:USB_B J1
U 1 1 5D9872F6
P 1200 5100
F 0 "J1" H 1255 5567 50  0000 C CNN
F 1 "USB_B" H 1255 5476 50  0000 C CNN
F 2 "uChipProjects:USB_B_OST_USB-B1HSxx_Horizontal" H 1350 5050 50  0001 C CNN
F 3 " ~" H 1350 5050 50  0001 C CNN
	1    1200 5100
	1    0    0    -1  
$EndComp
Wire Wire Line
	12650 6100 13200 6100
Wire Wire Line
	13200 6100 13200 6850
Wire Wire Line
	12650 6300 12900 6300
Wire Wire Line
	12900 6300 12900 6850
$Comp
L Device:R R11
U 1 1 5D987774
P 12900 7000
F 0 "R11" H 12970 7046 50  0000 L CNN
F 1 "22" H 12970 6955 50  0000 L CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 12830 7000 50  0001 C CNN
F 3 "~" H 12900 7000 50  0001 C CNN
	1    12900 7000
	1    0    0    -1  
$EndComp
$Comp
L Device:R R13
U 1 1 5D987822
P 13200 7000
F 0 "R13" H 13270 7046 50  0000 L CNN
F 1 "22" H 13270 6955 50  0000 L CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 13130 7000 50  0001 C CNN
F 3 "~" H 13200 7000 50  0001 C CNN
	1    13200 7000
	1    0    0    -1  
$EndComp
Wire Wire Line
	12900 7150 12900 7600
Wire Wire Line
	12900 7600 12150 7600
Text Label 12150 7600 0    50   ~ 0
VSYNC
Wire Wire Line
	13200 7150 13200 7750
Wire Wire Line
	13200 7750 12150 7750
Text Label 12150 7750 0    50   ~ 0
HSYNC
$Comp
L power:GND #PWR030
U 1 1 5D987942
P 12350 6950
F 0 "#PWR030" H 12350 6700 50  0001 C CNN
F 1 "GND" H 12355 6777 50  0000 C CNN
F 2 "" H 12350 6950 50  0001 C CNN
F 3 "" H 12350 6950 50  0001 C CNN
	1    12350 6950
	1    0    0    -1  
$EndComp
$Comp
L Device:R R9
U 1 1 5D9879A4
P 9400 6800
F 0 "R9" V 9193 6800 50  0000 C CNN
F 1 "390" V 9284 6800 50  0000 C CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 9330 6800 50  0001 C CNN
F 3 "~" H 9400 6800 50  0001 C CNN
	1    9400 6800
	0    1    1    0   
$EndComp
$Comp
L Device:R R10
U 1 1 5D987AD7
P 9400 7150
F 0 "R10" V 9193 7150 50  0000 C CNN
F 1 "820" V 9284 7150 50  0000 C CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 9330 7150 50  0001 C CNN
F 3 "~" H 9400 7150 50  0001 C CNN
	1    9400 7150
	0    1    1    0   
$EndComp
Text Label 8850 6800 0    50   ~ 0
bB1
Wire Wire Line
	9250 7150 8850 7150
Text Label 8850 7150 0    50   ~ 0
bB0
Wire Wire Line
	10200 7150 9550 7150
Wire Wire Line
	9550 6800 10200 6800
Wire Wire Line
	12050 6400 11750 6400
Wire Wire Line
	11750 6400 11750 6500
Wire Wire Line
	12050 6000 11750 6000
Wire Wire Line
	11750 6000 11750 6400
Connection ~ 11750 6400
Wire Wire Line
	12050 5800 11750 5800
Wire Wire Line
	11750 5800 11750 6000
Connection ~ 11750 6000
Wire Wire Line
	12050 5600 11750 5600
Wire Wire Line
	11750 5600 11750 5800
Connection ~ 11750 5800
Wire Wire Line
	12050 6500 11750 6500
Connection ~ 11750 6500
Wire Wire Line
	11750 6500 11750 6950
Wire Wire Line
	12350 6800 12350 6950
$Comp
L power:GND #PWR029
U 1 1 5D98941F
P 11750 6950
F 0 "#PWR029" H 11750 6700 50  0001 C CNN
F 1 "GND" H 11755 6777 50  0000 C CNN
F 2 "" H 11750 6950 50  0001 C CNN
F 3 "" H 11750 6950 50  0001 C CNN
	1    11750 6950
	1    0    0    -1  
$EndComp
Wire Wire Line
	10200 6800 10200 7150
$Comp
L Device:R R8
U 1 1 5D98979D
P 9400 6250
F 0 "R8" V 9193 6250 50  0000 C CNN
F 1 "2k2" V 9284 6250 50  0000 C CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 9330 6250 50  0001 C CNN
F 3 "~" H 9400 6250 50  0001 C CNN
	1    9400 6250
	0    1    1    0   
$EndComp
$Comp
L Device:R R7
U 1 1 5D98980B
P 9400 5900
F 0 "R7" V 9193 5900 50  0000 C CNN
F 1 "1k" V 9284 5900 50  0000 C CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 9330 5900 50  0001 C CNN
F 3 "~" H 9400 5900 50  0001 C CNN
	1    9400 5900
	0    1    1    0   
$EndComp
$Comp
L Device:R R6
U 1 1 5D98983B
P 9400 5550
F 0 "R6" V 9193 5550 50  0000 C CNN
F 1 "470" V 9284 5550 50  0000 C CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 9330 5550 50  0001 C CNN
F 3 "~" H 9400 5550 50  0001 C CNN
	1    9400 5550
	0    1    1    0   
$EndComp
Wire Wire Line
	10200 5900 10200 5550
Wire Wire Line
	10200 5550 9550 5550
Wire Wire Line
	9550 5900 10200 5900
Wire Wire Line
	9550 6250 10200 6250
Wire Wire Line
	9250 6800 8850 6800
Wire Wire Line
	9250 6250 8850 6250
Wire Wire Line
	9250 5900 8850 5900
Wire Wire Line
	9250 5550 8850 5550
Text Label 8850 5550 0    50   ~ 0
bG2
Text Label 8850 5900 0    50   ~ 0
bG1
Text Label 8850 6250 0    50   ~ 0
bG0
Connection ~ 10200 5900
Wire Wire Line
	12050 5900 10200 5900
Wire Wire Line
	10200 5900 10200 6250
Wire Wire Line
	10200 6800 11100 6800
Wire Wire Line
	11100 6800 11100 6100
Wire Wire Line
	11100 6100 12050 6100
Connection ~ 10200 6800
$Comp
L Device:R R5
U 1 1 5D98DF49
P 9400 5000
F 0 "R5" V 9193 5000 50  0000 C CNN
F 1 "2k2" V 9284 5000 50  0000 C CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 9330 5000 50  0001 C CNN
F 3 "~" H 9400 5000 50  0001 C CNN
	1    9400 5000
	0    1    1    0   
$EndComp
$Comp
L Device:R R4
U 1 1 5D98DF50
P 9400 4650
F 0 "R4" V 9193 4650 50  0000 C CNN
F 1 "1k" V 9284 4650 50  0000 C CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 9330 4650 50  0001 C CNN
F 3 "~" H 9400 4650 50  0001 C CNN
	1    9400 4650
	0    1    1    0   
$EndComp
$Comp
L Device:R R3
U 1 1 5D98DF57
P 9400 4300
F 0 "R3" V 9193 4300 50  0000 C CNN
F 1 "470" V 9284 4300 50  0000 C CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 9330 4300 50  0001 C CNN
F 3 "~" H 9400 4300 50  0001 C CNN
	1    9400 4300
	0    1    1    0   
$EndComp
Wire Wire Line
	10200 4650 10200 4300
Wire Wire Line
	10200 4300 9550 4300
Wire Wire Line
	9550 4650 10200 4650
Wire Wire Line
	9550 5000 10200 5000
Wire Wire Line
	9250 5000 8850 5000
Wire Wire Line
	9250 4650 8850 4650
Wire Wire Line
	9250 4300 8850 4300
Text Label 8850 4300 0    50   ~ 0
bR2
Text Label 8850 4650 0    50   ~ 0
bR1
Text Label 8850 5000 0    50   ~ 0
bR0
Connection ~ 10200 4650
Wire Wire Line
	10200 4650 10200 5000
Wire Wire Line
	11100 5700 12050 5700
Text Label 11300 5700 0    50   ~ 0
R
Text Label 11300 5900 0    50   ~ 0
G
Text Label 11300 6100 0    50   ~ 0
B
$Comp
L power:+3.3V #PWR025
U 1 1 5D991124
P 7100 4600
F 0 "#PWR025" H 7100 4450 50  0001 C CNN
F 1 "+3.3V" H 7115 4773 50  0000 C CNN
F 2 "" H 7100 4600 50  0001 C CNN
F 3 "" H 7100 4600 50  0001 C CNN
	1    7100 4600
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR022
U 1 1 5D9912E0
P 6000 5800
F 0 "#PWR022" H 6000 5650 50  0001 C CNN
F 1 "+3.3V" H 6015 5973 50  0000 C CNN
F 2 "" H 6000 5800 50  0001 C CNN
F 3 "" H 6000 5800 50  0001 C CNN
	1    6000 5800
	1    0    0    -1  
$EndComp
Wire Wire Line
	6000 5800 6600 5800
Wire Wire Line
	6600 4900 6250 4900
Text Label 6250 4900 0    50   ~ 0
R2
Wire Wire Line
	6600 5000 6250 5000
Text Label 6250 5000 0    50   ~ 0
R1
Wire Wire Line
	6600 5100 6250 5100
Text Label 6250 5100 0    50   ~ 0
R0
Wire Wire Line
	6600 5200 6250 5200
Text Label 6250 5200 0    50   ~ 0
G2
Wire Wire Line
	6600 5300 6250 5300
Text Label 6250 5300 0    50   ~ 0
G1
Wire Wire Line
	6600 5400 6250 5400
Text Label 6250 5400 0    50   ~ 0
G0
Wire Wire Line
	6600 5500 6250 5500
Text Label 6250 5500 0    50   ~ 0
B1
Wire Wire Line
	6600 5600 6250 5600
Text Label 6250 5600 0    50   ~ 0
B0
Text Label 7950 4900 2    50   ~ 0
bR2
Text Label 7950 5000 2    50   ~ 0
bR1
Text Label 7950 5100 2    50   ~ 0
bR0
Text Label 7950 5200 2    50   ~ 0
bG2
Text Label 7950 5300 2    50   ~ 0
bG1
Text Label 7950 5400 2    50   ~ 0
bG0
Text Label 7950 5500 2    50   ~ 0
bB1
Text Label 7950 5600 2    50   ~ 0
bB0
Wire Wire Line
	7950 4900 7600 4900
Wire Wire Line
	7950 5000 7600 5000
Wire Wire Line
	7950 5100 7600 5100
Wire Wire Line
	7950 5200 7600 5200
Wire Wire Line
	7950 5300 7600 5300
Wire Wire Line
	7950 5400 7600 5400
Wire Wire Line
	7950 5500 7600 5500
Wire Wire Line
	7950 5600 7600 5600
Wire Wire Line
	6600 5900 6250 5900
Text Label 6250 5900 0    50   ~ 0
DISABLE
$Comp
L power:GND #PWR026
U 1 1 5D9AB57B
P 7100 6250
F 0 "#PWR026" H 7100 6000 50  0001 C CNN
F 1 "GND" H 7105 6077 50  0000 C CNN
F 2 "" H 7100 6250 50  0001 C CNN
F 3 "" H 7100 6250 50  0001 C CNN
	1    7100 6250
	1    0    0    -1  
$EndComp
Wire Wire Line
	7100 6200 7100 6250
$Comp
L power:GND #PWR03
U 1 1 5D9AD1B4
P 1150 5600
F 0 "#PWR03" H 1150 5350 50  0001 C CNN
F 1 "GND" H 1155 5427 50  0000 C CNN
F 2 "" H 1150 5600 50  0001 C CNN
F 3 "" H 1150 5600 50  0001 C CNN
	1    1150 5600
	1    0    0    -1  
$EndComp
Wire Wire Line
	1100 5500 1100 5550
Wire Wire Line
	1100 5550 1150 5550
Wire Wire Line
	1200 5550 1200 5500
Wire Wire Line
	1150 5550 1150 5600
Connection ~ 1150 5550
Wire Wire Line
	1150 5550 1200 5550
Wire Wire Line
	1500 4900 2000 4900
Wire Wire Line
	2000 4900 2000 4800
$Comp
L power:+5V #PWR06
U 1 1 5D9B2C03
P 2000 4800
F 0 "#PWR06" H 2000 4650 50  0001 C CNN
F 1 "+5V" H 2015 4973 50  0000 C CNN
F 2 "" H 2000 4800 50  0001 C CNN
F 3 "" H 2000 4800 50  0001 C CNN
	1    2000 4800
	1    0    0    -1  
$EndComp
Wire Wire Line
	2000 4900 2000 5000
Connection ~ 2000 4900
$Comp
L Device:CP C2
U 1 1 5D9B4E0E
P 2000 5150
F 0 "C2" H 2118 5196 50  0000 L CNN
F 1 "100u" H 2118 5105 50  0000 L CNN
F 2 "uChipProjects:CP_Radial_D6.3mm_P2.50mm" H 2038 5000 50  0001 C CNN
F 3 "~" H 2000 5150 50  0001 C CNN
	1    2000 5150
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR07
U 1 1 5D9B4EB5
P 2000 5400
F 0 "#PWR07" H 2000 5150 50  0001 C CNN
F 1 "GND" H 2005 5227 50  0000 C CNN
F 2 "" H 2000 5400 50  0001 C CNN
F 3 "" H 2000 5400 50  0001 C CNN
	1    2000 5400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2000 5300 2000 5400
Wire Wire Line
	10200 5000 11100 5000
Connection ~ 10200 5000
Wire Wire Line
	11100 5000 11100 5700
$Comp
L uChipProjects:MCP1700-3302E-TO U1
U 1 1 5D9CC830
P 3050 4950
F 0 "U1" H 3050 5242 50  0000 C CNN
F 1 "MCP1700-3302E-TO" H 3050 5151 50  0000 C CNN
F 2 "uChipProjects:TO-92L_HandSolder" H 4000 4750 50  0001 C CNN
F 3 "" H 3050 5100 50  0001 C CNN
	1    3050 4950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR012
U 1 1 5D9CC8C4
P 3050 5250
F 0 "#PWR012" H 3050 5000 50  0001 C CNN
F 1 "GND" H 3055 5077 50  0000 C CNN
F 2 "" H 3050 5250 50  0001 C CNN
F 3 "" H 3050 5250 50  0001 C CNN
	1    3050 5250
	1    0    0    -1  
$EndComp
Wire Wire Line
	3050 5200 3050 5250
Wire Wire Line
	2750 4900 2550 4900
Wire Wire Line
	3350 4900 3600 4900
$Comp
L Device:C C4
U 1 1 5D9D2FE0
P 3600 5150
F 0 "C4" H 3715 5196 50  0000 L CNN
F 1 "1u" H 3715 5105 50  0000 L CNN
F 2 "uChipProjects:C_Disc_D5.0mm_W2.5mm_P5.00mm" H 3638 5000 50  0001 C CNN
F 3 "~" H 3600 5150 50  0001 C CNN
	1    3600 5150
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR011
U 1 1 5D9D30B1
P 2550 5400
F 0 "#PWR011" H 2550 5150 50  0001 C CNN
F 1 "GND" H 2555 5227 50  0000 C CNN
F 2 "" H 2550 5400 50  0001 C CNN
F 3 "" H 2550 5400 50  0001 C CNN
	1    2550 5400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2550 5300 2550 5400
Wire Wire Line
	2550 5000 2550 4900
Connection ~ 2550 4900
Wire Wire Line
	2550 4900 2000 4900
$Comp
L Device:C C3
U 1 1 5D9D9EE2
P 2550 5150
F 0 "C3" H 2665 5196 50  0000 L CNN
F 1 "1u" H 2665 5105 50  0000 L CNN
F 2 "uChipProjects:C_Disc_D5.0mm_W2.5mm_P5.00mm" H 2588 5000 50  0001 C CNN
F 3 "~" H 2550 5150 50  0001 C CNN
	1    2550 5150
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR015
U 1 1 5D9DC4CA
P 3600 5400
F 0 "#PWR015" H 3600 5150 50  0001 C CNN
F 1 "GND" H 3605 5227 50  0000 C CNN
F 2 "" H 3600 5400 50  0001 C CNN
F 3 "" H 3600 5400 50  0001 C CNN
	1    3600 5400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3600 5300 3600 5400
Wire Wire Line
	3600 5000 3600 4900
$Comp
L Device:C C1
U 1 1 5D9E385E
P 700 2350
F 0 "C1" H 815 2396 50  0000 L CNN
F 1 "1u" H 815 2305 50  0000 L CNN
F 2 "uChipProjects:C_Disc_D5.0mm_W2.5mm_P5.00mm" H 738 2200 50  0001 C CNN
F 3 "~" H 700 2350 50  0001 C CNN
	1    700  2350
	1    0    0    -1  
$EndComp
$Comp
L Device:C C7
U 1 1 5D9E3970
P 7850 6100
F 0 "C7" H 7965 6146 50  0000 L CNN
F 1 "1u" H 7965 6055 50  0000 L CNN
F 2 "uChipProjects:C_Disc_D5.0mm_W2.5mm_P5.00mm" H 7888 5950 50  0001 C CNN
F 3 "~" H 7850 6100 50  0001 C CNN
	1    7850 6100
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR027
U 1 1 5D9E3A48
P 7850 5950
F 0 "#PWR027" H 7850 5800 50  0001 C CNN
F 1 "+3.3V" H 7865 6123 50  0000 C CNN
F 2 "" H 7850 5950 50  0001 C CNN
F 3 "" H 7850 5950 50  0001 C CNN
	1    7850 5950
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR028
U 1 1 5D9E3AAB
P 7850 6250
F 0 "#PWR028" H 7850 6000 50  0001 C CNN
F 1 "GND" H 7855 6077 50  0000 C CNN
F 2 "" H 7850 6250 50  0001 C CNN
F 3 "" H 7850 6250 50  0001 C CNN
	1    7850 6250
	1    0    0    -1  
$EndComp
$Comp
L Device:C C6
U 1 1 5D9E8D89
P 4450 9400
F 0 "C6" H 4565 9446 50  0000 L CNN
F 1 "1u" H 4565 9355 50  0000 L CNN
F 2 "uChipProjects:C_Disc_D5.0mm_W2.5mm_P5.00mm" H 4488 9250 50  0001 C CNN
F 3 "~" H 4450 9400 50  0001 C CNN
	1    4450 9400
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR018
U 1 1 5D9E8D90
P 4450 9250
F 0 "#PWR018" H 4450 9100 50  0001 C CNN
F 1 "+3.3V" H 4465 9423 50  0000 C CNN
F 2 "" H 4450 9250 50  0001 C CNN
F 3 "" H 4450 9250 50  0001 C CNN
	1    4450 9250
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR019
U 1 1 5D9E8D96
P 4450 9550
F 0 "#PWR019" H 4450 9300 50  0001 C CNN
F 1 "GND" H 4455 9377 50  0000 C CNN
F 2 "" H 4450 9550 50  0001 C CNN
F 3 "" H 4450 9550 50  0001 C CNN
	1    4450 9550
	1    0    0    -1  
$EndComp
Wire Wire Line
	1150 2350 1050 2350
Wire Wire Line
	1050 2350 1050 2000
Wire Wire Line
	1050 2000 1450 2000
Wire Wire Line
	1450 2000 1450 2050
Wire Wire Line
	1450 2000 1450 1900
Connection ~ 1450 2000
Wire Wire Line
	1450 2650 1450 2750
$Comp
L power:GND #PWR05
U 1 1 5D9F6363
P 1450 2750
F 0 "#PWR05" H 1450 2500 50  0001 C CNN
F 1 "GND" H 1455 2577 50  0000 C CNN
F 2 "" H 1450 2750 50  0001 C CNN
F 3 "" H 1450 2750 50  0001 C CNN
	1    1450 2750
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR014
U 1 1 5D9F63F8
P 3600 4800
F 0 "#PWR014" H 3600 4650 50  0001 C CNN
F 1 "+3.3V" H 3615 4973 50  0000 C CNN
F 2 "" H 3600 4800 50  0001 C CNN
F 3 "" H 3600 4800 50  0001 C CNN
	1    3600 4800
	1    0    0    -1  
$EndComp
Wire Wire Line
	3600 4800 3600 4900
Connection ~ 3600 4900
$Comp
L power:+3.3V #PWR04
U 1 1 5D9FBF9F
P 1450 1900
F 0 "#PWR04" H 1450 1750 50  0001 C CNN
F 1 "+3.3V" H 1465 2073 50  0000 C CNN
F 2 "" H 1450 1900 50  0001 C CNN
F 3 "" H 1450 1900 50  0001 C CNN
	1    1450 1900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR02
U 1 1 5D9FBFD8
P 700 2500
F 0 "#PWR02" H 700 2250 50  0001 C CNN
F 1 "GND" H 705 2327 50  0000 C CNN
F 2 "" H 700 2500 50  0001 C CNN
F 3 "" H 700 2500 50  0001 C CNN
	1    700  2500
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR01
U 1 1 5D9FC011
P 700 2200
F 0 "#PWR01" H 700 2050 50  0001 C CNN
F 1 "+3.3V" H 715 2373 50  0000 C CNN
F 2 "" H 700 2200 50  0001 C CNN
F 3 "" H 700 2200 50  0001 C CNN
	1    700  2200
	1    0    0    -1  
$EndComp
$Comp
L Device:R R2
U 1 1 5D9FCC27
P 5550 9600
F 0 "R2" H 5480 9554 50  0000 R CNN
F 1 "390" H 5480 9645 50  0000 R CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 5480 9600 50  0001 C CNN
F 3 "~" H 5550 9600 50  0001 C CNN
	1    5550 9600
	-1   0    0    1   
$EndComp
$Comp
L Device:LED D1
U 1 1 5D9FCEF0
P 5550 9250
F 0 "D1" V 5588 9133 50  0000 R CNN
F 1 "YELOW LED" V 5497 9133 50  0000 R CNN
F 2 "uChipProjects:LED_Rectangular_W5.0mm_H2.0mm" H 5550 9250 50  0001 C CNN
F 3 "~" H 5550 9250 50  0001 C CNN
	1    5550 9250
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5550 9050 5550 9100
Wire Wire Line
	5550 9400 5550 9450
Wire Wire Line
	5550 9750 5550 9800
$Comp
L power:GND #PWR024
U 1 1 5DA05ADC
P 5550 9800
F 0 "#PWR024" H 5550 9550 50  0001 C CNN
F 1 "GND" H 5555 9627 50  0000 C CNN
F 2 "" H 5550 9800 50  0001 C CNN
F 3 "" H 5550 9800 50  0001 C CNN
	1    5550 9800
	1    0    0    -1  
$EndComp
Wire Wire Line
	2500 9500 2400 9500
Wire Wire Line
	2500 9700 2400 9700
$Comp
L power:GND #PWR010
U 1 1 5DA0C28C
P 2400 10250
F 0 "#PWR010" H 2400 10000 50  0001 C CNN
F 1 "GND" H 2405 10077 50  0000 C CNN
F 2 "" H 2400 10250 50  0001 C CNN
F 3 "" H 2400 10250 50  0001 C CNN
	1    2400 10250
	1    0    0    -1  
$EndComp
Wire Wire Line
	3500 1750 2950 1750
Text Label 2950 1750 0    50   ~ 0
R0
Wire Wire Line
	3500 1900 2950 1900
Text Label 2950 1900 0    50   ~ 0
HSYNC
Wire Wire Line
	3500 2050 2950 2050
Wire Wire Line
	3500 2200 2950 2200
Wire Wire Line
	3500 2500 2950 2500
Text Label 2950 2500 0    50   ~ 0
R2
Wire Wire Line
	3500 2650 2950 2650
Text Label 2950 2650 0    50   ~ 0
R1
Wire Wire Line
	1750 2350 3500 2350
Wire Wire Line
	3500 2800 3300 2800
Wire Wire Line
	3300 2800 3300 2950
$Comp
L power:GND #PWR013
U 1 1 5DA3C242
P 3300 2950
F 0 "#PWR013" H 3300 2700 50  0001 C CNN
F 1 "GND" H 3305 2777 50  0000 C CNN
F 2 "" H 3300 2950 50  0001 C CNN
F 3 "" H 3300 2950 50  0001 C CNN
	1    3300 2950
	1    0    0    -1  
$EndComp
Wire Wire Line
	12750 1750 13050 1750
Wire Wire Line
	13050 1750 13050 1550
$Comp
L power:+5V #PWR032
U 1 1 5DA40491
P 13050 1550
F 0 "#PWR032" H 13050 1400 50  0001 C CNN
F 1 "+5V" H 13065 1723 50  0000 C CNN
F 2 "" H 13050 1550 50  0001 C CNN
F 3 "" H 13050 1550 50  0001 C CNN
	1    13050 1550
	1    0    0    -1  
$EndComp
Wire Wire Line
	12750 1900 13200 1900
Text Label 13200 1900 2    50   ~ 0
VSYNC
Text Label 13200 2050 2    50   ~ 0
VDAC
Text Label 13200 2200 2    50   ~ 0
G0
Text Label 13200 2350 2    50   ~ 0
B1
Text Label 13200 2500 2    50   ~ 0
G2
Text Label 13200 2650 2    50   ~ 0
G1
Text Label 13200 2800 2    50   ~ 0
B0
Text Notes 15550 2500 2    50   ~ 10
NOTE!!! \nIf G0 is connected to some pull down node, then a \nstrong pull up is required, so that during reset G0\n stays high. Otherwise uChip won't boot up.
Wire Wire Line
	12750 2050 13200 2050
Wire Wire Line
	13200 2200 12750 2200
Wire Wire Line
	12750 2350 13200 2350
Wire Wire Line
	13200 2500 12750 2500
Wire Wire Line
	13200 2650 12750 2650
Wire Wire Line
	13200 2800 12750 2800
$Comp
L Device:CP C5
U 1 1 5DA5F201
P 4100 5150
F 0 "C5" H 4218 5196 50  0000 L CNN
F 1 "100u" H 4218 5105 50  0000 L CNN
F 2 "uChipProjects:CP_Radial_D6.3mm_P2.50mm" H 4138 5000 50  0001 C CNN
F 3 "~" H 4100 5150 50  0001 C CNN
	1    4100 5150
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR016
U 1 1 5DA5F27D
P 4100 5400
F 0 "#PWR016" H 4100 5150 50  0001 C CNN
F 1 "GND" H 4105 5227 50  0000 C CNN
F 2 "" H 4100 5400 50  0001 C CNN
F 3 "" H 4100 5400 50  0001 C CNN
	1    4100 5400
	1    0    0    -1  
$EndComp
Wire Wire Line
	4100 5300 4100 5400
Wire Wire Line
	4100 5000 4100 4900
Wire Wire Line
	4100 4900 3600 4900
$Comp
L Device:C C8
U 1 1 5DA68681
P 1350 6900
F 0 "C8" V 1098 6900 50  0000 C CNN
F 1 "1u" V 1189 6900 50  0000 C CNN
F 2 "uChipProjects:C_Disc_D5.0mm_W2.5mm_P5.00mm" H 1388 6750 50  0001 C CNN
F 3 "~" H 1350 6900 50  0001 C CNN
	1    1350 6900
	0    1    1    0   
$EndComp
$Comp
L Device:C C9
U 1 1 5DA68960
P 2300 7150
F 0 "C9" H 2185 7104 50  0000 R CNN
F 1 "22n" H 2185 7195 50  0000 R CNN
F 2 "uChipProjects:C_Disc_D5.0mm_W2.5mm_P5.00mm" H 2338 7000 50  0001 C CNN
F 3 "~" H 2300 7150 50  0001 C CNN
	1    2300 7150
	-1   0    0    1   
$EndComp
$Comp
L Device:R R12
U 1 1 5DA68D4A
P 1800 6900
F 0 "R12" V 1593 6900 50  0000 C CNN
F 1 "1k" V 1684 6900 50  0000 C CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 1730 6900 50  0001 C CNN
F 3 "~" H 1800 6900 50  0001 C CNN
	1    1800 6900
	0    1    1    0   
$EndComp
Wire Wire Line
	1650 6900 1500 6900
Wire Wire Line
	1950 6900 2300 6900
Wire Wire Line
	2300 6900 2300 7000
$Comp
L power:GND #PWR031
U 1 1 5DA72286
P 2300 7400
F 0 "#PWR031" H 2300 7150 50  0001 C CNN
F 1 "GND" H 2305 7227 50  0000 C CNN
F 2 "" H 2300 7400 50  0001 C CNN
F 3 "" H 2300 7400 50  0001 C CNN
	1    2300 7400
	1    0    0    -1  
$EndComp
$Comp
L Device:R R14
U 1 1 5DA72345
P 2750 7150
F 0 "R14" H 2680 7104 50  0000 R CNN
F 1 "10k" H 2680 7195 50  0000 R CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 2680 7150 50  0001 C CNN
F 3 "~" H 2750 7150 50  0001 C CNN
	1    2750 7150
	-1   0    0    1   
$EndComp
Wire Wire Line
	2750 6900 2300 6900
Connection ~ 2300 6900
$Comp
L power:GND #PWR033
U 1 1 5DA772E2
P 2750 7400
F 0 "#PWR033" H 2750 7150 50  0001 C CNN
F 1 "GND" H 2755 7227 50  0000 C CNN
F 2 "" H 2750 7400 50  0001 C CNN
F 3 "" H 2750 7400 50  0001 C CNN
	1    2750 7400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2300 7300 2300 7400
Wire Wire Line
	2750 7000 2750 6900
Wire Wire Line
	2750 7300 2750 7400
Wire Wire Line
	2750 6900 3200 6900
Connection ~ 2750 6900
Text Label 2950 2050 0    50   ~ 0
DISABLE
Text Label 2950 2200 0    50   ~ 0
SDCS
$Comp
L Device:R R1
U 1 1 5DA8B65E
P 2100 8850
F 0 "R1" H 2030 8804 50  0000 R CNN
F 1 "10k" H 2030 8895 50  0000 R CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 2030 8850 50  0001 C CNN
F 3 "~" H 2100 8850 50  0001 C CNN
	1    2100 8850
	-1   0    0    1   
$EndComp
$Comp
L power:+3.3V #PWR08
U 1 1 5DA8B9EC
P 2100 8700
F 0 "#PWR08" H 2100 8550 50  0001 C CNN
F 1 "+3.3V" H 2115 8873 50  0000 C CNN
F 2 "" H 2100 8700 50  0001 C CNN
F 3 "" H 2100 8700 50  0001 C CNN
	1    2100 8700
	1    0    0    -1  
$EndComp
Wire Wire Line
	2100 9800 2500 9800
Wire Wire Line
	2400 9500 2400 8700
$Comp
L power:+3.3V #PWR09
U 1 1 5DA95C25
P 2400 8700
F 0 "#PWR09" H 2400 8550 50  0001 C CNN
F 1 "+3.3V" H 2415 8873 50  0000 C CNN
F 2 "" H 2400 8700 50  0001 C CNN
F 3 "" H 2400 8700 50  0001 C CNN
	1    2400 8700
	1    0    0    -1  
$EndComp
Wire Wire Line
	2100 9000 2100 9800
Wire Wire Line
	2400 9700 2400 10250
Wire Wire Line
	2100 9800 1550 9800
Connection ~ 2100 9800
Wire Wire Line
	2500 9400 1550 9400
Text Notes 850  9400 0    50   ~ 0
MOSI (SER1P0)
Text Notes 850  9800 0    50   ~ 0
MISO (SER1P3)
Text Notes 850  9600 0    50   ~ 0
SCLK (SER1P1)
Wire Wire Line
	2500 9600 1550 9600
Wire Wire Line
	2500 9300 1550 9300
Text Notes 1150 9300 0    50   ~ 0
SS
Wire Wire Line
	1200 6900 800  6900
Text Label 800  6900 0    50   ~ 0
VDAC
$Comp
L uChipProjects:JACKAUDIO35 J5
U 1 1 5DAE0D7B
P 3500 7200
F 0 "J5" H 3879 7371 50  0000 L CNN
F 1 "JACKAUDIO35" H 3879 7280 50  0000 L CNN
F 2 "uChipProjects:SJ-3525NG" H 3450 7500 50  0001 C CNN
F 3 "" H 3450 7500 50  0001 C CNN
	1    3500 7200
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 7300 3200 7300
Wire Wire Line
	3200 7300 3200 7400
$Comp
L power:GND #PWR034
U 1 1 5DAE69FA
P 3200 7400
F 0 "#PWR034" H 3200 7150 50  0001 C CNN
F 1 "GND" H 3205 7227 50  0000 C CNN
F 2 "" H 3200 7400 50  0001 C CNN
F 3 "" H 3200 7400 50  0001 C CNN
	1    3200 7400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 7000 3200 7000
Wire Wire Line
	3200 7000 3200 6900
Connection ~ 3200 6900
Wire Wire Line
	3200 6900 3400 6900
Wire Wire Line
	3200 7000 3200 7100
Wire Wire Line
	3200 7100 3400 7100
Connection ~ 3200 7000
Wire Wire Line
	3400 7200 3200 7200
Wire Wire Line
	3200 7200 3200 7100
Connection ~ 3200 7100
Text Label 1550 9400 0    50   ~ 0
R0
Text Label 1550 9300 0    50   ~ 0
SDCS
Text Label 1550 9600 0    50   ~ 0
R2
Text Label 1550 9800 0    50   ~ 0
B1
$Comp
L Graphic:Logo_Open_Hardware_Large #LOGO1
U 1 1 5DBACF56
P 12100 9150
F 0 "#LOGO1" H 12100 9650 50  0001 C CNN
F 1 "Logo_Open_Hardware_Large" H 12100 8750 50  0001 C CNN
F 2 "uChipProjects:OSHW-Logo2_9.8x8mm_SilkScreen" H 12100 9150 50  0001 C CNN
F 3 "~" H 12100 9150 50  0001 C CNN
	1    12100 9150
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H1
U 1 1 5DBAD4C4
P 14500 5400
F 0 "H1" H 14600 5446 50  0000 L CNN
F 1 "MountingHole" H 14600 5355 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_Pad" H 14500 5400 50  0001 C CNN
F 3 "~" H 14500 5400 50  0001 C CNN
	1    14500 5400
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H2
U 1 1 5DBAD6A5
P 14500 5600
F 0 "H2" H 14600 5646 50  0000 L CNN
F 1 "MountingHole" H 14600 5555 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_Pad" H 14500 5600 50  0001 C CNN
F 3 "~" H 14500 5600 50  0001 C CNN
	1    14500 5600
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H3
U 1 1 5DBAD701
P 14500 5800
F 0 "H3" H 14600 5846 50  0000 L CNN
F 1 "MountingHole" H 14600 5755 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_Pad" H 14500 5800 50  0001 C CNN
F 3 "~" H 14500 5800 50  0001 C CNN
	1    14500 5800
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H4
U 1 1 5DBAD75D
P 14500 6000
F 0 "H4" H 14600 6046 50  0000 L CNN
F 1 "MountingHole" H 14600 5955 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_Pad" H 14500 6000 50  0001 C CNN
F 3 "~" H 14500 6000 50  0001 C CNN
	1    14500 6000
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR023
U 1 1 5DBBF473
P 5550 9050
F 0 "#PWR023" H 5550 8900 50  0001 C CNN
F 1 "+3.3V" H 5565 9223 50  0000 C CNN
F 2 "" H 5550 9050 50  0001 C CNN
F 3 "" H 5550 9050 50  0001 C CNN
	1    5550 9050
	1    0    0    -1  
$EndComp
$Comp
L Device:R R15
U 1 1 5DBBF59F
P 7050 9600
F 0 "R15" H 6980 9554 50  0000 R CNN
F 1 "390" H 6980 9645 50  0000 R CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 6980 9600 50  0001 C CNN
F 3 "~" H 7050 9600 50  0001 C CNN
	1    7050 9600
	-1   0    0    1   
$EndComp
$Comp
L Device:LED D2
U 1 1 5DBBF5A6
P 7050 9250
F 0 "D2" V 7088 9133 50  0000 R CNN
F 1 "RED LED" V 6997 9133 50  0000 R CNN
F 2 "uChipProjects:LED_Rectangular_W5.0mm_H2.0mm" H 7050 9250 50  0001 C CNN
F 3 "~" H 7050 9250 50  0001 C CNN
	1    7050 9250
	0    -1   -1   0   
$EndComp
Wire Wire Line
	7050 9050 7050 9100
Wire Wire Line
	7050 9400 7050 9450
$Comp
L power:+3.3V #PWR035
U 1 1 5DBBF5B6
P 7050 9050
F 0 "#PWR035" H 7050 8900 50  0001 C CNN
F 1 "+3.3V" H 7065 9223 50  0000 C CNN
F 2 "" H 7050 9050 50  0001 C CNN
F 3 "" H 7050 9050 50  0001 C CNN
	1    7050 9050
	1    0    0    -1  
$EndComp
Wire Wire Line
	7050 9850 6400 9850
Wire Wire Line
	7050 9750 7050 9850
Text Label 6400 9850 0    50   ~ 0
SDCS
$Comp
L uChipProjects:Micro_SD_CardDet J2
U 1 1 5DBE1F36
P 3400 9650
F 0 "J2" H 4230 9491 50  0000 L CNN
F 1 "Micro_SD_CardDet" H 4230 9400 50  0000 L CNN
F 2 "uChipProjects:HYC77TF09200" H 4550 9950 50  0001 C CNN
F 3 "http://katalog.we-online.de/em/datasheet/693072010801.pdf" H 3400 9650 50  0001 C CNN
	1    3400 9650
	1    0    0    -1  
$EndComp
Wire Wire Line
	9800 9250 9450 9250
Text Label 9450 9250 0    50   ~ 0
R2
Wire Wire Line
	9800 9350 9450 9350
Text Label 9450 9350 0    50   ~ 0
R1
Wire Wire Line
	9800 9450 9450 9450
Text Label 9450 9450 0    50   ~ 0
R0
Wire Wire Line
	9800 9550 9450 9550
Text Label 9450 9550 0    50   ~ 0
G2
Wire Wire Line
	9800 9650 9450 9650
Text Label 9450 9650 0    50   ~ 0
G1
Wire Wire Line
	9800 9750 9450 9750
Text Label 9450 9750 0    50   ~ 0
G0
Wire Wire Line
	9800 9850 9450 9850
Text Label 9450 9850 0    50   ~ 0
B1
Wire Wire Line
	9800 9950 9450 9950
Text Label 9450 9950 0    50   ~ 0
B0
$Comp
L power:GND #PWR0101
U 1 1 5DBF4A29
P 9600 10400
F 0 "#PWR0101" H 9600 10150 50  0001 C CNN
F 1 "GND" H 9605 10227 50  0000 C CNN
F 2 "" H 9600 10400 50  0001 C CNN
F 3 "" H 9600 10400 50  0001 C CNN
	1    9600 10400
	1    0    0    -1  
$EndComp
Text Label 9450 9050 0    50   ~ 0
DISABLE
Wire Wire Line
	9450 9050 9800 9050
$Comp
L power:+3.3V #PWR0102
U 1 1 5DC08280
P 9250 9050
F 0 "#PWR0102" H 9250 8900 50  0001 C CNN
F 1 "+3.3V" H 9265 9223 50  0000 C CNN
F 2 "" H 9250 9050 50  0001 C CNN
F 3 "" H 9250 9050 50  0001 C CNN
	1    9250 9050
	1    0    0    -1  
$EndComp
Wire Wire Line
	9250 9050 9250 9150
Wire Wire Line
	9250 9150 9800 9150
$Comp
L power:+5V #PWR0103
U 1 1 5DC12186
P 9200 10000
F 0 "#PWR0103" H 9200 9850 50  0001 C CNN
F 1 "+5V" H 9215 10173 50  0000 C CNN
F 2 "" H 9200 10000 50  0001 C CNN
F 3 "" H 9200 10000 50  0001 C CNN
	1    9200 10000
	1    0    0    -1  
$EndComp
Wire Wire Line
	9200 10000 9200 10150
Wire Wire Line
	9200 10150 9800 10150
Wire Wire Line
	9800 10050 9600 10050
Wire Wire Line
	9600 10050 9600 10400
$Comp
L Connector_Generic:Conn_01x12 J3
U 1 1 5DC3B09B
P 10000 9550
F 0 "J3" H 10080 9542 50  0000 L CNN
F 1 "Conn_01x12" H 10080 9451 50  0000 L CNN
F 2 "uChipProjects:PinHeader_1x12_P2.54mm_Vertical" H 10000 9550 50  0001 C CNN
F 3 "~" H 10000 9550 50  0001 C CNN
	1    10000 9550
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 5D9BBF74
P 3400 10700
F 0 "#PWR0104" H 3400 10450 50  0001 C CNN
F 1 "GND" H 3405 10527 50  0000 C CNN
F 2 "" H 3400 10700 50  0001 C CNN
F 3 "" H 3400 10700 50  0001 C CNN
	1    3400 10700
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 10600 3400 10650
Text Notes 2600 8400 0    100  ~ 0
SD CARD READER
Text Notes 5600 8400 0    100  ~ 0
INDICATION LEDs
Text Notes 10300 4100 0    100  ~ 0
DAC AND VGA PORT
Text Notes 9000 8400 0    100  ~ 0
EXPANSION PORT
Text Notes 6800 4150 0    100  ~ 0
BUFFER
Text Notes 1200 4150 0    100  ~ 0
DC IN AND SD+OSCILLATOR 3.3V REGULATOR
Text Notes 7300 1300 0    100  ~ 0
uCHIP AND OSCILLATOR
Text Notes 6100 950  0    200  ~ 0
uCHIP SIMPLE VGA CONSOLE V2.1
Text Notes 14000 4950 0    100  ~ 0
MOUNTING HOLES
Text Notes 1750 6300 0    100  ~ 0
AUDIO FILTER AND OUTPUT
Text Notes 2250 6650 0    50   ~ 0
Note: this is not an amplified output!\nConnecting directly an earpiecce will \ngive a very low sound output!
Text Notes 5850 7100 0    50   ~ 0
The buffer is only required for the SD card, \nand the SPI video mode. However, If you do not require it,\nput a 560-Ohm resistor between G0 and the 3.3V, to prevent\nSWDCK being sensed as "low" during reset. If such pin is held\nlow, uChip will not boot up. \n\nDo NOT use a 74HC245 instead of a 74AHC245!
Wire Wire Line
	3300 10600 3300 10650
Wire Wire Line
	3300 10650 3400 10650
Connection ~ 3400 10650
Wire Wire Line
	3400 10650 3400 10700
Wire Wire Line
	3500 10600 3500 10650
Wire Wire Line
	3500 10650 3400 10650
Wire Wire Line
	3500 10650 3600 10650
Wire Wire Line
	3600 10650 3600 10600
Connection ~ 3500 10650
$EndSCHEMATC
