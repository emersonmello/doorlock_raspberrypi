#include "nfcutils.h"
#include "rp_settings.h"
#include <string.h>

char *charToHex(char *str) {
    int size = strlen(str);
    int i;
    char *buf_str = (char *) malloc(4 * size + 1);
    char *buf_ptr = buf_str;

    for (i = 0; i < size; i++) {
        buf_ptr += sprintf(buf_ptr, "\\x%02X", str[i]);
    }
    sprintf(buf_ptr, "\n");
    *(buf_ptr + 1) = '\0';
    return buf_ptr;
}

//void subString(char *str, int len, char *a) {
//    for(int i =0; i < len; i++){
//        a[i] = str[i];
//    }
//    a[len] = '\0';
//}

int CardTransmit(nfc_device *pnd, uint8_t *capdu, size_t capdulen, uint8_t *rapdu, size_t *rapdulen) {
    int res;
    size_t szPos;
    printf("=> ");
    for (szPos = 0; szPos < capdulen; szPos++) {
        printf("%hhx ", capdu[szPos]);
    }
    printf("\n");
    if ((res = nfc_initiator_transceive_bytes(pnd, capdu, capdulen, rapdu, *rapdulen, 500)) < 0) {
        return -1;
    } else {
        *rapdulen = (size_t) res;
        printf("<= ");
        for (szPos = 0; szPos < *rapdulen; szPos++) {
            printf("%hhx ", rapdu[szPos]);
        }
        printf("\n");
        //        for (szPos = 0; szPos < *rapdulen; szPos++) {
        //            printf("%c", (char) rapdu[szPos]);
        //        }
        //        printf("len: %d\n", *rapdulen);
        return 0;
    }
}

void initNFC() {
    nfc_device *pnd;
    nfc_target nt;
    nfc_context *context;
    nfc_init(&context);
    if (context == NULL) {
        printf("Unable to init libnfc (malloc)\n");
        exit(EXIT_FAILURE);
    }
    const char *acLibnfcVersion = nfc_version();

    pnd = nfc_open(context, NULL);

    if (pnd == NULL) {
        printf("ERROR: %s", "Unable to open NFC device.");
        exit(EXIT_FAILURE);
    }
    if (nfc_initiator_init(pnd) < 0) {
        nfc_perror(pnd, "nfc_initiator_init");
        exit(EXIT_FAILURE);
    }

    printf("NFC reader: %s opened\n", nfc_device_get_name(pnd));

    const nfc_modulation nmMifare = {
        .nmt = NMT_ISO14443A,
        .nbr = NBR_106,
    };
    printf("Polling for target...\n");
    while (nfc_initiator_select_passive_target(pnd, nmMifare, NULL, 0, &nt) <= 0);
    printf("Target detected!\n");
    uint8_t capdu[264];
    size_t capdulen;
    uint8_t rapdu[264];
    size_t rapdulen;
    // Select application
    memcpy(capdu, "\x00\xA4\x04\x00\x07\xF0\x39\x41\x48\x14\x81\x00\x00", 13);
    capdulen = 13;
    rapdulen = sizeof (rapdu);
    if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0)
        exit(EXIT_FAILURE);
    printf("Application selected!\n");


    //////////////////////////////////////////////////////

    char str[264];
    char resultStr[264];
    uint32_t host;
    host = htonl(*(uint32_t *) rapdu);
    sprintf(str, "%x", host);
    printf("\nstring:  %s\n", str);

    printf("DOOR: %s, %s \n", DOOR_HELLO, str);




    //    int c, i=0, k=0;
    //    int j = 4;
    //    while (j >= 0){
    //        sscanf(str+k,"%2x", &c);
    //        resultStr[i++] = c;
    //        k+=2;
    //        j--;
    //    }
    //    resultStr[i]='\0';
    //    
    //    printf("result: %d %s\n", k ,resultStr);
    //////////////////////////////////////////////////////




    if ((rapdu[0] == 0x48) && (rapdu[1] == 0x45)) {
        printf("Card said HELLO\n");
    }
    //////////////////////////////////////////////////////


    printf("Sending READY!\n");
    memcpy(capdu, DOOR_READY, sizeof (DOOR_READY));
    capdulen = 5;
    rapdulen = sizeof (rapdu);
    if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0)
        exit(EXIT_FAILURE);




    printf("ok\n");
    nfc_close(pnd);
    nfc_exit(context);
}

