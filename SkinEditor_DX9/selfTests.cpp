#include "selfTests.h"

#include "../LR2/LR2_skinmanage.h"
#include "seHelper.h"
#include "seObjectEditor.h"
#include "skin.h"
#include "skinBrowser.h"
#include "uiCatalog.h"

#include <cstdio>
#include <cstring>
#include <set>
#include <string>

namespace {

bool IsSchemaField(const char* command, int column, const char* expected) {
    CSTR field = GetCommandHelp(command, column);
    field.trimWhiteSpace();
    return field.body && std::strcmp(field.outstr(), expected) == 0;
}

bool GroupContains(const SEObjectGroupDef& group, const char* command) {
    for (const std::string& candidate : group.commands) {
        if (candidate == command) return true;
    }
    return false;
}

const SEObjectGroupDef* FindGroup(const SEObjectEditorModel& model,
    const char* name) {
    const std::vector<SEObjectGroupDef>& groups = model.Groups();
    for (const SEObjectGroupDef& group : groups) {
        if (group.name == name) return &group;
    }
    return nullptr;
}

}

int RunSchemaContractSelfTest() {
    // Passing no override path exercises the RCDATA used by packaged builds.
    if (LoadCommandHelp(nullptr) != 0) return 1;
    if (arr_CommandHelp.count != 117) return 2;

    if (!IsSchemaField("#SRC_NUMBER", 11, "$num")) return 3;
    if (!IsSchemaField("#SRC_SLIDER", 13, "$type")) return 4;
    if (!IsSchemaField("#SRC_BUTTON", 11, "$type")) return 5;
    if (!IsSchemaField("#SRC_BARGRAPH", 11, "$type")) return 6;

    if (GetCommandValueKind("#SRC_NUMBER", "$num") != SE_VALUE_NUMBER)
        return 7;
    if (GetCommandValueKind("#SRC_SLIDER", "$type") != SE_VALUE_SLIDER)
        return 8;
    if (GetCommandValueKind("#SRC_BUTTON", "$type") != SE_VALUE_BUTTON)
        return 9;
    if (GetCommandValueKind("#SRC_BARGRAPH", "$type") != SE_VALUE_BARGRAPH)
        return 10;

    SEObjectEditorModel model;
    if (!model.LoadGroups(nullptr)) return 11;
    if (model.Groups().size() != 39) return 12;

    const SEObjectGroupDef* number = FindGroup(model, "NUMBER");
    if (!number || !GroupContains(*number, "#SRC_NUMBER") ||
        !GroupContains(*number, "#DST_NUMBER"))
        return 13;

    const SEObjectGroupDef* note = FindGroup(model, "NOTE");
    if (!note || !GroupContains(*note, "#SRC_NOTE") ||
        !GroupContains(*note, "#DST_NOTE"))
        return 14;

    return 0;
}

int RunUiCatalogSelfTest() {
    if (kSEUIWindowSpecCount != static_cast<std::size_t>(SEUIWindowId::Count))
        return 1;
    if (kSEUISurfaceSpecCount != static_cast<std::size_t>(SEUISurfaceId::Count))
        return 2;

    std::set<std::string> keys;
    std::set<std::string> titles;
    for (std::size_t index = 0; index < kSEUIWindowSpecCount; ++index) {
        const SEUIWindowSpec& spec = kSEUIWindowSpecs[index];
        if (static_cast<std::size_t>(spec.id) != index) return 3;
        if (!spec.key || !*spec.key || !spec.title || !*spec.title ||
            !spec.purpose || !*spec.purpose || !spec.ownerFunction ||
            !*spec.ownerFunction || !spec.group || !*spec.group ||
            !spec.defaultDock || !*spec.defaultDock)
            return 4;
        if (std::strstr(spec.title, "##")) return 5;
        if (!keys.insert(spec.key).second) return 6;
        if (!titles.insert(spec.title).second) return 7;
    }
    for (std::size_t index = 0; index < kSEUISurfaceSpecCount; ++index) {
        const SEUISurfaceSpec& spec = kSEUISurfaceSpecs[index];
        if (static_cast<std::size_t>(spec.id) != index) return 8;
        if (!spec.key || !*spec.key || !spec.title || !*spec.title ||
            !spec.purpose || !*spec.purpose || !spec.ownerFunction ||
            !*spec.ownerFunction || !spec.kind || !*spec.kind)
            return 9;
        if (!keys.insert(spec.key).second) return 10;
        if (!titles.insert(spec.title).second) return 11;
    }

    char title[128];
    FormatSEUIWindowTitle(title, sizeof(title), SEUIWindowId::ImageManager, 7);
    if (std::strcmp(title, "Image Manager##image-manager-7") != 0) return 12;
    FormatSEUIWindowTitle(title, sizeof(title), SEUIWindowId::History, 3);
    if (std::strcmp(title, "History##history-3") != 0) return 13;
    if (std::strcmp(SEUIWindowSpecFor(SEUIWindowId::ObjectBrowser).defaultDock,
        "left-upper") != 0) return 14;
    if (std::strcmp(SEUIWindowSpecFor(SEUIWindowId::ObjectInspector).defaultDock,
        "left-lower") != 0) return 15;
    if (std::strcmp(SEUIWindowSpecFor(SEUIWindowId::TextEditor).defaultDock,
        "center-tabs") != 0) return 16;
    if (std::strcmp(SEUIWindowSpecFor(SEUIWindowId::ObjectProperty).group,
        "Advanced") != 0) return 17;

    return 0;
}

