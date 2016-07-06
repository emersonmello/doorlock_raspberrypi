/* 
 * File:   main.c
 * Author: Emerson Ribeiro de Mello <mello@ifsc.edu.br>
 *
 * Created on 30 June 2016, 09:33
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nfc/nfc.h>   
#include "rp_settings.h"
#include "curlutils.h"
#include "nfcutils.h"


nfc_device *pnd;
nfc_target nt;
nfc_context *context;

void nfcInitListen() {
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
}

void nfcProtocol() {
    char message[1024];
    char UAFMessage[2048];
    char cardResponse[15];
    memoryStruct chunk;

    uint8_t capdu[264];
    size_t capdulen;
    uint8_t rapdu[4096];
    size_t rapdulen;
    char *response = 0;


    const nfc_modulation nmMifare = {
        .nmt = NMT_ISO14443A,
        .nbr = NBR_106,
    };

    printf("Polling for target...\n");
    while (nfc_initiator_select_passive_target(pnd, nmMifare, NULL, 0, &nt) <= 0);
    printf("Target detected!\n");


    // Select application
    memcpy(capdu, APDU, sizeof (APDU));
    capdulen = sizeof (APDU); // 13
    rapdulen = sizeof (rapdu);
    if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0)
        exit(EXIT_FAILURE);
    printf("Application selected!\n");

    if (strncmp(rapdu, DOOR_HELLO, (int) rapdulen)) {
        printf("I'm expecting HELLO msg, but card sent to me: %s. len: %d", response, (int) rapdulen);
        exit(EXIT_FAILURE); // TODO, handle this error instead of exit
    }

    // FIDO Auth Request Message
    sprintf(UAFMessage, AUTH_REQUEST_MSG, SCHEME, HOSTNAME, PORT, AUTH_REQUEST_ENDPOINT);
    chunk = getHttpRequest(UAFMessage);

    if (chunk.size <= 0) {
        printf("size: [%ld], message: %s\n", (long) chunk.size, chunk.memory);
        exit(EXIT_FAILURE); // TODO, handle this error instead of exit
    }

    size_t blocks = (chunk.size / 259) + 1;

    char *buffer[blocks];
    blockSplit(chunk.memory, buffer, blocks);
    sprintf(message, "BLOCK:%ld", blocks);
    printf("Sending number of blocks: %s\n", message);

    memcpy(capdu, message, strlen(message));
    capdulen = strlen(message);
    rapdulen = sizeof (rapdu);
    if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0) {
        printf("error\n");
        exit(EXIT_FAILURE);
    }

    int totalSent = 0;

    if (strncmp(rapdu, DOOR_NEXT, (int) rapdulen)) {
        printf("I'm expecting NEXT msg, but card sent to me: %s. len: %d", response, (int) rapdulen);
        exit(EXIT_FAILURE); // TODO, handle this error instead of exit
    }
    response = NULL;
    do { // Sending UAFRequestMessage to card
        memcpy(capdu, buffer[totalSent], strlen(buffer[totalSent]));
        capdulen = strlen(buffer[totalSent]);
        rapdulen = sizeof (rapdu);
        if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0) {
            printf("error\n");
            exit(EXIT_FAILURE);
        }

        if (strncmp(rapdu, DOOR_OK, (int) rapdulen)) {
            printf("I'm expecting OK msg, but card sent to me: %s. len: %d", response, (int) rapdulen);
            exit(EXIT_FAILURE); // TODO, handle this error instead of exit
        }
        response = NULL;
        totalSent++;
    } while (totalSent < blocks);


    printf("Sending READY!\n");
    memcpy(capdu, DOOR_READY, sizeof (DOOR_READY));
    capdulen = strlen(DOOR_READY);
    rapdulen = sizeof (rapdu);
    do {
        if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0)
            exit(EXIT_FAILURE);

        if (strncmp(rapdu, DOOR_WAIT, (int) rapdulen) == 0) {
            continue;
        }

        if (strncmp(rapdu, DOOR_DONE, (int) rapdulen) == 0) {
            printf("Sending RESPONSE!\n");
            memcpy(capdu, DOOR_RESPONSE, sizeof (DOOR_RESPONSE));
            capdulen = strlen(DOOR_RESPONSE);
            rapdulen = sizeof (rapdu);
            if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0) {
                printf("Error?");
                exit(EXIT_FAILURE);
            }
            break;
        }
    } while (1);

    strncpy(message, rapdu, (int) rapdulen);
    char *p = strtok(message, ":");
    char *endptr;
    long val;
    if (strcmp(p, "BLOCK") == 0) {
        p = strtok(NULL, ":");
        val = strtol(p, &endptr, 10);
        char UAFmsg[259 * val];
        int pos = 0;
        do {
            printf("Sending NEXT!\n");
            memcpy(capdu, DOOR_NEXT, sizeof (DOOR_NEXT));
            capdulen = strlen(DOOR_NEXT);
            rapdulen = sizeof (rapdu);
            if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0) {
                exit(EXIT_FAILURE);
            }
            //            strncpy(message, rapdu, (int) rapdulen);
            //            message[(int) rapdulen +1] = '\0';
            //            printf("message: %s\n", message);
            strncat(UAFmsg, rapdu, (int) rapdulen);
            val--;
        } while (val > 0);
        // FIDO Auth Request Message
        sprintf(UAFMessage, AUTH_REQUEST_MSG, SCHEME, HOSTNAME, PORT, AUTH_RESPONSE_ENDPOINT);
        curlFetchStruct *result = postHttpRequest(UAFMessage, UAFmsg);
        
        if (strstr(result->payload,"KEY_NOT_REGISTERED") != NULL){
            printf("Access denied!\n");
            memcpy(capdu, DOOR_DENY, sizeof (DOOR_DENY));
            capdulen = strlen(DOOR_DENY);
            rapdulen = sizeof (rapdu);
            if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0) {
                exit(EXIT_FAILURE);
            }
        }else{
            printf("Access granted!\n");
            memcpy(capdu, DOOR_GRANTED, sizeof (DOOR_GRANTED));
            capdulen = strlen(DOOR_GRANTED);
            rapdulen = sizeof (rapdu);
            if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0) {
                exit(EXIT_FAILURE);
            }
            
        }
        strncpy(message, rapdu, (int) rapdulen);
        if (strstr(message,"BYE") != NULL){
            printf("bye!\n");
        }else{
            printf(":-(\n");
        }
    }

    printf("end.\n");
    nfc_close(pnd);
    nfc_exit(context);
}

/*
 * 
 */
int main(int argc, char** argv) {
    nfc_init(&context);
    if (context == NULL) {
        printf("Unable to init libnfc (malloc)\n");
        exit(EXIT_FAILURE);
    }

    nfcInitListen();
    nfcProtocol();

    return (EXIT_SUCCESS);
}

