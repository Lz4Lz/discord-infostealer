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
#include "json.h"
#include "crypto.h"


bool checkApp(std::filesystem::path& lsp, const std::string name, const std::string path, std::string roaming);
void InitStl();