int RunSkinBrowserSelfTest() {
    if (!SEIsSkinScriptFilename("skin.lr2skin")) return 1;
    if (!SEIsSkinScriptFilename("skin.LR2SS")) return 2;
    if (SEIsSkinScriptFilename("skin.txt")) return 3;

    char temporaryRoot[MAX_PATH] = {};
    char scanRoot[MAX_PATH] = {};
    if (!GetTempPathA(MAX_PATH, temporaryRoot)) return 4;
    if (!GetTempFileNameA(temporaryRoot, "SEB", 0, scanRoot)) return 5;
    DeleteFileA(scanRoot);
    if (!CreateDirectoryA(scanRoot, NULL)) return 6;

    const std::string nested = std::string(scanRoot) + "\\nested";
    const std::string rootSkin = std::string(scanRoot) + "\\root.LR2SKIN";
    const std::string nestedSkin = nested + "\\nested.lr2ss";
    const std::string ignored = std::string(scanRoot) + "\\readme.txt";
    int result = 0;
    if (!CreateDirectoryA(nested.c_str(), NULL)) result = 7;

    const auto writeEmptyFile = [](const std::string& path) {
        FILE* file = fopen(path.c_str(), "wb");
        if (!file) return false;
        fclose(file);
        return true;
    };
    if (result == 0 && (!writeEmptyFile(rootSkin) ||
        !writeEmptyFile(nestedSkin) || !writeEmptyFile(ignored))) result = 8;

    if (result == 0) {
        const SESkinFolderScanResult scan = SEScanSkinFolder(scanRoot);
        if (!scan.success) result = 9;
        else if (scan.files.size() != 2) result = 10;
        else if (_stricmp(scan.files[0].c_str(), nestedSkin.c_str()) != 0 ||
            _stricmp(scan.files[1].c_str(), rootSkin.c_str()) != 0) result = 11;
    }
    if (result == 0) {
        const std::string missing = std::string(scanRoot) + "\\missing";
        if (SEScanSkinFolder(missing.c_str()).success) result = 12;
    }

    DeleteFileA(ignored.c_str());
    DeleteFileA(nestedSkin.c_str());
    DeleteFileA(rootSkin.c_str());
    RemoveDirectoryA(nested.c_str());
    RemoveDirectoryA(scanRoot);

    if (result == 0) {
        SkinManage skinData = {};
        if (!InitSkinData(&skinData)) result = 13;
        else {
            skinData.Data[0].skinFile.assign("first.lr2skin");
            skinData.Data[0].customs[0].title.assign("Option");
            skinData.Data[0].customs[0].op_label[0].assign("Choice");
            skinData.Data[0].customs[0].dst_op_count = 1;
            skinData.Data[0].custom_count = 1;
            skinData.Count = 1;
            CSTR* const labelTable = skinData.Data[0].customs[0].op_label;
            if (!ResetSkinData(&skinData)) result = 14;
            else if (skinData.Count != 0 || skinData.Data[0].skinFile.body ||
                skinData.Data[0].customs[0].title.body ||
                skinData.Data[0].customs[0].op_label != labelTable ||
                skinData.Data[0].customs[0].op_label[0].body ||
                skinData.Data[0].customs[0].dst_op_count != 0) result = 15;
        }
    }
    return result;
}

int RunPreviewSimulatorSelfTest() {
    LR2SEPreviewChartNote notes[256] = {};
    int count = LR2SEBuildPreviewChart(SKINTYPE_7KEYS, notes, 256);
    if (count != 180) return 1;
    if (notes[0].lane != 0 || notes[0].timingMs != 2200 ||
        notes[0].kind != LR2SE_PREVIEW_NOTE_NORMAL)
        return 2;

    bool laneSeen[20] = {};
    bool laneHasPrevious[20] = {};
    unsigned int lanePrevious[20] = {};
    int longNotes = 0;
    int mines = 0;
    unsigned int previousTiming = 0;
    for (int index = 0; index < count; ++index) {
        const LR2SEPreviewChartNote& note = notes[index];
        if (note.lane < 0 || note.lane > 7) return 3;
        if (note.timingMs < previousTiming) return 4;
        if (laneHasPrevious[note.lane] && note.timingMs <= lanePrevious[note.lane])
            return 5;
        previousTiming = note.timingMs;
        lanePrevious[note.lane] = note.timingMs;
        laneHasPrevious[note.lane] = true;
        laneSeen[note.lane] = true;
        if (note.kind == LR2SE_PREVIEW_NOTE_LONG) {
            if (note.endTimingMs <= note.timingMs) return 6;
            longNotes++;
        }
        else {
            if (note.endTimingMs != 0) return 7;
            if (note.kind == LR2SE_PREVIEW_NOTE_MINE) mines++;
        }
    }
    for (int lane = 0; lane <= 7; ++lane) {
        if (!laneSeen[lane]) return 8;
    }
    if (longNotes == 0 || mines == 0) return 9;

    count = LR2SEBuildPreviewChart(SKINTYPE_14KEYS, notes, 256);
    if (count != 180) return 10;
    bool firstPlayer = false;
    bool secondPlayer = false;
    for (int index = 0; index < count; ++index) {
        firstPlayer = firstPlayer || (notes[index].lane >= 0 && notes[index].lane <= 7);
        secondPlayer = secondPlayer || (notes[index].lane >= 10 && notes[index].lane <= 17);
    }
    if (!firstPlayer || !secondPlayer) return 11;

    count = LR2SEBuildPreviewChart(SKINTYPE_9KEYS, notes, 256);
    if (count != 180) return 12;
    for (int index = 0; index < count; ++index) {
        if (notes[index].lane < 1 || notes[index].lane > 9) return 13;
    }

    if (LR2SEBuildPreviewChart(SKINTYPE_14KEYS, notes, 5) != 5)
        return 14;
    if (LR2SEBuildPreviewChart(SKINTYPE_7KEYS, nullptr, 256) != 0)
        return 15;

    return 0;
}
