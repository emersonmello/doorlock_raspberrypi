/* 
 * File:   nfcutils.h
 * Author: Emerson Ribeiro de Mello <mello@ifsc.edu.br>
 *
 * Created on 30 June 2016, 10:15
 */

#ifndef NFCUTILS_H
#define NFCUTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <nfc/nfc.h>    

    char *charToHex(char *str);
//    void subString(char *str, int len, char *a);
    int CardTransmit(nfc_device *pnd, uint8_t *capdu, size_t capdulen, uint8_t *rapdu, size_t *rapdulen);
    void initNFC();



#ifdef __cplusplus
}
#endif

#endif /* NFCUTILS_H */

