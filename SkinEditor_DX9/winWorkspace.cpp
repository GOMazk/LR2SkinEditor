#include "imgui/imgui.h"

#include "../LR2/structure.h"
#include "../LR2/LR2_skinmanage.h"
#include "../LR2/LR2_skinload.h"
#include "../LR2/LR2_skindraw.h"
#include "../LR2/LR2_skinobject.h"
#include "../LR2/LR2_configsave.h"
#include "../LR2/En_fileutil.h"
#include "../LR2/Scene07_Skinselect.h"
#include "../LR2/En_timer.h"
#include "../LR2/En_value.h" //for ByTime

#include "../lib/DxLib/DxLib.h"
#include "winWorkspace.h"

#include "skin.h"
#include "op.h"
#include "arr.hpp"
#include "seHelper.h"
#include "seUI.h"
#include "inputwrap.h"
#include "imgui/imgui_internal.h"
#include <algorithm>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <fstream>
#include <map>
#include <sstream>
#include <shellapi.h>
#include <commdlg.h>
#include <string>
#include <vector>

#pragma comment(lib, "Comdlg32.lib")

static void WriteSkinLoadLog(const char* stage, const char* detail = NULL) {
    FILE* fp = fopen("SkinEditor_load_crash.log", "a");
    if (!fp) return;
    fprintf(fp, "%s%s%s\n", stage ? stage : "(null)", detail ? " : " : "", detail ? detail : "");
    fflush(fp);
    fclose(fp);
}

static bool SkinPathExists(const char* path) {
    if (!path || !*path) return false;
    WIN32_FIND_DATAA findData{};
    HANDLE find = FindFirstFileA(path, &findData);
    if (find == INVALID_HANDLE_VALUE) return false;
    FindClose(find);
    return true;
}

// Some distributed skins were renamed after authoring (for example
// LITONE5_PLAYAC -> LITONE5) while their internal paths kept the old theme
// folder. Resolve the same Play-relative asset inside the folder containing
// the opened skin without modifying the source text saved by the editor.
static bool ResolveSiblingPlayPath(const char* requested, const char* mainSkinPath,
    char* resolved, size_t resolvedSize) {
    if (!requested || !mainSkinPath || !resolved || resolvedSize == 0) return false;
    if (SkinPathExists(requested)) return false;

    std::string request = requested;
    std::string mainPath = mainSkinPath;
    std::replace(request.begin(), request.end(), '/', '\\');
    std::replace(mainPath.begin(), mainPath.end(), '/', '\\');
    const size_t requestPlay = request.find("\\Play\\");
    const size_t mainPlay = mainPath.find("\\Play\\");
    if (requestPlay == std::string::npos || mainPlay == std::string::npos) return false;

    const std::string candidate = mainPath.substr(0, mainPlay) + request.substr(requestPlay);
    if (!SkinPathExists(candidate.c_str())) return false;
    strncpy_s(resolved, resolvedSize, candidate.c_str(), _TRUNCATE);
    return true;
}

static bool SaveResolutionToSkinFile(const char* path, int width, int height);

static bool BrowseSkinSavePath(const char* initialPath, char* selectedPath,
    size_t selectedPathSize) {
    if (!selectedPath || selectedPathSize == 0) return false;
    char path[MAX_PATH] = {};
    if (initialPath && *initialPath)
        strncpy_s(path, initialPath, _TRUNCATE);

    OPENFILENAMEA dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = GetActiveWindow();
    dialog.lpstrFilter = "LR2 skin script (*.lr2skin)\0*.lr2skin\0All files (*.*)\0*.*\0\0";
    dialog.lpstrFile = path;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrDefExt = "lr2skin";
    dialog.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR |
        OFN_OVERWRITEPROMPT;
    if (!GetSaveFileNameA(&dialog)) return false;
    strncpy_s(selectedPath, selectedPathSize, path, _TRUNCATE);
    return true;
}

const char* SKINTYPESTR[]= {
    "7KEYS",
    "5KEYS",
    "14KEYS",
    "10KEYS",
    "9KEYS",
    "SELECT",
    "DECIDE",
    "RESULT",
    "KEYCONFIG",
    "SKINSELECT",
    "SOUNDSET",
    "THEME",
    "7KEYSBATTLE",
    "5KEYSBATTLE",
    "9KEYSBATTLE",
    "COURSERESULT",
    "OPENING",
    "MODESELECT",
    "MODEDECIDE",
    "COURSESELECT",
    "COURSEEDIT"
}; 

// Draw every symbolic CSV value through the same command-aware resolver.
// $type means scene type only for #INFORMATION; BUTTON, SLIDER and BARGRAPH
// each own a different LR2 type namespace.
static bool ContainsAsciiInsensitive(const std::string& text,
    const std::string& query) {
    if (query.empty()) return true;
    return std::search(text.begin(), text.end(), query.begin(), query.end(),
        [](unsigned char left, unsigned char right) {
            return std::tolower(left) == std::tolower(right);
        }) != text.end();
}

static bool DrawCommandValueCombo(const char* label, const char* command,
    const char* columnHelp, int current, int& selected) {
    const SECommandValueKind kind = GetCommandValueKind(command, columnHelp);
    const int itemCount = GetCommandValueItemCount(kind);
    if (kind == SE_VALUE_NONE || itemCount <= 0) return false;

    selected = current;
    const bool isOption = kind == SE_VALUE_OPTION;
    const char* currentName = GetCommandValueName(kind, current);
    const std::string currentNameUtf8 = Cp932ToUtf8(currentName ? currentName : "");
    char preview[160];
    if (isOption && current < 0)
        snprintf(preview, sizeof(preview), "NOT %03d:%s", -current, currentNameUtf8.c_str());
    else if (!currentNameUtf8.empty())
        snprintf(preview, sizeof(preview), "%03d:%s", current, currentNameUtf8.c_str());
    else
        snprintf(preview, sizeof(preview), "%03d", current);

    const ImGuiID comboId = ImGui::GetID(label);
    if (ImGui::BeginCombo(label, preview)) {
        static std::map<ImGuiID, std::string> filters;
        std::string& filter = filters[comboId];
        char filterBuffer[128] = {};
        strncpy_s(filterBuffer, filter.c_str(), _TRUNCATE);
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::InputTextWithHint("##SymbolFilter", "Search id or name...",
            filterBuffer, IM_ARRAYSIZE(filterBuffer)))
            filter.assign(filterBuffer);
        ImGui::Separator();

        std::vector<int> filteredItems;
        filteredItems.reserve(itemCount);
        for (int item = 0; item < itemCount; ++item) {
            const int value = GetCommandValueAt(kind, item);
            const char* valueName = GetCommandValueName(kind, value);
            const std::string valueNameUtf8 = Cp932ToUtf8(valueName ? valueName : "");
            char candidate[192];
            if (isOption && value < 0)
                snprintf(candidate, sizeof(candidate), "NOT %03d:%s", -value, valueNameUtf8.c_str());
            else
                snprintf(candidate, sizeof(candidate), "%03d:%s", value, valueNameUtf8.c_str());
            if (ContainsAsciiInsensitive(candidate, filter)) filteredItems.push_back(item);
        }

        ImGuiListClipper clipper;
        clipper.Begin((int)filteredItems.size());
        while (clipper.Step()) {
            for (int filtered = clipper.DisplayStart; filtered < clipper.DisplayEnd; ++filtered) {
                const int item = filteredItems[filtered];
                const int value = GetCommandValueAt(kind, item);
                const char* valueName = GetCommandValueName(kind, value);
                const std::string valueNameUtf8 = Cp932ToUtf8(valueName ? valueName : "");
                char option[160];
                if (isOption && value < 0)
                    snprintf(option, sizeof(option), "NOT %03d:%s", -value, valueNameUtf8.c_str());
                else if (!valueNameUtf8.empty())
                    snprintf(option, sizeof(option), "%03d:%s", value, valueNameUtf8.c_str());
                else
                    snprintf(option, sizeof(option), "%03d", value);
                ImGui::PushID(item);
                if (ImGui::Selectable(option, current == value)) {
                    selected = value;
                    filter.clear();
                }
                if (current == value) ImGui::SetItemDefaultFocus();
                ImGui::PopID();
            }
        }
        ImGui::EndCombo();
    }
    return true;
}

static bool AssignCommandField(CSVbuf& values, const char* command,
    const char* fieldName, int value) {
    if (!command || !fieldName) return false;
    for (int column = 1; column < 30; ++column) {
        CSTR help = GetCommandHelp(command, column);
        help.trimWhiteSpace();
        const char* label = help.body ? help.outstr() : "";
        if (*label == '$') ++label;
        if (_stricmp(label, fieldName) != 0) continue;
        char text[32];
        snprintf(text, sizeof(text), "%d", value);
        values.str[column].assign(text);
        return true;
    }
    return false;
}

static bool ReadCommandField(CSVbuf& values, const char* command,
    const char* fieldName, int& value) {
    if (!command || !fieldName) return false;
    for (int column = 1; column < 30; ++column) {
        CSTR help = GetCommandHelp(command, column);
        help.trimWhiteSpace();
        const char* label = help.body ? help.outstr() : "";
        if (*label == '$') ++label;
        if (_stricmp(label, fieldName) != 0) continue;
        value = values.str[column].body ? atol(values.str[column].outstr()) : 0;
        return true;
    }
    return false;
}

static bool IsAssetBackedObjectCommand(const char* command) {
    return command && (!strcmp(command, "#SRC_IMAGE") ||
        !strcmp(command, "#SRC_NUMBER") ||
        !strcmp(command, "#SRC_SLIDER") ||
        !strcmp(command, "#SRC_BUTTON"));
}

static const char* AssetBackedObjectTypeName(const char* command) {
    if (!command) return "Unknown";
    if (!strcmp(command, "#SRC_IMAGE")) return "Image";
    if (!strcmp(command, "#SRC_NUMBER")) return "Number";
    if (!strcmp(command, "#SRC_SLIDER")) return "Slider";
    if (!strcmp(command, "#SRC_BUTTON")) return "Button";
    return command;
}

static void InitializeAssetBackedSource(CSVbuf& values, const char* command,
    const IMG& asset, int divX, int divY, int cycle, int timer) {
    if (!IsAssetBackedObjectCommand(command)) return;
    AssignCommandField(values, command, "(NULL)", 0);
    AssignCommandField(values, command, "index", 0);
    AssignCommandField(values, command, "gr", asset.gr);
    AssignCommandField(values, command, "x", asset.x);
    AssignCommandField(values, command, "y", asset.y);
    AssignCommandField(values, command, "w", asset.w);
    AssignCommandField(values, command, "h", asset.h);
    AssignCommandField(values, command, "div_x", divX);
    AssignCommandField(values, command, "div_y", divY);
    AssignCommandField(values, command, "cycle", cycle);
    AssignCommandField(values, command, "timer", timer);

    // Command-specific values remain editable through their symbolic combos.
    AssignCommandField(values, command, "op1", 0);
    AssignCommandField(values, command, "op2", 0);
    AssignCommandField(values, command, "op3", 0);
    AssignCommandField(values, command, "num", 0);
    AssignCommandField(values, command, "align(0right1left2middle)", 0);
    AssignCommandField(values, command, "keta", 1);
    AssignCommandField(values, command, "muki", 0);
    AssignCommandField(values, command, "range", 0);
    AssignCommandField(values, command, "type", 0);
    AssignCommandField(values, command, "disable", 0);
    AssignCommandField(values, command, "click", 0);
    AssignCommandField(values, command, "panel", 0);
}

// Loaded scripts are wrapped by in-memory $FILE start/end markers.  Rows
// outside those markers are intentionally ignored by the Preview runtime and
// are not owned by a file when saved.  New top-level commands therefore belong
// immediately before the matching owner's end marker, never after the entire
// flattened include tree.
static bool IsSameOwnerPath(const char* left, const char* right) {
    if (!left || !right) return false;
    char fullLeft[MAX_PATH] = {};
    char fullRight[MAX_PATH] = {};
    const char* compareLeft = left;
    const char* compareRight = right;
    if (GetFullPathNameA(left, MAX_PATH, fullLeft, NULL)) compareLeft = fullLeft;
    if (GetFullPathNameA(right, MAX_PATH, fullRight, NULL)) compareRight = fullRight;
    return _stricmp(compareLeft, compareRight) == 0;
}

static void AssignRootFileOwner(ARR& skinfileLines, const char* fallback,
    CSTR& owner) {
    owner.assign(fallback && *fallback ? fallback : "");
    for (int row = 0; row < skinfileLines.count; ++row) {
        SKINFILELINEREAD& candidate =
            ((SKINFILELINEREAD*)skinfileLines.data)[row];
        const char* text = candidate.line.body ? candidate.line.outstr() : "";
        if (!candidate.isSEcomment || strncmp(text, "$FILE ", 6) != 0 ||
            strstr(text, " start") == NULL)
            continue;
        if (candidate.filename.body && *candidate.filename.outstr())
            owner.assign(candidate.filename.outstr());
        return;
    }
}

static int FindOwnerFileEndRow(ARR& skinfileLines, const char* owner) {
    if (!owner || !*owner) return skinfileLines.count;
    for (int row = skinfileLines.count - 1; row >= 0; --row) {
        SKINFILELINEREAD& candidate =
            ((SKINFILELINEREAD*)skinfileLines.data)[row];
        if (!candidate.filename.body ||
            !IsSameOwnerPath(candidate.filename.outstr(), owner) ||
            !candidate.line.body || !candidate.isSEcomment)
            continue;
        const char* text = candidate.line.outstr();
        if (!strncmp(text, "$FILE ", 6) && strstr(text, " end") != NULL)
            return row;
    }
    return skinfileLines.count;
}

static int CalculateTrailingGraphicId(ARR& skinfileLines) {
    struct GraphicConditionFrame {
        int beforeBlock;
        int maximumBranchEnd;
    };
    std::vector<GraphicConditionFrame> stack;
    int graphicId = 0;
    for (int row = 0; row < skinfileLines.count; ++row) {
        SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
        if (line.isComment || !line.csv.str[0].body) continue;
        if (line.csv.str[0].isSame("#IF")) {
            stack.push_back({ graphicId, graphicId });
        } else if (line.csv.str[0].isSame("#ELSEIF") ||
            line.csv.str[0].isSame("#ELSE")) {
            if (!stack.empty()) {
                GraphicConditionFrame& frame = stack.back();
                frame.maximumBranchEnd = (std::max)(frame.maximumBranchEnd,
                    graphicId);
                graphicId = frame.beforeBlock;
            }
        } else if (line.csv.str[0].isSame("#ENDIF")) {
            if (!stack.empty()) {
                GraphicConditionFrame frame = stack.back();
                stack.pop_back();
                frame.maximumBranchEnd = (std::max)(frame.maximumBranchEnd,
                    graphicId);
                graphicId = frame.maximumBranchEnd;
            }
        } else if (line.csv.str[0].isSame("#IMAGE")) {
            ++graphicId;
        }
    }
    return graphicId;
}

static std::string ResolveGeneratedImageDiskPath(const char* enteredPath,
    const char* mainSkinPath) {
    if (!enteredPath || !*enteredPath) return std::string();
    std::error_code error;
    std::filesystem::path output(enteredPath);
    if (output.is_relative()) {
        std::filesystem::path base(mainSkinPath && *mainSkinPath
            ? mainSkinPath : ".");
        if (base.has_filename()) base = base.parent_path();
        output = base / output;
    }
    output = std::filesystem::absolute(output, error).lexically_normal();
    return error ? std::string() : output.string();
}

static std::string MakePortableGeneratedImagePath(const char* diskPath,
    const char* mainSkinPath) {
    if (!diskPath || !*diskPath || !mainSkinPath || !*mainSkinPath)
        return diskPath ? diskPath : "";
    std::error_code error;
    std::filesystem::path root = std::filesystem::absolute(
        std::filesystem::path(mainSkinPath), error).parent_path();
    if (error) return diskPath;
    std::filesystem::path output = std::filesystem::absolute(
        std::filesystem::path(diskPath), error);
    if (error) return diskPath;
    std::filesystem::path relative = std::filesystem::relative(output, root,
        error);
    if (error || relative.empty()) return diskPath;
    const std::string relativeText = relative.string();
    if (relativeText == ".." || relativeText.rfind("..\\", 0) == 0 ||
        relativeText.rfind("../", 0) == 0) return diskPath;
    return std::string(".\\") + relativeText;
}

static std::string MakeUniqueGeneratedImagePath(const char* selectedImagePath,
    const char* mainSkinPath, const char* stem) {
    std::filesystem::path folder;
    if (selectedImagePath && *selectedImagePath)
        folder = std::filesystem::path(selectedImagePath).parent_path();
    if (folder.empty() && mainSkinPath && *mainSkinPath)
        folder = std::filesystem::path(mainSkinPath).parent_path();
    if (folder.empty()) folder = ".";
    const std::string baseStem = stem && *stem ? stem : "new_image";
    for (int suffix = 0; suffix < 10000; ++suffix) {
        std::string filename = baseStem;
        if (suffix > 0) filename += "_" + std::to_string(suffix);
        filename += ".png";
        const std::filesystem::path candidate = folder / filename;
        if (GetFileAttributesA(candidate.string().c_str()) ==
            INVALID_FILE_ATTRIBUTES) return candidate.string();
    }
    return (folder / (baseStem + ".png")).string();
}

// Change a canvas scale while keeping the image point below the mouse at the
// same screen position. This must be called from the scrolling window/child
// that directly owns the canvas.
static bool ApplyMouseCenteredWheelZoom(float& scale, float minimum, float maximum,
    const ImVec2& canvasOrigin, const ImVec2& canvasSize) {
    ImGuiIO& io = ImGui::GetIO();
    if (!io.KeyCtrl || io.MouseWheel == 0.0f ||
        !ImGui::IsWindowHovered(ImGuiHoveredFlags_None) ||
        !ImGui::IsMouseHoveringRect(canvasOrigin,
            ImVec2(canvasOrigin.x + canvasSize.x, canvasOrigin.y + canvasSize.y), true))
        return false;

    const float oldScale = scale;
    float newScale = oldScale * powf(1.15f, io.MouseWheel);
    if (newScale < minimum) newScale = minimum;
    if (newScale > maximum) newScale = maximum;
    io.MouseWheel = 0.0f;
    if (newScale == oldScale) return false;

    const float ratio = newScale / oldScale;
    const ImVec2 mouse = io.MousePos;
    const float newScrollX = ImGui::GetScrollX() +
        (mouse.x - canvasOrigin.x) * (ratio - 1.0f);
    const float newScrollY = ImGui::GetScrollY() +
        (mouse.y - canvasOrigin.y) * (ratio - 1.0f);
    scale = newScale;
    ImGui::SetScrollX(newScrollX);
    ImGui::SetScrollY(newScrollY);
    return true;
}

// ImGui's DX9 backend uses linear filtering for every texture. Keep that for
// reduced canvases, but use point sampling while magnified so source pixels do
// not get blended into a blurry image. The callbacks bracket only the canvas
// draw commands, leaving fonts, icons and the rest of the UI unchanged.
static void SetCanvasPointSampling(const ImDrawList*, const ImDrawCmd*) {
    if (!g_pd3dDevice) return;
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
}

static void SetCanvasLinearSampling(const ImDrawList*, const ImDrawCmd*) {
    if (!g_pd3dDevice) return;
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
}

static bool BeginSharpMagnifiedCanvas(float scale) {
    if (scale <= 1.0f) return false;
    ImGui::GetWindowDrawList()->AddCallback(SetCanvasPointSampling, NULL);
    return true;
}

static void EndSharpMagnifiedCanvas(bool sharp) {
    if (sharp)
        ImGui::GetWindowDrawList()->AddCallback(SetCanvasLinearSampling, NULL);
}

///////////////////
byte TexTransparent[] =    "\x42\x4D\x76\x02\x00\x00\x00\x00\x00\x00\x76\x00\x00\x00\x28\x00\x00\x00\x20\x00\x00\x00\x20\x00\x00\x00\x01\x00\x04\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x00\x00\x00\x80\x80\x00\x80\x00\x00\x00\x80\x00\x80\x00\x80\x80\x00\x00\x80\x80\x80\x00\xC0\xC0\xC0\x00\x00\x00\xFF\x00\x00\xFF\x00\x00\x00\xFF\xFF\x00\xFF\x00\x00\x00\xFF\x00\xFF\x00\xFF\xFF\x00\x00\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";

PDIRECT3DTEXTURE9 transBackground;
int makeTransBackground(){
    int x, y;
    LoadTextureFromMemory(TexTransparent, sizeof(TexTransparent), &transBackground, &x, &y);
    //LoadTextureFromRawMemory(TexTransparent, renderer, &transBackground, 64, 64, 1);
    return 0;
}


///////////////////
int WORKSPACE::proc() {

    return 0;
}
int WORKSPACE::init() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    zoom = 1.0f;
    ImageManagerZoom = 0.0f;
    imagePixelPaintMode = false;
    imagePixelPaintLastX = -1;
    imagePixelPaintLastY = -1;
    imagePixelPaintLastButton = -1;
    imagePixelPaintDirtyPaths.clear();
    imagePixelPaintStatus.clear();
    imageNewDialogRequested = false;
    imageMergeDialogRequested = false;
    imageToolOutputPathUtf8[0] = '\0';
    imageToolStatus.clear();
    imageManagerGeneratedGrFocusRequest = -1;
    DstViewZoom = 0.0f;
    assetThumbnailSize = 96.0f;
    assetAnimateSrc = true;
    assetSearch[0] = '\0';
    assetBrowserFocusRequest = -1;
    imageManagerFocusRequest = -1;
    newObjectCsvInitialized = false;
    newObjectInitializedCommand = -1;
    newObjectAssetIndex = -1;
    newObjectFocusRequest = false;
    newObjectOwner.assign("");

    initFlag = 1;
    return 0;
}
int WORKSPACE::draw() {
    if (initFlag == 0) init();
    if (loaded && pendingHistorySnapshotRestore >= 0) {
        const int snapshotIndex = pendingHistorySnapshotRestore;
        pendingHistorySnapshotRestore = -1;
        if (snapshotIndex < (int)historyDocumentSnapshots.size())
            RestoreDocumentSnapshot(historyDocumentSnapshots[snapshotIndex]);
    }
    // A drop is queued by Object Browser after it has already submitted this
    // frame. Apply it at the next frame boundary, before any texture-backed
    // window can observe stale row indices.
    if (loaded && pendingObjectReorder) ApplyPendingObjectReorder();
    const unsigned long long editorNow = GetTickCount64();
    const bool editorRebuildReady = editorDerivedRebuildRequestedAt == 0 ||
        editorNow - editorDerivedRebuildRequestedAt >= 80;
    if (loaded && editorDerivedRebuildPending && editorRebuildReady) {
        // This runs before any window submits image draw commands, so releasing
        // and lazily recreating editor textures cannot leave stale ImGui texture
        // pointers in the current frame's draw lists.
        editorDerivedRebuildPending = false;
        if (RebuildEditorDerivedState() == 0) {
            RebuildObjectModel();
            RefreshPreviewSelectionBounds();
        }
    } else if (loaded && objectModelRebuildPending && !editorDerivedRebuildPending) {
        RebuildObjectModel();
        RefreshPreviewSelectionBounds();
    }
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
    ImGui::SetNextWindowViewport(viewport->ID);
    const ImGuiWindowFlags workspaceFlags = ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin(title, &alive, workspaceFlags);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", NULL, &wNewskin)) {
                //
            }
            if (ImGui::MenuItem("Open", NULL, &wSkinList)) {
                ScanSkins();
            }
            if (loaded) {
                if (ImGui::MenuItem("Save", "Ctrl+S")) SaveCurrentSkin();
                if (ImGui::MenuItem("Save as", "Ctrl+Shift+S")) {
                    newPath[0] = '\0';
                    wSaveMenu = true;
                }
                ImGui::MenuItem("Export", NULL, &wSaveMenu2); //todo
            }
            ImGui::EndMenu();
        }
        if (loaded) {
            
            if (ImGui::BeginMenu("Windows")) {
                struct WindowToggle { const char* label; bool* visible; };
                const WindowToggle tools[] = {
                    { "Preview", &wPreview }, { "Customize", &wCustomize },
                    { "Image Manager", &wImgManager }, { "Asset Browser", &wAssetBrowser },
                    { "Text Editor", &wTextEdit },
                    { "File Manager", &wFileManager }, { "Simple Preview", &wSimplePreview },
                    { "DST View", &wDstView }, { "Object Browser", &wObjectBrowser },
                    { "Object Inspector", &wObjectInspector }, { "Object Manager", &wObjectManager },
                    { "Object Manager Test", &wObjectManagerTest }, { "Object Property", &wProperty },
                    { "Option List", &wOpList }, { "History", &wHistory }
                };
                for (const WindowToggle& tool : tools)
                    ImGui::MenuItem(tool.label, NULL, tool.visible);
                ImGui::Separator();
                if (ImGui::MenuItem("Reset default layout")) dockLayoutBuilt = false;
                ImGui::EndMenu();
            }
        }
        ImGui::EndMenuBar();
    }

    // The toolbar exposes common actions without removing their menu entries.
    // It only emits intent; existing WORKSPACE methods remain the single source
    // of truth for loading, saving and history behavior.
    if (SEUI::BeginToolbar("##WorkspaceToolbar")) {
        if (SEUI::ActionButton("New", "Create a skin from an initial preset")) wNewskin = true;
        ImGui::SameLine();
        if (SEUI::ActionButton("Open", "Open a skin registered in LR2")) {
            ScanSkins();
            wSkinList = true;
        }
        ImGui::SameLine();
        if (SEUI::ActionButton("Save", "Save changes to the current skin (Ctrl+S)", loaded))
            SaveCurrentSkin();
        ImGui::SameLine();
        if (SEUI::ActionButton("Save as", "Save a copy and switch the working path", loaded)) {
            newPath[0] = '\0';
            wSaveMenu = true;
        }
        ImGui::SameLine();
        if (SEUI::ActionButton("Undo", "Undo the last Object Editor change (Ctrl+Z)", loaded))
            UndoLastEdit();
        SEUI::ToolbarSeparator();
        char resolutionLabel[48];
        snprintf(resolutionLabel, sizeof(resolutionLabel), "%d x %d",
            loaded ? meta.targetX : 0, loaded ? meta.targetY : 0);
        if (SEUI::ActionButton(resolutionLabel,
            "Change the loaded workspace resolution", loaded)) {
            workspaceResolutionWidth = meta.targetX;
            workspaceResolutionHeight = meta.targetY;
            workspaceResolutionState = 0;
            workspaceResolutionMessage.clear();
            workspaceResolutionDialogRequested = true;
        }
        ImGui::SameLine();
        if (SEUI::ActionButton("Reset layout", "Restore the default editor panel arrangement", loaded))
            dockLayoutBuilt = false;
        ImGui::SameLine();
        SEUI::HelpMarker("All tools are still available from Windows. Add future common actions to this toolbar; keep their domain logic in WORKSPACE.");
        ImGui::SameLine(0.0f, 16.0f);
        const bool recentSaveFailure = lastSaveState < 0 &&
            GetTickCount64() - lastSaveMessageAt < 8000;
        const char* statusLabel = !loaded ? "NO SKIN" :
            recentSaveFailure ? "SAVE FAILED" :
            IsDocumentDirty() ? "MODIFIED" :
            !imagePixelPaintDirtyPaths.empty() ? "IMAGE EDIT" : "SAVED";
        const ImVec4 statusColor = !loaded ? SEUI::Colors::Muted() :
            recentSaveFailure ? SEUI::Colors::Danger() :
            (IsDocumentDirty() || !imagePixelPaintDirtyPaths.empty())
                ? SEUI::Colors::Warning() : SEUI::Colors::Success();
        SEUI::StatusPill(statusLabel, statusColor);
        if (!lastSaveMessage.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("%s", lastSaveMessage.c_str());
        if (loaded) {
            ImGui::SameLine(0.0f, 10.0f);
            ImGui::TextDisabled("%s  |  %d x %d  |  %d objects", SKINTYPESTR[meta.type],
                meta.targetX, meta.targetY, arr_seobj.count);
        }
    }
    SEUI::EndToolbar();

    char resolutionPopupTitle[64];
    snprintf(resolutionPopupTitle, sizeof(resolutionPopupTitle),
        "Workspace resolution##%d", num);
    if (workspaceResolutionDialogRequested) {
        ImGui::OpenPopup(resolutionPopupTitle);
        workspaceResolutionDialogRequested = false;
    }
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(),
        ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(resolutionPopupTitle, NULL,
        ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Canvas resolution");
        ImGui::TextDisabled("Object coordinates are kept as-is; they are not scaled.");
        ImGui::TextColored(SEUI::Colors::Warning(),
            "Apply writes #INFORMATION to the current skin immediately and reloads the workspace.");
        ImGui::SetNextItemWidth(240.0f);
        int resolution[2] = { workspaceResolutionWidth, workspaceResolutionHeight };
        if (ImGui::InputInt2("Width / Height", resolution)) {
            workspaceResolutionWidth = (std::max)(64, (std::min)(7680, resolution[0]));
            workspaceResolutionHeight = (std::max)(64, (std::min)(4320, resolution[1]));
        }

        struct WorkspaceResolutionPreset { const char* label; int width; int height; };
        const WorkspaceResolutionPreset presets[] = {
            { "640 x 480", 640, 480 },
            { "1280 x 720", 1280, 720 },
            { "1920 x 1080", 1920, 1080 }
        };
        for (int presetIndex = 0; presetIndex < IM_ARRAYSIZE(presets); ++presetIndex) {
            if (presetIndex > 0) ImGui::SameLine();
            if (ImGui::Button(presets[presetIndex].label)) {
                workspaceResolutionWidth = presets[presetIndex].width;
                workspaceResolutionHeight = presets[presetIndex].height;
            }
        }

        const bool scriptDirty = IsDocumentDirty();
        const bool imageDirty = !imagePixelPaintDirtyPaths.empty();
        if (scriptDirty)
            ImGui::TextColored(SEUI::Colors::Warning(),
                "Save the current script changes before changing resolution.");
        if (imageDirty)
            ImGui::TextColored(SEUI::Colors::Warning(),
                "Save or revert the pending pixel edits before reloading.");
        if (!workspaceResolutionMessage.empty())
            ImGui::TextColored(workspaceResolutionState < 0
                ? SEUI::Colors::Danger() : SEUI::Colors::Success(), "%s",
                workspaceResolutionMessage.c_str());

        if (scriptDirty) {
            if (ImGui::Button("Save changes")) SaveCurrentSkin();
            ImGui::SameLine();
        }
        ImGui::BeginDisabled(!loaded || scriptDirty || imageDirty);
        if (ImGui::Button("Apply", ImVec2(100.0f, 0.0f))) {
            const int requestedWidth = workspaceResolutionWidth;
            const int requestedHeight = workspaceResolutionHeight;
            if (requestedWidth == meta.targetX && requestedHeight == meta.targetY) {
                workspaceResolutionState = 1;
                workspaceResolutionMessage = "Resolution is unchanged.";
                ImGui::CloseCurrentPopup();
            } else if (!SaveResolutionToSkinFile(mainpath,
                requestedWidth, requestedHeight)) {
                workspaceResolutionState = -1;
                workspaceResolutionMessage = "Failed to update #INFORMATION; the original file was preserved.";
            } else {
                meta.targetX = requestedWidth;
                meta.targetY = requestedHeight;
                loaded = (LoadSkin(mainpath) == 0);
                workspaceResolutionState = loaded ? 1 : -1;
                workspaceResolutionMessage = loaded
                    ? "Resolution saved and workspace reloaded."
                    : "Resolution was saved, but the workspace could not be reloaded.";
                if (loaded) ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100.0f, 0.0f)))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (loaded) {
    char dockName[64];
    snprintf(dockName, sizeof(dockName), "WorkspaceDock##%d", num);
    const ImGuiID dockspaceId = ImGui::GetID(dockName);
    ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    if (!dockLayoutBuilt) {
        ImGui::DockBuilderRemoveNode(dockspaceId);
        ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);
        // Default workspace, from left to right:
        // Object Browser | Object Inspector | Preview tools | OpList.
        // Asset Browser stays below Preview for drag/drop. Customize returns
        // to the compact right column below OpList.
        ImGuiID objectBrowserDock = 0;
        ImGuiID remainingDock = dockspaceId;
        ImGui::DockBuilderSplitNode(remainingDock, ImGuiDir_Left, 0.19f,
            &objectBrowserDock, &remainingDock);

        ImGuiID objectInspectorDock = 0;
        ImGui::DockBuilderSplitNode(remainingDock, ImGuiDir_Left, 0.235f,
            &objectInspectorDock, &remainingDock);

        ImGuiID opListDock = 0;
        ImGuiID centerDock = remainingDock;
        ImGui::DockBuilderSplitNode(remainingDock, ImGuiDir_Right, 0.21f,
            &opListDock, &centerDock);

        ImGuiID customizeDock = 0;
        ImGuiID opListUpperDock = opListDock;
        ImGui::DockBuilderSplitNode(opListDock, ImGuiDir_Down, 0.32f,
            &customizeDock, &opListUpperDock);
        opListDock = opListUpperDock;

        ImGuiID assetBrowserDock = 0;
        ImGuiID previewDock = centerDock;
        ImGui::DockBuilderSplitNode(centerDock, ImGuiDir_Down, 0.30f,
            &assetBrowserDock, &previewDock);

        char previewTitle[64];
        char objectBrowserTitle[64];
        char objectInspectorTitle[64];
        char imageManagerTitle[64];
        char assetBrowserTitle[64];
        char dstViewTitle[64];
        char opListTitle[64];
        char customizeTitle[64];
        snprintf(previewTitle, sizeof(previewTitle), "Preview##%d", num);
        snprintf(objectBrowserTitle, sizeof(objectBrowserTitle), "Object Browser##%d", num);
        snprintf(objectInspectorTitle, sizeof(objectInspectorTitle), "Object Inspector##%d", num);
        snprintf(imageManagerTitle, sizeof(imageManagerTitle), "ImageManager##%d", num);
        snprintf(assetBrowserTitle, sizeof(assetBrowserTitle), "Asset Browser##%d", num);
        snprintf(dstViewTitle, sizeof(dstViewTitle), "dstView##%d", num);
        snprintf(opListTitle, sizeof(opListTitle), "OpList##%d", num);
        snprintf(customizeTitle, sizeof(customizeTitle), "Customize##%d", num);
        ImGui::DockBuilderDockWindow(previewTitle, previewDock);
        ImGui::DockBuilderDockWindow(imageManagerTitle, previewDock);
        ImGui::DockBuilderDockWindow(assetBrowserTitle, assetBrowserDock);
        ImGui::DockBuilderDockWindow(dstViewTitle, previewDock);
        ImGui::DockBuilderDockWindow(objectBrowserTitle, objectBrowserDock);
        ImGui::DockBuilderDockWindow(objectInspectorTitle, objectInspectorDock);
        ImGui::DockBuilderDockWindow(opListTitle, opListDock);
        ImGui::DockBuilderDockWindow(customizeTitle, customizeDock);
        ImGui::DockBuilderFinish(dockspaceId);
        dockLayoutBuilt = true;
    }
    }

    if (loaded && false) {
        char tempTitle[260];

        snprintf(tempTitle, sizeof(tempTitle), "objList##%d", num);
        if (ImGui::BeginChild(tempTitle, { 230,400 }, ImGuiChildFlags_ResizeX | ImGuiChildFlags_FrameStyle)) {
            for (int i = 0; i < arr_seobj.count; i++) {
                ImGui::PushID(i);
                SEOBJ& seobj = ((SEOBJ*)arr_seobj.data)[i];

                char buf[260];
                sprintf(buf, "%03d(%s)", i, seobj.name.outstr());

                if (ImGui::Selectable(buf, selected_obj == i)) {
                    selected_obj = i;
                }
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
        
        //ImGui::SameLine();
        //preview

        ImGui::Separator();
        SEOBJ& seobj = ((SEOBJ*)arr_seobj.data)[selected_obj];
        snprintf(tempTitle, sizeof(tempTitle), "objProperty##%d", num);
        if (ImGui::BeginChild(tempTitle, { 400,-1 }, ImGuiChildFlags_ResizeX | ImGuiChildFlags_FrameStyle)) {
            ImGui::PushID(selected_obj);

            for (int b = 0; b < seobj.body.count; b++) {
                ImGui::Text("%s", ((CSTR*)seobj.body.data)[b]);
            }

            if (seobj.type2 == 0) {
                if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None))
                {
                    if (ImGui::BeginTabItem("SRC"))
                    {
                        ImGui::SeparatorText("basic");
                        ImGui::InputText("gr", ((CSVbuf*)seobj.bodyCSV.data)[0].str[2], IM_COUNTOF(((CSVbuf*)seobj.bodyCSV.data)[0].str[2].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        ImGui::InputText("x", ((CSVbuf*)seobj.bodyCSV.data)[0].str[3], IM_COUNTOF(((CSVbuf*)seobj.bodyCSV.data)[0].str[3].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        ImGui::InputText("y", ((CSVbuf*)seobj.bodyCSV.data)[0].str[4], IM_COUNTOF(((CSVbuf*)seobj.bodyCSV.data)[0].str[4].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        ImGui::InputText("w", ((CSVbuf*)seobj.bodyCSV.data)[0].str[5], IM_COUNTOF(((CSVbuf*)seobj.bodyCSV.data)[0].str[5].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        ImGui::InputText("h", ((CSVbuf*)seobj.bodyCSV.data)[0].str[6], IM_COUNTOF(((CSVbuf*)seobj.bodyCSV.data)[0].str[6].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        ImGui::SeparatorText("animation");
                        ImGui::InputText("div_x", ((CSVbuf*)seobj.bodyCSV.data)[0].str[7], IM_COUNTOF(((CSVbuf*)seobj.bodyCSV.data)[0].str[7].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        ImGui::InputText("div_y", ((CSVbuf*)seobj.bodyCSV.data)[0].str[8], IM_COUNTOF(((CSVbuf*)seobj.bodyCSV.data)[0].str[8].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        ImGui::InputText("cycle", ((CSVbuf*)seobj.bodyCSV.data)[0].str[9], IM_COUNTOF(((CSVbuf*)seobj.bodyCSV.data)[0].str[9].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        //ImGui::InputText("timer", ((CSVbuf*)seobj.bodyCSV.data)[0].str[10], IM_COUNTOF(((CSVbuf*)seobj.bodyCSV.data)[0].str[10].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        CstrInputText("timer", &((CSVbuf*)seobj.bodyCSV.data)[0].str[10], ImGuiInputTextFlags_None);
                        ImGui::Text("Data: %p\nSize: %d\nCapacity: %d", (void*)((CSVbuf*)seobj.bodyCSV.data)[0].str[10].body, ((CSVbuf*)seobj.bodyCSV.data)[0].str[10].length(), ((CSVbuf*)seobj.bodyCSV.data)[0].str[10].msize());


                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("DST"))
                    {
                        ImGui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            }

            ImGui::PopID();
        }
        ImGui::EndChild();
    }
    else if (!loaded) {
        SEUI::EmptyState("Start with a skin",
            "Open an existing LR2 skin or create a scene preset. Preview, Image Manager, DST View and Object Editor will appear in the default workspace layout.");
        const float buttonsWidth = 210.0f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (std::max)(0.0f,
            (ImGui::GetContentRegionAvail().x - buttonsWidth) * 0.5f));
        if (SEUI::ActionButton("Open skin", "Open from LR2 skin list")) {
            ScanSkins();
            wSkinList = true;
        }
        ImGui::SameLine();
        if (SEUI::ActionButton("New skin", "Create from a scene preset")) wNewskin = true;
    }
    

    ImGui::End();

    //subwindows
    //HOW TO ADD FEATURE - STEP 4 : call function. end
    if (wSkinList) drawSkinList();
    if (wNewskin) drawNewskin();

    if (wSaveMenu) drawSaveMenu();
    if (wSaveMenu2) drawSaveMenu2();

    if (wTextEdit) drawTextEdit();

    if (wPreview) drawPreview();
    if (wCustomize) drawCustomize();
    if (wImgManager) drawImgManager();
    if (wAssetBrowser) drawAssetBrowser();
    if (wFileManager) drawFileManager();
    if (wSimplePreview) drawSimplePreview();
    if (wDstView) drawDstView();
    // Older call sites use wObjectEditor as an "open the editor" command.
    // Translate it into the two independently dockable panes.
    if (wObjectEditor) {
        wObjectBrowser = true;
        wObjectInspector = true;
        wObjectEditor = false;
    }
    if (wObjectBrowser || wObjectInspector) {
        drawObjectEditor();
    }
    if (wObjectManager) drawObjectManager();
    if (wObjectManagerTest) drawObjectManagerTest();
    if (wProperty) drawProperty();
    if (wOpList) drawOpList();
    if (wHistory) drawHistory();

    if (wNewObject) drawNewObject();

    ImGuiIO& shortcutIO = ImGui::GetIO();
    if (loaded && shortcutIO.KeyCtrl && !shortcutIO.WantTextInput) {
        if (ImGui::IsKeyPressed(ImGuiKey_Z, false)) UndoLastEdit();
        if (ImGui::IsKeyPressed(ImGuiKey_S, false)) {
            if (shortcutIO.KeyShift) {
                newPath[0] = '\0';
                wSaveMenu = true;
            } else {
                SaveCurrentSkin();
            }
        }
    }


    return 0;
}

int WORKSPACE::ScanSkins() {
    InitSkinData(&g.skinData);
    MakeSkinList(&g.skinData, CSTR("LR2files\\Theme\\"));
    MakeSkinList(&g.skinData, CSTR("LR2files\\Sound\\"));
    return 1;
}

int WORKSPACE::drawSkinList() {
    static int item_selected_idx = 0;
    int oldSelected = item_selected_idx;
    static PDIRECT3DTEXTURE9 preview_tex;
    static int preview_size_x, preview_size_y;
    static bool isPreview = false;
    static int resolutionSaveState = 0; // 0: untouched, 1: saved, -1: failed
    static std::string resolutionSavePath;

    const ImVec2 workSize = ImGui::GetMainViewport()->WorkSize;
    const ImVec2 minimumSize(
        (std::min)(820.0f, workSize.x),
        (std::min)(540.0f, workSize.y));
    ImGui::SetNextWindowSizeConstraints(minimumSize, workSize);
    ImGui::Begin("Open from skinlist", &wSkinList);

    if (g.skinData.Count <= 0) {
        ImGui::TextUnformatted("No skins found.");
        ImGui::End();
        return 0;
    }
    if (item_selected_idx < 0 || item_selected_idx >= g.skinData.Count)
        item_selected_idx = 0;

    if (ImGui::BeginChild(ImGuiChildFlags_FrameStyle, { 300,-1 }))
    {
        for (int n = 0; n < g.skinData.Count; n++)
        {
            const bool is_selected = (item_selected_idx == n);
            char itemname[260];
            const std::string titleUtf8 = Cp932ToUtf8(g.skinData.Data[n].title.outstr());
            snprintf(itemname, sizeof(itemname), "%02d:%s -%s", n, titleUtf8.c_str(), SKINTYPESTR[g.skinData.Data[n].type]);
            if (ImGui::Selectable(itemname, is_selected)) {
                item_selected_idx = n;
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndChild();
    }
    

    int& m = item_selected_idx;
    ImGui::SameLine(0, 0);
    ImGui::BeginGroup();
    //ImTextureRef preview_texid = io.Fonts->TexRef;

    if (oldSelected != m) {
        isPreview = LoadTextureFromFile(g.skinData.Data[m].thumbnail.outstr(), &preview_tex, &preview_size_x, &preview_size_y);
        oldSelected = m;
        resolutionSaveState = 0;
        resolutionSavePath.clear();
    }
    ImGui::Image(isPreview ? preview_tex : NULL, { 320, 240 }, { 0, 0 }, { 1, 1 });
    ImGui::Text("%s", Cp932ToUtf8(g.skinData.Data[m].skinFile.outstr()).c_str());
    ImGui::Text("%s", Cp932ToUtf8(g.skinData.Data[m].title.outstr()).c_str());
    ImGui::Text("%s", Cp932ToUtf8(g.skinData.Data[m].maker.outstr()).c_str());
    ImGui::Text("%s", SKINTYPESTR[g.skinData.Data[m].type]);
    ImGui::SeparatorText("Resolution");
    ImGui::TextColored(ImVec4(1.0f, 0.78f, 0.28f, 1.0f),
        "AUTO SAVE: Changes are written directly to the original skin file and cannot be undone.");
    int resolution[2] = {
        g.skinData.Data[m].targetX,
        g.skinData.Data[m].targetY
    };
    ImGui::SetNextItemWidth(100.0f);
    if (ImGui::InputInt2("Width / Height (?)", resolution)) {
        if (resolution[0] < 64) resolution[0] = 64;
        if (resolution[0] > 7680) resolution[0] = 7680;
        if (resolution[1] < 64) resolution[1] = 64;
        if (resolution[1] > 4320) resolution[1] = 4320;
        g.skinData.Data[m].targetX = resolution[0];
        g.skinData.Data[m].targetY = resolution[1];
    }
    auto persistResolution = [&]() {
        const char* skinPath = g.skinData.Data[m].skinFile.outstr();
        resolutionSaveState = SaveResolutionToSkinFile(skinPath,
            g.skinData.Data[m].targetX, g.skinData.Data[m].targetY) ? 1 : -1;
        resolutionSavePath = skinPath ? skinPath : "";
    };
    if (ImGui::IsItemDeactivatedAfterEdit()) persistResolution();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        ImGui::SetTooltip("The #INFORMATION width/height fields will be updated.\nPress Enter or leave this field to save immediately.");

    struct ResolutionPreset { const char* label; int width; int height; };
    const ResolutionPreset presets[] = {
        { "640 x 480", 640, 480 },
        { "1280 x 720", 1280, 720 },
        { "1920 x 1080", 1920, 1080 }
    };
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.32f, 0.20f, 0.48f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.44f, 0.28f, 0.63f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.53f, 0.34f, 0.72f, 1.0f));
    for (int i = 0; i < 3; ++i) {
        if (i > 0) ImGui::SameLine();
        if (ImGui::Button(presets[i].label)) {
            g.skinData.Data[m].targetX = presets[i].width;
            g.skinData.Data[m].targetY = presets[i].height;
            persistResolution();
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
            ImGui::SetTooltip("Apply this resolution and save it to the skin file immediately.");
    }
    ImGui::PopStyleColor(3);
    if (resolutionSaveState > 0)
        ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.40f, 1.0f),
            "Saved resolution to #INFORMATION: %s", resolutionSavePath.c_str());
    else if (resolutionSaveState < 0)
        ImGui::TextColored(ImVec4(1.0f, 0.30f, 0.30f, 1.0f),
            "Failed to save resolution: %s", resolutionSavePath.c_str());

    if (ImGui::Button("LOAD", { 0, 0 })) {
        meta = g.skinData.Data[m];
        snprintf(title, 260, "%s -%s", meta.title.outstr(), SKINTYPESTR[meta.type]);
        
        strncpy(mainpath, g.skinData.Data[m].skinFile.outstr(), MAX_PATH);
        loaded = (LoadSkin(mainpath) == 0);
        if (loaded) wSkinList = false;
        
    }

    ImGui::SameLine(0, 3);
    if (ImGui::Button("BROWSE", { 0, 0 })) {
        const char* skinPath = g.skinData.Data[m].skinFile.outstr();
        if (skinPath && *skinPath) {
            char fullPath[MAX_PATH] = {};
            if (!GetFullPathNameA(skinPath, MAX_PATH, fullPath, NULL))
                strncpy(fullPath, skinPath, MAX_PATH - 1);
            std::string explorerArgs = "/select,\"";
            explorerArgs += fullPath;
            explorerArgs += "\"";
            ShellExecuteA(NULL, "open", "explorer.exe", explorerArgs.c_str(), NULL, SW_SHOWNORMAL);
        }
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        ImGui::SetTooltip("Open this skin file's folder in Windows Explorer.");
    ImGui::SameLine(0, 0);
    ImGui::EndGroup();
    ImGui::End();

    return 0;
}

int WORKSPACE::LoadSkinScript(char* path) {
    if (!path || !*path) return -1;
    char canonicalPath[MAX_PATH] = {};
    if (!GetFullPathNameA(path, MAX_PATH, canonicalPath, NULL))
        strncpy(canonicalPath, path, MAX_PATH - 1);
    for (const std::string& activePath : loadScriptStack) {
        if (_stricmp(activePath.c_str(), canonicalPath) == 0) {
            WriteSkinLoadLog("Include cycle blocked", canonicalPath);
            return -2;
        }
    }

    FILE* pFile;

    pFile = fopen(canonicalPath, "rb");
    if (!pFile) return -1;
    loadScriptStack.push_back(canonicalPath);

    //TODO : $FILE are stacked when save/load
    SKINFILELINEREAD* readS = (SKINFILELINEREAD*)skinfileLines.Get_new();
    readS->line.resize(1024);
    sprintf(readS->line, "$FILE \'%s\' start", canonicalPath);
    readS->isComment = true;
    readS->isSEcomment = true;
    readS->numTotal = skinfileLines.count-1;
    readS->num = 0;
    readS->filename.assign(canonicalPath);

    int c = 1;
    while (1) {
        std::string readLine;
        char readbuf[1024];
        while (fgets(readbuf, sizeof(readbuf), pFile) != 0) {
            readLine += readbuf;
            const size_t len = strlen(readbuf);
            if (len > 0 && readbuf[len - 1] == '\n') break;
        }
        if (readLine.empty()) break;

        SKINFILELINEREAD* read = (SKINFILELINEREAD*)skinfileLines.Get_new();
        read->line.resize(1024);
        read->filename.resize(260);
        read->line.assign(readLine.c_str());
            
        DealWhiteSpace(&read->line);
        read->numTotal = skinfileLines.count - 1;
        read->filename.assign(canonicalPath);
        read->num = c;
        read->isComment = (*read->line.atPos(0) != '#');
        read->isSEcomment = (*read->line.atPos(0) == '$');

        if(!read->isComment || read->line.left(11).isSame("$SRC_IMAGE,")) {
            SplitCSV(read->line, &read->csv, ",");
            read->csvColumnCount = CountCsvColumns(read->line);
        }
            
        c++;

        if (read->line.left(8).isSame("#INCLUDE")) {
            const char* includeText = read->csv.str[1].body ? read->csv.str[1].outstr() : "";
            char includePath[MAX_PATH] = {};
            CSTR parent(canonicalPath);
            parent.assign(parent.getDirectory());
            snprintf(includePath, sizeof(includePath), "%s%s", parent.outstr(), includeText);
            if (GetFileAttributesA(includePath) == INVALID_FILE_ATTRIBUTES)
                strncpy(includePath, includeText, sizeof(includePath) - 1);
            char siblingInclude[MAX_PATH] = {};
            if (ResolveSiblingPlayPath(includePath, mainpath,
                siblingInclude, sizeof(siblingInclude)))
                strncpy_s(includePath, siblingInclude, _TRUNCATE);
            if (*includePath) {
                CSTR tmp(includePath);
                arr_subpath.push_back(&tmp);
                const int includeResult = LoadSkinScript(includePath);
                if (includeResult < 0 && includeResult != -2)
                    WriteSkinLoadLog("Include open failed", includePath);
            }
        }
    }

    SKINFILELINEREAD* readE = (SKINFILELINEREAD*)skinfileLines.Get_new();
    readE->line.resize(1024);
    sprintf(readE->line, "$FILE \'%s\' end", canonicalPath);
    readE->isComment = true;
    readE->isSEcomment = true;
    readE->numTotal = skinfileLines.count - 1;
    readE->num = c;
    readE->filename.assign(canonicalPath);
        

    fclose(pFile);
    loadScriptStack.pop_back();

    return 0;
}

int WORKSPACE::ParseSkinLegacyObjectsAndAssets() {
    int objNow = -1;
    
    //obj groups
    for (int i = 0; i < skinfileLines.count; i++) {
        SKINFILELINEREAD& read = ((SKINFILELINEREAD*)skinfileLines.data)[i];
        SEOBJ* obj = NULL;
        if (read.csv.str[0].left(4).isSame("#SRC")) {
            read.isSRC = true;
            read.objID = ++objNow;
            
            obj = (SEOBJ*)arr_seobj.Get_new();
            obj->type2 = 0;
            obj->name.assign(read.csv.str[0]);
            obj->body.Alloc(sizeof(CSTR),2);
            obj->bodyCSV.Alloc(sizeof(CSVbuf), 2);

            CSTR* bodyline = (CSTR*)obj->body.Get_new();
            CSVbuf* bodyCSV = (CSVbuf*)obj->bodyCSV.Get_new();
            bodyline->assign(read.line);
            SplitCSV(*bodyline, bodyCSV, ",");

            //get all IMG presets fromSRC here
            if (read.csv.str[0].isDiff("#SRC_TEXT")){
                //check duplicated
                const int existingImage = FindIMG(read.csv.val[2], read.csv.val[3],
                    read.csv.val[4], read.csv.val[5], read.csv.val[6], read.ifgroup);
                if (existingImage == arr_IMG.count) {
                    IMG* img = (IMG*)arr_IMG.Get_new();
                    cstrSprintf(&img->name,"%s", read.csv.str[0].outstr());
                    img->gr = read.csv.val[2];
                    img->x = read.csv.val[3];
                    img->y = read.csv.val[4];
                    img->w = read.csv.val[5];
                    img->h = read.csv.val[6];
                    img->ifGroup = read.ifgroup;
                    img->sourceDeclare = read.numTotal;
                    img->editorDeclare = -1;
                } else {
                    // Keep one crop card for duplicate coordinates, but bind
                    // it to an animated declaration when one exists. This is
                    // what lets Asset Browser recover the correct frame grid.
                    IMG& img = ((IMG*)arr_IMG.data)[existingImage];
                    const int currentFrames = (std::max)(1, read.csv.val[7]) *
                        (std::max)(1, read.csv.val[8]);
                    bool existingAnimated = false;
                    if (img.sourceDeclare >= 0 && img.sourceDeclare < skinfileLines.count) {
                        SKINFILELINEREAD& previous =
                            ((SKINFILELINEREAD*)skinfileLines.data)[img.sourceDeclare];
                        existingAnimated = previous.csv.val[9] > 0 &&
                            (std::max)(1, previous.csv.val[7]) *
                            (std::max)(1, previous.csv.val[8]) > 1;
                    }
                    if (!existingAnimated && read.csv.val[9] > 0 && currentFrames > 1)
                        img.sourceDeclare = read.numTotal;
                }
            }
        }
        else if (read.csv.str[0].left(4).isSame("#DST")) {
            read.isDST = true;
            read.objID = objNow;

            // Some large skins contain conditionally expanded DST commands
            // before a usable SRC. Never dereference an empty object list.
            if (arr_seobj.count <= 0) {
                read.isDST = false;
                read.isOther = true;
                read.objID = -1;
                continue;
            }
            obj = (SEOBJ*)arr_seobj.Get_last();

            CSTR* bodyline = (CSTR*)obj->body.Get_new();
            CSVbuf* bodyCSV = (CSVbuf*)obj->bodyCSV.Get_new();
            bodyline->assign(read.line);
            SplitCSV(*bodyline, bodyCSV, ",");
        }
        else if(read.csv.str[0].left(1).isSame("#") ){
            read.isOther = true;
            read.objID = -1;
            
            obj = (SEOBJ*)arr_seobj.Get_new(); 
            obj->type2 = -1;
            obj->name.assign(read.csv.str[0]); 
            obj->body.Alloc(sizeof(CSTR), 1);
            obj->bodyCSV.Alloc(sizeof(CSVbuf), 1);

            CSTR* bodyline = (CSTR*)obj->body.Get_new();
            CSVbuf* bodyCSV = (CSVbuf*)obj->bodyCSV.Get_new();
            bodyline->assign(read.line);
            SplitCSV(*bodyline, bodyCSV, ",");
        }
    }

    // Editor-only image crops are serialized with the #SRC_IMAGE column
    // layout and a '$' command head. Parse them after all real SRC commands so
    // an in-use crop is represented by its actual Object, not duplicated as a
    // manual Asset.
    for (int i = 0; i < skinfileLines.count; ++i) {
        SKINFILELINEREAD& read = ((SKINFILELINEREAD*)skinfileLines.data)[i];
        const char* text = read.line.body ? read.line.outstr() : "";
        if (strncmp(text, "$SRC_IMAGE,", 11) != 0) continue;

        CSVbuf assetCsv;
        SplitCSV(read.line, &assetCsv, ",");
        if (FindIMG(assetCsv.val[2], assetCsv.val[3], assetCsv.val[4],
            assetCsv.val[5], assetCsv.val[6], read.ifgroup) != arr_IMG.count)
            continue;

        IMG* img = (IMG*)arr_IMG.Get_new();
        img->name.assign("manual crop");
        img->gr = assetCsv.val[2];
        img->x = assetCsv.val[3];
        img->y = assetCsv.val[4];
        img->w = assetCsv.val[5];
        img->h = assetCsv.val[6];
        img->ifGroup = read.ifgroup;
        img->sourceDeclare = -2;
        img->editorDeclare = i;
    }

    return 0;
}

int WORKSPACE::ParseSkinConditions() {
    struct ConditionFrame {
        int parentGroup;
        int currentGroup;
        int nextOrder;
    };
    std::vector<ConditionFrame> conditionStack;
    IFUNIT rootIf;
    arr_ifunit.push_back(&rootIf);

    for (int i = 0; i < skinfileLines.count; ++i) {
        SKINFILELINEREAD& read = ((SKINFILELINEREAD*)skinfileLines.data)[i];
        const int currentGroup = conditionStack.empty()
            ? 0 : conditionStack.back().currentGroup;
        if (read.isComment) {
            // Editor metadata placed next to a command inherits the same IF
            // branch even though LR2 treats the '$' line as a comment.
            read.ifgroup = currentGroup;
            continue;
        }

        if (read.csv.str[0].isSame("#IF")) {
            IFUNIT branch;
            for (int value = 0; value < 10; ++value)
                branch.data[value] = read.csv.val[value + 1];
            branch.depth = (int)conditionStack.size() + 1;
            branch.order = 0;
            branch.parentID = currentGroup;
            const int newGroup = arr_ifunit.count;
            arr_ifunit.push_back(&branch);
            conditionStack.push_back({ currentGroup, newGroup, 0 });
            read.ifgroup = newGroup;
            read.isIfGroupHead = true;
            read.isGroupHead = true;
        } else if (read.csv.str[0].isSame("#ELSEIF") ||
            read.csv.str[0].isSame("#ELSE")) {
            if (conditionStack.empty()) {
                read.ifgroup = 0;
            } else {
                ConditionFrame& frame = conditionStack.back();
                IFUNIT branch;
                if (read.csv.str[0].isSame("#ELSEIF")) {
                    for (int value = 0; value < 10; ++value)
                        branch.data[value] = read.csv.val[value + 1];
                }
                branch.depth = (int)conditionStack.size();
                branch.order = ++frame.nextOrder;
                branch.parentID = frame.parentGroup;
                const int newGroup = arr_ifunit.count;
                arr_ifunit.push_back(&branch);
                frame.currentGroup = newGroup;
                read.ifgroup = newGroup;
            }
            read.isIfGroupHead = true;
            read.isGroupHead = true;
        } else if (read.csv.str[0].isSame("#ENDIF")) {
            if (conditionStack.empty()) {
                read.ifgroup = 0;
            } else {
                read.ifgroup = conditionStack.back().currentGroup;
                conditionStack.pop_back();
            }
            read.isIfGroupEnd = true;
            read.isGroupEnd = true;
        } else {
            read.ifgroup = currentGroup;
        }
    }
    return 0;
}

int WORKSPACE::ParseSkinGraphics() {
    struct GraphicConditionFrame {
        int grCountBeforeBlock;
        int maxBranchEnd;
    };
    std::vector<GraphicConditionFrame> conditionStack;
    int grCount = 0;

    for (int i = 0; i < skinfileLines.count; ++i) {
        SKINFILELINEREAD& read = ((SKINFILELINEREAD*)skinfileLines.data)[i];
        if (read.isComment) continue;
        if (read.csv.str[0].isSame("#IF")) {
            conditionStack.push_back({ grCount, grCount });
            continue;
        }
        if (read.csv.str[0].isSame("#ELSEIF") || read.csv.str[0].isSame("#ELSE")) {
            if (!conditionStack.empty()) {
                GraphicConditionFrame& frame = conditionStack.back();
                if (grCount > frame.maxBranchEnd) frame.maxBranchEnd = grCount;
                grCount = frame.grCountBeforeBlock;
            }
            continue;
        }
        if (read.csv.str[0].isSame("#ENDIF")) {
            if (!conditionStack.empty()) {
                GraphicConditionFrame frame = conditionStack.back();
                conditionStack.pop_back();
                if (grCount > frame.maxBranchEnd) frame.maxBranchEnd = grCount;
                grCount = frame.maxBranchEnd;
            }
            continue;
        }


        if (read.csv.str[0].isSame("#CUSTOMFILE")) {
            CSTR* tmpstr = (CSTR*)(arr_CustomFile.Get_new());
            tmpstr->assign(read.csv.str[2]);
        }

        else if (read.csv.str[0].isSame("#IMAGE")) {
            
            CSTR line(read.csv.str[1]);

            bool isWild = false;

            for (int wc = 0; wc < arr_CustomFile.count; wc++) {
                if (line.isSame(((CSTR*)arr_CustomFile.data)[wc].outstr())) {
                    isWild = true;
                    break;
                }
            }
            if (strrchr(line.outstr(), '*')) isWild = true;

            if (!isWild) {
                SRCGR* tmp = (SRCGR*)(arr_SRCGR.Get_new());
                tmp->path.assign(line);
                
                char* cur = strrchr(read.csv.str[1].outstr(), '/');
                if (cur == NULL) cur = strrchr(read.csv.str[1].outstr(), '\\');
                if (cur)         tmp->filename.assign(cur + 1);

                tmp->grID = grCount;
                tmp->isIf = read.ifgroup;
                tmp->declare = i;
                tmp->wildcard = false;
            }
            
            else if (isWild) {
                WIN32_FIND_DATA FindFileData;
                HANDLE hFindFile;

                CSTR str1(line.left(line.findStrPos("*")));
                CSTR str2(line.right(line.length() - str1.length() - 1));
                CSTR str3(str1);
                str3.add("*");

                hFindFile = FindFirstFileA(str3, (LPWIN32_FIND_DATAA)&FindFileData);
                if (hFindFile != (HANDLE)-1) {
                    do {
                        if (strcmp("..", (char*)FindFileData.cFileName) && strcmp(".", (char*)FindFileData.cFileName)) {

                            SRCGR* tmp2 = (SRCGR*)(arr_SRCGR.Get_new());
                            tmp2->path.assign(str1);
                            tmp2->path.add(FindFileData.cFileName);
                            tmp2->filename.assign(FindFileData.cFileName);

                            tmp2->fromWildcard = true;
                            tmp2->grID = grCount;
                            tmp2->isIf = read.ifgroup;
                            tmp2->declare = i;
                        }
                    } while (FindNextFileA(hFindFile, (LPWIN32_FIND_DATAA)&FindFileData));
                    FindClose(hFindFile);
                }
            }

            grCount++;
            ((IFUNIT*)arr_ifunit.data)[read.ifgroup].grCount++;
        }
    }
    return 0;
}

int WORKSPACE::ParseSkinSourcesAndDestinations() {
    int srcNow = -1;
    int srcOld = -1;
    int objTypeCount[100] = { 0, };
    currentLeadDST = -1;

    auto appendSpecialSourcePlaceholder = [&](SKINFILELINEREAD& sourceLine) {
        SRC* placeholder = (SRC*)arr_SRC.Get_new();
        placeholder->gr = sourceLine.csv.val[2];
        placeholder->sizeX = 1;
        placeholder->sizeY = 1;
        placeholder->div_x = 1;
        placeholder->div_y = 1;
        placeholder->declare = sourceLine.numTotal;
        placeholder->name.assign("LR2 special source placeholder");
        ++srcNow;
    };

    for (int i = 0; i < skinfileLines.count; ++i) {
        SKINFILELINEREAD& read = ((SKINFILELINEREAD*)skinfileLines.data)[i];
        if (read.isComment) continue;


        ////////////////
        if (read.csv.str[0].isSame("#SRC_IMAGE")) {

            if (read.csv.val[2] == 110 || read.csv.val[2] == 111) {
                // LR2 uses graphic IDs 110/111 as special placeholders.
                // The old code advanced srcNow without adding an ARR entry,
                // shifting every subsequent DST->src index and eventually
                // reading past arr_SRC on skins such as tricoro HD.
                SRC* placeholder = (SRC*)(arr_SRC.Get_new());
                placeholder->gr = 0;
                placeholder->sizeX = 1;
                placeholder->sizeY = 1;
                placeholder->div_x = 1;
                placeholder->div_y = 1;
                placeholder->declare = read.numTotal;
                placeholder->objType = 10;
                placeholder->objID = objTypeCount[9]++;
                placeholder->name.assign("LR2 special image placeholder");
                srcNow++;
                continue;
            }

            SRC *src = (SRC*)(arr_SRC.Get_new());
                        
            src->gr = read.csv.val[2];
            src->x = read.csv.val[3];
            src->y = read.csv.val[4];


            src->sizeX = read.csv.val[5];// == -1 ? img.sizeX - src->x : read.csv.val[5];
            src->sizeY = read.csv.val[6];// == -1 ? img.sizeY - src->y : read.csv.val[6];

            src->div_x = read.csv.val[7];
            src->div_y = read.csv.val[8];
            src->cycle = read.csv.val[9];
            src->timer = read.csv.val[10];

            src->declare = read.numTotal;

            char tmp[260];
            if (src->timer) {
                sprintf(tmp, "T%d(%s) : %d*%d ##%d", src->timer, timerName(src->timer), src->sizeX, src->sizeY, read.numTotal);
            }
            else {
                sprintf(tmp, "noname : %d*%d ##%d", src->sizeX, src->sizeY, read.numTotal);
            }
            src->name.assign(tmp);

            src->objType = 10;
            src->objID = objTypeCount[9]++;
            
            srcNow++;
        }

        else if (read.csv.str[0].isSame("#DST_BGA")) {

        }
        else if (read.csv.str[0].isSame("#SRC_NOWJUDGE_1P") ||
            read.csv.str[0].isSame("#SRC_NOWCOMBO_1P") ||
            read.csv.str[0].isSame("#SRC_NOWJUDGE_2P") ||
            read.csv.str[0].isSame("#SRC_NOWCOMBO_2P")) {
            // These PLAY objects are loaded into dedicated LR2 arrays rather
            // than otherObject[], but the editor still needs an SRC entry so
            // the following indexed DST rows start a new selectable object.
            // Without this, their DST rows were attached to the preceding
            // generic object (or discarded) and Preview had no bounds to flash.
            SRC* src = (SRC*)arr_SRC.Get_new();
            src->gr = read.csv.val[2];
            src->x = read.csv.val[3];
            src->y = read.csv.val[4];
            src->sizeX = read.csv.val[5];
            src->sizeY = read.csv.val[6];
            src->div_x = read.csv.val[7];
            src->div_y = read.csv.val[8];
            src->cycle = read.csv.val[9];
            src->timer = read.csv.val[10];
            src->align = read.csv.val[12];
            src->keta = read.csv.val[13];
            src->declare = read.numTotal;
            src->objType = 10;
            src->objID = objTypeCount[9]++;

            char tmp[260];
            snprintf(tmp, sizeof(tmp), "%s %d ##%d",
                read.csv.str[0].outstr(), read.csv.val[1], read.numTotal);
            src->name.assign(tmp);
            ++srcNow;
        }
        else if (read.csv.str[0].isSame("#DST_IMAGE") || read.csv.str[0].isSame("#DST_SLIDER") || read.csv.str[0].isSame("#DST_BUTTON") 
            || read.csv.str[0].isSame("#DST_BARGRAPH") || read.csv.str[0].isSame("#DST_NUMBER") || read.csv.str[0].isSame("#DST_TEXT") || read.csv.str[0].isSame("#DST_ONMOUSE")
            || read.csv.str[0].left(5).isSame("#DST_")) {
            
            DST* dst = NULL;
            if (srcNow > srcOld) {
                dst = (DST*)(arr_DST.Get_new());

                dst->name.assign(dstName(dst->op1));
                dst->animation = 0;
                currentLeadDST = arr_DST.count - 1;
                dst->src = srcNow;
                dst->declare = i;

                dst->arr_animation.Free();
                dst->arr_animation.Alloc(sizeof(DST_ANIMATION), 1);
                srcOld = srcNow;
            }
            else {// if(srcNow == srcOld){
                if (currentLeadDST < 0 || currentLeadDST >= arr_DST.count) continue;
                dst = &(((DST*)arr_DST.data)[currentLeadDST]);
            }
            
            if (!dst) continue;
            DST_ANIMATION* dstd = (DST_ANIMATION*)(dst->arr_animation.Get_new());

            dstd->time = read.csv.val[2];
            dstd->x = read.csv.val[3];
            dstd->y = read.csv.val[4];
            dstd->w = read.csv.val[5];
            dstd->h = read.csv.val[6];

            dstd->acc = read.csv.val[7];
            dstd->a = read.csv.val[8];
            dstd->r = read.csv.val[9];
            dstd->g = read.csv.val[10];
            dstd->b = read.csv.val[11];

            dstd->blend = read.csv.val[12];
            dstd->filter = read.csv.val[13];
            dstd->angle = read.csv.val[14];
            dstd->center = read.csv.val[15];

            if (dst->animation == 0) {
                dst->loop = read.csv.val[16];
                dst->timer = read.csv.val[17];

                dst->op1 = read.csv.val[18];
                dst->op2 = read.csv.val[19];
                dst->op3 = read.csv.val[20];
                dst->op4 = read.csv.val[21];
                //op5 is on 22??
            }
            dst->animation++;
            dst->leadDST = currentLeadDST;
        }
        

        else if (read.csv.str[0].isSame("#SRC_NUMBER")) { 

            if (read.csv.val[2] == 110 || read.csv.val[2] == 111) {
                appendSpecialSourcePlaceholder(read); continue;
            }

            SRC* src = (SRC*)(arr_SRC.Get_new());

            src->gr = read.csv.val[2];
            src->x = read.csv.val[3];
            src->y = read.csv.val[4];


            src->sizeX = read.csv.val[5];// == -1 ? img.sizeX - src->x : read.csv.val[5];
            src->sizeY = read.csv.val[6];// == -1 ? img.sizeY - src->y : read.csv.val[6];

            src->div_x = read.csv.val[7]; //have to be 10, 11, 24 set divs
            src->div_y = read.csv.val[8];
            src->cycle = read.csv.val[9];
            src->timer = read.csv.val[10];

            src->num = read.csv.val[11];
            src->align = read.csv.val[12];
            src->keta = read.csv.val[13];

            src->declare = read.numTotal;

            char tmp[260];
            if (src->timer || src->type) {
                sprintf(tmp, "T%d(%s) : %s %d*%d ##%d", src->timer, timerName(src->timer), numberName(src->num), src->sizeX, src->sizeY, read.numTotal);
            }
            else {
                sprintf(tmp, "noname : %d*%d ##%d", src->sizeX, src->sizeY, read.numTotal);
            }
            src->name.assign(tmp);

            src->objType = 7;
            src->objID = objTypeCount[6]++;

            srcNow++;

        }
        
        else if (read.csv.str[0].isSame("#SRC_SLIDER")) {

            if (read.csv.val[2] == 110 || read.csv.val[2] == 111) {
                appendSpecialSourcePlaceholder(read); continue;
            }

            SRC* src = (SRC*)(arr_SRC.Get_new());

            src->gr = read.csv.val[2];
            src->x = read.csv.val[3];
            src->y = read.csv.val[4];


            src->sizeX = read.csv.val[5];// == -1 ? img.sizeX - src->x : read.csv.val[5];
            src->sizeY = read.csv.val[6];// == -1 ? img.sizeY - src->y : read.csv.val[6];

            src->div_x = read.csv.val[7]; 
            src->div_y = read.csv.val[8];
            src->cycle = read.csv.val[9];
            src->timer = read.csv.val[10];

            src->muki = read.csv.val[11];
            src->range = read.csv.val[12];
            src->type = read.csv.val[13];
            src->disable = read.csv.val[14];

            src->declare = read.numTotal;

            char tmp[260];
            if (src->timer || src->type) {
                sprintf(tmp, "T%d(%s) : %s %d*%d ##%d", src->timer, timerName(src->timer), sliderName(src->type), src->sizeX, src->sizeY, read.numTotal);
            }
            else {
                sprintf(tmp, "noname : %d*%d ##%d", src->sizeX, src->sizeY, read.numTotal);
            }
            src->name.assign(tmp);

            srcNow++;
        }
        else if (read.csv.str[0].isSame("#SRC_BUTTON")) {

            if (read.csv.val[2] == 110 || read.csv.val[2] == 111) {
                appendSpecialSourcePlaceholder(read); continue;
            }

            SRC* src = (SRC*)(arr_SRC.Get_new());

            src->gr = read.csv.val[2];
            src->x = read.csv.val[3];
            src->y = read.csv.val[4];


            src->sizeX = read.csv.val[5];// == -1 ? img.sizeX - src->x : read.csv.val[5];
            src->sizeY = read.csv.val[6];// == -1 ? img.sizeY - src->y : read.csv.val[6];

            src->div_x = read.csv.val[7];
            src->div_y = read.csv.val[8];
            src->cycle = read.csv.val[9];
            src->timer = read.csv.val[10];

            src->type = read.csv.val[11];
            src->click = read.csv.val[12];
            src->panel = read.csv.val[13];
            src->plusonly = read.csv.val[14];

            src->declare = read.numTotal;

            char tmp[260];
            if (src->timer || src->type) {
                sprintf(tmp, "T%d(%s) : %s %d*%d ##%d", src->timer, timerName(src->timer), buttonName(src->type), src->sizeX, src->sizeY, read.numTotal);
            }
            else {
                sprintf(tmp, "noname : %d*%d ##%d", src->sizeX, src->sizeY, read.numTotal);
            }
            src->name.assign(tmp);

            srcNow++;
        }
        else if (read.csv.str[0].isSame("#SRC_BARGRAPH")) {

            if (read.csv.val[2] == 110 || read.csv.val[2] == 111) {
                appendSpecialSourcePlaceholder(read); continue;
            }

            SRC* src = (SRC*)(arr_SRC.Get_new());

            src->gr = read.csv.val[2];
            src->x = read.csv.val[3];
            src->y = read.csv.val[4];


            src->sizeX = read.csv.val[5];// == -1 ? img.sizeX - src->x : read.csv.val[5];
            src->sizeY = read.csv.val[6];// == -1 ? img.sizeY - src->y : read.csv.val[6];

            src->div_x = read.csv.val[7];
            src->div_y = read.csv.val[8];
            src->cycle = read.csv.val[9];
            src->timer = read.csv.val[10];

            src->type = read.csv.val[11];
            src->muki = read.csv.val[12];

            src->declare = read.numTotal;

            char tmp[260];
            if (src->timer || src->type) {
                sprintf(tmp, "T%d(%s) : %s %d*%d ##%d", src->timer, timerName(src->timer), bargraphName(src->type), src->sizeX, src->sizeY, read.numTotal);
            }
            else {
                sprintf(tmp, "noname : %d*%d ##%d", src->sizeX, src->sizeY, read.numTotal);
            }
            src->name.assign(tmp);

            srcNow++;
         }
        else if (read.csv.str[0].isSame("#SRC_TEXT")) {

            if (read.csv.val[2] == 110 || read.csv.val[2] == 111) {
                appendSpecialSourcePlaceholder(read); continue;
            }

            SRC* src = (SRC*)(arr_SRC.Get_new());

            src->gr = read.csv.val[2];
            src->x = read.csv.val[3];
            src->y = read.csv.val[4];


            src->sizeX = read.csv.val[5];// == -1 ? img.sizeX - src->x : read.csv.val[5];
            src->sizeY = read.csv.val[6];// == -1 ? img.sizeY - src->y : read.csv.val[6];

            src->div_x = read.csv.val[7]; //have to be 10, 11, 24 set divs
            src->div_y = read.csv.val[8];
            src->cycle = read.csv.val[9];
            src->timer = read.csv.val[10];

            src->num = read.csv.val[11];
            src->align = read.csv.val[12];
            src->keta = read.csv.val[13];

            src->declare = read.numTotal;

            char tmp[260];
            if (src->timer || src->type) {
                sprintf(tmp, "T%d(%s) : %s %d*%d ##%d", src->timer, timerName(src->timer), numberName(src->num), src->sizeX, src->sizeY, read.numTotal);
            }
            else {
                sprintf(tmp, "noname : %d*%d ##%d", src->sizeX, src->sizeY, read.numTotal);
            }
            src->name.assign(tmp);
            srcNow++;
        }
        else if (read.csv.str[0].isSame("#SRC_ONMOUSE")) {

            if (read.csv.val[2] == 110 || read.csv.val[2] == 111) {
                appendSpecialSourcePlaceholder(read); continue;
            }

            SRC* src = (SRC*)(arr_SRC.Get_new());

            src->gr = read.csv.val[2];
            src->x = read.csv.val[3];
            src->y = read.csv.val[4];


            src->sizeX = read.csv.val[5];// == -1 ? img.sizeX - src->x : read.csv.val[5];
            src->sizeY = read.csv.val[6];// == -1 ? img.sizeY - src->y : read.csv.val[6];

            src->div_x = read.csv.val[7]; //have to be 10, 11, 24 set divs
            src->div_y = read.csv.val[8];
            src->cycle = read.csv.val[9];
            src->timer = read.csv.val[10];

            src->num = read.csv.val[11];
            src->align = read.csv.val[12];
            src->keta = read.csv.val[13];

            src->declare = read.numTotal;

            char tmp[260];
            if (src->timer || src->type) {
                sprintf(tmp, "T%d(%s) : %s %d*%d ##%d", src->timer, timerName(src->timer), numberName(src->num), src->sizeX, src->sizeY, read.numTotal);
            }
            else {
                sprintf(tmp, "noname : %d*%d ##%d", src->sizeX, src->sizeY, read.numTotal);
            }
            src->name.assign(tmp);
            srcNow++;
        }
        else if (read.csv.str[0].isSame("#SRC_BGA")) {

            if (read.csv.val[2] == 110 || read.csv.val[2] == 111) {
                appendSpecialSourcePlaceholder(read); continue;
            }

            SRC* src = (SRC*)(arr_SRC.Get_new());

            src->gr = read.csv.val[2];
            src->x = read.csv.val[3];
            src->y = read.csv.val[4];


            src->sizeX = read.csv.val[5];// == -1 ? img.sizeX - src->x : read.csv.val[5];
            src->sizeY = read.csv.val[6];// == -1 ? img.sizeY - src->y : read.csv.val[6];

            src->div_x = read.csv.val[7]; //have to be 10, 11, 24 set divs
            src->div_y = read.csv.val[8];
            src->cycle = read.csv.val[9];
            src->timer = read.csv.val[10];

            src->num = read.csv.val[11];
            src->align = read.csv.val[12];
            src->keta = read.csv.val[13];

            src->declare = read.numTotal;

            char tmp[260];
            if (src->timer || src->type) {
                sprintf(tmp, "T%d(%s) : %s %d*%d ##%d", src->timer, timerName(src->timer), numberName(src->num), src->sizeX, src->sizeY, read.numTotal);
            }
            else {
                sprintf(tmp, "noname : %d*%d ##%d", src->sizeX, src->sizeY, read.numTotal);
            }
            src->name.assign(tmp);
            srcNow++;
        }

    }
    return 0;
}

int WORKSPACE::LoadSkinGraphicMetadata() {
    for (int n = 0; n < arr_SRCGR.count; n++) {
        SRCGR& img = ((SRCGR*)arr_SRCGR.data)[n];
        CSTR& path = ((SRCGR*)arr_SRCGR.data)[n].path;

        char siblingAsset[MAX_PATH] = {};
        if (ResolveSiblingPlayPath(path.outstr(), mainpath,
            siblingAsset, sizeof(siblingAsset)))
            path.assign(siblingAsset);

        WriteSkinLoadLog("ParseSkin image", path.outstr());

        if (path.isSame("CONTINUE")) {
            img.sizeX = 0;
            img.sizeY = 0;
            continue;
        }

        //TODO: deal *

		// Complex skins list hundreds of wildcard/custom candidates. Uploading
		// every candidate here exhausts the 32-bit address space. Preserve the
		// metadata and create the editor texture only when it becomes visible.
		img.texture = NULL;
		img.loaded = false;
		img.sizeX = 0;
		img.sizeY = 0;
		GetImageSizeFromFile(path.outstr(), &img.sizeX, &img.sizeY);
    }
    return 0;
}

int WORKSPACE::ParseSkin() {
    // Keep the passes explicit. Each phase consumes skinfileLines and only
    // writes its own derived index, making invalidation and debugging local.
    if (ParseSkinConditions() != 0) return -1;
    if (ParseSkinLegacyObjectsAndAssets() != 0) return -1;
    if (ParseSkinGraphics() != 0) return -1;
    if (ParseSkinSourcesAndDestinations() != 0) return -1;
    return LoadSkinGraphicMetadata();
}

// Structural CSV edits invalidate every editor-side row index. The LR2
// Preview has its own reload path, while Preview hit-testing, DST View, Image
// Manager and Asset Browser use these derived arrays. Rebuild them together so
// no window observes a half-old model after adding/removing commands.
int WORKSPACE::RebuildEditorDerivedState() {
    for (int i = 0; i < arr_SRCGR.count; ++i) {
        SRCGR& image = ((SRCGR*)arr_SRCGR.data)[i];
        if (image.texture) {
            image.texture->Release();
            image.texture = NULL;
        }
        for (int wildcard = 0; wildcard < image.arr_wildcard.count; ++wildcard)
            ((CSTR*)image.arr_wildcard.data)[wildcard].~CSTR();
        image.arr_wildcard.Free();
        image.path.~CSTR();
        image.filename.~CSTR();
        image.name.~CSTR();
    }
    arr_SRCGR.Free();
    arr_SRCGR.Alloc(sizeof(SRCGR), 10);

    for (int i = 0; i < arr_SRC.count; ++i)
        ((SRC*)arr_SRC.data)[i].name.~CSTR();
    arr_SRC.Free();
    arr_SRC.Alloc(sizeof(SRC), 100);

    for (int i = 0; i < arr_DST.count; ++i) {
        DST& destination = ((DST*)arr_DST.data)[i];
        destination.name.~CSTR();
        destination.arr_animation.Free();
    }
    arr_DST.Free();
    arr_DST.Alloc(sizeof(DST), 100);

    for (int i = 0; i < arr_IMG.count; ++i)
        ((IMG*)arr_IMG.data)[i].name.~CSTR();
    arr_IMG.Free();
    arr_IMG.Alloc(sizeof(IMG), 400);

    for (int i = 0; i < arr_CustomFile.count; ++i)
        ((CSTR*)arr_CustomFile.data)[i].~CSTR();
    arr_CustomFile.Free();
    arr_CustomFile.Alloc(sizeof(CSTR), 10);

    for (int i = 0; i < arr_seobj.count; ++i) {
        SEOBJ& object = ((SEOBJ*)arr_seobj.data)[i];
        object.name.~CSTR();
        for (int body = 0; body < object.body.count; ++body)
            ((CSTR*)object.body.data)[body].~CSTR();
        object.body.Free();
        for (int csv = 0; csv < object.bodyCSV.count; ++csv) {
            CSVbuf& values = ((CSVbuf*)object.bodyCSV.data)[csv];
            for (int column = 0; column < 30; ++column)
                values.str[column].~CSTR();
        }
        object.bodyCSV.Free();
        object.srcc.name.~CSTR();
        object.dstt.name.~CSTR();
        object.dstt.arr_animation.Free();
    }
    arr_seobj.Free();
    arr_seobj.Alloc(sizeof(SEOBJ), 400);

    arr_ifunit.Free();
    arr_ifunit.Alloc(sizeof(IFUNIT), 50);
    currentLeadDST = -1;
    oldIf = -1;

    for (int row = 0; row < skinfileLines.count; ++row) {
        SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
        line.ifgroup = 0;
        line.isIfGroupHead = false;
        line.isIfGroupEnd = false;
        line.isGroupHead = false;
        line.isGroupEnd = false;
        line.isObjectHead = false;
        line.isObjectEnd = false;
        line.objID = -1;
        line.objType = -1;
        line.objInTypeID = -1;
        line.isSRC = false;
        line.isDST = false;
        line.isOther = false;
    }

    const int result = ParseSkin();
    if (src_selected >= arr_IMG.count) src_selected = arr_IMG.count - 1;
    if (src_selected < 0) src_selected = 0;
    if (gr_selected >= arr_SRCGR.count) gr_selected = arr_SRCGR.count - 1;
    if (gr_selected < 0) gr_selected = 0;
    if (selected_dst >= arr_DST.count) selected_dst = arr_DST.count - 1;
    if (selected_dst < 0) selected_dst = 0;
    imageManagerFocusRequest = -1;
    return result;
}

int WORKSPACE::LoadSkin(char* path) {
    remove("SkinEditor_load_crash.log");
    WriteSkinLoadLog("LoadSkin begin", path);
    editorDerivedRebuildPending = false;
    skinSizeX = meta.targetX;
    skinSizeY = meta.targetY;

    skinfileLines.Free();
    skinfileLines.Alloc(sizeof(SKINFILELINEREAD), 1000);
    arr_subpath.Free();
    arr_subpath.Alloc(sizeof(CSTR), 1);
    arr_ifunit.Free();
    arr_ifunit.Alloc(sizeof(IFUNIT), 50);

    // Release the editor-side D3D textures before discarding their records.
    // This is especially important when switching away from an HD skin with
    // hundreds of customization images.
    for (int i = 0; i < arr_SRCGR.count; ++i) {
        SRCGR& oldImage = ((SRCGR*)arr_SRCGR.data)[i];
        if (oldImage.texture != NULL) {
            oldImage.texture->Release();
            oldImage.texture = NULL;
        }
    }
    arr_SRCGR.Free();
    arr_SRCGR.Alloc(sizeof(SRCGR), 10);
    arr_SRC.Free();
    arr_SRC.Alloc(sizeof(SRC), 100);
    arr_CustomFile.Free();
    arr_CustomFile.Alloc(sizeof(CSTR), 10);

    for (int i = 0; i < arr_DST.count; i++)
        ((DST*)arr_DST.data)[i].arr_animation.Free();
    arr_DST.Free();
    arr_DST.Alloc(sizeof(DST), 100);
    for (int i = 0; i < arr_DST.count; i++)
        ((DST*)arr_DST.data)[i].arr_animation.Alloc(sizeof(DST_ANIMATION), 1);

    arr_IMG.Free();
    arr_IMG.Alloc(sizeof(IMG),400);
    arr_seobj.Free();
    arr_seobj.Alloc(sizeof(SEOBJ),400);

    arr_history.Free();
    arr_history.Alloc(sizeof(HISTORY), 1);
    historyDocumentSnapshots.clear();
    pendingHistorySnapshotRestore = -1;
    pendingObjectReorder = false;
    pendingObjectReorderSource = SEObjectSelectionKey();
    pendingObjectReorderTarget = SEObjectSelectionKey();
    assetBrowserFocusRequest = -1;

    //LR2
    // Delete graph handles owned by the previously loaded preview before the
    // skstruct is reset. Otherwise memset loses the handles and leaks all
    // DxLib graph memory across skin changes.
    for (int i = 0; i < g.skstruct.count && i < 200; ++i) {
        if (g.skstruct.GrHandle[i] >= 0) {
            DeleteGraph(g.skstruct.GrHandle[i]);
            g.skstruct.GrHandle[i] = -1;
        }
    }

    // Changing DxLib's graph mode invalidates every graph handle. Do it before
    // loading the new skin; doing it after ReadSkinSE made HD skins lose all
    // of the images that had just been loaded. 640x480 skins appeared to work
    // only because they often did not trigger a real device reset.
    DeleteGraph(previewScreen);
    previewScreen = -1;
    if (texture_preview) {
        texture_preview->Release();
        texture_preview = NULL;
    }
    texture_preview_width = 0;
    texture_preview_height = 0;
    previewLastRenderAt = 0;
    previewTextureDirty = true;
    SetGraphMode(skinSizeX, skinSizeY, 32, 60);
    SetDrawScreen(DX_SCREEN_BACK);

    for (int i = 0; i < 200; i++) g.skstruct.caption[i].fillzero();
    for (int i = 0; i < 10; i++) g.skstruct.helpfilePath[i].fillzero();
    for (int i = 0; i < 20; i++) g.skstruct.customfileRANDOM[i].fillzero();
    for (int i = 0; i < 20; i++) g.skstruct.customfile[i].fillzero();
    g.skstruct.skinMD5.fillzero();
    g.skstruct.skFontname.fillzero();
    memset(&g.skstruct, 0, sizeof(skstruct));
    for (int i = 0; i < 200; i++) g.skstruct.caption[i].fillzero();
    for (int i = 0; i < 200; i++) g.skstruct.caption[i].assign("(null)");
    for (int i = 0; i < 200; i++) g.skstruct.GrHandle[i] = -1;
    for (int i = 0; i < 10; i++) g.skstruct.helpfilePath[i].fillzero();
    for (int i = 0; i < 10; i++) g.skstruct.helpfilePath[i].assign("(null)");
    //g.skstruct.skFontname.assign(&config.skin.fontname);
    //g.skstruct.disableimagefont = (config.skin.disableimagefont != 0);
    g.skstruct.skinMD5.fillzero();
    g.skstruct.skinMD5.resize2(34);
    AllocDrawingBuffer(&g.skstruct.drBuf);
    // SkinEditor does not provide audio playback. Keep every inherited LR2
    // audio entry point on its disabled path so fmod.dll is optional.
    g.audio.is_fmod_disabled = 1;
    //LoadScene(&g.skstruct, mainpath, 0, 0);

    //SE
    WriteSkinLoadLog("LoadSkinScript begin", path);
    loadScriptStack.clear();
    if (LoadSkinScript(path) != 0) {
        WriteSkinLoadLog("LoadSkinScript failed", path);
        return -1;
    }
    WriteSkinLoadLog("LoadSkinScript complete");

    // Dynamic #IF options (clear/fail, rank, key mode, difficulty, etc.) are
    // used while ParseSkin decides which image branches to keep. Prepare the
    // standalone scene state first; doing this after ParseSkin left complex
    // RESULT skins with only their unconditional chart objects.
    LR2SEInit(&g);
    LR2SEPreparePreviewState(&g, meta.type);
    WriteSkinLoadLog("LR2SE preview state prepared");
    ParseSkin();
    WriteSkinLoadLog("ParseSkin complete");
    if (objectEditorModel.Groups().empty())
        objectEditorModel.LoadGroups("..\\skinObjGroup.txt");
    RebuildObjectModel();
    WriteSkinLoadLog("ObjectEditor Rebuild complete");
    
    WriteSkinLoadLog("LR2SEInit complete");
    LoadSceneSE();
    WriteSkinLoadLog("LoadSceneSE complete");
    LR2SEResetRenderFault();
    previewReloadPending = false;

    //MakeObjects();
    





    SetWindowUserCloseEnableFlag(0); //DxLib

    
    //previewScreen = LoadGraph("ex.bmp");
    //previewScreen = MakeGraph(skinSizeX, skinSizeY); //MakeScreen vs MakeGraph
    //SetDrawScreen(previewScreen);
    previewScreen = MakeSoftImage(skinSizeX, skinSizeY);
    WriteSkinLoadLog("LoadSkin complete");
    //previewScreen = MakeARGB8ColorSoftImage(skinSizeX, skinSizeY); //for SDL3

    wPreview = 1;
    wObjectEditor = true;
    ClearObjectSelection();
    wImgManager = true;
    wAssetBrowser = true;
    wDstView = true;
    ImageManagerZoom = 0.0f;
    imagePixelPaintMode = false;
    imagePixelPaintLastX = -1;
    imagePixelPaintLastY = -1;
    imagePixelPaintLastButton = -1;
    imagePixelPaintDirtyPaths.clear();
    imagePixelPaintStatus.clear();
    imageNewDialogRequested = false;
    imageMergeDialogRequested = false;
    imageToolOutputPathUtf8[0] = '\0';
    imageToolStatus.clear();
    imageManagerGeneratedGrFocusRequest = -1;
    assetThumbnailSize = 96.0f;
    assetSearch[0] = '\0';
    imageManagerFocusRequest = -1;
    newObjectCsvInitialized = false;
    newObjectInitializedCommand = -1;
    newObjectAssetIndex = -1;
    newObjectFocusRequest = false;
    DstViewZoom = 0.0f;
    wOpList = true;
    wCustomize = true;
    wTreeView = false;
    dockLayoutBuilt = false;
    wObjectManagerTest = false;
    documentRevision = 0;
    savedDocumentRevision = 0;
    lastSaveState = 0;
    lastSaveMessage.clear();
    lastSaveMessageAt = 0;
    newPath[0] = '\0';
    workspaceResolutionDialogRequested = false;
    workspaceResolutionState = 0;
    workspaceResolutionMessage.clear();

    return 0;
}

//copied from LR2 LoadScene
int WORKSPACE::LoadSceneSE() {
    skstruct* sk = &g.skstruct;

    InitSkin(sk, 0, false);

    //copy of ReadSkinCustomize()
    sk->adjust.dark_type = 0;
    sk->adjust.judge_x = 0;
    sk->adjust.judge_y = 0;
    sk->adjust.note_1p_x = 0;
    sk->adjust.note_1p_y = 0;
    sk->adjust.note_2p_x = 0;
    sk->adjust.note_2p_y = 0;
    sk->adjust.unk18 = 0;
    sk->adjust.unk1c = 0;
    sk->adjust.size_x = 0;
    sk->adjust.size_y = 0;
    sk->adjust.shift_x = 0;
    sk->adjust.shift_y = 0;
    sk->adjust.rate_x = 100;
    sk->adjust.rate_y = 100;
    
    // Apply the skin list's selected customization before evaluating #IF.
    // Many real skins place the whole visible layout under options 900-999;
    // leaving those options unset produces a valid but completely black preview.
    sk->op[0] = 1;
    sk->op[999] = 0;
    // DECIDE skins cannot inherit #IMAGE,CONTINUE resources when opened as a
    // standalone editor document.  Prefer an explicitly supplied SELECT LINK
    // OFF branch so its self-contained image table is used.
    if (meta.type == SKINTYPE_DECIDE) {
        for (int i = 0; i < meta.custom_count; ++i) {
            SkinCustom& custom = meta.customs[i];
            if (_stricmp(custom.title.outstr(), "SELECT LINK") != 0) continue;
            for (int option = 0; option < custom.dst_op_count; ++option) {
                if (_stricmp(custom.op_label[option].outstr(), "OFF") == 0) {
                    custom.dst_op_selected = option;
                    break;
                }
            }
        }
    }
    for (int i = 0; i < meta.custom_count; ++i) {
        const SkinCustom& custom = meta.customs[i];
        if (custom.dst_op_start <= 0) continue;
        for (int option = 0; option < custom.dst_op_count; ++option) {
            const int op = custom.dst_op_start + option;
            if (op >= 900 && op < 1000) sk->op[op] = 0;
        }
        const int selectedOp = custom.dst_op_start + custom.dst_op_selected;
        if (selectedOp >= 900 && selectedOp < 1000) sk->op[selectedOp] = 1;
    }
    // LR2 refreshes all built-in options immediately before loading a scene.
    // These include RESULT_CLEAR/FAIL, rank, key mode and difficulty. The
    // editor previously initialized only customization options (900-999).
    LR2SEPreparePreviewState(&g, meta.type);
    for (int option = 0; option < 900; ++option)
        sk->op[option] = GetOptionFlag_dst(&g, option) ? 1 : 0;

    const int result = ReadSkinSE();
    {
        int validImageHandles = 0;
        int drawableImages = 0;
        for (int image = 0; image < sk->count; ++image) {
            if (sk->GrHandle[image] >= 0) ++validImageHandles;
        }
        for (int source = 0; source < sk->image.srcSize; ++source) {
            if (sk->image.src[source].graphcount > 0 &&
                sk->image.dst[source].dstCount > 0)
                ++drawableImages;
        }
        char runtimeSummary[512];
        sprintf_s(runtimeSummary,
            "Runtime scene images=%d validHandles=%d imageSrc=%d drawable=%d "
            "customFiles=%d op90=%d op91=%d op200=%d op201=%d result=%d",
            sk->count, validImageHandles, sk->image.srcSize, drawableImages,
            sk->customfile_count, sk->op[90], sk->op[91], sk->op[200],
            sk->op[201], result);
        WriteSkinLoadLog(runtimeSummary);

        const int detailCount = (std::min)(sk->image.srcSize, 24);
        for (int source = 0; source < detailCount; ++source) {
            SRCstruct& src = sk->image.src[source];
            DSTstruct& dst = sk->image.dst[source];
            const int graphHandle = src.graphcount > 0 ? src.grHandles[0] : -1;
            char sourceSummary[256];
            sprintf_s(sourceSummary,
                "Runtime imageSrc[%d] gr=%d handle=%d graphs=%d dst=%d timer=%d op=%d/%d/%d",
                source, src.n, graphHandle, src.graphcount, dst.dstCount,
                dst.timer, dst.op1, dst.op2, dst.op3);
            WriteSkinLoadLog(sourceSummary);
        }
    }
    // ReadSkinSE returns the accumulated skin sort/BGA number, not a success
    // code.  Complex skins commonly return a positive value, so gating scene
    // preparation on result == 0 left SELECT state entirely uninitialized.
    if (meta.type == SKINTYPE_SELECT) {
            char selectState[512];
            sprintf_s(selectState,
                "SELECT state center=%d songs=%d imageCount=%d image0=%d image9=%d "
                "body0Graphs=%d body0Handle=%d off9=%d/%d off10=%d/%d "
                "on10=%d/%d title0Graphs=%d title0Dst=%d/%d",
                g.skstruct.BAR_CENTER, g.sSelect.bmsListCount, g.skstruct.count,
                g.skstruct.GrHandle[0], g.skstruct.GrHandle[9],
                g.skstruct.src_BAR_BODY[0].graphcount,
                g.skstruct.src_BAR_BODY[0].graphcount > 0
                    ? g.skstruct.src_BAR_BODY[0].grHandles[0] : -1,
                g.skstruct.dst_BAR_BODY_OFF[9].dstCount,
                g.skstruct.dst_BAR_BODY_OFF[9].dataSize,
                g.skstruct.dst_BAR_BODY_OFF[10].dstCount,
                g.skstruct.dst_BAR_BODY_OFF[10].dataSize,
                g.skstruct.dst_BAR_BODY_ON[10].dstCount,
                g.skstruct.dst_BAR_BODY_ON[10].dataSize,
                g.skstruct.src_BAR_TITLE[0].graphcount,
                g.skstruct.dst_BAR_TITLE[0].dstCount,
                g.skstruct.dst_BAR_TITLE[0].dataSize);
        WriteSkinLoadLog(selectState);
    }
    else if (meta.type == SKINTYPE_DECIDE) {
        int validImages = 0;
        int validSources = 0;
        int firstValidSource = -1;
        int firstValidHandle = -1;
        for (int i = 0; i < g.skstruct.count; ++i)
            if (g.skstruct.GrHandle[i] >= 0) ++validImages;
        for (int i = 0; i < g.skstruct.image.srcSize; ++i) {
            SRCstruct& src = g.skstruct.image.src[i];
            bool valid = false;
            for (int frame = 0; frame < src.graphcount; ++frame) {
                if (src.grHandles && src.grHandles[frame] >= 0) {
                    valid = true;
                    if (firstValidSource < 0) {
                        firstValidSource = i;
                        firstValidHandle = src.grHandles[frame];
                    }
                    break;
                }
            }
            if (valid) ++validSources;
        }
        char decideState[384];
        sprintf_s(decideState,
            "DECIDE state images=%d/%d sources=%d/%d first=%d:%d "
            "dst0=%d/%d proc=%d phase=%d timer0=%.0f",
            validImages, g.skstruct.count, validSources, g.skstruct.image.srcSize,
            firstValidSource, firstValidHandle,
            g.skstruct.image.srcSize > 0 ? g.skstruct.image.dst[0].dstCount : 0,
            g.skstruct.image.srcSize > 0 ? g.skstruct.image.dst[0].dataSize : 0,
            g.procSelecter, g.procPhase, GetTimeLapse(0, &g.timer1));
        WriteSkinLoadLog(decideState);
    }
    return result;
}

// Persist the resolution from the skin picker without requiring the skin to
// be loaded first. LR2 stores the target width and height in fields 6 and 7 of
// #INFORMATION. The final replace is atomic so a failed write leaves the
// original skin untouched.
static bool SaveResolutionToSkinFile(const char* path, int width, int height) {
    if (!path || !*path || width <= 0 || height <= 0) return false;
    FILE* input = fopen(path, "rb");
    if (!input) return false;
    if (fseek(input, 0, SEEK_END) != 0) { fclose(input); return false; }
    const long fileSize = ftell(input);
    if (fileSize < 0 || fseek(input, 0, SEEK_SET) != 0) { fclose(input); return false; }
    std::string contents((size_t)fileSize, '\0');
    if (fileSize > 0 && fread(&contents[0], 1, (size_t)fileSize, input) != (size_t)fileSize) {
        fclose(input);
        return false;
    }
    fclose(input);

    size_t informationStart = std::string::npos;
    size_t informationEnd = std::string::npos;

    for (size_t lineStart = 0; lineStart <= contents.size();) {
        size_t lineBreak = contents.find('\n', lineStart);
        const size_t fullEnd = lineBreak == std::string::npos ? contents.size() : lineBreak + 1;
        size_t contentEnd = lineBreak == std::string::npos ? contents.size() : lineBreak;
        if (contentEnd > lineStart && contents[contentEnd - 1] == '\r') --contentEnd;
        size_t commandStart = lineStart;
        if (commandStart == 0 && contents.size() >= 3 &&
            (unsigned char)contents[0] == 0xEF && (unsigned char)contents[1] == 0xBB &&
            (unsigned char)contents[2] == 0xBF) commandStart = 3;
        while (commandStart < contentEnd &&
            (contents[commandStart] == ' ' || contents[commandStart] == '\t')) ++commandStart;
        const size_t commandLength = contentEnd - commandStart;
        if (commandLength >= 12 &&
            _strnicmp(contents.c_str() + commandStart, "#INFORMATION", 12) == 0 &&
            (commandLength == 12 || contents[commandStart + 12] == ',' ||
                contents[commandStart + 12] == ' ' || contents[commandStart + 12] == '\t')) {
            informationStart = commandStart;
            informationEnd = contentEnd;
            break;
        }
        if (lineBreak == std::string::npos) break;
        lineStart = fullEnd;
    }

    if (informationStart == std::string::npos) return false;

    std::string information = contents.substr(informationStart, informationEnd - informationStart);
    std::vector<std::string> fields;
    size_t fieldStart = 0;
    for (;;) {
        const size_t comma = information.find(',', fieldStart);
        fields.push_back(information.substr(fieldStart,
            comma == std::string::npos ? std::string::npos : comma - fieldStart));
        if (comma == std::string::npos) break;
        fieldStart = comma + 1;
    }
    while (fields.size() < 8) fields.push_back("");
    fields[6] = std::to_string(width);
    fields[7] = std::to_string(height);

    std::string updatedInformation;
    for (size_t i = 0; i < fields.size(); ++i) {
        if (i) updatedInformation += ',';
        updatedInformation += fields[i];
    }
    contents.replace(informationStart, informationEnd - informationStart, updatedInformation);

    const std::string tempPath = std::string(path) + ".resolution.tmp";
    FILE* output = fopen(tempPath.c_str(), "wb");
    if (!output) return false;
    const bool writeOk = contents.empty() ||
        fwrite(contents.data(), 1, contents.size(), output) == contents.size();
    const bool closeOk = fclose(output) == 0;
    if (!writeOk || !closeOk) {
        remove(tempPath.c_str());
        return false;
    }
    if (!MoveFileExA(tempPath.c_str(), path,
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        remove(tempPath.c_str());
        return false;
    }
    return true;
}

static std::vector<unsigned char> BuildPreviewRuntimeMask(ARR& skinfileLines, skstruct* sk) {
    struct ConditionalState {
        bool outerActive = true;
        bool branchTaken = false;
        bool currentActive = true;
    };
    struct FileState {
        bool enabled = true;
        bool pendingIncludeEnabled = false;
        std::vector<ConditionalState> conditions;
    };

    std::vector<unsigned char> enabled((size_t)skinfileLines.count, 0);
    std::vector<FileState> files;

    auto currentActive = [&]() -> bool {
        if (files.empty() || !files.back().enabled) return false;
        return files.back().conditions.empty() ||
            files.back().conditions.back().currentActive;
    };
    auto matches = [&](const CSVbuf& csv) -> bool {
        for (int column = 1; column < 10; ++column) {
            const int option = csv.val[column];
            if (option < 0 || option > 999 || sk->op[option] == 0)
                return false;
        }
        return true;
    };

    for (int rowIndex = 0; rowIndex < skinfileLines.count; ++rowIndex) {
        SKINFILELINEREAD& row = ((SKINFILELINEREAD*)skinfileLines.data)[rowIndex];
        const char* text = row.line.body ? row.line.outstr() : "";

        if (row.isSEcomment && !strncmp(text, "$FILE ", 6)) {
            const bool isStart = strstr(text, " start") != NULL;
            const bool isEnd = strstr(text, " end") != NULL;
            if (isStart) {
                FileState state;
                state.enabled = files.empty() ? true : files.back().pendingIncludeEnabled;
                if (!files.empty()) files.back().pendingIncludeEnabled = false;
                files.push_back(state);
            }
            else if (isEnd && !files.empty()) {
                files.pop_back();
            }
            continue;
        }
        if (files.empty() || row.isComment) continue;

        const char* command = row.csv.str[0].body ? row.csv.str[0].outstr() : "";
        FileState& file = files.back();
        if (!_stricmp(command, "#IF")) {
            ConditionalState condition;
            condition.outerActive = currentActive();
            const bool branchMatches = matches(row.csv);
            condition.branchTaken = branchMatches;
            condition.currentActive = condition.outerActive && branchMatches;
            file.conditions.push_back(condition);
            continue;
        }
        if (!_stricmp(command, "#ELSEIF")) {
            if (!file.conditions.empty()) {
                ConditionalState& condition = file.conditions.back();
                const bool branchMatches = !condition.branchTaken && matches(row.csv);
                condition.currentActive = condition.outerActive && branchMatches;
                condition.branchTaken = condition.branchTaken || branchMatches;
            }
            continue;
        }
        if (!_stricmp(command, "#ELSE")) {
            if (!file.conditions.empty()) {
                ConditionalState& condition = file.conditions.back();
                const bool branchMatches = !condition.branchTaken;
                condition.currentActive = condition.outerActive && branchMatches;
                condition.branchTaken = true;
            }
            continue;
        }
        if (!_stricmp(command, "#ENDIF")) {
            if (!file.conditions.empty()) file.conditions.pop_back();
            continue;
        }

        const bool active = currentActive();
        if (!_stricmp(command, "#INCLUDE")) {
            file.pendingIncludeEnabled = active;
            continue;
        }
        if (!active) continue;

        enabled[(size_t)rowIndex] = 1;
        if (!_stricmp(command, "#SETOPTION")) {
            const int option = row.csv.val[1];
            if (option >= 0 && option < 1000)
                sk->op[option] = row.csv.val[2] != 0;
        }
    }
    return enabled;
}

int WORKSPACE::ReadSkinSE() {
    
    CSTR dir(mainpath);
    dir.assign(dir.getDirectory());
    bool flag_skipFont = false;

    CSVbuf csv;

    int tSkin_num = 0;
    int line = 0;
    int IFCOUNT = 0, IFSWITCH[100] = {};

    bool flipside = false;

    skstruct* sk = &g.skstruct;
    const std::vector<unsigned char> runtimeLineEnabled =
        BuildPreviewRuntimeMask(skinfileLines, sk);


    for (int i = 0; i < skinfileLines.count; i++) {

        ExpandSkinObjectMax(&g.skstruct.image, 50);
        ExpandSkinObjectMax(&g.skstruct.otherObject[0], 20);
        ExpandSkinObjectMax(&g.skstruct.otherObject[1], 20);
        ExpandSkinObjectMax(&g.skstruct.otherObject[2], 20);
        ExpandSkinObjectMax(&g.skstruct.otherObject[3], 20);
        ExpandSkinObjectMax(&g.skstruct.otherObject[4], 20);
        ExpandSkinObjectMax(&g.skstruct.otherObject[6], 20);
        ExpandSkinObjectMax(&g.skstruct.otherObject[7], 20);
        ExpandSkinObjectMax(&g.skstruct.otherObject[5], 20);


        SKINFILELINEREAD& read = ((SKINFILELINEREAD*)(skinfileLines.data))[i];
        if (!runtimeLineEnabled[(size_t)i]) continue;
        // ReadSkinSE normalizes and temporarily edits its input while parsing.
        // Never do that to the editor's authoritative line: repeated preview
        // rebuilds would otherwise remove one real character on every pass.
        CSTR fBuf(read.line);
        line++;

        char loadLine[128];
        sprintf_s(loadLine, "expanded line %d, source line %d, command %.64s", i, read.num, fBuf.outstr());
#ifdef _DEBUG
        WriteSkinLoadLog("ReadSkinSE", loadLine);
#endif

        if (read.line.length() <= 6) continue;
        if (strncmp(read.line.atPos(0), "#", 1)) continue;

        while (fBuf.length() > 0) {
            const char tail = *fBuf.atPos(fBuf.length() - 1);
            if (tail != '\r' && tail != '\n') break;
            fBuf.nullAtPos(fBuf.length() - 1);
        }
        fBuf.trimWhiteSpace();
        DealWhiteSpace(&fBuf);
        if (!fBuf.left(1).isDiff("#")) {
            if (fBuf.left(3).isSame("#IF")) {
                if (IFCOUNT != 99) {
                    IFSWITCH[IFCOUNT + 1] = 1;
                    SplitCSV(fBuf, &csv, ",");
                    for (int i = 1; i < 10; i++) {
                        if (csv.val[i] < 0 || csv.val[i]>999 || sk->op[csv.val[i]] == 0) {
                            i = 10;
                            IFSWITCH[IFCOUNT + 1] = 2;
                        }
                    }
                    IFCOUNT++;
                }
                else {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\nネスト可?な#IFの上限に達しました。\n", line, fBuf);
                    if (IFSWITCH[IFCOUNT] > 1) {
                        *fBuf.atPos(0) = '\0';
                        continue;
                    }
                }
            }
            else if (fBuf.left(7).isSame("#ELSEIF") && IFSWITCH[IFCOUNT] != 3) {
                if (IFCOUNT) {
                    if (IFSWITCH[IFCOUNT] == 1) IFSWITCH[IFCOUNT] = 3;
                    else {
                        IFSWITCH[IFCOUNT] = 1;
                        SplitCSV(fBuf, &csv, ",");
                        for (int i = 1; i < 10; i++) {
                            if (csv.val[i] < 0 || csv.val[i]>999 || sk->op[csv.val[i]] == 0) {
                                i = 10;
                                IFSWITCH[IFCOUNT] = 2;
                            }
                        }
                    }
                }
                else ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n対応する#IFが見つかりません。\n", line, fBuf);
            }
            else if (fBuf.left(5).isSame("#ELSE") && IFSWITCH[IFCOUNT] != 3) {
                if (IFCOUNT == 0) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n対応する#IFが見つかりません。\n", line, fBuf);
                }
                else {
                    IFSWITCH[IFCOUNT] = (IFSWITCH[IFCOUNT] == 1) ? 3 : 1;
                }
            }
            else if (fBuf.left(6).isSame("#ENDIF")) {
                if (IFCOUNT == 0) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n対応する#IFが見つかりません。\n", line, fBuf);
                }
                else {
                    IFSWITCH[IFCOUNT] = 0;
                    IFCOUNT--;
                }
            }
        }

        if (IFCOUNT > 0) {
            bool inactiveAncestor = false;
            for (int depth = 1; depth <= IFCOUNT; ++depth) {
                if (IFSWITCH[depth] > 1) {
                    inactiveAncestor = true;
                    break;
                }
            }
            if (inactiveAncestor) {
                // Every enclosing condition must be active. Checking only the
                // innermost level allowed a nested #IF to leak objects out of
                // an inactive parent branch and overwrite image slots.
                continue;
            }
        }


        if (!fBuf.left(1).isDiff("#")) {
            if (fBuf.left(6).isSame("#IMAGE")) {
                if (sk->count == 100) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\nこれ以上#IMAGEを登?できません。\n", line, fBuf);
                }
                else {
                    SplitCSV(fBuf, &csv, ",");
                    if (csv.str[1].isSame("CONTINUE")) {
                        sk->caption[sk->count].assign("CONTINUE");
                        sk->count++;
                    }
                    else {
                        // A failed LoadGraph still leaves the requested path in
                        // caption.  Retrying the same skin then used to skip the
                        // load forever and produced a permanently black preview.
                        if (sk->GrHandle[sk->count] == -1 ||
                            sk->caption[sk->count].isDiff(&csv.str[1]) ||
                            sk->grIsMovie[sk->count] == 1 ||
                            sk->caption[sk->count].findStrPos("*") != -1) {
                            DeleteGraph(sk->GrHandle[sk->count]);
                            sk->caption[sk->count].assign(&csv.str[1]);
                            if (csv.str[1].right(3).isSame("mpg") || csv.str[1].right(3).isSame("avi") || csv.str[1].right(3).isSame("wma") || csv.str[1].right(3).isSame("ogv")) {
                                sk->grIsMovie[sk->count] = 1;
                            }
                            else {
                                sk->grIsMovie[sk->count] = 0;
                            }
                            for (int i = 0; i < sk->customfile_count; i++) {
                                if (sk->customfileRANDOM[i].isSame(csv.str[1].left(sk->customfileRANDOM[i].length())) && sk->customfile[i].isDiff("RANDOM") != 0 && sk->customfile[i].isDiff("ERROR") && sk->customfile[i].length() > 0) {
                                    csv.str[1].replace("*", sk->customfile[i]);
                                    break;
                                }
                            }
                            char siblingImage[MAX_PATH] = {};
                            if (ResolveSiblingPlayPath(csv.str[1].outstr(), mainpath,
                                siblingImage, sizeof(siblingImage)))
                                csv.str[1].assign(siblingImage);
                            CSTR temp(GetRandomFileNoError(csv.str[1], dir), 0);
                            sk->GrHandle[sk->count] = LoadGraph(temp);
                            sk->caption[sk->count].assign(&temp);
                        }
                        sk->count++;
                    }
                }
            }
            else if (fBuf.left(5).isSame("#FONT") && !flag_skipFont) {
                if (sk->num_of_struct == 10) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\nこれ以上#FONTを登?できません。\n", line, fBuf);
                }
                else {
                    SplitCSV(fBuf, &csv, ",");
                    //sk->fontHandle[sk->num_of_struct] = CreateFontToHandle(sk->fontname, csv.val[1], csv.val[2], csv.val[3], 0, -1, 0, -1, -1); //DxLib3.02
                    sk->fontHandle[sk->num_of_struct] = CreateFontToHandle(sk->fontname, csv.val[1], csv.val[2], csv.val[3], 0, -1, 0, -1); //DxLib3.24f
                    if (sk->fontHandle[sk->num_of_struct] == -1) {
                        sk->fontHandle[sk->num_of_struct] = 0;
                    }
                    sk->num_of_struct = sk->num_of_struct + 1;
                }
            }
            else if (fBuf.left(10).isSame("#SRC_IMAGE")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->image.src[sk->image.srcSize], &csv, sk);
                if (sk->image.src[sk->image.srcSize].graphcount < 1 || sk->image.src[sk->image.srcSize].count < 1) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n画像の登?に失敗しました。\n", line, fBuf);
                }
                if (sk->image.srcSize > 0 && (sk->image.dst[sk->image.srcSize - 1].dstCount < 1 || sk->image.dst[sk->image.srcSize - 1].dataSize < 1)) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n(この行のエラ?ではありません)ひとつ前の#SRC_IMAGEに対応した#DST_IMAGEが存在しないか、登?に失敗したようです\n", line, fBuf);
                }
                sk->image.srcSize++;
            }
            else if (fBuf.left(10).isSame("#DST_IMAGE") && sk->image.srcSize > 0) {
                int oldDstCount = sk->image.dst[sk->image.srcSize - 1].dstCount;
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->image.dst[sk->image.srcSize - 1], &csv, tSkin_num);
                if (sk->image.dst[sk->image.srcSize - 1].dstCount < 1 || sk->image.dst[sk->image.srcSize - 1].dataSize < 1) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\nDSTの登?に失敗しました。DSTの一番最初がエラ?を起こしている可?性があります。\n", line, fBuf);
                }
                else if (sk->image.dst[sk->image.srcSize - 1].dstCount == oldDstCount) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\nDSTの登?に失敗しました。この行の登?のみ失敗しました。\n", line, fBuf);
                }
            }
            else if (fBuf.left(9).isSame("#SRC_TEXT")) {
                SplitCSV(fBuf, &csv, ",");
                sk->otherObject[0].src[sk->otherObject[0].srcSize].n = csv.val[1];
                sk->otherObject[0].src[sk->otherObject[0].srcSize].fontHandle = sk->fontHandle[csv.val[2]];
                sk->otherObject[0].src[sk->otherObject[0].srcSize].cycle = csv.val[2];
                sk->otherObject[0].src[sk->otherObject[0].srcSize].st = csv.val[3];
                sk->otherObject[0].src[sk->otherObject[0].srcSize].align = csv.val[4];
                sk->otherObject[0].src[sk->otherObject[0].srcSize].op1 = csv.val[5];
                sk->otherObject[0].src[sk->otherObject[0].srcSize].op2 = csv.val[6];
                sk->otherObject[0].src[sk->otherObject[0].srcSize].op3 = csv.val[7];
                sk->otherObject[0].src[sk->otherObject[0].srcSize].op4 = csv.val[8];
                sk->otherObject[0].src[sk->otherObject[0].srcSize].op5 = csv.val[9];
                sk->otherObject[0].srcSize++;
            }
            else if (fBuf.left(9).isSame("#DST_TEXT") && sk->otherObject[0].srcSize > 0) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->otherObject[0].dst[sk->otherObject[0].srcSize - 1], &csv, tSkin_num);
            }
            else if (fBuf.left(11).isSame("#SRC_SLIDER")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->otherObject[2].src[sk->otherObject[2].srcSize], &csv, sk);
                if (sk->otherObject[2].src[sk->otherObject[2].srcSize].graphcount < 1 || sk->otherObject[2].src[sk->otherObject[2].srcSize].count < 1) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n画像の登?に失敗しました。\n", line, fBuf);
                }
                if (sk->otherObject[2].srcSize > 0 && (sk->otherObject[2].dst[sk->otherObject[2].srcSize - 1].dstCount < 1 || sk->otherObject[2].dst[sk->otherObject[2].srcSize - 1].dataSize < 1)) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n(この行のエラ?ではありません)ひとつ前の#SRC_SLIDERに対応した#DST_SLIDERが存在しないか、登?に失敗したようです\n", line, fBuf);
                }
                sk->otherObject[2].srcSize++;
            }
            else if (fBuf.left(11).isSame("#DST_SLIDER") && sk->otherObject[2].srcSize > 0) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->otherObject[2].dst[sk->otherObject[2].srcSize - 1], &csv, tSkin_num);
            }
            else if (fBuf.left(11).isSame("#SRC_BUTTON")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->otherObject[1].src[sk->otherObject[1].srcSize], &csv, sk);
                if (sk->otherObject[1].src[sk->otherObject[1].srcSize].graphcount < 1 || sk->otherObject[1].src[sk->otherObject[1].srcSize].count < 1) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n画像の登?に失敗しました。\n", line, fBuf);
                }
                if (sk->otherObject[1].srcSize > 0 && (sk->otherObject[1].dst[sk->otherObject[1].srcSize - 1].dstCount < 1 || sk->otherObject[1].dst[sk->otherObject[1].srcSize - 1].dataSize < 1)) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n(この行のエラ?ではありません)ひとつ前の#SRC_BUTTONに対応した#DST_BUTTONが存在しないか、登?に失敗したようです\n", line, fBuf);
                }
                sk->otherObject[1].srcSize++;
            }
            else if (fBuf.left(11).isSame("#DST_BUTTON") && sk->otherObject[1].srcSize > 0) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->otherObject[1].dst[sk->otherObject[1].srcSize - 1], &csv, tSkin_num);

            }
            else if (fBuf.left(12).isSame("#SRC_ONMOUSE")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->otherObject[3].src[sk->otherObject[3].srcSize], &csv, sk);
                if (sk->otherObject[3].src[sk->otherObject[3].srcSize].graphcount < 1 || sk->otherObject[3].src[sk->otherObject[3].srcSize].count < 1) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n画像の登?に失敗しました。\n", line, fBuf);
                }
                if (sk->otherObject[3].srcSize > 0 && (sk->otherObject[3].dst[sk->otherObject[3].srcSize - 1].dstCount < 1 || sk->otherObject[3].dst[sk->otherObject[3].srcSize - 1].dataSize < 1)) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n(この行のエラ?ではありません)ひとつ前の#SRC_BGAに対応した#DST_BGAが存在しないか、登?に失敗したようです\n", line, fBuf);
                }
                sk->otherObject[3].srcSize++;
            }
            else if (fBuf.left(12).isSame("#DST_ONMOUSE") && sk->otherObject[3].srcSize > 0) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->otherObject[3].dst[sk->otherObject[3].srcSize - 1], &csv, tSkin_num);
            }
            else if (fBuf.left(8).isSame("#SRC_BGA")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->otherObject[4].src[sk->otherObject[4].srcSize], &csv, sk);
                if (sk->otherObject[4].src[sk->otherObject[4].srcSize].graphcount < 1 || sk->otherObject[4].src[sk->otherObject[4].srcSize].count < 1) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n画像の登?に失敗しました。\n", line, fBuf);
                }
                if (sk->otherObject[4].srcSize > 0 && (sk->otherObject[4].dst[sk->otherObject[4].srcSize - 1].dstCount < 1 || sk->otherObject[4].dst[sk->otherObject[4].srcSize - 1].dataSize < 1)) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n(この行のエラ?ではありません)ひとつ前の#SRC_BGAに対応した#DST_BGAが存在しないか、登?に失敗したようです\n", line, fBuf);
                }
                sk->otherObject[4].srcSize++;
            }
            else if (fBuf.left(8).isSame("#DST_BGA") && sk->otherObject[4].srcSize > 0) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->otherObject[4].dst[sk->otherObject[4].srcSize - 1], &csv, tSkin_num);
                tSkin_num++;
            }
            else if (fBuf.left(11).isSame("#SRC_NUMBER")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->otherObject[6].src[sk->otherObject[6].srcSize], &csv, sk);
                if (sk->otherObject[6].src[sk->otherObject[6].srcSize].graphcount < 1 || sk->otherObject[6].src[sk->otherObject[6].srcSize].count < 1) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n画像の登?に失敗しました。\n", line, fBuf);
                }
                if (sk->otherObject[6].srcSize > 0 && (sk->otherObject[6].dst[sk->otherObject[6].srcSize - 1].dstCount < 1 || sk->otherObject[6].dst[sk->otherObject[6].srcSize - 1].dataSize < 1)) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n(この行のエラ?ではあ りません)ひとつ前の#SRC_NUMBERに対応した#DST_NUMBERが 存在しないか、登?に失敗したようです\n", line, fBuf);
                }
                sk->otherObject[6].srcSize++;
            }
            else if (fBuf.left(11).isSame("#DST_NUMBER") && sk->otherObject[6].srcSize > 0) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->otherObject[6].dst[sk->otherObject[6].srcSize - 1], &csv, tSkin_num);
            }
            else if (fBuf.left(9).isSame("#SRC_MASK")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->otherObject[7].src[sk->otherObject[7].srcSize], &csv, sk);
                sk->otherObject[7].srcSize++;
            }
            else if (fBuf.left(9).isSame("#DST_MASK") && sk->otherObject[7].srcSize > 0) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->otherObject[7].dst[sk->otherObject[7].srcSize - 1], &csv, tSkin_num);
            }
            else if (fBuf.left(13).isSame("#SRC_BARGRAPH")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->otherObject[5].src[sk->otherObject[5].srcSize], &csv, sk);
                if (sk->otherObject[5].src[sk->otherObject[5].srcSize].graphcount < 1 || sk->otherObject[5].src[sk->otherObject[5].srcSize].count < 1) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n画像の登?に失敗しまし た。\n", line, fBuf);
                }
                if (sk->otherObject[5].srcSize > 0 && (sk->otherObject[5].dst[sk->otherObject[5].srcSize - 1].dstCount < 1 || sk->otherObject[5].dst[sk->otherObject[5].srcSize - 1].dataSize < 1)) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n(この行のエラ?ではあ りません)ひとつ前の#SRC_BARGRAPHに対応した#DST_BARGRAP Hが存在しないか、登?に失敗したようです\n", line, fBuf);
                }
                sk->otherObject[5].srcSize++;
            }
            else if (fBuf.left(13).isSame("#DST_BARGRAPH") && sk->otherObject[5].srcSize > 0) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->otherObject[5].dst[sk->otherObject[5].srcSize - 1], &csv, tSkin_num);
                tSkin_num++;
            }
            else if (fBuf.left(13).isSame("#SRC_BAR_BODY")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 29, line, read.line.outstr())) {
                    ReadSRC(&sk->src_BAR_BODY[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(17).isSame("#DST_BAR_BODY_OFF")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 29, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_BODY_OFF[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(16).isSame("#DST_BAR_BODY_ON")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 29, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_BODY_ON[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(11).isSame("#BAR_CENTER")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 29, line, read.line.outstr())) {
                    SplitCSV(fBuf, &csv, ",");
                    sk->BAR_CENTER = csv.val[1];
                }
            }
            else if (fBuf.left(14).isSame("#SRC_BAR_TITLE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 4, line, read.line.outstr())) {
                    ReadSRC_BAR_TITLE(&sk->src_BAR_TITLE[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(14).isSame("#DST_BAR_TITLE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 4, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_TITLE[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(11).isSame("#TRANSCOLOR")) {
                SplitCSV(fBuf, &csv, ",");
                SetTransColor(csv.val[1], csv.val[2], csv.val[3]);
            }
            else if (fBuf.left(11).isSame("#TRANSCLOLR")) {
                SplitCSV(fBuf, &csv, ",");
                SetTransColor(csv.val[1], csv.val[2], csv.val[3]);
            }
            else if (fBuf.left(12).isSame("#TRANSCLOLOR")) {
                SplitCSV(fBuf, &csv, ",");
                SetTransColor(csv.val[1], csv.val[2], csv.val[3]);
            }
            else if (fBuf.left(11).isSame("#STARTINPUT")) {
                SplitCSV(fBuf, &csv, ",");
                sk->startinput_start = csv.val[1];
                sk->startinput_rank = csv.val[2];
                sk->startinput_update = csv.val[3];
            }
            else if (fBuf.left(10).isSame("#SCENETIME")) {
                SplitCSV(fBuf, &csv, ",");
                sk->scenetime = csv.val[1];
            }
            else if (fBuf.left(8).isSame("#FADEOUT")) {
                SplitCSV(fBuf, &csv, ",");
                sk->fadeout = csv.val[1];
            }
            else if (fBuf.left(6).isSame("#CLOSE")) {
                SplitCSV(fBuf, &csv, ",");
                sk->close = csv.val[1];
            }
            else if (fBuf.left(5).isSame("#SKIP")) {
                SplitCSV(fBuf, &csv, ",");
                sk->close = csv.val[1];
            }
            else if (fBuf.left(10).isSame("#PLAYSTART")) {
                SplitCSV(fBuf, &csv, ",");
                sk->playstart = csv.val[1];
            }
            else if (fBuf.left(10).isSame("#LOADSTART")) {
                SplitCSV(fBuf, &csv, ",");
                sk->loadstart = csv.val[1];
            }
            else if (fBuf.left(8).isSame("#LOADEND")) {
                SplitCSV(fBuf, &csv, ",");
                sk->loadend = csv.val[1];
            }
            else if (fBuf.left(14).isSame("#BAR_AVAILABLE")) {
                SplitCSV(fBuf, &csv, ",");
                sk->bar_availabe_from = csv.val[1];
                sk->bar_availabe_to = csv.val[2];
            }
            else if (fBuf.left(14).isSame("#SRC_BAR_FLASH")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->src_BAR_FLASH, &csv, sk);
            }
            else if (fBuf.left(14).isSame("#DST_BAR_FLASH")) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->dst_BAR_FLASH, &csv, tSkin_num);
            }
            else if (fBuf.left(18).isSame("#DST_BAR_STAGEFILE")) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->dst_BAR_STAGEFILE, &csv, tSkin_num);
            }
            else if (fBuf.left(16).isSame("#SRC_MOUSECURSOR")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->src_MOUSECURSOR, &csv, sk);
            }
            else if (fBuf.left(16).isSame("#DST_MOUSECURSOR")) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->dst_MOUSECURSOR, &csv, tSkin_num);
            }
            else if (fBuf.left(14).isSame("#SRC_BAR_LEVEL")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 10, line, read.line.outstr())) {
                    ReadSRC(&sk->src_BAR_LEVEL[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(14).isSame("#DST_BAR_LEVEL")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 10, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_LEVEL[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(9).isSame("#SRC_NOTE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_NOTE[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(9).isSame("#SRC_MINE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_MINE[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(13).isSame("#SRC_LN_START")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_LN_START[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(11).isSame("#SRC_LN_END")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_LN_END[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(12).isSame("#SRC_LN_BODY")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_LN_BODY[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(14).isSame("#SRC_AUTO_NOTE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_AUTO_NOTE[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(14).isSame("#SRC_AUTO_MINE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_AUTO_MINE[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(18).isSame("#SRC_AUTO_LN_START")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_AUTO_LN_START[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(16).isSame("#SRC_AUTO_LN_END")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_AUTO_LN_END[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(17).isSame("#SRC_AUTO_LN_BODY")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_AUTO_LN_BODY[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(9).isSame("#DST_NOTE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadDST(&sk->dst_NOTE[csv.val[1]], &csv, tSkin_num);
                    tSkin_num += 2;
                }
            }
            else if (fBuf.left(16).isSame("#SRC_NOWJUDGE_1P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 5, line, read.line.outstr())) {
                    ReadSRC(&sk->src_NOWJUDGE_1P[csv.val[1]], &csv, sk);
                    sk->src_NOWJUDGE_1P[csv.val[1]].timer = 46;
                }
            }
            else if (fBuf.left(16).isSame("#DST_NOWJUDGE_1P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 5, line, read.line.outstr())) {
                    ReadDST(&sk->dst_NOWJUDGE_1P[csv.val[1]], &csv, tSkin_num);
                    sk->dst_NOWJUDGE_1P[csv.val[1]].timer = 46;
                }
            }
            else if (fBuf.left(16).isSame("#SRC_NOWCOMBO_1P")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->src_NOWCOMBO_1P[csv.val[1]], &csv, sk);
                sk->src_NOWCOMBO_1P[csv.val[1]].timer = 46;
            }
            else if (fBuf.left(16).isSame("#DST_NOWCOMBO_1P")) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->dst_NOWCOMBO_1P[csv.val[1]], &csv, tSkin_num);
                sk->dst_NOWJUDGE_1P[csv.val[1]].timer = 46; //???mistake?
            }
            else if (fBuf.left(16).isSame("#SRC_NOWJUDGE_2P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 5, line, read.line.outstr())) {
                    ReadSRC(&sk->src_NOWJUDGE_2P[csv.val[1]], &csv, sk);
                    sk->src_NOWJUDGE_2P[csv.val[1]].timer = 47;
                }
            }
            else if (fBuf.left(16).isSame("#DST_NOWJUDGE_2P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 5, line, read.line.outstr())) {
                    ReadDST(&sk->dst_NOWJUDGE_2P[csv.val[1]], &csv, tSkin_num);
                    sk->dst_NOWJUDGE_2P[csv.val[1]].timer = 47;
                }
            }
            else if (fBuf.left(16).isSame("#SRC_NOWCOMBO_2P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 5, line, read.line.outstr())) {
                    ReadSRC(&sk->src_NOWCOMBO_2P[csv.val[1]], &csv, sk);
                    sk->src_NOWCOMBO_2P[csv.val[1]].timer = 47;
                }
            }
            else if (fBuf.left(16).isSame("#DST_NOWCOMBO_2P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 5, line, read.line.outstr())) {
                    ReadDST(&sk->dst_NOWCOMBO_2P[csv.val[1]], &csv, tSkin_num);
                    sk->dst_NOWJUDGE_2P[csv.val[1]].timer = 47; //???mistake?
                }
            }
            else if (fBuf.left(16).isSame("#SRC_GROOVEGAUGE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadSRC(&sk->src_GROOVEGAUGE[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(16).isSame("#DST_GROOVEGAUGE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadDST(&sk->dst_GROOVEGAUGE[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(18).isSame("#SRC_GAUGECHART_1P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadSRC(&sk->src_GAUGECHART_1P[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(18).isSame("#DST_GAUGECHART_1P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadDST(&sk->dst_GAUGECHART_1P[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(18).isSame("#SRC_GAUGECHART_2P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadSRC(&sk->src_GAUGECHART_2P[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(18).isSame("#DST_GAUGECHART_2P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadDST(&sk->dst_GAUGECHART_2P[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(15).isSame("#SRC_SCORECHART")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 2, line, read.line.outstr())) {
                    ReadSRC(&sk->src_SCORECHART[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(15).isSame("#DST_SCORECHART")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 2, line, read.line.outstr())) {
                    ReadDST(&sk->dst_SCORECHART[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(9).isSame("#SRC_LINE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadSRC(&sk->src_LINE[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(9).isSame("#DST_LINE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadDST(&sk->dst_LINE[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(14).isSame("#SRC_JUDGELINE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadSRC(&sk->src_JUDGELINE[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(14).isSame("#DST_JUDGELINE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadDST(&sk->dst_JUDGELINE[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(13).isSame("#SRC_BAR_LAMP")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadSRC(&sk->src_BAR_LAMP[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(13).isSame("#DST_BAR_LAMP")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_LAMP[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(16).isSame("#SRC_BAR_MY_LAMP")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadSRC(&sk->src_BAR_MY_LAMP[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(16).isSame("#DST_BAR_MY_LAMP")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_MY_LAMP[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(19).isSame("#SRC_BAR_RIVAL_LAMP")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadSRC(&sk->src_BAR_RIVAL_LAMP[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(19).isSame("#DST_BAR_RIVAL_LAMP")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_RIVAL_LAMP[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(13).isSame("#SRC_BAR_STAR")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 5, line, read.line.outstr())) {
                    ReadSRC(&sk->src_BAR_STAR[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(13).isSame("#DST_BAR_STAR")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 5, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_STAR[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(14).isSame("#SRC_THUMBNAIL")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->src_THUMBNAIL, &csv, sk);
            }
            else if (fBuf.left(14).isSame("#DST_THUMBNAIL")) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->dst_THUMBNAIL, &csv, tSkin_num);
            }
            else if (fBuf.left(11).isSame("#SRC_README")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadSRC_BAR_TITLE(&sk->src_README[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(11).isSame("#DST_README")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadDST(&sk->dst_README[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(20).isSame("#DST_EVENT_LOADINGBG")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 4, line, read.line.outstr())) {
                    ReadDST(&sk->dst_EVENT_LOADINGBG[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(22).isSame("#SRC_EVENT_MODE_CURSOR")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->src_EVENT_MODE_CURSOR, &csv, sk);
            }
            else if (fBuf.left(25).isSame("#DST_EVENT_MODE_CURSOR_ON")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 10, line, read.line.outstr())) {
                    ReadDST(&sk->dst_EVENT_MODE_CURSOR_ON[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(26).isSame("#DST_EVENT_MODE_CURSOR_OFF")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 10, line, read.line.outstr())) {
                    ReadDST(&sk->dst_EVENT_MODE_CURSOR_OFF[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(17).isSame("#EVENT_STARTINPUT")) {
                SplitCSV(fBuf, &csv, ",");
                sk->event_STARTINPUT[0] = csv.val[1];
                sk->event_STARTINPUT[1] = csv.val[2];
                sk->event_STARTINPUT[2] = csv.val[3];
                sk->event_STARTINPUT[3] = csv.val[4];
                sk->event_STARTINPUT[4] = csv.val[5];
                sk->event_STARTINPUT[5] = csv.val[6];
                sk->event_STARTINPUT[6] = csv.val[7];
                sk->event_STARTINPUT[7] = csv.val[8];
                sk->event_STARTINPUT[8] = csv.val[9];
                sk->event_STARTINPUT[9] = csv.val[10];
            }
            else if (fBuf.left(14).isSame("#EVENT_FADEOUT")) {
                SplitCSV(fBuf, &csv, ",");
                sk->event_FADEOUT[0] = csv.val[1];
                sk->event_FADEOUT[1] = csv.val[2];
                sk->event_FADEOUT[2] = csv.val[3];
                sk->event_FADEOUT[3] = csv.val[4];
                sk->event_FADEOUT[4] = csv.val[5];
                sk->event_FADEOUT[5] = csv.val[6];
                sk->event_FADEOUT[6] = csv.val[7];
                sk->event_FADEOUT[7] = csv.val[8];
                sk->event_FADEOUT[8] = csv.val[9];
                sk->event_FADEOUT[9] = csv.val[10];
            }
            else if (fBuf.left(8).isSame("#LR2FONT") && !flag_skipFont) {
                SplitCSV(fBuf, &csv, ",");
                if (sk->num_of_ImageFont == 10) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\nこれ以上の登?はできま せん。\n", line, fBuf);
                }
                else if (csv.val[2] == 1 || sk->disableimagefont == 0) {
                    if (csv.str[1].isDiff("CONTINUE")) {
                        for (int i = 0; i < sk->customfile_count; i++) {
                            if (sk->customfileRANDOM[i].isSame(csv.str[1].left(sk->customfileRANDOM[i].length())) && sk->customfile[i].isDiff("RANDOM") && sk->customfile[i].isDiff("ERROR") && sk->customfile[i].length() > 0) {
                                csv.str[1].replace("*", sk->customfile[i]);
                                //line?
                                break;
                            }
                        }
                        ReadImageFont(GetRandomFileNoError(csv.str[1], dir), &sk->ImageFonts[sk->num_of_ImageFont]);
                    }
                    sk->num_of_ImageFont++;
                }
                else {
                    InitImageFont(&sk->ImageFonts[sk->num_of_ImageFont]);
                    sk->num_of_ImageFont++;
                }
            }
            else if (fBuf.left(9).isSame("#HELPFILE")) {
                SplitCSV(fBuf, &csv, ",");
                if (sk->helpfileCount < 10) {
                    sk->helpfilePath[sk->helpfileCount].assign(&csv.str[1]);
                    sk->helpfileCount = sk->helpfileCount + 1;
                }
            }
            else if (fBuf.left(6).isSame("#NOBGA")) {
                sk->flag_BGA = 0;
            }
            else if (fBuf.left(11).isSame("#FLIPRESULT")) {
                sk->flag_flip = true;
                sk->op[350] = false;
                sk->op[351] = true;
            }
            else if (fBuf.left(9).isSame("#FLIPSIDE")) {
                flipside = true;
            }
            else if (fBuf.left(12).isSame("#DISABLEFLIP")) {
                sk->flag_flip = false;
                sk->op[350] = true;
                sk->op[351] = false;
            }
            else if (fBuf.left(11).isSame("#TEXTMERGIN")) {
                SplitCSV(fBuf, &csv, ",");
                sk->textmergin_1 = csv.val[1];
                sk->textmergin_2 = csv.val[2];
            }
            else if (fBuf.left(12).isSame("#SCRATCHSIDE")) {
                SplitCSV(fBuf, &csv, ",");
                sk->scratchside_1 = csv.val[1];
                sk->scratchside_2 = csv.val[2];
            }
            else if (fBuf.left(8).isSame("#INCLUDE")) {
                /*SplitCSV(fBuf, &csv, ",");
                for (int i = 0; i < sk->customfile_count; i++) {
                    if (sk->customfileRANDOM[i].isSame(csv.str[1].left(sk->customfileRANDOM[i].length()))
                        && sk->customfile[i].isDiff("RANDOM") && sk->customfile[i].isDiff("ERROR")
                        && (sk->customfile[i].length() > 0)) {

                        csv.str[1].replace("*", sk->customfile[i]);
                        break;
                    }
                }
                if (tSkin_num == 0) tSkin_num = 1;
                tSkin_num += ReadSkinSE(sk, GetRandomFileNoError(csv.str[1], dir), unused, tSkin_num, sku, flag_skipFont);*/
            }
            else if (fBuf.left(13).isSame("#CUSTOMOPTION")) {
                sk->customfile_count++;
            }
            else if (fBuf.left(11).isSame("#CUSTOMFILE")) {
                SplitCSV(fBuf, &csv, ",");
                char siblingCustom[MAX_PATH] = {};
                if (ResolveSiblingPlayPath(csv.str[2].outstr(), mainpath,
                    siblingCustom, sizeof(siblingCustom)))
                    csv.str[2].assign(siblingCustom);
                sk->customfileRANDOM[sk->customfile_count].assign(&csv.str[2]);
                sk->customfile[sk->customfile_count].assign("RANDOM");
                if (sk->customfile[sk->customfile_count].isSame("RANDOM")) {
                    sk->customfile[sk->customfile_count].assign(GetRandomFile(sk->customfileRANDOM[sk->customfile_count], 1));
                }
                sk->customfile_count++;
            }
            else if (fBuf.left(13).isSame("#CUSTOMFOLDER")) {
                SplitCSV(fBuf, &csv, ",");
                sk->customfileRANDOM[sk->customfile_count].assign(&csv.str[2]);
                sk->customfile[sk->customfile_count].assign("RANDOM");
                sk->customfile_count++;
            }
            else if (fBuf.left(13).isSame("#RELOADBANNER")) {
                sk->reloadbanner = 1;
            }
            else if (fBuf.left(10).isSame("#SETOPTION")) {
                SplitCSV(fBuf, &csv, ",");
                if (csv.val[1] < 1000) {
                    sk->op[csv.val[1]] = (csv.val[2] != 0);
                }
                else {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n#SETOPTIONの第一引数(オプション値)は900?999の範囲内にして下さい。\n", line, fBuf);
                }
            }
            else if (fBuf.left(13).isSame("#SRC_BAR_RANK")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadSRC(&sk->src_BAR_RANK[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(13).isSame("#DST_BAR_RANK")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_RANK[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(14).isSame("#SRC_BAR_RIVAL")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadSRC(&sk->src_BAR_RIVAL[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(14).isSame("#DST_BAR_RIVAL")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_RIVAL[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(11).isSame("#HORIZONTAL")) {
                sk->horizontal = 1;
            }
        }
        tSkin_num++;
    }

    if(flipside)ApplyFlipside(sk);

    return tSkin_num;
}

//deprecated
//int WORKSPACE::MakeObjects() {
//
//    for (int i = 0; i < arr_DST.count; i++) {
//        SEOBJ* seobj = (SEOBJ*)arr_seobj.Get_new();
//        seobj->ID = i;
//        seobj->dst = i;
//        seobj->src = ((DST*)arr_DST.data)[i].src;
//
//        seobj->name.assign("");
//        if (((DST*)arr_DST.data)[i].op1 >= 0) {
//            seobj->name.add(dstName(((DST*)arr_DST.data)[i].op1));
//        }
//        else {
//            seobj->name.add("Not_");
//            seobj->name.add(dstName(-((DST*)arr_DST.data)[i].op1));
//        }
//
//        seobj->ifGroup = ((SRC*)arr_SRC.data)[seobj->src].ifGroup;
//        if (seobj->ifGroup) {
//            seobj->name.add("_");
//        }
//
//        seobj->igType;
//        seobj->igID;
//        
//    }
//    
//    
//
//    return 0;
//}

int WORKSPACE::SeInit() {
    zoom = 1.0f;
    ImageManagerZoom = 0.0f;
    DstViewZoom = 0.0f;
}
int WORKSPACE::SeLoadInit() {

}


int WORKSPACE::drawCustomize() {

    char title[260];
    snprintf(title, sizeof(title), "Customize##%d", num);
    ImGui::Begin(title, &wCustomize);

    for (int i = 0; i < meta.custom_count; i++) {
        SkinCustom& cu = meta.customs[i];

        const std::string customTitleUtf8 = Cp932ToUtf8(cu.title.outstr());
        ImGui::Text("%s", customTitleUtf8.c_str());
        ImGui::SameLine();
        
        char item[256];
        std::string selectedLabelUtf8 = Cp932ToUtf8(cu.op_label[cu.dst_op_selected].outstr());

        if(cu.dst_op_start)
            snprintf(item, sizeof(item), "%03d : %s", cu.dst_op_start + cu.dst_op_selected, selectedLabelUtf8.c_str());
        else
            snprintf(item, sizeof(item), "FILE : %s", selectedLabelUtf8.c_str());

        char label[32];
        snprintf(label, sizeof(label), "##%d", i);
        if (ImGui::BeginCombo(label, item, ImGuiComboFlags_WidthFitPreview))
        {
            for (int n = 0; n < cu.dst_op_count; n++)
            {
                const std::string optionLabelUtf8 = Cp932ToUtf8(cu.op_label[n].outstr());
                if (cu.dst_op_start)
                    snprintf(item, sizeof(item), "%03d : %s", cu.dst_op_start + n, optionLabelUtf8.c_str());
                else
                    snprintf(item, sizeof(item), "FILE : %s", optionLabelUtf8.c_str());

                const bool is_selected = (cu.dst_op_selected == n);
                if (ImGui::Selectable(item, is_selected))
                    cu.dst_op_selected = n;

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }
    
    ImGui::End();
    return 0;
}

int WORKSPACE::RefreshPreviewSelectionBounds() {
    bool firstBounds = true;
    bool lastBounds = true;
    float minX = 0, minY = 0, maxX = 0, maxY = 0;
    float lastMinX = 0, lastMinY = 0, lastMaxX = 0, lastMaxY = 0;
    const std::vector<SEObjectInstance>& objects = objectEditorModel.Objects();

    auto includeBounds = [](float x, float y, float w, float h, bool& empty,
        float& left, float& top, float& right, float& bottom) {
        const float frameRight = x + w;
        const float frameBottom = y + h;
        const float x1 = x < frameRight ? x : frameRight;
        const float y1 = y < frameBottom ? y : frameBottom;
        const float x2 = x > frameRight ? x : frameRight;
        const float y2 = y > frameBottom ? y : frameBottom;
        if (empty) {
            left = x1; top = y1; right = x2; bottom = y2; empty = false;
        } else {
            if (x1 < left) left = x1;
            if (y1 < top) top = y1;
            if (x2 > right) right = x2;
            if (y2 > bottom) bottom = y2;
        }
    };
    auto sameFrame = [](const DST_ANIMATION& editorFrame, const DSTdraw& runtimeFrame) {
        return std::abs(editorFrame.x - runtimeFrame.x) < 0.5f &&
            std::abs(editorFrame.y - runtimeFrame.y) < 0.5f &&
            std::abs(editorFrame.w - runtimeFrame.w) < 0.5f &&
            std::abs(editorFrame.h - runtimeFrame.h) < 0.5f;
    };

    for (int modelIndex : preview_selected_object_model_indices) {
        if (modelIndex < 0 || modelIndex >= (int)objects.size()) continue;
        const SEObjectInstance& object = objects[modelIndex];
        DST* selectedDst = NULL;
        for (int dstIndex = 0; dstIndex < arr_DST.count; ++dstIndex) {
            DST& candidate = ((DST*)arr_DST.data)[dstIndex];
            if (candidate.arr_animation.count <= 0 ||
                std::find(object.rows.begin(), object.rows.end(), candidate.declare) == object.rows.end())
                continue;
            if (!selectedDst) selectedDst = &candidate;
            if (GetOptionFlag_dst(&g, candidate.op1) && GetOptionFlag_dst(&g, candidate.op2) &&
                GetOptionFlag_dst(&g, candidate.op3)) {
                selectedDst = &candidate;
                break;
            }
        }
        if (!selectedDst) continue;
        DST_ANIMATION* frames = (DST_ANIMATION*)selectedDst->arr_animation.data;

        int textAlign = 0;
        int numberAlign = 0;
        bool isTextObject = false;
        bool isNumberObject = false;
        SRC* editorSlider = NULL;
        for (int objectRow : object.rows) {
            if (objectRow < 0 || objectRow >= skinfileLines.count) continue;
            SKINFILELINEREAD& sourceLine = ((SKINFILELINEREAD*)skinfileLines.data)[objectRow];
            if (sourceLine.csv.str[0].isSame("#SRC_TEXT")) {
                isTextObject = true;
                textAlign = sourceLine.csv.val[4];
            } else if (sourceLine.csv.str[0].isSame("#SRC_NUMBER")) {
                isNumberObject = true;
                numberAlign = sourceLine.csv.val[12];
            } else if (sourceLine.csv.str[0].isSame("#SRC_SLIDER")) {
                for (int srcIndex = 0; srcIndex < arr_SRC.count; ++srcIndex) {
                    SRC& candidate = ((SRC*)arr_SRC.data)[srcIndex];
                    if (candidate.declare == objectRow) {
                        editorSlider = &candidate;
                        break;
                    }
                }
            }
        }
        auto alignedX = [&](float x, float w) {
            if (isTextObject) {
                if (textAlign == 1) return x - w * 0.5f;
                if (textAlign == 2) return x - w;
            }
            if (isNumberObject) {
                if (numberAlign == 0) return x - w;
                if (numberAlign == 2) return x - w * 0.5f;
            }
            return x;
        };

        // SliderByTime renders the knob at the animated DST position plus
        // SRCstruct::sx/sy. Highlight that current rectangle, not the raw
        // animation endpoints stored in the CSV.
        bool includedRuntimeSlider = false;
        if (editorSlider) {
            SkinObject& runtimeSliders = g.skstruct.otherObject[2];
            for (int runtimeIndex = 0; runtimeIndex < runtimeSliders.srcSize; ++runtimeIndex) {
                SRCstruct& runtimeSrc = runtimeSliders.src[runtimeIndex];
                DSTstruct& runtimeDst = runtimeSliders.dst[runtimeIndex];
                if (runtimeDst.dstCount != selectedDst->arr_animation.count ||
                    runtimeDst.dstCount <= 0 || !runtimeDst.draw ||
                    runtimeSrc.op1 != editorSlider->muki ||
                    runtimeSrc.op2 != editorSlider->range ||
                    runtimeSrc.op3 != editorSlider->type ||
                    runtimeSrc.op4 != editorSlider->disable ||
                    runtimeDst.timer != selectedDst->timer ||
                    runtimeDst.loop != selectedDst->loop ||
                    runtimeDst.op1 != selectedDst->op1 ||
                    runtimeDst.op2 != selectedDst->op2 ||
                    runtimeDst.op3 != selectedDst->op3 ||
                    !sameFrame(frames[0], runtimeDst.draw[0]) ||
                    !sameFrame(frames[selectedDst->arr_animation.count - 1],
                        runtimeDst.draw[runtimeDst.dstCount - 1]))
                    continue;

                DSTdraw rendered = SetDSTdrawByTime(runtimeDst,
                    GetTimeLapse(runtimeDst.timer, &g.timer1));
                if (rendered.time != -1) {
                    includeBounds(rendered.x + runtimeSrc.sx, rendered.y + runtimeSrc.sy,
                        rendered.w, rendered.h, firstBounds, minX, minY, maxX, maxY);
                    includedRuntimeSlider = true;
                }
                break;
            }
        }
        if (includedRuntimeSlider) continue;

        includeBounds(alignedX(frames[0].x, frames[0].w), frames[0].y,
            frames[0].w, frames[0].h, firstBounds, minX, minY, maxX, maxY);
        DST_ANIMATION& lastFrame = frames[selectedDst->arr_animation.count - 1];
        includeBounds(alignedX(lastFrame.x, lastFrame.w), lastFrame.y,
            lastFrame.w, lastFrame.h, lastBounds,
            lastMinX, lastMinY, lastMaxX, lastMaxY);
    }

    preview_selected_obj_valid = !firstBounds;
    preview_selected_obj_last_valid = !lastBounds;
    if (!firstBounds) preview_selected_obj = { minX, minY, maxX - minX, maxY - minY };
    if (!lastBounds) preview_selected_obj_last = {
        lastMinX, lastMinY, lastMaxX - lastMinX, lastMaxY - lastMinY };
    return preview_selected_obj_valid ? 0 : -1;
}

int WORKSPACE::drawPreview() {
    // Preview texture is owned by this workspace and recreated whenever
    // the loaded skin resolution changes.
    
    char title[260];
    snprintf(title, sizeof(title), "Preview##%d", num);

    static bool playing = false;

    // An inactive dock tab returns false here. Do not run LR2 rendering or
    // transfer a full preview texture while the user cannot see it.
    if (!ImGui::Begin(title, &wPreview, ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::End();
        return 0;
    }

    const unsigned long long previewNow = GetTickCount64();
    bool previewFrameUpdated = false;

    // Object Editor changes first update the in-memory CSV. Rebuild the LR2
    // preview objects shortly after the last edit so coordinates and all DST
    // properties become visible without saving and reopening the skin.
    if (previewReloadPending && previewNow - previewReloadRequestedAt >= 80) {
        LoadSceneSE();
        LR2SEResetRenderFault();
        previewReloadPending = false;
        previewLastRenderAt = 0;
        RefreshPreviewSelectionBounds();
    }

    // Keep editing and live scene playback at the same responsive refresh
    // rate. Image candidates are now loaded lazily, so the former 15/30 FPS
    // memory-pressure workaround is no longer needed.
    const unsigned long long previewFrameInterval = 16;
    if (previewLastRenderAt == 0 || previewNow - previewLastRenderAt >= previewFrameInterval) {
        // Scene processing queues PLAY notes, judge lines and other dynamic
        // objects in skstruct.drBuf.  It must run before LR2SEDrawLoop: that
        // function draws the queued objects, captures the preview and then
        // clears the buffer.  Running this after the draw loop made every
        // sample-BMS note get cleared at the start of the following frame
        // without ever reaching the preview texture.
        if (playing && LR2SESceneProcSafe(&g, meta.type) == -1)
            playing = false;

        // SELECT still needs its editor-side bar/BGA placeholders while its
        // timers are running; the original selector loop is not driven here.
        const bool staticSpecialPreview = !playing || meta.type == SKINTYPE_SELECT;
        if (LR2SEDrawLoopSafe(&g, previewScreen, skinSizeX, skinSizeY, staticSpecialPreview) == 0) {
            previewFrameUpdated = true;
            previewTextureDirty = true;
        }
        previewLastRenderAt = previewNow;
    }

    float previewCanvasScale = 1.0f / zoom;
    float zoomPercent = previewCanvasScale * 100.0f;
    if (ImGui::SliderFloat("zoom##zoom", &zoomPercent, 25.0f, 1600.0f,
        "%.0f%%", ImGuiSliderFlags_Logarithmic)) {
        if (zoomPercent < 1.0f) zoomPercent = 1.0f;
        previewCanvasScale = zoomPercent / 100.0f;
        zoom = 1.0f / previewCanvasScale;
    }
    ImVec2 p = ImGui::GetCursorScreenPos();
    const ImVec2 oldPreviewCanvasSize(skinSizeX * previewCanvasScale,
        skinSizeY * previewCanvasScale);
    if (ApplyMouseCenteredWheelZoom(previewCanvasScale, 0.25f, 16.0f,
        p, oldPreviewCanvasSize))
        zoom = 1.0f / previewCanvasScale;
    //TODO init zoom value
    //TODO support HD skins
    
    void* previewPixels = NULL;
    if (previewTextureDirty || !texture_preview)
        previewPixels = GetImageAddressSoftImage(previewScreen);
    const size_t previewBytes = (size_t)skinSizeX * (size_t)skinSizeY * 4;
    if (texture_preview &&
        (texture_preview_width != skinSizeX || texture_preview_height != skinSizeY)) {
        texture_preview->Release();
        texture_preview = NULL;
    }
    if (!texture_preview && previewPixels) {
        if (LoadTextureFromRawMemory(previewPixels, previewBytes,
            skinSizeX, skinSizeY, &texture_preview)) {
            texture_preview_width = skinSizeX;
            texture_preview_height = skinSizeY;
            previewTextureDirty = false;
        }
    }
    else if (previewPixels && previewFrameUpdated) {
        RefreshTextureByRawMemory(previewPixels, previewBytes,
            skinSizeX, skinSizeY, &texture_preview);
        previewTextureDirty = false;
    }

    // An Image does not claim mouse input, so dragging an Object over it can
    // also start ImGui's window move behavior. Use a transparent button as the
    // preview canvas so the canvas owns the drag from its first frame.
    const ImVec2 previewCanvasSize(skinSizeX * previewCanvasScale,
        skinSizeY * previewCanvasScale);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 0, 0, 0));
    const bool sharpPreview = BeginSharpMagnifiedCanvas(previewCanvasScale);
    ImGui::ImageButton("##preview_canvas", texture_preview, previewCanvasSize,
        ImVec2(0, 0), ImVec2(1, 1));
    EndSharpMagnifiedCanvas(sharpPreview);
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    // Asset Browser sends a stable IMG index. Preview owns coordinate
    // conversion because it is the only window that knows the current canvas
    // origin, scroll and zoom. Delivery opens the existing New Object form;
    // no CSV or runtime Object is mutated until the user confirms that form.
    if (ImGui::BeginDragDropTarget()) {
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(
            "SKINEDITOR_IMG_ASSET",
            ImGuiDragDropFlags_AcceptBeforeDelivery |
            ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
        if (payload && payload->DataSize == sizeof(int)) {
            const int assetIndex = *(const int*)payload->Data;
            if (assetIndex >= 0 && assetIndex < arr_IMG.count) {
                IMG& asset = ((IMG*)arr_IMG.data)[assetIndex];
                const int textureIndex = ResolveIMGTextureIndex(assetIndex);
                SRCGR* source = textureIndex >= 0 && textureIndex < arr_SRCGR.count
                    ? &((SRCGR*)arr_SRCGR.data)[textureIndex] : NULL;
                int cropW = asset.w;
                int cropH = asset.h;
                if (source && source->sizeX > 0 && source->sizeY > 0) {
                    if (cropW == -1) cropW = source->sizeX - asset.x;
                    if (cropH == -1) cropH = source->sizeY - asset.y;
                }
                cropW = (std::max)(1, cropW);
                cropH = (std::max)(1, cropH);

                int divX = 1;
                int divY = 1;
                int cycle = 0;
                int timer = 0;
                ResolveIMGDivision(assetIndex, divX, divY, cycle, timer);
                (void)timer;
                const long long frameCount64 = (long long)divX * (long long)divY;
                const int frameCount = frameCount64 > 0 && frameCount64 <= INT_MAX
                    ? (int)frameCount64 : 1;
                int frame = 0;
                if (assetAnimateSrc && cycle > 0 && frameCount > 1) {
                    const long long elapsedMs =
                        (long long)(ImGui::GetTime() * 1000.0);
                    frame = (int)(((elapsedMs % cycle) * frameCount) / cycle);
                    if (frame >= frameCount) frame = frameCount - 1;
                }
                const int frameX = frame % divX;
                const int frameY = frame / divX;
                const int frameCropX0 = asset.x + (cropW * frameX) / divX;
                const int frameCropY0 = asset.y + (cropH * frameY) / divY;
                const int frameCropX1 = asset.x + (cropW * (frameX + 1)) / divX;
                const int frameCropY1 = asset.y + (cropH * (frameY + 1)) / divY;
                const int frameW = (std::max)(1, frameCropX1 - frameCropX0);
                const int frameH = (std::max)(1, frameCropY1 - frameCropY0);

                const ImVec2 mouse = ImGui::GetIO().MousePos;
                const int dropX = (std::max)(0, (std::min)(skinSizeX,
                    (int)floorf((mouse.x - p.x) / previewCanvasScale)));
                const int dropY = (std::max)(0, (std::min)(skinSizeY,
                    (int)floorf((mouse.y - p.y) / previewCanvasScale)));
                const ImVec2 ghostMin(p.x + dropX * previewCanvasScale,
                    p.y + dropY * previewCanvasScale);
                const ImVec2 ghostMax(ghostMin.x + frameW * previewCanvasScale,
                    ghostMin.y + frameH * previewCanvasScale);
                const ImVec2 previewMax(p.x + previewCanvasSize.x,
                    p.y + previewCanvasSize.y);
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                drawList->PushClipRect(p, previewMax, true);
                if (source && source->texture && source->sizeX > 0 && source->sizeY > 0) {
                    const int cropX0 = (std::max)(0,
                        (std::min)(source->sizeX, frameCropX0));
                    const int cropY0 = (std::max)(0,
                        (std::min)(source->sizeY, frameCropY0));
                    const int cropX1 = (std::max)(cropX0,
                        (std::min)(source->sizeX, frameCropX1));
                    const int cropY1 = (std::max)(cropY0,
                        (std::min)(source->sizeY, frameCropY1));
                    const ImVec2 uv0(cropX0 / (float)source->sizeX,
                        cropY0 / (float)source->sizeY);
                    const ImVec2 uv1(cropX1 / (float)source->sizeX,
                        cropY1 / (float)source->sizeY);
                    const bool sharpGhost = BeginSharpMagnifiedCanvas(previewCanvasScale);
                    drawList->AddImage(source->texture, ghostMin, ghostMax, uv0, uv1,
                        IM_COL32(255, 255, 255, 180));
                    EndSharpMagnifiedCanvas(sharpGhost);
                } else {
                    drawList->AddRectFilled(ghostMin, ghostMax,
                        IM_COL32(76, 151, 255, 75));
                }
                drawList->AddRect(ghostMin, ghostMax,
                    IM_COL32(76, 190, 255, 255), 0.0f, 0, 2.0f);
                drawList->PopClipRect();

                if (payload->IsDelivery())
                    OpenNewObjectFromAsset(assetIndex, dropX, dropY);
            }
        }
        ImGui::EndDragDropTarget();
    }

    // Slider positions can change every scene frame without editing the CSV.
    // Refresh only slider selections here so the highlight follows the knob,
    // while an active preview drag keeps its own temporary coordinates.
    if (!preview_object_dragging && !preview_selected_object_model_indices.empty()) {
        bool sliderSelected = false;
        const std::vector<SEObjectInstance>& selectedObjects = objectEditorModel.Objects();
        for (int modelIndex : preview_selected_object_model_indices) {
            if (modelIndex < 0 || modelIndex >= (int)selectedObjects.size()) continue;
            for (int row : selectedObjects[modelIndex].rows) {
                if (row < 0 || row >= skinfileLines.count) continue;
                SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                if (line.csv.str[0].isSame("#SRC_SLIDER")) {
                    sliderSelected = true;
                    break;
                }
            }
            if (sliderSelected) break;
        }
        if (sliderSelected) RefreshPreviewSelectionBounds();
    }

    if (ImGui::Button("MainStart")) {
        LoadSceneSE();
        playing = (LR2SESceneInitSafe(&g, meta.type) == 0);
    }

    if (ImGui::BeginCombo("##Timer",timerName(timerSelected))) {
        for (int timer = 0; timer < 200; timer++) {
            ImGui::PushID(timer);
            const bool is_selected = (timerSelected == timer);
            char timerNameBuf[64];
            sprintf(timerNameBuf, "%03d:%s", timer, timerName(timer));
            if (ImGui::Selectable(timerNameBuf, is_selected))
                timerSelected = timer;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
            

            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Start")) {
        SetTimeLapse(timerSelected, &g.timer1);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        ResetTimeLapse(timerSelected, &g.timer1);
    }

    

    //D_IDirect3DSurface9* d9 = (D_IDirect3DSurface9*)GetUseDirect3D9BackBufferSurface();
    //d9->GetDC()

    // Drag the highlighted Object. All DST animation rows receive the same
    // delta, preserving animation while EditValue records CSV and History.
    if (preview_selected_obj_valid) {
        const float previewScale = 1.0f / zoom;
        float hitX1 = preview_selected_obj.x;
        float hitY1 = preview_selected_obj.y;
        float hitX2 = preview_selected_obj.x + preview_selected_obj.w;
        float hitY2 = preview_selected_obj.y + preview_selected_obj.h;
        if (hitX1 > hitX2) std::swap(hitX1, hitX2);
        if (hitY1 > hitY2) std::swap(hitY1, hitY2);
        // Keep at least a 20x20 screen-space grab area. Very small or thin
        // LR2 objects are otherwise nearly impossible to catch with a mouse.
        const float grabPadding = 10.0f;
        const ImVec2 hitMin(p.x + hitX1 * previewScale - grabPadding,
            p.y + hitY1 * previewScale - grabPadding);
        const ImVec2 hitMax(p.x + hitX2 * previewScale + grabPadding,
            p.y + hitY2 * previewScale + grabPadding);
        const ImVec2 imageMax(p.x + skinSizeX * previewScale,
            p.y + skinSizeY * previewScale);
        const ImVec2 mousePos = ImGui::GetIO().MousePos;
        const bool overPreviewImage = mousePos.x >= p.x && mousePos.x <= imageMax.x &&
            mousePos.y >= p.y && mousePos.y <= imageMax.y;
        const bool overSelectedObject = overPreviewImage &&
            mousePos.x >= hitMin.x && mousePos.x <= hitMax.x &&
            mousePos.y >= hitMin.y && mousePos.y <= hitMax.y;

        if (overSelectedObject || preview_object_dragging)
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        if (overSelectedObject && !preview_object_dragging)
            ImGui::SetTooltip("Drag to move Object");

        // Read MouseDown directly. The preview Image may already own the
        // frame's click, so relying on IsMouseClicked can miss the drag start.
        if (!preview_object_dragging && overSelectedObject && ImGui::GetIO().MouseDown[ImGuiMouseButton_Left]) {
            preview_object_dragging = true;
            preview_drag_mouse_start = mousePos;
            preview_drag_object_start_x = preview_selected_obj.x;
            preview_drag_object_start_y = preview_selected_obj.y;
            preview_drag_last_start_x = preview_selected_obj_last.x;
            preview_drag_last_start_y = preview_selected_obj_last.y;
        }

        if (preview_object_dragging) {
            const ImVec2 mouseNow = ImGui::GetIO().MousePos;
            const float deltaX = (mouseNow.x - preview_drag_mouse_start.x) * zoom;
            const float deltaY = (mouseNow.y - preview_drag_mouse_start.y) * zoom;
            preview_selected_obj.x = preview_drag_object_start_x + deltaX;
            preview_selected_obj.y = preview_drag_object_start_y + deltaY;
            if (preview_selected_obj_last_valid) {
                preview_selected_obj_last.x = preview_drag_last_start_x + deltaX;
                preview_selected_obj_last.y = preview_drag_last_start_y + deltaY;
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                const int moveX = (int)std::round(deltaX);
                const int moveY = (int)std::round(deltaY);
                if ((moveX != 0 || moveY != 0) && !preview_selected_object_model_indices.empty()) {
                    for (int selectedModelIndex : preview_selected_object_model_indices) {
                    if (selectedModelIndex < 0 ||
                        selectedModelIndex >= (int)objectEditorModel.Objects().size()) continue;
                    const SEObjectInstance& selectedObject = objectEditorModel.Objects()[selectedModelIndex];
                    for (int rowIndex = 0; rowIndex < (int)selectedObject.rows.size(); ++rowIndex) {
                        const int row = selectedObject.rows[rowIndex];
                        if (row < 0 || row >= skinfileLines.count) continue;
                        SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                        if (!line.csv.str[0].body || strncmp(line.csv.str[0].outstr(), "#DST", 4) != 0) continue;
                        EditValue(row, 3, line.csv.val[3] + moveX);
                        EditValue(row, 4, line.csv.val[4] + moveY);
                    }
                    for (int dstIndex = 0; dstIndex < arr_DST.count; ++dstIndex) {
                        DST& dst = ((DST*)arr_DST.data)[dstIndex];
                        bool belongsToObject = false;
                        for (int rowIndex = 0; rowIndex < (int)selectedObject.rows.size(); ++rowIndex) {
                            if (dst.declare == selectedObject.rows[rowIndex]) { belongsToObject = true; break; }
                        }
                        if (!belongsToObject) continue;
                        for (int frameIndex = 0; frameIndex < dst.arr_animation.count; ++frameIndex) {
                            DST_ANIMATION& frame = ((DST_ANIMATION*)dst.arr_animation.data)[frameIndex];
                            frame.x += moveX;
                            frame.y += moveY;
                        }
                    }
                    }
                }
                preview_selected_obj.x = preview_drag_object_start_x + moveX;
                preview_selected_obj.y = preview_drag_object_start_y + moveY;
                if (preview_selected_obj_last_valid) {
                    preview_selected_obj_last.x = preview_drag_last_start_x + moveX;
                    preview_selected_obj_last.y = preview_drag_last_start_y + moveY;
                }
                preview_object_dragging = false;
            }
        }

        // Fine positioning: move every selected Object by exactly one skin
        // pixel per arrow-key step. Key repeat is enabled for held keys, but
        // text/property editing keeps ownership of the arrows.
        if (!preview_object_dragging && !ImGui::GetIO().WantTextInput && !ImGui::IsAnyItemActive()) {
            int keyMoveX = 0;
            int keyMoveY = 0;
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, true)) --keyMoveX;
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, true)) ++keyMoveX;
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, true)) --keyMoveY;
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, true)) ++keyMoveY;
            if ((keyMoveX != 0 || keyMoveY != 0) &&
                !preview_selected_object_model_indices.empty()) {
                for (int selectedModelIndex : preview_selected_object_model_indices) {
                    if (selectedModelIndex < 0 ||
                        selectedModelIndex >= (int)objectEditorModel.Objects().size()) continue;
                    const SEObjectInstance& selectedObject =
                        objectEditorModel.Objects()[selectedModelIndex];
                    for (int rowIndex = 0; rowIndex < (int)selectedObject.rows.size(); ++rowIndex) {
                        const int row = selectedObject.rows[rowIndex];
                        if (row < 0 || row >= skinfileLines.count) continue;
                        SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                        if (!line.csv.str[0].body ||
                            strncmp(line.csv.str[0].outstr(), "#DST", 4) != 0) continue;
                        EditValue(row, 3, line.csv.val[3] + keyMoveX);
                        EditValue(row, 4, line.csv.val[4] + keyMoveY);
                    }
                    for (int dstIndex = 0; dstIndex < arr_DST.count; ++dstIndex) {
                        DST& dst = ((DST*)arr_DST.data)[dstIndex];
                        bool belongsToObject = false;
                        for (int rowIndex = 0; rowIndex < (int)selectedObject.rows.size(); ++rowIndex) {
                            if (dst.declare == selectedObject.rows[rowIndex]) {
                                belongsToObject = true;
                                break;
                            }
                        }
                        if (!belongsToObject) continue;
                        for (int frameIndex = 0; frameIndex < dst.arr_animation.count; ++frameIndex) {
                            DST_ANIMATION& frame = ((DST_ANIMATION*)dst.arr_animation.data)[frameIndex];
                            frame.x += keyMoveX;
                            frame.y += keyMoveY;
                        }
                    }
                }
                preview_selected_obj.x += keyMoveX;
                preview_selected_obj.y += keyMoveY;
                if (preview_selected_obj_last_valid) {
                    preview_selected_obj_last.x += keyMoveX;
                    preview_selected_obj_last.y += keyMoveY;
                }
            }
        }
    }
    // Flash the selected Object's destination rectangle over the preview.
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if (preview_selected_obj_valid && ((int)GetTimeLapse(1, &g.timer1) % 500 < 300)) {
        const float previewScale = 1.0f / zoom;
        auto drawDstBounds = [&](const PreviewSelectionBounds& bounds, ImU32 color) {
            float x1 = bounds.x;
            float y1 = bounds.y;
            float x2 = bounds.x + bounds.w;
            float y2 = bounds.y + bounds.h;
            if (x1 > x2) std::swap(x1, x2);
            if (y1 > y2) std::swap(y1, y2);
            ImVec2 dstTopLeft(p.x + x1 * previewScale - 2.0f,
                p.y + y1 * previewScale - 2.0f);
            ImVec2 dstBottomRight(p.x + x2 * previewScale + 2.0f,
                p.y + y2 * previewScale + 2.0f);
            draw_list->AddRect(dstTopLeft, dstBottomRight, color,
                0.0f, ImDrawFlags_Closed, 3.0f);
            draw_list->AddRect(ImVec2(dstTopLeft.x + 2, dstTopLeft.y + 2),
                ImVec2(dstBottomRight.x - 2, dstBottomRight.y - 2),
                IM_COL32(255, 255, 255, 210), 0.0f, ImDrawFlags_Closed, 1.0f);
        };
        // Cyan marks the starting DST frame; red marks the final destination.
        drawDstBounds(preview_selected_obj, IM_COL32(32, 210, 255, 255));
        if (preview_selected_obj_last_valid)
            drawDstBounds(preview_selected_obj_last, IM_COL32(255, 48, 48, 255));
    }

    //test objects on cursor
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGuiIO& io = ImGui::GetIO();
        clickPos = { io.MousePos.x,io.MousePos.y };
        drawRightClick = true;
    }
    // Do not clear drawRightClick on a left press here.  The popup items are
    // processed later in this function; clearing it first skipped the entire
    // popup block on the exact frame a Selectable was clicked.
    preview_hover_obj_valid = false;
    if(drawRightClick){
        ImGui::PushID(num);
        if (ImGui::BeginPopupContextWindow()) {
            auto branchConditionMatches = [&](int ifgroup) -> bool {
                if (ifgroup <= 0 || ifgroup >= arr_ifunit.count) return ifgroup == 0;
                for (int row = 0; row < skinfileLines.count; ++row) {
                    SKINFILELINEREAD& header = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    if (header.ifgroup != ifgroup || !header.isIfGroupHead ||
                        !header.csv.str[0].body) continue;
                    const char* command = header.csv.str[0].outstr();
                    if (!strcmp(command, "#ELSE")) return true;
                    if (strcmp(command, "#IF") && strcmp(command, "#ELSEIF")) return false;
                    for (int col = 1; col <= 10; ++col) {
                        const int option = header.csv.val[col];
                        if (option != 0 && !GetOptionFlag_dst(&g, option)) return false;
                    }
                    return true;
                }
                return false;
            };
            auto rootIfgroup = [&](int ifgroup) -> int {
                if (ifgroup <= 0 || ifgroup >= arr_ifunit.count) return 0;
                IFUNIT& unit = ((IFUNIT*)arr_ifunit.data)[ifgroup];
                if (unit.order == 0) return ifgroup;
                for (int group = ifgroup - 1; group > 0; --group) {
                    IFUNIT& candidate = ((IFUNIT*)arr_ifunit.data)[group];
                    if (candidate.depth == unit.depth && candidate.parentID == unit.parentID &&
                        candidate.order == 0) return group;
                }
                return ifgroup;
            };
            std::function<bool(int)> branchIsActive = [&](int ifgroup) -> bool {
                if (ifgroup == 0) return true;
                if (ifgroup < 0 || ifgroup >= arr_ifunit.count ||
                    !branchConditionMatches(ifgroup)) return false;
                IFUNIT& unit = ((IFUNIT*)arr_ifunit.data)[ifgroup];
                if (!branchIsActive(unit.parentID)) return false;
                const int chainRoot = rootIfgroup(ifgroup);
                for (int sibling = 1; sibling < arr_ifunit.count; ++sibling) {
                    IFUNIT& previous = ((IFUNIT*)arr_ifunit.data)[sibling];
                    if (previous.parentID == unit.parentID && previous.depth == unit.depth &&
                        previous.order < unit.order && rootIfgroup(sibling) == chainRoot &&
                        branchConditionMatches(sibling)) return false;
                }
                return true;
            };

            std::vector<int> listedObjectModels;
            for (int i = 0; i < arr_DST.count; i++) {
                DST& dst = ((DST*)arr_DST.data)[i];
                if (dst.arr_animation.count <= 0 ||
                    !GetOptionFlag_dst(&g, dst.op1) ||
                    !GetOptionFlag_dst(&g, dst.op2) ||
                    !GetOptionFlag_dst(&g, dst.op3)) continue;
                DST_ANIMATION& dstd = ((DST_ANIMATION*)dst.arr_animation.data)[dst.arr_animation.count - 1];
                const float hitScale = 1.0f / zoom;
                float hitX1 = dstd.x, hitY1 = dstd.y;
                float hitX2 = dstd.x + dstd.w, hitY2 = dstd.y + dstd.h;
                if (hitX1 > hitX2) std::swap(hitX1, hitX2);
                if (hitY1 > hitY2) std::swap(hitY1, hitY2);
                ImVec2 dstposLU = { p.x + hitX1 * hitScale, p.y + hitY1 * hitScale };
                ImVec2 dstposRB = { p.x + hitX2 * hitScale, p.y + hitY2 * hitScale };
                
                if (dstposLU.x <= clickPos.x && clickPos.x <= dstposRB.x && dstposLU.y <= clickPos.y && clickPos.y <= dstposRB.y) {
                    int objectModelIndex = -1;
                    const std::vector<SEObjectInstance>& objects = objectEditorModel.Objects();
                    for (int modelIndex = 0; modelIndex < (int)objects.size(); ++modelIndex) {
                        if (std::find(objects[modelIndex].rows.begin(), objects[modelIndex].rows.end(),
                            dst.declare) != objects[modelIndex].rows.end()) {
                            objectModelIndex = modelIndex;
                            break;
                        }
                    }
                    if (objectModelIndex < 0 ||
                        !branchIsActive(objects[objectModelIndex].ifgroup)) continue;
                    if (std::find(listedObjectModels.begin(), listedObjectModels.end(), objectModelIndex) !=
                        listedObjectModels.end()) continue;
                    listedObjectModels.push_back(objectModelIndex);

                    // Resolve the thumbnail from this Object's matching SRC
                    // row, rather than trusting DST::src's parser-global
                    // sequence number.  Indexed object families can contain
                    // several SRC/DST command pairs, so also match the command
                    // suffix and index column.
                    int thumbnailSrc = -1;
                    const int dstRow = dst.declare;
                    SKINFILELINEREAD* dstLine = dstRow >= 0 && dstRow < skinfileLines.count
                        ? &((SKINFILELINEREAD*)skinfileLines.data)[dstRow] : NULL;
                    std::string wantedSrcCommand;
                    int wantedKey = 0;
                    bool hasWantedKey = false;
                    if (dstLine && dstLine->csv.str[0].body) {
                        const char* dstCommand = dstLine->csv.str[0].outstr();
                        if (!strncmp(dstCommand, "#DST_", 5))
                            wantedSrcCommand = std::string("#SRC_") + (dstCommand + 5);
                        if (dstLine->csv.str[1].body) {
                            wantedKey = dstLine->csv.val[1];
                            hasWantedKey = true;
                        }
                    }
                    int fallbackSrc = -1;
                    for (int objectRow : objects[objectModelIndex].rows) {
                        if (objectRow < 0 || objectRow >= skinfileLines.count) continue;
                        SKINFILELINEREAD& sourceLine =
                            ((SKINFILELINEREAD*)skinfileLines.data)[objectRow];
                        const char* sourceCommand = sourceLine.csv.str[0].body
                            ? sourceLine.csv.str[0].outstr() : "";
                        if (strncmp(sourceCommand, "#SRC_", 5)) continue;
                        int sourceArrayIndex = -1;
                        for (int sourceIndex = 0; sourceIndex < arr_SRC.count; ++sourceIndex) {
                            if (((SRC*)arr_SRC.data)[sourceIndex].declare == objectRow) {
                                sourceArrayIndex = sourceIndex;
                                break;
                            }
                        }
                        if (sourceArrayIndex < 0) continue;
                        if (fallbackSrc < 0) fallbackSrc = sourceArrayIndex;
                        const bool commandMatches = wantedSrcCommand.empty() ||
                            wantedSrcCommand == sourceCommand;
                        const bool keyMatches = !hasWantedKey || !sourceLine.csv.str[1].body ||
                            sourceLine.csv.val[1] == wantedKey;
                        if (commandMatches && keyMatches) {
                            thumbnailSrc = sourceArrayIndex;
                            break;
                        }
                    }
                    if (thumbnailSrc < 0) thumbnailSrc = fallbackSrc;
                    if (thumbnailSrc < 0 && dst.src >= 0 && dst.src < arr_SRC.count)
                        thumbnailSrc = dst.src;

                    char hitLabel[256];
                    const std::string hitName = Cp932ToUtf8(objects[objectModelIndex].name.c_str());
                    snprintf(hitLabel, sizeof(hitLabel), "Object %03d  %s##PreviewHit%d",
                        objectModelIndex, hitName.c_str(), objectModelIndex);
                    const ImVec2 hitRowStart = ImGui::GetCursorScreenPos();
                    bool chooseObject = ImGui::Selectable(hitLabel, false,
                        ImGuiSelectableFlags_AllowDoubleClick, ImVec2(310.0f, 64.0f));
                    bool hoverObject = ImGui::IsItemHovered();
                    ImGui::SetCursorScreenPos(ImVec2(hitRowStart.x + 215.0f, hitRowStart.y + 2.0f));
                    if (thumbnailSrc >= 0 && thumbnailSrc < arr_SRC.count &&
                        ((SRC*)arr_SRC.data)[thumbnailSrc].gr != 110 &&
                        ((SRC*)arr_SRC.data)[thumbnailSrc].gr != 111) {
                        printSrcImgEx(((SRC*)arr_SRC.data)[thumbnailSrc], 90, 60);
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) chooseObject = true;
                        if (ImGui::IsItemHovered()) hoverObject = true;
                    } else {
                        ImGui::TextDisabled("Special source");
                        if (ImGui::IsItemHovered()) hoverObject = true;
                    }
                    // SetCursorScreenPos() alone no longer contributes an item to the
                    // parent layout (Dear ImGui 1.89+).  Submit a zero-size item at
                    // the restored row end so the popup's content bounds and next
                    // row cursor are both finalized without triggering the layout
                    // assertion.
                    ImGui::SetCursorScreenPos(ImVec2(hitRowStart.x, hitRowStart.y + 66.0f));
                    ImGui::Dummy(ImVec2(0.0f, 0.0f));
                    if (hoverObject) {
                        preview_hover_obj = { dstd.x, dstd.y, dstd.w, dstd.h };
                        preview_hover_obj_valid = true;
                    }
                    if (chooseObject && objectModelIndex >= 0) {
                        wObjectEditor = true;
                        selected_object_group = -1;
                        selected_user_object_group = -1;
                        selected_object_editor = objectModelIndex;
                        SetObjectSelection(std::vector<int>(1, objectModelIndex),
                            objectModelIndex, objectModelIndex, true);
                        preview_object_dragging = false;

                        RefreshPreviewSelectionBounds();
                        ImGui::CloseCurrentPopup();
                    }
                }
            }

            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    if (preview_hover_obj_valid && ((int)GetTimeLapse(1, &g.timer1) % 400 < 250)) {
        float x1 = preview_hover_obj.x;
        float y1 = preview_hover_obj.y;
        float x2 = preview_hover_obj.x + preview_hover_obj.w;
        float y2 = preview_hover_obj.y + preview_hover_obj.h;
        if (x1 > x2) std::swap(x1, x2);
        if (y1 > y2) std::swap(y1, y2);
        const float previewScale = 1.0f / zoom;
        draw_list->AddRect(
            ImVec2(p.x + x1 * previewScale, p.y + y1 * previewScale),
            ImVec2(p.x + x2 * previewScale, p.y + y2 * previewScale),
            IM_COL32(255, 230, 40, 235), 0.0f, ImDrawFlags_Closed, 1.0f);
    }
    

    ImGui::End();

    if (!wPreview) SetWindowVisibleFlag(0);
    return 0;
}

int WORKSPACE::drawTextEdit() {
    char title[260];
    snprintf(title, sizeof(title), "TextEdit##%d", num);

    ImGui::Begin(title, &wTextEdit, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Hide")) {

            ImGui::MenuItem("Blank", NULL, &hideBlank);
            ImGui::MenuItem("Comment", NULL, &hideComment);

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    //vertical scroll & skip hidden //todo only mouse on
    if(ImGui::IsWindowHovered()) {
        ImGuiIO& io = ImGui::GetIO();
        textCursor -= io.MouseWheel;
        if (textCursor < 0) textCursor = 0;
        SKINFILELINEREAD* read1 = &((SKINFILELINEREAD*)skinfileLines.data)[textCursor];
        while ((hideBlank && *read1->line.atPos(0) == '\0') || (hideComment && read1->isComment)) {
            if (io.MouseWheel < 0) textCursor++;
            else textCursor--;

            if (textCursor < 0) {
                textCursor = 0;
                read1 = &((SKINFILELINEREAD*)skinfileLines.data)[textCursor];
                while ((hideBlank && *read1->line.atPos(0) == '\0') || (hideComment && read1->isComment)) {
                    textCursor++;
                    read1 = &((SKINFILELINEREAD*)skinfileLines.data)[textCursor];
                }
            }
            read1 = &((SKINFILELINEREAD*)skinfileLines.data)[textCursor];
        }
        if (textCursor < 0) textCursor = 0;
        if (textCursor >= skinfileLines.count) textCursor = skinfileLines.count - 1;
    }
    
    
    //print every line
    int printed = 0;
    for (int n = textCursor; n < skinfileLines.count && printed < 30; n++) {
        SKINFILELINEREAD& read = ((SKINFILELINEREAD*)skinfileLines.data)[n];

        ImVec4 color;
        color = { 1.0f, 1.0f, 1.0f, 1.0f };

        if ((hideBlank && *read.line.atPos(0) == '\0') || (hideComment && read.isComment)) {
            continue;
        }
        /*char itemname[260];
        snprintf(itemname, sizeof(itemname), "%04d:%04d: %s", read.numTotal, read.num, read.line.outstr());*/

        //ImGui::PushID(n);
        //ImGui::Button(" ");
        ///*if (ImGui::BeginItemTooltip())
        //{
        //    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        //    ImGui::Text("%d", read.ifgroup);
        //    ImGui::EndTooltip();
        //}*/

        //ImGui::PopID();

        //ImGui::SameLine();

        IFUNIT& ifs = ((IFUNIT*)arr_ifunit.data)[read.ifgroup];
        bool head = read.isIfGroupHead || read.isGroupHead;//(read.numTotal == ifs.declare);
        bool isHide = ifs.hide;
        int parIFCursor = ifs.parentID;
        for (int i = 0; i < ifs.depth; i++) {
            isHide |= ((IFUNIT*)arr_ifunit.data)[parIFCursor].hide;
            parIFCursor = ((IFUNIT*)arr_ifunit.data)[parIFCursor].parentID;
        }
        if (isHide && !head) continue;

        ImGui::PushID(n);
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV((head + isHide) / 3.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV((head + isHide) / 3.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV((head + isHide) / 3.0f, 0.8f, 0.8f));
        if (ImGui::Button(" ")) {
            if (head){//read.numTotal == ifs.declare) {
                ifs.hide ^= 1;
            }
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right), 1) {
            if (ImGui::BeginPopupContextItem()) {
                ImGui::Text("selected : %d", n);
                ImGui::MenuItem("move");
                if (ImGui::MenuItem("insert")) {
                    InsertLine(n);
                }
                ImGui::MenuItem("group");
                if (ImGui::MenuItem("delete")) {
                    DeleteLine(n);
                }
                ImGui::EndPopup();
            }
        }

        ImGui::PopStyleColor(3);
        ImGui::PopID();
        ImGui::SameLine();


        ImGui::Text("%d ", read.ifgroup);
        
        ImGui::SameLine();
        
        if (read.isComment) {
            ImGui::TextDisabled("%04d:%04d: ", read.numTotal, read.num);

            //ImGui::SameLine();
            //ImGui::PushID(n);
            //ImGui::Button(" ");
            //
            ///*if (ImGui::BeginItemTooltip())
            //{
            //    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            //    ImGui::Text("%d", read.ifgroup);
            //    ImGui::EndTooltip();
            //}*/

            //ImGui::PopID();

            ImGui::SameLine();
            //ImGui::TextDisabled("%s", read.line.outstr());
            ImGui::PushID(n);
            CstrInputText("", &read.line, ImGuiInputTextFlags_None);

            static CSTR tmp;
            if (ImGui::IsItemActivated()) {
                tmp.assign(read.line.outstr());
            }
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                HISTORY* hs = (HISTORY*)arr_history.Get_new();
                hs->op = overwriteLine;
                hs->target = n;
                CsvToLine(n);
                hs->older.line.assign(tmp);
                hs->newer.line.assign(read.line.outstr());
            }
            ImGui::PopID();
            //ImGui::TextColored(color, "%04d:%04d: %s", read.numTotal, read.num, read.line.outstr());


        }
        else {
            //ImGui::TextColored(color, "%04d:%04d: %s", read.numTotal, read.num, read.line.outstr());

            for (int depth = 0; depth < ((IFUNIT*)(arr_ifunit.data))[read.ifgroup].depth; depth++) {
                ImGui::TextColored(color, "_");
                ImGui::SameLine();
            }
            ImGui::TextColored(color, "%04d:%04d: ", read.numTotal, read.num);

            ImGui::SameLine();

            char tablename[260];
            snprintf(tablename, sizeof(tablename), "##w%d_text", num);
            if (ImGui::BeginTable(tablename, 22, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoPadInnerX , {4000,18},3999))
            {
                ImGui::PushItemWidth(FLT_MAX);
                ImGui::TableNextRow();

                //color
                ImU32 row_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
                if(read.isSRC)
                    row_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.65f)); 
                else if (read.isDST)
                    row_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.65f));
                else if (read.isOther)
                    row_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, row_bg_color);

                for (int column = 0; column < 22; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    if (read.csv.str[column].atPos(0) == nullptr) {
                        ImGui::TextDisabled("%s", Cp932ToUtf8(read.csv.str[column].outstr()).c_str());
                    }
                    else {
                        //ImGui::Text("%s", read.csv.str[column]);
                        char inputname[260];
                        sprintf(inputname, "##%d_%d_cell", read.numTotal, column);
                        ImGui::SetNextItemWidth(-FLT_MIN);

                        
                        CSTR valueHelp = GetCommandHelp(read.csv.str[0].outstr(), column);
                        valueHelp.trimWhiteSpace();
                        int selectedValue = read.csv.val[column];
                        if (DrawCommandValueCombo(inputname, read.csv.str[0].outstr(),
                            valueHelp.body ? valueHelp.outstr() : "", read.csv.val[column], selectedValue)) {
                            if (selectedValue != read.csv.val[column]) EditValue(n, column, selectedValue);
                        } else {
                            static CSTR tmp;
                            CstrInputText(inputname, &read.csv.str[column], ImGuiInputTextFlags_AutoSelectAll);
                            //ImGui::InputText(inputname, read.csv.str[column], 260, ImGuiInputTextFlags_AutoSelectAll);
                            if (ImGui::IsItemActivated()) {
                                tmp.assign(read.line.outstr());
                            }
                            if (ImGui::IsItemDeactivatedAfterEdit()) {
                                HISTORY* hs = (HISTORY*)arr_history.Get_new();
                                hs->op = overwriteLine;
                                hs->target = n;
                                CsvToLine(n);
                                hs->older.line.assign(tmp);
                                hs->newer.line.assign(read.line.outstr());
                            }
                        }
                            
                        
                    }

                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone) && ImGui::BeginTooltip())
                    {
                        ImGui::Text("%d", read.objID);;
                        //ImGui::Text("%d",ifs.grCount);
                        ImGui::Text("%s", Cp932ToUtf8(
                            GetCommandHelp(read.csv.str[0].outstr(), column).outstr()).c_str());
                        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                        const std::string cellValueUtf8 = Cp932ToUtf8(read.csv.str[column].outstr());
                        ImGui::TextUnformatted(cellValueUtf8.c_str());
                        if (read.csv.str[0].isSame("#SRC_IMAGE")) {

                            int handle = read.csv.val[2];
                            EnsureSRCGRTexture(handle);
                            SRCGR& img = ((SRCGR*)arr_SRCGR.data)[handle];

                            int iX = read.csv.val[3];
                            int iY = read.csv.val[4];
                            int iW = read.csv.val[5] == -1 ? img.sizeX - iX : read.csv.val[5];
                            int iH = read.csv.val[6] == -1 ? img.sizeY - iY : read.csv.val[6];

                            if (img.texture != NULL) {
                                ImVec2 display_min = ImVec2(iX / (float)img.sizeX, iY / (float)img.sizeY);
                                ImVec2 display_max = ImVec2((iX + iW) / (float)img.sizeX, (iY + iH) / (float)img.sizeY);
                                ImVec2 display_size = ImVec2(iW, iH);

                                ImGui::Image(img.texture, display_size, display_min, display_max);;
                            }
                            //printSrcImg(((SRC*)arr_SRC.data)[handle]);
                        }
                        ImGui::PopTextWrapPos();
                        ImGui::EndTooltip();
                    }
                }
                ImGui::EndTable();
            }
        }
        
        printed++;
        /*if (ImGui::BeginChild(ImGuiChildFlags_FrameStyle))
        {
            for (int n = 0; n < skinfileLines.count; n++){
                SKINFILELINEREAD& read = ((SKINFILELINEREAD*)skinfileLines.data)[n];

                const bool is_selected = 0;
                char itemname[260];
                snprintf(itemname, sizeof(itemname), "%04d:%04d: %s", read.numTotal, read.num, read.line.outstr());
                ImGui::Selectable(itemname, is_selected);
            }
        }
        ImGui::EndChild();*/


    }

    ImGui::End();
    return 0;
}

bool WORKSPACE::EnsureSRCGRTexture(int iSRCGR) {
    if (iSRCGR < 0 || iSRCGR >= arr_SRCGR.count) return false;
    SRCGR& img = ((SRCGR*)arr_SRCGR.data)[iSRCGR];
    if (img.texture != NULL) return true;
    if (img.loaded || img.path.isSame("CONTINUE")) return false;

    // Mark the attempt so missing optional files are not retried every frame.
    img.loaded = true;
    if (LoadTextureFromFile(img.path.outstr(), &img.texture, &img.sizeX, &img.sizeY))
        return true;

    int dxf = DxLib::FileRead_open(img.path);
    if (dxf < 0) return false;
    const int fileSize = FileRead_size(img.path.outstr());
    void* buffer = fileSize > 0 ? malloc((size_t)fileSize) : NULL;
    bool loaded = false;
    if (buffer && DxLib::FileRead_read(buffer, fileSize, dxf) >= 0)
        loaded = LoadTextureFromMemory(buffer, (size_t)fileSize,
            &img.texture, &img.sizeX, &img.sizeY);
    DxLib::FileRead_close(dxf);
    if (buffer) free(buffer);
    return loaded;
}

//deprecated
int WORKSPACE::loadSRC() {

    for (int n = 0; n < arr_SRCGR.count; n++) {
        CSTR& path = ((SRCGR*)arr_SRCGR.data)[n].path;
        SRCGR& img = ((SRCGR*)arr_SRCGR.data)[n];

        bool isLoaded = EnsureSRCGRTexture(n);
    }
    return 0;
}


int WORKSPACE::printSrcImg(SRC src, bool button) {
    EnsureSRCGRTexture(src.gr);
    SRCGR &img = ((SRCGR*)arr_SRCGR.data)[src.gr];
    if (img.texture != NULL) {
        int sizeX = src.sizeX == -1 ? img.sizeX - src.x : src.sizeX;
        int sizeY = src.sizeY == -1 ? img.sizeY - src.y : src.sizeY;
        ImVec2 display_min = ImVec2(src.x / (float)img.sizeX, src.y / (float)img.sizeY);
        ImVec2 display_max = ImVec2((src.x + sizeX) / (float)img.sizeX, (src.y + sizeY) / (float)img.sizeY);
        ImVec2 display_size = ImVec2(sizeX, sizeY);

        if (src.cycle && (src.div_x >= 1 || src.div_y >= 1)) {
            if (src.div_x == 0) src.div_x = 1;
            if (src.div_y == 0) src.div_y = 1;
            ImVec2 chopsize = { src.sizeX / (float)src.div_x , src.sizeY / (float)src.div_y };

            int units = src.div_x * src.div_y;
            int tick = src.cycle / units;
            int ani = ((int)GetTimeLapse(0, &(g.timer1)) % src.cycle) / tick;

            int ax = ani % src.div_x;
            int ay = ani / src.div_x;

            ImVec2 chopstart = { (src.x + chopsize.x * ax) / (float)img.sizeX ,
                                (src.y + chopsize.y * ay) / (float)img.sizeY };

            ImVec2 chopend = { (src.x - 1 + chopsize.x * (ax + 1)) / (float)img.sizeX ,
                                (src.y - 1 + chopsize.y * (ay + 1)) / (float)img.sizeY };

            if (button) ImGui::ImageButton("aaaa", img.texture, { (float)150,(float)100 }, chopstart, chopend);
            else ImGui::Image(img.texture, chopsize, chopstart, chopend);
            
        }
        else {
            if (button) ImGui::ImageButton("aaaa", img.texture, { (float)150,(float)100 }, display_min, display_max);
            else ImGui::Image(img.texture, display_size, display_min, display_max);;
        }
    }
    return 0;
}

int WORKSPACE::printSrcImgButton(SRC src, int num, int w, int h) {
    EnsureSRCGRTexture(src.gr);
    SRCGR& img = ((SRCGR*)arr_SRCGR.data)[src.gr];
    ImGui::PushID(num);
    if (img.texture != NULL) {
        int sizeX = src.sizeX == -1 ? img.sizeX - src.x : src.sizeX;
        int sizeY = src.sizeY == -1 ? img.sizeY - src.y : src.sizeY;
        ImVec2 display_min = ImVec2(src.x / (float)img.sizeX, src.y / (float)img.sizeY);
        ImVec2 display_max = ImVec2((src.x + sizeX) / (float)img.sizeX, (src.y + sizeY) / (float)img.sizeY);
        ImVec2 display_size = ImVec2(sizeX, sizeY);

        if (src.cycle && (src.div_x >= 1 || src.div_y >= 1)) {
            if (src.div_x == 0) src.div_x = 1;
            if (src.div_y == 0) src.div_y = 1;
            ImVec2 chopsize = { src.sizeX / (float)src.div_x , src.sizeY / (float)src.div_y };

            int units = src.div_x * src.div_y;
            int tick = src.cycle / units;
            int ani = ((int)GetTimeLapse(0, &(g.timer1)) % src.cycle) / tick;

            int ax = ani % src.div_x;
            int ay = ani / src.div_x;

            ImVec2 chopstart = { (src.x + chopsize.x * ax) / (float)img.sizeX ,
                                (src.y + chopsize.y * ay) / (float)img.sizeY };

            ImVec2 chopend = { (src.x - 1 + chopsize.x * (ax + 1)) / (float)img.sizeX ,
                                (src.y - 1 + chopsize.y * (ay + 1)) / (float)img.sizeY };

            ImGui::ImageButton("imgbutton", img.texture, { (float)w,(float)h }, chopstart, chopend);
            //ImGui::Image(img.texture, chopsize, chopstart, chopend);

        }
        else {
            ImGui::ImageButton("imgbutton", img.texture, { (float)w,(float)h }, display_min, display_max);
            //ImGui::Image(img.texture, display_size, display_min, display_max);;
        }
    }
    ImGui::PopID();
    return 0;
}

int WORKSPACE::printSrcImgEx(SRC src, int w, int h, bool ignoreIfGroup) {
    if (src.declare >= 0 && src.declare < skinfileLines.count) {
        SKINFILELINEREAD& sourceLine = ((SKINFILELINEREAD*)skinfileLines.data)[src.declare];
        if (sourceLine.csv.str[0].isSame("#SRC_IMAGE") &&
            (sourceLine.csv.val[2] == 110 || sourceLine.csv.val[2] == 111))
            return -1;
    }
    int imageIndex = -1;
    const int sourceIfgroup = src.declare >= 0 && src.declare < skinfileLines.count
        ? ((SKINFILELINEREAD*)skinfileLines.data)[src.declare].ifgroup : 0;
    std::vector<std::pair<int, int> > imageCandidates;
    for (int candidate = 0; candidate < arr_SRCGR.count; ++candidate) {
        if (((SRCGR*)arr_SRCGR.data)[candidate].grID == src.gr) {
            SRCGR& candidateImage = ((SRCGR*)arr_SRCGR.data)[candidate];
            // Graphic IDs are local to the active IF branch in many LR2
            // skins.  Reusing the first matching grID can therefore crop the
            // right SRC rectangle from a completely different texture sheet.
            int candidateScore = !ignoreIfGroup && candidateImage.isIf == sourceIfgroup ? 100 : 0;
            for (int custom = 0; custom < g.skstruct.customfile_count; ++custom) {
                const char* selected = g.skstruct.customfile[custom].body
                    ? g.skstruct.customfile[custom].outstr() : "";
                if (!*selected || !candidateImage.filename.body) continue;
                const char* selectedName = strrchr(selected, '\\');
                if (!selectedName) selectedName = strrchr(selected, '/');
                selectedName = selectedName ? selectedName + 1 : selected;
                if (_stricmp(candidateImage.filename.outstr(), selectedName) == 0) {
                    candidateScore += 10;
                    break;
                }
            }
            imageCandidates.push_back(std::make_pair(candidateScore, candidate));
        }
    }
    std::stable_sort(imageCandidates.begin(), imageCandidates.end(),
        [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.first > b.first;
        });
    // dstView requests ignoreIfGroup: walk every declaration sharing the
    // logical gr number and use the first texture that can actually load,
    // regardless of whether its #IF branch is currently active.
    for (const std::pair<int, int>& candidate : imageCandidates) {
        EnsureSRCGRTexture(candidate.second);
        if (((SRCGR*)arr_SRCGR.data)[candidate.second].texture) {
            imageIndex = candidate.second;
            break;
        }
    }
    if (imageIndex < 0) return -1;
    SRCGR& img = ((SRCGR*)arr_SRCGR.data)[imageIndex];
    ImGui::PushID(num);
    if (img.texture != NULL) {
        int sizeX = src.sizeX == -1 ? img.sizeX - src.x : src.sizeX;
        int sizeY = src.sizeY == -1 ? img.sizeY - src.y : src.sizeY;
        ImVec2 display_min = ImVec2(src.x / (float)img.sizeX, src.y / (float)img.sizeY);
        ImVec2 display_max = ImVec2((src.x + sizeX) / (float)img.sizeX, (src.y + sizeY) / (float)img.sizeY);
        ImVec2 display_size = ImVec2(sizeX, sizeY);

        if (src.cycle && (src.div_x >= 1 || src.div_y >= 1)) {
            if (src.div_x == 0) src.div_x = 1;
            if (src.div_y == 0) src.div_y = 1;
            ImVec2 chopsize = { src.sizeX / (float)src.div_x , src.sizeY / (float)src.div_y };

            int units = src.div_x * src.div_y;
            int tick = src.cycle / units;
            int ani = ((int)GetTimeLapse(0, &(g.timer1)) % src.cycle) / tick;

            int ax = ani % src.div_x;
            int ay = ani / src.div_x;

            ImVec2 chopstart = { (src.x + chopsize.x * ax) / (float)img.sizeX ,
                                (src.y + chopsize.y * ay) / (float)img.sizeY };

            ImVec2 chopend = { (src.x - 1 + chopsize.x * (ax + 1)) / (float)img.sizeX ,
                                (src.y - 1 + chopsize.y * (ay + 1)) / (float)img.sizeY };

            ImGui::Image(img.texture, { (float)w,(float)h }, chopstart, chopend);

        }
        else {
            ImGui::Image(img.texture, { (float)w,(float)h }, display_min, display_max);;
        }
    }
    ImGui::PopID();
    return 0;
}

int WORKSPACE::ResolveIMGTextureIndex(int imageIndex) {
    if (imageIndex < 0 || imageIndex >= arr_IMG.count) return -1;
    IMG& tag = ((IMG*)arr_IMG.data)[imageIndex];

    // A logical gr number may be declared by several #IMAGE commands. Prefer
    // the declaration in the crop's IF branch, then the active custom-file
    // filename. This is the same ordering used by object thumbnails and keeps
    // an atlas crop from silently resolving to another branch's texture.
    std::vector<std::pair<int, int> > candidates;
    for (int candidate = 0; candidate < arr_SRCGR.count; ++candidate) {
        SRCGR& source = ((SRCGR*)arr_SRCGR.data)[candidate];
        if (source.grID != tag.gr) continue;

        int score = source.isIf == tag.ifGroup ? 100 : 0;
        for (int custom = 0; custom < g.skstruct.customfile_count; ++custom) {
            const char* selected = g.skstruct.customfile[custom].body
                ? g.skstruct.customfile[custom].outstr() : "";
            if (!*selected || !source.filename.body) continue;
            const char* selectedName = strrchr(selected, '\\');
            if (!selectedName) selectedName = strrchr(selected, '/');
            selectedName = selectedName ? selectedName + 1 : selected;
            if (_stricmp(source.filename.outstr(), selectedName) == 0) {
                score += 10;
                break;
            }
        }
        candidates.push_back(std::make_pair(score, candidate));
    }

    std::stable_sort(candidates.begin(), candidates.end(),
        [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
            return a.first > b.first;
        });
    for (const std::pair<int, int>& candidate : candidates) {
        if (EnsureSRCGRTexture(candidate.second)) return candidate.second;
    }
    return candidates.empty() ? -1 : candidates.front().second;
}

int WORKSPACE::ResolveIMGSourceIndex(int imageIndex) const {
    if (imageIndex < 0 || imageIndex >= arr_IMG.count) return -1;
    const IMG& tag = ((const IMG*)arr_IMG.data)[imageIndex];
    if (tag.sourceDeclare == -2) return -1;

    if (tag.sourceDeclare >= 0) {
        for (int sourceIndex = 0; sourceIndex < arr_SRC.count; ++sourceIndex) {
            const SRC& source = ((const SRC*)arr_SRC.data)[sourceIndex];
            if (source.declare == tag.sourceDeclare) return sourceIndex;
        }
    }

    // Compatibility fallback for assets created before sourceDeclare existed.
    // Prefer an animated declaration when identical crop coordinates occur.
    int bestSource = -1;
    int bestScore = -1;
    for (int sourceIndex = 0; sourceIndex < arr_SRC.count; ++sourceIndex) {
        const SRC& source = ((const SRC*)arr_SRC.data)[sourceIndex];
        if (source.gr != tag.gr || source.x != tag.x || source.y != tag.y ||
            source.sizeX != tag.w || source.sizeY != tag.h) continue;
        int sourceIfgroup = 0;
        if (source.declare >= 0 && source.declare < skinfileLines.count)
            sourceIfgroup = ((const SKINFILELINEREAD*)skinfileLines.data)
                [source.declare].ifgroup;
        int score = sourceIfgroup == tag.ifGroup ? 10 : 0;
        if (source.cycle > 0 && (std::max)(1, source.div_x) *
            (std::max)(1, source.div_y) > 1) score += 100;
        if (score > bestScore) {
            bestScore = score;
            bestSource = sourceIndex;
        }
    }
    return bestSource;
}

void WORKSPACE::ResolveIMGDivision(int imageIndex, int& divX, int& divY,
    int& cycle, int& timer) const {
    divX = 1;
    divY = 1;
    cycle = 0;
    timer = 0;
    const int sourceIndex = ResolveIMGSourceIndex(imageIndex);
    if (sourceIndex < 0 || sourceIndex >= arr_SRC.count) return;
    const SRC& source = ((const SRC*)arr_SRC.data)[sourceIndex];
    divX = (std::max)(1, source.div_x);
    divY = (std::max)(1, source.div_y);
    cycle = (std::max)(0, source.cycle);
    timer = source.timer;
}

bool WORKSPACE::SelectIMGAsset(int imageIndex, bool requestImageManagerScroll) {
    if (imageIndex < 0 || imageIndex >= arr_IMG.count) return false;
    IMG& tag = ((IMG*)arr_IMG.data)[imageIndex];
    src_selected = imageIndex;
    grID_selected = tag.gr;
    gr_selected = ResolveIMGTextureIndex(imageIndex);
    if (requestImageManagerScroll) {
        imageManagerFocusRequest = imageIndex;
        wImgManager = true;
    }
    return true;
}

int WORKSPACE::RegisterGeneratedImage(const char* diskPath, int width, int height,
    std::string& errorText) {
    errorText.clear();
    if (!diskPath || !*diskPath || width <= 0 || height <= 0) {
        errorText = "The generated image metadata is invalid.";
        return -1;
    }

    const int newGraphicId = CalculateTrailingGraphicId(skinfileLines);
    if (newGraphicId < 0 || newGraphicId >= 100) {
        errorText = "LR2 supports graphic IDs 0-99; no free trailing slot remains.";
        return -1;
    }
    const std::string storedPath = MakePortableGeneratedImagePath(diskPath,
        mainpath);
    if (storedPath.empty() || storedPath.find(',') != std::string::npos) {
        errorText = "The CSV image path is empty or contains a comma.";
        return -1;
    }

    CSTR owner(mainpath);
    AssignRootFileOwner(skinfileLines, mainpath, owner);
    int insertAt = FindOwnerFileEndRow(skinfileLines,
        owner.body ? owner.outstr() : mainpath);

    const std::string imageLine = std::string("#IMAGE,") + storedPath;
    if (InsertLine(insertAt) != 0) {
        errorText = "Could not insert the #IMAGE declaration.";
        return -1;
    }
    SKINFILELINEREAD& imageDeclaration =
        ((SKINFILELINEREAD*)skinfileLines.data)[insertAt];
    imageDeclaration.filename.assign(owner);
    imageDeclaration.ifgroup = 0;
    CSTR imagePlaceholder(imageDeclaration.line);
    if (EditLine(insertAt, imagePlaceholder, CSTR(imageLine.c_str())) != 0) {
        errorText = "Could not write the #IMAGE declaration.";
        return -1;
    }

    char assetLine[256] = {};
    snprintf(assetLine, sizeof(assetLine),
        "$SRC_IMAGE,0,%d,0,0,%d,%d,1,1,0,0,0,0,0",
        newGraphicId, width, height);
    ++insertAt;
    if (InsertLine(insertAt) != 0) {
        errorText = "The #IMAGE was added, but its Asset metadata could not be inserted.";
        return -1;
    }
    SKINFILELINEREAD& assetDeclaration =
        ((SKINFILELINEREAD*)skinfileLines.data)[insertAt];
    assetDeclaration.filename.assign(owner);
    assetDeclaration.ifgroup = 0;
    CSTR assetPlaceholder(assetDeclaration.line);
    if (EditLine(insertAt, assetPlaceholder, CSTR(assetLine)) != 0) {
        errorText = "The #IMAGE was added, but its Asset metadata could not be written.";
        return -1;
    }

    imageManagerGeneratedGrFocusRequest = newGraphicId;
    assetSearch[0] = '\0';
    wImgManager = true;
    wAssetBrowser = true;
    return newGraphicId;
}

bool WORKSPACE::OpenNewObjectFromAsset(int imageIndex, int dropX, int dropY) {
    if (imageIndex < 0 || imageIndex >= arr_IMG.count || arr_CommandHelp.count <= 0)
        return false;

    CSVbuf* commands = (CSVbuf*)arr_CommandHelp.data;
    int imageCommand = -1;
    for (int commandIndex = 0; commandIndex < arr_CommandHelp.count; ++commandIndex) {
        if (commands[commandIndex].str[0].body &&
            commands[commandIndex].str[0].isSame("#SRC_IMAGE")) {
            imageCommand = commandIndex;
            break;
        }
    }
    if (imageCommand < 0) return false;

    IMG& asset = ((IMG*)arr_IMG.data)[imageIndex];
    selected_command = imageCommand;
    newCommandIncludeAll = false;
    newObjectAssetIndex = imageIndex;
    newObjectDropX = dropX;
    newObjectDropY = dropY;
    newObjectDropW = asset.w;
    newObjectDropH = asset.h;
    const int textureIndex = ResolveIMGTextureIndex(imageIndex);
    if (textureIndex >= 0 && textureIndex < arr_SRCGR.count) {
        SRCGR& source = ((SRCGR*)arr_SRCGR.data)[textureIndex];
        if (newObjectDropW == -1) newObjectDropW = source.sizeX - asset.x;
        if (newObjectDropH == -1) newObjectDropH = source.sizeY - asset.y;
    }
    int assetDivX = 1;
    int assetDivY = 1;
    int assetCycle = 0;
    int assetTimer = 0;
    ResolveIMGDivision(imageIndex, assetDivX, assetDivY,
        assetCycle, assetTimer);
    newObjectDropW = (std::max)(1, newObjectDropW / assetDivX);
    newObjectDropH = (std::max)(1, newObjectDropH / assetDivY);

    // The selected Object is the user's current branch and owner-file context.
    // With no selection, create an unconditional Object inside the root file.
    AssignRootFileOwner(skinfileLines, mainpath, newObjectOwner);
    newObjectInsertPosition = FindOwnerFileEndRow(skinfileLines,
        newObjectOwner.body ? newObjectOwner.outstr() : mainpath);
    newObjectIfgroup = 0;
    int contextModel = preview_selected_object_model_index;
    const std::vector<SEObjectInstance>& objects = objectEditorModel.Objects();
    if ((contextModel < 0 || contextModel >= (int)objects.size()) &&
        preview_selected_object_model_indices.size() == 1)
        contextModel = preview_selected_object_model_indices.front();
    if (contextModel >= 0 && contextModel < (int)objects.size()) {
        const SEObjectInstance& contextObject = objects[contextModel];
        newObjectIfgroup = contextObject.ifgroup;
        if (!contextObject.rows.empty()) {
            newObjectInsertPosition = contextObject.rows.back() + 1;
            const int ownerRow = contextObject.rows.front();
            if (ownerRow >= 0 && ownerRow < skinfileLines.count) {
                SKINFILELINEREAD& source =
                    ((SKINFILELINEREAD*)skinfileLines.data)[ownerRow];
                if (source.filename.body && *source.filename.outstr())
                    newObjectOwner.assign(source.filename.outstr());
            }
        }
    }

    SplitCSV("", &nCsv, ",");
    nCsv.str[0].assign("#SRC_IMAGE");
    AssignCommandField(nCsv, "#SRC_IMAGE", "(NULL)", 0);
    AssignCommandField(nCsv, "#SRC_IMAGE", "gr", asset.gr);
    AssignCommandField(nCsv, "#SRC_IMAGE", "x", asset.x);
    AssignCommandField(nCsv, "#SRC_IMAGE", "y", asset.y);
    AssignCommandField(nCsv, "#SRC_IMAGE", "w", asset.w);
    AssignCommandField(nCsv, "#SRC_IMAGE", "h", asset.h);
    AssignCommandField(nCsv, "#SRC_IMAGE", "div_x", assetDivX);
    AssignCommandField(nCsv, "#SRC_IMAGE", "div_y", assetDivY);
    AssignCommandField(nCsv, "#SRC_IMAGE", "cycle", assetCycle);
    AssignCommandField(nCsv, "#SRC_IMAGE", "timer", assetTimer);
    AssignCommandField(nCsv, "#SRC_IMAGE", "op1", 0);
    AssignCommandField(nCsv, "#SRC_IMAGE", "op2", 0);
    AssignCommandField(nCsv, "#SRC_IMAGE", "op3", 0);
    newObjectCsvInitialized = true;
    newObjectInitializedCommand = imageCommand;

    const char* assetName = asset.name.body ? asset.name.outstr() : "";
    if (*assetName && _strnicmp(assetName, "#SRC", 4) != 0 &&
        _stricmp(assetName, "noname") != 0) {
        newObjectName.assign(assetName);
    } else {
        char generatedName[64];
        snprintf(generatedName, sizeof(generatedName), "Asset %03d", imageIndex);
        newObjectName.assign(generatedName);
    }

    newObjectFocusRequest = true;
    wNewObject = true;
    return true;
}

int WORKSPACE::drawAssetBrowser() {
    char title[64];
    snprintf(title, sizeof(title), "Asset Browser##%d", num);
    if (!ImGui::Begin(title, &wAssetBrowser)) {
        ImGui::End();
        return 0;
    }

    if (arr_IMG.count <= 0) {
        SEUI::EmptyState("No image crops",
            "Image crops parsed from #SRC commands will appear here as reusable assets.");
        ImGui::End();
        return 0;
    }

    ImGui::SetNextItemWidth((std::min)(260.0f,
        (std::max)(120.0f, ImGui::GetContentRegionAvail().x * 0.42f)));
    ImGui::InputTextWithHint("##AssetSearch", "Search asset, gr or command...",
        assetSearch, sizeof(assetSearch));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(150.0f);
    ImGui::SliderFloat("Thumbnail", &assetThumbnailSize, 48.0f, 192.0f, "%.0f px");
    assetThumbnailSize = (std::max)(48.0f, (std::min)(192.0f, assetThumbnailSize));
    ImGui::SameLine();
    ImGui::Checkbox("Animate SRC", &assetAnimateSrc);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip("Preview div_x/div_y frames using the SRC cycle value.");

    auto lowercase = [](std::string value) {
        std::transform(value.begin(), value.end(), value.begin(),
            [](unsigned char c) { return (char)std::tolower(c); });
        return value;
    };
    const std::string query = lowercase(assetSearch);
    std::vector<int> filteredAssets;
    filteredAssets.reserve(arr_IMG.count);
    for (int imageIndex = 0; imageIndex < arr_IMG.count; ++imageIndex) {
        IMG& tag = ((IMG*)arr_IMG.data)[imageIndex];
        if (query.empty()) {
            filteredAssets.push_back(imageIndex);
            continue;
        }
        const std::string tagName = Cp932ToUtf8(tag.name.body ? tag.name.outstr() : "");
        char metadata[256];
        snprintf(metadata, sizeof(metadata), "%03d asset %d gr %d g%02d if %d %s",
            imageIndex, imageIndex, tag.gr, tag.gr, tag.ifGroup, tagName.c_str());
        if (query.empty() || lowercase(metadata).find(query) != std::string::npos)
            filteredAssets.push_back(imageIndex);
    }

    ImGui::SameLine();
    ImGui::TextDisabled("%d / %d", (int)filteredAssets.size(), arr_IMG.count);
    ImGui::Separator();

    if (filteredAssets.empty()) {
        SEUI::EmptyState("No matching assets", "Clear the search text to show every crop.");
        ImGui::End();
        return 0;
    }

    if (ImGui::BeginChild("##AssetGrid", ImVec2(0.0f, 0.0f),
        ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar)) {
        const ImGuiStyle& style = ImGui::GetStyle();
        const float padding = 7.0f;
        const float spacing = (std::max)(6.0f, style.ItemSpacing.x);
        const float cardWidth = assetThumbnailSize + padding * 2.0f;
        const float cardHeight = assetThumbnailSize + padding * 2.0f +
            ImGui::GetTextLineHeight() * 2.0f + 5.0f;
        const float itemStepX = cardWidth + spacing;
        const float itemStepY = cardHeight + spacing;
        const float availableWidth = ImGui::GetContentRegionAvail().x;
        const int columnCount = (std::max)(1,
            (int)((availableWidth + spacing) / itemStepX));
        const int rowCount = ((int)filteredAssets.size() + columnCount - 1) / columnCount;
        if (assetBrowserFocusRequest >= 0) {
            const std::vector<int>::const_iterator requested = std::find(
                filteredAssets.begin(), filteredAssets.end(), assetBrowserFocusRequest);
            if (requested != filteredAssets.end()) {
                const int requestedCard = (int)(requested - filteredAssets.begin());
                const int requestedRow = requestedCard / columnCount;
                ImGui::SetScrollY((std::max)(0.0f,
                    requestedRow * itemStepY - itemStepY));
            }
            assetBrowserFocusRequest = -1;
        }
        const ImVec2 gridStart = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        std::map<std::pair<int, int>, int> visibleTextureCache;

        // Follow imgui_demo's asset-browser pattern: clip whole rows, position
        // a real Selectable for every visible card, then paint the thumbnail
        // into that item's rectangle. The Selectable is important; extending
        // the parent only with SetCursorPos triggers an ImGui assertion and
        // also breaks hit testing.
        ImGuiListClipper clipper;
        clipper.Begin(rowCount, itemStepY);
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
                const int rowStart = row * columnCount;
                const int rowEnd = (std::min)(rowStart + columnCount,
                    (int)filteredAssets.size());
                for (int filteredIndex = rowStart; filteredIndex < rowEnd; ++filteredIndex) {
                    const int column = filteredIndex - rowStart;
                    const int imageIndex = filteredAssets[filteredIndex];
                    IMG& tag = ((IMG*)arr_IMG.data)[imageIndex];
                    const ImVec2 cardPos(gridStart.x + column * itemStepX,
                        gridStart.y + row * itemStepY);

                    ImGui::SetCursorScreenPos(cardPos);
                    ImGui::PushID(imageIndex);
                    const bool selected = imageIndex == src_selected;
                    const bool activated = ImGui::Selectable("##AssetCard", selected,
                        ImGuiSelectableFlags_None, ImVec2(cardWidth, cardHeight));
                    const bool hovered = ImGui::IsItemHovered();
                    if (activated) SelectIMGAsset(imageIndex, true);

                    char managerTitle[64];
                    snprintf(managerTitle, sizeof(managerTitle), "ImageManager##%d", num);
                    if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        SelectIMGAsset(imageIndex, true);
                        ImGui::SetWindowFocus(managerTitle);
                    }

                    const ImVec2 cardMin = ImGui::GetItemRectMin();
                    const ImVec2 cardMax = ImGui::GetItemRectMax();
                    const ImVec2 thumbMin(cardMin.x + padding, cardMin.y + padding);
                    const ImVec2 thumbMax(thumbMin.x + assetThumbnailSize,
                        thumbMin.y + assetThumbnailSize);
                    drawList->AddRectFilled(thumbMin, thumbMax,
                        ImGui::GetColorU32(ImGuiCol_FrameBg));
                    if (transBackground) {
                        drawList->AddImage(transBackground, thumbMin, thumbMax, ImVec2(0, 0),
                            ImVec2(assetThumbnailSize / 32.0f, assetThumbnailSize / 32.0f));
                    }

                    const std::pair<int, int> textureKey(tag.gr, tag.ifGroup);
                    std::map<std::pair<int, int>, int>::iterator cachedTexture =
                        visibleTextureCache.find(textureKey);
                    int textureIndex = -1;
                    if (cachedTexture != visibleTextureCache.end()) {
                        textureIndex = cachedTexture->second;
                    } else {
                        textureIndex = ResolveIMGTextureIndex(imageIndex);
                        visibleTextureCache[textureKey] = textureIndex;
                    }
                    SRCGR* source = textureIndex >= 0 && textureIndex < arr_SRCGR.count
                        ? &((SRCGR*)arr_SRCGR.data)[textureIndex] : NULL;
                    const int sourceIndex = ResolveIMGSourceIndex(imageIndex);
                    const SRC* animationSource = sourceIndex >= 0 && sourceIndex < arr_SRC.count
                        ? &((SRC*)arr_SRC.data)[sourceIndex] : NULL;
                    int animationFrame = 0;
                    int animationFrames = 1;
                    bool validCrop = false;
                    if (source && source->texture && source->sizeX > 0 && source->sizeY > 0) {
                        const int requestedW = tag.w == -1 ? source->sizeX - tag.x : tag.w;
                        const int requestedH = tag.h == -1 ? source->sizeY - tag.y : tag.h;
                        int requestedX0 = tag.x;
                        int requestedY0 = tag.y;
                        int requestedX1 = tag.x + requestedW;
                        int requestedY1 = tag.y + requestedH;
                        if (assetAnimateSrc && animationSource &&
                            animationSource->cycle > 0) {
                            const int divX = (std::max)(1,
                                (std::min)(4096, animationSource->div_x));
                            const int divY = (std::max)(1,
                                (std::min)(4096, animationSource->div_y));
                            const long long frameCount64 =
                                (long long)divX * (long long)divY;
                            if (frameCount64 > 1 && frameCount64 <= INT_MAX) {
                                animationFrames = (int)frameCount64;
                                const long long elapsedMs =
                                    (long long)(ImGui::GetTime() * 1000.0);
                                const long long cyclePosition = elapsedMs %
                                    (long long)animationSource->cycle;
                                animationFrame = (int)((cyclePosition * animationFrames) /
                                    animationSource->cycle);
                                if (animationFrame >= animationFrames)
                                    animationFrame = animationFrames - 1;
                                const int frameX = animationFrame % divX;
                                const int frameY = animationFrame / divX;
                                requestedX0 = tag.x + (requestedW * frameX) / divX;
                                requestedX1 = tag.x + (requestedW * (frameX + 1)) / divX;
                                requestedY0 = tag.y + (requestedH * frameY) / divY;
                                requestedY1 = tag.y + (requestedH * (frameY + 1)) / divY;
                            }
                        }
                        const int cropX0 = (std::max)(0,
                            (std::min)(source->sizeX, requestedX0));
                        const int cropY0 = (std::max)(0,
                            (std::min)(source->sizeY, requestedY0));
                        const int cropX1 = (std::max)(0,
                            (std::min)(source->sizeX, requestedX1));
                        const int cropY1 = (std::max)(0,
                            (std::min)(source->sizeY, requestedY1));
                        const int cropW = cropX1 - cropX0;
                        const int cropH = cropY1 - cropY0;
                        if (cropW > 0 && cropH > 0) {
                            const float imageScale = (std::min)(assetThumbnailSize / cropW,
                                assetThumbnailSize / cropH);
                            const ImVec2 displaySize(cropW * imageScale, cropH * imageScale);
                            const ImVec2 imageMin(thumbMin.x +
                                (assetThumbnailSize - displaySize.x) * 0.5f,
                                thumbMin.y + (assetThumbnailSize - displaySize.y) * 0.5f);
                            const ImVec2 imageMax(imageMin.x + displaySize.x,
                                imageMin.y + displaySize.y);
                            const ImVec2 uv0(cropX0 / (float)source->sizeX,
                                cropY0 / (float)source->sizeY);
                            const ImVec2 uv1(cropX1 / (float)source->sizeX,
                                cropY1 / (float)source->sizeY);
                            const bool sharp = BeginSharpMagnifiedCanvas(imageScale);
                            drawList->AddImage(source->texture, imageMin, imageMax, uv0, uv1);
                            EndSharpMagnifiedCanvas(sharp);
                            validCrop = true;
                        }
                    }
                    if (!validCrop) {
                        const char* missing = "Missing crop";
                        const ImVec2 textSize = ImGui::CalcTextSize(missing);
                        drawList->AddText(ImVec2(
                            thumbMin.x + (assetThumbnailSize - textSize.x) * 0.5f,
                            thumbMin.y + (assetThumbnailSize - textSize.y) * 0.5f),
                            ImGui::GetColorU32(ImGuiCol_TextDisabled), missing);
                    }
                    drawList->AddRect(thumbMin, thumbMax,
                        ImGui::GetColorU32(ImGuiCol_Border));

                    char primaryLabel[64];
                    snprintf(primaryLabel, sizeof(primaryLabel), "%03d   gr %d",
                        imageIndex, tag.gr);
                    const std::string tagName = Cp932ToUtf8(
                        tag.name.body ? tag.name.outstr() : "(unnamed)");
                    drawList->PushClipRect(cardMin, cardMax, true);
                    drawList->AddText(ImVec2(cardMin.x + padding, thumbMax.y + 4.0f),
                        ImGui::GetColorU32(ImGuiCol_Text), primaryLabel);
                    drawList->AddText(ImVec2(cardMin.x + padding,
                        thumbMax.y + 4.0f + ImGui::GetTextLineHeight()),
                        ImGui::GetColorU32(ImGuiCol_TextDisabled), tagName.c_str());
                    drawList->PopClipRect();
                    if (selected)
                        drawList->AddRect(cardMin, cardMax,
                            ImGui::GetColorU32(ImGuiCol_NavHighlight), 0.0f, 0, 2.0f);

                    if (ImGui::BeginPopupContextItem("##AssetContext")) {
                        if (ImGui::MenuItem("Open in Image Manager")) {
                            SelectIMGAsset(imageIndex, true);
                            ImGui::SetWindowFocus(managerTitle);
                        }
                        if (ImGui::MenuItem("Pixel paint in Image Manager")) {
                            SelectIMGAsset(imageIndex, true);
                            imagePixelPaintMode = true;
                            imagePixelPaintLastX = -1;
                            imagePixelPaintLastY = -1;
                            imagePixelPaintLastButton = -1;
                            ImGui::SetWindowFocus(managerTitle);
                        }
                        ImGui::EndPopup();
                    }
                    if (ImGui::BeginDragDropSource()) {
                        ImGui::SetDragDropPayload("SKINEDITOR_IMG_ASSET", &imageIndex,
                            sizeof(imageIndex), ImGuiCond_Once);
                        ImGui::Text("Asset %03d  gr %d", imageIndex, tag.gr);
                        ImGui::TextDisabled("%s", tagName.c_str());
                        ImGui::Separator();
                        ImGui::TextDisabled("Drop on Preview to configure a new Object.");
                        ImGui::EndDragDropSource();
                    }
                    if (hovered && !ImGui::IsMouseDragging(ImGuiMouseButton_Left) &&
                        ImGui::BeginTooltip()) {
                        ImGui::Text("Asset %03d", imageIndex);
                        ImGui::Text("%s", tagName.c_str());
                        ImGui::Text("gr %d  x %d  y %d  w %d  h %d",
                            tag.gr, tag.x, tag.y, tag.w, tag.h);
                        ImGui::Text("IF branch %d", tag.ifGroup);
                        if (animationSource) {
                            ImGui::Text("SRC div %d x %d  cycle %d ms",
                                (std::max)(1, animationSource->div_x),
                                (std::max)(1, animationSource->div_y),
                                animationSource->cycle);
                            if (animationFrames > 1)
                                ImGui::Text("Animation frame %d / %d",
                                    animationFrame + 1, animationFrames);
                        }
                        if (source && source->filename.body)
                            ImGui::Text("Texture: %s",
                                Cp932ToUtf8(source->filename.outstr()).c_str());
                        ImGui::Separator();
                        ImGui::TextDisabled("Click to select; double-click to open in Image Manager.");
                        ImGui::EndTooltip();
                    }
                    ImGui::PopID();
                }
            }
        }
        clipper.End();
    }
    ImGui::EndChild();
    ImGui::End();
    return 0;
}

int WORKSPACE::drawImgManager() {
    char title[260];
    snprintf(title, sizeof(title), "ImageManager##%d", num);
    if (!ImGui::Begin(title, &wImgManager, ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::End();
        return 0;
    }

    if (arr_IMG.count <= 0 || arr_SRCGR.count <= 0) {
        SEUI::EmptyState("No image data",
            "Load a skin containing #IMAGE and #SRC declarations.");
        ImGui::End();
        return 0;
    }
    if (src_selected < 0 || src_selected >= arr_IMG.count)
        SelectIMGAsset(0, false);

    static bool newSquare = 0;
    bool clicked = 0;
    int deleteImageRequest = -1;
    const float imageEditPanelHeight = 150.0f;
    if (imageManagerGeneratedGrFocusRequest >= 0) {
        for (int imageIndex = 0; imageIndex < arr_IMG.count; ++imageIndex) {
            IMG& generated = ((IMG*)arr_IMG.data)[imageIndex];
            if (generated.gr != imageManagerGeneratedGrFocusRequest) continue;
            clicked = SelectIMGAsset(imageIndex, false);
            assetBrowserFocusRequest = imageIndex;
            imageManagerFocusRequest = imageIndex;
            imageManagerGeneratedGrFocusRequest = -1;
            break;
        }
    }
    if (imageManagerFocusRequest >= 0) {
        clicked = SelectIMGAsset(imageManagerFocusRequest, false);
        imageManagerFocusRequest = -1;
    }

    //testing new img list
    ImGui::BeginGroup();
    ImGui::SameLine();
    snprintf(title, sizeof(title), "ImgList##%d", num);
    if (ImGui::BeginChild(title, { 250,-imageEditPanelHeight },
        ImGuiChildFlags_ResizeX | ImGuiChildFlags_FrameStyle)) {

        for (int i = 0; i < arr_IMG.count; i++) {
            IMG& img = ((IMG*)arr_IMG.data)[i];
            char buf[260];
            sprintf(buf, "%03d %02d:%s", i, img.gr, img.name.outstr());
            ImGui::PushID(i);
            if (ImGui::Selectable(buf, i == src_selected)) {
                SelectIMGAsset(i, false);
                clicked = true;
            }
            
            ImGui::PopID();
        }
        IMG& img = ((IMG*)arr_IMG.data)[src_selected];
        if (ImGui::BeginPopupContextWindow()) {
            ImGui::Text("selected : %03d", src_selected);
            
            ImGui::Separator();
            if (ImGui::MenuItem("Delete")) {
                deleteImageRequest = src_selected;
            }
            ImGui::Separator();
            if(ImGui::MenuItem("New")) {
                //mosue drag
                newSquare = 1;
            }
            ImGui::EndPopup();
        }
        
    }
    ImGui::EndChild();
    if (deleteImageRequest >= 0) {
        DeleteIMG(deleteImageRequest);
        if (arr_IMG.count <= 0) {
            ImGui::EndGroup();
            ImGui::End();
            return 0;
        }
        src_selected = (std::max)(0,
            (std::min)(src_selected, arr_IMG.count - 1));
        SelectIMGAsset(src_selected, false);
        clicked = true;
    }
    //left bottom
    ImGui::Separator();
    snprintf(title, sizeof(title), "ImgEdit##%d", num);
    if (ImGui::BeginChild(title, { 250,imageEditPanelHeight },
        ImGuiChildFlags_ResizeX | ImGuiChildFlags_FrameStyle)) {
        IMG& img = ((IMG*)arr_IMG.data)[src_selected];
        
        char buf[64];
        strncpy(buf, img.name.outstr(), sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        ImGui::InputText("##name", buf, sizeof(buf));
        img.name.assign(buf);

        int editedGr = img.gr;
        int editedPosition[2] = { img.x, img.y };
        int editedSize[2] = { img.w, img.h };
        bool cropChanged = ImGui::InputInt("gr", &editedGr);
        cropChanged |= ImGui::InputInt2("x / y", editedPosition);
        cropChanged |= ImGui::InputInt2("w / h", editedSize);

        if (cropChanged && ModifyIMG(src_selected, editedGr,
            editedPosition[0], editedPosition[1], editedSize[0], editedSize[1]) == 0) {
            grID_selected = ((IMG*)arr_IMG.data)[src_selected].gr;
            gr_selected = ResolveIMGTextureIndex(src_selected);
        }
    }
    ImGui::EndChild();
    ImGui::EndGroup();
    // test new image view
    ImGui::SameLine(0, 0);

    ImGui::BeginGroup();

    static int gr_type;
    auto formatGraphicLabel = [](int index, SRCGR& graphic) {
        char prefix[64];
        snprintf(prefix, sizeof(prefix), "%03d %02d [IF %03d] : ",
            index, graphic.grID, graphic.isIf);
        const std::string filenameUtf8 = Cp932ToUtf8(
            graphic.filename.body ? graphic.filename.outstr() : "");
        return std::string(prefix) + filenameUtf8;
    };
    std::string graphicPreview = "No texture";
    IMG& selectedImageTag = ((IMG*)arr_IMG.data)[src_selected];
    if (gr_selected < 0 || gr_selected >= arr_SRCGR.count ||
        ((SRCGR*)arr_SRCGR.data)[gr_selected].grID != grID_selected ||
        ((SRCGR*)arr_SRCGR.data)[gr_selected].isIf != selectedImageTag.ifGroup) {
        for (int i = 0; i < arr_SRCGR.count; i++) {
            SRCGR& gr = ((SRCGR*)arr_SRCGR.data)[i];
            if (gr.grID == grID_selected && gr.isIf == selectedImageTag.ifGroup) {
                gr_selected = i;
                break;
            }
        }
        if (gr_selected < 0 || gr_selected >= arr_SRCGR.count ||
            ((SRCGR*)arr_SRCGR.data)[gr_selected].grID != grID_selected) {
            for (int i = 0; i < arr_SRCGR.count; ++i) {
                if (((SRCGR*)arr_SRCGR.data)[i].grID == grID_selected) {
                    gr_selected = i;
                    break;
                }
            }
        }
    }
    if (gr_selected >= 0 && gr_selected < arr_SRCGR.count) {
        SRCGR& selectedGraphic = ((SRCGR*)arr_SRCGR.data)[gr_selected];
        graphicPreview = formatGraphicLabel(gr_selected, selectedGraphic);
    }

    if (ImGui::BeginCombo("##grSelect", graphicPreview.c_str())) {
        for (int i = 0; i < arr_SRCGR.count; i++) {
            SRCGR& gr = ((SRCGR*)arr_SRCGR.data)[i];
            // Show every texture declaration sharing this logical gr number.
            // The current SRC branch only controls the automatic default.
            if (gr.grID != grID_selected) continue;
            const std::string graphicLabel = formatGraphicLabel(i, gr);
            ImGui::PushID(i);
            if (ImGui::Selectable(graphicLabel.c_str(), i == gr_selected)) {
                gr_selected = i;
            }

            ImGui::PopID();
        }
        ImGui::EndCombo();
    }

    if (gr_selected < 0 || gr_selected >= arr_SRCGR.count) {
        SEUI::EmptyState("Texture declaration not found",
            "This crop references a gr number that has no matching #IMAGE declaration.");
        ImGui::EndGroup();
        ImGui::End();
        return 0;
    }

    EnsureSRCGRTexture(gr_selected);
    SRCGR& img = ((SRCGR*)arr_SRCGR.data)[gr_selected];
    ImGui::Text("%03d_%d ", gr_selected, img.grID);

    ImGui::SameLine(0, 0);
    const std::string imagePathUtf8 = Cp932ToUtf8(
        img.path.body ? img.path.outstr() : "");
    ImGui::Text("%s %d %d", imagePathUtf8.c_str(), img.sizeX, img.sizeY);
    ImGui::SameLine(0, 0);
    snprintf(title, sizeof(title), "grReload##%d", num);
    ImGui::Button(title);
    ImGui::SameLine(0, 0);
    ImGui::ColorEdit4("MyColor##3", (float*)&bgColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None);

    auto initializeImageToolPath = [&](const char* stem) {
        const std::string outputPath = MakeUniqueGeneratedImagePath(
            img.path.body ? img.path.outstr() : NULL, mainpath, stem);
        const std::string outputUtf8 = Cp932ToUtf8(outputPath.c_str());
        strncpy_s(imageToolOutputPathUtf8, sizeof(imageToolOutputPathUtf8),
            outputUtf8.c_str(), _TRUNCATE);
        imageToolStatus.clear();
        imageToolRegisterInCsv = true;
    };

    if (ImGui::Button("New image##imageToolNew")) {
        imageNewWidth = img.sizeX > 0 ? img.sizeX : (std::max)(1, skinSizeX);
        imageNewHeight = img.sizeY > 0 ? img.sizeY : (std::max)(1, skinSizeY);
        imageNewColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        initializeImageToolPath("new_image");
        imageNewDialogRequested = true;
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!img.texture || arr_IMG.count <= 0);
    if (ImGui::Button("Merge image##imageToolMerge")) {
        imageMergeAssetIndex = (std::max)(0,
            (std::min)(src_selected, arr_IMG.count - 1));
        initializeImageToolPath("merged_image");
        imageMergeDialogRequested = true;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::TextDisabled("New files are appended as the final gr slot.");

    char newImagePopup[96] = {};
    char mergeImagePopup[96] = {};
    snprintf(newImagePopup, sizeof(newImagePopup), "New image##newImage%d", num);
    snprintf(mergeImagePopup, sizeof(mergeImagePopup),
        "Merge image##mergeImage%d", num);
    if (imageNewDialogRequested) {
        ImGui::OpenPopup(newImagePopup);
        imageNewDialogRequested = false;
    }
    if (imageMergeDialogRequested) {
        ImGui::OpenPopup(mergeImagePopup);
        imageMergeDialogRequested = false;
    }

    ImGui::SetNextWindowSize(ImVec2(560.0f, 0.0f), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal(newImagePopup, NULL,
        ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Create a blank image file");
        ImGui::TextDisabled("PNG is recommended. Existing files are never overwritten.");
        ImGui::Separator();
        ImGui::SetNextItemWidth(420.0f);
        ImGui::InputText("Output file", imageToolOutputPathUtf8,
            sizeof(imageToolOutputPathUtf8));
        int dimensions[2] = { imageNewWidth, imageNewHeight };
        ImGui::SetNextItemWidth(220.0f);
        if (ImGui::InputInt2("Width / height", dimensions)) {
            imageNewWidth = dimensions[0];
            imageNewHeight = dimensions[1];
        }
        ImGui::SetNextItemWidth(220.0f);
        ImGui::ColorEdit4("Fill color", (float*)&imageNewColor,
            ImGuiColorEditFlags_AlphaPreviewHalf);
        ImGui::Checkbox("Register in this skin CSV", &imageToolRegisterInCsv);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Adds #IMAGE plus a full-size editor Asset without renumbering existing gr IDs.");
        if (!imageToolStatus.empty())
            ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.42f, 1.0f), "%s",
                imageToolStatus.c_str());
        ImGui::Separator();
        if (ImGui::Button("Create", ImVec2(100.0f, 0.0f))) {
            const std::string enteredCp932 = Utf8ToCp932(
                imageToolOutputPathUtf8);
            const std::string diskPath = ResolveGeneratedImageDiskPath(
                enteredCp932.c_str(), mainpath);
            imageToolStatus.clear();
            if (diskPath.empty()) {
                imageToolStatus = "The output path is invalid.";
            } else if (imageToolRegisterInCsv &&
                (CalculateTrailingGraphicId(skinfileLines) >= 100 ||
                    MakePortableGeneratedImagePath(diskPath.c_str(), mainpath)
                        .find(',') != std::string::npos)) {
                imageToolStatus = "No free gr slot remains, or the CSV path contains a comma.";
            } else {
                const int red = (std::max)(0, (std::min)(255,
                    (int)(imageNewColor.x * 255.0f + 0.5f)));
                const int green = (std::max)(0, (std::min)(255,
                    (int)(imageNewColor.y * 255.0f + 0.5f)));
                const int blue = (std::max)(0, (std::min)(255,
                    (int)(imageNewColor.z * 255.0f + 0.5f)));
                const int alpha = (std::max)(0, (std::min)(255,
                    (int)(imageNewColor.w * 255.0f + 0.5f)));
                char createError[256] = {};
                if (!CreateSolidImageFileAtomic(diskPath.c_str(), imageNewWidth,
                    imageNewHeight, D3DCOLOR_ARGB(alpha, red, green, blue),
                    createError, sizeof(createError))) {
                    imageToolStatus = createError;
                } else {
                    int registeredGr = -1;
                    if (imageToolRegisterInCsv)
                        registeredGr = RegisterGeneratedImage(diskPath.c_str(),
                            imageNewWidth, imageNewHeight, imageToolStatus);
                    if (!imageToolRegisterInCsv || registeredGr >= 0) {
                        imageToolStatus = imageToolRegisterInCsv
                            ? "Created and registered as gr " +
                                std::to_string(registeredGr) + "."
                            : "Created image file.";
                        ImGui::CloseCurrentPopup();
                    } else {
                        imageToolStatus = "The image file was created, but CSV registration failed: " +
                            imageToolStatus;
                    }
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100.0f, 0.0f))) {
            imageToolStatus.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::SetNextWindowSize(ImVec2(600.0f, 0.0f), ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal(mergeImagePopup, NULL,
        ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Base image: gr %d  %d x %d", img.grID, img.sizeX, img.sizeY);
        ImGui::TextDisabled("The Asset is placed in the first transparent space; the canvas expands automatically if needed.");
        ImGui::Separator();
        imageMergeAssetIndex = (std::max)(0,
            (std::min)(imageMergeAssetIndex, arr_IMG.count - 1));
        IMG& overlayTag = ((IMG*)arr_IMG.data)[imageMergeAssetIndex];
        char overlayPreview[320] = {};
        snprintf(overlayPreview, sizeof(overlayPreview), "%03d  gr %d  %s",
            imageMergeAssetIndex, overlayTag.gr,
            Cp932ToUtf8(overlayTag.name.body ? overlayTag.name.outstr() : "").c_str());
        ImGui::SetNextItemWidth(420.0f);
        if (ImGui::BeginCombo("Overlay Asset", overlayPreview)) {
            for (int assetIndex = 0; assetIndex < arr_IMG.count; ++assetIndex) {
                IMG& candidate = ((IMG*)arr_IMG.data)[assetIndex];
                char label[320] = {};
                snprintf(label, sizeof(label), "%03d  gr %d  %s",
                    assetIndex, candidate.gr,
                    Cp932ToUtf8(candidate.name.body
                        ? candidate.name.outstr() : "").c_str());
                if (ImGui::Selectable(label,
                    assetIndex == imageMergeAssetIndex))
                    imageMergeAssetIndex = assetIndex;
            }
            ImGui::EndCombo();
        }
        ImGui::SetNextItemWidth(420.0f);
        ImGui::InputText("Output file", imageToolOutputPathUtf8,
            sizeof(imageToolOutputPathUtf8));
        ImGui::Checkbox("Register in this skin CSV", &imageToolRegisterInCsv);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Adds #IMAGE plus a full-size editor Asset without renumbering existing gr IDs.");
        if (!imageToolStatus.empty())
            ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.42f, 1.0f), "%s",
                imageToolStatus.c_str());
        ImGui::Separator();
        if (ImGui::Button("Merge", ImVec2(100.0f, 0.0f))) {
            imageToolStatus.clear();
            IMG& selectedOverlay = ((IMG*)arr_IMG.data)[imageMergeAssetIndex];
            const int overlayTextureIndex = ResolveIMGTextureIndex(
                imageMergeAssetIndex);
            if (overlayTextureIndex < 0 ||
                !EnsureSRCGRTexture(overlayTextureIndex) || !img.texture) {
                imageToolStatus = "The base or overlay texture could not be loaded.";
            } else {
                SRCGR& overlayGraphic =
                    ((SRCGR*)arr_SRCGR.data)[overlayTextureIndex];
                const std::string enteredCp932 = Utf8ToCp932(
                    imageToolOutputPathUtf8);
                const std::string diskPath = ResolveGeneratedImageDiskPath(
                    enteredCp932.c_str(), mainpath);
                if (diskPath.empty()) {
                    imageToolStatus = "The output path is invalid.";
                } else if (imageToolRegisterInCsv &&
                    (CalculateTrailingGraphicId(skinfileLines) >= 100 ||
                        MakePortableGeneratedImagePath(diskPath.c_str(), mainpath)
                            .find(',') != std::string::npos)) {
                    imageToolStatus = "No free gr slot remains, or the CSV path contains a comma.";
                } else {
                    int mergedWidth = 0;
                    int mergedHeight = 0;
                    int placedX = 0;
                    int placedY = 0;
                    bool canvasExpanded = false;
                    char mergeError[256] = {};
                    if (!MergeTextureRegionAutoToImageFileAtomic(diskPath.c_str(),
                        img.texture, overlayGraphic.texture,
                        selectedOverlay.x, selectedOverlay.y,
                        selectedOverlay.w, selectedOverlay.h,
                        &placedX, &placedY, &canvasExpanded,
                        &mergedWidth, &mergedHeight,
                        mergeError, sizeof(mergeError))) {
                        imageToolStatus = mergeError;
                    } else {
                        int registeredGr = -1;
                        if (imageToolRegisterInCsv)
                            registeredGr = RegisterGeneratedImage(diskPath.c_str(),
                                mergedWidth, mergedHeight, imageToolStatus);
                        if (!imageToolRegisterInCsv || registeredGr >= 0) {
                            imageToolStatus = "Merged at " +
                                std::to_string(placedX) + ", " +
                                std::to_string(placedY);
                            if (canvasExpanded)
                                imageToolStatus += " with canvas expansion";
                            imageToolStatus += imageToolRegisterInCsv
                                ? "; registered as gr " +
                                    std::to_string(registeredGr) + "."
                                : ".";
                            ImGui::CloseCurrentPopup();
                        } else {
                            imageToolStatus = "The merged file was created, but CSV registration failed: " +
                                imageToolStatus;
                        }
                    }
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100.0f, 0.0f))) {
            imageToolStatus.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (!imageToolStatus.empty()) {
        const bool success = imageToolStatus.find("Created") == 0 ||
            imageToolStatus.find("Merged") == 0;
        ImGui::TextColored(success
            ? ImVec4(0.45f, 0.85f, 0.58f, 1.0f)
            : ImVec4(1.0f, 0.45f, 0.42f, 1.0f), "%s",
            imageToolStatus.c_str());
    }

    auto fitImageManagerZoom = [&]() {
        const ImVec2 available = ImGui::GetContentRegionAvail();
        const float fitX = img.sizeX > 0 ? (available.x - 20.0f) / img.sizeX : 1.0f;
        const float fitY = img.sizeY > 0 ? (available.y - 70.0f) / img.sizeY : 1.0f;
        float fit = fitX < fitY ? fitX : fitY;
        if (fit < 0.05f) fit = 0.05f;
        if (fit > 16.0f) fit = 16.0f;
        return fit;
    };
    if (!(ImageManagerZoom > 0.0f) || ImageManagerZoom > 16.0f)
        ImageManagerZoom = fitImageManagerZoom();
    float imageZoomPercent = ImageManagerZoom * 100.0f;
    ImGui::SetNextItemWidth(180.0f);
    if (ImGui::SliderFloat("Zoom##imageManagerZoom", &imageZoomPercent, 5.0f, 1600.0f,
        "%.0f%%", ImGuiSliderFlags_Logarithmic))
        ImageManagerZoom = imageZoomPercent / 100.0f;
    ImGui::SameLine();
    if (ImGui::Button("Fit##imageManagerZoomFit")) ImageManagerZoom = fitImageManagerZoom();
    ImGui::SameLine();
    if (ImGui::Button("100%##imageManagerZoomReset")) ImageManagerZoom = 1.0f;

    const std::string paintPath = img.path.body ? img.path.outstr() : "";
    const bool paintDirty = !paintPath.empty() &&
        imagePixelPaintDirtyPaths.find(paintPath) != imagePixelPaintDirtyPaths.end();
    if (ImGui::Checkbox("Pixel paint##imagePixelPaintMode", &imagePixelPaintMode)) {
        imagePixelPaintLastX = -1;
        imagePixelPaintLastY = -1;
        imagePixelPaintLastButton = -1;
        imagePixelPaintStatus.clear();
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(110.0f);
    ImGui::ColorEdit4("Color##imagePixelPaintColor",
        (float*)&imagePixelPaintColor,
        ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_NoInputs);
    ImGui::SameLine();
    ImGui::BeginDisabled(!paintDirty);
    if (ImGui::Button("Save image##imagePixelPaintSave")) {
        char saveError[256] = {};
        if (SaveTextureToImageFileAtomic(paintPath.c_str(), img.texture,
            saveError, sizeof(saveError))) {
            imagePixelPaintDirtyPaths.erase(paintPath);
            imagePixelPaintStatus =
                "Saved. Original backup: .skineditor-pixel.bak";
            for (int graphicIndex = 0; graphicIndex < arr_SRCGR.count; ++graphicIndex) {
                if (graphicIndex == gr_selected) continue;
                SRCGR& sibling = ((SRCGR*)arr_SRCGR.data)[graphicIndex];
                if (!sibling.path.body || _stricmp(sibling.path.outstr(),
                    paintPath.c_str()) != 0) continue;
                if (sibling.texture) sibling.texture->Release();
                sibling.texture = NULL;
                sibling.loaded = false;
            }
            // The script itself did not change. Rebuild texture-backed editor
            // state without marking the document as modified.
            editorDerivedRebuildPending = true;
            editorDerivedRebuildRequestedAt = 0;
            previewReloadPending = true;
            previewReloadRequestedAt = GetTickCount64();
            previewTextureDirty = true;
        } else {
            imagePixelPaintStatus = saveError;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Revert##imagePixelPaintRevert")) {
        for (int graphicIndex = 0; graphicIndex < arr_SRCGR.count; ++graphicIndex) {
            SRCGR& sibling = ((SRCGR*)arr_SRCGR.data)[graphicIndex];
            if (!sibling.path.body || _stricmp(sibling.path.outstr(),
                paintPath.c_str()) != 0) continue;
            if (sibling.texture) sibling.texture->Release();
            sibling.texture = NULL;
            sibling.loaded = false;
        }
        imagePixelPaintDirtyPaths.erase(paintPath);
        imagePixelPaintLastX = -1;
        imagePixelPaintLastY = -1;
        imagePixelPaintLastButton = -1;
        imagePixelPaintStatus = "Reverted unsaved pixel edits.";
        EnsureSRCGRTexture(gr_selected);
    }
    ImGui::EndDisabled();
    if (imagePixelPaintMode)
        ImGui::TextDisabled("Left: draw  |  Right: erase  |  Middle: pick color");
    if (!imagePixelPaintStatus.empty()) {
        if (imagePixelPaintStatus.find("Saved.") == 0 ||
            imagePixelPaintStatus.find("Reverted") == 0)
            ImGui::TextColored(ImVec4(0.45f, 0.85f, 0.58f, 1.0f), "%s",
                imagePixelPaintStatus.c_str());
        else
            ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.42f, 1.0f), "%s",
                imagePixelPaintStatus.c_str());
    }

    snprintf(title, sizeof(title), "ImgWorking##%d", num);
    if (ImGui::BeginChild(title, { -1, -1 }, ImGuiChildFlags_Borders | ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_HorizontalScrollbar)) {
        const ImVec2 pb = ImGui::GetCursorScreenPos();
        const ImVec2 oldImageDisplaySize(img.sizeX * ImageManagerZoom,
            img.sizeY * ImageManagerZoom);
        ApplyMouseCenteredWheelZoom(ImageManagerZoom, 0.05f, 16.0f,
            pb, oldImageDisplaySize);
        const float imageScale = ImageManagerZoom;
        const ImVec2 imageDisplaySize(img.sizeX * imageScale, img.sizeY * imageScale);
        const bool sharpImageManager = BeginSharpMagnifiedCanvas(imageScale);
        ImGui::Image(transBackground, imageDisplaySize, { 0,0 },
            { img.sizeX / (float)32, img.sizeY / (float)32 });
        ImGui::SetCursorScreenPos(pb);
        if (img.texture)
            ImGui::ImageWithBg(img.texture, imageDisplaySize, { 0,0 }, { 1, 1 }, bgColor);
        EndSharpMagnifiedCanvas(sharpImageManager);

        const ImVec2 paintCanvasMax(pb.x + imageDisplaySize.x,
            pb.y + imageDisplaySize.y);
        const bool paintCanvasHovered = imagePixelPaintMode && img.texture &&
            ImGui::IsMouseHoveringRect(pb, paintCanvasMax, true);
        if (paintCanvasHovered) {
            const ImVec2 mouse = ImGui::GetIO().MousePos;
            const int pixelX = (std::max)(0, (std::min)(img.sizeX - 1,
                (int)floorf((mouse.x - pb.x) / imageScale)));
            const int pixelY = (std::max)(0, (std::min)(img.sizeY - 1,
                (int)floorf((mouse.y - pb.y) / imageScale)));
            ImDrawList* paintDrawList = ImGui::GetWindowDrawList();
            const ImVec2 pixelMin(pb.x + pixelX * imageScale,
                pb.y + pixelY * imageScale);
            const ImVec2 pixelMax(pb.x + (pixelX + 1) * imageScale,
                pb.y + (pixelY + 1) * imageScale);
            paintDrawList->AddRect(pixelMin, pixelMax,
                IM_COL32(255, 255, 255, 255), 0.0f, 0, 1.0f);
            paintDrawList->AddRect(ImVec2(pixelMin.x - 1.0f, pixelMin.y - 1.0f),
                ImVec2(pixelMax.x + 1.0f, pixelMax.y + 1.0f),
                IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
                D3DCOLOR sampled = 0;
                if (ReadTexturePixel(img.texture, pixelX, pixelY, &sampled)) {
                    imagePixelPaintColor.x = ((sampled >> 16) & 0xff) / 255.0f;
                    imagePixelPaintColor.y = ((sampled >> 8) & 0xff) / 255.0f;
                    imagePixelPaintColor.z = (sampled & 0xff) / 255.0f;
                    imagePixelPaintColor.w = ((sampled >> 24) & 0xff) / 255.0f;
                    imagePixelPaintStatus = "Picked pixel color.";
                }
            }

            int paintButton = -1;
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) paintButton = 0;
            else if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) paintButton = 1;
            if (paintButton >= 0) {
                const int startX = imagePixelPaintLastButton == paintButton &&
                    imagePixelPaintLastX >= 0 ? imagePixelPaintLastX : pixelX;
                const int startY = imagePixelPaintLastButton == paintButton &&
                    imagePixelPaintLastY >= 0 ? imagePixelPaintLastY : pixelY;
                const int red = (std::max)(0, (std::min)(255,
                    (int)(imagePixelPaintColor.x * 255.0f + 0.5f)));
                const int green = (std::max)(0, (std::min)(255,
                    (int)(imagePixelPaintColor.y * 255.0f + 0.5f)));
                const int blue = (std::max)(0, (std::min)(255,
                    (int)(imagePixelPaintColor.z * 255.0f + 0.5f)));
                const int alpha = (std::max)(0, (std::min)(255,
                    (int)(imagePixelPaintColor.w * 255.0f + 0.5f)));
                const D3DCOLOR paintColor = paintButton == 1
                    ? D3DCOLOR_ARGB(0, 0, 0, 0)
                    : D3DCOLOR_ARGB(alpha, red, green, blue);
                bool painted = false;
                for (int graphicIndex = 0; graphicIndex < arr_SRCGR.count;
                    ++graphicIndex) {
                    SRCGR& sibling = ((SRCGR*)arr_SRCGR.data)[graphicIndex];
                    if (!sibling.path.body ||
                        _stricmp(sibling.path.outstr(), paintPath.c_str()) != 0)
                        continue;
                    EnsureSRCGRTexture(graphicIndex);
                    if (!sibling.texture || sibling.sizeX != img.sizeX ||
                        sibling.sizeY != img.sizeY) continue;
                    painted |= PaintTextureLine(sibling.texture, startX, startY,
                        pixelX, pixelY, paintColor);
                }
                if (painted) {
                    imagePixelPaintDirtyPaths[paintPath] = true;
                    imagePixelPaintStatus = paintButton == 1
                        ? "Unsaved transparent pixels." : "Unsaved painted pixels.";
                } else {
                    imagePixelPaintStatus =
                        "This texture format cannot be edited pixel-by-pixel.";
                }
                imagePixelPaintLastX = pixelX;
                imagePixelPaintLastY = pixelY;
                imagePixelPaintLastButton = paintButton;
            } else {
                imagePixelPaintLastX = -1;
                imagePixelPaintLastY = -1;
                imagePixelPaintLastButton = -1;
            }
        } else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
            !ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
            imagePixelPaintLastX = -1;
            imagePixelPaintLastY = -1;
            imagePixelPaintLastButton = -1;
        }


        if (img.texture != NULL) {
            const ImVec2 p = ImGui::GetCursorScreenPos();
            ImVec2 grpos = { p.x, p.y - imageDisplaySize.y - 4 };

            IMG& src = ((IMG*)arr_IMG.data)[src_selected];
            int sizeX = src.w == -1 ? img.sizeX - src.x : src.w;
            int sizeY = src.h == -1 ? img.sizeY - src.y : src.h;

            ImVec2 srcposLU = { grpos.x + src.x * imageScale - 1,
                grpos.y + src.y * imageScale - 1 };
            ImVec2 srcposRB = { grpos.x + (src.x + sizeX) * imageScale + 1,
                grpos.y + (src.y + sizeY) * imageScale + 1 };

            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            bool flicking = ((int)GetTimeLapse(1, &g.timer1) % 200 > 100);
            ImColor color = flicking ? 0xff0080ff : 0x000000;
            draw_list->AddRect(srcposLU, srcposRB, color, 0.0f, ImDrawFlags_Closed, 1.0f);

            if (clicked) {
                ImGui::SetScrollX(src.x * imageScale);
                ImGui::SetScrollY(src.y * imageScale);
            }

            for (int i = 0; i < arr_IMG.count; i++) {
                IMG& hoversrc = ((IMG*)arr_IMG.data)[i];
                if (hoversrc.gr != src.gr || hoversrc.ifGroup != src.ifGroup) continue;
                srcposLU = { grpos.x + hoversrc.x * imageScale,
                    grpos.y + hoversrc.y * imageScale };
                srcposRB = { grpos.x + (hoversrc.x + hoversrc.w) * imageScale,
                    grpos.y + (hoversrc.y + hoversrc.h) * imageScale };
                if (ImGui::IsMouseHoveringRect(srcposLU, srcposRB)) {
                    flicking = ((int)GetTimeLapse(1, &g.timer1) % 200 < 100);
                    ImColor color = flicking ? 0xffff0000 : 0x000000;
                    draw_list->AddRect(srcposLU, srcposRB, color, 0.0f, ImDrawFlags_Closed, 1.0f);

                    if (ImGui::IsItemClicked()) {
                        src_selected = i;
                    }
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone) && ImGui::BeginTooltip())
                    {
                        ImGui::Text("%03d : %d %d ~ %d %d", i, hoversrc.x, hoversrc.y, hoversrc.x + hoversrc.w, hoversrc.y + hoversrc.h);
                        ImGui::Text("Size %d %d", hoversrc.w, hoversrc.h);
                        ImGui::EndTooltip();
                    }
                    
                }
            }

            if (newSquare) {
                ImGui::BeginTooltip();
                ImGui::Text("click any image");
                ImGui::EndTooltip();
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    ImGuiIO& io = ImGui::GetIO();
                    clickPos = { (io.MousePos.x - grpos.x) / imageScale,
                        (io.MousePos.y - grpos.y) / imageScale };
                    
                    int x = clickPos.x, y = clickPos.y, w, h;
                    AutoSRCObjectPos( &((SRCGR*)arr_SRCGR.data)[gr_selected], &x,&y,&w,&h);
                    SRCGR& selectedGraphic = ((SRCGR*)arr_SRCGR.data)[gr_selected];
                    const int logicalGr = selectedGraphic.grID;
                    const int graphicIfgroup = selectedGraphic.isIf;
                    const int graphicDeclare = selectedGraphic.declare;
                    CSTR assetOwner(mainpath);
                    if (graphicDeclare >= 0 && graphicDeclare < skinfileLines.count) {
                        SKINFILELINEREAD& declaration =
                            ((SKINFILELINEREAD*)skinfileLines.data)[graphicDeclare];
                        if (declaration.filename.body && *declaration.filename.outstr())
                            assetOwner.assign(declaration.filename);
                    }

                    const int newImageIndex = NewIMG(logicalGr,
                        x, y, w, h, graphicIfgroup);
                    if (newImageIndex >= 0) {
                        // Persist a reusable crop without making LR2 render an
                        // Object. The fields intentionally mirror #SRC_IMAGE;
                        // only the command head changes to '$'.
                        char assetLine[256];
                        snprintf(assetLine, sizeof(assetLine),
                            "$SRC_IMAGE,0,%d,%d,%d,%d,%d,1,1,0,0,0,0,0",
                            logicalGr, x, y, w, h);
                        int metadataInsertAt = graphicDeclare >= 0
                            ? graphicDeclare + 1
                            : FindOwnerFileEndRow(skinfileLines, assetOwner.outstr());
                        while (metadataInsertAt >= 0 &&
                            metadataInsertAt < skinfileLines.count) {
                            SKINFILELINEREAD& following =
                                ((SKINFILELINEREAD*)skinfileLines.data)[metadataInsertAt];
                            const char* followingText = following.line.body
                                ? following.line.outstr() : "";
                            if (strncmp(followingText, "$SRC_IMAGE,", 11) != 0) break;
                            ++metadataInsertAt;
                        }
                        if (InsertLine(metadataInsertAt) == 0) {
                            SKINFILELINEREAD& metadata =
                                ((SKINFILELINEREAD*)skinfileLines.data)[metadataInsertAt];
                            CSTR placeholder(metadata.line);
                            EditLine(metadataInsertAt, placeholder, CSTR(assetLine));
                            metadata.filename.assign(assetOwner);
                            metadata.ifgroup = graphicIfgroup;
                            SplitCSV(metadata.line, &metadata.csv, ",");
                            metadata.csvColumnCount = CountCsvColumns(metadata.line);
                            ((IMG*)arr_IMG.data)[newImageIndex].editorDeclare =
                                metadataInsertAt;
                        }
                        assetSearch[0] = '\0';
                        assetBrowserFocusRequest = newImageIndex;
                        SelectIMGAsset(newImageIndex, false);
                        clicked = true;
                    }

                    newSquare = 0;
                }
            }
        }
        

        
    }
    ImGui::EndChild();
    ImGui::EndGroup();

    ImGui::End();
    return 0;
}

//stretch find
int AutoSRCObjectPos(SRCGR* gr, int* x, int* y, int* w, int* h) {

    D3DLOCKED_RECT lockedRect;

    HRESULT hr = gr->texture->LockRect(0, &lockedRect, NULL, D3DLOCK_READONLY);

    if (SUCCEEDED(hr)) {
        DWORD* pPixelData = (DWORD*)lockedRect.pBits;

        int xCur = *x, yCur = *y;
        int wCur = 1, hCur = 1;

        while (1) {
            bool xDone = true, yDone = true;

            //expand x
            for (int cur = yCur; cur < yCur + hCur; cur++) {
                while (pPixelData[cur * (lockedRect.Pitch / 4) + xCur - 1] & 0xFF000000) {
                    xCur--;
                    xDone = false;
                }
                while (pPixelData[cur * (lockedRect.Pitch / 4) + xCur + wCur] & 0xFF000000) {
                    wCur++;
                    xDone = false;
                }
            }
            
            //expand y
            for (int cur = xCur; cur < xCur + wCur; cur++) {
                while (pPixelData[(yCur - 1) * (lockedRect.Pitch / 4) + cur] & 0xFF000000) {
                    yCur--;
                    yDone = false;
                }
                while (pPixelData[(yCur + hCur) * (lockedRect.Pitch / 4) + cur] & 0xFF000000) {
                    hCur++;
                    yDone = false;
                }
            }
            //check
            if (xDone && yDone) break;
        }

        *x = xCur;
        *y = yCur;
        *w = wCur;
        *h = hCur;
        
        gr->texture->UnlockRect(0);
    }
    return 0;
}

int WORKSPACE::drawSrc(int iSRCGR, int iSRCID, int posX, int posY, int w, int h, bool stretch) {
    EnsureSRCGRTexture(iSRCGR);
    SRCGR& img = ((SRCGR*)arr_SRCGR.data)[iSRCGR];

    if (img.texture != NULL) {
        SRC& src = ((SRC*)arr_SRC.data)[iSRCID];

        int sizeX = src.sizeX == -1 ? img.sizeX - src.x : src.sizeX;
        int sizeY = src.sizeY == -1 ? img.sizeY - src.y : src.sizeY;
        ImVec2 display_min, display_max, display_size;
        if (stretch) {
            display_min = ImVec2(src.x / (float)img.sizeX, src.y / (float)img.sizeY);
            display_max = ImVec2((src.x + sizeX) / (float)img.sizeX, (src.y + sizeY) / (float)img.sizeY);
            display_size = ImVec2((float)w, (float)h);
        }
        else {
            display_min = ImVec2(src.x / (float)img.sizeX, src.y / (float)img.sizeY);
            display_max = ImVec2((src.x + sizeX) / (float)img.sizeX, (src.y + sizeY) / (float)img.sizeY);
            display_size = ImVec2(sizeX, sizeY);
        }

        if (src.cycle && (src.div_x >= 1 || src.div_y >= 1)) {
            if (src.div_x == 0) src.div_x = 1;
            if (src.div_y == 0) src.div_y = 1;
            ImVec2 chopsize = { src.sizeX / (float)src.div_x , src.sizeY / (float)src.div_y };

            int units = src.div_x * src.div_y;
            int tick = src.cycle / units;
            int ani = ((int)GetTimeLapse(0, &(g.timer1)) % src.cycle) / tick;

            int ax = ani % src.div_x;
            int ay = ani / src.div_x;

            ImVec2 chopstart = { (src.x + chopsize.x * ax) / (float)img.sizeX ,
                                (src.y + chopsize.y * ay) / (float)img.sizeY };

            ImVec2 chopend = { (src.x - 1 + chopsize.x * (ax + 1)) / (float)img.sizeX ,
                                (src.y - 1 + chopsize.y * (ay + 1)) / (float)img.sizeY };

            
            //ImVec2 pb = ImGui::GetCursorScreenPos();
            //ImVec2 pos = { (float)posX, (float)posY};
            //ImGui::SetCursorPos(pb);
            ImGui::Image(img.texture, chopsize, chopstart, chopend);
            /*ImGui::SetCursorPos(pb);*/
            /*ImGui::Dummy(pb);*/
        }
        else {
            /*ImVec2 pb = ImGui::GetCursorScreenPos();
            ImVec2 pos = { (float)posX, (float)posY};
            ImGui::SetCursorPos(pb);*/
            ImGui::Image(img.texture, display_size, display_min, display_max);;
            /*ImGui::SetCursorPos(pb);*/
            /*ImGui::Dummy(pb);*/
        }
    }
    return 0;
}

int WORKSPACE::drawSrc(int iSRCGR, int iSRCID) {
    return drawSrc(iSRCGR, iSRCID, 0, 0);
}

//int WORKSPACE::drawSrc(IMG img) {
//
//    return 0;
//}


int WORKSPACE::drawSaveMenu() {
    char title[260], input[32], result[32];
    snprintf(title, sizeof(title), "SaveMenu##%d", num);
    snprintf(input, sizeof(input), "##savePathInput%d", num);


    if(ImGui::Begin(title, &wSaveMenu, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_Modal)) {

        ImGui::Text("old path is %s ", mainpath);
        if (newPath[0] == '\0') { //very init
            strncpy_s(newPath, mainpath, _TRUNCATE);
        }

        if (ImGui::Button("BROWSE", { 0, 0 })) {
            BrowseSkinSavePath(newPath, newPath, IM_ARRAYSIZE(newPath));
        }
        ImGui::SameLine(0, 0);
        ImGui::InputText(input, newPath, IM_ARRAYSIZE(newPath));
        exist = IsFileExist(newPath) || !strcmp(mainpath, newPath); //TODO reduce cpu usage

        if (exist) ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "CAUTION: OVERWRITE");
        ImGui::SeparatorText("Script files");
        ImGui::RadioButton("merge scripts", &split, 0);
        ImGui::RadioButton("split scripts", &split, 1);
        if (split)
            ImGui::TextColored(ImVec4(1.0f, 0.78f, 0.28f, 1.0f),
                "Included scripts stay linked and are saved to their existing paths.");
        else
            ImGui::TextDisabled("Included scripts are embedded in the new main script.");
        ImGui::TextDisabled("Image, font, and other resource files are not copied.");
        if (_stricmp(mainpath, newPath) != 0)
            ImGui::TextColored(ImVec4(0.35f, 0.75f, 1.0f, 1.0f),
                "After Save As, editing will continue on the new main script.");
        ImGui::SeparatorText("Comments");
        ImGui::RadioButton("maintain memo", &nocomment, 0);
        ImGui::RadioButton("delete memo", &nocomment, 1);
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone) && ImGui::BeginTooltip()){
            ImGui::Text("this will remove all group, only for Scene start speed on LR2.\nAre your sure this?");
            ImGui::EndTooltip();
        }
        
        snprintf(result, sizeof(result), "SaveResult##Save%d", num);
        if (ImGui::Button("SAVE", { 0, 0 })) {
            char previousMainPath[MAX_PATH] = {};
            strncpy_s(previousMainPath, mainpath, _TRUNCATE);
            success = (SaveSkinScript(newPath, split, nocomment) == 0);
            if (success && _stricmp(previousMainPath, newPath) != 0) {
                // Save As continues editing the newly written main script.
                // In merged mode every expanded line now belongs to that file;
                // in split mode only the former main-file lines change owner.
                for (int row = 0; row < skinfileLines.count; ++row) {
                    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    const char* owner = line.filename.body ? line.filename.outstr() : previousMainPath;
                    if (!split || _stricmp(owner, previousMainPath) == 0)
                        line.filename.assign(newPath);
                }
                strncpy(mainpath, newPath, MAX_PATH - 1);
                mainpath[MAX_PATH - 1] = '\0';
            }
            if (success) {
                MarkDocumentSaved();
                lastSaveMessage = "Saved As and switched the workspace path";
            } else {
                lastSaveState = -1;
                lastSaveMessage = "Save As failed; original files were preserved";
                lastSaveMessageAt = GetTickCount64();
            }
            ImGui::OpenPopup(result);
        }
        
        if (ImGui::BeginPopupModal(result, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            if (success) {
                ImGui::TextUnformatted("Saved successfully.");
                ImGui::TextWrapped("The workspace is now using: %s", mainpath);
            } else {
                ImGui::TextUnformatted("Save failed - original files were preserved.");
            }
            if (ImGui::Button("OK")) {
                wSaveMenu = 0;
                success = 0;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::End();
    }
    return 0;
}
//{
//    CSTR savepath(mainpath);
//    savepath.add("_master");
//    SaveMaster(savepath);
//}

int WORKSPACE::drawSaveMenu2() {
    char title[260], input[32], result[32];
    snprintf(title, sizeof(title), "SaveMenu##%d", num);
    snprintf(input, sizeof(input), "##savePathInput%d", num);


    if (ImGui::Begin(title, &wSaveMenu2, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_Modal)) {

        ImGui::Text("old path is %s ", mainpath);
        if (newPath[0] == '\0') { //very init
            strncpy(newPath, mainpath, sizeof(newPath));
        }

        if (ImGui::Button("BROWSE", { 0, 0 })) {
            //TODO
        }
        ImGui::SameLine(0, 0);
        ImGui::InputText(input, newPath, IM_ARRAYSIZE(newPath));
        exist = IsFileExist(newPath) || !strcmp(mainpath, newPath); //TODO reduce cpu usage

        if (exist) ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "CAUTION: OVERWRITE");
        ImGui::SeparatorText("Script files");
        ImGui::RadioButton("merge scripts", &split, 0);
        ImGui::RadioButton("split scripts", &split, 1);
        ImGui::SeparatorText("Comments");
        ImGui::RadioButton("maintain memo", &nocomment, 0);
        ImGui::RadioButton("delete memo", &nocomment, 1);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone) && ImGui::BeginTooltip()) {
            ImGui::Text("this will remove all group, only for Scene start speed on LR2.\nAre your sure this?");
            ImGui::EndTooltip();
        }

        if (ImGui::Button("SAVE", { 0, 0 })) {
            success = (SaveSkinScript2(newPath, split, nocomment) == 0);
            if (success) snprintf(result, sizeof(result), "SaveResult##Save%d", num);
            ImGui::OpenPopup(result);
        }

        snprintf(result, sizeof(result), "SaveResult##Save%d", num);
        if (ImGui::BeginPopupModal(result, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted(success ? "save success" : "save failed - original files were preserved");
            if (ImGui::Button("OK")) {
                wSaveMenu2 = 0;
                success = 0;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::End();
    }
    return 0;
}

int WORKSPACE::drawFileManager() {
    char title[260];
    snprintf(title, sizeof(title), "FileManager##%d", num);

    ImGui::Begin(title, &wFileManager);
    //ImGui::Text("they are related files. %d scripts, %d images", arr_subpath.count, arr_imgpath.count);
    ImGui::SeparatorText("Scripts");
    for (int i = 0; i < arr_subpath.count; i++) {
        CSTR& path = ((CSTR*)arr_subpath.data)[i];
        ImGui::Text("%s", path.outstr());
    }
    ImGui::SeparatorText("Images");
    for (int i = 0; i < arr_SRCGR.count; i++) {
        SRCGR& img = ((SRCGR*)arr_SRCGR.data)[i];
        ImGui::Text("(%03d)%02d - %s %s", img.isIf, img.grID, img.path.outstr(), img.fromWildcard? "from *":"");
    }

    ImGui::End();
    return 0;
}

int WORKSPACE::drawTreeView() {
    char title[260];
    snprintf(title, sizeof(title), "TreeView##%d", num);
    ImGui::Begin(title, &wTreeView);

    snprintf(title, sizeof(title), "Tree##%d", num);
    

    if (ImGui::BeginTable(title, 24, ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody)) //ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody))
    {

    //    // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
    //    ImGui::TableSetupColumn("Command", ImGuiTableColumnFlags_NoHide);
    //    for (int p = 0; p < 24 - 1; p++) {
    //        ImGui::TableSetupColumn("Params", ImGuiTableColumnFlags_WidthFixed, 12.0f);
    //    }
    //    ImGui::TableHeadersRow();

    //    /*static int hideGroups[MAX_IFDEPTH];
    //    for (int i = 0; i < MAX_IFDEPTH; i++) {
    //        hideGroups[i] = -1;
    //    }*/
    //    
    //    for (int l = 0; l < skinfileLines.count; l++) {
    //        int hide = 0;
    //        SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[l];
    //        ImGui::TableNextRow();
    //        ImGui::TableNextColumn();

    //        if (line.isComment) continue;
    //        CSTR& command = line.csv.str[0];
    //        CSTR *params = line.csv.str;
    //        bool is_head = //!(strcmp(command, "#INCLUDE")) ||
    //                        !(strcmp(command, "#IF")) ||
    //                        !(strcmp(command, "#ELSEIF")) ||
    //                        !(strcmp(command, "#ELSE"));
    //        bool is_end = !(strcmp(command, "#ELSEIF")) ||
    //                        !(strcmp(command, "#ELSE")) ||
    //                        !(strcmp(command, "#ENDIF"));

    //        static ImGuiTreeNodeFlags tree_node_flags_base = ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_DrawLinesFull;
    //        ImGuiTreeNodeFlags node_flags = tree_node_flags_base;


    //        int lineID = ((SKINFILELINEREAD*)skinfileLines.data)[l].numTotal;
    //        sprintf(title, "%s##%d", command.outstr(), lineID);

    //        if (is_end) {
    //            ImGui::TreePop();
    //        }
    //        if (is_head)
    //        {   
    //            bool open = ImGui::TreeNodeEx(title, node_flags);
    //            
    //            for (int p = 1; p < 24 - 1; p++) {
    //                ImGui::TableNextColumn();
    //                ImGui::Text("%s", params[p].outstr());
    //            };
    //        }
    //        
    //        /*else if (!hide){
    //            ImGui::TreeNodeEx(title, node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);

    //            for (int p = 1; p < 24 - 1; p++) {
    //                ImGui::TableNextColumn();
    //                ImGui::Text("%s", params[p]);
    //            };
    //        }*/

    //    }        
        ImGui::EndTable();
    }

    ImGui::End();
    return 0;
}

int WORKSPACE::wildcardTOAll(char* path) {
    WIN32_FIND_DATA FindFileData;
    LPWIN32_FIND_DATAA lpFindFileData;
    HANDLE hFindFile = FindFirstFileA(path, (LPWIN32_FIND_DATAA)&FindFileData);
    if (hFindFile == (HANDLE)-1) {
        return -1;
    }




    return 0;
}

namespace {
    static bool IsPlayPresetType(int type) {
        return (type >= 0 && type <= 4) || (type >= 12 && type <= 14);
    }

    static bool IsInitialPresetType(int type) {
        return IsPlayPresetType(type) || type == 5 || type == 6 || type == 7;
    }

    static bool IsSafePresetRelativePath(const std::string& value) {
        if (value.empty() || value[0] == '\\' || value[0] == '/' || value.find(':') != std::string::npos)
            return false;
        std::filesystem::path path(value);
        for (const std::filesystem::path& part : path) {
            if (part == ".." || part == ".") return false;
        }
        return true;
    }

    static std::string CsvSafeField(const std::string& value) {
        std::string result = value;
        for (char& ch : result) {
            if (ch == ',' || ch == '\r' || ch == '\n') ch = ' ';
        }
        return result;
    }

    static bool WritePresetAtlasBmp(const std::filesystem::path& outputPath, std::string& error) {
        const int width = 256, height = 256;
        const int rowBytes = (width * 3 + 3) & ~3;
        std::vector<unsigned char> bmp(54 + rowBytes * height, 0);
        auto put16 = [&](int offset, unsigned value) {
            bmp[offset] = (unsigned char)(value & 0xff);
            bmp[offset + 1] = (unsigned char)((value >> 8) & 0xff);
        };
        auto put32 = [&](int offset, unsigned value) {
            for (int i = 0; i < 4; ++i) bmp[offset + i] = (unsigned char)((value >> (i * 8)) & 0xff);
        };
        bmp[0] = 'B'; bmp[1] = 'M';
        put32(2, (unsigned)bmp.size());
        put32(10, 54); put32(14, 40); put32(18, width); put32(22, height);
        put16(26, 1); put16(28, 24); put32(34, rowBytes * height);

        auto pixelColor = [&](int x, int y, unsigned char& r, unsigned char& g, unsigned char& b) {
            r = 14; g = 20; b = 31; // background tile at x >= 192
            if (x < 192) {
                if (y < 16) { r = 235; g = 242; b = 255; }
                else if (y < 32) { r = 70; g = 180; b = 255; }
                else if (y < 48) { r = 255; g = 70; b = 85; }
                else if (y < 64) { r = 70; g = 230; b = 150; }
                else if (y < 72) { r = 255; g = 220; b = 70; }
                else if (y < 104) {
                    const int band = (x / 32) % 6;
                    static const unsigned char colors[6][3] = {
                        { 250, 250, 250 }, { 90, 210, 255 }, { 100, 255, 150 },
                        { 255, 220, 80 }, { 255, 120, 80 }, { 255, 70, 100 }
                    };
                    r = colors[band][0]; g = colors[band][1]; b = colors[band][2];
                }
                else if (y < 120) {
                    const int digit = (x / 16) % 10;
                    const bool edge = (x % 16 < 2) || (x % 16 > 13) || (y % 16 < 2) || (y % 16 > 13);
                    const unsigned char shade = edge ? 40 : (unsigned char)(120 + digit * 12);
                    r = shade; g = (unsigned char)(210 - digit * 8); b = 255;
                }
                else if (y < 136) { r = 75; g = 235; b = 105; }
            }
        };
        for (int y = 0; y < height; ++y) {
            const int fileY = height - 1 - y;
            for (int x = 0; x < width; ++x) {
                unsigned char r, g, b;
                pixelColor(x, y, r, g, b);
                const size_t pos = 54 + (size_t)fileY * rowBytes + (size_t)x * 3;
                bmp[pos] = b; bmp[pos + 1] = g; bmp[pos + 2] = r;
            }
        }

        const std::filesystem::path tempPath = outputPath.string() + ".tmp";
        std::ofstream output(tempPath, std::ios::binary | std::ios::trunc);
        if (!output) { error = "Could not create the preset image."; return false; }
        output.write((const char*)bmp.data(), (std::streamsize)bmp.size());
        output.close();
        if (!output) { std::filesystem::remove(tempPath); error = "Could not write the preset image."; return false; }
        std::error_code ec;
        std::filesystem::rename(tempPath, outputPath, ec);
        if (ec) { std::filesystem::remove(tempPath); error = "Could not finalize the preset image."; return false; }
        return true;
    }

    static void AppendDst(std::ostringstream& skin, const char* command, int index,
        int x, int y, int w, int h, int timer = 0) {
        skin << command << ',' << index << ",0," << x << ',' << y << ',' << w << ',' << h
            << ",0,255,255,255,255,0,0,0,0,0," << timer << ",0,0,0\r\n";
    }

    static void AppendSelectPreset(std::ostringstream& skin, int width, int height) {
        const int barW = (std::max)(240, width * 54 / 100);
        const int barH = (std::max)(24, height / 15);
        const int barX = (width - barW) / 2;
        const int centerY = height / 2 - barH / 2;

        skin << "$SE_OBJECT_NAME,Selection Bar Sources\r\n$SE_OBJECT_ID,preset_select_bar_sources\r\n";
        for (int barType = 0; barType < 10; ++barType)
            skin << "#SRC_BAR_BODY," << barType << ",0,0,72,192,32,1,1,0,0\r\n";
        for (int slot = 0; slot <= 20; ++slot) {
            const int y = centerY + (slot - 10) * barH;
            skin << "#DST_BAR_BODY_OFF," << slot << ",0," << barX << ',' << y << ',' << barW << ',' << barH
                << ",0,210,150,165,185,0,0,0,0,0,0\r\n";
            skin << "#DST_BAR_BODY_ON," << slot << ",0," << barX << ',' << y << ',' << barW << ',' << barH
                << ",0,255,255,255,255,0,0,0,0,0,0\r\n";
        }
        skin << "#BAR_CENTER,10\r\n#BAR_AVAILABLE,0,20\r\n";

        skin << "$SE_OBJECT_NAME,Selection Bar Titles\r\n$SE_OBJECT_ID,preset_select_bar_titles\r\n";
        skin << "#FONT," << (std::max)(14, height / 32) << ",2,2,Arial\r\n";
        skin << "#SRC_BAR_TITLE,0,0,0,0\r\n";
        skin << "#DST_BAR_TITLE,0,0,16,5," << (barW - 32)
            << ',' << (std::max)(14, barH * 55 / 100) << ",0,255,255,255,255,0,0,0,0,0,0,0,0,0\r\n";
        skin << "#SRC_BAR_TITLE,1,0,0,0\r\n";
        skin << "#DST_BAR_TITLE,1,0,16,5," << (barW - 32)
            << ',' << (std::max)(14, barH * 55 / 100) << ",0,255,255,235,120,0,0,0,0,0,0,0,0,0\r\n";
    }

    static void AppendDecidePreset(std::ostringstream& skin, int width, int height) {
        const int panelW = width * 68 / 100;
        const int panelH = height * 46 / 100;
        const int panelX = (width - panelW) / 2;
        const int panelY = (height - panelH) / 2;

        skin << "$SE_OBJECT_NAME,Decision Panel\r\n$SE_OBJECT_ID,preset_decide_panel\r\n";
        skin << "#SRC_IMAGE,0,0,0,72,192,32,1,1,0,0,0,0,0\r\n";
        AppendDst(skin, "#DST_IMAGE", 0, panelX, panelY, panelW, panelH);

        skin << "$SE_OBJECT_NAME,Decision Accent\r\n$SE_OBJECT_ID,preset_decide_accent\r\n";
        skin << "#SRC_IMAGE,0,0,0,64,192,8,1,1,0,0,0,0,0\r\n";
        AppendDst(skin, "#DST_IMAGE", 0, panelX, panelY, panelW, (std::max)(4, height / 90));
        AppendDst(skin, "#DST_IMAGE", 0, panelX, panelY + panelH - (std::max)(4, height / 90),
            panelW, (std::max)(4, height / 90));

        skin << "$SE_OBJECT_NAME,Decision Flash\r\n$SE_OBJECT_ID,preset_decide_flash\r\n";
        skin << "#SRC_IMAGE,0,0,0,0,192,16,1,1,0,0,0,0,0\r\n";
        skin << "#DST_IMAGE,0,0," << panelX << ',' << (panelY + panelH / 2) << ',' << panelW << ','
            << (std::max)(8, height / 30) << ",0,0,255,255,255,1,0,0,0,500,0,0,0,0\r\n";
        skin << "#DST_IMAGE,0,500," << panelX << ',' << (panelY + panelH / 2) << ',' << panelW << ','
            << (std::max)(8, height / 30) << ",0,220,255,255,255,1,0,0,0,0,0,0,0,0\r\n";
    }

    static void AppendResultNumber(std::ostringstream& skin, const char* name, const char* id,
        int numberId, int x, int y, int digitW, int digitH, int digits) {
        skin << "$SE_OBJECT_NAME," << name << "\r\n$SE_OBJECT_ID," << id << "\r\n";
        skin << "#SRC_NUMBER,0,0,0,104,160,16,10,1,0,0," << numberId << ",0," << digits << "\r\n";
        AppendDst(skin, "#DST_NUMBER", 0, x, y, digitW, digitH);
    }

    static void AppendResultPreset(std::ostringstream& skin, int width, int height) {
        const int panelX = width * 8 / 100;
        const int panelY = height * 14 / 100;
        const int panelW = width * 84 / 100;
        const int panelH = height * 70 / 100;
        const int digitW = (std::max)(12, width / 55);
        const int digitH = (std::max)(18, height / 24);

        skin << "$SE_OBJECT_NAME,Result Panel\r\n$SE_OBJECT_ID,preset_result_panel\r\n";
        skin << "#SRC_IMAGE,0,0,0,72,192,32,1,1,0,0,0,0,0\r\n";
        AppendDst(skin, "#DST_IMAGE", 0, panelX, panelY, panelW, panelH);

        const int leftX = panelX + panelW / 12;
        const int rightX = panelX + panelW * 58 / 100;
        const int firstY = panelY + panelH / 8;
        const int row = digitH * 3 / 2;
        AppendResultNumber(skin, "EX Score", "preset_result_exscore", 101, leftX, firstY, digitW, digitH, 6);
        AppendResultNumber(skin, "Max Combo", "preset_result_maxcombo", 105, leftX, firstY + row, digitW, digitH, 5);
        AppendResultNumber(skin, "Perfect", "preset_result_perfect", 110, rightX, firstY, digitW, digitH, 5);
        AppendResultNumber(skin, "Great", "preset_result_great", 111, rightX, firstY + row, digitW, digitH, 5);
        AppendResultNumber(skin, "Miss Count", "preset_result_miss", 114, rightX, firstY + row * 2, digitW, digitH, 5);

        const int chartX = leftX;
        const int chartY = panelY + panelH * 60 / 100;
        const int chartW = panelW * 38 / 100;
        const int chartH = panelH * 24 / 100;
        skin << "$SE_OBJECT_NAME,Groove Gauge Chart\r\n$SE_OBJECT_ID,preset_result_gauge_chart\r\n";
        skin << "#SRC_GAUGECHART_1P,0,0,0,120,2,2,1,1,0,0," << chartW << ',' << chartH << ",500,2000\r\n";
        AppendDst(skin, "#DST_GAUGECHART_1P", 0, chartX, chartY, 2, 2);
        skin << "$SE_OBJECT_NAME,Score Chart\r\n$SE_OBJECT_ID,preset_result_score_chart\r\n";
        skin << "#SRC_SCORECHART,0,0,0,16,2,2,1,1,0,0," << chartW << ',' << chartH << ",500,2000\r\n";
        AppendDst(skin, "#DST_SCORECHART", 0, chartX + panelW * 46 / 100, chartY, 2, 2);
    }

    static bool BuildInitialPreset(int type, int width, int height, const std::string& titleUtf8,
        const std::string& makerUtf8, const std::string& relativeSkinUtf8,
        std::string& outputSkinPath, std::string& outputAtlasPath, std::string& error) {
        if (!IsInitialPresetType(type)) { error = "This scene does not have a preset yet."; return false; }
        if (!IsSafePresetRelativePath(relativeSkinUtf8)) {
            error = "Use a relative path below LR2files/Theme without '..' or a drive name.";
            return false;
        }

        std::string relativeSkin = Utf8ToCp932(relativeSkinUtf8.c_str());
        std::replace(relativeSkin.begin(), relativeSkin.end(), '/', '\\');
        if (relativeSkin.size() < 8 || _stricmp(relativeSkin.c_str() + relativeSkin.size() - 8, ".lr2skin") != 0)
            relativeSkin += ".lr2skin";
        const std::filesystem::path skinPath = std::filesystem::path("LR2files") / "Theme" / relativeSkin;
        const std::filesystem::path atlasPath = skinPath.parent_path() / "preset.bmp";
        if (std::filesystem::exists(skinPath) || std::filesystem::exists(atlasPath)) {
            error = "The skin file or preset.bmp already exists. Choose a new folder/path.";
            return false;
        }
        std::error_code ec;
        std::filesystem::create_directories(skinPath.parent_path(), ec);
        if (ec) { error = "Could not create the skin directory."; return false; }

        std::string atlasScriptPath = atlasPath.string();
        std::replace(atlasScriptPath.begin(), atlasScriptPath.end(), '/', '\\');
        const std::string title = CsvSafeField(Utf8ToCp932(titleUtf8.empty() ? "New Skin" : titleUtf8.c_str()));
        const std::string maker = CsvSafeField(Utf8ToCp932(makerUtf8.empty() ? "SkinEditor" : makerUtf8.c_str()));

        int keysPerPlayer = 7;
        if (type == 1 || type == 3 || type == 13) keysPerPlayer = 5;
        else if (type == 4 || type == 14) keysPerPlayer = 9;
        const bool twoPlayers = type == 2 || type == 3 || type >= 12;
        const bool hasScratch = keysPerPlayer != 9;
        const int lanesPerPlayer = keysPerPlayer + (hasScratch ? 1 : 0);
        const int playerCount = twoPlayers ? 2 : 1;
        const int margin = (std::max)(16, width / 40);
        const int gap = twoPlayers ? (std::max)(12, width / 50) : 0;
        const int fieldWidth = twoPlayers ? (width - margin * 2 - gap) / 2 : (std::max)(220, width * 38 / 100);
        const int fieldTop = (std::max)(20, height / 18);
        const int judgeY = height * 78 / 100;
        const int laneHeight = (std::max)(8, height / 60);

        std::ostringstream skin;
        skin << "// Generated by SkinEditor initial preset\r\n";
        skin << "#INFORMATION," << type << ',' << title << ',' << maker << ',' << atlasScriptPath
            << ",," << width << ',' << height << "\r\n";
        skin << "#RESOLUTION," << width << ',' << height << "\r\n";
        skin << "#ENDOFHEADER\r\n\r\n";
        if (type == 5)
            skin << "#STARTINPUT,1000\r\n#LOADSTART,0\r\n#SCENETIME,3600000\r\n#FADEOUT,500\r\n#CLOSE,1000\r\n";
        else if (type == 6)
            skin << "#STARTINPUT,500\r\n#LOADSTART,0\r\n#SCENETIME,3000\r\n#SKIP,250\r\n#FADEOUT,550\r\n#CLOSE,1000\r\n";
        else if (type == 7)
            skin << "#STARTINPUT,500,1600,500\r\n#LOADSTART,0\r\n#SCENETIME,3600000\r\n#FADEOUT,550\r\n#CLOSE,1000\r\n";
        else
            skin << "#STARTINPUT,1000\r\n#LOADSTART,0\r\n#SCENETIME,3600000\r\n#FADEOUT,500\r\n#CLOSE,1000\r\n";
        skin << "#IMAGE," << atlasScriptPath << "\r\n\r\n";

        skin << "$SE_OBJECT_NAME,Background\r\n$SE_OBJECT_ID,preset_background\r\n";
        skin << "#SRC_IMAGE,0,0,192,0,64,256,1,1,0,0,0,0,0\r\n";
        AppendDst(skin, "#DST_IMAGE", 0, 0, 0, width, height);

        if (IsPlayPresetType(type)) {
            const int bgaW = twoPlayers ? width * 32 / 100 : width - fieldWidth - margin * 3;
        const int bgaH = (std::max)(120, height * 48 / 100);
        const int bgaX = twoPlayers ? (width - bgaW) / 2 : margin * 2 + fieldWidth;
        skin << "$SE_OBJECT_NAME,BGA\r\n$SE_OBJECT_ID,preset_bga\r\n";
        AppendDst(skin, "#DST_BGA", 0, bgaX, fieldTop, bgaW, bgaH);

            int generatedLane = 0;
            for (int player = 0; player < playerCount; ++player) {
            const int fieldX = twoPlayers ? margin + player * (fieldWidth + gap) : margin;
            const int laneWidth = (std::max)(8, fieldWidth / lanesPerPlayer);
            const int usedWidth = laneWidth * lanesPerPlayer;
            for (int lane = 0; lane < lanesPerPlayer; ++lane, ++generatedLane) {
                // LR2 reserves 0/10 for the two scratches. PMS uses 1..9,
                // while DP/Battle uses a second bank beginning at 10.
                const int laneIndex = player * 10 + lane + (keysPerPlayer == 9 ? 1 : 0);
                const int laneX = fieldX + lane * laneWidth;
                const int noteSourceY = lane % 2 ? 16 : 0;
                skin << "$SE_OBJECT_NAME,Note P" << (player + 1) << " Lane " << lane << "\r\n";
                skin << "$SE_OBJECT_ID,preset_note_" << generatedLane << "\r\n";
                skin << "#SRC_NOTE," << laneIndex << ",0,0," << noteSourceY << ",16,16,1,1,0,0,0,0,0\r\n";
                skin << "#SRC_MINE," << laneIndex << ",0,0,32,16,16,1,1,0,0,0,0,0\r\n";
                skin << "#SRC_LN_BODY," << laneIndex << ",0,0,48,16,16,1,1,0\r\n";
                skin << "#SRC_LN_END," << laneIndex << ",0,16,48,16,16,1,1,0\r\n";
                skin << "#SRC_LN_START," << laneIndex << ",0,32,48,16,16,1,1,0\r\n";
                AppendDst(skin, "#DST_NOTE", laneIndex, laneX, judgeY, laneWidth, laneHeight);
            }

            skin << "$SE_OBJECT_NAME,Judge Line P" << (player + 1) << "\r\n";
            skin << "$SE_OBJECT_ID,preset_judgeline_" << player << "\r\n";
            skin << "#SRC_JUDGELINE," << player << ",0,0,64,192,8,1,1,0,0,0,0,0\r\n";
            AppendDst(skin, "#DST_JUDGELINE", player, fieldX, judgeY, usedWidth, laneHeight);

            skin << "$SE_OBJECT_NAME,Groove Gauge P" << (player + 1) << "\r\n";
            skin << "$SE_OBJECT_ID,preset_gauge_" << player << "\r\n";
            skin << "#SRC_GROOVEGAUGE," << player << ",0,0,120,192,16,48,1,0,0,4,0\r\n";
            AppendDst(skin, "#DST_GROOVEGAUGE", player, fieldX, height * 90 / 100,
                (std::max)(2, usedWidth / 50), (std::max)(10, height / 35));

            // LR2 documents FAST/SLOW counters 212/214 for 1P. Do not create
            // misleading duplicate P2 counters in DP/Battle presets.
            if (player == 0) {
                const int numberX = fieldX;
                const int numberY = height * 84 / 100;
                skin << "$SE_OBJECT_NAME,FAST\r\n$SE_OBJECT_ID,preset_fast\r\n";
                skin << "#SRC_NUMBER,0,0,0,104,160,16,10,1,0,0,212,0,4\r\n";
                AppendDst(skin, "#DST_NUMBER", 0, numberX, numberY,
                    (std::max)(8, fieldWidth / 24), (std::max)(12, height / 35));
                skin << "$SE_OBJECT_NAME,SLOW\r\n$SE_OBJECT_ID,preset_slow\r\n";
                skin << "#SRC_NUMBER,0,0,0,104,160,16,10,1,0,0,214,0,4\r\n";
                AppendDst(skin, "#DST_NUMBER", 0, numberX + fieldWidth / 2, numberY,
                    (std::max)(8, fieldWidth / 24), (std::max)(12, height / 35));
            }
            }
        }
        else if (type == 5) AppendSelectPreset(skin, width, height);
        else if (type == 6) AppendDecidePreset(skin, width, height);
        else if (type == 7) AppendResultPreset(skin, width, height);

        if (!WritePresetAtlasBmp(atlasPath, error)) return false;
        const std::filesystem::path tempSkinPath = skinPath.string() + ".tmp";
        std::ofstream output(tempSkinPath, std::ios::binary | std::ios::trunc);
        if (!output) { std::filesystem::remove(atlasPath); error = "Could not create the skin script."; return false; }
        const std::string contents = skin.str();
        output.write(contents.data(), (std::streamsize)contents.size());
        output.close();
        if (!output) {
            std::filesystem::remove(tempSkinPath); std::filesystem::remove(atlasPath);
            error = "Could not write the skin script."; return false;
        }
        std::filesystem::rename(tempSkinPath, skinPath, ec);
        if (ec) {
            std::filesystem::remove(tempSkinPath); std::filesystem::remove(atlasPath);
            error = "Could not finalize the skin script."; return false;
        }
        outputSkinPath = skinPath.string();
        outputAtlasPath = atlasPath.string();
        return true;
    }
}

int WORKSPACE::drawNewskin() {
    char windowTitle[260];
    snprintf(windowTitle, sizeof(windowTitle), "NewSkin##%d", num);
    if (ImGui::Begin(windowTitle, &wNewskin)) {
        static char skinTitle[128] = "New Skin";
        static char maker[128] = "SkinEditor";
        static char relativePath[MAX_PATH] = "NewSkin\\skin.lr2skin";
        static int resolution[2] = { 1280, 720 };
        static int selectedType = 0;
        static std::string createMessage;
        static bool createSucceeded = false;

        SEUI::SectionHeader("Create a new skin", "Choose a scene preset, then customize it in Object Editor.");
        ImGui::InputText("Title", skinTitle, IM_ARRAYSIZE(skinTitle));
        ImGui::InputText("Maker", maker, IM_ARRAYSIZE(maker));
        if (ImGui::BeginCombo("Scene / key mode", SKINTYPESTR[selectedType])) {
            const int presetTypes[] = { 0, 1, 2, 3, 4, 12, 13, 14, 5, 6, 7 };
            for (int type : presetTypes) {
                const bool selected = selectedType == type;
                if (ImGui::Selectable(SKINTYPESTR[type], selected)) selectedType = type;
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::InputInt2("Resolution", resolution);
        resolution[0] = (std::max)(64, (std::min)(7680, resolution[0]));
        resolution[1] = (std::max)(64, (std::min)(4320, resolution[1]));
        ImGui::TextUnformatted("LR2files\\Theme\\");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(340.0f);
        ImGui::InputText("##NewSkinPath", relativePath, IM_ARRAYSIZE(relativePath));
        ImGui::TextDisabled("Creates a scene starter script and preset.bmp. Existing files are never overwritten.");

        ImGui::Dummy(ImVec2(0.0f, 4.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.46f, 0.78f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.56f, 0.94f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.17f, 0.39f, 0.70f, 1.00f));
        const bool createRequested = ImGui::Button("Create and open");
        ImGui::PopStyleColor(3);
        if (createRequested) {
            std::string createdSkin, createdAtlas, error;
            createSucceeded = BuildInitialPreset(selectedType, resolution[0], resolution[1],
                skinTitle, maker, relativePath, createdSkin, createdAtlas, error);
            if (createSucceeded) {
                const std::string titleCp932 = Utf8ToCp932(skinTitle);
                const std::string makerCp932 = Utf8ToCp932(maker);
                meta.type = (SKINTYPE)selectedType;
                meta.targetX = resolution[0];
                meta.targetY = resolution[1];
                meta.title.assign(titleCp932.c_str());
                meta.maker.assign(makerCp932.c_str());
                meta.skinFile.assign(createdSkin.c_str());
                meta.thumbnail.assign(createdAtlas.c_str());
                strncpy_s(mainpath, createdSkin.c_str(), _TRUNCATE);
                snprintf(title, 260, "%s -%s", titleCp932.c_str(), SKINTYPESTR[selectedType]);
                loaded = LoadSkin(mainpath) == 0;
                if (loaded) {
                    newPath[0] = '\0';
                    createMessage = "Created and opened: " + createdSkin;
                    wNewskin = false;
                } else {
                    createSucceeded = false;
                    createMessage = "Files were created, but loading failed: " + createdSkin;
                }
            } else {
                createMessage = error;
            }
        }
        if (!createMessage.empty()) {
            ImGui::TextColored(createSucceeded ? ImVec4(0.35f, 0.85f, 0.40f, 1.0f) :
                ImVec4(1.0f, 0.30f, 0.30f, 1.0f), "%s", createMessage.c_str());
        }
        ImGui::End();
    }
    return 0;
}

int WORKSPACE::drawSimplePreview() {
    char title[260];
    snprintf(title, sizeof(title), "SimplePreview##%d", num);
    if (ImGui::Begin(title, &wSimplePreview, ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
        //ImDrawList* draw_list = ImGui::GetWindowDrawList();
        //
        //ImVec2 pb = ImGui::GetCursorScreenPos();
        //pb.x += 400;
        //pb.y += 400;
        ////ImVec2 bgSize = { skinSizeX,skinSizeY };
        //ImGui::Dummy(pb);
        //draw_list->AddRectFilled(pb, { skinSizeX+pb.x,skinSizeY+pb.y }, 0xFF000000, 0, ImDrawFlags_None);
        //
        //ImGui::SetCursorScreenPos(pb);

        //DST *dst = ((DST*)arr_DST.data);

        ////for (int i = 0; i < arr_DST.count; i++) {
        ////    ImGui::SetCursorScreenPos(pb);
        ////    ImVec2 PointTopLeft = { (float)dst[i].x, (float)dst[i].y };
        ////    ImVec2 PointBottomRight = { (float)dst[i].x + dst[i].w, (float)dst[i].y + dst[i].h };
        ////    //ImVec4 xywh = {PointTopLeft , PointBottomRight };
        ////    draw_list->AddRectFilled(PointTopLeft, PointBottomRight, (0xFF000000 | (0xFF0000 >> i)), 0, ImDrawFlags_None);

        ////    //drawSrc(((SRC*)arr_SRC.data)[dst[i].src].gr, dst[i].src, dst[i].x, dst[i].y);
        ////    ImGui::Dummy({ (float)dst[i].w, (float)dst[i].h });
        ////}

        //skinSizeX;
        //skinSizeY;
        ////zoom in zoom out
        //
        //


        //ImGui::End();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        ImGui::Text("dst animation");
        ImGui::SameLine(0, 0);
        ImGui::ColorEdit4("MyColor##3", (float*)&bgColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None);
        const ImVec2 pbs = ImGui::GetCursorScreenPos();

        const ImVec2 pb = ImGui::GetCursorPos();
        ImGui::SetCursorPos(pb);
        ImGui::Image(transBackground, { (float)skinSizeX, (float)skinSizeY }, { 0,0 }, { skinSizeX / (float)32.0, skinSizeY / (float)32.0 });
        const ImVec2 belowImage = ImGui::GetCursorPos();

        draw_list->AddRectFilled(pbs, { skinSizeX + pbs.x,skinSizeY + pbs.y }, ImGui::GetColorU32(bgColor), 0, ImDrawFlags_None);

        DST* dst = ((DST*)arr_DST.data);

        for (int i = 0; i < arr_DST.count; i++) {
            //ImVec2 pos = { pb.x + dst[i].x, pb.y + dst[i].y };

            if (dst[i].src != -1) {

                float dx, dy, dw, dh;
                float da, dr, dg, db;

                DST_ANIMATION& dstdFirst = ((DST_ANIMATION*)dst[i].arr_animation.data)[0];
                DST_ANIMATION& dstdLast = ((DST_ANIMATION*)dst[i].arr_animation.data)[dst->arr_animation.count - 1];

                int tStart = dstdFirst.time;
                int tEnd = dstdLast.time;
                int viewTime = (int)DstViewTime;
                dst[i].loop;
                int t = tEnd;
                int ani;

                if (tStart <= tEnd && tStart <= viewTime && (0 <= dst[i].loop || viewTime <= tEnd)) {
                    if (tStart == tEnd || dst[i].loop == tEnd) {
                        if (viewTime < t) {
                            t = viewTime;
                        }
                    }
                    else if (dst[i].loop < tEnd) {
                        t = viewTime;
                        if (tEnd < viewTime) {
                            //if (dst[i].loop == -1) continue; // only for SE
                            t = (int)(viewTime - dst[i].loop) % (tEnd - dst[i].loop) + dst[i].loop;
                        }
                    }
                    else {
                        t = 0;
                    }

                    ani = 0;
                    for (int j = 0; j < dst->arr_animation.count; j++) {
                        if (((DST_ANIMATION*)dst[i].arr_animation.data)[j].time <= t) {
                            ani = j;
                        }
                    }
                    DST_ANIMATION dstd1 = ((DST_ANIMATION*)dst[i].arr_animation.data)[ani];
                    if (t != dstd1.time && ani != dst->arr_animation.count - 1) {
                        DST_ANIMATION dstd2 = ((DST_ANIMATION*)dst[i].arr_animation.data)[ani + 1];
                        dx = ChangeValueByTime(dstd1.x, dstd2.x, dstd1.time, dstd2.time, t, dstdFirst.acc);
                        dy = ChangeValueByTime(dstd1.y, dstd2.y, dstd1.time, dstd2.time, t, dstdFirst.acc);
                        dw = ChangeValueByTime(dstd1.w, dstd2.w, dstd1.time, dstd2.time, t, dstdFirst.acc);
                        dh = ChangeValueByTime(dstd1.h, dstd2.h, dstd1.time, dstd2.time, t, dstdFirst.acc);

                        da = ChangeValueByTime(dstd1.a, dstd2.a, dstd1.time, dstd2.time, t, dstdFirst.acc);
                        dr = ChangeValueByTime(dstd1.r, dstd2.r, dstd1.time, dstd2.time, t, dstdFirst.acc);
                        dg = ChangeValueByTime(dstd1.g, dstd2.g, dstd1.time, dstd2.time, t, dstdFirst.acc);
                        db = ChangeValueByTime(dstd1.b, dstd2.b, dstd1.time, dstd2.time, t, dstdFirst.acc);
                    }
                    else {
                        dx = dstd1.x;
                        dy = dstd1.y;
                        dw = dstd1.w;
                        dh = dstd1.h;

                        da = dstd1.a;
                        dr = dstd1.r;
                        dg = dstd1.g;
                        db = dstd1.b;
                    }
                    ImVec2 pos = { pb.x + dx, pb.y + dy };
                    ImGui::SetCursorPos(pos);

                    drawSrc(((SRC*)arr_SRC.data)[dst[i].src].gr, dst[i].src, 0, 0, dw, dh, 1);

                }
            }

        }
        ImGui::End();
    }


    return 0;
}

int WORKSPACE::drawDstView() {

    char title[260];
    snprintf(title, sizeof(title), "dstView##%d", num);
    if (ImGui::Begin(title, &wDstView)) {
        if (arr_DST.count <= 0) {
            selected_dst = -1;
            dst_view_scroll_request = -1;
            ImGui::TextDisabled("No DST objects.");
            ImGui::End();
            return 0;
        }
        if (selected_dst < 0 || selected_dst >= arr_DST.count)
            selected_dst = 0;

        snprintf(title, sizeof(title), "dstList##%d", num);
        if (ImGui::BeginChild(title, { 250,-1 }, ImGuiChildFlags_ResizeX | ImGuiChildFlags_FrameStyle)) {
            for (int i = 0; i < arr_DST.count; i++) {
                ImGui::PushID(i);
                DST& dst = ((DST*)arr_DST.data)[i];

                char buf[260];
                sprintf(buf, "%03d:%03d(%s)", dst.src, dst.timer, timerName(dst.timer));
                if (ImGui::Selectable(buf, selected_dst == i)) {
                    selected_dst = i;
                    dst_view_scroll_request = -1;
                    SetTimeLapse(1, &g.timer1);

                    const std::vector<SEObjectInstance>& objects = objectEditorModel.Objects();
                    int objectModelIndex = -1;
                    for (int modelIndex = 0; modelIndex < (int)objects.size(); ++modelIndex) {
                        if (std::find(objects[modelIndex].rows.begin(), objects[modelIndex].rows.end(),
                            dst.declare) != objects[modelIndex].rows.end()) {
                            objectModelIndex = modelIndex;
                            break;
                        }
                    }
                    if (objectModelIndex >= 0) {
                        wObjectEditor = true;
                        selected_object_group = -1;
                        selected_user_object_group = -1;
                        SetObjectSelection(std::vector<int>(1, objectModelIndex),
                            objectModelIndex, objectModelIndex, true);
                        preview_object_dragging = false;
                        RefreshPreviewSelectionBounds();
                    }
                }
                if (dst_view_scroll_request == i)
                    ImGui::SetScrollHereY(0.5f);
                if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone) && ImGui::BeginTooltip()) {
                    if (dst.op1) ImGui::Text("%03d(%s)\n", dst.op1, dstName(dst.op1));
                    if (dst.op2) ImGui::Text("%03d(%s)\n", dst.op2, dstName(dst.op2));
                    if (dst.op3) ImGui::Text("%03d(%s)", dst.op3, dstName(dst.op3));
                    ImGui::EndTooltip();
                }
                ImGui::PopID();
            }
            dst_view_scroll_request = -1;
        }
        ImGui::EndChild();
        ImGui::SameLine();

        snprintf(title, sizeof(title), "dstAnimationView##%d", num);
        // Only the list pane owns the vertical splitter. Making this preview
        // child resizable caused ImGui to persist the tiny width it had during
        // the first docking frame. A non-resizable zero-width child always
        // consumes the full space left by the list, including on first open.
        if (ImGui::BeginChild(title, { 0,-1 },
            ImGuiChildFlags_FrameStyle,
            ImGuiWindowFlags_HorizontalScrollbar)) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            ImGui::Text("dst animation");
            ImGui::SameLine(0, 0);
            ImGui::ColorEdit4("MyColor##3", (float*)&bgColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None);
            auto fitDstViewZoom = [&]() {
                const ImVec2 available = ImGui::GetContentRegionAvail();
                const float fitX = skinSizeX > 0 ? (available.x - 20.0f) / skinSizeX : 1.0f;
                const float fitY = skinSizeY > 0 ? (available.y - 120.0f) / skinSizeY : 1.0f;
                float fit = fitX < fitY ? fitX : fitY;
                if (fit < 0.05f) fit = 0.05f;
                if (fit > 16.0f) fit = 16.0f;
                return fit;
            };
            if (!(DstViewZoom > 0.0f) || DstViewZoom > 16.0f)
                DstViewZoom = fitDstViewZoom();
            float dstZoomPercent = DstViewZoom * 100.0f;
            ImGui::SetNextItemWidth(220.0f);
            if (ImGui::SliderFloat("Zoom##dstViewZoom", &dstZoomPercent, 5.0f, 1600.0f,
                "%.0f%%", ImGuiSliderFlags_Logarithmic))
                DstViewZoom = dstZoomPercent / 100.0f;
            ImGui::SameLine();
            if (ImGui::Button("Fit##dstViewZoomFit")) DstViewZoom = fitDstViewZoom();
            ImGui::SameLine();
            if (ImGui::Button("100%##dstViewZoomReset")) DstViewZoom = 1.0f;

            const ImVec2 pbs = ImGui::GetCursorScreenPos();
            const ImVec2 oldDstCanvasSize(skinSizeX * DstViewZoom,
                skinSizeY * DstViewZoom);
            ApplyMouseCenteredWheelZoom(DstViewZoom, 0.05f, 16.0f,
                pbs, oldDstCanvasSize);
            const float dstScale = DstViewZoom;
            const float canvasWidth = skinSizeX * dstScale;
            const float canvasHeight = skinSizeY * dstScale;

            const ImVec2 pb = ImGui::GetCursorPos();
            ImGui::SetCursorPos(pb);
            const bool sharpDstView = BeginSharpMagnifiedCanvas(dstScale);
            ImGui::Image(transBackground, { canvasWidth, canvasHeight }, { 0,0 },
                { skinSizeX / (float)32.0, skinSizeY / (float)32.0 });
            const ImVec2 belowImage = ImGui::GetCursorPos();

            draw_list->AddRectFilled(pbs, { canvasWidth + pbs.x, canvasHeight + pbs.y },
                ImGui::GetColorU32(bgColor), 0, ImDrawFlags_None);

            DST* dst = ((DST*)arr_DST.data);

            for (int i = 0; i < arr_DST.count; i++) {
                //ImVec2 pos = { pb.x + dst[i].x, pb.y + dst[i].y };
                
                if (i == selected_dst) {
                    
                    if (dst[i].src >= 0 && dst[i].src < arr_SRC.count &&
                        dst[i].arr_animation.count > 0) {

                        float dx, dy, dw, dh;
                        float da, dr, dg, db;

                        DST_ANIMATION &dstdFirst = ((DST_ANIMATION*)dst[i].arr_animation.data)[0];
                        DST_ANIMATION &dstdLast = ((DST_ANIMATION*)dst[i].arr_animation.data)[dst[i].arr_animation.count-1];

                        int tStart = dstdFirst.time;
                        int tEnd = dstdLast.time;
                        int viewTime = (int)DstViewTime;
                        dst[i].loop;
                        int t = tEnd;
                        int ani;

                        if (tStart <= tEnd && tStart <= viewTime && (0 <= dst[i].loop || viewTime <= tEnd)) {
                            if (tStart == tEnd || dst[i].loop == tEnd) {
                                if (viewTime < t) {
                                    t = viewTime;
                                }
                            }
                            else if (dst[i].loop < tEnd) {
                                t = viewTime;
                                if (tEnd < viewTime) {
                                    //if (dst[i].loop == -1) continue; // only for SE
                                    t = (int)(viewTime - dst[i].loop) % (tEnd - dst[i].loop) + dst[i].loop;
                                }
                            }
                            else {
                                t = 0;
                            }

                            ani = 0;
                            for (int j = 0; j < dst[i].arr_animation.count; j++) {
                                if (((DST_ANIMATION*)dst[i].arr_animation.data)[j].time <= t) {
                                    ani = j;
                                }
                            }
                            DST_ANIMATION dstd1 = ((DST_ANIMATION*)dst[i].arr_animation.data)[ani];
                            if (t != dstd1.time && ani != dst[i].arr_animation.count-1) {
                                DST_ANIMATION dstd2 = ((DST_ANIMATION*)dst[i].arr_animation.data)[ani+1];
                                dx = ChangeValueByTime(dstd1.x, dstd2.x, dstd1.time, dstd2.time, t, dstdFirst.acc);
                                dy = ChangeValueByTime(dstd1.y, dstd2.y, dstd1.time, dstd2.time, t, dstdFirst.acc);
                                dw = ChangeValueByTime(dstd1.w, dstd2.w, dstd1.time, dstd2.time, t, dstdFirst.acc);
                                dh = ChangeValueByTime(dstd1.h, dstd2.h, dstd1.time, dstd2.time, t, dstdFirst.acc);

                                da = ChangeValueByTime(dstd1.a, dstd2.a, dstd1.time, dstd2.time, t, dstdFirst.acc);
                                dr = ChangeValueByTime(dstd1.r, dstd2.r, dstd1.time, dstd2.time, t, dstdFirst.acc);
                                dg = ChangeValueByTime(dstd1.g, dstd2.g, dstd1.time, dstd2.time, t, dstdFirst.acc);
                                db = ChangeValueByTime(dstd1.b, dstd2.b, dstd1.time, dstd2.time, t, dstdFirst.acc);
                            }
                            else {
                                dx = dstd1.x;
                                dy = dstd1.y;
                                dw = dstd1.w;
                                dh = dstd1.h;

                                da = dstd1.a;
                                dr = dstd1.r;
                                dg = dstd1.g;
                                db = dstd1.b;
                            }
                            ImVec2 pos = { pb.x + dx * dstScale, pb.y + dy * dstScale };
                            ImGui::SetCursorPos(pos);

                            printSrcImgEx(((SRC*)arr_SRC.data)[dst[i].src],
                                (int)std::round(dw * dstScale), (int)std::round(dh * dstScale), true);
                            
                        }
                    }
                     
                }
            }
            EndSharpMagnifiedCanvas(sharpDstView);

            //skinSizeX;
            //skinSizeY;
            //zoom in zoom out

            
            ImGui::SetCursorPos(belowImage);
            snprintf(title, sizeof(title), "dstAnimationViewTimer##%d", num);
            if (!isDstViewTimeStop) {
                DstViewTime = GetTimeLapse(1, &g.timer1);
            }
            ImGui::SliderFloat(title, &DstViewTime, 0, 240000, "%.0f", 0);// ImGuiSliderFlags_)
            if(ImGui::Button("PLAY")) {
                SetTimeLapse(1,&g.timer1);
                isDstViewTimeStop = 0;
            }
            ImGui::SameLine(0, 0);
            if (ImGui::Button("STOP")) {
                ResetTimeLapse(1,&g.timer1);
                isDstViewTimeStop = 1;
            }
            
            dst = &((DST*)arr_DST.data)[selected_dst];
            if (dst->op1) ImGui::Text("%03d(%s)\n", dst->op1, dstName(dst->op1));
            if (dst->op2) ImGui::Text("%03d(%s)\n", dst->op2, dstName(dst->op2));
            if (dst->op3) ImGui::Text("%03d(%s)", dst->op3, dstName(dst->op3));
            ImGui::Text("%s %dlines", dst->name.outstr(), dst->animation);
            for (int i = 0; i < dst->arr_animation.count; i++)
            {
                DST_ANIMATION dstd = ((DST_ANIMATION*)dst->arr_animation.data)[i];                
                ImGui::Text("%d %.0f %.0f %.0f %.0f", dstd.time, dstd.x, dstd.y, dstd.w, dstd.h);
                if (i == 0) ImGui::Text("%d", dst->loop);;
            }
            
        }
        ImGui::EndChild();
    }
    ImGui::End();
    return 0;
}

//HOW TO ADD FEATURE - STEP 2 : write function

int WORKSPACE::drawObjectManagerTest() {

    char title[260];
    snprintf(title, sizeof(title), "objectManagerTest##%d", num);

    int objcount = 0;
    if (ImGui::Begin(title, &wObjectManagerTest)) {
        char buf[260];

        for (int i = 0; i < g.skstruct.image.srcSize; i++) {
            SRCstruct& src = g.skstruct.image.src[i];
            DSTstruct& dst = g.skstruct.image.dst[i];
            sprintf(buf, "$OBJ_IMAGE%03d: %s %s %s %s %s %s %s %s", i, timerName(src.timer,1), dstName(src.op1,1), dstName(src.op2,1), dstName(src.op3,1), timerName(dst.timer,1), dstName(dst.op1,1), dstName(dst.op2,1), dstName(dst.op3,1));
            if (ImGui::Selectable(buf)) {
                preview_selected_obj = { dst.draw[dst.dstCount-1].x, dst.draw[dst.dstCount - 1].y, dst.draw[dst.dstCount - 1].w, dst.draw[dst.dstCount - 1].h};
                selectedObjectTest = objcount;
            }
            objcount++;
            //ImGui::Text("#SRC_IMAGE%03d: %s %s %s %s", i, timerName(src.timer), src.op1 ? dstName(src.op1) : "", src.op2 ? dstName(src.op2) : "", src.op3 ? dstName(src.op3) : "");
            //ImGui::Text("#DST_IMAGE%03d: %s %s %s %s", i, timerName(dst.timer), dst.op1 ? dstName(dst.op1) : "", dst.op2 ? dstName(dst.op2) : "", dst.op3 ? dstName(dst.op3) : "");
        }
        for (int i = 0; i < g.skstruct.otherObject[0].srcSize; i++) {
            SRCstruct& src = g.skstruct.otherObject[0].src[i];
            DSTstruct& dst = g.skstruct.otherObject[0].dst[i];
            sprintf(buf, "$OBJ_TEXT%03d: %s %s %s %s %s", i, textName(src.st), timerName(dst.timer, 1), dstName(dst.op1, 1), dstName(dst.op2, 1), dstName(dst.op3, 1));
            if (ImGui::Selectable(buf)) {
                preview_selected_obj = { dst.draw[dst.dstCount - 1].x, dst.draw[dst.dstCount - 1].y, dst.draw[dst.dstCount - 1].w, dst.draw[dst.dstCount - 1].h };
                selectedObjectTest = objcount;
            }
            objcount++;
        }
        for (int i = 0; i < g.skstruct.otherObject[1].srcSize; i++) {
            SRCstruct& src = g.skstruct.otherObject[1].src[i];
            DSTstruct& dst = g.skstruct.otherObject[1].dst[i];
            sprintf(buf, "$OBJ_BUTTON%03d: %s %s %s panel:%d %s %s %s %s", i, timerName(src.timer, 1), buttonName(src.op1),src.op2? "":"disabled", src.op3, timerName(dst.timer, 1), dstName(dst.op1, 1), dstName(dst.op2, 1), dstName(dst.op3, 1));
            if (ImGui::Selectable(buf)) {
                preview_selected_obj = { dst.draw[dst.dstCount - 1].x, dst.draw[dst.dstCount - 1].y, dst.draw[dst.dstCount - 1].w, dst.draw[dst.dstCount - 1].h };
                selectedObjectTest = objcount;
            }
        }
        for (int i = 0; i < g.skstruct.otherObject[2].srcSize; i++) {
            SRCstruct& src = g.skstruct.otherObject[2].src[i];
            DSTstruct& dst = g.skstruct.otherObject[2].dst[i];
            ImGui::Text("#SRC_SLIDER%03d: %s %s %s %s", i, timerName(src.timer), src.op1 ? dstName(src.op1) : "", src.op2 ? dstName(src.op2) : "", src.op3 ? dstName(src.op3) : "");
            ImGui::Text("#DST_SLIDER%03d: %s %s %s %s", i, timerName(dst.timer), dst.op1 ? dstName(dst.op1) : "", dst.op2 ? dstName(dst.op2) : "", dst.op3 ? dstName(dst.op3) : "");
        }
        for (int i = 0; i < g.skstruct.otherObject[3].srcSize; i++) {
            SRCstruct& src = g.skstruct.otherObject[3].src[i];
            DSTstruct& dst = g.skstruct.otherObject[3].dst[i];
            ImGui::Text("#SRC_ONMOUSET%03d: %s %s %s %s", i, timerName(src.timer), src.op1 ? dstName(src.op1) : "", src.op2 ? dstName(src.op2) : "", src.op3 ? dstName(src.op3) : "");
            ImGui::Text("#DST_ONMOUSE%03d: %s %s %s %s", i, timerName(dst.timer), dst.op1 ? dstName(dst.op1) : "", dst.op2 ? dstName(dst.op2) : "", dst.op3 ? dstName(dst.op3) : "");
        }
        for (int i = 0; i < g.skstruct.otherObject[4].srcSize; i++) {
            SRCstruct& src = g.skstruct.otherObject[4].src[i];
            DSTstruct& dst = g.skstruct.otherObject[4].dst[i];
            ImGui::Text("#SRC_BGA%03d: %s %s %s %s", i, timerName(src.timer), src.op1 ? dstName(src.op1) : "", src.op2 ? dstName(src.op2) : "", src.op3 ? dstName(src.op3) : "");
            ImGui::Text("#DST_BGA%03d: %s %s %s %s", i, timerName(dst.timer), dst.op1 ? dstName(dst.op1) : "", dst.op2 ? dstName(dst.op2) : "", dst.op3 ? dstName(dst.op3) : "");
        }
        for (int i = 0; i < g.skstruct.otherObject[5].srcSize; i++) {
            SRCstruct& src = g.skstruct.otherObject[5].src[i];
            DSTstruct& dst = g.skstruct.otherObject[5].dst[i];
            ImGui::Text("#SRC_BARGRAPH%03d: %s %s %s %s", i, timerName(src.timer), src.op1 ? dstName(src.op1) : "", src.op2 ? dstName(src.op2) : "", src.op3 ? dstName(src.op3) : "");
            ImGui::Text("#DST_BARGRAPH%03d: %s %s %s %s", i, timerName(dst.timer), dst.op1 ? dstName(dst.op1) : "", dst.op2 ? dstName(dst.op2) : "", dst.op3 ? dstName(dst.op3) : "");
        }
        for (int i = 0; i < g.skstruct.otherObject[6].srcSize; i++) {
            SRCstruct& src = g.skstruct.otherObject[6].src[i];
            DSTstruct& dst = g.skstruct.otherObject[6].dst[i];
            sprintf(buf, "$OBJ_NUMBER%03d: %s %s %s %s %s %s", i, timerName(src.timer,1), numberName(src.op1), timerName(dst.timer,1), dstName(dst.op1,1), dstName(dst.op2,1), dstName(dst.op3,1));
            if (ImGui::Selectable(buf)) {
                preview_selected_obj = { dst.draw[dst.dstCount - 1].x, dst.draw[dst.dstCount - 1].y, dst.draw[dst.dstCount - 1].w, dst.draw[dst.dstCount - 1].h };
            }
        }
        for (int i = 0; i < g.skstruct.otherObject[7].srcSize; i++) {
            SRCstruct& src = g.skstruct.otherObject[7].src[i];
            DSTstruct& dst = g.skstruct.otherObject[7].dst[i];
            ImGui::Text("#SRC_MASK%03d: %s %s %s %s", i, timerName(src.timer), src.op1 ? dstName(src.op1) : "", src.op2 ? dstName(src.op2) : "", src.op3 ? dstName(src.op3) : "");
            ImGui::Text("#DST_MASK%03d: %s %s %s %s", i, timerName(dst.timer), dst.op1 ? dstName(dst.op1) : "", dst.op2 ? dstName(dst.op2) : "", dst.op3 ? dstName(dst.op3) : "");
        }

        //select

        //
        if(true){
            SRCstruct& src = g.skstruct.src_MOUSECURSOR;
            DSTstruct& dst = g.skstruct.dst_MOUSECURSOR;
            ImGui::Text("#SRC_MOUSECURSOR: %s %s %s %s", timerName(src.timer), src.op1 ? dstName(src.op1) : "", src.op2 ? dstName(src.op2) : "", src.op3 ? dstName(src.op3) : "");
            ImGui::Text("#DST_MOUSECURSOR: %s %s %s %s", timerName(dst.timer), dst.op1 ? dstName(dst.op1) : "", dst.op2 ? dstName(dst.op2) : "", dst.op3 ? dstName(dst.op3) : "");
        }

        for(int i = 0 ; i < 20 ; i++){
            SRCstruct& src = g.skstruct.src_NOTE[i];
            ImGui::Text("#SRC_NOTE%02d: %s %s %s %s", i, timerName(src.timer), src.op1 ? dstName(src.op1) : "", src.op2 ? dstName(src.op2) : "", src.op3 ? dstName(src.op3) : "");
        }

    }
    if (ImGui::BeginPopupContextWindow()){
        ImGui::Text("selected : %d", selectedObjectTest);
        ImGui::Separator();
        ImGui::MenuItem("move");
        ImGui::MenuItem("delete");
        ImGui::EndPopup();
    }
    ImGui::End();
    return 0;
}

//TODO - iftree, *wildcardtree, insert, delete, group, dst thing
//group should have both if / endif
// new file to "skin wizard", which makes mockup and create texture template
int WORKSPACE::drawNewObject() {

    char title[260];
    snprintf(title, sizeof(title), "%s##%d",
        newCommandIncludeAll ? "New Command / Setting" : "New Object", num);
    const bool assetDropModal =
        newObjectAssetIndex >= 0 && newObjectAssetIndex < arr_IMG.count;
    bool windowContentsVisible = false;
    if (assetDropModal) {
        // A dropped Asset must not reuse the docked position of the ordinary
        // New Object tool window. Open it as a centered modal so the pending
        // drop is immediately visible even on a wide multi-pane layout.
        if (newObjectFocusRequest) {
            ImGui::OpenPopup(title);
            newObjectFocusRequest = false;
        }
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        const float modalWidth = (std::min)(620.0f, viewport->WorkSize.x * 0.85f);
        const float modalHeight = (std::min)(720.0f, viewport->WorkSize.y * 0.82f);
        ImGui::SetNextWindowPos(
            ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.5f,
                viewport->WorkPos.y + viewport->WorkSize.y * 0.5f),
            ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(modalWidth, modalHeight), ImGuiCond_Appearing);
        windowContentsVisible = ImGui::BeginPopupModal(title, &wNewObject,
            ImGuiWindowFlags_AlwaysVerticalScrollbar);
    }
    else {
        if (newObjectFocusRequest) {
            ImGui::SetNextWindowFocus();
            newObjectFocusRequest = false;
        }
        windowContentsVisible = ImGui::Begin(title, &wNewObject);
    }
    if (windowContentsVisible) {

        CSVbuf* csv = ((CSVbuf*)arr_CommandHelp.data);
        bool& isCsvInit = newObjectCsvInitialized;
        int& initializedCommand = newObjectInitializedCommand;
        auto isCreatableObjectCommand = [&](const char* command) {
            if (!command || !*command) return false;
            const std::vector<SEObjectGroupDef>& groups = objectEditorModel.Groups();
            for (const SEObjectGroupDef& group : groups)
                for (const std::string& candidate : group.commands)
                    if (candidate == command) return true;
            return false;
        };
        auto isSelectableCommand = [&](const char* command) {
            if (!command || command[0] != '#') return false;
            if (assetDropModal) return IsAssetBackedObjectCommand(command);
            return newCommandIncludeAll || isCreatableObjectCommand(command);
        };
        if (arr_CommandHelp.count <= 0) {
            ImGui::TextDisabled("Object command definitions are not loaded.");
            if (assetDropModal) ImGui::EndPopup();
            else ImGui::End();
            return 0;
        }
        if (selected_command < 0 || selected_command >= arr_CommandHelp.count ||
            !isSelectableCommand(csv[selected_command].str[0].outstr())) {
            selected_command = 0;
            while (selected_command < arr_CommandHelp.count &&
                !isSelectableCommand(csv[selected_command].str[0].outstr()))
                ++selected_command;
            if (selected_command >= arr_CommandHelp.count) selected_command = 0;
        }
        if (assetDropModal)
            ImGui::TextDisabled("Choose how the dropped crop will be used.");
        const char* commandComboLabel = assetDropModal ? "Object type" : "command";
        const char* commandPreview = assetDropModal
            ? AssetBackedObjectTypeName(csv[selected_command].str[0].outstr())
            : csv[selected_command].str[0].outstr();
        if (ImGui::BeginCombo(commandComboLabel, commandPreview, ImGuiComboFlags_None)) {
            for (int op = 0; op < arr_CommandHelp.count; op++) {
                if (!csv[op].str[0].body ||
                    !isSelectableCommand(csv[op].str[0].outstr())) continue;
                ImGui::PushID(op);
                const bool is_selected = (selected_command == op);
                char opname[64];
                const char* cmd = csv[op].str[0].outstr();
                const char* category = isCreatableObjectCommand(cmd) ? "Object" :
                    ((!strcmp(cmd, "#INFORMATION") || !strcmp(cmd, "#RESOLUTION") ||
                        !strcmp(cmd, "#CUSTOMOPTION") || !strcmp(cmd, "#CUSTOMFILE") ||
                        !strcmp(cmd, "#ENDOFHEADER")) ? "Header" :
                    ((!strcmp(cmd, "#IF") || !strcmp(cmd, "#ELSEIF") ||
                        !strcmp(cmd, "#ELSE") || !strcmp(cmd, "#ENDIF")) ? "Condition" : "Setting"));
                if (assetDropModal)
                    sprintf(opname, "%s  (%s)", AssetBackedObjectTypeName(cmd), cmd);
                else
                    sprintf(opname, "[%s] %s", category, csv[op].str[0].body);

                if (ImGui::Selectable(opname, is_selected)) {
                    selected_command = op;
                    isCsvInit = false;
                }
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();

                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        if (!isCsvInit || initializedCommand != selected_command) {
            SplitCSV("", &nCsv, ",");
            nCsv.str[0].assign(csv[selected_command].str[0].outstr());
            if (newObjectAssetIndex >= 0 && newObjectAssetIndex < arr_IMG.count &&
                IsAssetBackedObjectCommand(nCsv.str[0].outstr())) {
                IMG& asset = ((IMG*)arr_IMG.data)[newObjectAssetIndex];
                int assetDivX = 1;
                int assetDivY = 1;
                int assetCycle = 0;
                int assetTimer = 0;
                ResolveIMGDivision(newObjectAssetIndex, assetDivX, assetDivY,
                    assetCycle, assetTimer);
                InitializeAssetBackedSource(nCsv, nCsv.str[0].outstr(), asset,
                    assetDivX, assetDivY, assetCycle, assetTimer);
            }
            initializedCommand = selected_command;
            isCsvInit = true;
        }
        if (newObjectAssetIndex >= 0 && newObjectAssetIndex < arr_IMG.count) {
            IMG& asset = ((IMG*)arr_IMG.data)[newObjectAssetIndex];
            ImGui::SeparatorText("Dropped asset");
            ImGui::Text("Asset %03d  gr %d  ->  Preview (%d, %d)",
                newObjectAssetIndex, asset.gr, newObjectDropX, newObjectDropY);
            ImGui::SameLine();
            ImGui::TextDisabled("IF branch %d", newObjectIfgroup);
            int dropPosition[2] = { newObjectDropX, newObjectDropY };
            int dropSize[2] = { newObjectDropW, newObjectDropH };
            ImGui::SetNextItemWidth(220.0f);
            if (ImGui::InputInt2("DST x / y", dropPosition)) {
                newObjectDropX = dropPosition[0];
                newObjectDropY = dropPosition[1];
            }
            ImGui::SetNextItemWidth(220.0f);
            if (ImGui::InputInt2("DST w / h", dropSize)) {
                newObjectDropW = (std::max)(1, dropSize[0]);
                newObjectDropH = (std::max)(1, dropSize[1]);
            }
        }
        const bool selectedIsObjectCommand =
            isCreatableObjectCommand(csv[selected_command].str[0].outstr());
        if (selectedIsObjectCommand)
            CstrInputText("Name", &newObjectName, ImGuiInputTextFlags_None);
        for (int column = 1; column < 30; column++) {
            ImGui::PushID(column);
            if (csv[selected_command].str[column].isDiff("")) {
                const char* command = csv[selected_command].str[0].outstr();
                CSTR help = GetCommandHelp(command, column);
                help.trimWhiteSpace();
                const int current = nCsv.str[column].body ? atol(nCsv.str[column].outstr()) : 0;
                int selectedValue = current;
                const std::string labelUtf8 = Cp932ToUtf8(csv[selected_command].str[column].outstr());
                if (DrawCommandValueCombo(labelUtf8.c_str(), command,
                    help.body ? help.outstr() : "", current, selectedValue)) {
                    if (selectedValue != current) {
                        char valueText[32];
                        snprintf(valueText, sizeof(valueText), "%d", selectedValue);
                        nCsv.str[column].assign(valueText);
                    }
                } else {
                    CstrInputText(csv[selected_command].str[column], &nCsv.str[column], ImGuiInputTextFlags_None);
                }
            }
            ImGui::PopID();
        }


        if (ImGui::Button("OK")) {
            nCsv.str[0].assign(csv[selected_command].str[0].outstr());
            CSTR srcLine;
            CsvToCSTR(nCsv, srcLine);
            int insertAt = newObjectInsertPosition;
            const char* ownerFile = newObjectOwner.body && *newObjectOwner.outstr()
                ? newObjectOwner.outstr() : mainpath;
            const int ownerEndRow = FindOwnerFileEndRow(skinfileLines, ownerFile);
            if (insertAt < 0 || insertAt > ownerEndRow)
                insertAt = ownerEndRow;
            const char* selectedCommand = csv[selected_command].str[0].outstr();
            const bool isHeaderCommand = !strcmp(selectedCommand, "#INFORMATION") ||
                !strcmp(selectedCommand, "#RESOLUTION") || !strcmp(selectedCommand, "#CUSTOMOPTION") ||
                !strcmp(selectedCommand, "#CUSTOMFILE") || !strcmp(selectedCommand, "#ENDOFHEADER");
            if (newCommandIncludeAll && isHeaderCommand && strcmp(selectedCommand, "#ENDOFHEADER") != 0) {
                for (int row = 0; row < skinfileLines.count; ++row) {
                    SKINFILELINEREAD& candidate = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    if (candidate.csv.str[0].body && candidate.csv.str[0].isSame("#ENDOFHEADER")) {
                        insertAt = row;
                        break;
                    }
                }
            }

            int wrapFirst = skinfileLines.count;
            int wrapLast = -1;
            bool wrapSameCondition = true;
            if (newCommandIncludeAll && !strcmp(selectedCommand, "#IF")) {
                const std::vector<SEObjectInstance>& objects = objectEditorModel.Objects();
                for (int modelIndex : preview_selected_object_model_indices) {
                    if (modelIndex < 0 || modelIndex >= (int)objects.size()) continue;
                    const SEObjectInstance& object = objects[modelIndex];
                    if (object.ifgroup != newObjectIfgroup) wrapSameCondition = false;
                    for (int row : object.rows) {
                        if (row < wrapFirst) wrapFirst = row;
                        if (row > wrapLast) wrapLast = row;
                    }
                    if (!object.rows.empty() && object.rows.front() > 0) {
                        const int metaRow = object.rows.front() - 1;
                        SKINFILELINEREAD& meta = ((SKINFILELINEREAD*)skinfileLines.data)[metaRow];
                        const char* metaText = meta.line.body ? meta.line.outstr() : "";
                        for (int row = metaRow; row >= 0; --row) {
                            SKINFILELINEREAD& objectMeta = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                            const char* objectMetaText = objectMeta.line.body ? objectMeta.line.outstr() : "";
                            if (strncmp(objectMetaText, "$SE_OBJECT_ID,", 14) == 0 ||
                                strncmp(objectMetaText, "$SE_OBJECT_NAME,", 16) == 0) {
                                if (row < wrapFirst) wrapFirst = row;
                                continue;
                            }
                            break;
                        }
                    }
                }
                if (!wrapSameCondition) { wrapFirst = skinfileLines.count; wrapLast = -1; }
            }

            auto insertObjectLine = [&](const char* text) {
                InsertLine(insertAt);
                SKINFILELINEREAD& inserted = ((SKINFILELINEREAD*)skinfileLines.data)[insertAt];
                CSTR oldLine(inserted.line);
                EditLine(insertAt, oldLine, CSTR(text));
                inserted.filename.assign(ownerFile);
                inserted.ifgroup = newObjectIfgroup;
                ++insertAt;
            };
            const bool selectedIsObject = selectedIsObjectCommand;
            std::string createdObjectId;
            const bool wrapSelection = newCommandIncludeAll && !strcmp(selectedCommand, "#IF") &&
                wrapLast >= wrapFirst;
            if (wrapSelection) insertAt = wrapFirst;
            if (selectedIsObject) {
                char generatedId[64];
                snprintf(generatedId, sizeof(generatedId), "obj_%08llX_new", GetTickCount64());
                if (newObjectName.body && *newObjectName.outstr()) {
                    std::string nameLine = std::string("$SE_OBJECT_NAME,") + newObjectName.outstr();
                    insertObjectLine(nameLine.c_str());
                }
                std::string idLine = std::string("$SE_OBJECT_ID,") + generatedId;
                insertObjectLine(idLine.c_str());
                createdObjectId = generatedId;
            }
            insertObjectLine(srcLine.outstr());

            if (wrapSelection) {
                const int ifRow = wrapFirst;
                IFUNIT condition = {};
                for (int value = 0; value < 10; ++value)
                    condition.data[value] = nCsv.str[value + 1].body ? atol(nCsv.str[value + 1]) : 0;
                condition.parentID = newObjectIfgroup;
                condition.depth = newObjectIfgroup >= 0 && newObjectIfgroup < arr_ifunit.count
                    ? ((IFUNIT*)arr_ifunit.data)[newObjectIfgroup].depth + 1 : 1;
                condition.order = 0;
                const int newIfgroup = arr_ifunit.count;
                arr_ifunit.push_back(&condition);

                ((SKINFILELINEREAD*)skinfileLines.data)[ifRow].ifgroup = newIfgroup;
                ((SKINFILELINEREAD*)skinfileLines.data)[ifRow].isIfGroupHead = true;
                ((SKINFILELINEREAD*)skinfileLines.data)[ifRow].isGroupHead = true;
                // The opening line shifted every wrapped row down by one.
                for (int row = wrapFirst + 1; row <= wrapLast + 1; ++row)
                    ((SKINFILELINEREAD*)skinfileLines.data)[row].ifgroup = newIfgroup;
                insertAt = wrapLast + 2;
                insertObjectLine("#ENDIF");
                const int endifRow = insertAt - 1;
                ((SKINFILELINEREAD*)skinfileLines.data)[endifRow].ifgroup = newIfgroup;
                ((SKINFILELINEREAD*)skinfileLines.data)[endifRow].isIfGroupEnd = true;
                ((SKINFILELINEREAD*)skinfileLines.data)[endifRow].isGroupEnd = true;
            }
            else if (newCommandIncludeAll && !strcmp(selectedCommand, "#IF")) {
                insertObjectLine("#ENDIF");
            }

            // Most editable objects are an SRC declaration followed by one
            // matching DST frame. Create that frame automatically when the
            // command exists in skinHelper.txt.
            std::string dstCommand = selectedCommand;
            if (dstCommand.compare(0, 4, "#SRC") == 0) dstCommand.replace(0, 4, "#DST");
            for (int commandIndex = 0; selectedIsObject &&
                strncmp(selectedCommand, "#SRC", 4) == 0 && commandIndex < arr_CommandHelp.count; ++commandIndex) {
                if (!csv[commandIndex].str[0].body ||
                    dstCommand != csv[commandIndex].str[0].outstr()) continue;
                CSVbuf dstCsv;
                SplitCSV("", &dstCsv, ",");
                dstCsv.str[0].assign(dstCommand.c_str());
                if (newObjectAssetIndex >= 0 && newObjectAssetIndex < arr_IMG.count &&
                    IsAssetBackedObjectCommand(selectedCommand)) {
                    AssignCommandField(dstCsv, dstCommand.c_str(), "(NULL)", 0);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "time", 0);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "x", newObjectDropX);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "y", newObjectDropY);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "w", newObjectDropW);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "h", newObjectDropH);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "acc", 0);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "a", 255);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "r", 255);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "g", 255);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "b", 255);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "blend", 0);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "filter", 0);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "angle", 0);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "center", 0);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "loop", 0);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "timer", 0);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "op1", 0);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "op2", 0);
                    AssignCommandField(dstCsv, dstCommand.c_str(), "op3", 0);
                }
                // SRC w/h describe the complete animation sheet. A DST frame
                // displays one divided cell, so initialize its dimensions from
                // the final values entered in the New Object form. Keep the
                // existing fallback for LR2's special negative/unknown sizes.
                int sourceWidth = 0;
                int sourceHeight = 0;
                int sourceDivX = 0;
                int sourceDivY = 0;
                if (ReadCommandField(nCsv, selectedCommand, "w", sourceWidth) &&
                    ReadCommandField(nCsv, selectedCommand, "div_x", sourceDivX) &&
                    sourceWidth > 0 && sourceDivX > 0)
                    AssignCommandField(dstCsv, dstCommand.c_str(), "w",
                        sourceWidth / sourceDivX);
                if (ReadCommandField(nCsv, selectedCommand, "h", sourceHeight) &&
                    ReadCommandField(nCsv, selectedCommand, "div_y", sourceDivY) &&
                    sourceHeight > 0 && sourceDivY > 0)
                    AssignCommandField(dstCsv, dstCommand.c_str(), "h",
                        sourceHeight / sourceDivY);
                // Preserve an explicit object index when both commands use it.
                CSTR srcIndexHelp = GetCommandHelp(csv[selected_command].str[0].outstr(), 1);
                CSTR dstIndexHelp = GetCommandHelp(dstCommand.c_str(), 1);
                if (srcIndexHelp.isSame("index") && dstIndexHelp.isSame("index"))
                    dstCsv.str[1].assign(nCsv.str[1]);
                CSTR dstLine;
                CsvToCSTR(dstCsv, dstLine);
                insertObjectLine(dstLine.outstr());
                break;
            }

            newObjectInsertPosition = -1;
            RebuildObjectModel();
            if (!createdObjectId.empty()) {
                const std::vector<SEObjectInstance>& rebuilt =
                    objectEditorModel.Objects();
                for (int modelIndex = 0; modelIndex < (int)rebuilt.size(); ++modelIndex) {
                    if (rebuilt[modelIndex].editorId != createdObjectId) continue;
                    // A select request also clears Browser filters/search, so
                    // the newly created Object cannot remain hidden.
                    wObjectBrowser = true;
                    wObjectInspector = true;
                    SetObjectSelection(std::vector<int>(1, modelIndex),
                        modelIndex, modelIndex, true);
                    preview_object_dragging = false;
                    preview_selected_obj_valid = false;
                    preview_selected_obj_last_valid = false;
                    break;
                }
            }
            previewReloadPending = true;
            previewReloadRequestedAt = GetTickCount64();
            isCsvInit = false;
            newObjectName.assign("");
            newObjectOwner.assign("");
            newObjectAssetIndex = -1;
            if (assetDropModal) ImGui::CloseCurrentPopup();
            wNewObject = 0;
        }
        if (assetDropModal && wNewObject) {
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
                wNewObject = false;
            }
        }
    }
    if (assetDropModal) {
        if (windowContentsVisible) ImGui::EndPopup();
    }
    else {
        ImGui::End();
    }
    if (!wNewObject) {
        newObjectCsvInitialized = false;
        newObjectInitializedCommand = -1;
        newObjectAssetIndex = -1;
        newObjectFocusRequest = false;
        newObjectInsertPosition = -1;
        newObjectOwner.assign("");
    }
    return 0;
}

int WORKSPACE::drawObjectManager() {

    char title[260];
    snprintf(title, sizeof(title), "objectManager##%d", num);
    if (ImGui::Begin(title, &wObjectManager)) {

        snprintf(title, sizeof(title), "objList##%d", num);
        if (ImGui::BeginChild(title, { 250,-1 }, ImGuiChildFlags_ResizeX | ImGuiChildFlags_FrameStyle)) {
            for (int i = 0; i < arr_seobj.count; i++) {
                ImGui::PushID(i);
                SEOBJ& seobj = ((SEOBJ*)arr_seobj.data)[i];

                SRC& src = ((SRC*)arr_SRC.data)[((SEOBJ*)arr_seobj.data)[i].src];
                DST& dst = ((DST*)arr_DST.data)[((SEOBJ*)arr_seobj.data)[i].dst];
                

                SKINFILELINEREAD& readS = ((SKINFILELINEREAD*)skinfileLines.data)[src.declare];
                SKINFILELINEREAD& readD = ((SKINFILELINEREAD*)skinfileLines.data)[dst.declare];
                

                char buf[260];
                //if (readS.ifgroup == readD.ifgroup) {
                //    sprintf(buf, "%02d_", readS.ifgroup);
                //    for (int k = 0; k < ((IFUNIT*)arr_ifunit.data)[readS.ifgroup].depth; k++)
                //        sprintf(buf, "%s_", buf);
                //    sprintf(buf, "%s%03d(%s)", buf, seobj.ID, seobj.name.outstr());
                //}
                //else {
                //    sprintf(buf, "%03d(%s)", seobj.ID, seobj.name.outstr());
                //}

                //sprintf(buf, "%s %02d:%02d", buf, src.objType, src.objID);
                sprintf(buf, "%03d(%s)", i, seobj.name.outstr());

                if (ImGui::Selectable(buf, selected_obj == i)) {
                    selected_obj = i;
                }
                ImGui::PopID();
            }

            if (ImGui::BeginPopupContextWindow()) {
                ImGui::Text("selected : %03d", selected_obj);
                ImGui::Separator();
                if (ImGui::MenuItem("Delete")) {
                    arr_seobj.DeleteAt(selected_obj);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("New")) {
                    AssignRootFileOwner(skinfileLines, mainpath, newObjectOwner);
                    newObjectInsertPosition = FindOwnerFileEndRow(skinfileLines,
                        newObjectOwner.body ? newObjectOwner.outstr() : mainpath);
                    newObjectIfgroup = 0;
                    newCommandIncludeAll = false;
                    newObjectName.assign("");
                    wNewObject = 1;
                    
                    //SEOBJ* nObj = (SEOBJ*)arr_seobj.Get_newAt(selected_obj);

                    //nObj->name = "newObject";
                    //nObj->body.Alloc(sizeof(CSTR), 2);
                    //nObj->bodyCSV.Alloc(sizeof(CSVbuf), 2);


                    //SplitCSV(&nObj->body[0], &nObj->bodyCSV[0], ",");
                }
                ImGui::EndPopup();
            }
        }
        ImGui::EndChild();
        ImGui::SameLine();

        SEOBJ& seobj = ((SEOBJ*)arr_seobj.data)[selected_obj];
        SRC& src = ((SRC*)arr_SRC.data)[((SEOBJ*)arr_seobj.data)[selected_obj].src];
        DST& dst = ((DST*)arr_DST.data)[((SEOBJ*)arr_seobj.data)[selected_obj].dst];

        SKINFILELINEREAD* srcline = &((SKINFILELINEREAD*)skinfileLines.data)[src.declare];
        SKINFILELINEREAD* dstline = &((SKINFILELINEREAD*)skinfileLines.data)[dst.declare];
            
        snprintf(title, sizeof(title), "objProperty##%d", num);
        if (ImGui::BeginChild(title, { 400,-1 }, ImGuiChildFlags_ResizeX | ImGuiChildFlags_FrameStyle)) {
            ImGui::PushID(selected_obj);
            ImGui::Text(dst.name.outstr());
            
            for (int b = 0; b < seobj.body.count; b++) {
                ImGui::Text("%s", ((CSTR*)seobj.body.data)[b]);
            }
            /*ImGui::Text("%s", srcline->line);
            for (int d = 0; d < dst.arr_animation.count; d++) {
                ImGui::Text("%s", (dstline+d)->line);
                
            }*/
            if (seobj.type2 == 0) {
                if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None))
                {
                    if (ImGui::BeginTabItem("SRC"))
                    {
                        ImGui::SeparatorText("basic");
                        int k = FindIMG(((CSVbuf*)seobj.bodyCSV.data)[0].val[2]
                            , ((CSVbuf*)seobj.bodyCSV.data)[0].val[3]
                            , ((CSVbuf*)seobj.bodyCSV.data)[0].val[4]
                            , ((CSVbuf*)seobj.bodyCSV.data)[0].val[5]
                            , ((CSVbuf*)seobj.bodyCSV.data)[0].val[6]);

                        char buf[64];
                        sprintf(buf, "%d:%s", k, ((IMG*)arr_IMG.data)[k].name);
                        if (ImGui::BeginCombo("##IMGs", buf,ImGuiComboFlags_None)) {
                            for (int n = 0; n < arr_IMG.count; n++) {
                                ImGui::PushID(n);
                                sprintf(buf, "%d:%s", n, ((IMG*)arr_IMG.data)[n].name);
                                if (ImGui::Selectable(buf, n == k, ImGuiSelectableFlags_None, { 0,0 })) {
                                    ((IMG*)arr_IMG.data)[n].gr = ((CSVbuf*)seobj.bodyCSV.data)[0].val[2];
                                    ((IMG*)arr_IMG.data)[n].x = ((CSVbuf*)seobj.bodyCSV.data)[0].val[3];
                                    ((IMG*)arr_IMG.data)[n].y = ((CSVbuf*)seobj.bodyCSV.data)[0].val[4];
                                    ((IMG*)arr_IMG.data)[n].w = ((CSVbuf*)seobj.bodyCSV.data)[0].val[5];
                                    ((IMG*)arr_IMG.data)[n].h = ((CSVbuf*)seobj.bodyCSV.data)[0].val[6];

                                }
                                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone) && ImGui::BeginTooltip())
                                {
                                    IMG imgg = ((IMG*)arr_IMG.data)[n];
                                    int handle = imgg.gr;

                                    EnsureSRCGRTexture(handle);
                                    SRCGR& img = ((SRCGR*)arr_SRCGR.data)[handle];

                                    int iX = imgg.x;
                                    int iY = imgg.y;
                                    int iW = imgg.w == -1 ? img.sizeX - iX : imgg.w;
                                    int iH = imgg.h == -1 ? img.sizeY - iY : imgg.h;

                                    if (img.texture != NULL) {
                                        ImVec2 display_min = ImVec2(iX / (float)img.sizeX, iY / (float)img.sizeY);
                                        ImVec2 display_max = ImVec2((iX + iW) / (float)img.sizeX, (iY + iH) / (float)img.sizeY);
                                        ImVec2 display_size = ImVec2(iW, iH);

                                        ImGui::Image(img.texture, display_size, display_min, display_max);;
                                    }
                                    //printSrcImg(((SRC*)arr_SRC.data)[handle]);
                                    
                                    
                                    ImGui::EndTooltip();
                                }
                                ImGui::PopID();
                            }
                            ImGui::EndCombo();
                        }
                        CSVbuf* csv = (CSVbuf*)seobj.bodyCSV.data;
                        
                        ImGui::PushItemWidth(45);
                        /*ImGui::InputInt("gr", &csv[0].val[2], 0, 0, ImGuiInputTextFlags_None);
                        ImGui::InputInt("x", &csv[0].val[3], 0, 0, ImGuiInputTextFlags_None);
                        ImGui::InputInt("y", &csv[0].val[4], 0, 0, ImGuiInputTextFlags_None);
                        ImGui::InputInt("w", &csv[0].val[5], 0, 0, ImGuiInputTextFlags_None);
                        ImGui::InputInt("h", &csv[0].val[6], 0, 0, ImGuiInputTextFlags_None);*/
                        CstrInputText("gr", &csv[0].str[2], ImGuiInputTextFlags_CharsDecimal);
                        CstrInputText("x", &csv[0].str[3], ImGuiInputTextFlags_CharsDecimal);
                        CstrInputText("y", &csv[0].str[4], ImGuiInputTextFlags_CharsDecimal);
                        CstrInputText("w", &csv[0].str[5], ImGuiInputTextFlags_CharsDecimal);
                        CstrInputText("h", &csv[0].str[6], ImGuiInputTextFlags_CharsDecimal);
                        

                        ImGui::SeparatorText("animation");
                        /*ImGui::InputInt("div_x", &csv[0].val[7], 0, 0, ImGuiInputTextFlags_None);
                        ImGui::InputInt("div_y", &csv[0].val[8], 0, 0, ImGuiInputTextFlags_None);
                        ImGui::InputInt("cycle", &csv[0].val[9], 0, 0, ImGuiInputTextFlags_None);*/
                        CstrInputText("div_x", &csv[0].str[7], ImGuiInputTextFlags_CharsDecimal);
                        CstrInputText("div_y", &csv[0].str[8], ImGuiInputTextFlags_CharsDecimal);
                        CstrInputText("cycle", &csv[0].str[9], ImGuiInputTextFlags_CharsDecimal);
                        //ImGui::InputText("timer", csv[0].str[10], IM_COUNTOF(csv[0].str[10].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        if (ImGui::BeginCombo("timer",csv[0].str[10], ImGuiComboFlags_None)) {
                            for (int op = 0; op < 200; op++) {
                                ImGui::PushID(op);
                                const bool is_selected = (atol(csv[0].str[10]) == op);
                                char opname[64];
                                sprintf(opname, "%03d:%s", op, timerName(op));

                                if (ImGui::Selectable(opname, is_selected)) {
                                    csv[0].val[10] = op;
                                    ltoa(op, csv[0].str[10],10);
                                    //EditValue(n, column, op);
                                }
                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();

                                ImGui::PopID();
                            }
                            ImGui::EndCombo();
                        }

                        ImGui::PopItemWidth();


                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("DST"))
                    {
                        CSVbuf* csv = (CSVbuf*)seobj.bodyCSV.data;
                        ImGui::PushItemWidth(45);
                        ImGui::SeparatorText("basic");
                        //ImGui::InputText("op1", csv[1].str[18], IM_COUNTOF(csv[1].str[18].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        if (ImGui::BeginCombo("op1", csv[1].str[18], ImGuiComboFlags_None)) {
                            for (int op = 0; op < 200; op++) {
                                ImGui::PushID(op);
                                const bool is_selected = (atol(csv[1].str[18]) == op);
                                char opname[64];
                                sprintf(opname, "%03d:%s", op, dstName(op));

                                if (ImGui::Selectable(opname, is_selected)) {
                                    csv[1].val[18] = op;
                                    ltoa(op, csv[1].str[18], 10);
                                }
                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();

                                ImGui::PopID();
                            }
                            ImGui::EndCombo();
                        }
                        //ImGui::InputText("op2", csv[1].str[19], IM_COUNTOF(csv[1].str[19].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        if (ImGui::BeginCombo("op2", csv[1].str[19], ImGuiComboFlags_None)) {
                            for (int op = 0; op < 200; op++) {
                                ImGui::PushID(op);
                                const bool is_selected = (atol(csv[1].str[19]) == op);
                                char opname[64];
                                sprintf(opname, "%03d:%s", op, dstName(op));

                                if (ImGui::Selectable(opname, is_selected)) {
                                    csv[1].val[19] = op;
                                    ltoa(op, csv[1].str[19], 10);
                                }
                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();

                                ImGui::PopID();
                            }
                            ImGui::EndCombo();
                        }
                        //ImGui::InputText("op3", csv[1].str[20], IM_COUNTOF(csv[1].str[20].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        if (ImGui::BeginCombo("op3", csv[1].str[20], ImGuiComboFlags_None)) {
                            for (int op = 0; op < 200; op++) {
                                ImGui::PushID(op);
                                const bool is_selected = (atol(csv[1].str[20]) == op);
                                char opname[64];
                                sprintf(opname, "%03d:%s", op, dstName(op));

                                if (ImGui::Selectable(opname, is_selected)) {
                                    csv[1].val[20] = op;
                                    ltoa(op, csv[1].str[20], 10);
                                }
                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();

                                ImGui::PopID();
                            }
                            ImGui::EndCombo();
                        }
                        //ImGui::InputText("timer", csv[1].str[17], IM_COUNTOF(csv[1].str[17].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        if (ImGui::BeginCombo("timer", csv[1].str[17], ImGuiComboFlags_None)) {
                            for (int op = 0; op < 200; op++) {
                                ImGui::PushID(op);
                                const bool is_selected = (atol(csv[1].str[17]) == op);
                                char opname[64];
                                sprintf(opname, "%03d:%s", op, timerName(op));

                                if (ImGui::Selectable(opname, is_selected)) {
                                    csv[1].val[17] = op;
                                    ltoa(op, csv[1].str[17], 10);
                                }
                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();

                                ImGui::PopID();
                            }
                            ImGui::EndCombo();
                        }
                        //ImGui::InputText("loop", csv[1].str[16], IM_COUNTOF(csv[1].str[16].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        //ImGui::InputInt("loop", &csv[1].val[16],0,0);
                        CstrInputText("loop", &csv[1].str[16], ImGuiInputTextFlags_CharsDecimal);

                        ImGui::SeparatorText("animation");
                        if(ImGui::Button("+")) {
                            CSVbuf* csvN = (CSVbuf*)seobj.bodyCSV.Get_new();
                            CSTR* bodyN = (CSTR*)seobj.body.Get_new();
                            bodyN->assign(((CSTR*)seobj.body.data)[seobj.body.count - 2]);
                            memset(csvN, 0, sizeof(CSVbuf));
                            for (int i = 0; i < 30; i++) {
                                csvN->str[i].assign("");
                            }
                            SplitCSV(bodyN->body, csvN, ",");
                            CSVatol(csvN);
                            CSVltoa (csvN,10);
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("-")) {
                            if (seobj.bodyCSV.count > 2) {
                                seobj.bodyCSV.DeleteAt(seobj.bodyCSV.count-1);
                                seobj.body.DeleteAt(seobj.bodyCSV.count - 1);
                            }
                        }
                        ImGui::BeginGroup();
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("time");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("x");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("y");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("w");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("h");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("acc");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("a");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("r");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("g");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("b");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("blend");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("filter");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("angle");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("center");
                        ImGui::EndGroup();
                        for (int i = 1; i < seobj.bodyCSV.count; i++) {
                            ImGui::SameLine();

                            ImGui::PushID(i);
                            ImGui::BeginGroup();
                            ImGui::InputInt("##time", &csv[i].val[2],0,0,ImGuiInputTextFlags_None);
                            ImGui::InputInt("##x", &csv[i].val[3], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##y", &csv[i].val[4], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##w", &csv[i].val[5], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##h", &csv[i].val[6], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##acc", &csv[i].val[7], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##a", &csv[i].val[8], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##r", &csv[i].val[9], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##g", &csv[i].val[10], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##b", &csv[i].val[11], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##blend", &csv[i].val[12], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##filter", &csv[i].val[13], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##angle", &csv[i].val[14], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##center", &csv[i].val[15], 0, 0, ImGuiInputTextFlags_None);
                            //CSVltoa(&csv[i],10);
                            ImGui::EndGroup();
                            ImGui::PopID();
                        }
                        ImGui::PopItemWidth();
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            }
            else if (seobj.type2 == -1) {


            }

            ImGui::PopID();
        }
        ImGui::EndChild();



    }
    ImGui::End();

    return 0;
}

int WORKSPACE::drawProperty() {

    char title[260];
    snprintf(title, sizeof(title), "objectProperty##%d", num);
    if (ImGui::Begin(title, &wProperty)) {
        selectedObjectTest;

    }
    ImGui::End();
    return 0;
}


int WORKSPACE::drawOpList() {

    char title[260];
    snprintf(title, sizeof(title), "OpList##%d", num);
    if (ImGui::Begin(title, &wOpList)) {
        //init ops
        for (int i = 0; i < 1000; i++) {
            op[i] = GetOptionFlag_dst(&g, i);
        }
        for (int i = 0; i < 1000; i++) {
            ImGui::PushID(i);
            ImGui::Text("%03d", i);
            ImGui::SameLine();
            ImGui::Checkbox(dstName(i), &op[i]);
            ImGui::PopID();
        }
    }
    ImGui::End();
    return 0;
}

int WORKSPACE::drawHistory() {


    ImGui::PushID(num);
    if (ImGui::Begin("History", &wHistory)) {

        int item_selected_idx = 0;
    
        for (int n = 0; n < arr_history.count; n++){
            ImGui::PushID(n);
            const bool is_selected = (item_selected_idx == n);
            char itemname[260];
            
            HISTORY& hs = ((HISTORY*)arr_history.data)[n];
            sprintf(itemname, "%d %d\n%s \n%s", hs.op, hs.target, hs.older.line.outstr(), hs.newer.line.outstr());
            if (ImGui::Selectable(itemname, is_selected)) {
                item_selected_idx = n;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();

            ImGui::PopID();
        }

    }
    ImGui::End();
    ImGui::PopID();

    return 0;
}



// 
// [file ---> (line, csv) ---> LR2]
// file ---> line, csv ---> current SE
// csv -> line -> file

// file -> csv -> file : this is textedit
// we need objedit
// csv -> obj(multi line) -> csv


//utf-8 shift-jis problem




std::vector<std::unique_ptr<WORKSPACE> > workspaceList;
// Data-driven Object Editor.  The schema comes from skinObjGroup.txt and
// argument names from skinHelper.txt.  The editor works directly on
// skinfileLines so the existing CSV/table/save path remains authoritative.
int WORKSPACE::drawObjectEditor() {
    char browserTitle[128];
    char inspectorTitle[128];
    snprintf(browserTitle, sizeof(browserTitle), "Object Browser##%d", num);
    snprintf(inspectorTitle, sizeof(inspectorTitle), "Object Inspector##%d", num);

    if (object_editor_select_request >= 0 && wObjectBrowser)
        ImGui::SetNextWindowFocus();
    const bool browserWasBegun = wObjectBrowser;
    const bool drawBrowser = browserWasBegun ? ImGui::Begin(browserTitle, &wObjectBrowser) : false;

    auto drawInspectorMessage = [&](const char* message) {
        if (!wObjectInspector) return;
        if (ImGui::Begin(inspectorTitle, &wObjectInspector)) ImGui::TextUnformatted(message);
        ImGui::End();
    };

    if (objectEditorModel.Groups().empty()) {
        if (drawBrowser) ImGui::Text("skinObjGroup.txt not loaded.");
        if (browserWasBegun) ImGui::End();
        drawInspectorMessage("skinObjGroup.txt not loaded.");
        return 0;
    }
    if (!loaded || skinfileLines.count <= 0) {
        if (drawBrowser) ImGui::Text("No skin loaded.");
        if (browserWasBegun) ImGui::End();
        drawInspectorMessage("No skin loaded.");
        return 0;
    }

    if (objectEditorLastLineCount != skinfileLines.count) {
        RebuildObjectModel();
        selected_object_editor = 0;
    }

    const std::vector<SEObjectGroupDef>& groups = objectEditorModel.Groups();
    if (selected_object_group >= (int)groups.size()) selected_object_group = -1;

    auto formatConditionHeader = [&](SKINFILELINEREAD& row) -> std::string {
        std::string result = row.csv.str[0].body ? row.csv.str[0].outstr() : "#IF";
        const char* command = row.csv.str[0].body ? row.csv.str[0].outstr() : "";
        if (strcmp(command, "#IF") != 0 && strcmp(command, "#ELSEIF") != 0) return result;
        for (int col = 1; col < 30; ++col) {
            if (!row.csv.str[col].body || row.csv.str[col].length() == 0) continue;
            const int op = row.csv.val[col];
            const int optionId = op < 0 ? -op : op;
            const char* optionName = dstName(optionId);
            char option[160];
            if (op < 0)
                snprintf(option, sizeof(option), " !%03d:%s", optionId, optionName ? optionName : "");
            else
                snprintf(option, sizeof(option), " %03d:%s", optionId, optionName ? optionName : "");
            result += option;
        }
        return result;
    };

    const int requestedObjectModel = object_editor_select_request;
    if (requestedObjectModel >= 0) {
        // A Preview hit must remain reachable even when the Object Editor was
        // showing a type/user-group/search filter that excludes it.
        selected_object_group = -1;
        selected_user_object_group = -1;
        objectBrowserActiveOnly = false;
        objectSearch[0] = '\0';
    }
    // Object Browser: filters on top, condition/object list below.
    if (drawBrowser) {
        const float filterPanelHeight = ImGui::GetFrameHeightWithSpacing() * 4.0f +
            ImGui::GetStyle().WindowPadding.y * 2.0f + 18.0f;
        const bool drawFilters = ImGui::BeginChild("ObjectFilters",
            ImVec2(0, filterPanelHeight), true,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        if (drawFilters) {
        ImGui::TextDisabled("Type");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        const char* typePreview = selected_user_object_group < 0 && selected_object_group >= 0
            ? groups[selected_object_group].name.c_str() : "All Objects";
        if (ImGui::BeginCombo("##ObjectTypeFilter", typePreview)) {
            if (ImGui::Selectable("All Objects", selected_user_object_group < 0 && selected_object_group < 0)) {
                selected_object_group = -1;
                selected_user_object_group = -1;
                selected_object_editor = 0;
            }
            for (int gidx = 0; gidx < (int)groups.size(); ++gidx) {
                std::vector<int> objs = objectEditorModel.ObjectsForGroup(gidx);
                if (objs.empty()) continue;
                char label[128];
                snprintf(label, sizeof(label), "%s (%d)", groups[gidx].name.c_str(), (int)objs.size());
                if (ImGui::Selectable(label, selected_user_object_group < 0 && selected_object_group == gidx)) {
                    selected_object_group = gidx;
                    selected_user_object_group = -1;
                    selected_object_editor = 0;
                }
            }
            ImGui::EndCombo();
        }

        const std::vector<SEUserObjectGroup>& userGroups = objectEditorModel.UserGroups();
        ImGui::TextDisabled("Group");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        const std::string groupPreviewUtf8 = selected_user_object_group >= 0 &&
            selected_user_object_group < (int)userGroups.size()
            ? Cp932ToUtf8(userGroups[selected_user_object_group].name.c_str()) : "No My Group";
        if (ImGui::BeginCombo("##ObjectUserGroupFilter", groupPreviewUtf8.c_str())) {
            if (ImGui::Selectable("No My Group", selected_user_object_group < 0))
                selected_user_object_group = -1;
            for (int gidx = 0; gidx < (int)userGroups.size(); ++gidx) {
                std::vector<int> members = objectEditorModel.ObjectsForUserGroup(gidx);
                char label[160];
                const std::string nameUtf8 = Cp932ToUtf8(userGroups[gidx].name.c_str());
                snprintf(label, sizeof(label), "%s (%d)##user%d", nameUtf8.c_str(),
                    (int)members.size(), gidx);
                if (ImGui::Selectable(label, selected_user_object_group == gidx)) {
                    selected_user_object_group = gidx;
                    selected_object_editor = 0;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputTextWithHint("##ObjectSearch", "Search objects...", objectSearch, sizeof(objectSearch));
        ImGui::Checkbox("Active objects only", &objectBrowserActiveOnly);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Show objects whose IF branch and DST options are currently enabled.");

        if (requestCreateGroupPopup) {
            ImGui::OpenPopup("Create Object Group");
            requestCreateGroupPopup = false;
        }
        if (ImGui::BeginPopupModal("Create Object Group", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputText("Group name", newObjectGroupName, sizeof(newObjectGroupName));
            if (ImGui::Button("Create")) {
                struct PendingId { int row; int sourceNum; std::string owner; std::string id; };
                std::vector<PendingId> pendingIds;
                std::vector<std::string> memberIds;
                const std::vector<SEObjectInstance>& allObjects = objectEditorModel.Objects();
                const unsigned long long stamp = GetTickCount64();
                for (int modelIndex : preview_selected_object_model_indices) {
                    if (modelIndex < 0 || modelIndex >= (int)allObjects.size()) continue;
                    const SEObjectInstance& object = allObjects[modelIndex];
                    std::string id = object.editorId;
                    if (id.empty() && !object.rows.empty()) {
                        char generated[64];
                        snprintf(generated, sizeof(generated), "obj_%08llX_%04d", stamp, modelIndex);
                        id = generated;
                        const int firstRow = object.rows.front();
                        SKINFILELINEREAD& source = ((SKINFILELINEREAD*)skinfileLines.data)[firstRow];
                        PendingId pending;
                        pending.row = firstRow;
                        pending.sourceNum = source.num;
                        pending.owner = source.filename.body ? source.filename.outstr() : mainpath;
                        pending.id = id;
                        pendingIds.push_back(pending);
                    }
                    if (!id.empty()) memberIds.push_back(id);
                }

                std::sort(pendingIds.begin(), pendingIds.end(), [](const PendingId& a, const PendingId& b) {
                    return a.row > b.row;
                });
                auto insertMetaLine = [&](int position, const char* text, const char* owner, int sourceNum) {
                    InsertLine(position);
                    SKINFILELINEREAD& inserted = ((SKINFILELINEREAD*)skinfileLines.data)[position];
                    CSTR oldLine(inserted.line);
                    EditLine(position, oldLine, CSTR(text));
                    inserted.filename.assign(owner && *owner ? owner : mainpath);
                    inserted.num = sourceNum;
                    inserted.numTotal = position;
                };
                for (const PendingId& pending : pendingIds) {
                    std::string line = "$SE_OBJECT_ID," + pending.id;
                    insertMetaLine(pending.row, line.c_str(), pending.owner.c_str(), pending.sourceNum);
                }

                const int groupNumber = (int)userGroups.size();
                std::string begin = std::string("$SE_GROUP_BEGIN,") +
                    (newObjectGroupName[0] ? newObjectGroupName : "New Group");
                insertMetaLine(skinfileLines.count, begin.c_str(), mainpath, skinfileLines.count);
                for (const std::string& id : memberIds) {
                    std::string member = "$SE_GROUP_MEMBER," + id;
                    insertMetaLine(skinfileLines.count, member.c_str(), mainpath, skinfileLines.count);
                }
                insertMetaLine(skinfileLines.count, "$SE_GROUP_END", mainpath, skinfileLines.count);

                RebuildObjectModel();
                selected_user_object_group = groupNumber;
                selected_object_editor = 0;
                preview_selected_object_model_indices.clear();
                const std::vector<SEObjectInstance>& rebuiltObjects = objectEditorModel.Objects();
                for (int objectIndex = 0; objectIndex < (int)rebuiltObjects.size(); ++objectIndex) {
                    if (std::find(memberIds.begin(), memberIds.end(), rebuiltObjects[objectIndex].editorId) != memberIds.end())
                        preview_selected_object_model_indices.push_back(objectIndex);
                }
                preview_selected_object_model_index = preview_selected_object_model_indices.empty()
                    ? -1 : preview_selected_object_model_indices.front();
                SetObjectSelection(preview_selected_object_model_indices,
                    preview_selected_object_model_index,
                    preview_selected_object_model_index, false);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        }
        ImGui::EndChild();
    }

    std::vector<int> groupObjects;
    if (selected_user_object_group >= 0)
        groupObjects = objectEditorModel.ObjectsForUserGroup(selected_user_object_group);
    else if (selected_object_group >= 0)
        groupObjects = objectEditorModel.ObjectsForGroup(selected_object_group);
    else {
        const std::vector<SEObjectInstance>& allObjects = objectEditorModel.Objects();
        for (int i = 0; i < (int)allObjects.size(); ++i) groupObjects.push_back(i);
    }

    auto syncDstSelectionForObject = [&](int objectModelIndex, bool focusDstView) {
        const std::vector<SEObjectInstance>& allObjects = objectEditorModel.Objects();
        if (objectModelIndex < 0 || objectModelIndex >= (int)allObjects.size()) return;
        const SEObjectInstance& object = allObjects[objectModelIndex];

        int firstDst = -1;
        int enabledDst = -1;
        bool currentDstBelongs = false;
        for (int dstIndex = 0; dstIndex < arr_DST.count; ++dstIndex) {
            DST& candidate = ((DST*)arr_DST.data)[dstIndex];
            if (std::find(object.rows.begin(), object.rows.end(), candidate.declare) == object.rows.end())
                continue;
            if (firstDst < 0) firstDst = dstIndex;
            if (dstIndex == selected_dst) currentDstBelongs = true;
            if (enabledDst < 0 && GetOptionFlag_dst(&g, candidate.op1) &&
                GetOptionFlag_dst(&g, candidate.op2) && GetOptionFlag_dst(&g, candidate.op3))
                enabledDst = dstIndex;
        }
        if (!currentDstBelongs) {
            const int matchingDst = enabledDst >= 0 ? enabledDst : firstDst;
            if (matchingDst >= 0) {
                selected_dst = matchingDst;
                dst_view_scroll_request = matchingDst;
                SetTimeLapse(1, &g.timer1);
            }
        } else {
            dst_view_scroll_request = selected_dst;
        }
        if (focusDstView && wDstView) {
            char dstWindowTitle[64];
            snprintf(dstWindowTitle, sizeof(dstWindowTitle), "dstView##%d", num);
            ImGui::SetWindowFocus(dstWindowTitle);
        }
    };

    if (requestedObjectModel >= 0) {
        const std::vector<int>::const_iterator requestedIt = std::find(
            groupObjects.begin(), groupObjects.end(), requestedObjectModel);
        if (requestedIt != groupObjects.end())
            selected_object_editor = (int)(requestedIt - groupObjects.begin());
        syncDstSelectionForObject(requestedObjectModel, false);
        object_editor_select_request = -1;
    }

    if (objectSearch[0]) {
        auto containsText = [](const char* text, const char* query) {
            if (!text || !query || !*query) return true;
            const size_t queryLength = strlen(query);
            for (const char* cursor = text; *cursor; ++cursor)
                if (_strnicmp(cursor, query, queryLength) == 0) return true;
            return false;
        };
        const std::vector<SEObjectInstance>& allObjects = objectEditorModel.Objects();
        groupObjects.erase(std::remove_if(groupObjects.begin(), groupObjects.end(), [&](int objectIndex) {
            if (objectIndex < 0 || objectIndex >= (int)allObjects.size()) return true;
            const SEObjectInstance& object = allObjects[objectIndex];
            if (containsText(object.name.c_str(), objectSearch) ||
                containsText(object.editorId.c_str(), objectSearch)) return false;
            const SEObjectGroupDef* def = objectEditorModel.Group(object.group);
            if (def && containsText(def->name.c_str(), objectSearch)) return false;
            for (int row : object.rows) {
                if (row < 0 || row >= skinfileLines.count) continue;
                SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                if (line.line.body && containsText(line.line.outstr(), objectSearch)) return false;
            }
            return true;
        }), groupObjects.end());
    }
    int contextCreateModel = -1;
    int contextDeleteModel = -1;

    // Middle: control-flow is metadata, never an Object command.
    // Sibling #IF/#ELSEIF/#ELSE branches are displayed together.
    if (drawBrowser) {
        const bool drawObjectList = ImGui::BeginChild("ObjectList", ImVec2(0, 0), true);
        if (drawObjectList) {
        struct BranchBlock {
            int ifgroup;
            int order;
            std::string label;
            std::vector<int> localObjectIndices;
        };
        struct ConditionBlock {
            int rootIfgroup;
            int depth;
            std::string label;
            std::vector<BranchBlock> branches;
        };
        std::vector<ConditionBlock> conditions;

        auto getHeader = [&](int ifgroup, std::string& label, int& order) -> bool {
            label = "";
            order = 0;
            if (ifgroup <= 0 || ifgroup >= arr_ifunit.count) return false;
            IFUNIT& unit = ((IFUNIT*)arr_ifunit.data)[ifgroup];
            order = unit.order;
            for (int rno = 0; rno < skinfileLines.count; ++rno) {
                SKINFILELINEREAD& r = ((SKINFILELINEREAD*)skinfileLines.data)[rno];
                if (r.ifgroup != ifgroup || !r.isIfGroupHead || !r.csv.str[0].body) continue;
                label = formatConditionHeader(r);
                return true;
            }
            return false;
        };

        auto getRootIfgroup = [&](int ifgroup) -> int {
            if (ifgroup <= 0 || ifgroup >= arr_ifunit.count) return 0;
            IFUNIT& u = ((IFUNIT*)arr_ifunit.data)[ifgroup];
            if (u.order == 0) return ifgroup;
            int root = ifgroup;
            for (int j = ifgroup - 1; j > 0; --j) {
                IFUNIT& c = ((IFUNIT*)arr_ifunit.data)[j];
                if (c.depth == u.depth && c.parentID == u.parentID && c.order == 0) {
                    root = j;
                    break;
                }
            }
            return root;
        };

        // Cache the exact active branch state. A branch is active only when
        // its parent is active, its own IF/ELSEIF matches, and no earlier
        // sibling branch has already matched.
        const unsigned long long statusNow = GetTickCount64();
        const bool refreshBranchStatus =
            objectStatusCacheLineCount != skinfileLines.count ||
            objectStatusCacheIfCount != arr_ifunit.count ||
            statusNow - objectStatusCacheAt >= 200;
        if (refreshBranchStatus) {
            std::vector<bool> branchMatches(arr_ifunit.count, true);
            objectBranchActive.assign(arr_ifunit.count, false);
            if (!objectBranchActive.empty()) objectBranchActive[0] = true;
            for (int row = 0; row < skinfileLines.count; ++row) {
                SKINFILELINEREAD& header = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                if (!header.isIfGroupHead || header.ifgroup <= 0 ||
                    header.ifgroup >= arr_ifunit.count || !header.csv.str[0].body) continue;
                const char* command = header.csv.str[0].outstr();
                if (!strcmp(command, "#ELSE")) {
                    branchMatches[header.ifgroup] = true;
                } else {
                    bool matches = !strcmp(command, "#IF") || !strcmp(command, "#ELSEIF");
                    for (int col = 1; matches && col <= 10; ++col) {
                        const int option = header.csv.val[col];
                        const int optionId = option < 0 ? -option : option;
                        if (optionId > 999) {
                            matches = false;
                        } else {
                            const bool optionEnabled = optionId == 0 || g.skstruct.op[optionId] != 0;
                            if ((option >= 0) != optionEnabled) matches = false;
                        }
                    }
                    branchMatches[header.ifgroup] = matches;
                }
            }
            for (int group = 1; group < arr_ifunit.count; ++group) {
                IFUNIT& unit = ((IFUNIT*)arr_ifunit.data)[group];
                const int chainRoot = getRootIfgroup(group);
                bool earlierTaken = false;
                for (int sibling = 1; sibling < group; ++sibling) {
                    IFUNIT& previous = ((IFUNIT*)arr_ifunit.data)[sibling];
                    if (previous.parentID == unit.parentID && previous.depth == unit.depth &&
                        previous.order < unit.order && getRootIfgroup(sibling) == chainRoot &&
                        branchMatches[sibling]) {
                        earlierTaken = true;
                        break;
                    }
                }
                const bool parentActive = unit.parentID >= 0 &&
                    unit.parentID < (int)objectBranchActive.size()
                    ? objectBranchActive[unit.parentID] : false;
                objectBranchActive[group] = parentActive && !earlierTaken && branchMatches[group];
            }
            objectStatusCacheLineCount = skinfileLines.count;
            objectStatusCacheIfCount = arr_ifunit.count;
            objectStatusCacheAt = statusNow;
        }

        // Command metadata never changes while the helper definition is
        // loaded. Cache the $op column positions instead of linearly searching
        // the whole help table for every DST, every UI frame.
        static std::map<std::string, std::vector<int> > dstOptionColumns;
        static std::map<std::string, std::vector<int> > commandTimerColumns;
        auto optionColumnsFor = [&](const char* command) -> const std::vector<int>& {
            const std::string key = command ? command : "";
            std::map<std::string, std::vector<int> >::iterator found = dstOptionColumns.find(key);
            if (found != dstOptionColumns.end()) return found->second;
            std::vector<int> columns;
            for (int col = 1; col < 30; ++col) {
                CSTR optionHelp = GetCommandHelp(key.c_str(), col);
                optionHelp.trimWhiteSpace();
                if (optionHelp.left(3).isSame("$op")) columns.push_back(col);
            }
            return dstOptionColumns.insert(std::make_pair(key, columns)).first->second;
        };
        auto timerColumnsFor = [&](const char* command) -> const std::vector<int>& {
            const std::string key = command ? command : "";
            std::map<std::string, std::vector<int> >::iterator found = commandTimerColumns.find(key);
            if (found != commandTimerColumns.end()) return found->second;
            std::vector<int> columns;
            for (int col = 1; col < 30; ++col) {
                CSTR help = GetCommandHelp(key.c_str(), col);
                help.trimWhiteSpace();
                if (help.left(6).isSame("$timer")) columns.push_back(col);
            }
            return commandTimerColumns.insert(std::make_pair(key, columns)).first->second;
        };

        // Use the same definition as the green/red Object Browser rows:
        // at least one DST, every DST option enabled, and the owning IF branch active.
        auto objectIsActive = [&](const SEObjectInstance& object) -> bool {
            bool hasDst = false;
            bool allDstEnabled = true;
            for (int row : object.rows) {
                if (row < 0 || row >= skinfileLines.count) continue;
                SKINFILELINEREAD& dstRow = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                if (!dstRow.csv.str[0].body) continue;
                const char* command = dstRow.csv.str[0].outstr();
                if (strncmp(command, "#DST", 4) != 0) continue;
                hasDst = true;
                const std::vector<int>& optionColumns = optionColumnsFor(command);
                for (int col : optionColumns) {
                    if (!GetOptionFlag_dst(&g, dstRow.csv.val[col])) {
                        allDstEnabled = false;
                        break;
                    }
                }
                if (!allDstEnabled) break;
            }

            int ifgroup = object.ifgroup;
            if (ifgroup == 0 && !object.rows.empty())
                ifgroup = ((SKINFILELINEREAD*)skinfileLines.data)[object.rows.front()].ifgroup;
            const bool conditionActive = ifgroup <= 0 ||
                (ifgroup < (int)objectBranchActive.size() && objectBranchActive[ifgroup]);
            return hasDst && allDstEnabled && conditionActive;
        };

        for (int oi = 0; oi < (int)groupObjects.size(); ++oi) {
            const SEObjectInstance& o = objectEditorModel.Objects()[groupObjects[oi]];
            if (objectBrowserActiveOnly && !objectIsActive(o)) continue;
            int ifgroup = o.ifgroup;
            if (ifgroup == 0 && !o.rows.empty())
                ifgroup = ((SKINFILELINEREAD*)skinfileLines.data)[o.rows[0]].ifgroup;

            if (ifgroup == 0) {
                int ci = -1;
                for (int i = 0; i < (int)conditions.size(); ++i)
                    if (conditions[i].rootIfgroup == 0) { ci = i; break; }
                if (ci < 0) {
                    ConditionBlock c;
                    c.rootIfgroup = 0; c.depth = 0; c.label = "ALWAYS";
                    conditions.push_back(c); ci = (int)conditions.size() - 1;
                }
                if (conditions[ci].branches.empty()) {
                    BranchBlock b; b.ifgroup = 0; b.order = 0; b.label = "ALWAYS";
                    conditions[ci].branches.push_back(b);
                }
                conditions[ci].branches[0].localObjectIndices.push_back(oi);
                continue;
            }

            int root = getRootIfgroup(ifgroup);
            int ci = -1;
            for (int i = 0; i < (int)conditions.size(); ++i)
                if (conditions[i].rootIfgroup == root) { ci = i; break; }
            if (ci < 0) {
                ConditionBlock c;
                c.rootIfgroup = root;
                c.depth = ((IFUNIT*)arr_ifunit.data)[root].depth;
                int dummyOrder = 0;
                getHeader(root, c.label, dummyOrder);
                if (c.label.empty()) c.label = "#IF";
                conditions.push_back(c); ci = (int)conditions.size() - 1;
            }

            int bi = -1;
            for (int i = 0; i < (int)conditions[ci].branches.size(); ++i)
                if (conditions[ci].branches[i].ifgroup == ifgroup) { bi = i; break; }
            if (bi < 0) {
                BranchBlock b;
                b.ifgroup = ifgroup;
                getHeader(ifgroup, b.label, b.order);
                if (b.label.empty()) b.label = (b.order == 0) ? "#IF" : "#ELSE";
                conditions[ci].branches.push_back(b); bi = (int)conditions[ci].branches.size() - 1;
            }
            conditions[ci].branches[bi].localObjectIndices.push_back(oi);
        }

        // A condition containing only nested conditions has no direct object,
        // so the object-driven pass above cannot discover it. Add every
        // missing ancestor chain to keep the navigator rooted at the actual
        // top-level #IF instead of starting midway through the source tree.
        std::vector<int> visibleConditionRoots;
        for (const ConditionBlock& condition : conditions)
            if (condition.rootIfgroup > 0) visibleConditionRoots.push_back(condition.rootIfgroup);
        for (int visibleRoot : visibleConditionRoots) {
            int ancestorBranch = visibleRoot > 0 && visibleRoot < arr_ifunit.count
                ? ((IFUNIT*)arr_ifunit.data)[visibleRoot].parentID : 0;
            while (ancestorBranch > 0 && ancestorBranch < arr_ifunit.count) {
                const int ancestorRoot = getRootIfgroup(ancestorBranch);
                bool alreadyPresent = false;
                for (const ConditionBlock& condition : conditions) {
                    if (condition.rootIfgroup == ancestorRoot) {
                        alreadyPresent = true;
                        break;
                    }
                }
                if (!alreadyPresent) {
                    ConditionBlock ancestor;
                    ancestor.rootIfgroup = ancestorRoot;
                    ancestor.depth = ((IFUNIT*)arr_ifunit.data)[ancestorRoot].depth;
                    int dummyOrder = 0;
                    getHeader(ancestorRoot, ancestor.label, dummyOrder);
                    if (ancestor.label.empty()) ancestor.label = "#IF";
                    for (int group = ancestorRoot; group < arr_ifunit.count; ++group) {
                        if (getRootIfgroup(group) != ancestorRoot) continue;
                        BranchBlock branch;
                        branch.ifgroup = group;
                        getHeader(group, branch.label, branch.order);
                        if (branch.label.empty()) branch.label = branch.order == 0 ? "#IF" : "#ELSE";
                        ancestor.branches.push_back(branch);
                    }
                    conditions.push_back(ancestor);
                }
                ancestorBranch = ((IFUNIT*)arr_ifunit.data)[ancestorRoot].parentID;
            }
        }
        std::stable_sort(conditions.begin(), conditions.end(), [](const ConditionBlock& a, const ConditionBlock& b) {
            if (a.rootIfgroup == b.rootIfgroup) return false;
            if (a.rootIfgroup == 0) return true;
            if (b.rootIfgroup == 0) return false;
            return a.rootIfgroup < b.rootIfgroup;
        });

        if (objectBranchTreeIfCount != arr_ifunit.count) {
            objectBranchTreeOpen.clear();
            objectBranchTreeIfCount = arr_ifunit.count;
        }

        int requestedIfgroup = 0;
        if (requestedObjectModel >= 0 && requestedObjectModel <
            (int)objectEditorModel.Objects().size()) {
            const SEObjectInstance& requestedObject =
                objectEditorModel.Objects()[requestedObjectModel];
            requestedIfgroup = requestedObject.ifgroup;
            if (requestedIfgroup == 0 && !requestedObject.rows.empty())
                requestedIfgroup = ((SKINFILELINEREAD*)skinfileLines.data)
                    [requestedObject.rows.front()].ifgroup;
            int pathBranch = requestedIfgroup;
            while (pathBranch > 0 && pathBranch < arr_ifunit.count) {
                objectBranchTreeOpen[pathBranch] = true;
                const int pathRoot = getRootIfgroup(pathBranch);
                pathBranch = pathRoot > 0 && pathRoot < arr_ifunit.count
                    ? ((IFUNIT*)arr_ifunit.data)[pathRoot].parentID : 0;
            }
        }

        auto branchIsOnRequestedPath = [&](int branchIfgroup) -> bool {
            int pathBranch = requestedIfgroup;
            while (pathBranch > 0 && pathBranch < arr_ifunit.count) {
                if (pathBranch == branchIfgroup) return true;
                const int pathRoot = getRootIfgroup(pathBranch);
                pathBranch = pathRoot > 0 && pathRoot < arr_ifunit.count
                    ? ((IFUNIT*)arr_ifunit.data)[pathRoot].parentID : 0;
            }
            return false;
        };

        for (int ci = 0; ci < (int)conditions.size(); ++ci) {
            ConditionBlock& cond = conditions[ci];
            // Conditions are stored in source order, but visually form a
            // tree. Do not draw a nested condition when its owning branch is
            // collapsed; this makes folding a parent hide the whole subtree.
            if (cond.rootIfgroup > 0 && cond.rootIfgroup < arr_ifunit.count) {
                bool ancestorCollapsed = false;
                int parentBranch = ((IFUNIT*)arr_ifunit.data)[cond.rootIfgroup].parentID;
                while (parentBranch > 0 && parentBranch < arr_ifunit.count) {
                    std::map<int, bool>::const_iterator parentOpen = objectBranchTreeOpen.find(parentBranch);
                    if (parentOpen != objectBranchTreeOpen.end() && !parentOpen->second) {
                        ancestorCollapsed = true;
                        break;
                    }
                    const int parentRoot = getRootIfgroup(parentBranch);
                    parentBranch = parentRoot > 0 && parentRoot < arr_ifunit.count
                        ? ((IFUNIT*)arr_ifunit.data)[parentRoot].parentID : 0;
                }
                if (ancestorCollapsed) continue;
            }
            const float conditionIndent = cond.depth > 1
                ? (float)(cond.depth - 1) * ImGui::GetStyle().IndentSpacing : 0.0f;
            if (conditionIndent > 0.0f) ImGui::Indent(conditionIndent);
            ImGui::PushID(cond.rootIfgroup);

            auto drawBranchObjects = [&](BranchBlock& branch) {
                for (int k = 0; k < (int)branch.localObjectIndices.size(); ++k) {
                    int oi = branch.localObjectIndices[k];
                    const SEObjectInstance& o = objectEditorModel.Objects()[groupObjects[oi]];
                    char label[256];
                    int firstRow = o.rows.empty() ? -1 : o.rows[0];
                    int key = -1;
                    if (firstRow >= 0) {
                        SKINFILELINEREAD& r = ((SKINFILELINEREAD*)skinfileLines.data)[firstRow];
                        if (r.csv.str[1].body) key = r.csv.val[1];
                    }
                    std::vector<int> labelOps;
                    for (int rowIndex = 0; rowIndex < (int)o.rows.size() && labelOps.size() < 2; ++rowIndex) {
                        const int row = o.rows[rowIndex];
                        if (row < 0 || row >= skinfileLines.count) continue;
                        SKINFILELINEREAD& opRow = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                        const char* opCommand = opRow.csv.str[0].body ? opRow.csv.str[0].outstr() : "";
                        const std::vector<int>& labelOptionColumns = optionColumnsFor(opCommand);
                        for (int col : labelOptionColumns) {
                            if (labelOps.size() >= 2) break;
                            const int op = opRow.csv.val[col];
                            if (op == 0) continue;
                            bool duplicate = false;
                            for (std::size_t existing = 0; existing < labelOps.size(); ++existing)
                                if (labelOps[existing] == op) { duplicate = true; break; }
                            if (!duplicate) labelOps.push_back(op);
                        }
                    }

                    std::string approximateName;
                    if (!o.name.empty()) {
                        approximateName = Cp932ToUtf8(o.name.c_str());
                    } else {
                        for (std::size_t opIndex = 0; opIndex < labelOps.size(); ++opIndex) {
                            const int op = labelOps[opIndex];
                            const int optionId = op < 0 ? -op : op;
                            const char* optionName = dstName(optionId);
                            if (!optionName || !*optionName) continue;
                            if (!approximateName.empty()) approximateName += " / ";
                            if (op < 0) approximateName += "!";
                            approximateName += optionName;
                        }
                        // Automatic label priority for unnamed objects is
                        // option first, then timer. Timer 0 is the ubiquitous
                        // MainTimer and is intentionally not descriptive.
                        if (approximateName.empty()) {
                            std::vector<int> labelTimers;
                            for (int row : o.rows) {
                                if (row < 0 || row >= skinfileLines.count || labelTimers.size() >= 2) continue;
                                SKINFILELINEREAD& timerRow = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                                const char* command = timerRow.csv.str[0].body
                                    ? timerRow.csv.str[0].outstr() : "";
                                const std::vector<int>& timerColumns = timerColumnsFor(command);
                                for (int col : timerColumns) {
                                    const int timer = timerRow.csv.val[col];
                                    if (timer == 0) continue;
                                    if (std::find(labelTimers.begin(), labelTimers.end(), timer) == labelTimers.end())
                                        labelTimers.push_back(timer);
                                    if (labelTimers.size() >= 2) break;
                                }
                            }
                            for (int timer : labelTimers) {
                                const char* name = timerName(timer, true);
                                if (!name || !*name) continue;
                                if (!approximateName.empty()) approximateName += " / ";
                                approximateName += name;
                            }
                        }
                    }

                    const SEObjectGroupDef* objectTypeDef = objectEditorModel.Group(o.group);
                    const char* objectTypeName = objectTypeDef ? objectTypeDef->name.c_str() : "OBJECT";
                    if (key >= 0 && !approximateName.empty())
                        snprintf(label, sizeof(label), "%03d  [%s]  #%d  %s", oi, objectTypeName, key, approximateName.c_str());
                    else if (key >= 0)
                        snprintf(label, sizeof(label), "%03d  [%s]  #%d", oi, objectTypeName, key);
                    else if (!approximateName.empty())
                        snprintf(label, sizeof(label), "%03d  [%s]  %s", oi, objectTypeName, approximateName.c_str());
                    else
                        snprintf(label, sizeof(label), "%03d  [%s]", oi, objectTypeName);
                    bool hasDst = false;
                    for (int rowIndex = 0; rowIndex < (int)o.rows.size(); ++rowIndex) {
                        int row = o.rows[rowIndex];
                        if (row < 0 || row >= skinfileLines.count) continue;
                        SKINFILELINEREAD& dstRow = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                        if (!dstRow.csv.str[0].body) continue;
                        const char* command = dstRow.csv.str[0].outstr();
                        if (strncmp(command, "#DST", 4) != 0) continue;
                        hasDst = true;
                    }
                    const bool objectEnabled = objectIsActive(o);

                    ImGui::PushID(oi);
                    if (hasDst) {
                        const ImVec4 rowColor = objectEnabled
                            ? ImVec4(0.18f, 0.48f, 0.23f, 0.62f)
                            : ImVec4(0.55f, 0.18f, 0.18f, 0.62f);
                        const ImVec4 hoverColor = objectEnabled
                            ? ImVec4(0.24f, 0.62f, 0.30f, 0.78f)
                            : ImVec4(0.68f, 0.22f, 0.22f, 0.78f);
                        const ImVec2 rowMin = ImGui::GetCursorScreenPos();
                        const float rowWidth = ImGui::GetContentRegionAvail().x;
                        ImGui::GetWindowDrawList()->AddRectFilled(rowMin,
                            ImVec2(rowMin.x + rowWidth, rowMin.y + ImGui::GetFrameHeight()),
                            ImGui::GetColorU32(rowColor));
                        ImGui::PushStyleColor(ImGuiCol_Header, rowColor);
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, hoverColor);
                        ImGui::PushStyleColor(ImGuiCol_HeaderActive, hoverColor);
                    }
                    const int modelIndex = groupObjects[oi];
                    const bool isMultiSelected = std::find(preview_selected_object_model_indices.begin(),
                        preview_selected_object_model_indices.end(), modelIndex) !=
                        preview_selected_object_model_indices.end();
                    const bool clicked = ImGui::Selectable(label, isMultiSelected);
                    const ImVec2 reorderRowMin = ImGui::GetItemRectMin();
                    const ImVec2 reorderRowMax = ImGui::GetItemRectMax();
                    if (modelIndex == requestedObjectModel)
                        ImGui::SetScrollHereY(0.5f);
                    if (hasDst) ImGui::PopStyleColor(3);
                    if (clicked) {
                        selected_object_editor = oi;
                        preview_selected_object_model_index = modelIndex;
                        syncDstSelectionForObject(modelIndex, true);
                        preview_object_dragging = false;
                        preview_selected_obj_valid = false;
                        preview_selected_obj_last_valid = false;

                        if (ImGui::GetIO().KeyShift && preview_selection_anchor_model_index >= 0) {
                            std::vector<int>::const_iterator anchorIt = std::find(groupObjects.begin(), groupObjects.end(),
                                preview_selection_anchor_model_index);
                            std::vector<int>::const_iterator clickedIt = std::find(groupObjects.begin(), groupObjects.end(), modelIndex);
                            if (anchorIt != groupObjects.end() && clickedIt != groupObjects.end()) {
                                int anchorPosition = (int)(anchorIt - groupObjects.begin());
                                int clickedPosition = (int)(clickedIt - groupObjects.begin());
                                if (anchorPosition > clickedPosition) std::swap(anchorPosition, clickedPosition);
                                if (!ImGui::GetIO().KeyCtrl) preview_selected_object_model_indices.clear();
                                for (int rangePosition = anchorPosition; rangePosition <= clickedPosition; ++rangePosition) {
                                    const int rangeModelIndex = groupObjects[rangePosition];
                                    if (std::find(preview_selected_object_model_indices.begin(),
                                        preview_selected_object_model_indices.end(), rangeModelIndex) ==
                                        preview_selected_object_model_indices.end())
                                        preview_selected_object_model_indices.push_back(rangeModelIndex);
                                }
                            } else {
                                preview_selected_object_model_indices.clear();
                                preview_selected_object_model_indices.push_back(modelIndex);
                                preview_selection_anchor_model_index = modelIndex;
                            }
                        } else if (ImGui::GetIO().KeyCtrl) {
                            std::vector<int>::iterator selectedIt = std::find(
                                preview_selected_object_model_indices.begin(),
                                preview_selected_object_model_indices.end(), modelIndex);
                            if (selectedIt == preview_selected_object_model_indices.end())
                                preview_selected_object_model_indices.push_back(modelIndex);
                            else
                                preview_selected_object_model_indices.erase(selectedIt);
                            preview_selection_anchor_model_index = modelIndex;
                        } else {
                            preview_selected_object_model_indices.clear();
                            preview_selected_object_model_indices.push_back(modelIndex);
                            preview_selection_anchor_model_index = modelIndex;
                        }

                        const bool clickedRemainsSelected = std::find(
                            preview_selected_object_model_indices.begin(),
                            preview_selected_object_model_indices.end(), modelIndex) !=
                            preview_selected_object_model_indices.end();
                        const int activeModelIndex = clickedRemainsSelected ? modelIndex :
                            (preview_selected_object_model_indices.empty() ? -1 :
                                preview_selected_object_model_indices.front());
                        SetObjectSelection(preview_selected_object_model_indices,
                            activeModelIndex, preview_selection_anchor_model_index, false);

                        RefreshPreviewSelectionBounds();
                    }
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                        ImGui::SetDragDropPayload("SKINEDITOR_OBJECT_REORDER", &modelIndex,
                            sizeof(modelIndex), ImGuiCond_Once);
                        ImGui::TextUnformatted("Move Object");
                        ImGui::TextDisabled("%s", label);
                        ImGui::Separator();
                        ImGui::TextDisabled("Drop above or below an Object in the same IF branch and file.");
                        ImGui::EndDragDropSource();
                    }
                    if (ImGui::BeginDragDropTarget()) {
                        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(
                            "SKINEDITOR_OBJECT_REORDER",
                            ImGuiDragDropFlags_AcceptBeforeDelivery);
                        if (payload && payload->DataSize == sizeof(int)) {
                            const int sourceModelIndex = *(const int*)payload->Data;
                            const bool placeAfter = ImGui::GetIO().MousePos.y >
                                (reorderRowMin.y + reorderRowMax.y) * 0.5f;
                            const bool validTarget =
                                CanReorderObject(sourceModelIndex, modelIndex);
                            const float markerY = placeAfter
                                ? reorderRowMax.y : reorderRowMin.y;
                            ImGui::GetWindowDrawList()->AddLine(
                                ImVec2(reorderRowMin.x, markerY),
                                ImVec2(reorderRowMax.x, markerY),
                                ImGui::GetColorU32(validTarget
                                    ? ImVec4(0.30f, 0.78f, 1.0f, 1.0f)
                                    : ImVec4(0.95f, 0.32f, 0.32f, 1.0f)),
                                2.0f);
                            if (payload->IsDelivery() && validTarget)
                                QueueObjectReorder(sourceModelIndex, modelIndex, placeAfter);
                        }
                        ImGui::EndDragDropTarget();
                    }
                    if (ImGui::BeginPopupContextItem("ObjectContext")) {
                        ImGui::TextDisabled("Object %03d", oi);
                        if (ImGui::MenuItem("Create Object (new)")) {
                            AssignRootFileOwner(skinfileLines, mainpath, newObjectOwner);
                            newObjectInsertPosition = o.rows.empty()
                                ? FindOwnerFileEndRow(skinfileLines,
                                    newObjectOwner.body ? newObjectOwner.outstr() : mainpath)
                                : o.rows.back() + 1;
                            if (!o.rows.empty()) {
                                SKINFILELINEREAD& ownerRow =
                                    ((SKINFILELINEREAD*)skinfileLines.data)[o.rows.front()];
                                if (ownerRow.filename.body && *ownerRow.filename.outstr())
                                    newObjectOwner.assign(ownerRow.filename.outstr());
                            }
                            newObjectIfgroup = o.ifgroup;
                            newCommandIncludeAll = false;
                            newObjectName.assign("");
                            wNewObject = true;
                        }
                        if (ImGui::MenuItem("Create Command / Setting")) {
                            AssignRootFileOwner(skinfileLines, mainpath, newObjectOwner);
                            newObjectInsertPosition = o.rows.empty()
                                ? FindOwnerFileEndRow(skinfileLines,
                                    newObjectOwner.body ? newObjectOwner.outstr() : mainpath)
                                : o.rows.back() + 1;
                            if (!o.rows.empty()) {
                                SKINFILELINEREAD& ownerRow =
                                    ((SKINFILELINEREAD*)skinfileLines.data)[o.rows.front()];
                                if (ownerRow.filename.body && *ownerRow.filename.outstr())
                                    newObjectOwner.assign(ownerRow.filename.outstr());
                            }
                            newObjectIfgroup = o.ifgroup;
                            newCommandIncludeAll = true;
                            newObjectName.assign("");
                            wNewObject = true;
                        }
                        if (ImGui::MenuItem("Create Object (duplicate)")) contextCreateModel = modelIndex;
                        if (ImGui::MenuItem("Remove Object")) contextDeleteModel = modelIndex;
                        ImGui::Separator();
                        if (ImGui::MenuItem("Create Group from Selection", NULL, false,
                            !preview_selected_object_model_indices.empty()))
                            requestCreateGroupPopup = true;
                        ImGui::EndPopup();
                    }
                    ImGui::PopID();
                }
            };

            if (cond.rootIfgroup == 0) {
                // Unconditional objects are not part of a condition block.
                if (requestedObjectModel >= 0 && requestedIfgroup == 0)
                    ImGui::SetNextItemOpen(true, ImGuiCond_Always);
                bool open = ImGui::TreeNodeEx("always", ImGuiTreeNodeFlags_DefaultOpen, "ALWAYS");
                if (open) {
                    for (int bi = 0; bi < (int)cond.branches.size(); ++bi)
                        drawBranchObjects(cond.branches[bi]);
                    ImGui::TreePop();
                }
            }
            else {
                // The real #IF is the condition node; no synthetic CONDITION
                // wrapper is needed. ELSEIF/ELSE remain sibling branches.
                std::stable_sort(cond.branches.begin(), cond.branches.end(),
                    [](const BranchBlock& a, const BranchBlock& b) { return a.order < b.order; });
                for (int bi = 0; bi < (int)cond.branches.size(); ++bi) {
                    BranchBlock& branch = cond.branches[bi];
                    ImGui::PushID(branch.ifgroup);
                    const bool isBranchActive = branch.ifgroup > 0 &&
                        branch.ifgroup < (int)objectBranchActive.size() &&
                        objectBranchActive[branch.ifgroup];
                    bool isParentActive = false;
                    if (branch.ifgroup > 0 && branch.ifgroup < arr_ifunit.count) {
                        const int parent = ((IFUNIT*)arr_ifunit.data)[branch.ifgroup].parentID;
                        isParentActive = parent >= 0 &&
                            parent < (int)objectBranchActive.size() && objectBranchActive[parent];
                    }
                    const ImVec4 branchColor = !isParentActive
                        ? ImVec4(0.28f, 0.28f, 0.30f, 0.62f)
                        : (isBranchActive
                            ? ImVec4(0.18f, 0.48f, 0.23f, 0.62f)
                            : ImVec4(0.55f, 0.18f, 0.18f, 0.62f));
                    const ImVec4 branchHoverColor = !isParentActive
                        ? ImVec4(0.38f, 0.38f, 0.41f, 0.78f)
                        : (isBranchActive
                            ? ImVec4(0.24f, 0.62f, 0.30f, 0.78f)
                            : ImVec4(0.68f, 0.22f, 0.22f, 0.78f));
                    const ImVec2 branchRowMin = ImGui::GetCursorScreenPos();
                    ImGui::GetWindowDrawList()->AddRectFilled(branchRowMin,
                        ImVec2(branchRowMin.x + ImGui::GetContentRegionAvail().x,
                            branchRowMin.y + ImGui::GetFrameHeight()),
                        ImGui::GetColorU32(branchColor));
                    ImGui::PushStyleColor(ImGuiCol_Header, branchColor);
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, branchHoverColor);
                    ImGui::PushStyleColor(ImGuiCol_HeaderActive, branchHoverColor);
                    if (branchIsOnRequestedPath(branch.ifgroup))
                        ImGui::SetNextItemOpen(true, ImGuiCond_Always);
                    bool branchOpen = ImGui::TreeNodeEx("branch", ImGuiTreeNodeFlags_DefaultOpen,
                        "%s  (%d)", branch.label.c_str(), (int)branch.localObjectIndices.size());
                    const bool showElseContext = branch.label == "#ELSE" &&
                        ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip);
                    objectBranchTreeOpen[branch.ifgroup] = branchOpen;
                    ImGui::PopStyleColor(3);
                    if (ImGui::BeginPopupContextItem("BranchContext")) {
                        int insertPosition = skinfileLines.count;
                        CSTR branchOwner(mainpath);
                        for (int row = 0; row < skinfileLines.count; ++row) {
                            SKINFILELINEREAD& candidate = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                            if (candidate.ifgroup != branch.ifgroup) continue;
                            if (candidate.filename.body && *candidate.filename.outstr())
                                branchOwner.assign(candidate.filename.outstr());
                            if (candidate.isIfGroupEnd) {
                                insertPosition = row;
                                break;
                            }
                            insertPosition = row + 1;
                        }
                        if (ImGui::MenuItem("Create Object in this branch")) {
                            newObjectInsertPosition = insertPosition;
                            newObjectOwner.assign(branchOwner);
                            newObjectIfgroup = branch.ifgroup;
                            newCommandIncludeAll = false;
                            newObjectName.assign("");
                            wNewObject = true;
                        }
                        if (ImGui::MenuItem("Create Command / Setting in this branch")) {
                            newObjectInsertPosition = insertPosition;
                            newObjectOwner.assign(branchOwner);
                            newObjectIfgroup = branch.ifgroup;
                            newCommandIncludeAll = true;
                            newObjectName.assign("");
                            wNewObject = true;
                        }
                        ImGui::EndPopup();
                    }
                    if (showElseContext) {
                        std::vector<std::pair<int, std::string> > precedingConditions;
                        for (int group = 1; group < arr_ifunit.count; ++group) {
                            if (getRootIfgroup(group) != cond.rootIfgroup) continue;
                            std::string siblingLabel;
                            int siblingOrder = 0;
                            if (!getHeader(group, siblingLabel, siblingOrder) ||
                                siblingOrder >= branch.order || siblingLabel == "#ELSE") continue;
                            precedingConditions.push_back(std::make_pair(siblingOrder, siblingLabel));
                        }
                        std::stable_sort(precedingConditions.begin(), precedingConditions.end(),
                            [](const std::pair<int, std::string>& a,
                                const std::pair<int, std::string>& b) {
                                return a.first < b.first;
                            });

                        if (ImGui::BeginTooltip()) {
                            ImGui::TextDisabled("ELSE fallback for");
                            for (const std::pair<int, std::string>& condition : precedingConditions) {
                                ImGui::Bullet();
                                ImGui::SameLine();
                                ImGui::TextUnformatted(condition.second.c_str());
                            }
                            if (precedingConditions.empty())
                                ImGui::TextDisabled("(condition header not found)");
                            ImGui::Separator();
                            ImGui::TextDisabled("Used when none of the conditions above match.");
                            ImGui::EndTooltip();
                        }
                    }
                    if (branchOpen) {
                        drawBranchObjects(branch);
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
            }
            ImGui::PopID();
            if (conditionIndent > 0.0f) ImGui::Unindent(conditionIndent);
        }
        }
        ImGui::EndChild();
    }

    if (contextCreateModel >= 0 || contextDeleteModel >= 0) {
        const std::vector<SEObjectInstance>& allObjects = objectEditorModel.Objects();
        if (contextCreateModel >= 0 && contextCreateModel < (int)allObjects.size()) {
            const SEObjectInstance sourceObject = allObjects[contextCreateModel];
            struct ClonedLine { std::string text, owner; int sourceNum, ifgroup; };
            std::vector<ClonedLine> cloneLines;
            for (int row : sourceObject.rows) {
                if (row < 0 || row >= skinfileLines.count) continue;
                SKINFILELINEREAD& source = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                ClonedLine clone;
                clone.text = source.line.body ? source.line.outstr() : "";
                clone.owner = source.filename.body ? source.filename.outstr() : mainpath;
                clone.sourceNum = source.num;
                clone.ifgroup = source.ifgroup;
                cloneLines.push_back(clone);
            }
            if (!cloneLines.empty()) {
                char generatedId[64];
                snprintf(generatedId, sizeof(generatedId), "obj_%08llX_%04d", GetTickCount64(), contextCreateModel);
                int insertPosition = sourceObject.rows.back() + 1;
                auto insertCloneLine = [&](const char* text, const char* owner, int sourceNum, int ifgroup) {
                    InsertLine(insertPosition);
                    SKINFILELINEREAD& inserted = ((SKINFILELINEREAD*)skinfileLines.data)[insertPosition];
                    CSTR oldLine(inserted.line);
                    EditLine(insertPosition, oldLine, CSTR(text));
                    inserted.filename.assign(owner && *owner ? owner : mainpath);
                    inserted.num = sourceNum;
                    inserted.ifgroup = ifgroup;
                    ++insertPosition;
                };
                std::string idLine = std::string("$SE_OBJECT_ID,") + generatedId;
                insertCloneLine(idLine.c_str(), cloneLines.front().owner.c_str(), cloneLines.front().sourceNum,
                    cloneLines.front().ifgroup);
                for (const ClonedLine& clone : cloneLines)
                    insertCloneLine(clone.text.c_str(), clone.owner.c_str(), clone.sourceNum, clone.ifgroup);

                if (selected_user_object_group >= 0) {
                    int currentGroup = -1;
                    for (int row = 0; row < skinfileLines.count; ++row) {
                        SKINFILELINEREAD& meta = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                        const char* text = meta.line.body ? meta.line.outstr() : "";
                        if (strncmp(text, "$SE_GROUP_BEGIN,", 16) == 0) ++currentGroup;
                        else if (strcmp(text, "$SE_GROUP_END") == 0 && currentGroup == selected_user_object_group) {
                            InsertLine(row);
                            SKINFILELINEREAD& member = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                            CSTR oldLine(member.line);
                            std::string memberLine = std::string("$SE_GROUP_MEMBER,") + generatedId;
                            EditLine(row, oldLine, CSTR(memberLine.c_str()));
                            member.filename.assign(mainpath);
                            break;
                        }
                    }
                }
                RebuildObjectModel();
                preview_selected_object_model_indices.clear();
                const std::vector<SEObjectInstance>& rebuilt = objectEditorModel.Objects();
                for (int i = 0; i < (int)rebuilt.size(); ++i) {
                    if (rebuilt[i].editorId == generatedId) {
                        SetObjectSelection(std::vector<int>(1, i), i, i, true);
                        break;
                    }
                }
            }
        } else if (contextDeleteModel >= 0 && contextDeleteModel < (int)allObjects.size()) {
            const SEObjectInstance deleting = allObjects[contextDeleteModel];
            std::vector<int> deleteRows = deleting.rows;
            if (!deleting.rows.empty()) {
                for (int row = deleting.rows.front() - 1; row >= 0; --row) {
                    SKINFILELINEREAD& meta = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    const char* text = meta.line.body ? meta.line.outstr() : "";
                    if (strncmp(text, "$SE_OBJECT_ID,", 14) == 0 ||
                        strncmp(text, "$SE_OBJECT_NAME,", 16) == 0) {
                        deleteRows.push_back(row);
                        continue;
                    }
                    if (*text == '#') break;
                    if (*text && *text != '$' && strncmp(text, "//", 2) != 0) break;
                }
            }
            if (!deleting.editorId.empty()) {
                const std::string memberLine = "$SE_GROUP_MEMBER," + deleting.editorId;
                for (int row = 0; row < skinfileLines.count; ++row) {
                    SKINFILELINEREAD& meta = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    if (meta.line.body && memberLine == meta.line.outstr()) deleteRows.push_back(row);
                }
            }
            std::sort(deleteRows.begin(), deleteRows.end());
            deleteRows.erase(std::unique(deleteRows.begin(), deleteRows.end()), deleteRows.end());
            for (std::vector<int>::reverse_iterator it = deleteRows.rbegin(); it != deleteRows.rend(); ++it)
                DeleteLine(*it);
            RebuildObjectModel();
            selected_object_editor = 0;
        }
        if (browserWasBegun) ImGui::End();
        return 0;
    }
    if (browserWasBegun) ImGui::End();

    if (!wObjectInspector) return 0;
    if (!ImGui::Begin(inspectorTitle, &wObjectInspector)) {
        ImGui::End();
        return 0;
    }

    int renameObjectModelIndex = -1;
    std::string renamedObjectName;

    // Object Inspector: actual Object data. #IF/#ELSEIF/#ELSE are context only.
    const bool drawProperties = ImGui::BeginChild("ObjectProperties", ImVec2(0, 0), true);
    if (drawProperties) {
        if (selected_object_editor >= 0 && selected_object_editor < (int)groupObjects.size()) {
            SEObjectInstance& obj = objectEditorModel.ObjectsMutable()[groupObjects[selected_object_editor]];
            const SEObjectGroupDef* def = objectEditorModel.Group(obj.group);
            if (def) {
                ImGui::Text("%s", def->name.c_str());

                int ifgroup = obj.ifgroup;
                if (ifgroup == 0 && !obj.rows.empty())
                    ifgroup = ((SKINFILELINEREAD*)skinfileLines.data)[obj.rows[0]].ifgroup;
                ImGui::SameLine();
                if (ifgroup == 0) {
                    ImGui::TextDisabled("[ALWAYS]");
                } else {
                    std::string flowName = "IF";
                    if (ifgroup < arr_ifunit.count) {
                        IFUNIT& unit = ((IFUNIT*)arr_ifunit.data)[ifgroup];
                        if (unit.order > 0) flowName = "ELSE";
                        for (int rno = 0; rno < skinfileLines.count; ++rno) {
                            SKINFILELINEREAD& r = ((SKINFILELINEREAD*)skinfileLines.data)[rno];
                            if (r.ifgroup != ifgroup || !r.isIfGroupHead || !r.csv.str[0].body) continue;
                            flowName = formatConditionHeader(r);
                            break;
                        }
                    }
                    ImGui::TextDisabled("[%s]", flowName.c_str());
                }
                ImGui::Separator();

                char objectName[256];
                const std::string objectNameUtf8 = Cp932ToUtf8(obj.name.c_str());
                strncpy(objectName, objectNameUtf8.c_str(), sizeof(objectName) - 1);
                objectName[sizeof(objectName) - 1] = '\0';
                ImGui::SetNextItemWidth(260.0f);
                const bool objectNameChanged =
                    ImGui::InputText("Name", objectName, sizeof(objectName));
                if (objectNameChanged)
                    obj.name = Utf8ToCp932(objectName);
                const bool commitObjectName = ImGui::IsItemDeactivatedAfterEdit() ||
                    (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter));
                if (commitObjectName) {
                    renameObjectModelIndex = groupObjects[selected_object_editor];
                    renamedObjectName = obj.name;
                }

                auto drawTaggedImageSelector = [&](int row, const char* command) {
                    if (!command || strncmp(command, "#SRC", 4) != 0 || arr_IMG.count <= 0) return;
                    SKINFILELINEREAD& srcLine = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    const char* fieldNames[5] = { "gr", "x", "y", "w", "h" };
                    int columns[5] = { -1, -1, -1, -1, -1 };
                    for (int col = 1; col < 30; ++col) {
                        CSTR help = GetCommandHelp(command, col);
                        help.trimWhiteSpace();
                        const char* label = help.body ? help.outstr() : "";
                        if (*label == '$') ++label;
                        for (int field = 0; field < 5; ++field)
                            if (_stricmp(label, fieldNames[field]) == 0) columns[field] = col;
                    }
                    for (int field = 0; field < 5; ++field) if (columns[field] < 0) return;

                    const int currentTag = FindIMG(srcLine.csv.val[columns[0]], srcLine.csv.val[columns[1]],
                        srcLine.csv.val[columns[2]], srcLine.csv.val[columns[3]], srcLine.csv.val[columns[4]]);
                    char preview[260];
                    if (currentTag >= 0 && currentTag < arr_IMG.count) {
                        IMG& tag = ((IMG*)arr_IMG.data)[currentTag];
                        const std::string tagNameUtf8 = Cp932ToUtf8(
                            tag.name.body ? tag.name.outstr() : "noname");
                        snprintf(preview, sizeof(preview), "%03d  %s", currentTag,
                            tagNameUtf8.c_str());
                    } else snprintf(preview, sizeof(preview), "Custom coordinates");

                    ImGui::SetNextItemWidth(300.0f);
                    if (ImGui::BeginCombo("Tagged image", preview)) {
                        for (int tagIndex = 0; tagIndex < arr_IMG.count; ++tagIndex) {
                            IMG& tag = ((IMG*)arr_IMG.data)[tagIndex];
                            char tagLabel[260];
                            const std::string tagNameUtf8 = Cp932ToUtf8(
                                tag.name.body ? tag.name.outstr() : "noname");
                            snprintf(tagLabel, sizeof(tagLabel), "%03d  G%02d  %s", tagIndex, tag.gr,
                                tagNameUtf8.c_str());
                            if (ImGui::Selectable(tagLabel, tagIndex == currentTag)) {
                                for (int field = 0; field < 5; ++field) {
                                    const int values[5] = { tag.gr, tag.x, tag.y, tag.w, tag.h };
                                    EditValue(row, columns[field], values[field]);
                                }
                                src_selected = tagIndex;
                                gr_selected = -1;
                                for (int candidate = 0; candidate < arr_SRCGR.count; ++candidate) {
                                    SRCGR& source = ((SRCGR*)arr_SRCGR.data)[candidate];
                                    if (source.grID == tag.gr && source.isIf == tag.ifGroup) {
                                        gr_selected = candidate;
                                        break;
                                    }
                                }
                            }
                            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone) &&
                                tag.gr >= 0) {
                                int tagImageIndex = -1;
                                // Preview the tag against the branch of the
                                // SRC currently being edited. Coordinate tags
                                // themselves are shared across branches.
                                for (int candidate = 0; candidate < arr_SRCGR.count; ++candidate) {
                                    SRCGR& source = ((SRCGR*)arr_SRCGR.data)[candidate];
                                    if (source.grID == tag.gr && source.isIf == srcLine.ifgroup) {
                                        tagImageIndex = candidate;
                                        break;
                                    }
                                }
                                if (tagImageIndex < 0) {
                                    for (int candidate = 0; candidate < arr_SRCGR.count; ++candidate) {
                                        if (((SRCGR*)arr_SRCGR.data)[candidate].grID == tag.gr) {
                                            tagImageIndex = candidate;
                                            break;
                                        }
                                    }
                                }
                                if (tagImageIndex < 0) continue;
                                EnsureSRCGRTexture(tagImageIndex);
                                SRCGR& sourceImage = ((SRCGR*)arr_SRCGR.data)[tagImageIndex];
                                const int tagW = tag.w == -1 ? sourceImage.sizeX - tag.x : tag.w;
                                const int tagH = tag.h == -1 ? sourceImage.sizeY - tag.y : tag.h;
                                if (sourceImage.texture && sourceImage.sizeX > 0 && sourceImage.sizeY > 0 &&
                                    tagW > 0 && tagH > 0 && ImGui::BeginTooltip()) {
                                    float scale = 1.0f;
                                    if (tagW > 240 || tagH > 180) {
                                        const float sx = 240.0f / tagW, sy = 180.0f / tagH;
                                        scale = sx < sy ? sx : sy;
                                    }
                                    ImGui::Image(sourceImage.texture, ImVec2(tagW * scale, tagH * scale),
                                        ImVec2(tag.x / (float)sourceImage.sizeX, tag.y / (float)sourceImage.sizeY),
                                        ImVec2((tag.x + tagW) / (float)sourceImage.sizeX,
                                            (tag.y + tagH) / (float)sourceImage.sizeY));
                                    ImGui::Text("gr %d  x %d  y %d  w %d  h %d", tag.gr, tag.x, tag.y, tag.w, tag.h);
                                    ImGui::EndTooltip();
                                }
                            }
                        }
                        ImGui::EndCombo();
                    }
                };

                auto drawObjectPropertyRow = [&](int row, int compactDstIndex) {
                    if (row < 0 || row >= skinfileLines.count) return;
                    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    const char* command = line.csv.str[0].body ? line.csv.str[0].outstr() : "";
                    const bool compactDst = compactDstIndex >= 0;
                    ImGui::PushID(row);
                    if (compactDst) {
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 2.0f));
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 2.0f));
                    }
                    if (compactDst || ImGui::CollapsingHeader(command, ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (!compactDst) drawTaggedImageSelector(row, command);
                        int maxcol = 30;
                        for (int col = 1; col < maxcol; ++col) {
                            CSTR help = GetCommandHelp(command, col);
                            help.trimWhiteSpace();
                            if (!line.csv.str[col].body && !help.body) continue;
                            const char* label = help.body ? help.outstr() : "value";
                            if (!label || !*label) continue;
                            // In animation frames after the first DST, these values are inherited
                            // from the first row and are not used independently. Match by field
                            // name because their column positions differ between DST commands.
                            const bool isSharedDstField = _strnicmp(label, "loop", 4) == 0 ||
                                _strnicmp(label, "timer", 5) == 0 || _strnicmp(label, "$timer", 6) == 0 ||
                                _strnicmp(label, "op1", 3) == 0 || _strnicmp(label, "$op1", 4) == 0 ||
                                _strnicmp(label, "op2", 3) == 0 || _strnicmp(label, "$op2", 4) == 0 ||
                                _strnicmp(label, "op3", 3) == 0 || _strnicmp(label, "$op3", 4) == 0;
                            if (compactDstIndex > 0 && isSharedDstField) continue;
                            ImGui::PushID(col);
                            const char* widgetLabel = label;
                            if (compactDst) {
                                if (compactDstIndex == 0) {
                                    ImGui::AlignTextToFramePadding();
                                    ImGui::TextDisabled("%s", label);
                                    ImGui::SameLine(62.0f);
                                }
                                widgetLabel = "##value";
                                ImGui::SetNextItemWidth(-FLT_MIN);
                            }
                            const int current = line.csv.val[col];
                            int selectedValue = current;
                            if (DrawCommandValueCombo(widgetLabel, command,
                                help.body ? help.outstr() : "", current, selectedValue)) {
                                if (selectedValue != current) EditValue(row, col, selectedValue);
                            } else {
                                CSTR before(line.csv.str[col]);
                                CstrInputText(widgetLabel, &line.csv.str[col], ImGuiInputTextFlags_EnterReturnsTrue);
                                if (before.isDiff(line.csv.str[col])) {
                                    CSTR newValue(line.csv.str[col]);
                                    line.csv.str[col] = before;
                                    EditValue(row, col, newValue.outstr());
                                }
                            }
                            ImGui::PopID();
                        }
                    }
                    if (compactDst) ImGui::PopStyleVar(2);
                    ImGui::PopID();
                };

                std::vector<int> srcRows;
                std::vector<int> dstRows;
                bool requestAddDstFrame = false;
                bool requestRemoveDstFrame = false;
                for (std::size_t ri = 0; ri < obj.rows.size(); ++ri) {
                    const int row = obj.rows[ri];
                    if (row < 0 || row >= skinfileLines.count) continue;
                    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    const char* command = line.csv.str[0].body ? line.csv.str[0].outstr() : "";
                    if (strncmp(command, "#DST", 4) == 0)
                        dstRows.push_back(row);
                    else
                        srcRows.push_back(row);
                }

                if (ImGui::BeginTabBar("ObjectPropertyTabs")) {
                    if (ImGui::BeginTabItem("SRC")) {
                        ImGui::SeparatorText("basic");
                        if (srcRows.empty()) ImGui::TextDisabled("No SRC properties.");
                        for (std::size_t i = 0; i < srcRows.size(); ++i)
                            drawObjectPropertyRow(srcRows[i], -1);
                        ImGui::EndTabItem();
                    }

                    char dstTabLabel[64];
                    // Keep the tab identity stable while its visible count
                    // changes, so +DST/-DST does not switch back to SRC.
                    snprintf(dstTabLabel, sizeof(dstTabLabel), "DST (%d)###ObjectDstTab", (int)dstRows.size());
                    if (ImGui::BeginTabItem(dstTabLabel)) {
                        ImGui::BeginDisabled(dstRows.empty());
                        if (ImGui::Button("+ DST")) requestAddDstFrame = true;
                        ImGui::EndDisabled();
                        ImGui::SameLine();
                        ImGui::BeginDisabled(dstRows.size() <= 1);
                        if (ImGui::Button("- DST")) requestRemoveDstFrame = true;
                        ImGui::EndDisabled();
                        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && dstRows.size() <= 1)
                            ImGui::SetTooltip("An Object must keep at least one DST command.");
                        ImGui::SeparatorText(dstRows.size() > 1 ? "animation frames" : "basic");
                        if (dstRows.empty()) {
                            ImGui::TextDisabled("No DST properties.");
                        } else if (dstRows.size() == 1) {
                            drawObjectPropertyRow(dstRows[0], -1);
                        } else {
                            // Most DST values are short integers. Keep animation
                            // columns dense so several frames fit on screen.
                            const float labelWidth = 50.0f;
                            const float valueWidth = 82.0f;
                            const float cellPaddingWidth = ImGui::GetStyle().CellPadding.x * 2.0f;
                            const float innerWidth = labelWidth + valueWidth * (float)dstRows.size() +
                                cellPaddingWidth * ((float)dstRows.size() + 1.0f) + 16.0f;
                            const ImGuiTableFlags flags = ImGuiTableFlags_Borders |
                                ImGuiTableFlags_ScrollX | ImGuiTableFlags_SizingFixedFit |
                                ImGuiTableFlags_RowBg;
                            if (ImGui::BeginTable("DSTHorizontalRows", (int)dstRows.size() + 1, flags,
                                ImVec2(0.0f, 0.0f), innerWidth)) {
                                // Keep property names and the authoritative first DST visible;
                                // only animation frames after DST 1 scroll horizontally.
                                ImGui::TableSetupScrollFreeze(2, 1);
                                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, labelWidth);
                                for (int i = 0; i < (int)dstRows.size(); ++i) {
                                    char columnLabel[32];
                                    snprintf(columnLabel, sizeof(columnLabel), "DST %d", i + 1);
                                    ImGui::TableSetupColumn(columnLabel, ImGuiTableColumnFlags_WidthFixed, valueWidth);
                                }
                                ImGui::TableHeadersRow();

                                SKINFILELINEREAD& firstDst = ((SKINFILELINEREAD*)skinfileLines.data)[dstRows[0]];
                                const char* firstCommand = firstDst.csv.str[0].body ? firstDst.csv.str[0].outstr() : "";
                                for (int col = 1; col < 30; ++col) {
                                    CSTR firstHelp = GetCommandHelp(firstCommand, col);
                                    firstHelp.trimWhiteSpace();
                                    // Multi-DST rows follow the documented command fields only.
                                    // Ignore trailing CSV values and CR/LF-only helper entries.
                                    if (!firstHelp.body) continue;
                                    const char* rowLabel = firstHelp.outstr();
                                    if (!rowLabel || !*rowLabel) continue;

                                    const bool sharedField = _strnicmp(rowLabel, "loop", 4) == 0 ||
                                        _strnicmp(rowLabel, "timer", 5) == 0 || _strnicmp(rowLabel, "$timer", 6) == 0 ||
                                        _strnicmp(rowLabel, "op1", 3) == 0 || _strnicmp(rowLabel, "$op1", 4) == 0 ||
                                        _strnicmp(rowLabel, "op2", 3) == 0 || _strnicmp(rowLabel, "$op2", 4) == 0 ||
                                        _strnicmp(rowLabel, "op3", 3) == 0 || _strnicmp(rowLabel, "$op3", 4) == 0;

                                    ImGui::TableNextRow();
                                    ImGui::TableSetColumnIndex(0);
                                    ImGui::AlignTextToFramePadding();
                                    ImGui::TextDisabled("%s", rowLabel);

                                    for (int i = 0; i < (int)dstRows.size(); ++i) {
                                        ImGui::TableSetColumnIndex(i + 1);
                                        if (i > 0 && sharedField) continue;

                                        const int row = dstRows[i];
                                        SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                                        CSTR help = GetCommandHelp(line.csv.str[0].outstr(), col);
                                        help.trimWhiteSpace();
                                        ImGui::PushID(row);
                                        ImGui::PushID(col);
                                        ImGui::SetNextItemWidth(-FLT_MIN);
                                        const int current = line.csv.val[col];
                                        int selectedValue = current;
                                        const SECommandValueKind valueKind = GetCommandValueKind(
                                            line.csv.str[0].outstr(), help.body ? help.outstr() : "");
                                        if (DrawCommandValueCombo("##value", line.csv.str[0].outstr(),
                                            help.body ? help.outstr() : "", current, selectedValue)) {
                                            if (selectedValue != current) EditValue(row, col, selectedValue);
                                            if ((valueKind == SE_VALUE_OPTION || valueKind == SE_VALUE_TIMER) &&
                                                ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
                                                const char* valueName = GetCommandValueName(valueKind, current);
                                                ImGui::SetTooltip("%03d:%s", current, valueName ? valueName : "");
                                            }
                                        } else {
                                            CSTR before(line.csv.str[col]);
                                            CstrInputText("##value", &line.csv.str[col], ImGuiInputTextFlags_EnterReturnsTrue);
                                            if (before.isDiff(line.csv.str[col])) {
                                                CSTR newValue(line.csv.str[col]);
                                                line.csv.str[col] = before;
                                                EditValue(row, col, newValue.outstr());
                                            }
                                        }
                                        ImGui::PopID();
                                        ImGui::PopID();
                                    }
                                }
                                ImGui::EndTable();
                            }
                        }
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }

                // Apply structural edits after all widgets referencing the
                // current rows have been submitted. InsertLine/DeleteLine
                // update source-row indices and are already integrated with
                // Ctrl+Z history.
                const std::string editedObjectId = obj.editorId;
                const int editedObjectGroup = obj.group;
                const int editedObjectAnchorRow = obj.rows.empty() ? -1 : obj.rows.front();
                auto restoreEditedObjectSelection = [&]() {
                    const std::vector<SEObjectInstance>& rebuilt = objectEditorModel.Objects();
                    int restoredModel = -1;
                    for (int modelIndex = 0; modelIndex < (int)rebuilt.size(); ++modelIndex) {
                        const SEObjectInstance& candidate = rebuilt[modelIndex];
                        if (!editedObjectId.empty() && candidate.editorId == editedObjectId) {
                            restoredModel = modelIndex;
                            break;
                        }
                        if (editedObjectId.empty() && candidate.group == editedObjectGroup &&
                            std::find(candidate.rows.begin(), candidate.rows.end(), editedObjectAnchorRow) !=
                            candidate.rows.end()) {
                            restoredModel = modelIndex;
                            break;
                        }
                    }
                    if (restoredModel >= 0) {
                        SetObjectSelection(std::vector<int>(1, restoredModel),
                            restoredModel, restoredModel, true);
                    }
                };
                if (requestAddDstFrame && !dstRows.empty()) {
                    const int sourceRow = dstRows.back();
                    const int insertAt = sourceRow + 1;
                    SKINFILELINEREAD& source = ((SKINFILELINEREAD*)skinfileLines.data)[sourceRow];
                    CSTR duplicatedLine(source.line);
                    CSTR sourceOwner(source.filename);
                    const int sourceFileLine = source.num;
                    const int sourceIfgroup = source.ifgroup;
                    const int sourceObjID = source.objID;
                    const int sourceObjType = source.objType;
                    const int sourceObjInTypeID = source.objInTypeID;
                    if (InsertLine(insertAt) == 0) {
                        // Keep the insertion as the single undo record. The
                        // content initialization is part of that insertion.
                        applyingHistory = true;
                        CSTR placeholder(((SKINFILELINEREAD*)skinfileLines.data)[insertAt].line);
                        EditLine(insertAt, placeholder, duplicatedLine);
                        applyingHistory = false;
                        SKINFILELINEREAD& inserted =
                            ((SKINFILELINEREAD*)skinfileLines.data)[insertAt];
                        inserted.filename.assign(sourceOwner.body ? sourceOwner.outstr() : mainpath);
                        inserted.num = sourceFileLine;
                        inserted.ifgroup = sourceIfgroup;
                        inserted.isDST = true;
                        inserted.isSRC = false;
                        inserted.isOther = false;
                        inserted.objID = sourceObjID;
                        inserted.objType = sourceObjType;
                        inserted.objInTypeID = sourceObjInTypeID;
                        RebuildObjectModel();
                        restoreEditedObjectSelection();
                        previewReloadPending = true;
                        previewReloadRequestedAt = GetTickCount64();
                    }
                } else if (requestRemoveDstFrame && dstRows.size() > 1) {
                    if (DeleteLine(dstRows.back()) == 0) {
                        RebuildObjectModel();
                        restoreEditedObjectSelection();
                        previewReloadPending = true;
                        previewReloadRequestedAt = GetTickCount64();
                    }
                }
            }
        } else {
            ImGui::Text("No object.");
        }
    }
    ImGui::EndChild();

    // Name insertion can shift every following CSV row. Apply it only after
    // all Inspector widgets for this frame have released their row references.
    if (renameObjectModelIndex >= 0)
        SetObjectName(renameObjectModelIndex, renamedObjectName.c_str());

    ImGui::End();
    return 0;
}
