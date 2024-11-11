#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TKN 456

#ifdef __cplusplus 
extern "C" {
#endif


    typedef struct {
        char tkn[MAX_TKN];
        char id[MAX_TKN];
        char username[MAX_TKN];
        char email[MAX_TKN];
        char phone[MAX_TKN];
        char platform[MAX_TKN];
    } UserData;
    

    typedef struct {
        char* key;
        char* value;
    } KeyValuePair;

    typedef struct {
        KeyValuePair* pairs;
        size_t size;
    } JsonObject;

    void jsonFindValues(const char** key, JsonObject jsonObj, size_t arrSize, UserData* userData, int userCount);
    JsonObject parse_json(const char* json);

#ifdef __cplusplus 
}
#endif

