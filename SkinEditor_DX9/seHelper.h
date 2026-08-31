#pragma once
#include <string>
#include "arr.hpp"
#include "../LR2/strclass.h"

extern ARR arr_CommandHelp;

enum SECommandValueKind {
	SE_VALUE_NONE = 0,
	SE_VALUE_SKIN_TYPE,
	SE_VALUE_OPTION,
	SE_VALUE_TEXT,
	SE_VALUE_NUMBER,
	SE_VALUE_TIMER,
	SE_VALUE_BUTTON,
	SE_VALUE_SLIDER,
	SE_VALUE_BARGRAPH
};

int LoadCommandHelp(const char* file);
bool LoadEmbeddedTextResource(int resourceId, std::string& text);
CSTR GetCommandHelp(const char* command, int column);
SECommandValueKind GetCommandValueKind(const char* command, const char* columnHelp);
const char* GetCommandValueName(SECommandValueKind kind, int value);
int GetCommandValueItemCount(SECommandValueKind kind);
int GetCommandValueAt(SECommandValueKind kind, int item);
