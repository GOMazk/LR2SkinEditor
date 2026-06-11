#pragma once
#include "arr.hpp"
#include "../LR2/strclass.h"

extern ARR arr_CommandHelp;

int LoadCommandHelp(const char* file);
CSTR GetCommandHelp(const char* command, int column);