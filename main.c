/* 
 * File:   main.c
 * Author: Emerson Ribeiro de Mello <mello@ifsc.edu.br>
 *
 * Created on 30 June 2016, 09:33
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <nfc/nfc.h>   
#include "rp_settings.h"
#include "curlutils.h"
#include "wiring-gpio.h"


nfc_device *pnd;
nfc_target nt;
nfc_context *context;

void signal_handler(int signal) {

    switch (signal) {
        case SIGHUP:
        case SIGUSR1:
        case SIGINT:
        case SIGTERM:
        case SIGQUIT:
            printf("\nFinalizing....\n");
            cleaning();
            exit(EXIT_SUCCESS);
        default:
            fprintf(stderr, "\nCaught wrong signal: %d\n", signal);
            exit(EXIT_FAILURE);
    }
}

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

int CardTransmit(nfc_device *pnd, uint8_t *capdu, size_t capdulen, uint8_t *rapdu, size_t *rapdulen) {
    int res;
    size_t szPos;
    //    printf("=> ");
    //    for (szPos = 0; szPos < capdulen; szPos++) {
    //        printf("%hhx ", capdu[szPos]);
    //    }
    //    printf("\n");
    if ((res = nfc_initiator_transceive_bytes(pnd, capdu, capdulen, rapdu, *rapdulen, 500)) < 0) {
        return -1;
    } else {
        *rapdulen = (size_t) res;
        //        printf("<= ");
        //        for (szPos = 0; szPos < *rapdulen; szPos++) {
        //            printf("%hhx ", rapdu[szPos]);
        //        }
        //        printf("\n");
        //        for (szPos = 0; szPos < *rapdulen; szPos++) {
        //            printf("%c", (char) rapdu[szPos]);
        //        }
        //        printf("len: %d\n", *rapdulen);
        return 0;
    }
}

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
    memoryStruct chunk;

    uint8_t capdu[264];
    size_t capdulen;
    uint8_t rapdu[264];
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
        printf("** Opss ** I'm expecting HELLO msg, but card sent to me: %s. len: %d\n", response, (int) rapdulen);
        return;
    }

    // FIDO Auth Request Message
    printf("Doing AuthRequest to FIDO UAF Server\n");
    sprintf(UAFMessage, AUTH_REQUEST_MSG, SCHEME, HOSTNAME, PORT, AUTH_REQUEST_ENDPOINT);
    chunk = getHttpRequest(UAFMessage);

    if (chunk.size <= 0) {
        printf("** Opss ** Error to connect to FIDO Server\n");
        memcpy(capdu, "ERROR", 5);
        capdulen = 5;
        rapdulen = sizeof (rapdu);
        if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0) {
            printf("Error to sent error message to card...\n");
        }
        return;
    }

    size_t blocks = (chunk.size / BLOCK_SIZE) + 1;

    char *buffer[blocks];
    blockSplit(chunk.memory, buffer, blocks);
    sprintf(message, "BLOCK:%ld", blocks);
    printf("Sending number of blocks: %s\n", message);
    free(chunk.memory);

    memcpy(capdu, message, strlen(message));
    capdulen = strlen(message);
    rapdulen = sizeof (rapdu);
    if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0) {
        printf("Error to send number of blocks\n");
        return;
    }

    int totalSent = 0;

    if (strncmp(rapdu, DOOR_NEXT, (int) rapdulen)) {
        printf("** Opss ** I'm expecting NEXT msg, but card sent to me: %s. len: %d", response, (int) rapdulen);
        return;
    }

    response = NULL;
    do { // Sending UAFRequestMessage to card
        memcpy(capdu, buffer[totalSent], strlen(buffer[totalSent]));
        capdulen = strlen(buffer[totalSent]);
        rapdulen = sizeof (rapdu);
        if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0) {
            int i = strlen(buffer[totalSent]);
            printf("error to send UAFRequestMessage to card. Message size: %d, message: %s\n", i, buffer[totalSent]);
            return;
        }

        if (strncmp(rapdu, DOOR_OK, (int) rapdulen)) {
            printf("** Opss ** I'm expecting OK msg, but card sent to me: %s. len: %d", response, (int) rapdulen);
            return;
        }
        response = NULL;
        free(buffer[totalSent]);
        totalSent++;
    } while (totalSent < blocks);


    printf("Sending READY!\n");
    memcpy(capdu, DOOR_READY, sizeof (DOOR_READY));
    capdulen = strlen(DOOR_READY);
    rapdulen = sizeof (rapdu);
    unsigned long timeout = LONG_MAX;
    do {
        if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0) {
            printf("Error to received WAIT\n");
            return;
        }

        if (strncmp(rapdu, DOOR_WAIT, (int) rapdulen) == 0) {
            continue;
        }

        if (strncmp(rapdu, DOOR_DONE, (int) rapdulen) == 0) {
            printf("Sending RESPONSE!\n");
            memcpy(capdu, DOOR_RESPONSE, sizeof (DOOR_RESPONSE));
            capdulen = strlen(DOOR_RESPONSE);
            rapdulen = sizeof (rapdu);
            if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0) {
                printf("Error RESPONSE...\n");
                return;
            }
            break;
        }
        timeout--;
    } while (timeout > 0);

    if (timeout == 0) {
        printf("Error. Client is not responding...\n");
        return;
    }

    strncpy(message, rapdu, (int) rapdulen);
    char *p = strtok(message, ":");
    char *endptr;
    long val;
    if (strcmp(p, "BLOCK") == 0) {
        p = strtok(NULL, ":");
        val = strtol(p, &endptr, 10);
        char *UAFmsg = malloc(sizeof (char) * BLOCK_SIZE * val + 1);
        UAFmsg[0] = '\0';
        do {
            printf("Sending NEXT!\n");
            memcpy(capdu, DOOR_NEXT, sizeof (DOOR_NEXT));
            capdulen = strlen(DOOR_NEXT);
            rapdulen = sizeof (rapdu);
            if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0) {
                printf("Error NEXT\n");
                return;
            }
            strncat(UAFmsg, rapdu, (int) rapdulen);
            val--;
        } while (val > 0);
        // FIDO Auth Request Message
        printf("Forwarding card response to FIDO UAF Server: \n");
        sprintf(UAFMessage, AUTH_REQUEST_MSG, SCHEME, HOSTNAME, PORT, AUTH_RESPONSE_ENDPOINT);
        //curlFetchStruct *result = postHttpRequest(UAFMessage, UAFmsg);
        json_object *result = postHttpRequest(UAFMessage, UAFmsg);
        free(UAFmsg);

        //        if (result->size <= 0) {
        if (result == NULL) {
            printf("Error to connect to FIDO Server\n");
            memcpy(capdu, "ERROR", 5);
            capdulen = 5;
            rapdulen = sizeof (rapdu);
            if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0) {
                printf("Error to send response to card\n");
            }
            return;
        }
        printf("Resul payload: %s \n", json_object_to_json_string(result));
        const char *resultStr = json_object_to_json_string(result);

        
        if (strstr(resultStr, "SUCCESS") == NULL) {
            printf("Access denied!\n");
            memcpy(capdu, DOOR_DENY, sizeof (DOOR_DENY));
            capdulen = strlen(DOOR_DENY);
            rapdulen = sizeof (rapdu);
            if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0) {
                return;
            }
        } else {
            printf("Access granted!\n");
            memcpy(capdu, DOOR_GRANTED, sizeof (DOOR_GRANTED));
            capdulen = strlen(DOOR_GRANTED);
            rapdulen = sizeof (rapdu);
            if (CardTransmit(pnd, capdu, capdulen, rapdu, &rapdulen) < 0) {
                return;
            }
            doorlock(1);
            doorlock(0);
        }
        /* free json object */
        json_object_put(result);
        
        strncpy(message, rapdu, (int) rapdulen);
        if (strstr(message, "BYE") != NULL) {
            printf("bye!\n");
        } else {
            printf(":-(\n");
        }
    }
}

/*
 * 
 */
int main(int argc, char** argv) {

    // Handling Linux Signals
    if (signal(SIGHUP, signal_handler) == SIG_ERR) {
        perror("\nCan't catch SIGHUP\n");
    }
    if (signal(SIGQUIT, signal_handler) == SIG_ERR) {
        perror("\nCan't catch SIGQUIT\n");
    }
    if (signal(SIGUSR1, signal_handler) == SIG_ERR) {
        perror("\nCan't catch SIGUSR1\n");
    }
    // supervisord will send this one
    if (signal(SIGTERM, signal_handler) == SIG_ERR) {
        perror("\nCan't catch SIGTERM\n");
    }
    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        perror("\nCan't catch SIGINT\n");
    }

    // About GPIO & wiringPI Lib
    setupWiring();

    while (1) {
        nfc_init(&context);
        if (context == NULL) {
            printf("Unable to init libnfc (malloc)\n");
            exit(EXIT_FAILURE);
        }
        nfcInitListen();
        nfcProtocol();
        nfc_close(pnd);
        nfc_exit(context);
        sleep(2);
        printf("starting again...\n");
    }

    return (EXIT_SUCCESS);
}

