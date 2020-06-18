EESchema Schematic File Version 4
LIBS:uChipVGA2V1-cache
EELAYER 26 0
EELAYER END
$Descr A3 16535 11693
encoding utf-8
Sheet 1 1
Title "uChip Simple VGA Console (uSVC)"
Date ""
Rev "2.1"
Comp "next-hack.com"
Comment1 "Author: N.W."
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
P 12300 5700
F 0 "J4" H 12300 6567 50  0000 C CNN
F 1 "DB15_Female_HighDensity_MountingHoles" H 12300 6476 50  0000 C CNN
F 2 "uChipProjects:DSUB-15-HD_Female_Horizontal_P2.29x2.54mm_EdgePinOffset8.55mm_Housed_MountingHolesOffset11.00mm-Pads1.8mm" H 11350 6100 50  0001 C CNN
F 3 " ~" H 11350 6100 50  0001 C CNN
	1    12300 5700
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
P 7050 5000
F 0 "U2" H 7150 5800 50  0000 C CNN
F 1 "74AHC245" H 7300 5700 50  0000 C CNN
F 2 "uChipProjects:DIP-20_W7.62mm_Socket_LongPads" H 7050 5000 50  0001 C CNN
F 3 "http://www.ti.com/lit/gpn/sn74HC245" H 7050 5000 50  0001 C CNN
	1    7050 5000
	1    0    0    -1  
$EndComp
$Comp
L Connector:USB_B J1
U 1 1 5D9872F6
P 1150 4700
F 0 "J1" H 1205 5167 50  0000 C CNN
F 1 "USB_B" H 1205 5076 50  0000 C CNN
F 2 "uChipProjects:USB_B_OST_USB-B1HSxx_Horizontal" H 1300 4650 50  0001 C CNN
F 3 " ~" H 1300 4650 50  0001 C CNN
	1    1150 4700
	1    0    0    -1  
$EndComp
Wire Wire Line
	12600 5700 13150 5700
Wire Wire Line
	13150 5700 13150 6450
Wire Wire Line
	12600 5900 12850 5900
Wire Wire Line
	12850 5900 12850 6450
$Comp
L Device:R R11
U 1 1 5D987774
P 12850 6600
F 0 "R11" H 12920 6646 50  0000 L CNN
F 1 "22" H 12920 6555 50  0000 L CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 12780 6600 50  0001 C CNN
F 3 "~" H 12850 6600 50  0001 C CNN
	1    12850 6600
	1    0    0    -1  
$EndComp
$Comp
L Device:R R13
U 1 1 5D987822
P 13150 6600
F 0 "R13" H 13220 6646 50  0000 L CNN
F 1 "22" H 13220 6555 50  0000 L CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 13080 6600 50  0001 C CNN
F 3 "~" H 13150 6600 50  0001 C CNN
	1    13150 6600
	1    0    0    -1  
$EndComp
Wire Wire Line
	12850 6750 12850 7200
Wire Wire Line
	12850 7200 12100 7200
Text Label 12100 7200 0    50   ~ 0
VSYNC
Wire Wire Line
	13150 6750 13150 7350
Wire Wire Line
	13150 7350 12100 7350
Text Label 12100 7350 0    50   ~ 0
HSYNC
$Comp
L power:GND #PWR030
U 1 1 5D987942
P 12300 6550
F 0 "#PWR030" H 12300 6300 50  0001 C CNN
F 1 "GND" H 12305 6377 50  0000 C CNN
F 2 "" H 12300 6550 50  0001 C CNN
F 3 "" H 12300 6550 50  0001 C CNN
	1    12300 6550
	1    0    0    -1  
$EndComp
$Comp
L Device:R R9
U 1 1 5D9879A4
P 9350 6400
F 0 "R9" V 9143 6400 50  0000 C CNN
F 1 "390" V 9234 6400 50  0000 C CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 9280 6400 50  0001 C CNN
F 3 "~" H 9350 6400 50  0001 C CNN
	1    9350 6400
	0    1    1    0   
$EndComp
$Comp
L Device:R R10
U 1 1 5D987AD7
P 9350 6750
F 0 "R10" V 9143 6750 50  0000 C CNN
F 1 "820" V 9234 6750 50  0000 C CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 9280 6750 50  0001 C CNN
F 3 "~" H 9350 6750 50  0001 C CNN
	1    9350 6750
	0    1    1    0   
$EndComp
Text Label 8800 6400 0    50   ~ 0
bB1
Wire Wire Line
	9200 6750 8800 6750
Text Label 8800 6750 0    50   ~ 0
bB0
Wire Wire Line
	10150 6750 9500 6750
Wire Wire Line
	9500 6400 10150 6400
Wire Wire Line
	12000 6000 11700 6000
Wire Wire Line
	11700 6000 11700 6100
Wire Wire Line
	12000 5600 11700 5600
Wire Wire Line
	11700 5600 11700 6000
Connection ~ 11700 6000
Wire Wire Line
	12000 5400 11700 5400
Wire Wire Line
	11700 5400 11700 5600
Connection ~ 11700 5600
Wire Wire Line
	12000 5200 11700 5200
Wire Wire Line
	11700 5200 11700 5400
