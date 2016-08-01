<!-- TOC depthFrom:1 depthTo:6 withLinks:1 updateOnSave:1 orderedList:0 -->

- [Doorlock NFC card reader for Raspberry PI](#doorlock-nfc-card-reader-for-raspberry-pi)
	- [Motivation](#motivation)
	- [Hardware requirements](#hardware-requirements)
		- [Wiring Raspberry PI 2 B & PNB532](#wiring-raspberry-pi-2-b-pnb532)
	- [Software requirments](#software-requirments)
		- [Installing required packages](#installing-required-packages)
		- [Freeing UART on the Raspberry PI running Raspbian GNU/Linux 7](#freeing-uart-on-the-raspberry-pi-running-raspbian-gnulinux-7)
		- [Installing libnfc from source](#installing-libnfc-from-source)
			- [Preparing the environment](#preparing-the-environment)
			- [Run config & build](#run-config-build)
		- [Installing wiringPi from source](#installing-wiringpi-from-source)
		- [Testing](#testing)
		- [Running FIDO UAF Server, Card Reader and Opening Door Android Application](#running-fido-uaf-server-card-reader-and-opening-door-android-application)
	- [References](#references)

<!-- /TOC -->

# Doorlock NFC card reader for Raspberry PI

## Motivation

Is it possible to use [FIDO UAF Standard](https://fidoalliance.org/specifications/download/) on an Android mobile + NFC to opening doors?

This project is a simple prototype to verify how FIDO could be used in this scenario (see [FIDO Security Reference](https://fidoalliance.org/specs/fido-uaf-v1.0-ps-20141208/fido-security-ref-v1.0-ps-20141208.html#threats-to-the-secure-channel-between-client-and-relying-party)). This [card reader](https://github.com/emersonmello/doorlock_raspberrypi) uses NFC to communicate with a specific [Android Opening Door App](https://github.com/emersonmello/openingdoor), that emulates a NFC card using Android's [Host-based Card Emulation](https://developer.android.com/guide/topics/connectivity/nfc/hce.html) functionality.

The [card reader](https://github.com/emersonmello/doorlock_raspberrypi) and [Android Opening Door App](https://github.com/emersonmello/openingdoor) depend of a third-party, called [FIDO UAF RP Server](https://github.com/emersonmello/UAF).

Figure below shows all necessary components and the relation between them

![alt text](components.png "Communication diagram")

1. [FIDO UAF Demo Server](https://github.com/emersonmello/UAF)
  - If you prefer, there is a [Docker container](https://www.docker.com/what-docker) ready to use here: https://github.com/emersonmello/docker-fidouafserver
1. [Door lock NFC card reader](https://github.com/emersonmello/doorlock_raspberrypi) <- You are working on it right now!
1. [Dummy FIDO UAF Client](https://github.com/emersonmello/dummyuafclient)
1. [Opening Door Android App](https://github.com/emersonmello/openingdoor)

## Hardware requirements

- 01 [Raspberry PI 2 B](https://www.raspberrypi.org/products/raspberry-pi-2-model-b/)
- 01 [Adafruit PN532](https://www.adafruit.com/products/364)
- 01 Breadboard to connect raspberry and PN532
- 01 N channel MOSFET - https://www.adafruit.com/products/355
- 01 Lock-style solenoid - https://www.adafruit.com/product/1512
- 02 Resistors - 300 Ohms
- 02 LED (red and green)
- 02 Diode 1N4001 - https://www.adafruit.com/product/755

### Wiring Raspberry PI 2 B & PNB532

1. To use UART on PNB532 breakout you must set to **OFF** the **SEL0** and **SEL1** jumpers
1. Follow instructions (and picture) below to connect all components


|Raspberry PI 2 B    | Wire color | PNB532 |
|--------------------|:----------:|:------:|
| Pin 2 (5v)         | RED        | 5.0V   |
| Pin 6 (ground)     | BLACK      | GND    |
| Pin 8 (BCM 14 TXD) | YELLOW     | TXD    |
| Pin 10 (BCM 15 RXD)| GREEN      | RXD    |


![alt text](hw-wiring.png "Wiring raspberry PI 2 B & PNB532 & solenoid")

|Raspberry PI 2 B    | Wire color | Component                |
|--------------------|:----------:|:------------------------:|
| Pin 11 (BCM 17)    | BLUE       | Green LED cathode (-)    |
| Pin 13 (BCM 27)    | BLUE       | RED LED cathode (-)      |
| Pin 15 (BCM 22)    | PURPLE     | Diode #1 anode (+)       |
| Pin 39 (Ground)    | BLACK      | Breadboard negative rail |



## Software requirments

- [Libcurl](https://curl.haxx.se/libcurl/)
- [Libnfc >= 1.7.1](https://github.com/nfc-tools/libnfc)
- [Json-c >= 0.11-4](https://github.com/json-c/json-c)
- [wiringPi library](http://wiringpi.com/)


### Installing required packages

	sudo apt-get install git build-essential autoconf libtool libpcsclite-dev
	sudo apt-get install libusb-dev libcurl4-openssl-dev libjson0-dev


### Freeing UART on the Raspberry PI running Raspbian GNU/Linux 7

	sudo raspi-config

- Go to option 8 	"Advanced Options" - (Ps: for Raspbian 8 go to option 9)
- Go to option A8 "Serial" and select **NO**
- Finish and reboot: `sudo shutdown -r now`

### Installing libnfc from source

#### Preparing the environment

	cd ~ && sudo mkdir -p /etc/nfc/devices.d
	git clone https://github.com/nfc-tools/libnfc.git
	cd libnfc

	sudo cp contrib/libnfc/pn532_uart_on_rpi.conf.sample /etc/nfc/devices.d/pn532_uart_on_rpi.conf

	echo "allow_instrusive_scan =  true" | sudo tee -a  /etc/nfc/devices.d/pn532_uart_on_rpi.conf

#### Run config & build

	autoreconf -vis
	./configure --with-drivers=pn532_uart --sysconfdir=/etc --prefix=/usr
	sudo make clean && sudo make install all

### Installing wiringPi from source

- Please, follow the instructions provided by [official website](http://wiringpi.com/download-and-install).




### Testing

You can test your setup reading an ISO14443-A card using `nfc-poll` program that came with `libnfc`. Place a card on the reader and run the command:


    cd ~/libnfc/examples
    ./nfc-poll

### Running FIDO UAF Server, Card Reader and Opening Door Android Application

Please, follow steps showed [here](Setup.md)


## References

- https://learn.adafruit.com/adafruit-nfc-rfid-on-raspberry-pi/
- https://curl.haxx.se/libcurl/c/libcurl-tutorial.html
- https://gist.github.com/leprechau/e6b8fef41a153218e1f4
- https://github.com/nfc-tools/libnfc/tree/master/examples
- https://netbeans.org/kb/docs/cnd/remotedev-tutorial.html
- http://pinout.xyz/

![alt text](pinout.png "Raspberry PI 2 B pinout")
