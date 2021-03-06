EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:iotad
LIBS:iotad-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
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
L 24C16 U1
U 1 1 56B5F31C
P 3500 1650
F 0 "U1" H 3650 2000 60  0000 C CNN
F 1 "24C16" H 3700 1300 60  0000 C CNN
F 2 "SMD_Packages:SOIC-8-N" H 3500 1650 60  0001 C CNN
F 3 "" H 3500 1650 60  0000 C CNN
	1    3500 1650
	1    0    0    -1  
$EndComp
$Comp
L MAX31820 U2
U 1 1 56B77B21
P 3500 2700
F 0 "U2" H 3500 2850 60  0000 C CNN
F 1 "MAX31820" H 3450 2950 60  0000 C CNN
F 2 "TO_SOT_Packages_THT:TO-92_Molded_Wide" H 2750 2900 60  0001 C CNN
F 3 "" H 2750 2900 60  0000 C CNN
	1    3500 2700
	1    0    0    -1  
$EndComp
$Comp
L CONN_01X06 P2
U 1 1 56B77BCA
P 5100 2450
F 0 "P2" H 5100 2800 50  0000 C CNN
F 1 "CONN_01X06" V 5200 2450 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x06" H 5100 2450 60  0001 C CNN
F 3 "" H 5100 2450 60  0000 C CNN
	1    5100 2450
	1    0    0    -1  
$EndComp
$Comp
L R R2
U 1 1 56B77C45
P 3650 3100
F 0 "R2" V 3730 3100 50  0000 C CNN
F 1 "4.8k" V 3650 3100 50  0000 C CNN
F 2 "Resistors_SMD:R_1206_HandSoldering" V 3580 3100 30  0001 C CNN
F 3 "" H 3650 3100 30  0000 C CNN
	1    3650 3100
	0    1    1    0   
$EndComp
Wire Wire Line
	3800 3100 3900 3100
Wire Wire Line
	3900 3100 3900 2700
Wire Wire Line
	3100 2150 3100 2700
Connection ~ 2800 1550
Connection ~ 3100 2150
Connection ~ 2800 1650
Wire Wire Line
	4200 1550 4250 1550
Wire Wire Line
	4250 1550 4250 1050
Wire Wire Line
	4250 1050 2800 1050
Connection ~ 2800 1450
Wire Wire Line
	4900 2200 3100 2200
Connection ~ 3100 2200
Text GLabel 4900 2500 0    60   Input ~ 0
RX
Text GLabel 4900 2600 0    60   Input ~ 0
TX
Text GLabel 3500 3100 0    60   Input ~ 0
DATA
Text GLabel 4200 1750 2    60   Input ~ 0
SCL
Text GLabel 4200 1850 2    60   Input ~ 0
SDA
Text GLabel 3900 2700 1    60   Input ~ 0
VCC
Text GLabel 3500 1150 2    60   Input ~ 0
VCC
Wire Wire Line
	2800 1050 2800 2150
Wire Wire Line
	2800 2150 3100 2150
Connection ~ 2800 2150
$Comp
L CONN_01X10 P1
U 1 1 56CE078F
P 1750 2400
F 0 "P1" H 1750 2950 50  0000 C CNN
F 1 "CONN_01X10" V 1850 2400 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x10" H 1750 2400 50  0001 C CNN
F 3 "" H 1750 2400 50  0000 C CNN
	1    1750 2400
	-1   0    0    1   
$EndComp
Text GLabel 1950 2850 2    60   Input ~ 0
5v
Text GLabel 1950 2750 2    60   Input ~ 0
GND
Text GLabel 2800 2150 0    60   Input ~ 0
GND
Text GLabel 1950 2650 2    60   Input ~ 0
SDA
Text GLabel 1950 2550 2    60   Input ~ 0
SCL
Text GLabel 1950 2450 2    60   Input ~ 0
VCC
Text GLabel 1950 2250 2    60   Input ~ 0
RX
Text GLabel 1950 2150 2    60   Input ~ 0
TX
Text GLabel 1950 2050 2    60   Input ~ 0
DATA
$Comp
L LED D1
U 1 1 56CE0997
P 2100 1550
F 0 "D1" H 2100 1650 50  0000 C CNN
F 1 "LED" H 2100 1450 50  0000 C CNN
F 2 "LEDs:LED_1206" H 2100 1550 50  0001 C CNN
F 3 "" H 2100 1550 50  0000 C CNN
	1    2100 1550
	0    -1   -1   0   
$EndComp
$Comp
L R R1
U 1 1 56CE09D6
P 2100 1200
F 0 "R1" V 2180 1200 50  0000 C CNN
F 1 "R" V 2100 1200 50  0000 C CNN
F 2 "Resistors_SMD:R_1206_HandSoldering" V 2030 1200 50  0001 C CNN
F 3 "" H 2100 1200 50  0000 C CNN
	1    2100 1200
	-1   0    0    1   
$EndComp
Wire Wire Line
	1950 1950 1950 1050
Wire Wire Line
	1950 1050 2100 1050
Text GLabel 2100 1750 2    60   Input ~ 0
GND
Text GLabel 3500 2150 2    60   Input ~ 0
GND
$EndSCHEMATC