Connection ~ 11700 5400
Wire Wire Line
	12000 6100 11700 6100
Connection ~ 11700 6100
Wire Wire Line
	11700 6100 11700 6550
Wire Wire Line
	12300 6400 12300 6550
$Comp
L power:GND #PWR029
U 1 1 5D98941F
P 11700 6550
F 0 "#PWR029" H 11700 6300 50  0001 C CNN
F 1 "GND" H 11705 6377 50  0000 C CNN
F 2 "" H 11700 6550 50  0001 C CNN
F 3 "" H 11700 6550 50  0001 C CNN
	1    11700 6550
	1    0    0    -1  
$EndComp
Wire Wire Line
	10150 6400 10150 6750
$Comp
L Device:R R8
U 1 1 5D98979D
P 9350 5850
F 0 "R8" V 9143 5850 50  0000 C CNN
F 1 "2k2" V 9234 5850 50  0000 C CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 9280 5850 50  0001 C CNN
F 3 "~" H 9350 5850 50  0001 C CNN
	1    9350 5850
	0    1    1    0   
$EndComp
$Comp
L Device:R R7
U 1 1 5D98980B
P 9350 5500
F 0 "R7" V 9143 5500 50  0000 C CNN
F 1 "1k" V 9234 5500 50  0000 C CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 9280 5500 50  0001 C CNN
F 3 "~" H 9350 5500 50  0001 C CNN
	1    9350 5500
	0    1    1    0   
$EndComp
$Comp
L Device:R R6
U 1 1 5D98983B
P 9350 5150
F 0 "R6" V 9143 5150 50  0000 C CNN
F 1 "470" V 9234 5150 50  0000 C CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 9280 5150 50  0001 C CNN
F 3 "~" H 9350 5150 50  0001 C CNN
	1    9350 5150
	0    1    1    0   
$EndComp
Wire Wire Line
	10150 5500 10150 5150
Wire Wire Line
	10150 5150 9500 5150
Wire Wire Line
	9500 5500 10150 5500
Wire Wire Line
	9500 5850 10150 5850
Wire Wire Line
	9200 6400 8800 6400
Wire Wire Line
	9200 5850 8800 5850
Wire Wire Line
	9200 5500 8800 5500
Wire Wire Line
	9200 5150 8800 5150
Text Label 8800 5150 0    50   ~ 0
bG2
Text Label 8800 5500 0    50   ~ 0
bG1
Text Label 8800 5850 0    50   ~ 0
bG0
Connection ~ 10150 5500
Wire Wire Line
	12000 5500 10150 5500
Wire Wire Line
	10150 5500 10150 5850
Wire Wire Line
	10150 6400 11050 6400
Wire Wire Line
	11050 6400 11050 5700
Wire Wire Line
	11050 5700 12000 5700
Connection ~ 10150 6400
$Comp
L Device:R R5
U 1 1 5D98DF49
P 9350 4600
F 0 "R5" V 9143 4600 50  0000 C CNN
F 1 "2k2" V 9234 4600 50  0000 C CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 9280 4600 50  0001 C CNN
F 3 "~" H 9350 4600 50  0001 C CNN
	1    9350 4600
	0    1    1    0   
$EndComp
$Comp
L Device:R R4
U 1 1 5D98DF50
P 9350 4250
F 0 "R4" V 9143 4250 50  0000 C CNN
F 1 "1k" V 9234 4250 50  0000 C CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 9280 4250 50  0001 C CNN
F 3 "~" H 9350 4250 50  0001 C CNN
	1    9350 4250
	0    1    1    0   
$EndComp
$Comp
L Device:R R3
U 1 1 5D98DF57
P 9350 3900
F 0 "R3" V 9143 3900 50  0000 C CNN
F 1 "470" V 9234 3900 50  0000 C CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 9280 3900 50  0001 C CNN
F 3 "~" H 9350 3900 50  0001 C CNN
	1    9350 3900
	0    1    1    0   
$EndComp
Wire Wire Line
	10150 4250 10150 3900
Wire Wire Line
	10150 3900 9500 3900
Wire Wire Line
	9500 4250 10150 4250
Wire Wire Line
	9500 4600 10150 4600
Wire Wire Line
	9200 4600 8800 4600
Wire Wire Line
	9200 4250 8800 4250
Wire Wire Line
	9200 3900 8800 3900
Text Label 8800 3900 0    50   ~ 0
bR2
Text Label 8800 4250 0    50   ~ 0
bR1
Text Label 8800 4600 0    50   ~ 0
bR0
Connection ~ 10150 4250
Wire Wire Line
	10150 4250 10150 4600
Wire Wire Line
	11050 5300 12000 5300
Text Label 11250 5300 0    50   ~ 0
R
Text Label 11250 5500 0    50   ~ 0
G
Text Label 11250 5700 0    50   ~ 0
B
$Comp
L power:+3.3V #PWR025
U 1 1 5D991124
P 7050 4200
F 0 "#PWR025" H 7050 4050 50  0001 C CNN
F 1 "+3.3V" H 7065 4373 50  0000 C CNN
F 2 "" H 7050 4200 50  0001 C CNN
F 3 "" H 7050 4200 50  0001 C CNN
	1    7050 4200
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR022
U 1 1 5D9912E0
P 5950 5400
F 0 "#PWR022" H 5950 5250 50  0001 C CNN
F 1 "+3.3V" H 5965 5573 50  0000 C CNN
F 2 "" H 5950 5400 50  0001 C CNN
F 3 "" H 5950 5400 50  0001 C CNN
	1    5950 5400
	1    0    0    -1  
