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
bool IsNowComboDestinationCommand(const char* command);
bool IsNowComboRelativeField(const char* command, const char* fieldName);
const char* NowComboCoordinateHelp();

bool ApplyMouseCenteredWheelZoom(float& scale, float minimum, float maximum,
    const ImVec2& canvasOrigin, const ImVec2& canvasSize);
bool BeginSharpMagnifiedCanvas(float scale);
void EndSharpMagnifiedCanvas(bool sharp);
// Fit the entire skin into the available canvas without upscaling small skins.
float CalculatePreviewFitScale(float skinWidth, float skinHeight, const ImVec2& available);

std::string FormatObjectConditionHeader(SKINFILELINEREAD& row);
// Browser search input is UTF-8; object names and source rows remain CP932.
bool ObjectBrowserTextMatchesSearch(const char* cp932Text, const char* utf8Query);
void AssignRootFileOwner(ARR& skinfileLines, const char* fallback,
    CSTR& owner);
int FindOwnerFileEndRow(ARR& skinfileLines, const char* owner);
