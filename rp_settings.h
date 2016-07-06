/* 
 * File:   rp_settings.h
 * Author: Emerson Ribeiro de Mello <mello@ifsc.edu.br>
 *
 * Created on 29 June 2016, 17:16
 */

#ifndef RP_SETTINGS_H
#define RP_SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#define HOSTNAME "10.66.66.27"
#define SCHEME "http://"
#define PORT 8123
#define REG_REQUEST_ENDPOINT "/fidouaf/v1/public/regRequest"
#define REG_RESPONSE_ENDPOINT "/fidouaf/v1/public/regResponse"
#define REG_REQUEST_MSG "%s%s:%d%s/%s"
    
#define AUTH_REQUEST_ENDPOINT "/fidouaf/v1/public/authRequest"
#define AUTH_RESPONSE_ENDPOINT "/fidouaf/v1/public/authResponse"
#define AUTH_REQUEST_MSG "%s%s:%d%s"

#define APDU "\x00\xA4\x04\x00\x07\xF0\x39\x41\x48\x14\x81\x00\x00"
#define DOOR_HELLO "\x48\x45\x4C\x4C\x4F" //"HELLO"
#define DOOR_READY "\x52\x45\x41\x44\x59" //"READY"
#define DOOR_WAIT "\x57\x41\x49\x54"  //"WAIT"
#define DOOR_ERROR "\x45\x52\x52\x4F\x52"//"ERROR"
#define DOOR_DONE "\x44\x4F\x4E\x45" //"DONE"
#define DOOR_GRANTED "\x47\x52\x41\x4E\x54\x45\x44" //"GRANTED"
#define DOOR_READER_ERROR "\x52\x45\x41\x44\x45\x52\x5F\x45\x52\x52\x4F\x52" //"READER_ERROR"
#define DOOR_DENY "\x44\x45\x4E\x59" //"DENY"
#define DOOR_BYE "\x42\x59\x45" //"BYE"
#define DOOR_NEXT "\x4E\x45\x58\x54" //"NEXT"
#define DOOR_OK "\x4F\x4B" //"OK"
#define DOOR_SUCCESS "\x53\x55\x43\x43\x45\x53\x53" //"SUCCESS"
#define DOOR_RESPONSE "\x52\x45\x53\x50\x4F\x4E\x53\x45" //"RESPONSE"
#define DOOR_RESULT "\x52\x45\x53\x55\x4C\x54" //"RESULT"


#ifdef __cplusplus
}
#endif

#endif /* RP_SETTINGS_H */