$EndComp
Wire Wire Line
	5950 5400 6550 5400
Wire Wire Line
	6550 4500 6200 4500
Text Label 6200 4500 0    50   ~ 0
R2
Wire Wire Line
	6550 4600 6200 4600
Text Label 6200 4600 0    50   ~ 0
R1
Wire Wire Line
	6550 4700 6200 4700
Text Label 6200 4700 0    50   ~ 0
R0
Wire Wire Line
	6550 4800 6200 4800
Text Label 6200 4800 0    50   ~ 0
G2
Wire Wire Line
	6550 4900 6200 4900
Text Label 6200 4900 0    50   ~ 0
G1
Wire Wire Line
	6550 5000 6200 5000
Text Label 6200 5000 0    50   ~ 0
G0
Wire Wire Line
	6550 5100 6200 5100
Text Label 6200 5100 0    50   ~ 0
B1
Wire Wire Line
	6550 5200 6200 5200
Text Label 6200 5200 0    50   ~ 0
B0
Text Label 7900 4500 2    50   ~ 0
bR2
Text Label 7900 4600 2    50   ~ 0
bR1
Text Label 7900 4700 2    50   ~ 0
bR0
Text Label 7900 4800 2    50   ~ 0
bG2
Text Label 7900 4900 2    50   ~ 0
bG1
Text Label 7900 5000 2    50   ~ 0
bG0
Text Label 7900 5100 2    50   ~ 0
bB1
Text Label 7900 5200 2    50   ~ 0
bB0
Wire Wire Line
	7900 4500 7550 4500
Wire Wire Line
	7900 4600 7550 4600
Wire Wire Line
	7900 4700 7550 4700
Wire Wire Line
	7900 4800 7550 4800
Wire Wire Line
	7900 4900 7550 4900
Wire Wire Line
	7900 5000 7550 5000
Wire Wire Line
	7900 5100 7550 5100
Wire Wire Line
	7900 5200 7550 5200
Wire Wire Line
	6550 5500 6200 5500
Text Label 6200 5500 0    50   ~ 0
DISABLE
$Comp
L power:GND #PWR026
U 1 1 5D9AB57B
P 7050 5850
F 0 "#PWR026" H 7050 5600 50  0001 C CNN
F 1 "GND" H 7055 5677 50  0000 C CNN
F 2 "" H 7050 5850 50  0001 C CNN
F 3 "" H 7050 5850 50  0001 C CNN
	1    7050 5850
	1    0    0    -1  
$EndComp
Wire Wire Line
	7050 5800 7050 5850
$Comp
L power:GND #PWR03
U 1 1 5D9AD1B4
P 1100 5200
F 0 "#PWR03" H 1100 4950 50  0001 C CNN
F 1 "GND" H 1105 5027 50  0000 C CNN
F 2 "" H 1100 5200 50  0001 C CNN
F 3 "" H 1100 5200 50  0001 C CNN
	1    1100 5200
	1    0    0    -1  
$EndComp
Wire Wire Line
	1050 5100 1050 5150
Wire Wire Line
	1050 5150 1100 5150
Wire Wire Line
	1150 5150 1150 5100
Wire Wire Line
	1100 5150 1100 5200
Connection ~ 1100 5150
Wire Wire Line
	1100 5150 1150 5150
Wire Wire Line
	1450 4500 1950 4500
Wire Wire Line
	1950 4500 1950 4400
$Comp
L power:+5V #PWR06
U 1 1 5D9B2C03
P 1950 4400
F 0 "#PWR06" H 1950 4250 50  0001 C CNN
F 1 "+5V" H 1965 4573 50  0000 C CNN
F 2 "" H 1950 4400 50  0001 C CNN
F 3 "" H 1950 4400 50  0001 C CNN
	1    1950 4400
	1    0    0    -1  
$EndComp
Wire Wire Line
	1950 4500 1950 4600
Connection ~ 1950 4500
$Comp
L Device:CP C2
U 1 1 5D9B4E0E
P 1950 4750
F 0 "C2" H 2068 4796 50  0000 L CNN
F 1 "100u" H 2068 4705 50  0000 L CNN
F 2 "uChipProjects:CP_Radial_D6.3mm_P2.50mm" H 1988 4600 50  0001 C CNN
F 3 "~" H 1950 4750 50  0001 C CNN
	1    1950 4750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR07
U 1 1 5D9B4EB5
P 1950 5000
F 0 "#PWR07" H 1950 4750 50  0001 C CNN
F 1 "GND" H 1955 4827 50  0000 C CNN
F 2 "" H 1950 5000 50  0001 C CNN
F 3 "" H 1950 5000 50  0001 C CNN
	1    1950 5000
	1    0    0    -1  
$EndComp
Wire Wire Line
	1950 4900 1950 5000
Wire Wire Line
	10150 4600 11050 4600
Connection ~ 10150 4600
Wire Wire Line
	11050 4600 11050 5300
