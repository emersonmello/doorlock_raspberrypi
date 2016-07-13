# Setting Up Scenario

Figure below shows all necessary components and the relation between them. 

![alt text](components.png "Communication diagram")

##Getting necessary codes 

1. [FIDO UAF Demo Server](https://github.com/emersonmello/UAF)
  - If you prefer, there is a [Docker container](https://www.docker.com/what-docker) ready to use here: https://github.com/emersonmello/docker-fidouafserver
1. [Door lock NFC card reader](https://github.com/emersonmello/doorlock_raspberrypi) - You are already here!
1. [Dummy FIDO UAF Client](https://github.com/emersonmello/dummyuafclient)
1. [Opening Door Android App](https://github.com/emersonmello/openingdoor)

## Setting up

1. Start **FIDO UAF Demo Server**
1. Change **HOSTNAME** and **PORT** values on [Door lock NFC card reader's rp_settings.h file](https://github.com/emersonmello/doorlock_raspberrypi/blob/master/rp_settings.h) to the **IP Address** and **PORT** where you are running the **FIDO UAF Demo Server**
1. Compile **Door lock NFC card reader** project and run it (sorry, you must be root because it is a requirement of wiringPi lib)
1. Install **Dummy FIDO UAF Client** and **Opening Door Android App** on your Android phone
1. On **Opening Door Android App** touch on "Settings" on the main application menu and update "server endpoint" field to the **IP Address** and **PORT** where you are running the **FIDO UAF Demo Server**  
1. On **Opening Door Android App** touch on "See app facetID" on the main application menu and insert the showed value in FIDO UAF Demo Server MySQL database:
  - For instance: ```INSERT  INTO facets (fDesc) values ('android:apk-key-hash:Lir5oIjf552K/XN4bTul0VS3GfM')```

## Testing 

1. Open **Opening Door Android App** and touch "Register" button
1. Tap your mobile phone on "NFC reader"
1. Follow the instructions provided by application (i.e. put your finger on the sensor, etc.) and you should see the message "Access Granted"


