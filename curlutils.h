/* 
 * File:   curlutils.h
 * Author: Emerson Ribeiro de Mello <mello@ifsc.edu.br>
 *
 * Created on 30 June 2016, 09:53
 */

#ifndef CURLUTILS_H
#define CURLUTILS_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdlib.h>
#include <string.h>
#include <json/json.h>
#include <curl/curl.h>
    
struct MemoryStruct {
    char *memory;
    size_t size;
} typedef memoryStruct;

/* holder for curl fetch */
struct curl_fetch_st {
    char *payload;
    size_t size;
} typedef curlFetchStruct;

#define BLOCK_SIZE 259
char* subString (const char* input, int offset, int len, char* dest);
void blockSplit(const char* input, char **result, size_t block);
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);
size_t curl_callback (void *contents, size_t size, size_t nmemb, void *userp);
CURLcode curl_fetch_url(CURL *ch, const char *url, struct curl_fetch_st *fetch);
struct curl_fetch_st* postHttpRequest(char *url, char *payload);
memoryStruct getHttpRequest(char *);
    


#ifdef __cplusplus
}
#endif

#endif /* CURLUTILS_H */