$Comp
L uChipProjects:MCP1700-3302E-TO U1
U 1 1 5D9CC830
P 3000 4550
F 0 "U1" H 3000 4842 50  0000 C CNN
F 1 "MCP1700-3302E-TO" H 3000 4751 50  0000 C CNN
F 2 "uChipProjects:TO-92L_HandSolder" H 3950 4350 50  0001 C CNN
F 3 "" H 3000 4700 50  0001 C CNN
	1    3000 4550
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR012
U 1 1 5D9CC8C4
P 3000 4850
F 0 "#PWR012" H 3000 4600 50  0001 C CNN
F 1 "GND" H 3005 4677 50  0000 C CNN
F 2 "" H 3000 4850 50  0001 C CNN
F 3 "" H 3000 4850 50  0001 C CNN
	1    3000 4850
	1    0    0    -1  
$EndComp
Wire Wire Line
	3000 4800 3000 4850
Wire Wire Line
	2700 4500 2500 4500
Wire Wire Line
	3300 4500 3550 4500
$Comp
L Device:C C4
U 1 1 5D9D2FE0
P 3550 4750
F 0 "C4" H 3665 4796 50  0000 L CNN
F 1 "1u" H 3665 4705 50  0000 L CNN
F 2 "uChipProjects:C_Disc_D5.0mm_W2.5mm_P5.00mm" H 3588 4600 50  0001 C CNN
F 3 "~" H 3550 4750 50  0001 C CNN
	1    3550 4750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR011
U 1 1 5D9D30B1
P 2500 5000
F 0 "#PWR011" H 2500 4750 50  0001 C CNN
F 1 "GND" H 2505 4827 50  0000 C CNN
F 2 "" H 2500 5000 50  0001 C CNN
F 3 "" H 2500 5000 50  0001 C CNN
	1    2500 5000
	1    0    0    -1  
$EndComp
Wire Wire Line
	2500 4900 2500 5000
Wire Wire Line
	2500 4600 2500 4500
Connection ~ 2500 4500
Wire Wire Line
	2500 4500 1950 4500
$Comp
L Device:C C3
U 1 1 5D9D9EE2
P 2500 4750
F 0 "C3" H 2615 4796 50  0000 L CNN
F 1 "1u" H 2615 4705 50  0000 L CNN
F 2 "uChipProjects:C_Disc_D5.0mm_W2.5mm_P5.00mm" H 2538 4600 50  0001 C CNN
F 3 "~" H 2500 4750 50  0001 C CNN
	1    2500 4750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR015
U 1 1 5D9DC4CA
P 3550 5000
F 0 "#PWR015" H 3550 4750 50  0001 C CNN
F 1 "GND" H 3555 4827 50  0000 C CNN
F 2 "" H 3550 5000 50  0001 C CNN
F 3 "" H 3550 5000 50  0001 C CNN
	1    3550 5000
	1    0    0    -1  
$EndComp
Wire Wire Line
	3550 4900 3550 5000
Wire Wire Line
	3550 4600 3550 4500
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
P 7800 5700
F 0 "C7" H 7915 5746 50  0000 L CNN
F 1 "1u" H 7915 5655 50  0000 L CNN
F 2 "uChipProjects:C_Disc_D5.0mm_W2.5mm_P5.00mm" H 7838 5550 50  0001 C CNN
F 3 "~" H 7800 5700 50  0001 C CNN
	1    7800 5700
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR027
U 1 1 5D9E3A48
P 7800 5550
F 0 "#PWR027" H 7800 5400 50  0001 C CNN
F 1 "+3.3V" H 7815 5723 50  0000 C CNN
F 2 "" H 7800 5550 50  0001 C CNN
F 3 "" H 7800 5550 50  0001 C CNN
	1    7800 5550
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR028
U 1 1 5D9E3AAB
P 7800 5850
F 0 "#PWR028" H 7800 5600 50  0001 C CNN
F 1 "GND" H 7805 5677 50  0000 C CNN
F 2 "" H 7800 5850 50  0001 C CNN
F 3 "" H 7800 5850 50  0001 C CNN
	1    7800 5850
	1    0    0    -1  
$EndComp
$Comp
L Device:C C6
U 1 1 5D9E8D89
P 4350 8750
F 0 "C6" H 4465 8796 50  0000 L CNN
F 1 "1u" H 4465 8705 50  0000 L CNN
F 2 "uChipProjects:C_Disc_D5.0mm_W2.5mm_P5.00mm" H 4388 8600 50  0001 C CNN
F 3 "~" H 4350 8750 50  0001 C CNN
	1    4350 8750
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR018
U 1 1 5D9E8D90
P 4350 8600
F 0 "#PWR018" H 4350 8450 50  0001 C CNN
F 1 "+3.3V" H 4365 8773 50  0000 C CNN
F 2 "" H 4350 8600 50  0001 C CNN
F 3 "" H 4350 8600 50  0001 C CNN
	1    4350 8600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR019
