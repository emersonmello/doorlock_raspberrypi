<!-- TOC depthFrom:1 depthTo:6 withLinks:1 updateOnSave:1 orderedList:0 -->

- [Setting Up Scenario](#setting-up-scenario)
	- [Getting necessary codes](#getting-necessary-codes)
	- [Setting up](#setting-up)
		- [FIDO UAF Demo Server](#fido-uaf-demo-server)
		- [Door lock NFC card reader on Raspberry PI](#door-lock-nfc-card-reader-on-raspberry-pi)
		- [On Android Phone](#on-android-phone)
	- [Testing](#testing)
	- [UML Sequence Diagram](#uml-sequence-diagram)
- [Starting doorlock automatically on raspberry boot](#starting-doorlock-automatically-on-raspberry-boot)

<!-- /TOC -->

# Setting Up Scenario

Figure below shows all necessary components and the relation between them.

![alt text](components.png "Communication diagram")

## Getting necessary codes

1. [FIDO UAF Demo Server](https://github.com/emersonmello/UAF)
  - If you prefer, there is a [Docker container](https://www.docker.com/what-docker) ready to use here: https://github.com/emersonmello/docker-fidouafserver
1. [Door lock NFC card reader](https://github.com/emersonmello/doorlock_raspberrypi)  <- You are working on it right now!
1. [Dummy FIDO UAF Client](https://github.com/emersonmello/dummyuafclient)
1. [Opening Door Android App](https://github.com/emersonmello/openingdoor)

## Setting up

### FIDO UAF Demo Server

1. Start **FIDO UAF Demo Server**

### Door lock NFC card reader on Raspberry PI

1. `cd ~ && git clone https://github.com/emersonmello/doorlock_raspberrypi.git`
2. `cd doorlock_raspberrypi`
1. Change **HOSTNAME** and **PORT** values on [Door lock NFC card reader's rp_settings.h file](https://github.com/emersonmello/doorlock_raspberrypi/blob/master/rp_settings.h) to the **IP Address** and **PORT** where you are running the **FIDO UAF Demo Server**
  2. For instance: `nano rp_settings.h`
1. Compile **Door lock NFC card reader** project
  2. `make clean && make`
1. Run it (sorry, you must be root because it is a requirement of wiringPi lib)
    - For instance: `sudo ./dist/Debug/GNU-Linux/doorlock_raspberrypi`

### On Android Phone

1. Install **Dummy FIDO UAF Client** and **Opening Door Android App** on your Android phone
1. On **Opening Door Android App** touch on "Settings" on the main application menu and update "server endpoint" field to the **IP Address** and **PORT** where you are running the **FIDO UAF Demo Server**  
1. On **Opening Door Android App** touch on "See app facetID" on the main application menu and insert the showed value in FIDO UAF Demo Server MySQL database:
  - For instance: ```INSERT  INTO facets (fDesc) values ('android:apk-key-hash:Lir5oIjf552K/XN4bTul0VS3GfM')```

## Testing

1. Open **Opening Door Android App** and touch "Register" button
1. Tap your mobile phone on "NFC reader"
1. Follow the instructions provided by application (i.e. put your finger on the sensor, etc.) and you should see the message "Access Granted"

## UML Sequence Diagram

![alt text](sd_doorlock.png "Communication diagram")


# Starting doorlock automatically on raspberry boot

I'm using [supervisord](http://supervisord.org/) to handle this task because it can also restart a failed process

1. Copy `doorlock_raspberrypi` binary to /usr/local/bin
  - `sudo cp ~/doorlock_raspberrypi/dist/Debug/GNU-Linux/doorlock_raspberrypi /usr/local/bin`
1. Installing python2.7
    - `sudo apt-get install python`
1. Installing supervisord
    - `sudo easy_install supervisor`
    - `echo_supervisord_conf | sudo tee /etc/supervisord.conf`
1. Adding a program section to supervisord's configuration file
    - Add the follow lines at the end of **/etc/supervisord.conf** file
    ```
    [program:doorlock]
    command=/usr/local/bin/doorlock_raspberrypi
    ```
    - For instance:
      - ```printf "[program:doorlock]\ncommand=/usr/local/bin/doorlock_raspberrypi\n" | sudo tee -a  /etc/supervisord.conf
     ```
1. Download [supervisord.sh](supervisord.sh) file and save it at `/etc/init.d`
  - `sudo cp ~/doorlock_raspberrypi/supervisord.sh /etc/init.d`
  - `chmod 755 /etc/init.d/supervisord.sh`
  - `sudo update-rc.d supervisord.sh defaults`
