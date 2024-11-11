#pragma once
#include <iostream>
#include <regex>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <map>

#include <thread>
#include <filesystem>
#include <Windows.h>
#include <wincrypt.h>

#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/ssl.h>

#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "user32.lib")

std::vector<uint8_t> base64DecodeToBytes(const std::string& base64String);

std::string decryptValue(const std::vector<uint8_t>& buffer, const std::vector<uint8_t>& master_key);

std::vector<uint8_t> get_master_key(const std::string& path);