U 1 1 5D9E8D96
P 4350 8900
F 0 "#PWR019" H 4350 8650 50  0001 C CNN
F 1 "GND" H 4355 8727 50  0000 C CNN
F 2 "" H 4350 8900 50  0001 C CNN
F 3 "" H 4350 8900 50  0001 C CNN
	1    4350 8900
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
P 3550 4400
F 0 "#PWR014" H 3550 4250 50  0001 C CNN
F 1 "+3.3V" H 3565 4573 50  0000 C CNN
F 2 "" H 3550 4400 50  0001 C CNN
F 3 "" H 3550 4400 50  0001 C CNN
	1    3550 4400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3550 4400 3550 4500
Connection ~ 3550 4500
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
P 5450 8950
F 0 "R2" H 5380 8904 50  0000 R CNN
F 1 "390" H 5380 8995 50  0000 R CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 5380 8950 50  0001 C CNN
F 3 "~" H 5450 8950 50  0001 C CNN
	1    5450 8950
	-1   0    0    1   
$EndComp
$Comp
L Device:LED D1
U 1 1 5D9FCEF0
P 5450 8600
F 0 "D1" V 5488 8483 50  0000 R CNN
F 1 "YELOW LED" V 5397 8483 50  0000 R CNN
F 2 "uChipProjects:LED_Rectangular_W5.0mm_H2.0mm" H 5450 8600 50  0001 C CNN
F 3 "~" H 5450 8600 50  0001 C CNN
	1    5450 8600
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5450 8400 5450 8450
Wire Wire Line
	5450 8750 5450 8800
Wire Wire Line
	5450 9100 5450 9150
$Comp
L power:GND #PWR024
U 1 1 5DA05ADC
P 5450 9150
F 0 "#PWR024" H 5450 8900 50  0001 C CNN
F 1 "GND" H 5455 8977 50  0000 C CNN
F 2 "" H 5450 9150 50  0001 C CNN
F 3 "" H 5450 9150 50  0001 C CNN
	1    5450 9150
	1    0    0    -1  
$EndComp
Wire Wire Line
	2400 8850 2300 8850
Wire Wire Line
	2400 9050 2300 9050
$Comp
L power:GND #PWR010
U 1 1 5DA0C28C
P 2300 9600
F 0 "#PWR010" H 2300 9350 50  0001 C CNN
F 1 "GND" H 2305 9427 50  0000 C CNN
F 2 "" H 2300 9600 50  0001 C CNN
F 3 "" H 2300 9600 50  0001 C CNN
	1    2300 9600
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
P 4050 4750
F 0 "C5" H 4168 4796 50  0000 L CNN
F 1 "100u" H 4168 4705 50  0000 L CNN
F 2 "uChipProjects:CP_Radial_D6.3mm_P2.50mm" H 4088 4600 50  0001 C CNN
F 3 "~" H 4050 4750 50  0001 C CNN
	1    4050 4750
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR016
U 1 1 5DA5F27D
P 4050 5000
F 0 "#PWR016" H 4050 4750 50  0001 C CNN
F 1 "GND" H 4055 4827 50  0000 C CNN
F 2 "" H 4050 5000 50  0001 C CNN
F 3 "" H 4050 5000 50  0001 C CNN
	1    4050 5000
	1    0    0    -1  
$EndComp
Wire Wire Line
	4050 4900 4050 5000
Wire Wire Line
	4050 4600 4050 4500
Wire Wire Line
	4050 4500 3550 4500
$Comp
L Device:C C8
U 1 1 5DA68681
P 1300 6500
F 0 "C8" V 1048 6500 50  0000 C CNN
F 1 "1u" V 1139 6500 50  0000 C CNN
F 2 "uChipProjects:C_Disc_D5.0mm_W2.5mm_P5.00mm" H 1338 6350 50  0001 C CNN
F 3 "~" H 1300 6500 50  0001 C CNN
	1    1300 6500
	0    1    1    0   
$EndComp
$Comp
L Device:C C9
U 1 1 5DA68960
P 2250 6750
F 0 "C9" H 2135 6704 50  0000 R CNN
F 1 "22n" H 2135 6795 50  0000 R CNN
F 2 "uChipProjects:C_Disc_D5.0mm_W2.5mm_P5.00mm" H 2288 6600 50  0001 C CNN
F 3 "~" H 2250 6750 50  0001 C CNN
	1    2250 6750
	-1   0    0    1   
$EndComp
$Comp
L Device:R R12
U 1 1 5DA68D4A
P 1750 6500
F 0 "R12" V 1543 6500 50  0000 C CNN
F 1 "1k" V 1634 6500 50  0000 C CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 1680 6500 50  0001 C CNN
F 3 "~" H 1750 6500 50  0001 C CNN
	1    1750 6500
	0    1    1    0   
$EndComp
Wire Wire Line
	1600 6500 1450 6500
Wire Wire Line
	1900 6500 2250 6500
Wire Wire Line
	2250 6500 2250 6600
$Comp
L power:GND #PWR031
U 1 1 5DA72286
P 2250 7000
F 0 "#PWR031" H 2250 6750 50  0001 C CNN
F 1 "GND" H 2255 6827 50  0000 C CNN
F 2 "" H 2250 7000 50  0001 C CNN
F 3 "" H 2250 7000 50  0001 C CNN
	1    2250 7000
	1    0    0    -1  
