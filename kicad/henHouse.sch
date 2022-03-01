EESchema Schematic File Version 4
EELAYER 30 0
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
L Arduino:Arduino_Nano_Every U?
U 1 1 621518A2
P 4900 3450
F 0 "U?" H 4900 4517 50  0000 C CNN
F 1 "Arduino_Nano_Every" H 4900 4426 50  0000 C CNN
F 2 "Arduino:Arduino_Nano_Every" H 4900 2700 50  0001 C CNN
F 3 "https://store.arduino.cc/usa/nano-every" H 4900 3450 50  0001 C CNN
	1    4900 3450
	1    0    0    -1  
$EndComp
$Comp
L Timer_RTC:DS3231MZ U?
U 1 1 621535A1
P 7600 2700
F 0 "U?" H 7600 2211 50  0000 C CNN
F 1 "DS3231MZ" H 7600 2120 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 7600 2200 50  0001 C CNN
F 3 "http://datasheets.maximintegrated.com/en/ds/DS3231M.pdf" H 7600 2100 50  0001 C CNN
	1    7600 2700
	1    0    0    -1  
$EndComp
$Comp
L ArduinoSD_card:SD_Card_Module A?
U 1 1 621555BF
P 2600 2550
F 0 "A?" H 2930 2596 50  0000 L CNN
F 1 "SD_Card_Module" H 2930 2505 50  0000 L CNN
F 2 "Charleslabs_Parts:SD_Card_Module" H 3550 2600 50  0001 C CNN
F 3 "" H 2300 2900 50  0001 C CNN
	1    2600 2550
	1    0    0    -1  
$EndComp
$Comp
L Device:R_PHOTO R?
U 1 1 621564FB
P 6200 1750
F 0 "R?" H 6270 1796 50  0000 L CNN
F 1 "R_PHOTO" H 6270 1705 50  0000 L CNN
F 2 "" V 6250 1500 50  0001 L CNN
F 3 "~" H 6200 1700 50  0001 C CNN
	1    6200 1750
	1    0    0    -1  
$EndComp
$Comp
L Device:R R?
U 1 1 621576C0
P 4500 1550
F 0 "R?" H 4570 1596 50  0000 L CNN
F 1 "R" H 4570 1505 50  0000 L CNN
F 2 "" V 4430 1550 50  0001 C CNN
F 3 "~" H 4500 1550 50  0001 C CNN
	1    4500 1550
	1    0    0    -1  
$EndComp
$Comp
L Device:LED D?
U 1 1 62158271
P 6650 4200
F 0 "D?" H 6643 4416 50  0000 C CNN
F 1 "LED" H 6643 4325 50  0000 C CNN
F 2 "" H 6650 4200 50  0001 C CNN
F 3 "~" H 6650 4200 50  0001 C CNN
	1    6650 4200
	1    0    0    -1  
$EndComp
$EndSCHEMATC
