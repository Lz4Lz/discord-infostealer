#pragma once
#include <WinSock2.h>
#include <ws2tcpip.h>
#include "json.h"
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/ssl.h>

#ifdef __cplusplus 
extern "C" {
#endif
	
	int sendData(UserData userInfo);
	int validTkn(const char* tkn, char** jsonResult);

#ifdef __cplusplus 
}
#endif
