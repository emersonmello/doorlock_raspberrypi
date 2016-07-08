#include "curlutils.h"

char* subString(const char* input, int offset, int len, char* dest) {
    int input_len = strlen(input);
    if (offset + len > input_len) {
        return NULL;
    }
    strncpy(dest, input + offset, len);
    return dest;
}

void blockSplit(const char* input, char **result, size_t block) {
    int s = strlen(input);
    int offset = 0;
    int len = BLOCK_SIZE;
    size_t pos = 0;
    for (; pos < block; pos++) {
        result[pos] = malloc(sizeof (char) * len);
        subString(input, offset, len, result[pos]);
        offset += 259;
        len = ((s - offset) < 259) ? s - offset : 259;
    }
}

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *) userp;

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

/* callback for curl fetch */
size_t curl_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb; /* calculate buffer size */
    struct curl_fetch_st *p = (struct curl_fetch_st *) userp; /* cast pointer to fetch struct */

    /* expand buffer */
    p->payload = (char *) realloc(p->payload, p->size + realsize + 1);

    /* check buffer */
    if (p->payload == NULL) {
        /* this isn't good */
        fprintf(stderr, "ERROR: Failed to expand buffer in curl_callback");
        /* free buffer */
        free(p->payload);
        /* return */
        return -1;
    }

    /* copy contents to buffer */
    memcpy(&(p->payload[p->size]), contents, realsize);

    /* set new buffer size */
    p->size += realsize;

    /* ensure null termination */
    p->payload[p->size] = 0;

    /* return size */
    return realsize;
}

/* fetch and return url body via curl */
CURLcode curl_fetch_url(CURL *ch, const char *url, struct curl_fetch_st *fetch) {
    CURLcode rcode; /* curl result code */

    /* init payload */
    fetch->payload = (char *) calloc(1, sizeof (fetch->payload));

    /* check payload */
    if (fetch->payload == NULL) {
        /* log error */
        fprintf(stderr, "ERROR: Failed to allocate payload in curl_fetch_url");
        /* return error */
        return CURLE_FAILED_INIT;
    }

    /* init size */
    fetch->size = 0;

    /* set url to fetch */
    curl_easy_setopt(ch, CURLOPT_URL, url);

    /* set calback function */
    curl_easy_setopt(ch, CURLOPT_WRITEFUNCTION, curl_callback);

    /* pass fetch struct pointer */
    curl_easy_setopt(ch, CURLOPT_WRITEDATA, (void *) fetch);

    /* set default user agent */
    curl_easy_setopt(ch, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    /* set timeout */
    curl_easy_setopt(ch, CURLOPT_TIMEOUT, 5);

    /* enable location redirects */
    curl_easy_setopt(ch, CURLOPT_FOLLOWLOCATION, 1);

    /* set maximum allowed redirects */
    curl_easy_setopt(ch, CURLOPT_MAXREDIRS, 1);

    /* fetch the url */
    rcode = curl_easy_perform(ch);

    /* return */
    return rcode;
}

void remove_all_chars(char* str, char c) {
    char *pr = str, *pw = str;
    while (*pr) {
        *pw = *pr++;
        pw += (*pw != c);
    }
    *pw = '\0';
}

struct curl_fetch_st *postHttpRequest(char *url, char *payload) {
    CURL *curl;
    CURLcode rcode;

    json_object *json; /* json post body */
    enum json_tokener_error jerr = json_tokener_success; /* json parse error */

    struct curl_fetch_st curl_fetch; /* curl fetch struct */
    struct curl_fetch_st *cf = &curl_fetch; /* pointer to fetch struct */
    struct curl_slist *headers = NULL; /* http headers to send with request */

    memoryStruct chunk;

    chunk.memory = malloc(1); /* will be grown as needed by the realloc above */
    chunk.size = 0; /* no data at this point */

    /* set content type */
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");

    //remove_all_chars(payload, '\\');
    json = json_tokener_parse(payload);
    
    json_object *foo = json_object_object_get(json,"uafProtocolMessage");
    char const *decoded = json_object_get_string(foo);
    enum json_type type;
     

    curl = curl_easy_init();

    if (curl) {
        /* set curl options */
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, decoded);

        /* Perform the request, res will get the return code */
        rcode = curl_fetch_url(curl, url, cf);

        /* always cleanup */
        curl_easy_cleanup(curl);

        /* free headers */
        curl_slist_free_all(headers);

        /* free json object */
        json_object_put(json);

        /* check return code */
        if (rcode != CURLE_OK || cf->size < 1) {
            /* log error */
            fprintf(stderr, "ERROR: Failed to fetch url (%s) - curl said: %s",
                    url, curl_easy_strerror(rcode));
            /* return error */
            //return NULL;
        }

        /* check payload */
        if (cf->payload != NULL) {
            /* debug -  print result */
            // printf("CURL Returned: \n%s\n", cf->payload);
            /* parse return */
            json = json_tokener_parse_verbose(cf->payload, &jerr);
            /* free payload */
            free(cf->payload);
        } else {
            /* error */
            fprintf(stderr, "ERROR: Failed to populate payload");
            /* free payload */
            free(cf->payload);
            /* return */
            //return NULL;
        }
        /* check error */
        if (jerr != json_tokener_success) {
            /* error */
            fprintf(stderr, "ERROR: Failed to parse json string");
            /* free json object */
            json_object_put(json);
            /* return */
            //return NULL;
        }
        /* debugging */
        //printf("Parsed JSON: %s\n", json_object_to_json_string(json));
    }
    return cf;
}

memoryStruct getHttpRequest(char *url) {
    CURL *curl;
    CURLcode res;

    memoryStruct chunk;

    chunk.memory = malloc(1); /* will be grown as needed by the realloc above */
    chunk.size = 0; /* no data at this point */

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        /* example.com is redirected, so we tell libcurl to follow redirection */
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        /* send all data to this function  */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &chunk);

        //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 100000);
        //curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, long speedlimit); 

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);

        /* Check for errors */
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
        }

        /* always cleanup */
        curl_easy_cleanup(curl);

    }
    return chunk;
}
