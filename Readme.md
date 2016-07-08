# Doorlock NFC card reader for Raspberry PI

## Motivation

Is it possible to use [FIDO UAF Standard](https://fidoalliance.org/specifications/download/) on an Android mobile + NFC to openning doors?

This project is a simple prototype to verify how FIDO could be used in this scenario. This ***card reader*** uses NFC to communicate with a specific ***Android Openning Door App***, that emulates a NFC card using Android's [Host-based Card Emulation](https://developer.android.com/guide/topics/connectivity/nfc/hce.html) functionality. 

The ***card reader*** and ***Android Openning Door App*** depend of a third-party, called **FIDO UAF RP Server**.


## Setup requirements

### Hardware

- [Raspberry PI 2 B](https://www.raspberrypi.org/products/raspberry-pi-2-model-b/)
- [Adafruit PN532](https://www.adafruit.com/products/364)
- A simple Breadboard to connect raspberry and PN532


### Software

- [Libcurl](https://curl.haxx.se/libcurl/)
- [Libnfc >= 1.7.1](https://github.com/nfc-tools/libnfc) 
- [Json-c >= 0.10-1.2](https://github.com/json-c/json-c)


### Installing required packages

	sudo apt-get install git build-essentials libusb-dev libcurl4-openssl-dev libjson0-dev autoconf libtool libpcsclite-dev
    

### Freeing UART on the Raspberry PI running Raspbian GNU/Linux 7

	sudo raspi-config
	
- Go to option 8 	"Advanced Options"
- Go to option A8 "Serial" and select **NO**
- Finish and reboot: `sudo shutdown -r now`	

### Installing libnfc from source

####Preparing the environment
	
	cd ~ && mkdir -p /etc/nfc/devices.d
	git clone https://github.com/nfc-tools/libnfc.git
	cd libnfc
	
    sudo cp contrib/libnfc/pn532_uart_on_rpi.conf.sample /etc/nfc/devices.d/pn532_uart_on_rpi.conf
    
	sudo echo "allow_instrusive_scan =  true" >> /etc/nfc/devices.d/pn532_uart_on_rpi.conf
	
####Run config & build
	
	autoreconf -vis
	./configure --with-drivers=pn532_uart --sysconfdir=/etc --prefix=/usr
	sudo make clean && sudo make install all


### Wiring Raspberry PI 2 B & PNB532

1. To use UART on PNB532 breakout you must set to **OFF** the **SEL0** and **SEL1** jumpers
1. Follow instructions (and picture) below


|Raspberry PI 2 B    | Wire color | PNB532 |
|--------------------|:----------:|:------:|
| Pin 2 (5v)         | RED        | 5.0V   |
| Pin 6 (ground)     | BLACK      | GND    |
| Pin 8 (BCM 14 TXD) | YELLOW     | TXD    |
| Pin 10 (BCM 15 RXD)| GREEN      | RXD    |


![alt text](hw-wiring.png "Wiring raspberry PI 2 B & PNB532")

### Testing

You can test your setup reading an ISO14443-A card using `nfc-poll` program that came with `libnfc`. Place a card on the reader and run the command:


    cd ~/libnfc/examples
    ./nfc-poll




## References

- https://learn.adafruit.com/adafruit-nfc-rfid-on-raspberry-pi/
- https://curl.haxx.se/libcurl/c/libcurl-tutorial.html
- https://gist.github.com/leprechau/e6b8fef41a153218e1f4
- https://github.com/nfc-tools/libnfc/tree/master/examples
- https://netbeans.org/kb/docs/cnd/remotedev-tutorial.html
- http://pinout.xyz/

![alt text](pinout.png "Raspberry PI 2 B pinout")