$EndComp
$Comp
L Device:R R14
U 1 1 5DA72345
P 2700 6750
F 0 "R14" H 2630 6704 50  0000 R CNN
F 1 "10k" H 2630 6795 50  0000 R CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 2630 6750 50  0001 C CNN
F 3 "~" H 2700 6750 50  0001 C CNN
	1    2700 6750
	-1   0    0    1   
$EndComp
Wire Wire Line
	2700 6500 2250 6500
Connection ~ 2250 6500
$Comp
L power:GND #PWR033
U 1 1 5DA772E2
P 2700 7000
F 0 "#PWR033" H 2700 6750 50  0001 C CNN
F 1 "GND" H 2705 6827 50  0000 C CNN
F 2 "" H 2700 7000 50  0001 C CNN
F 3 "" H 2700 7000 50  0001 C CNN
	1    2700 7000
	1    0    0    -1  
$EndComp
Wire Wire Line
	2250 6900 2250 7000
Wire Wire Line
	2700 6600 2700 6500
Wire Wire Line
	2700 6900 2700 7000
Wire Wire Line
	2700 6500 3150 6500
Connection ~ 2700 6500
Text Label 2950 2050 0    50   ~ 0
DISABLE
Text Label 2950 2200 0    50   ~ 0
SDCS
$Comp
L Device:R R1
U 1 1 5DA8B65E
P 2000 8200
F 0 "R1" H 1930 8154 50  0000 R CNN
F 1 "10k" H 1930 8245 50  0000 R CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 1930 8200 50  0001 C CNN
F 3 "~" H 2000 8200 50  0001 C CNN
	1    2000 8200
	-1   0    0    1   
$EndComp
$Comp
L power:+3.3V #PWR08
U 1 1 5DA8B9EC
P 2000 8050
F 0 "#PWR08" H 2000 7900 50  0001 C CNN
F 1 "+3.3V" H 2015 8223 50  0000 C CNN
F 2 "" H 2000 8050 50  0001 C CNN
F 3 "" H 2000 8050 50  0001 C CNN
	1    2000 8050
	1    0    0    -1  
$EndComp
Wire Wire Line
	2000 9150 2400 9150
Wire Wire Line
	2300 8850 2300 8050
$Comp
L power:+3.3V #PWR09
U 1 1 5DA95C25
P 2300 8050
F 0 "#PWR09" H 2300 7900 50  0001 C CNN
F 1 "+3.3V" H 2315 8223 50  0000 C CNN
F 2 "" H 2300 8050 50  0001 C CNN
F 3 "" H 2300 8050 50  0001 C CNN
	1    2300 8050
	1    0    0    -1  
$EndComp
Wire Wire Line
	2000 8350 2000 9150
Wire Wire Line
	2300 9050 2300 9600
Wire Wire Line
	2000 9150 1450 9150
Connection ~ 2000 9150
Wire Wire Line
	2400 8750 1450 8750
Text Notes 750  8750 0    50   ~ 0
MOSI (SER1P0)
Text Notes 750  9150 0    50   ~ 0
MISO (SER1P3)
Text Notes 750  8950 0    50   ~ 0
SCLK (SER1P1)
Wire Wire Line
	2400 8950 1450 8950
Wire Wire Line
	2400 8650 1450 8650
Text Notes 1050 8650 0    50   ~ 0
SS
Wire Wire Line
	1150 6500 750  6500
Text Label 750  6500 0    50   ~ 0
VDAC
$Comp
L uChipProjects:JACKAUDIO35 J5
U 1 1 5DAE0D7B
P 3450 6800
F 0 "J5" H 3829 6971 50  0000 L CNN
F 1 "JACKAUDIO35" H 3829 6880 50  0000 L CNN
F 2 "uChipProjects:SJ-3525NG" H 3400 7100 50  0001 C CNN
F 3 "" H 3400 7100 50  0001 C CNN
	1    3450 6800
	1    0    0    -1  
$EndComp
Wire Wire Line
	3350 6900 3150 6900
Wire Wire Line
	3150 6900 3150 7000
$Comp
L power:GND #PWR034
U 1 1 5DAE69FA
P 3150 7000
F 0 "#PWR034" H 3150 6750 50  0001 C CNN
F 1 "GND" H 3155 6827 50  0000 C CNN
F 2 "" H 3150 7000 50  0001 C CNN
F 3 "" H 3150 7000 50  0001 C CNN
	1    3150 7000
	1    0    0    -1  
$EndComp
Wire Wire Line
	3350 6600 3150 6600
Wire Wire Line
	3150 6600 3150 6500
Connection ~ 3150 6500
Wire Wire Line
	3150 6500 3350 6500
Wire Wire Line
	3150 6600 3150 6700
Wire Wire Line
	3150 6700 3350 6700
Connection ~ 3150 6600
Wire Wire Line
	3350 6800 3150 6800
Wire Wire Line
	3150 6800 3150 6700
