# ITSkate Remote

An open source remote for the VESC based on the arduino samd architecture and the HM-10 BLE module.

To build the board from scratch will require the following items:
	- All 3 finished boards (A BOM is in the works, but can be determined from the EAGLE files under "Hardware").
	- The Arduino IDE with the Arduino SAMD boards installed.
	- The bootloader, which can be loaded by adding http://isaactaylor.net/package_itaylor_index.json to Preferences>Additional Boards Manager URLs, and installing ITSkate Remote from the Board Manager.
	- A j-link, stlink v2, or other programmer for the samd chip. (This guide will use the stlink v2).

## Building the board

## Programming the HM-10s

Programming the HM-10 module takes 6 steps, using the AT commands. You may choose to perform this step later, as the TX software includes an AT passthrough mode.

You can learn more about the HM-10 and it's AT commands from the manual: http://fab.cba.mit.edu/classes/863.15/doc/tutorials/programming/bluetooth/bluetooth40_en.pdf

1. AT
You should recieve OK in return. If nothing appears, make sure you have a genuine HM10 and not a clone, then make sure you are not sending LF or CR, and that your baud rate is 9600 (we will be changing this later, but this is the default).

2. AT+ADDR?
Take note of the address of both the RX and TX.

3. AT+ROLE?
Setting the role will depend on if this is the TX or RX. The TX will be set to AT+ROLE1, because it is the master device. The RX will be set to AT+ROLE0 (left as default).

4. AT+CONxxxxxxxx
Replace the xxxxx with the address of the opposite board, performing the command first on the RX board, then the TX. If both are powered on, you should recieve OK+CONN from the TX.

5. AT+POWE3
Boost the power on both to 6dbm.

6. AT+BAUD4
Set the baud rate of the UART connection to 115200. 

7. AT+RESET
After resetting you'll have to reconnect with the baud rate of 115200. You can type AT to check your work.

## Building the bootloader

## Flashing the bootloader

## Loading the software


