#pragma once

#include "imgui/imgui.h"

#include <string>

struct SKINFILELINEREAD;
struct ARR;
class CSTR;

bool DrawCommandValueCombo(const char* label, const char* command,
    const char* columnHelp, int current, int& selected);
bool ResolveDstArgbColumns(const char* command, int columns[4]);
int FindCommandFieldColumn(const char* command, const char* fieldName);

bool ApplyMouseCenteredWheelZoom(float& scale, float minimum, float maximum,
    const ImVec2& canvasOrigin, const ImVec2& canvasSize);
bool BeginSharpMagnifiedCanvas(float scale);
void EndSharpMagnifiedCanvas(bool sharp);

std::string FormatObjectConditionHeader(SKINFILELINEREAD& row);
void AssignRootFileOwner(ARR& skinfileLines, const char* fallback,
    CSTR& owner);
int FindOwnerFileEndRow(ARR& skinfileLines, const char* owner);
