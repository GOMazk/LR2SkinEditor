#pragma once
#include <string>
#include "arr.hpp"
#include "../LR2/strclass.h"

extern ARR arr_CommandHelp;

int LoadCommandHelp(const char* file);
bool LoadEmbeddedTextResource(int resourceId, std::string& text);
CSTR GetCommandHelp(const char* command, int column);
