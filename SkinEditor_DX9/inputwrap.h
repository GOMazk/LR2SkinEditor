#pragma once
#include <imgui.h>
#include "../LR2/strclass.h"
#include <string>



int CstrResizeCallback(ImGuiInputTextCallbackData* data);
bool CstrInputText(const char* label, CSTR* my_str, ImGuiInputTextFlags flags = 0);
std::string Cp932ToUtf8(const char* text);
std::string Utf8ToCp932(const char* text);
int InputTextExample();
