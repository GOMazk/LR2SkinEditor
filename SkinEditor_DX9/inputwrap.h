#pragma once
#include <imgui.h>
#include "../LR2/strclass.h"



int CstrResizeCallback(ImGuiInputTextCallbackData* data);
bool CstrInputText(const char* label, CSTR* my_str, ImGuiInputTextFlags flags = 0);
int InputTextExample();