Connection ~ 3150 6700
Text Label 1450 8750 0    50   ~ 0
R0
Text Label 1450 8650 0    50   ~ 0
SDCS
Text Label 1450 8950 0    50   ~ 0
R2
Text Label 1450 9150 0    50   ~ 0
B1
$Comp
L Graphic:Logo_Open_Hardware_Large #LOGO1
U 1 1 5DBACF56
P 12050 8750
F 0 "#LOGO1" H 12050 9250 50  0001 C CNN
F 1 "Logo_Open_Hardware_Large" H 12050 8350 50  0001 C CNN
F 2 "uChipProjects:OSHW-Logo2_9.8x8mm_SilkScreen" H 12050 8750 50  0001 C CNN
F 3 "~" H 12050 8750 50  0001 C CNN
	1    12050 8750
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H1
U 1 1 5DBAD4C4
P 14450 5000
F 0 "H1" H 14550 5046 50  0000 L CNN
F 1 "MountingHole" H 14550 4955 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_Pad" H 14450 5000 50  0001 C CNN
F 3 "~" H 14450 5000 50  0001 C CNN
	1    14450 5000
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H2
U 1 1 5DBAD6A5
P 14450 5200
F 0 "H2" H 14550 5246 50  0000 L CNN
F 1 "MountingHole" H 14550 5155 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_Pad" H 14450 5200 50  0001 C CNN
F 3 "~" H 14450 5200 50  0001 C CNN
	1    14450 5200
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H3
U 1 1 5DBAD701
P 14450 5400
F 0 "H3" H 14550 5446 50  0000 L CNN
F 1 "MountingHole" H 14550 5355 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_Pad" H 14450 5400 50  0001 C CNN
F 3 "~" H 14450 5400 50  0001 C CNN
	1    14450 5400
	1    0    0    -1  
$EndComp
$Comp
L Mechanical:MountingHole H4
U 1 1 5DBAD75D
P 14450 5600
F 0 "H4" H 14550 5646 50  0000 L CNN
F 1 "MountingHole" H 14550 5555 50  0000 L CNN
F 2 "MountingHole:MountingHole_3.2mm_M3_Pad" H 14450 5600 50  0001 C CNN
F 3 "~" H 14450 5600 50  0001 C CNN
	1    14450 5600
	1    0    0    -1  
$EndComp
$Comp
L power:+3.3V #PWR023
U 1 1 5DBBF473
P 5450 8400
F 0 "#PWR023" H 5450 8250 50  0001 C CNN
F 1 "+3.3V" H 5465 8573 50  0000 C CNN
F 2 "" H 5450 8400 50  0001 C CNN
F 3 "" H 5450 8400 50  0001 C CNN
	1    5450 8400
	1    0    0    -1  
$EndComp
$Comp
L Device:R R15
U 1 1 5DBBF59F
P 6950 8950
F 0 "R15" H 6880 8904 50  0000 R CNN
F 1 "390" H 6880 8995 50  0000 R CNN
F 2 "uChipProjects:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal" V 6880 8950 50  0001 C CNN
F 3 "~" H 6950 8950 50  0001 C CNN
	1    6950 8950
	-1   0    0    1   
$EndComp
$Comp
L Device:LED D2
U 1 1 5DBBF5A6
P 6950 8600
F 0 "D2" V 6988 8483 50  0000 R CNN
F 1 "RED LED" V 6897 8483 50  0000 R CNN
F 2 "uChipProjects:LED_Rectangular_W5.0mm_H2.0mm" H 6950 8600 50  0001 C CNN
F 3 "~" H 6950 8600 50  0001 C CNN
	1    6950 8600
	0    -1   -1   0   
$EndComp
Wire Wire Line
	6950 8400 6950 8450
Wire Wire Line
	6950 8750 6950 8800
$Comp
L power:+3.3V #PWR035
U 1 1 5DBBF5B6
P 6950 8400
F 0 "#PWR035" H 6950 8250 50  0001 C CNN
F 1 "+3.3V" H 6965 8573 50  0000 C CNN
F 2 "" H 6950 8400 50  0001 C CNN
F 3 "" H 6950 8400 50  0001 C CNN
	1    6950 8400
	1    0    0    -1  
$EndComp
Wire Wire Line
	6950 9200 6300 9200
Wire Wire Line
	6950 9100 6950 9200
Text Label 6300 9200 0    50   ~ 0
SDCS
$Comp
L uChipProjects:Micro_SD_CardDet J2
U 1 1 5DBE1F36
P 3300 9000
F 0 "J2" H 4130 8841 50  0000 L CNN
F 1 "Micro_SD_CardDet" H 4130 8750 50  0000 L CNN
F 2 "uChipProjects:HYC77TF09200" H 4450 9300 50  0001 C CNN
F 3 "http://katalog.we-online.de/em/datasheet/693072010801.pdf" H 3300 9000 50  0001 C CNN
	1    3300 9000
	1    0    0    -1  
$EndComp
Wire Wire Line
	9700 8600 9350 8600
Text Label 9350 8600 0    50   ~ 0
R2
Wire Wire Line
	9700 8700 9350 8700
Text Label 9350 8700 0    50   ~ 0
R1
Wire Wire Line
	9700 8800 9350 8800
Text Label 9350 8800 0    50   ~ 0
R0
Wire Wire Line
	9700 8900 9350 8900
Text Label 9350 8900 0    50   ~ 0
G2
Wire Wire Line
	9700 9000 9350 9000
Text Label 9350 9000 0    50   ~ 0
G1
Wire Wire Line
	9700 9100 9350 9100
