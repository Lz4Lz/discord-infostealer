#include "json.h"

void jsonFindValues(const char** key, JsonObject jsonObj, size_t arrSize, UserData* userData, int userCount)
{
    for (size_t i = 0; i < jsonObj.size; i++)
    {
        for (size_t j = 0; j < arrSize; j++)
        {
            if (strcmp(key[j], jsonObj.pairs[i].key) == 0)
            {
                //printf("%s: %s\n", jsonObj.pairs[i].key, jsonObj.pairs[i].value);
                if (strcmp(jsonObj.pairs[i].key, "id") == 0)
                {
                    strcpy_s(userData[userCount].id, sizeof(userData->id), jsonObj.pairs[i].value);
                }
                else if (strcmp(jsonObj.pairs[i].key, "username") == 0)
                {
                    strcpy_s(userData[userCount].username, sizeof(userData->username), jsonObj.pairs[i].value);
                }
                else if (strcmp(jsonObj.pairs[i].key, "email") == 0)
                {
                    strcpy_s(userData[userCount].email, sizeof(userData->email), jsonObj.pairs[i].value);
                }
                else if (strcmp(jsonObj.pairs[i].key, "phone") == 0)
                {
                    strcpy_s(userData[userCount].phone, sizeof(userData->phone), jsonObj.pairs[i].value);
                }

            }
        }
    }
    for (size_t i = 0; i < jsonObj.size; i++)
    {
        free(jsonObj.pairs[i].key);
        free(jsonObj.pairs[i].value);
    }
    free(jsonObj.pairs);
}

JsonObject parse_json(const char* json) {
    JsonObject result;
    result.size = 0;
    result.pairs = NULL;

    // Skip leading whitespace
    while (*json != '\0' && (*json == ' ' || *json == '\t' || *json == '\n' || *json == '\r')) {
        ++json;
    }

    // Expect an opening brace
    if (*json != '{') {
        printf("Error: Expected '{'\n");
        return result;
    }
    ++json;

    // Parse key-value pairs
    while (*json != '}' && *json != '\0') {
        // Skip leading whitespace within the object
        while (*json != '}' && (*json == ' ' || *json == '\t' || *json == '\n' || *json == '\r')) {
            ++json;
        }

        // Parse key
        if (*json != '"') {
            printf("Error: Expected '\"' for key\n");
            break;
        }
        ++json;  // Skip the opening quote
        const char* key_start = json;

        while (*json != '"' && *json != '\0') {
            ++json;
        }

        if (*json != '"') {
            printf("Error: Expected '\"' for key\n");
            break;
        }

        char* key = malloc(json - key_start + 1);
        strncpy_s(key, json - key_start + 1, key_start, json - key_start);

        key[json - key_start] = '\0';
        ++json;  // Skip the closing quote

        // Skip whitespace after the key
        while (*json != '}' && (*json == ' ' || *json == '\t' || *json == '\n' || *json == '\r')) {
            ++json;
        }

        // Expect a colon
        if (*json != ':') {
            printf("Error: Expected ':' after key\n");
            free(key);
            break;
        }
        ++json;

        // Skip whitespace after the colon
        while (*json != '}' && (*json == ' ' || *json == '\t' || *json == '\n' || *json == '\r')) {
            ++json;
        }

        // Parse value
        char* value;
        if (*json == '"') {
            // Parse string value
            ++json;  // Skip the opening quote
            const char* value_start = json;
            while (*json != '"' && *json != '\0') {
                ++json;
            }
            if (*json != '"') {
                printf("Error: Expected '\"' for string value\n");
                free(key);
                break;
            }
            value = malloc(json - value_start + 1);
            strncpy_s(value, json - value_start + 1, value_start, json - value_start);

            value[json - value_start] = '\0';
            ++json;  // Skip the closing quote
        }
        else {
            // Parse non-string value
            const char* value_start = json;
            while (*json != ',' && *json != '}' && *json != '\0') {
                ++json;
            }
            value = malloc(json - value_start + 1);
            strncpy_s(value, json - value_start + 1, value_start, json - value_start);

            value[json - value_start] = '\0';
        }

        // Add key-value pair to the result
        result.pairs = realloc(result.pairs, (result.size + 1) * sizeof(KeyValuePair));
        if (result.pairs == NULL) {
            // Handle memory allocation failure
            printf("Error: Memory allocation failed\n");
            free(key);
            free(value);
            break;
        }
        result.pairs[result.size].key = key;
        result.pairs[result.size].value = value;
        ++result.size;

        // Skip whitespace and check for more pairs or the end of the object
        while (*json != '}' && *json != '\0') {
            if (*json == ',') {
                ++json;
                break;  // More pairs to come
            }
            else if (*json == '}') {
                ++json;
                break;  // End of the object
            }
            else if (*json == ' ' || *json == '\t' || *json == '\n' || *json == '\r') {
                ++json;
            }
            else {
                printf("Error: Unexpected character '%c'\n", *json);
                free(value);
                break;
            }
        }
    }

    // Expect a closing brace
    if (*json != '}') {
        printf("Error: Expected '}'\n");
    }
    else {
        ++json;
    }

    return result;
}