Text Label 9350 9100 0    50   ~ 0
G0
Wire Wire Line
	9700 9200 9350 9200
Text Label 9350 9200 0    50   ~ 0
B1
Wire Wire Line
	9700 9300 9350 9300
Text Label 9350 9300 0    50   ~ 0
B0
$Comp
L power:GND #PWR0101
U 1 1 5DBF4A29
P 9500 9750
F 0 "#PWR0101" H 9500 9500 50  0001 C CNN
F 1 "GND" H 9505 9577 50  0000 C CNN
F 2 "" H 9500 9750 50  0001 C CNN
F 3 "" H 9500 9750 50  0001 C CNN
	1    9500 9750
	1    0    0    -1  
$EndComp
Text Label 9350 8400 0    50   ~ 0
DISABLE
Wire Wire Line
	9350 8400 9700 8400
$Comp
L power:+3.3V #PWR0102
U 1 1 5DC08280
P 9150 8400
F 0 "#PWR0102" H 9150 8250 50  0001 C CNN
F 1 "+3.3V" H 9165 8573 50  0000 C CNN
F 2 "" H 9150 8400 50  0001 C CNN
F 3 "" H 9150 8400 50  0001 C CNN
	1    9150 8400
	1    0    0    -1  
$EndComp
Wire Wire Line
	9150 8400 9150 8500
Wire Wire Line
	9150 8500 9700 8500
$Comp
L power:+5V #PWR0103
U 1 1 5DC12186
P 9100 9350
F 0 "#PWR0103" H 9100 9200 50  0001 C CNN
F 1 "+5V" H 9115 9523 50  0000 C CNN
F 2 "" H 9100 9350 50  0001 C CNN
F 3 "" H 9100 9350 50  0001 C CNN
	1    9100 9350
	1    0    0    -1  
$EndComp
Wire Wire Line
	9100 9350 9100 9500
Wire Wire Line
	9100 9500 9700 9500
Wire Wire Line
	9700 9400 9500 9400
Wire Wire Line
	9500 9400 9500 9750
$Comp
L Connector_Generic:Conn_01x12 J3
U 1 1 5DC3B09B
P 9900 8900
F 0 "J3" H 9980 8892 50  0000 L CNN
F 1 "Conn_01x12" H 9980 8801 50  0000 L CNN
F 2 "uChipProjects:PinHeader_1x12_P2.54mm_Vertical" H 9900 8900 50  0001 C CNN
F 3 "~" H 9900 8900 50  0001 C CNN
	1    9900 8900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 5D9BBF74
P 3300 10050
F 0 "#PWR0104" H 3300 9800 50  0001 C CNN
F 1 "GND" H 3305 9877 50  0000 C CNN
F 2 "" H 3300 10050 50  0001 C CNN
F 3 "" H 3300 10050 50  0001 C CNN
	1    3300 10050
	1    0    0    -1  
$EndComp
Wire Wire Line
	3300 9950 3300 10000
Text Notes 2500 7750 0    100  ~ 0
SD CARD READER
Text Notes 5500 7750 0    100  ~ 0
INDICATION LEDs
Text Notes 10250 3700 0    100  ~ 0
DAC AND VGA PORT
Text Notes 8900 7750 0    100  ~ 0
EXPANSION PORT
Text Notes 6750 3750 0    100  ~ 0
BUFFER
Text Notes 1150 3750 0    100  ~ 0
DC IN AND SD+OSCILLATOR 3.3V REGULATOR
Text Notes 7300 1300 0    100  ~ 0
uCHIP AND OSCILLATOR
Text Notes 6100 950  0    200  ~ 0
uCHIP SIMPLE VGA CONSOLE V2.1
Text Notes 13950 4550 0    100  ~ 0
MOUNTING HOLES
Text Notes 1700 5900 0    100  ~ 0
AUDIO FILTER AND OUTPUT
Text Notes 2200 6250 0    50   ~ 0
Note: this is not an amplified output!\nConnecting directly an earpiecce will \ngive a very low sound output!
Text Notes 5800 6700 0    50   ~ 0
The buffer is only required for the SD card, \nand the SPI video mode. However, If you do not require it,\nput a 560-Ohm resistor between G0 and the 3.3V, to prevent\nSWDCK being sensed as "low" during reset. If such pin is held\nlow, uChip will not boot up. \n\nDo NOT use a 74HC245 instead of a 74AHC245!
Wire Wire Line
	3200 9950 3200 10000
Wire Wire Line
	3200 10000 3300 10000
Connection ~ 3300 10000
Wire Wire Line
	3300 10000 3300 10050
Wire Wire Line
	3400 9950 3400 10000
Wire Wire Line
	3400 10000 3300 10000
Wire Wire Line
	3400 10000 3500 10000
Wire Wire Line
	3500 10000 3500 9950
Connection ~ 3400 10000
Text Notes 650  11050 0    100  ~ 0
This schematics and layout are provided as is and they are distributed in the hope that they will be useful, but WITHOUT ANY WARRANTY \nexpress or implied, including - but not limited to, MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\nWe might make changes to specifications, product description and documentation at any time, without notice, therefore \nthere this schematics might not reflect the current version of the product.  You should not finalize a design using this information.
$EndSCHEMATC
