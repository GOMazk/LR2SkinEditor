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

#include <cmath>
#include <filesystem>

int CountCsvColumns(CSTR& line) {
    if (!line.body || line.length() <= 0) return 0;
    int count = 1;
    const char* text = line.outstr();
    for (int i = 0; text[i] != '\0'; ++i) {
        if (text[i] == ',') ++count;
    }
    return count > 30 ? 30 : count;
}

int RunWorkspaceReloadLifecycleSelfTest() {
    WORKSPACE workspace{};
    workspace.olrSourcePackagePath = "stale-source.olrskin";
    if (workspace.ResetEditorDocumentForLoad() != 0) return 1;
    if (!workspace.olrSourcePackagePath.empty()) return 13;

    SKINFILELINEREAD* line =
        (SKINFILELINEREAD*)workspace.skinfileLines.Get_new();
    if (!line) return 2;
    line->filename.assign("first.lr2skin");
    line->line.assign("#SRC_IMAGE,0,0,0,0,32,32,1,1,0");
    SplitCSV(line->line, &line->csv, ",");

    CSTR* includePath = (CSTR*)workspace.arr_subpath.Get_new();
    if (!includePath) return 3;
    includePath->assign("parts\\notes.csv");

    SRCGR* graphic = (SRCGR*)workspace.arr_SRCGR.Get_new();
    if (!graphic) return 4;
    graphic->path.assign("notes.png");
    graphic->filename.assign("notes.png");
    graphic->name.assign("notes");
    graphic->arr_wildcard.Alloc(sizeof(CSTR), 1);
    CSTR* wildcard = (CSTR*)graphic->arr_wildcard.Get_new();
    if (!wildcard) return 5;
    wildcard->assign("notes-red.png");

    SRC* source = (SRC*)workspace.arr_SRC.Get_new();
    DST* destination = (DST*)workspace.arr_DST.Get_new();
    IMG* image = (IMG*)workspace.arr_IMG.Get_new();
    CSTR* customFile = (CSTR*)workspace.arr_CustomFile.Get_new();
    SEOBJ* object = (SEOBJ*)workspace.arr_seobj.Get_new();
    if (!source || !destination || !image || !customFile || !object) return 6;
    source->name.assign("source");
    destination->name.assign("destination");
    destination->arr_animation.Alloc(sizeof(DST_ANIMATION), 1);
    destination->arr_animation.Get_new();
    image->name.assign("crop");
    customFile->assign("custom.png");
    object->name.assign("object");
    object->body.Alloc(sizeof(CSTR), 1);
    ((CSTR*)object->body.Get_new())->assign("#SRC_IMAGE,0");
    object->bodyCSV.Alloc(sizeof(CSVbuf), 1);
    ((CSVbuf*)object->bodyCSV.Get_new())->str[0].assign("#SRC_IMAGE");
    object->srcc.name.assign("object source");
    object->dstt.name.assign("object destination");
    object->dstt.arr_animation.Alloc(sizeof(DST_ANIMATION), 1);
    object->dstt.arr_animation.Get_new();
    workspace.arr_ifunit.Get_new();

    HISTORY* history = (HISTORY*)workspace.arr_history.Get_new();
    if (!history) return 7;
    history->older.line.assign("old");
    history->newer.line.assign("new");

    if (workspace.ResetEditorDocumentForLoad() != 0) return 8;
    if (!workspace.skinfileLines.data || workspace.skinfileLines.count != 0 ||
        !workspace.arr_subpath.data || workspace.arr_subpath.count != 0 ||
        !workspace.arr_SRCGR.data || workspace.arr_SRCGR.count != 0 ||
        !workspace.arr_SRC.data || workspace.arr_SRC.count != 0 ||
        !workspace.arr_DST.data || workspace.arr_DST.count != 0 ||
        !workspace.arr_IMG.data || workspace.arr_IMG.count != 0 ||
        !workspace.arr_CustomFile.data || workspace.arr_CustomFile.count != 0 ||
        !workspace.arr_seobj.data || workspace.arr_seobj.count != 0 ||
        !workspace.arr_history.data || workspace.arr_history.count != 0)
        return 9;

    // A second reset is the regression boundary: loading a new skin must not
    // double-destroy any nested CSTR or retain stale array counts.
    SKINFILELINEREAD* secondLine =
        (SKINFILELINEREAD*)workspace.skinfileLines.Get_new();
    CSTR* secondInclude = (CSTR*)workspace.arr_subpath.Get_new();
    if (!secondLine || !secondInclude) return 10;
    secondLine->line.assign("#IMAGE,second.png");
    secondInclude->assign("parts\\second.csv");
    if (workspace.ResetEditorDocumentForLoad() != 0) return 11;
    if (workspace.skinfileLines.count != 0 || workspace.arr_subpath.count != 0)
        return 12;
    std::string saveError;
    if (workspace.SaveOlrSkin("unused.olrskin", saveError) == 0 ||
        workspace.lastSaveState != -1 ||
        saveError.find("Load a skin") == std::string::npos)
        return 14;
    return 0;
}

int RunWorkspaceRuntimeReloadSmokeTest(const char* firstPath,
    const char* secondPath) {
    if (!firstPath || !*firstPath || !secondPath || !*secondPath) return 1;

    WORKSPACE workspace{};
    if (!InitSkinData(&workspace.g.skinData)) return 2;
    workspace.skinBrowserDataInitialized = true;

    const auto loadSkin = [&](const char* path, int parseError,
        int loadError) -> int {
        ResetSkinData(&workspace.g.skinData);
        ParseLR2SkinCustom(&workspace.g.skinData, CSTR(path));
        if (workspace.g.skinData.Count <= 0) return parseError;
        workspace.meta = workspace.g.skinData.Data[0];
        strncpy_s(workspace.mainpath, path, _TRUNCATE);
        if (workspace.LoadSkin(workspace.mainpath) != 0) return loadError;
        workspace.loaded = true;
        return 0;
    };

    const int firstResult = loadSkin(firstPath, 3, 4);
    if (firstResult != 0) return firstResult;
    return loadSkin(secondPath, 5, 6);
}

int RunWorkspaceRuntimeMultiWorkspaceSmokeTest(const char* firstPath,
    const char* secondPath) {
    if (!firstPath || !*firstPath || !secondPath || !*secondPath) return 1;

    std::unique_ptr<WORKSPACE> firstWorkspace(new WORKSPACE());
    std::unique_ptr<WORKSPACE> secondWorkspace(new WORKSPACE());
    if (!InitSkinData(&firstWorkspace->g.skinData)) return 2;
    if (!InitSkinData(&secondWorkspace->g.skinData)) return 3;
    firstWorkspace->skinBrowserDataInitialized = true;
    secondWorkspace->skinBrowserDataInitialized = true;

    const auto loadSkin = [](WORKSPACE& workspace, const char* path,
        int parseError, int loadError) -> int {
        ResetSkinData(&workspace.g.skinData);
        ParseLR2SkinCustom(&workspace.g.skinData, CSTR(path));
        if (workspace.g.skinData.Count <= 0) return parseError;
        workspace.meta = workspace.g.skinData.Data[0];
        strncpy_s(workspace.mainpath, path, _TRUNCATE);
        if (workspace.LoadSkin(workspace.mainpath) != 0) return loadError;
        workspace.loaded = true;
        return 0;
    };

    const int firstResult = loadSkin(*firstWorkspace, firstPath, 4, 5);
    if (firstResult != 0) return firstResult;
    const int secondResult = loadSkin(*secondWorkspace, secondPath, 6, 7);
    if (secondResult != 0) return secondResult;

    // Start and advance both scene runtimes for several alternating frames.
    // A one-frame draw-only probe did not catch inactive dock tabs returning
    // before the scene tick, which left every background Workspace paused.
    LR2SEResetRenderFault();
    if (LR2SESceneInitSafe(&firstWorkspace->g, firstWorkspace->meta.type,
        LR2SE_PREVIEW_CHART_SIMPLE) != 0)
        return 8;
    if (LR2SESceneInitSafe(&secondWorkspace->g, secondWorkspace->meta.type,
        LR2SE_PREVIEW_CHART_SIMPLE) != 0)
        return 9;
    if (!firstWorkspace->lr2CoreInitialized ||
        !secondWorkspace->lr2CoreInitialized)
        return 8;

    // Both runtimes stay on the UI thread. Alternating them here matches the
    // normal main-loop order without introducing a second LR2/DxLib thread.
    LR2SEResetRenderFault();
    if (LR2SESceneInitSafe(&firstWorkspace->g, firstWorkspace->meta.type,
        LR2SE_PREVIEW_CHART_SIMPLE) != 0)
        return 9;
    if (LR2SESceneInitSafe(&secondWorkspace->g, secondWorkspace->meta.type,
        LR2SE_PREVIEW_CHART_SIMPLE) != 0)
        return 10;
    firstWorkspace->previewSimulationPlaying = true;
    secondWorkspace->previewSimulationPlaying = true;
    firstWorkspace->previewLastRenderAt = 0;
    secondWorkspace->previewLastRenderAt = 0;

    const double firstStart = GetTimeLapse(41, &firstWorkspace->g.timer1);
    const double secondStart = GetTimeLapse(41, &secondWorkspace->g.timer1);
    for (int frame = 0; frame < 4; ++frame) {
        Sleep(20);
        const unsigned long long frameNow = GetTickCount64();
        if (!firstWorkspace->UpdatePreviewRuntime(frameNow)) return 10;
        if (!secondWorkspace->UpdatePreviewRuntime(frameNow)) return 11;
    }
    if (!firstWorkspace->previewSimulationPlaying ||
        !secondWorkspace->previewSimulationPlaying)
        return 12;
    if (GetTimeLapse(41, &firstWorkspace->g.timer1) <= firstStart)
        return 13;
    if (GetTimeLapse(41, &secondWorkspace->g.timer1) <= secondStart)
        return 14;
    return 0;
}

int RunSimpleModeProjectionSelfTest() {
    if (arr_CommandHelp.count <= 0 && LoadCommandHelp(nullptr) != 0) return 1;

    WORKSPACE workspace;
    workspace.skinfileLines.Alloc(sizeof(SKINFILELINEREAD), 24);
    workspace.arr_IMG.Alloc(sizeof(IMG), 4);
    workspace.arr_SRCGR.Alloc(sizeof(SRCGR), 4);

    const char* sourceRows[] = {
        "#SRC_NUMBER,0,0,0,0,100,20,10,1,0,0,101,0,4",
        "#SRC_NOWCOMBO_1P,0,0,0,0,100,20,10,1,0,0,0,0,4",
        "#SRC_NOWCOMBO_2P,0,0,0,0,100,20,10,1,0,0,0,0,4",
        "#SRC_NOWJUDGE_1P,0,0,0,0,100,20,1,1,0,0,0,0,0",
        "#SRC_NOWJUDGE_2P,0,0,0,0,100,20,1,1,0,0,0,0,0",
        "#SRC_LINE,0,0,0,0,1,1,1,1,0,0,0,0,0",
        "#SRC_JUDGELINE,0,0,0,0,1,1,1,1,0,0,0,0,0",
        "#SRC_NOTE,0,0,0,0,20,10,1,1,0,0,0,0,0",
        "#SRC_MINE,0,0,0,0,20,10,1,1,0,0,0,0,0",
        "#SRC_LN_START,0,0,0,0,20,10,1,1,0",
        "#SRC_LN_BODY,0,0,0,0,20,10,1,1,0",
        "#SRC_LN_END,0,0,0,0,20,10,1,1,0",
        "#SRC_AUTO_NOTE,0,0,0,0,20,10,1,1,0,0,0,0,0",
        "#SRC_AUTO_MINE,0,0,0,0,20,10,1,1,0,0,0,0,0",
        "#SRC_AUTO_LN_START,0,0,0,0,20,10,1,1,0,0,0,0,0",
        "#SRC_AUTO_LN_BODY,0,0,0,0,20,10,1,1,0,0,0,0,0",
        "#SRC_AUTO_LN_END,0,0,0,0,20,10,1,1,0,0,0,0,0",
        "#SRC_GROOVEGAUGE,0,0,0,0,20,10,1,1,0,0,0,0",
        "#SRC_GAUGECHART_1P,0,0,0,0,20,10,1,1,0,0,0,0,0,0",
        "#SRC_GAUGECHART_2P,0,0,0,0,20,10,1,1,0,0,0,0,0,0",
        "#SRC_SCORECHART,0,0,0,0,20,10,1,1,0,0,0,0,0,0",
        "#SRC_IMAGE,0,0,0,0,20,10,1,1,0,0,0,0,0"
    };
    for (const char* text : sourceRows) {
        SKINFILELINEREAD* row =
            (SKINFILELINEREAD*)workspace.skinfileLines.Get_new();
        row->line.assign(text);
        row->numTotal = workspace.skinfileLines.count - 1;
        row->num = row->numTotal + 1;
        SplitCSV(row->line, &row->csv, ",");
        row->csvColumnCount = CountCsvColumns(row->line);
    }

    // No Object Editor groups are loaded on purpose. Simple Mode must project
    // the LR2 source contract directly instead of silently becoming empty.
    const SESimpleModeCategoryCounts counts =
        workspace.GetSimpleModeCategoryCounts();
    if (counts.numberFonts != 3) return 2;
    if (counts.judgementFonts != 2) return 3;
    if (counts.gear != 2) return 4;
    if (counts.notes != 10) return 5;
    if (counts.gauge != 4) return 6;
    if (workspace.simpleModeProjectionGeneration != 1) return 7;

    // Reading an unchanged Workspace again must reuse the projection instead
    // of rescanning every source row as the Simple Mode window renders.
    const SESimpleModeCategoryCounts cachedCounts =
        workspace.GetSimpleModeCategoryCounts();
    if (cachedCounts.notes != counts.notes ||
        workspace.simpleModeProjectionGeneration != 1) return 8;

    ((SKINFILELINEREAD*)workspace.skinfileLines.data)[21]
        .csv.str[0].assign("#SRC_LINE");
    workspace.NotifyDocumentChanged(DOCUMENT_CHANGE_STRUCTURE);
    const SESimpleModeCategoryCounts refreshedCounts =
        workspace.GetSimpleModeCategoryCounts();
    if (refreshedCounts.gear != 3 ||
        workspace.simpleModeProjectionGeneration != 2) return 9;

    const int scopeResult = RunSimpleModeScopeRuleSelfTest();
    return scopeResult == 0 ? 0 : 20 + scopeResult;
}


int RunObjectReorderSelfTest() {
    if (arr_CommandHelp.count <= 0 && LoadCommandHelp(nullptr) != 0) return 66;
    char tempDirectory[MAX_PATH] = {};
    if (!GetTempPathA(MAX_PATH, tempDirectory)) return 1;
    char ownerPath[MAX_PATH] = {};
    char otherOwnerPath[MAX_PATH] = {};
    snprintf(ownerPath, sizeof(ownerPath), "%sSkinEditor_reorder_%lu.lr2skin",
        tempDirectory, GetCurrentProcessId());
    snprintf(otherOwnerPath, sizeof(otherOwnerPath),
        "%sSkinEditor_reorder_other_%lu.lr2skin", tempDirectory,
        GetCurrentProcessId());

    WORKSPACE workspace;
    workspace.skinfileLines.Alloc(sizeof(SKINFILELINEREAD), 32);
    workspace.arr_CustomFile.Alloc(sizeof(CSTR), 2);
    workspace.arr_SRCGR.Alloc(sizeof(SRCGR), 2);
    workspace.arr_IMG.Alloc(sizeof(IMG), 4);
    workspace.arr_SRC.Alloc(sizeof(SRC), 4);
    workspace.arr_DST.Alloc(sizeof(DST), 4);
    workspace.arr_seobj.Alloc(sizeof(SEOBJ), 4);
    workspace.arr_ifunit.Alloc(sizeof(IFUNIT), 4);
    workspace.arr_history.Alloc(sizeof(HISTORY), 4);
    strncpy_s(workspace.mainpath, ownerPath, _TRUNCATE);

    auto appendLine = [&](const char* text, const char* owner) {
        SKINFILELINEREAD* line = (SKINFILELINEREAD*)
            workspace.skinfileLines.Get_new();
        line->line.assign(text);
        line->filename.assign(owner);
        line->numTotal = workspace.skinfileLines.count - 1;
        line->num = line->numTotal;
        line->isComment = text[0] != '#';
        line->isSEcomment = text[0] == '$';
        SplitCSV(line->line, &line->csv, ",");
        line->csvColumnCount = CountCsvColumns(line->line);
    };

    const std::string fileStart = std::string("$FILE '") + ownerPath +
        "' start";
    const std::string fileEnd = std::string("$FILE '") + ownerPath +
        "' end";
    const std::string includeStart = std::string("$FILE '") + otherOwnerPath +
        "' start";
    const std::string includeEnd = std::string("$FILE '") + otherOwnerPath +
        "' end";
    appendLine(fileStart.c_str(), ownerPath);
    appendLine("#IF,900", ownerPath);
    appendLine("$SE_OBJECT_ID,left-object", ownerPath);
    appendLine("$SE_OBJECT_NAME,Left Object", ownerPath);
    appendLine("#SRC_IMAGE,0,0,0,0,1,1,1,1,0,0,0,0,0", ownerPath);
    appendLine("#DST_IMAGE,0,0,1,2,1,1,0,255,255,255,255,0,0,0,0,0,0,0,0,0", ownerPath);
    appendLine("#ELSE", ownerPath);
    appendLine("$SE_OBJECT_ID,right-object", ownerPath);
    appendLine("$SE_OBJECT_NAME,Right Object", ownerPath);
    appendLine("#SRC_IMAGE,0,0,0,0,1,1,1,1,0,0,0,0,0", ownerPath);
    appendLine("#DST_IMAGE,0,0,3,4,1,1,0,255,255,255,255,0,0,0,0,0,0,0,0,0", ownerPath);
    appendLine("#ENDIF", ownerPath);
    appendLine("$SE_OBJECT_ID,front-number", ownerPath);
    appendLine("$SE_OBJECT_NAME,Front Number", ownerPath);
    appendLine("#SRC_NUMBER,0,0,0,0,100,20,10,1,0,0,101,0,4", ownerPath);
    appendLine("#DST_NUMBER,0,0,7,8,10,20,0,255,255,255,255,0,0,0,0,0,0,0,0,0", ownerPath);
    appendLine("$SE_OBJECT_ID,front-text", ownerPath);
    appendLine("$SE_OBJECT_NAME,Front Text", ownerPath);
    appendLine("#SRC_TEXT,0,0,10,0,0,0", ownerPath);
    appendLine("#DST_TEXT,0,0,100,30,40,20,0,255,255,255,255,0,0,0,0,0,0,0,0,0", ownerPath);
    appendLine("$SE_OBJECT_ID,front-groove-gauge", ownerPath);
    appendLine("$SE_OBJECT_NAME,Front Groove Gauge", ownerPath);
    appendLine("#SRC_GROOVEGAUGE,0,0,0,0,40,16,4,1,0,0,5,-2", ownerPath);
    appendLine("#DST_GROOVEGAUGE,0,0,100,200,10,4,0,255,255,255,255,0,0,0,0,0,0,0,0,0", ownerPath);
    appendLine("$SE_OBJECT_ID,front-bga", ownerPath);
    appendLine("$SE_OBJECT_NAME,Front BGA", ownerPath);
    appendLine("#SRC_BGA,0,0,0,0,0,0,0,0,0,0,0,0,0", ownerPath);
    appendLine("#DST_BGA,0,0,11,12,320,180,0,255,255,255,255,0,0,0,0,0,0,0,0,0", ownerPath);
    appendLine("$SE_OBJECT_ID,combo-original", ownerPath);
    appendLine("#SRC_NOWCOMBO_1P,5,0,0,0,80,16,10,1,0,46,0,1,7", ownerPath);
    appendLine("#DST_NOWCOMBO_1P,5,0,10,20,8,16,0,255,255,255,255,1,0,0,0,0,46,0,0,0", ownerPath);
    appendLine("#DST_NOWCOMBO_1P,5,500,10,20,8,16,0,255,255,255,255,1,0,0,0,0,46,0,0,0", ownerPath);
    appendLine("$SE_OBJECT_ID,combo-duplicate", ownerPath);
    appendLine("#SRC_NOWCOMBO_1P,5,0,0,0,80,16,10,1,0,46,0,1,7", ownerPath);
    appendLine("#DST_NOWCOMBO_1P,5,0,30,40,8,16,0,255,255,255,255,1,0,0,0,0,46,0,0,0", ownerPath);
    appendLine("#DST_NOWCOMBO_1P,5,500,30,40,8,16,0,255,255,255,255,1,0,0,0,0,46,0,0,0", ownerPath);
    // Older duplicate behavior could copy an already merged Object, leaving
    // repeated SRC/DST bundles beneath one ID. Rebuild must split those runs
    // instead of presenting every SRC in one Inspector Object.
    appendLine("$SE_OBJECT_ID,combo-packed-copy", ownerPath);
    appendLine("#SRC_NOWCOMBO_1P,5,0,0,0,80,16,10,1,0,46,0,1,7", ownerPath);
    appendLine("#DST_NOWCOMBO_1P,5,0,50,60,8,16,0,255,255,255,255,1,0,0,0,0,46,0,0,0", ownerPath);
    appendLine("#DST_NOWCOMBO_1P,5,500,50,60,8,16,0,255,255,255,255,1,0,0,0,0,46,0,0,0", ownerPath);
    appendLine("#SRC_NOWCOMBO_1P,5,0,0,0,80,16,10,1,0,46,0,1,7", ownerPath);
    appendLine("#DST_NOWCOMBO_1P,5,0,70,80,8,16,0,255,255,255,255,1,0,0,0,0,46,0,0,0", ownerPath);
    appendLine("#DST_NOWCOMBO_1P,5,500,70,80,8,16,0,255,255,255,255,1,0,0,0,0,46,0,0,0", ownerPath);
    appendLine(includeStart.c_str(), otherOwnerPath);
    appendLine("#IF,901", otherOwnerPath);
    appendLine("$SE_OBJECT_ID,include-object", otherOwnerPath);
    appendLine("$SE_OBJECT_NAME,Include Object", otherOwnerPath);
    appendLine("#SRC_IMAGE,0,0,0,0,1,1,1,1,0,0,0,0,0", otherOwnerPath);
    appendLine("#DST_IMAGE,0,0,5,6,1,1,0,255,255,255,255,0,0,0,0,0,0,0,0,0", otherOwnerPath);
    appendLine("#ENDIF", otherOwnerPath);
    appendLine(includeEnd.c_str(), otherOwnerPath);
    appendLine(fileEnd.c_str(), ownerPath);

    if (workspace.RebuildEditorDerivedState() != 0) return 2;
    if (!workspace.objectEditorModel.LoadGroups(nullptr)) return 3;
    workspace.RebuildObjectModel();

    auto findObject = [&](const char* editorId) -> int {
        const std::vector<SEObjectInstance>& objects =
            workspace.objectEditorModel.Objects();
        for (int index = 0; index < (int)objects.size(); ++index) {
            if (objects[index].editorId == editorId) return index;
        }
        return -1;
    };

    int source = findObject("left-object");
    int target = findObject("right-object");
    const int frontNumber = findObject("front-number");
    const int frontText = findObject("front-text");
    const int frontGrooveGauge = findObject("front-groove-gauge");
    const int frontBga = findObject("front-bga");
    if (source < 0 || target < 0 || frontNumber < 0 || frontText < 0 ||
        frontGrooveGauge < 0 || frontBga < 0)
        return 4;
    const int originalCombo = findObject("combo-original");
    const int duplicatedCombo = findObject("combo-duplicate");
    if (originalCombo < 0 || duplicatedCombo < 0 ||
        originalCombo == duplicatedCombo) return 45;
    const std::vector<SEObjectInstance>& duplicateObjects =
        workspace.objectEditorModel.Objects();
    if (duplicateObjects[originalCombo].rows.size() != 3 ||
        duplicateObjects[duplicatedCombo].rows.size() != 3)
        return 46;
    for (int modelIndex : { originalCombo, duplicatedCombo }) {
        int sourceCount = 0;
        int destinationCount = 0;
        for (int row : duplicateObjects[modelIndex].rows) {
            const SKINFILELINEREAD& line =
                ((const SKINFILELINEREAD*)workspace.skinfileLines.data)[row];
            const char* command = line.csv.str[0].body
                ? line.csv.str[0].body : "";
            if (strcmp(command, "#SRC_NOWCOMBO_1P") == 0)
                ++sourceCount;
            else if (strcmp(command, "#DST_NOWCOMBO_1P") == 0)
                ++destinationCount;
        }
        if (sourceCount != 1 || destinationCount != 2) return 47;
    }
    int nowComboObjectCount = 0;
    for (const SEObjectInstance& object : duplicateObjects) {
        int sourceCount = 0;
        int destinationCount = 0;
        for (int row : object.rows) {
            const SKINFILELINEREAD& line =
                ((const SKINFILELINEREAD*)workspace.skinfileLines.data)[row];
            const char* command = line.csv.str[0].body
                ? line.csv.str[0].body : "";
            if (strcmp(command, "#SRC_NOWCOMBO_1P") == 0)
                ++sourceCount;
            else if (strcmp(command, "#DST_NOWCOMBO_1P") == 0)
                ++destinationCount;
        }
        if (sourceCount == 0 && destinationCount == 0) continue;
        ++nowComboObjectCount;
        if (sourceCount != 1 || destinationCount != 2) return 48;
    }
    if (nowComboObjectCount != 4) return 49;

    // Editing the shared key of an indexed Object must keep its SRC and every
    // DST animation row together. This is especially important after Clone:
    // the duplicate deliberately keeps the same LR2 index but has a distinct
    // editor ID, so a one-line edit would split it on the next model rebuild.
    workspace.SetObjectSelection(std::vector<int>(1, duplicatedCombo),
        duplicatedCombo, duplicatedCombo, false);
    int duplicatedComboSourceRow = -1;
    for (int row : duplicateObjects[duplicatedCombo].rows) {
        const SKINFILELINEREAD& line =
            ((const SKINFILELINEREAD*)workspace.skinfileLines.data)[row];
        if (line.csv.str[0].body &&
            strcmp(line.csv.str[0].body, "#SRC_NOWCOMBO_1P") == 0) {
            duplicatedComboSourceRow = row;
            break;
        }
    }
    if (duplicatedComboSourceRow < 0 ||
        workspace.EditValue(duplicatedComboSourceRow, 1, "3") != 0)
        return 67;
    if (workspace.arr_history.count != 4) return 68;
    workspace.RebuildObjectModel();
    int editedCombo = findObject("combo-duplicate");
    if (editedCombo < 0 ||
        workspace.objectEditorModel.Objects()[editedCombo].rows.size() != 3 ||
        workspace.ResolveObjectSelectionKey(workspace.objectSelection.active) !=
            editedCombo)
        return 69;
    for (int row : workspace.objectEditorModel.Objects()[editedCombo].rows) {
        const SKINFILELINEREAD& line =
            ((const SKINFILELINEREAD*)workspace.skinfileLines.data)[row];
        if (line.csv.val[1] != 3) return 70;
    }
    if (workspace.UndoLastEdit() != 0) return 71;
    workspace.RebuildObjectModel();
    editedCombo = findObject("combo-duplicate");
    if (editedCombo < 0 ||
        workspace.objectEditorModel.Objects()[editedCombo].rows.size() != 3 ||
        workspace.arr_history.count != 0)
        return 72;
    for (int row : workspace.objectEditorModel.Objects()[editedCombo].rows) {
        const SKINFILELINEREAD& line =
            ((const SKINFILELINEREAD*)workspace.skinfileLines.data)[row];
        if (line.csv.val[1] != 5) return 73;
    }

    // Simulate a file saved by the old one-line editor and make sure the next
    // SRC index edit also picks up its physically adjacent orphaned DST run.
    SKINFILELINEREAD& legacySplitSource =
        ((SKINFILELINEREAD*)workspace.skinfileLines.data)
            [duplicatedComboSourceRow];
    legacySplitSource.csv.str[1].assign("3");
    legacySplitSource.csv.val[1] = 3;
    workspace.CsvToLine(duplicatedComboSourceRow);
    workspace.RebuildObjectModel();
    editedCombo = findObject("combo-duplicate");
    if (editedCombo < 0 ||
        workspace.objectEditorModel.Objects()[editedCombo].rows.size() != 1)
        return 74;
    if (workspace.EditValue(duplicatedComboSourceRow, 1, "4") != 0 ||
        workspace.arr_history.count != 4)
        return 75;
    workspace.RebuildObjectModel();
    editedCombo = findObject("combo-duplicate");
    if (editedCombo < 0 ||
        workspace.objectEditorModel.Objects()[editedCombo].rows.size() != 3)
        return 76;
    for (int row : workspace.objectEditorModel.Objects()[editedCombo].rows) {
        const SKINFILELINEREAD& line =
            ((const SKINFILELINEREAD*)workspace.skinfileLines.data)[row];
        if (line.csv.val[1] != 4) return 77;
    }
    if (workspace.UndoLastEdit() != 0 || workspace.arr_history.count != 0)
        return 78;
    legacySplitSource.csv.str[1].assign("5");
    legacySplitSource.csv.val[1] = 5;
    workspace.CsvToLine(duplicatedComboSourceRow);
    workspace.RebuildObjectModel();
    editedCombo = findObject("combo-duplicate");
    if (editedCombo < 0 ||
        workspace.objectEditorModel.Objects()[editedCombo].rows.size() != 3)
        return 79;

    const std::vector<SEObjectInstance>& initialObjects =
        workspace.objectEditorModel.Objects();
    if (initialObjects[source].drawOrder < 0 ||
        initialObjects[target].drawOrder <= initialObjects[source].drawOrder ||
        initialObjects[frontNumber].drawOrder <= initialObjects[target].drawOrder ||
        initialObjects[source].firstDstRow >= initialObjects[target].firstDstRow ||
        initialObjects[target].firstDstRow >= initialObjects[frontNumber].firstDstRow)
        return 43;
    if (!(source < target && target < frontNumber)) return 44;
    if (initialObjects[source].ifgroup == initialObjects[target].ifgroup)
        return 5;
    DST_ANIMATION numberFrame = {};
    numberFrame.x = 7.0f;
    numberFrame.y = 8.0f;
    numberFrame.w = 10.0f;
    numberFrame.h = 20.0f;
    float numberX = 0.0f, numberY = 0.0f;
    float numberWidth = 0.0f, numberHeight = 0.0f;
    workspace.ResolvePreviewObjectFrameBounds(initialObjects[frontNumber],
        numberFrame, numberX, numberY, numberWidth, numberHeight);
    if (std::abs(numberX - 7.0f) >= 0.5f ||
        std::abs(numberY - 8.0f) >= 0.5f ||
        std::abs(numberWidth - 40.0f) >= 0.5f ||
        std::abs(numberHeight - 20.0f) >= 0.5f)
        return 50;

    int textSourceRow = -1;
    for (int row : initialObjects[frontText].rows) {
        SKINFILELINEREAD& line =
            ((SKINFILELINEREAD*)workspace.skinfileLines.data)[row];
        if (line.csv.str[0].isSame("#SRC_TEXT")) {
            textSourceRow = row;
            break;
        }
    }
    if (textSourceRow < 0) return 51;
    DST_ANIMATION textFrame = {};
    textFrame.x = 100.0f;
    textFrame.y = 30.0f;
    textFrame.w = 40.0f;
    textFrame.h = 20.0f;
    const float expectedTextX[] = { 100.0f, 80.0f, 60.0f };
    for (int align = 0; align < 3; ++align) {
        ((SKINFILELINEREAD*)workspace.skinfileLines.data)[textSourceRow]
            .csv.val[4] = align;
        float textX = 0.0f, textY = 0.0f;
        float textWidth = 0.0f, textHeight = 0.0f;
        workspace.ResolvePreviewObjectFrameBounds(initialObjects[frontText],
            textFrame, textX, textY, textWidth, textHeight);
        if (std::abs(textX - expectedTextX[align]) >= 0.5f ||
            std::abs(textY - 30.0f) >= 0.5f ||
            std::abs(textWidth - 40.0f) >= 0.5f ||
            std::abs(textHeight - 20.0f) >= 0.5f)
            return 52 + align;
    }
    ((SKINFILELINEREAD*)workspace.skinfileLines.data)[textSourceRow]
        .csv.val[4] = 0;
    DST_ANIMATION grooveGaugeFrame = {};
    grooveGaugeFrame.x = 100.0f;
    grooveGaugeFrame.y = 200.0f;
    grooveGaugeFrame.w = 10.0f;
    grooveGaugeFrame.h = 4.0f;
    float grooveGaugeX = 0.0f, grooveGaugeY = 0.0f;
    float grooveGaugeWidth = 0.0f, grooveGaugeHeight = 0.0f;
    workspace.ResolvePreviewObjectFrameBounds(
        initialObjects[frontGrooveGauge], grooveGaugeFrame,
        grooveGaugeX, grooveGaugeY, grooveGaugeWidth, grooveGaugeHeight);
    if (std::abs(grooveGaugeX - 100.0f) >= 0.5f ||
        std::abs(grooveGaugeY - 102.0f) >= 0.5f ||
        std::abs(grooveGaugeWidth - 255.0f) >= 0.5f ||
        std::abs(grooveGaugeHeight - 102.0f) >= 0.5f)
        return 55;
    std::vector<SEPreviewObjectDestination> grooveDestinations;
    workspace.CollectPreviewObjectDestinations(
        initialObjects[frontGrooveGauge], grooveDestinations);
    if (grooveDestinations.size() != 1 ||
        grooveDestinations[0].lastRow < 0 ||
        std::abs(grooveDestinations[0].frame.x - 100.0f) >= 0.5f ||
        std::abs(grooveDestinations[0].frame.y - 200.0f) >= 0.5f)
        return 56;
    std::vector<SEPreviewObjectDestination> bgaDestinations;
    workspace.CollectPreviewObjectDestinations(
        initialObjects[frontBga], bgaDestinations);
    if (bgaDestinations.size() != 1 ||
        bgaDestinations[0].lastRow < 0 ||
        std::abs(bgaDestinations[0].frame.x - 11.0f) >= 0.5f ||
        std::abs(bgaDestinations[0].frame.y - 12.0f) >= 0.5f ||
        std::abs(bgaDestinations[0].frame.w - 320.0f) >= 0.5f ||
        std::abs(bgaDestinations[0].frame.h - 180.0f) >= 0.5f)
        return 57;
    for (int dstIndex = 0; dstIndex < workspace.arr_DST.count; ++dstIndex) {
        const DST& cachedDestination =
            ((const DST*)workspace.arr_DST.data)[dstIndex];
        if (cachedDestination.declare == bgaDestinations[0].lastRow)
            return 58;
    }
    if (!workspace.CanReorderObject(source, target)) return 6;

    if (!workspace.QueueObjectReorder(source, target, true) ||
        workspace.ApplyPendingObjectReorder() != 0) return 7;
    if (workspace.arr_history.count != 1) return 8;
    if (workspace.RebuildEditorDerivedState() != 0) return 9;
    workspace.RebuildObjectModel();
    source = findObject("left-object");
    target = findObject("right-object");
    if (source < 0 || target < 0) return 10;
    const std::vector<SEObjectInstance>& movedObjects =
        workspace.objectEditorModel.Objects();
    if (movedObjects[source].ifgroup != movedObjects[target].ifgroup ||
        movedObjects[source].branchHeaderRow !=
            movedObjects[target].branchHeaderRow) return 11;

    if (workspace.UndoLastEdit() != 0 ||
        workspace.pendingHistorySnapshotRestore < 0) return 12;
    if (workspace.ApplyPendingHistorySnapshotRestore() != 0) return 13;
    if (workspace.RebuildEditorDerivedState() != 0) return 14;
    workspace.RebuildObjectModel();
    source = findObject("left-object");
    target = findObject("right-object");
    if (source < 0 || target < 0) return 15;
    const std::vector<SEObjectInstance>& restoredObjects =
        workspace.objectEditorModel.Objects();
    if (restoredObjects[source].ifgroup == restoredObjects[target].ifgroup)
        return 16;

    // Redo restores the complete forward snapshot and creates one matching
    // undo entry. This covers snapshot-backed Object moves as well as the
    // primitive line-edit history path used by Inspector and Preview.
    if (workspace.RedoLastEdit() != 0 ||
        workspace.ApplyPendingHistorySnapshotRestore() != 0) return 43;
    if (workspace.RebuildEditorDerivedState() != 0) return 44;
    workspace.RebuildObjectModel();
    source = findObject("left-object");
    target = findObject("right-object");
    if (source < 0 || target < 0 ||
        workspace.objectEditorModel.Objects()[source].ifgroup !=
            workspace.objectEditorModel.Objects()[target].ifgroup) return 45;
    if (workspace.UndoLastEdit() != 0 ||
        workspace.ApplyPendingHistorySnapshotRestore() != 0) return 46;
    if (workspace.RebuildEditorDerivedState() != 0) return 47;
    workspace.RebuildObjectModel();
    source = findObject("left-object");
    target = findObject("right-object");
    if (source < 0 || target < 0 ||
        workspace.objectEditorModel.Objects()[source].ifgroup ==
            workspace.objectEditorModel.Objects()[target].ifgroup) return 48;

    target = findObject("include-object");
    if (source < 0 || target < 0) return 17;
    if (!workspace.CanReorderObject(source, target)) return 18;
    if (!workspace.ObjectReorderRequiresConfirmation(source, target)) return 19;
    const unsigned long long revisionBeforeCancel = workspace.documentRevision;
    if (!workspace.QueueObjectReorder(source, target, true)) return 20;
    if (!workspace.objectReorderConfirmationPending ||
        workspace.pendingObjectReorder) return 21;
    workspace.CancelPendingObjectReorder();
    if (workspace.objectReorderConfirmationPending ||
        workspace.pendingObjectReorder ||
        workspace.documentRevision != revisionBeforeCancel) return 22;
    if (!workspace.QueueObjectReorder(source, target, true)) return 23;
    if (!workspace.ConfirmPendingObjectReorder() ||
        !workspace.pendingObjectReorder) return 24;
    if (workspace.ApplyPendingObjectReorder() != 0) return 25;
    if (workspace.RebuildEditorDerivedState() != 0) return 26;
    workspace.RebuildObjectModel();
    source = findObject("left-object");
    target = findObject("include-object");
    if (source < 0 || target < 0) return 27;
    const std::vector<SEObjectInstance>& crossFileObjects =
        workspace.objectEditorModel.Objects();
    if (crossFileObjects[source].ifgroup != crossFileObjects[target].ifgroup ||
        crossFileObjects[source].branchHeaderRow !=
            crossFileObjects[target].branchHeaderRow) return 28;
    for (int row : crossFileObjects[source].rows) {
        const SKINFILELINEREAD& line =
            ((const SKINFILELINEREAD*)workspace.skinfileLines.data)[row];
        if (!line.filename.body ||
            _stricmp(line.filename.body, otherOwnerPath) != 0) return 29;
    }
    bool movedIdOwner = false;
    bool movedNameOwner = false;
    for (int row = 0; row < workspace.skinfileLines.count; ++row) {
        const SKINFILELINEREAD& line =
            ((const SKINFILELINEREAD*)workspace.skinfileLines.data)[row];
        const char* text = line.line.body ? line.line.body : "";
        if (strcmp(text, "$SE_OBJECT_ID,left-object") == 0)
            movedIdOwner = line.filename.body &&
                _stricmp(line.filename.body, otherOwnerPath) == 0;
        if (strcmp(text, "$SE_OBJECT_NAME,Left Object") == 0)
            movedNameOwner = line.filename.body &&
                _stricmp(line.filename.body, otherOwnerPath) == 0;
    }
    if (!movedIdOwner || !movedNameOwner) return 30;

    // A legacy arr_DST cache may accidentally contain a following Object's
    // frame after a cross-file reorder. Preview bounds must still come only
    // from the selected Object's CSV rows, matching Inspector's DST count.
    DST* pollutedDestination = NULL;
    for (int dstIndex = 0; dstIndex < workspace.arr_DST.count; ++dstIndex) {
        DST& candidate = ((DST*)workspace.arr_DST.data)[dstIndex];
        if (std::find(crossFileObjects[source].rows.begin(),
                crossFileObjects[source].rows.end(), candidate.declare) !=
            crossFileObjects[source].rows.end()) {
            pollutedDestination = &candidate;
            break;
        }
    }
    if (!pollutedDestination) return 40;
    DST_ANIMATION* foreignFrame =
        (DST_ANIMATION*)pollutedDestination->arr_animation.Get_new();
    foreignFrame->time = 1000;
    foreignFrame->x = 900;
    foreignFrame->y = 901;
    foreignFrame->w = 20;
    foreignFrame->h = 21;
    workspace.SetObjectSelection(std::vector<int>(1, source), source, source,
        false);
    if (workspace.RefreshPreviewSelectionBounds() != 0) return 41;
    if (std::abs(workspace.preview_selected_obj.x - 1.0f) >= 0.5f ||
        std::abs(workspace.preview_selected_obj.y - 2.0f) >= 0.5f ||
        std::abs(workspace.preview_selected_obj.w - 1.0f) >= 0.5f ||
        std::abs(workspace.preview_selected_obj.h - 1.0f) >= 0.5f ||
        std::abs(workspace.preview_selected_obj_last.x - 1.0f) >= 0.5f ||
        std::abs(workspace.preview_selected_obj_last.y - 2.0f) >= 0.5f)
        return 42;

    if (workspace.UndoLastEdit() != 0 ||
        workspace.pendingHistorySnapshotRestore < 0) return 31;
    if (workspace.ApplyPendingHistorySnapshotRestore() != 0) return 32;
    if (workspace.RebuildEditorDerivedState() != 0) return 33;
    workspace.RebuildObjectModel();
    source = findObject("left-object");
    target = findObject("include-object");
    if (source < 0 || target < 0) return 34;
    const std::vector<SEObjectInstance>& crossRestoredObjects =
        workspace.objectEditorModel.Objects();
    if (crossRestoredObjects[source].ifgroup ==
        crossRestoredObjects[target].ifgroup) return 35;
    for (int row : crossRestoredObjects[source].rows) {
        const SKINFILELINEREAD& line =
            ((const SKINFILELINEREAD*)workspace.skinfileLines.data)[row];
        if (!line.filename.body ||
            _stricmp(line.filename.body, ownerPath) != 0) return 36;
    }

    const int objectCountBeforeCopy =
        (int)workspace.objectEditorModel.Objects().size();
    workspace.SetObjectSelection(std::vector<int>(1, source), source, source,
        false);
    if (workspace.CopySelectedObjects() != 1 ||
        !workspace.HasCopiedObjects()) return 50;
    if (workspace.PasteCopiedObjects() != 1) return 51;
    if ((int)workspace.objectEditorModel.Objects().size() !=
        objectCountBeforeCopy + 1) return 52;
    const int pastedModel = workspace.preview_selected_object_model_index;
    if (pastedModel < 0 ||
        workspace.objectEditorModel.Objects()[pastedModel].editorId.empty() ||
        workspace.objectEditorModel.Objects()[pastedModel].editorId ==
            "left-object") return 53;
    if (workspace.UndoLastEdit() != 0 ||
        workspace.ApplyPendingHistorySnapshotRestore() != 0) return 54;
    if (workspace.RebuildEditorDerivedState() != 0) return 55;
    workspace.RebuildObjectModel();
    if ((int)workspace.objectEditorModel.Objects().size() !=
        objectCountBeforeCopy) return 56;
    if (workspace.RedoLastEdit() != 0 ||
        workspace.ApplyPendingHistorySnapshotRestore() != 0) return 57;
    if (workspace.RebuildEditorDerivedState() != 0) return 58;
    workspace.RebuildObjectModel();
    if ((int)workspace.objectEditorModel.Objects().size() !=
        objectCountBeforeCopy + 1) return 59;

    // A normal edit after Undo must discard the forward branch.
    if (workspace.UndoLastEdit() != 0 ||
        workspace.ApplyPendingHistorySnapshotRestore() != 0) return 60;
    if (workspace.RebuildEditorDerivedState() != 0) return 61;
    workspace.RebuildObjectModel();
    source = findObject("left-object");
    if (source < 0 || workspace.objectEditorModel.Objects()[source].rows.empty())
        return 62;
    const int sourceRow = workspace.objectEditorModel.Objects()[source].rows[0];
    if (workspace.EditValue(sourceRow, 2,
        ((SKINFILELINEREAD*)workspace.skinfileLines.data)[sourceRow]
            .csv.val[2] + 1) != 0) return 63;
    if (workspace.RedoLastEdit() == 0) return 64;
    workspace.SetObjectSelection(std::vector<int>(1, source), source, source,
        false);
    if (workspace.DuplicateSelectedObjects() != 1 ||
        (int)workspace.objectEditorModel.Objects().size() !=
            objectCountBeforeCopy + 1) return 65;
    return 0;
}
int RunAssetMetadataSelfTest() {
    const int gifLayoutResult = RunGifSpriteLayoutSelfTest();
    if (gifLayoutResult != 0) return 105 + gifLayoutResult;
    if (arr_CommandHelp.count <= 0 &&
        LoadCommandHelp("..\\skinHelper.txt") != 0) return 9;
    const char* directAssetDropCommands[] = {
        "#SRC_IMAGE", "#SRC_NUMBER", "#SRC_SLIDER", "#SRC_BUTTON",
        "#SRC_BARGRAPH", "#SRC_ONMOUSE", "#SRC_MOUSECURSOR",
        "#SRC_BAR_FLASH", "#SRC_BAR_LEVEL", "#SRC_BAR_LAMP",
        "#SRC_BAR_MY_LAMP", "#SRC_BAR_RIVAL_LAMP", "#SRC_BAR_RANK",
        "#SRC_BAR_RIVAL", "#SRC_LINE", "#SRC_JUDGELINE",
        "#SRC_NOWJUDGE_1P", "#SRC_NOWCOMBO_1P", "#SRC_NOWJUDGE_2P",
        "#SRC_NOWCOMBO_2P", "#SRC_GROOVEGAUGE", "#SRC_GAUGECHART_1P",
        "#SRC_GAUGECHART_2P", "#SRC_SCORECHART"
    };
    for (const char* command : directAssetDropCommands)
        if (!SEIsDirectAssetDropObjectCommand(command)) return 130;
    const char* deferredAssetDropCommands[] = {
        "#SRC_TEXT", "#SRC_BAR_BODY", "#SRC_EVENT_MODE_CURSOR",
        "#SRC_NOTE", "#SRC_MINE", "#SRC_LN_START", "#SRC_AUTO_NOTE",
        "#SRC_BGA", "#SRC_MASK"
    };
    for (const char* command : deferredAssetDropCommands)
        if (SEIsDirectAssetDropObjectCommand(command)) return 131;

    // Exercise the Image Manager failure sequence: delete a non-last crop,
    // then reuse the vacant tail for a new crop. ARR byte-relocates records,
    // so the survivor and the new entry must still own distinct CSTR buffers.
    std::unique_ptr<WORKSPACE> reuseWorkspace(new WORKSPACE());
    reuseWorkspace->arr_IMG.Alloc(sizeof(IMG), 4);
    if (reuseWorkspace->NewIMG(0, 0, 0, 1, 1, 0) != 0 ||
        reuseWorkspace->NewIMG(0, 1, 0, 1, 1, 0) != 1)
        return 132;
    ((IMG*)reuseWorkspace->arr_IMG.data)[1].name.assign("survivor");
    if (reuseWorkspace->DeleteIMG(0) != 0 ||
        reuseWorkspace->NewIMG(0, 2, 0, 1, 1, 0) != 1)
        return 133;
    IMG* reusedImages = (IMG*)reuseWorkspace->arr_IMG.data;
    if (!reusedImages[0].name.isSame("survivor") ||
        !reusedImages[1].name.isSame("manual crop") ||
        reusedImages[0].name.body == reusedImages[1].name.body)
        return 134;
    for (int imageIndex = 0; imageIndex < reuseWorkspace->arr_IMG.count;
        ++imageIndex) {
        reusedImages[imageIndex].name.~CSTR();
        reusedImages[imageIndex].name.body = NULL;
    }
    reuseWorkspace->arr_IMG.Free();

    char tempDirectory[MAX_PATH] = {};
    if (!GetTempPathA(MAX_PATH, tempDirectory)) return 10;
    char outputPath[MAX_PATH] = {};
    snprintf(outputPath, sizeof(outputPath), "%sSkinEditor_asset_%lu.lr2skin",
        tempDirectory, GetCurrentProcessId());
    remove(outputPath);

    WORKSPACE workspace;
    workspace.skinfileLines.Alloc(sizeof(SKINFILELINEREAD), 8);
    workspace.arr_IMG.Alloc(sizeof(IMG), 8);
    workspace.arr_SRCGR.Alloc(sizeof(SRCGR), 4);
    workspace.arr_SRC.Alloc(sizeof(SRC), 4);
    workspace.arr_seobj.Alloc(sizeof(SEOBJ), 8);
    workspace.arr_ifunit.Alloc(sizeof(IFUNIT), 4);
    workspace.arr_history.Alloc(sizeof(HISTORY), 8);
    strncpy(workspace.mainpath, outputPath, MAX_PATH - 1);

    auto appendLine = [&](const char* text, int ifgroup) {
        SKINFILELINEREAD* line =
            (SKINFILELINEREAD*)workspace.skinfileLines.Get_new();
        line->line.assign(text);
        line->filename.assign(outputPath);
        line->numTotal = workspace.skinfileLines.count - 1;
        line->num = line->numTotal + 1;
        line->isComment = text[0] != '#';
        line->isSEcomment = text[0] == '$';
        line->ifgroup = ifgroup;
        SplitCSV(line->line, &line->csv, ",");
        line->csvColumnCount = CountCsvColumns(line->line);
    };

    const char* usedMetadata =
        "$SRC_IMAGE,0,5,10,20,30,40,1,1,0,0,0,0,0";
    const char* actualSource =
        "#SRC_NUMBER,0,5,10,20,30,40,5,2,1000,77,212,2,4";
    const char* unusedMetadata =
        "$SRC_IMAGE,0,5,50,60,70,80,1,1,0,0,0,0,0";
    appendLine("#IF,900", 0);
    appendLine(usedMetadata, 0);
    appendLine(actualSource, 0);
    appendLine(unusedMetadata, 0);
    appendLine("#ENDIF", 0);

    // LR2 wildcard candidates include directory and non-image matches. Keep
    // both in Image Manager, preserve the suffix after '*', and automatically
    // resolve only a loadable final image path.
    const std::string graphicRoot = std::string(outputPath) + "_graphics";
    const std::string graphicVariant = graphicRoot + "\\Default";
    const std::string brokenVariant = graphicRoot + "\\Broken";
    const std::string graphicImage = graphicVariant + "\\main.bmp";
    const std::string graphicFileCandidate = graphicRoot + "\\Default.png";
    if (!CreateDirectoryA(graphicRoot.c_str(), NULL) ||
        !CreateDirectoryA(graphicVariant.c_str(), NULL) ||
        !CreateDirectoryA(brokenVariant.c_str(), NULL)) return 43;
    const unsigned char onePixelBmp[] = {
        0x42, 0x4d, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x01, 0x00, 0x18, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
        0x00, 0x00
    };
    FILE* graphicFile = fopen(graphicImage.c_str(), "wb");
    if (!graphicFile) return 44;
    const bool wroteGraphic = fwrite(onePixelBmp, 1, sizeof(onePixelBmp),
        graphicFile) == sizeof(onePixelBmp);
    fclose(graphicFile);
    if (!wroteGraphic) return 44;
    FILE* candidateFile = fopen(graphicFileCandidate.c_str(), "wb");
    if (!candidateFile) return 44;
    const bool wroteCandidate = fwrite(onePixelBmp, 1, sizeof(onePixelBmp),
        candidateFile) == sizeof(onePixelBmp);
    fclose(candidateFile);
    if (!wroteCandidate) return 44;
    const std::string wildcardGraphic =
        std::string("#IMAGE,") + graphicRoot + "\\*\\main.bmp";
    appendLine(wildcardGraphic.c_str(), 0);
    const std::string wildcardFileGraphic =
        std::string("#IMAGE,") + graphicRoot + "\\*.png";
    appendLine(wildcardFileGraphic.c_str(), 0);

    if (workspace.ParseSkinConditions() != 0) return 11;
    const int assetIfgroup =
        ((SKINFILELINEREAD*)workspace.skinfileLines.data)[1].ifgroup;
    if (assetIfgroup <= 0 ||
        ((SKINFILELINEREAD*)workspace.skinfileLines.data)[2].ifgroup != assetIfgroup)
        return 12;
    if (workspace.ParseSkinLegacyObjectsAndAssets() != 0 ||
        workspace.arr_IMG.count != 2) return 11;
    if (workspace.ParseSkinGraphics() != 0 ||
        workspace.arr_SRCGR.count != 3) return 45;
    int invalidCandidate = -1;
    int directoryImageCandidate = -1;
    int fileImageCandidate = -1;
    for (int candidate = 0; candidate < workspace.arr_SRCGR.count; ++candidate) {
        SRCGR& graphic = ((SRCGR*)workspace.arr_SRCGR.data)[candidate];
        if (graphic.grID == 0 && graphic.filename.isSame("Default") &&
            graphic.path.body &&
            _stricmp(graphic.path.outstr(), graphicImage.c_str()) == 0)
            directoryImageCandidate = candidate;
        else if (graphic.grID == 0 && graphic.filename.isSame("Broken") &&
            graphic.path.body && GetFileAttributesA(graphic.path.outstr()) ==
                INVALID_FILE_ATTRIBUTES)
            invalidCandidate = candidate;
        else if (graphic.grID == 1 && graphic.filename.isSame("Default") &&
            graphic.path.body &&
            _stricmp(graphic.path.outstr(), graphicFileCandidate.c_str()) == 0)
            fileImageCandidate = candidate;
    }
    if (invalidCandidate < 0 || directoryImageCandidate < 0 ||
        fileImageCandidate < 0) return 46;
    SRCGR& invalidGraphic =
        ((SRCGR*)workspace.arr_SRCGR.data)[invalidCandidate];
    SRCGR& directoryImage =
        ((SRCGR*)workspace.arr_SRCGR.data)[directoryImageCandidate];
    SRCGR& fileImage = ((SRCGR*)workspace.arr_SRCGR.data)[fileImageCandidate];
    invalidGraphic.loaded = true;
    directoryImage.texture = reinterpret_cast<PDIRECT3DTEXTURE9>(1);
    const int resolverTag = workspace.NewIMG(0, 0, 0, 1, 1, 0);
    if (resolverTag != 2 ||
        workspace.ResolveIMGTextureIndex(resolverTag) != directoryImageCandidate)
        return 47;
    directoryImage.texture = NULL;
    directoryImage.loaded = true;
    if (workspace.ResolveIMGTextureIndex(resolverTag) != -1) return 48;
    if (workspace.DeleteIMG(resolverTag) != 0) return 49;
    fileImage.texture = reinterpret_cast<PDIRECT3DTEXTURE9>(1);
    const int fileResolverTag = workspace.NewIMG(1, 0, 0, 1, 1, 0);
    if (fileResolverTag != 2 ||
        workspace.ResolveIMGTextureIndex(fileResolverTag) != fileImageCandidate)
        return 50;
    fileImage.texture = NULL;
    if (workspace.DeleteIMG(fileResolverTag) != 0) return 51;
    IMG& used = ((IMG*)workspace.arr_IMG.data)[0];
    IMG& unused = ((IMG*)workspace.arr_IMG.data)[1];
    if (used.sourceDeclare != 2 || used.editorDeclare != -1 ||
        unused.sourceDeclare != -2 || unused.editorDeclare != 3 ||
        unused.ifGroup != assetIfgroup) return 13;

    SRC* dividedSource = (SRC*)workspace.arr_SRC.Get_new();
    dividedSource->declare = used.sourceDeclare;
    dividedSource->div_x = 5;
    dividedSource->div_y = 2;
    dividedSource->cycle = 1000;
    dividedSource->timer = 77;
    dividedSource->num = 212;
    dividedSource->align = 2;
    dividedSource->keta = 4;
    int testDivX = 0;
    int testDivY = 0;
    int testCycle = 0;
    int testTimer = 0;
    workspace.ResolveIMGDivision(0, testDivX, testDivY, testCycle, testTimer);
    if (testDivX != 5 || testDivY != 2 || testCycle != 1000 ||
        testTimer != 77 || used.w / testDivX != 6 || used.h / testDivY != 20)
        return 32;

    SplitCSV("", &workspace.nCsv, ",");
    workspace.nCsv.str[0].assign("#SRC_NUMBER");
    if (!workspace.InitializeAssetBackedObjectSource(workspace.nCsv,
        "#SRC_NUMBER", 0)) return 38;
    if (atol(workspace.nCsv.str[2].outstr()) != 5 ||
        atol(workspace.nCsv.str[7].outstr()) != 5 ||
        atol(workspace.nCsv.str[8].outstr()) != 2 ||
        atol(workspace.nCsv.str[11].outstr()) != 212 ||
        atol(workspace.nCsv.str[12].outstr()) != 2 ||
        atol(workspace.nCsv.str[13].outstr()) != 4) return 39;

    workspace.newObjectName.assign("");
    workspace.newObjectAutoName.clear();
    workspace.newObjectNameManuallyEdited = false;
    workspace.SynchronizeNewObjectAutoName("#SRC_NUMBER", false);
    if (!workspace.newObjectName.body ||
        strstr(workspace.newObjectName.outstr(), "fastCount") == NULL) return 40;
    workspace.nCsv.str[11].assign("214");
    workspace.SynchronizeNewObjectAutoName("#SRC_NUMBER", false);
    if (!workspace.newObjectName.body ||
        strstr(workspace.newObjectName.outstr(), "slowCount") == NULL) return 41;
    workspace.newObjectName.assign("My counter");
    workspace.newObjectNameManuallyEdited = true;
    workspace.SynchronizeNewObjectAutoName("#SRC_NUMBER", false);
    if (!workspace.newObjectName.isSame("My counter")) return 42;

    if (!workspace.objectEditorModel.LoadGroups(nullptr)) return 52;
    workspace.RebuildObjectModel();
    const int sourceObject = SEFindObjectForRow(
        workspace.objectEditorModel.Objects(), 2);
    if (sourceObject < 0) return 53;
    workspace.src_selected = 1;
    workspace.imageManagerFocusRequest = -1;
    workspace.SetObjectSelection(std::vector<int>(1, sourceObject),
        sourceObject, sourceObject, false);
    if (workspace.src_selected != 0 || workspace.grID_selected != used.gr ||
        workspace.imageManagerFocusRequest != 0) return 54;
    workspace.src_selected = 1;
    workspace.RestoreObjectSelection();
    if (workspace.src_selected != 0 || workspace.imageManagerFocusRequest != 0)
        return 55;
    if (!workspace.SetImageManagerHoveredObject(sourceObject, 123) ||
        workspace.imageManagerHoveredAssetIndex != 0 ||
        workspace.imageManagerHoveredAssetFrame != 123) return 56;
    if (workspace.SetImageManagerHoveredObject(-1, 124) ||
        workspace.imageManagerHoveredAssetIndex != -1 ||
        workspace.imageManagerHoveredAssetFrame != 124) return 57;
    std::vector<std::vector<int>> assetUsage;
    workspace.BuildImageAssetUsage(assetUsage);
    if (assetUsage.size() != 2 || assetUsage[0].size() != 1 ||
        assetUsage[0][0] != sourceObject) return 58;
    if (!assetUsage[1].empty()) return 59;
    const std::vector<std::vector<int>>& cachedUsage =
        workspace.ImageAssetUsage();
    if (cachedUsage.size() != 2 || cachedUsage[0].size() != 1 ||
        !cachedUsage[1].empty()) return 60;
    std::string deleteReason;
    if (workspace.CanDeleteIMG(0, &deleteReason) ||
        deleteReason.find("used by 1 Object") == std::string::npos)
        return 78;
    if (!workspace.CanDeleteIMG(1, &deleteReason) || !deleteReason.empty())
        return 79;

    std::vector<int> assignableRows;
    workspace.CollectImageAssignableSourceRows(sourceObject, assignableRows);
    if (assignableRows.size() != 1 || assignableRows.front() != 2) return 62;
    SKINFILELINEREAD& editableSource =
        ((SKINFILELINEREAD*)workspace.skinfileLines.data)[2];
    const std::string originalSourceLine = editableSource.line.outstr();
    const int historyBeforeApply = workspace.arr_history.count;
    if (!workspace.ApplyImageAssetToObjectSource(1, sourceObject, 2, false))
        return 63;
    int appliedColumns[5];
    if (!workspace.ResolveImageCropColumns("#SRC_NUMBER", appliedColumns) ||
        editableSource.csv.val[appliedColumns[0]] != unused.gr ||
        editableSource.csv.val[appliedColumns[1]] != unused.x ||
        editableSource.csv.val[appliedColumns[2]] != unused.y ||
        editableSource.csv.val[appliedColumns[3]] != unused.w ||
        editableSource.csv.val[appliedColumns[4]] != unused.h ||
        editableSource.csv.val[11] != 212 ||
        editableSource.csv.val[12] != 2 ||
        editableSource.csv.val[13] != 4) return 64;
    if (workspace.arr_history.count != historyBeforeApply + 1) return 65;
    if (workspace.UndoLastEdit() != 0 ||
        workspace.arr_history.count != historyBeforeApply ||
        !editableSource.line.body ||
        originalSourceLine != editableSource.line.outstr()) return 66;

    std::vector<SEImageDiagnostic> diagnostics;
    workspace.BuildImageDiagnostics(diagnostics);
    bool foundMissingGraphic = false;
    bool foundUnusedAsset = false;
    for (const SEImageDiagnostic& diagnostic : diagnostics) {
        foundMissingGraphic |= diagnostic.kind ==
            SEImageDiagnosticKind::MissingFile;
        foundUnusedAsset |= diagnostic.kind ==
            SEImageDiagnosticKind::UnusedAsset && diagnostic.assetIndex == 1;
    }
    if (!foundMissingGraphic || !foundUnusedAsset) return 67;

    const int historyBeforeReplace = workspace.arr_history.count;
    const int replaceDeclaration = fileImage.declare;
    const std::string originalImageDeclaration =
        ((SKINFILELINEREAD*)workspace.skinfileLines.data)
        [replaceDeclaration].line.outstr();
    std::string imageOperationError;
    if (!workspace.ReplaceImageDeclarationPath(fileImageCandidate,
        graphicImage.c_str(), imageOperationError) ||
        workspace.arr_history.count != historyBeforeReplace + 1 ||
        strstr(((SKINFILELINEREAD*)workspace.skinfileLines.data)
            [replaceDeclaration].line.outstr(), "#IMAGE,") !=
            ((SKINFILELINEREAD*)workspace.skinfileLines.data)
            [replaceDeclaration].line.outstr()) return 68;
    if (workspace.UndoLastEdit() != 0 ||
        workspace.arr_history.count != historyBeforeReplace ||
        originalImageDeclaration !=
            ((SKINFILELINEREAD*)workspace.skinfileLines.data)
            [replaceDeclaration].line.outstr()) return 69;

    // Grid splitting persists named, editor-only cells and records every row
    // insertion as one user-visible Undo action.
    std::unique_ptr<WORKSPACE> gridWorkspace(new WORKSPACE());
    gridWorkspace->skinfileLines.Alloc(sizeof(SKINFILELINEREAD), 8);
    gridWorkspace->arr_IMG.Alloc(sizeof(IMG), 8);
    gridWorkspace->arr_SRCGR.Alloc(sizeof(SRCGR), 2);
    gridWorkspace->arr_SRC.Alloc(sizeof(SRC), 2);
    gridWorkspace->arr_seobj.Alloc(sizeof(SEOBJ), 4);
    gridWorkspace->arr_ifunit.Alloc(sizeof(IFUNIT), 2);
    gridWorkspace->arr_history.Alloc(sizeof(HISTORY), 16);
    strncpy(gridWorkspace->mainpath, outputPath, MAX_PATH - 1);
    auto appendGridLine = [&](const char* text) {
        SKINFILELINEREAD* line =
            (SKINFILELINEREAD*)gridWorkspace->skinfileLines.Get_new();
        line->line.assign(text);
        line->filename.assign(outputPath);
        line->numTotal = gridWorkspace->skinfileLines.count - 1;
        line->num = line->numTotal;
        line->isComment = text[0] != '#';
        line->isSEcomment = text[0] == '$';
        line->ifgroup = 0;
        SplitCSV(line->line, &line->csv, ",");
        line->csvColumnCount = CountCsvColumns(line->line);
    };
    appendGridLine("#IMAGE,grid.png");
    appendGridLine("$SRC_IMAGE,0,0,0,0,8,8,1,1,0,0,0,0,0,base");
    SRCGR* gridGraphic = (SRCGR*)gridWorkspace->arr_SRCGR.Get_new();
    gridGraphic->path.assign(graphicImage.c_str());
    gridGraphic->filename.assign("grid.png");
    gridGraphic->texture = reinterpret_cast<PDIRECT3DTEXTURE9>(1);
    gridGraphic->loaded = true;
    gridGraphic->sizeX = 8;
    gridGraphic->sizeY = 8;
    gridGraphic->grID = 0;
    gridGraphic->isIf = 0;
    gridGraphic->declare = 0;
    const int gridBaseIndex = gridWorkspace->NewIMG(0, 0, 0, 8, 8, 0);
    ((IMG*)gridWorkspace->arr_IMG.data)[gridBaseIndex].editorDeclare = 1;
    std::vector<unsigned char> selectedGridCells(4, 1);
    std::vector<int> insertedGridRows;
    if (!gridWorkspace->RegisterImageAssetGrid(gridBaseIndex, 2, 2,
        selectedGridCells, "grid", insertedGridRows, imageOperationError) ||
        insertedGridRows.size() != 4 ||
        gridWorkspace->skinfileLines.count != 6 ||
        gridWorkspace->arr_history.count != 9) return 70;
    SKINFILELINEREAD& firstGridCell =
        ((SKINFILELINEREAD*)gridWorkspace->skinfileLines.data)
        [insertedGridRows.front()];
    if (!firstGridCell.line.body ||
        strstr(firstGridCell.line.outstr(),
            "$SRC_IMAGE,0,0,0,0,4,4,1,1,0,0,0,0,0,grid_001") !=
            firstGridCell.line.outstr()) return 71;
    if (gridWorkspace->ParseSkinConditions() != 0) return 72;
    if (gridWorkspace->ParseSkinLegacyObjectsAndAssets() != 0) return 74;
    if (gridWorkspace->arr_IMG.count != 5) return 75;
    if (!((IMG*)gridWorkspace->arr_IMG.data)[1].name.isSame("grid_001"))
        return 76;
    if (gridWorkspace->UndoLastEdit() != 0 ||
        gridWorkspace->arr_history.count != 0 ||
        gridWorkspace->skinfileLines.count != 2) return 73;

    const int manualIndex = workspace.NewIMG(17, 1, 2, 3, 4, 23);
    if (manualIndex != 2) return 14;
    const std::vector<std::vector<int>>& expandedUsage =
        workspace.ImageAssetUsage();
    if (expandedUsage.size() != 3 || !expandedUsage[2].empty()) return 61;
    IMG& manual = ((IMG*)workspace.arr_IMG.data)[manualIndex];
    if (manual.gr != 17 || manual.ifGroup != 23 ||
        manual.sourceDeclare != -2) return 15;

    if (workspace.SaveSkinScript(outputPath, false, true) != 0) return 16;
    workspace.loaded = true;
    workspace.documentRevision = 5;
    workspace.savedDocumentRevision = 4;
    if (!workspace.IsDocumentDirty()) return 36;
    if (workspace.SaveCurrentSkin() != 0 || workspace.IsDocumentDirty() ||
        workspace.lastSaveState != 1) return 37;
    FILE* saved = fopen(outputPath, "rb");
    if (!saved) return 17;
    std::string contents;
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), saved)) contents += buffer;
    fclose(saved);
    if (contents.find(usedMetadata) != std::string::npos) return 18;
    if (contents.find(actualSource) == std::string::npos) return 19;
    if (contents.find(unusedMetadata) == std::string::npos) return 20;

    std::unique_ptr<WORKSPACE> reloaded(new WORKSPACE());
    reloaded->skinfileLines.Alloc(sizeof(SKINFILELINEREAD), 8);
    reloaded->arr_IMG.Alloc(sizeof(IMG), 8);
    reloaded->arr_seobj.Alloc(sizeof(SEOBJ), 8);
    reloaded->arr_ifunit.Alloc(sizeof(IFUNIT), 4);
    reloaded->arr_history.Alloc(sizeof(HISTORY), 2);
    strncpy(reloaded->mainpath, outputPath, MAX_PATH - 1);
    std::size_t lineStart = 0;
    while (lineStart < contents.size()) {
        const std::size_t lineEnd = contents.find('\n', lineStart);
        std::string text = contents.substr(lineStart,
            lineEnd == std::string::npos ? std::string::npos : lineEnd - lineStart);
        if (!text.empty() && text.back() == '\r') text.pop_back();
        if (!text.empty()) {
            SKINFILELINEREAD* line =
                (SKINFILELINEREAD*)reloaded->skinfileLines.Get_new();
            line->line.assign(text.c_str());
            line->filename.assign(outputPath);
            line->numTotal = reloaded->skinfileLines.count - 1;
            line->isComment = text[0] != '#';
            line->isSEcomment = text[0] == '$';
            SplitCSV(line->line, &line->csv, ",");
            line->csvColumnCount = CountCsvColumns(line->line);
        }
        if (lineEnd == std::string::npos) break;
        lineStart = lineEnd + 1;
    }
    if (reloaded->ParseSkinConditions() != 0 ||
        reloaded->ParseSkinLegacyObjectsAndAssets() != 0 ||
        reloaded->arr_IMG.count != 2) return 21;
    IMG& reloadedManual = ((IMG*)reloaded->arr_IMG.data)[1];
    if (reloadedManual.sourceDeclare != -2 || reloadedManual.editorDeclare < 0 ||
        reloadedManual.ifGroup <= 0)
        return 22;

    const char* editedMetadata =
        "$SRC_IMAGE,0,5,51,61,69,79,1,1,0,0,0,0,0";
    if (reloaded->ModifyIMG(1, 5, 51, 61, 69, 79) != 0) return 27;
    if (reloadedManual.x != 51 || reloadedManual.y != 61 ||
        reloadedManual.w != 69 || reloadedManual.h != 79) return 28;
    if (reloaded->SaveSkinScript(outputPath, false, true) != 0) return 29;
    FILE* editedSave = fopen(outputPath, "rb");
    if (!editedSave) return 30;
    std::string editedContents;
    while (fgets(buffer, sizeof(buffer), editedSave)) editedContents += buffer;
    fclose(editedSave);
    if (editedContents.find(editedMetadata) == std::string::npos ||
        editedContents.find(unusedMetadata) != std::string::npos) return 31;

    if (reloaded->DeleteIMG(1) != 0 || reloaded->arr_IMG.count != 1)
        return 23;
    if (reloaded->SaveSkinScript(outputPath, false, true) != 0) return 24;
    FILE* deletedSave = fopen(outputPath, "rb");
    if (!deletedSave) return 25;
    std::string deletedContents;
    while (fgets(buffer, sizeof(buffer), deletedSave)) deletedContents += buffer;
    fclose(deletedSave);
    if (deletedContents.find(editedMetadata) != std::string::npos) return 26;

    // Generated images must use the next slot in LR2's active image table.
    // tricoro writes mutually exclusive 1P/2P layouts as two consecutive IF
    // blocks; adding both blocks' image counts produces an invalid gr even
    // though the generated #IMAGE itself is correctly appended at the root.
    std::unique_ptr<WORKSPACE> generatedWorkspace(new WORKSPACE());
    generatedWorkspace->skinfileLines.Alloc(sizeof(SKINFILELINEREAD), 12);
    generatedWorkspace->arr_CustomFile.Alloc(sizeof(CSTR), 2);
    generatedWorkspace->arr_SRCGR.Alloc(sizeof(SRCGR), 8);
    generatedWorkspace->arr_IMG.Alloc(sizeof(IMG), 8);
    generatedWorkspace->arr_SRC.Alloc(sizeof(SRC), 8);
    generatedWorkspace->arr_DST.Alloc(sizeof(DST), 8);
    generatedWorkspace->arr_seobj.Alloc(sizeof(SEOBJ), 8);
    generatedWorkspace->arr_ifunit.Alloc(sizeof(IFUNIT), 8);
    generatedWorkspace->arr_history.Alloc(sizeof(HISTORY), 8);
    strncpy(generatedWorkspace->mainpath, outputPath, MAX_PATH - 1);
    auto appendGeneratedLine = [&](const char* text) {
        SKINFILELINEREAD* line =
            (SKINFILELINEREAD*)generatedWorkspace->skinfileLines.Get_new();
        line->line.assign(text);
        line->filename.assign(outputPath);
        line->numTotal = generatedWorkspace->skinfileLines.count - 1;
        line->num = line->numTotal;
        line->isComment = text[0] != '#';
        line->isSEcomment = text[0] == '$';
        SplitCSV(line->line, &line->csv, ",");
        line->csvColumnCount = CountCsvColumns(line->line);
    };
    const std::string fileStart = std::string("$FILE '") + outputPath + "' start";
    const std::string fileEnd = std::string("$FILE '") + outputPath + "' end";
    appendGeneratedLine(fileStart.c_str());
    appendGeneratedLine("#IMAGE,base.png");
    appendGeneratedLine("#IF,900");
    appendGeneratedLine("#IMAGE,left_a.png");
    appendGeneratedLine("#IMAGE,left_b.png");
    appendGeneratedLine("#SRC_IMAGE,0,0,0,0,1,1,1,1,0,0,0,0,0");
    appendGeneratedLine("#DST_IMAGE,0,0,5,6,1,1,0,255,255,255,255,0,0,0,0,0,0,0,0,0");
    appendGeneratedLine("#ENDIF");
    appendGeneratedLine("#IF,901");
    appendGeneratedLine("#IMAGE,right_a.png");
    appendGeneratedLine("#IMAGE,right_b.png");
    appendGeneratedLine("#ENDIF");
    appendGeneratedLine(fileEnd.c_str());
    generatedWorkspace->g.skstruct.op[0] = 1;
    generatedWorkspace->g.skstruct.op[900] = 1;
    generatedWorkspace->g.skstruct.op[901] = 0;
    char generatedImagePath[MAX_PATH] = {};
    snprintf(generatedImagePath, sizeof(generatedImagePath),
        "%sSkinEditor_generated_%lu.png", tempDirectory, GetCurrentProcessId());
    remove(generatedImagePath);
    FILE* generatedImageFile = fopen(generatedImagePath, "wb");
    if (!generatedImageFile) return 90;
    const bool wroteGeneratedImage = fwrite(onePixelBmp, 1,
        sizeof(onePixelBmp), generatedImageFile) == sizeof(onePixelBmp);
    fclose(generatedImageFile);
    if (!wroteGeneratedImage) return 90;
    std::string registrationError;
    if (generatedWorkspace->RegisterGeneratedImage(generatedImagePath, 1, 1,
        registrationError) != 3) return 33;
    if (generatedWorkspace->skinfileLines.count != 15) return 34;
    SKINFILELINEREAD& generatedImage =
        ((SKINFILELINEREAD*)generatedWorkspace->skinfileLines.data)[12];
    SKINFILELINEREAD& generatedAsset =
        ((SKINFILELINEREAD*)generatedWorkspace->skinfileLines.data)[13];
    SKINFILELINEREAD& generatedEnd =
        ((SKINFILELINEREAD*)generatedWorkspace->skinfileLines.data)[14];
    if (!generatedImage.csv.str[0].isSame("#IMAGE") ||
        !generatedAsset.line.body ||
        strstr(generatedAsset.line.outstr(), "$SRC_IMAGE,0,3,0,0,1,1") !=
            generatedAsset.line.outstr() ||
        !generatedEnd.line.body || strstr(generatedEnd.line.outstr(), "$FILE ") !=
            generatedEnd.line.outstr()) return 35;

    // GIF conversion must retain variable frame delays in LR2's uniform
    // sprite animation model and must apply its indexed transparent color
    // before WIC's BGRA conversion can make it opaque. A repeated 50/100 ms
    // four-frame GIF therefore
    // becomes six cells in two streamed PNG row writes with a 300 ms cycle,
    // then registers the exact grid as an animated full-size Asset.
    char gifInputPath[MAX_PATH] = {};
    char gifSpritePath[MAX_PATH] = {};
    snprintf(gifInputPath, sizeof(gifInputPath),
        "%sSkinEditor_gif_%lu.gif", tempDirectory, GetCurrentProcessId());
    snprintf(gifSpritePath, sizeof(gifSpritePath),
        "%sSkinEditor_gif_%lu_sprite.png", tempDirectory,
        GetCurrentProcessId());
    remove(gifInputPath);
    remove(gifSpritePath);
    remove((std::string(gifSpritePath) + ".skineditor-gif.tmp").c_str());
    const unsigned char variableDelayGif[] = {
        'G', 'I', 'F', '8', '9', 'a',
        0x01, 0x00, 0x01, 0x00, 0x80, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
        0x21, 0xf9, 0x04, 0x01, 0x05, 0x00, 0x00, 0x00,
        0x2c, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00,
        0x02, 0x02, 0x44, 0x01, 0x00,
        0x21, 0xf9, 0x04, 0x00, 0x0a, 0x00, 0x00, 0x00,
        0x2c, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00,
        0x02, 0x02, 0x4c, 0x01, 0x00,
        0x21, 0xf9, 0x04, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x2c, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00,
        0x02, 0x02, 0x44, 0x01, 0x00,
        0x21, 0xf9, 0x04, 0x00, 0x0a, 0x00, 0x00, 0x00,
        0x2c, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00,
        0x02, 0x02, 0x4c, 0x01, 0x00,
        0x3b
    };
    FILE* gifInput = fopen(gifInputPath, "wb");
    if (!gifInput) return 81;
    const bool wroteGif = fwrite(variableDelayGif, 1,
        sizeof(variableDelayGif), gifInput) == sizeof(variableDelayGif);
    fclose(gifInput);
    if (!wroteGif) return 81;
    char gifError[256] = {};
    GifSpriteInfo gifInfo;
    if (!InspectGifSprite(gifInputPath, &gifInfo, gifError,
        sizeof(gifError))) return 82;
    if (gifInfo.sourceFrameCount != 4 || gifInfo.outputFrameCount != 6 ||
        gifInfo.frameWidth != 1 || gifInfo.frameHeight != 1 ||
        gifInfo.columns != 3 || gifInfo.rows != 2 ||
        gifInfo.sheetWidth != 3 || gifInfo.sheetHeight != 2 ||
        gifInfo.cycleMs != 300 || !gifInfo.timingDuplicated ||
        gifInfo.timingApproximate) return 83;
    if (!ConvertGifToSpriteSheetAtomic(gifInputPath, gifSpritePath,
        &gifInfo, gifError, sizeof(gifError))) return 84;
    int gifSheetWidth = 0;
    int gifSheetHeight = 0;
    if (!GetImageSizeFromFile(gifSpritePath, &gifSheetWidth,
        &gifSheetHeight) || gifSheetWidth != 3 || gifSheetHeight != 2)
        return 85;
    unsigned char gifFirstAlpha = 255;
    if (!ReadImageFilePixelAlpha(gifSpritePath, 0, 0, &gifFirstAlpha) ||
        gifFirstAlpha != 0) return 89;

    // Exercise the actual WIC scaling path without a huge test fixture. The
    // logical GIF canvas is 5000x1 while its only changed frame is 1x1, so the
    // LR2-compatible sheet must be reduced to 4096x1 during conversion.
    char scaledGifPath[MAX_PATH] = {};
    char scaledSpritePath[MAX_PATH] = {};
    snprintf(scaledGifPath, sizeof(scaledGifPath),
        "%sSkinEditor_gif_scaled_%lu.gif", tempDirectory,
        GetCurrentProcessId());
    snprintf(scaledSpritePath, sizeof(scaledSpritePath),
        "%sSkinEditor_gif_scaled_%lu.png", tempDirectory,
        GetCurrentProcessId());
    remove(scaledGifPath);
    remove(scaledSpritePath);
    const unsigned char wideCanvasGif[] = {
        'G', 'I', 'F', '8', '9', 'a',
        0x88, 0x13, 0x01, 0x00, 0x80, 0x00, 0x00,
        0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
        0x2c, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00,
        0x02, 0x02, 0x44, 0x01, 0x00, 0x3b
    };
    FILE* scaledGif = fopen(scaledGifPath, "wb");
    if (!scaledGif) return 107;
    const bool wroteScaledGif = fwrite(wideCanvasGif, 1,
        sizeof(wideCanvasGif), scaledGif) == sizeof(wideCanvasGif);
    fclose(scaledGif);
    if (!wroteScaledGif) return 107;
    GifSpriteInfo scaledInfo;
    if (!InspectGifSprite(scaledGifPath, &scaledInfo, gifError,
        sizeof(gifError)) || scaledInfo.sourceFrameWidth != 5000 ||
        scaledInfo.sourceFrameHeight != 1 || !scaledInfo.frameScaled ||
        scaledInfo.frameWidth != 4096 || scaledInfo.frameHeight != 1 ||
        scaledInfo.sheetWidth != 4096 || scaledInfo.sheetHeight != 1)
        return 108;
    if (!ConvertGifToSpriteSheetAtomic(scaledGifPath, scaledSpritePath,
        &scaledInfo, gifError, sizeof(gifError))) return 109;
    int scaledWidth = 0;
    int scaledHeight = 0;
    if (!GetImageSizeFromFile(scaledSpritePath, &scaledWidth,
        &scaledHeight) || scaledWidth != 4096 || scaledHeight != 1)
        return 110;
    remove(scaledGifPath);
    remove(scaledSpritePath);

    registrationError.clear();
    if (generatedWorkspace->RegisterGeneratedImage(gifSpritePath,
        gifInfo.sheetWidth, gifInfo.sheetHeight, registrationError,
        gifInfo.columns, gifInfo.rows, gifInfo.cycleMs,
        gifInfo.sourceFrameWidth, gifInfo.sourceFrameHeight) != 4) return 86;
    if (generatedWorkspace->skinfileLines.count != 17) return 87;
    SKINFILELINEREAD& gifAsset =
        ((SKINFILELINEREAD*)generatedWorkspace->skinfileLines.data)[15];
    if (!gifAsset.line.body ||
        strstr(gifAsset.line.outstr(),
            "$SRC_IMAGE,0,4,0,0,3,2,3,2,300,0,0,0,0") !=
            gifAsset.line.outstr()) return 88;

    // Reparse exactly as Image Manager does on the following frame. Generated
    // paths are portable (LR2files-rooted when possible, otherwise relative
    // to the owner), and editor-only GIF Assets must keep their grid/cycle
    // even though no real #SRC exists yet.
    if (generatedWorkspace->RebuildEditorDerivedState() != 0) return 91;
    int generatedGraphicIndex = -1;
    int gifGraphicIndex = -1;
    int gifAssetIndex = -1;
    for (int graphicIndex = 0;
        graphicIndex < generatedWorkspace->arr_SRCGR.count; ++graphicIndex) {
        const SRCGR& graphic =
            ((const SRCGR*)generatedWorkspace->arr_SRCGR.data)[graphicIndex];
        if (graphic.grID == 3) generatedGraphicIndex = graphicIndex;
        if (graphic.grID == 4) gifGraphicIndex = graphicIndex;
    }
    for (int assetIndex = 0; assetIndex < generatedWorkspace->arr_IMG.count;
        ++assetIndex) {
        const IMG& asset =
            ((const IMG*)generatedWorkspace->arr_IMG.data)[assetIndex];
        if (asset.gr == 4 && asset.editorDeclare >= 0)
            gifAssetIndex = assetIndex;
    }
    if (generatedGraphicIndex < 0 || gifGraphicIndex < 0 ||
        gifAssetIndex < 0) return 92;
    const SRCGR& reparsedGenerated =
        ((const SRCGR*)generatedWorkspace->arr_SRCGR.data)[generatedGraphicIndex];
    const SRCGR& reparsedGif =
        ((const SRCGR*)generatedWorkspace->arr_SRCGR.data)[gifGraphicIndex];
    if (reparsedGenerated.sizeX != 1 || reparsedGenerated.sizeY != 1 ||
        reparsedGif.sizeX != 3 || reparsedGif.sizeY != 2) return 93;
    int reparsedDivX = 0;
    int reparsedDivY = 0;
    int reparsedCycle = 0;
    int reparsedTimer = -1;
    generatedWorkspace->ResolveIMGDivision(gifAssetIndex, reparsedDivX,
        reparsedDivY, reparsedCycle, reparsedTimer);
    if (reparsedDivX != 3) return 101;
    if (reparsedDivY != 2) return 102;
    if (reparsedCycle != 300) return 103;
    if (reparsedTimer != 0) return 104;
    SplitCSV("", &generatedWorkspace->nCsv, ",");
    generatedWorkspace->nCsv.str[0].assign("#SRC_IMAGE");
    if (!generatedWorkspace->InitializeAssetBackedObjectSource(
        generatedWorkspace->nCsv, "#SRC_IMAGE", gifAssetIndex) ||
        atol(generatedWorkspace->nCsv.str[7].outstr()) != 3 ||
        atol(generatedWorkspace->nCsv.str[8].outstr()) != 2 ||
        atol(generatedWorkspace->nCsv.str[9].outstr()) != 300) return 95;

    // Even with an older IF Object selected, a generated Asset-backed Object
    // has to be inserted after its #IMAGE declaration. Otherwise tricoro's
    // flattened include order creates the SRC before the graphic handle.
    if (!generatedWorkspace->objectEditorModel.LoadGroups(nullptr)) return 96;
    generatedWorkspace->RebuildObjectModel();
    const int olderObject = SEFindObjectForRow(
        generatedWorkspace->objectEditorModel.Objects(), 5);
    if (olderObject < 0) return 97;
    generatedWorkspace->SetObjectSelection(std::vector<int>(1, olderObject),
        olderObject, olderObject, false);
    if (!generatedWorkspace->OpenNewObjectFromAsset(gifAssetIndex, 25, 35))
        return 98;
    if (generatedWorkspace->newObjectDropW != 1 ||
        generatedWorkspace->newObjectDropH != 1) return 111;
    const IMG& reparsedGifAsset =
        ((const IMG*)generatedWorkspace->arr_IMG.data)[gifAssetIndex];
    if (generatedWorkspace->newObjectInsertPosition !=
            reparsedGifAsset.editorDeclare + 1 ||
        generatedWorkspace->newObjectIfgroup != reparsedGifAsset.ifGroup ||
        !generatedWorkspace->newObjectOwner.body ||
        _stricmp(generatedWorkspace->newObjectOwner.outstr(), outputPath) != 0)
        return 99;

    CSTR relativeGenerated(std::filesystem::path(generatedImagePath)
        .filename().string().c_str());
    CSTR generatedDirectory(tempDirectory);
    CSTR fallbackResolved = GetRandomFileNoError(relativeGenerated,
        generatedDirectory);
    if (!fallbackResolved.body || fallbackResolved.isSame("ERROR") ||
        _stricmp(fallbackResolved.outstr(), generatedImagePath) != 0)
        return 100;

    // Stock LR2 can probe a path relative to the CSV directory but then pass
    // the unresolved ".\\..." string to LoadGraph(). Generated files below
    // the active LR2files tree therefore have to be serialized from LR2's
    // working directory, not from the skin script directory.
    const std::filesystem::path compatibilityRoot =
        std::filesystem::path(tempDirectory) /
        (std::string("SkinEditor_lr2_path_") +
            std::to_string(GetCurrentProcessId()));
    const std::filesystem::path compatibilitySkinDirectory =
        compatibilityRoot / "LR2files" / "Theme" / "PathTest";
    const std::filesystem::path compatibilitySkin =
        compatibilitySkinDirectory / "skin.lr2skin";
    const std::filesystem::path compatibilityImage =
        compatibilitySkinDirectory / "generated.bmp";
    std::error_code compatibilityError;
    std::filesystem::remove_all(compatibilityRoot, compatibilityError);
    compatibilityError.clear();
    std::filesystem::create_directories(compatibilitySkinDirectory,
        compatibilityError);
    if (compatibilityError) return 112;
    FILE* compatibilityImageFile = fopen(
        compatibilityImage.string().c_str(), "wb");
    if (!compatibilityImageFile) return 113;
    const bool wroteCompatibilityImage = fwrite(onePixelBmp, 1,
        sizeof(onePixelBmp), compatibilityImageFile) == sizeof(onePixelBmp);
    fclose(compatibilityImageFile);
    if (!wroteCompatibilityImage) return 113;

    std::unique_ptr<WORKSPACE> compatibilityWorkspace(new WORKSPACE());
    compatibilityWorkspace->skinfileLines.Alloc(sizeof(SKINFILELINEREAD), 8);
    compatibilityWorkspace->arr_CustomFile.Alloc(sizeof(CSTR), 2);
    compatibilityWorkspace->arr_SRCGR.Alloc(sizeof(SRCGR), 4);
    compatibilityWorkspace->arr_IMG.Alloc(sizeof(IMG), 4);
    compatibilityWorkspace->arr_history.Alloc(sizeof(HISTORY), 8);
    strncpy(compatibilityWorkspace->mainpath,
        compatibilitySkin.string().c_str(), MAX_PATH - 1);
    auto appendCompatibilityLine = [&](const char* text) {
        SKINFILELINEREAD* line = (SKINFILELINEREAD*)
            compatibilityWorkspace->skinfileLines.Get_new();
        line->line.assign(text);
        line->filename.assign(compatibilityWorkspace->mainpath);
        line->numTotal = compatibilityWorkspace->skinfileLines.count - 1;
        line->num = line->numTotal;
        line->isComment = text[0] != '#';
        line->isSEcomment = text[0] == '$';
        SplitCSV(line->line, &line->csv, ",");
        line->csvColumnCount = CountCsvColumns(line->line);
    };
    const std::string compatibilityStart = std::string("$FILE '") +
        compatibilitySkin.string() + "' start";
    const std::string compatibilityEnd = std::string("$FILE '") +
        compatibilitySkin.string() + "' end";
    appendCompatibilityLine(compatibilityStart.c_str());
    appendCompatibilityLine("#IMAGE,LR2files\\Theme\\PathTest\\base.bmp");
    appendCompatibilityLine(compatibilityEnd.c_str());
    std::string compatibilityRegistrationError;
    if (compatibilityWorkspace->RegisterGeneratedImage(
            compatibilityImage.string().c_str(), 1, 1,
            compatibilityRegistrationError) != 1 ||
        compatibilityWorkspace->skinfileLines.count != 5) return 114;
    SKINFILELINEREAD& compatibilityDeclaration =
        ((SKINFILELINEREAD*)compatibilityWorkspace->skinfileLines.data)[2];
    if (!compatibilityDeclaration.line.body ||
        _stricmp(compatibilityDeclaration.line.outstr(),
            "#IMAGE,LR2files\\Theme\\PathTest\\generated.bmp") != 0)
        return 115;
    std::filesystem::remove_all(compatibilityRoot, compatibilityError);
    remove(gifInputPath);
    remove(gifSpritePath);
    remove(generatedImagePath);

    // Reusing a logical gr must never insert another #IMAGE. The selected
    // disk file also has to be one of that fixed/wildcard declaration's
    // candidates, otherwise the Asset would preview a different texture.
    std::unique_ptr<WORKSPACE> targetWorkspace(new WORKSPACE());
    targetWorkspace->skinfileLines.Alloc(sizeof(SKINFILELINEREAD), 8);
    targetWorkspace->arr_CustomFile.Alloc(sizeof(CSTR), 2);
    targetWorkspace->arr_SRCGR.Alloc(sizeof(SRCGR), 4);
    targetWorkspace->arr_IMG.Alloc(sizeof(IMG), 4);
    targetWorkspace->arr_ifunit.Alloc(sizeof(IFUNIT), 4);
    targetWorkspace->arr_history.Alloc(sizeof(HISTORY), 8);
    strncpy(targetWorkspace->mainpath, outputPath, MAX_PATH - 1);
    auto appendTargetLine = [&](const char* text) {
        SKINFILELINEREAD* line =
            (SKINFILELINEREAD*)targetWorkspace->skinfileLines.Get_new();
        line->line.assign(text);
        line->filename.assign(outputPath);
        line->numTotal = targetWorkspace->skinfileLines.count - 1;
        line->num = line->numTotal;
        line->isComment = text[0] != '#';
        line->isSEcomment = text[0] == '$';
        SplitCSV(line->line, &line->csv, ",");
        line->csvColumnCount = CountCsvColumns(line->line);
    };
    appendTargetLine(fileStart.c_str());
    appendTargetLine(wildcardGraphic.c_str());
    appendTargetLine(fileEnd.c_str());
    if (targetWorkspace->ParseSkinConditions() != 0 ||
        targetWorkspace->ParseSkinGraphics() != 0 ||
        targetWorkspace->arr_SRCGR.count != 2) return 77;
    std::string targetRegistrationError;
    if (targetWorkspace->RegisterExistingImageAsset(1, graphicImage.c_str(),
        1, 1, targetRegistrationError) != 0 ||
        targetWorkspace->skinfileLines.count != 4) return 78;
    SKINFILELINEREAD& targetAsset =
        ((SKINFILELINEREAD*)targetWorkspace->skinfileLines.data)[2];
    if (!targetAsset.line.body ||
        strstr(targetAsset.line.outstr(), "$SRC_IMAGE,0,0,0,0,1,1") !=
            targetAsset.line.outstr() || targetAsset.ifgroup != 0) return 79;
    const int targetLineCount = targetWorkspace->skinfileLines.count;
    targetRegistrationError.clear();
    if (targetWorkspace->RegisterExistingImageAsset(1,
        graphicFileCandidate.c_str(), 1, 1, targetRegistrationError) >= 0 ||
        targetWorkspace->skinfileLines.count != targetLineCount ||
        targetRegistrationError.find("wildcard") == std::string::npos)
        return 80;

    remove(outputPath);
    remove((std::string(outputPath) + ".skineditor.tmp").c_str());
    remove((std::string(outputPath) + ".skineditor.bak").c_str());
    std::error_code graphicCleanupError;
    std::filesystem::remove_all(graphicRoot, graphicCleanupError);
    return 0;
}

int WORKSPACE::NewIMG(int gr, int x, int y, int w, int h, int ifGroup) {
    const int imageIndex = arr_IMG.count;
    IMG* img = (IMG*)arr_IMG.Get_new();
    if (!img) return -1;
    img->name = "manual crop";
    img->gr = gr;
    img->x = x;
    img->y = y;
    img->w = w;
    img->h = h;
    img->ifGroup = ifGroup;
    // -2 marks an editor-only crop with no declaring #SRC. It must stay a
    // static asset even when its rectangle happens to match another SRC.
    img->sourceDeclare = -2;
    img->editorDeclare = -1;

    //TODO:history here

    ++imageAssetUsageGeneration;
    return imageIndex;
}

int WORKSPACE::DeleteIMG(int pos) {
    if (pos < 0 || pos >= arr_IMG.count) return -1;
    const int metadataRow = ((IMG*)arr_IMG.data)[pos].editorDeclare;
    if (metadataRow >= 0 && metadataRow < skinfileLines.count) {
        SKINFILELINEREAD& metadata =
            ((SKINFILELINEREAD*)skinfileLines.data)[metadataRow];
        const char* text = metadata.line.body ? metadata.line.outstr() : "";
        if (strncmp(text, "$SRC_IMAGE,", 11) == 0) {
            DeleteLine(metadataRow);
        }
    }

    // ARR relocates entries with memcpy. Release only the removed CSTR, then
    // clear the duplicate tail slot after the surviving entries have moved.
    // Without clearing it, Delete(non-last) followed by NewIMG() reuses a
    // shallow copy of the last surviving name and corrupts the heap.
    IMG* images = (IMG*)arr_IMG.data;
    images[pos].name.~CSTR();
    images[pos].name.body = NULL;
    if (arr_IMG.DeleteAt(pos) != 0) return -1;
    if (arr_IMG.data && arr_IMG.count >= 0 && arr_IMG.count < arr_IMG.bufSize)
        memset(&((IMG*)arr_IMG.data)[arr_IMG.count], 0, sizeof(IMG));

    //TODO:history here

    ++imageAssetUsageGeneration;
    return 0;
}

int WORKSPACE::ModifyIMG(int pos, int gr, int x, int y, int w, int h) {
    if (pos < 0 || pos >= arr_IMG.count) return -1;
    IMG& img = ((IMG*)arr_IMG.data)[pos];

    // Keep a crop inside its resolved texture while preserving LR2's -1
    // "through the edge" size convention for hand-edited declarations.
    const int textureIndex = ResolveIMGTextureIndex(pos);
    if (textureIndex >= 0 && textureIndex < arr_SRCGR.count) {
        const SRCGR& texture = ((const SRCGR*)arr_SRCGR.data)[textureIndex];
        if (texture.sizeX > 0) {
            if (w != -1) {
                w = (std::max)(1, (std::min)(w, texture.sizeX));
                x = (std::max)(0, (std::min)(x, texture.sizeX - w));
            } else {
                x = (std::max)(0, (std::min)(x, texture.sizeX - 1));
            }
        } else x = (std::max)(0, x);
        if (texture.sizeY > 0) {
            if (h != -1) {
                h = (std::max)(1, (std::min)(h, texture.sizeY));
                y = (std::max)(0, (std::min)(y, texture.sizeY - h));
            } else {
                y = (std::max)(0, (std::min)(y, texture.sizeY - 1));
            }
        } else y = (std::max)(0, y);
    } else {
        x = (std::max)(0, x);
        y = (std::max)(0, y);
        if (w != -1) w = (std::max)(1, w);
        if (h != -1) h = (std::max)(1, h);
    }

    const int declaration = img.editorDeclare >= 0
        ? img.editorDeclare : img.sourceDeclare;
    if (declaration >= 0 && declaration < skinfileLines.count) {
        SKINFILELINEREAD& row =
            ((SKINFILELINEREAD*)skinfileLines.data)[declaration];
        const char* rowText = row.line.body ? row.line.outstr() : "";
        if (strncmp(rowText, "#SRC", 4) == 0 ||
            strncmp(rowText, "$SRC", 4) == 0) {
            CSVbuf editedCsv;
            SplitCSV(row.line, &editedCsv, ",");
            const int values[5] = { gr, x, y, w, h };
            for (int field = 0; field < 5; ++field) {
                char valueText[32];
                snprintf(valueText, sizeof(valueText), "%d", values[field]);
                editedCsv.str[field + 2].assign(valueText);
                editedCsv.val[field + 2] = values[field];
            }
            CSTR editedLine;
            CsvToCSTR(editedCsv, editedLine);
            CSTR oldLine(row.line);
            if (EditLine(declaration, oldLine, editedLine) != 0) return -1;
            // Editor-only $SRC rows are comments to LR2, so EditLine does not
            // populate their CSV cache. Keep it live until the deferred parser
            // rebuild runs on the next frame.
            if (editedLine.body && editedLine.outstr()[0] == '$') {
                SplitCSV(row.line, &row.csv, ",");
                row.csvColumnCount = CountCsvColumns(row.line);
            }
        }
    }

    img.gr = gr;
    img.x = x;
    img.y = y;
    img.w = w;
    img.h = h;

    ++imageAssetUsageGeneration;
    return 0;
}

bool WORKSPACE::CanDeleteIMG(int pos, std::string* reason) {
    if (reason) reason->clear();
    if (pos < 0 || pos >= arr_IMG.count) {
        if (reason) *reason = "The selected Asset is no longer available.";
        return false;
    }

    const std::vector<std::vector<int>>& usage = ImageAssetUsage();
    if (pos < (int)usage.size() && !usage[pos].empty()) {
        if (reason) {
            *reason = "This Asset is used by " +
                std::to_string(usage[pos].size()) + " Object" +
                (usage[pos].size() == 1 ? "." : "s.");
        }
        return false;
    }

    const IMG& asset = ((const IMG*)arr_IMG.data)[pos];
    if (asset.editorDeclare < 0 && asset.sourceDeclare != -2) {
        if (reason) {
            *reason = "This Asset is derived from an Object SRC declaration. "
                "Delete or edit that Object instead.";
        }
        return false;
    }
    return true;
}

//check duplicated
int WORKSPACE::FindIMG(int gr, int x, int y, int w, int h, int ifGroup) {
    int j = 0;
    for (j = 0; j < arr_IMG.count; j++) {
        IMG imgCompare = ((IMG*)arr_IMG.data)[j];
        if (imgCompare.gr == gr
            && imgCompare.x == x
            && imgCompare.y == y
            && imgCompare.w == w
            && imgCompare.h == h
            && (ifGroup < 0 || imgCompare.ifGroup == ifGroup))
            break;
    }
    return j;
}

bool WORKSPACE::ResolveImageCropColumns(const char* command,
    int columns[5]) const {
    if (!columns) return false;
    for (int field = 0; field < 5; ++field) columns[field] = -1;
    if (!command || strncmp(command, "#SRC", 4) != 0) return false;

    const char* fieldNames[5] = { "gr", "x", "y", "w", "h" };
    for (int column = 1; column < 30; ++column) {
        CSTR help = GetCommandHelp(command, column);
        help.trimWhiteSpace();
        const char* label = help.body ? help.outstr() : "";
        if (*label == '$') ++label;
        for (int field = 0; field < 5; ++field) {
            if (_stricmp(label, fieldNames[field]) == 0)
                columns[field] = column;
        }
    }
    for (int field = 0; field < 5; ++field)
        if (columns[field] < 0) return false;
    return true;
}

int WORKSPACE::FindImageAssetForRow(int row) {
    if (row < 0 || row >= skinfileLines.count || arr_IMG.count <= 0)
        return -1;
    SKINFILELINEREAD& line =
        ((SKINFILELINEREAD*)skinfileLines.data)[row];
    const char* command = line.csv.str[0].body
        ? line.csv.str[0].outstr() : "";
    int columns[5];
    if (!ResolveImageCropColumns(command, columns)) return -1;

    const int values[5] = {
        line.csv.val[columns[0]], line.csv.val[columns[1]],
        line.csv.val[columns[2]], line.csv.val[columns[3]],
        line.csv.val[columns[4]]
    };
    int imageIndex = FindIMG(values[0], values[1], values[2], values[3],
        values[4], line.ifgroup);
    // Editor-only presets created before branch metadata existed can still be
    // shared by coordinates. Prefer the Object branch, then use that legacy
    // compatibility path.
    if (imageIndex >= arr_IMG.count)
        imageIndex = FindIMG(values[0], values[1], values[2], values[3],
            values[4]);
    return imageIndex >= 0 && imageIndex < arr_IMG.count ? imageIndex : -1;
}

int WORKSPACE::FindImageAssetForObject(int modelIndex) {
    const std::vector<SEObjectInstance>& objects = objectEditorModel.Objects();
    if (modelIndex < 0 || modelIndex >= (int)objects.size()) return -1;
    for (int row : objects[modelIndex].rows) {
        const int imageIndex = FindImageAssetForRow(row);
        if (imageIndex >= 0) return imageIndex;
    }
    return -1;
}

void WORKSPACE::BuildImageAssetUsage(
    std::vector<std::vector<int>>& usage) {
    usage.clear();
    usage.resize((std::max)(0, arr_IMG.count));
    if (arr_IMG.count <= 0) return;

    // The declaring row is the stable and cheapest link between an IMG crop
    // and an Object SRC command. Keep the schema-based fallback for legacy
    // crops that predate sourceDeclare/editorDeclare metadata.
    std::map<int, int> assetByRow;
    for (int imageIndex = 0; imageIndex < arr_IMG.count; ++imageIndex) {
        IMG& asset = ((IMG*)arr_IMG.data)[imageIndex];
        if (asset.sourceDeclare >= 0 &&
            assetByRow.find(asset.sourceDeclare) == assetByRow.end())
            assetByRow[asset.sourceDeclare] = imageIndex;
        if (asset.editorDeclare >= 0 &&
            assetByRow.find(asset.editorDeclare) == assetByRow.end())
            assetByRow[asset.editorDeclare] = imageIndex;
    }

    const std::vector<SEObjectInstance>& objects = objectEditorModel.Objects();
    for (int modelIndex = 0; modelIndex < (int)objects.size(); ++modelIndex) {
        std::vector<int> objectAssets;
        for (int row : objects[modelIndex].rows) {
            int imageIndex = -1;
            const std::map<int, int>::const_iterator direct = assetByRow.find(row);
            if (direct != assetByRow.end()) {
                imageIndex = direct->second;
            } else if (row >= 0 && row < skinfileLines.count) {
                SKINFILELINEREAD& line =
                    ((SKINFILELINEREAD*)skinfileLines.data)[row];
                const char* command = line.csv.str[0].body
                    ? line.csv.str[0].outstr() : "";
                if (!strncmp(command, "#SRC", 4))
                    imageIndex = FindImageAssetForRow(row);
            }
            if (imageIndex < 0 || imageIndex >= arr_IMG.count ||
                std::find(objectAssets.begin(), objectAssets.end(), imageIndex) !=
                    objectAssets.end()) continue;
            objectAssets.push_back(imageIndex);
            usage[imageIndex].push_back(modelIndex);
        }
    }
}

const std::vector<std::vector<int>>& WORKSPACE::ImageAssetUsage() {
    if (imageAssetUsageCacheGeneration != imageAssetUsageGeneration ||
        imageAssetUsageCacheAssetCount != arr_IMG.count) {
        BuildImageAssetUsage(imageAssetUsageCache);
        imageAssetUsageCacheGeneration = imageAssetUsageGeneration;
        imageAssetUsageCacheAssetCount = arr_IMG.count;
    }
    return imageAssetUsageCache;
}

bool WORKSPACE::SynchronizeImageManagerToObject(int modelIndex) {
    const int imageIndex = FindImageAssetForObject(modelIndex);
    if (imageIndex < 0 || !SelectIMGAsset(imageIndex, false)) return false;
    // Do not force the Image Manager window open. If it is already available,
    // this one-shot request scrolls the selected crop into view when drawn.
    imageManagerFocusRequest = imageIndex;
    return true;
}

bool WORKSPACE::SetImageManagerHoveredObject(int modelIndex, int frameCount) {
    imageManagerHoveredAssetIndex = FindImageAssetForObject(modelIndex);
    imageManagerHoveredAssetFrame = frameCount;
    return imageManagerHoveredAssetIndex >= 0;
}


//int WORKSPACE::MoveObject() {
//    
//    return 0;
//}

int WORKSPACE::InsertLine(int pos) {

    if (pos < 0) pos = 0;
    if (pos > skinfileLines.count) pos = skinfileLines.count;

    SKINFILELINEREAD* line = (SKINFILELINEREAD*)skinfileLines.Get_newAt(pos);
    line->line.assign("newline");
    line->isComment = true;
    line->modified = true;
    line->num = pos;

    if (!applyingHistory) {
        HISTORY* hs = (HISTORY*)arr_history.Get_new();
        hs->op = insertLine;
        hs->target = pos;
    }

    for (int i = 0; i < skinfileLines.count; ++i)
        ((SKINFILELINEREAD*)skinfileLines.data)[i].numTotal = i;
    for (int i = 0; i < arr_SRC.count; ++i)
        if (((SRC*)arr_SRC.data)[i].declare >= pos) ++((SRC*)arr_SRC.data)[i].declare;
    for (int i = 0; i < arr_DST.count; ++i)
        if (((DST*)arr_DST.data)[i].declare >= pos) ++((DST*)arr_DST.data)[i].declare;
    for (int i = 0; i < arr_SRCGR.count; ++i)
        if (((SRCGR*)arr_SRCGR.data)[i].declare >= pos)
            ++((SRCGR*)arr_SRCGR.data)[i].declare;
    for (int i = 0; i < arr_IMG.count; ++i) {
        IMG& image = ((IMG*)arr_IMG.data)[i];
        if (image.sourceDeclare >= pos) ++image.sourceDeclare;
        if (image.editorDeclare >= pos) ++image.editorDeclare;
    }

    NotifyDocumentChanged(DOCUMENT_CHANGE_STRUCTURE);

    return 0;
}

SEObjectSelectionKey WORKSPACE::MakeObjectSelectionKey(int modelIndex) const {
    SEObjectSelectionKey key;
    const std::vector<SEObjectInstance>& objects = objectEditorModel.Objects();
    if (modelIndex < 0 || modelIndex >= (int)objects.size()) return key;
    const SEObjectInstance& object = objects[modelIndex];
    key.editorId = object.editorId;
    key.group = object.group;
    key.anchorRow = object.rows.empty() ? -1 : object.rows.front();
    return key;
}

int WORKSPACE::ResolveObjectSelectionKey(const SEObjectSelectionKey& key) const {
    if (!key.IsValid()) return -1;
    const std::vector<SEObjectInstance>& objects = objectEditorModel.Objects();
    if (!key.editorId.empty()) {
        for (int index = 0; index < (int)objects.size(); ++index)
            if (objects[index].editorId == key.editorId) return index;
    }
    if (key.anchorRow >= 0) {
        for (int index = 0; index < (int)objects.size(); ++index) {
            const SEObjectInstance& candidate = objects[index];
            if (key.group >= 0 && candidate.group != key.group) continue;
            if (std::find(candidate.rows.begin(), candidate.rows.end(), key.anchorRow) !=
                candidate.rows.end()) return index;
        }
    }
    return -1;
}

void WORKSPACE::ClearObjectSelection() {
    objectSelection.selected.clear();
    objectSelection.active = SEObjectSelectionKey();
    objectSelection.anchor = SEObjectSelectionKey();
    objectSelection.focusBrowserRequested = false;
    preview_selected_object_model_indices.clear();
    preview_selected_object_model_index = -1;
    preview_selection_anchor_model_index = -1;
    object_editor_select_request = -1;
    preview_selected_obj_valid = false;
    preview_selected_obj_last_valid = false;
}

void WORKSPACE::SetObjectSelection(const std::vector<int>& modelIndices,
    int activeModelIndex, int anchorModelIndex, bool requestBrowserFocus) {
    const std::vector<int> requestedIndices = modelIndices;
    objectSelection.selected.clear();
    preview_selected_object_model_indices.clear();
    for (int modelIndex : requestedIndices) {
        const SEObjectSelectionKey key = MakeObjectSelectionKey(modelIndex);
        if (!key.IsValid()) continue;
        if (std::find(preview_selected_object_model_indices.begin(),
            preview_selected_object_model_indices.end(), modelIndex) !=
            preview_selected_object_model_indices.end()) continue;
        objectSelection.selected.push_back(key);
        preview_selected_object_model_indices.push_back(modelIndex);
    }
    if (activeModelIndex < 0 && !preview_selected_object_model_indices.empty())
        activeModelIndex = preview_selected_object_model_indices.front();
    if (anchorModelIndex < 0) anchorModelIndex = activeModelIndex;
    objectSelection.active = MakeObjectSelectionKey(activeModelIndex);
    objectSelection.anchor = MakeObjectSelectionKey(anchorModelIndex);
    objectSelection.focusBrowserRequested = requestBrowserFocus;
    preview_selected_object_model_index = ResolveObjectSelectionKey(objectSelection.active);
    preview_selection_anchor_model_index = ResolveObjectSelectionKey(objectSelection.anchor);
    if (requestBrowserFocus && preview_selected_object_model_index >= 0)
        object_editor_select_request = preview_selected_object_model_index;
    if (preview_selected_object_model_index >= 0)
        SynchronizeImageManagerToObject(preview_selected_object_model_index);
}

void WORKSPACE::RestoreObjectSelection() {
    preview_selected_object_model_indices.clear();
    std::vector<SEObjectSelectionKey> resolvedKeys;
    for (const SEObjectSelectionKey& key : objectSelection.selected) {
        const int modelIndex = ResolveObjectSelectionKey(key);
        if (modelIndex < 0) continue;
        if (std::find(preview_selected_object_model_indices.begin(),
            preview_selected_object_model_indices.end(), modelIndex) ==
            preview_selected_object_model_indices.end()) {
            preview_selected_object_model_indices.push_back(modelIndex);
            resolvedKeys.push_back(MakeObjectSelectionKey(modelIndex));
        }
    }
    objectSelection.selected.swap(resolvedKeys);
    preview_selected_object_model_index = ResolveObjectSelectionKey(objectSelection.active);
    if (preview_selected_object_model_index < 0 && !preview_selected_object_model_indices.empty()) {
        preview_selected_object_model_index = preview_selected_object_model_indices.front();
        objectSelection.active = MakeObjectSelectionKey(preview_selected_object_model_index);
    }
    if (preview_selected_object_model_index < 0)
        objectSelection.active = SEObjectSelectionKey();
    preview_selection_anchor_model_index = ResolveObjectSelectionKey(objectSelection.anchor);
    if (preview_selection_anchor_model_index < 0)
        preview_selection_anchor_model_index = preview_selected_object_model_index;
    objectSelection.anchor = MakeObjectSelectionKey(preview_selection_anchor_model_index);
    if (objectSelection.focusBrowserRequested && preview_selected_object_model_index >= 0) {
        object_editor_select_request = preview_selected_object_model_index;
        objectSelection.focusBrowserRequested = false;
    }
    if (preview_selected_object_model_index < 0) {
        preview_selected_obj_valid = false;
        preview_selected_obj_last_valid = false;
    } else SynchronizeImageManagerToObject(preview_selected_object_model_index);
}

void WORKSPACE::RebuildObjectModel() {
    // Capture legacy index writers before rebuilding. New code writes the
    // stable selection directly through SetObjectSelection().
    if (objectSelection.selected.empty() &&
        !preview_selected_object_model_indices.empty()) {
        std::vector<SEObjectSelectionKey> selectedKeys;
        for (int modelIndex : preview_selected_object_model_indices) {
            const SEObjectSelectionKey key = MakeObjectSelectionKey(modelIndex);
            if (key.IsValid()) selectedKeys.push_back(key);
        }
        objectSelection.selected = selectedKeys;
        objectSelection.active = MakeObjectSelectionKey(preview_selected_object_model_index);
        objectSelection.anchor = MakeObjectSelectionKey(preview_selection_anchor_model_index);
    }
    objectEditorModel.Rebuild(*this);
    ++imageAssetUsageGeneration;
    objectEditorLastLineCount = skinfileLines.count;
    objectModelRebuildPending = false;
    InvalidateSimpleModeProjection();
    RestoreObjectSelection();
}

SkinDocumentSnapshot WORKSPACE::CaptureDocumentSnapshot() const {
    SkinDocumentSnapshot snapshot;
    snapshot.lines.reserve(skinfileLines.count);
    for (int row = 0; row < skinfileLines.count; ++row) {
        const SKINFILELINEREAD& source =
            ((const SKINFILELINEREAD*)skinfileLines.data)[row];
        SkinLineSnapshot line;
        line.filename = source.filename.body ? source.filename.body : "";
        line.line = source.line.body ? source.line.body : "";
        line.num = source.num;
        line.modified = source.modified;
        line.show = source.show;
        snapshot.lines.push_back(line);
    }
    snapshot.selection = objectSelection;
    if (snapshot.selection.selected.empty() &&
        !preview_selected_object_model_indices.empty()) {
        for (int modelIndex : preview_selected_object_model_indices) {
            const SEObjectSelectionKey key = MakeObjectSelectionKey(modelIndex);
            if (key.IsValid()) snapshot.selection.selected.push_back(key);
        }
        snapshot.selection.active =
            MakeObjectSelectionKey(preview_selected_object_model_index);
        snapshot.selection.anchor =
            MakeObjectSelectionKey(preview_selection_anchor_model_index);
    }
    return snapshot;
}

int WORKSPACE::RestoreDocumentSnapshot(const SkinDocumentSnapshot& snapshot) {
    // A Simple Mode import can append #IMAGE and $SRC_IMAGE rows. Resize the
    // live document before restoring so the whole batch remains one undo step.
    // InsertLine/DeleteLine still maintain derived declaration indices; the
    // final structure notification schedules a full parser rebuild.
    const bool previousApplyingHistory = applyingHistory;
    applyingHistory = true;
    while (skinfileLines.count > (int)snapshot.lines.size()) {
        if (DeleteLine(skinfileLines.count - 1) != 0) {
            applyingHistory = previousApplyingHistory;
            return -1;
        }
    }
    while (skinfileLines.count < (int)snapshot.lines.size()) {
        if (InsertLine(skinfileLines.count) != 0) {
            applyingHistory = previousApplyingHistory;
            return -1;
        }
    }

    for (int row = 0; row < skinfileLines.count; ++row) {
        SKINFILELINEREAD& destination =
            ((SKINFILELINEREAD*)skinfileLines.data)[row];
        const SkinLineSnapshot& source = snapshot.lines[row];
        destination.filename.assign(source.filename.c_str());
        destination.line.assign(source.line.c_str());
        destination.num = source.num;
        destination.numTotal = row;
        destination.modified = source.modified;
        destination.show = source.show;
        destination.isComment = source.line.empty() || source.line[0] != '#';
        destination.isSEcomment = !source.line.empty() && source.line[0] == '$';
        for (int column = 0; column < 30; ++column) {
            destination.csv.str[column].fillzero();
            destination.csv.val[column] = 0;
        }
        if (!destination.isComment) {
            SplitCSV(destination.line, &destination.csv, ",");
            destination.csvColumnCount = CountCsvColumns(destination.line);
        } else {
            destination.csvColumnCount = 0;
        }

        // These are derived by ParseSkin. Clearing them prevents consumers
        // from mistaking the previous row occupant for the restored line.
        destination.ifgroup = 0;
        destination.isIfGroupHead = false;
        destination.isIfGroupEnd = false;
        destination.isGroupHead = false;
        destination.isGroupEnd = false;
        destination.isObjectHead = false;
        destination.isObjectEnd = false;
        destination.objID = -1;
        destination.objType = -1;
        destination.objInTypeID = -1;
        destination.isSRC = false;
        destination.isDST = false;
        destination.isOther = false;
    }

    objectSelection = snapshot.selection;
    preview_selected_object_model_indices.clear();
    preview_selected_object_model_index = -1;
    preview_selection_anchor_model_index = -1;
    object_editor_select_request = -1;
    preview_selected_obj_valid = false;
    preview_selected_obj_last_valid = false;
    applyingHistory = previousApplyingHistory;
    NotifyDocumentChanged(DOCUMENT_CHANGE_STRUCTURE);
    return 0;
}

static bool GetObjectReorderOwner(const WORKSPACE& workspace,
    int modelIndex, std::string& owner) {
    owner.clear();
    const std::vector<SEObjectInstance>& objects =
        workspace.objectEditorModel.Objects();
    if (modelIndex < 0 || modelIndex >= (int)objects.size() ||
        objects[modelIndex].rows.empty()) return false;
    const int row = objects[modelIndex].rows.front();
    if (row < 0 || row >= workspace.skinfileLines.count) return false;
    const SKINFILELINEREAD& line =
        ((const SKINFILELINEREAD*)workspace.skinfileLines.data)[row];
    owner = line.filename.body && *line.filename.body
        ? line.filename.body : workspace.mainpath;
    return !owner.empty();
}

bool WORKSPACE::CanReorderObject(int sourceModelIndex, int targetModelIndex) const {
    const std::vector<SEObjectInstance>& objects = objectEditorModel.Objects();
    if (sourceModelIndex < 0 || targetModelIndex < 0 ||
        sourceModelIndex >= (int)objects.size() ||
        targetModelIndex >= (int)objects.size() ||
        sourceModelIndex == targetModelIndex) return false;

    auto objectBranch = [&](const SEObjectInstance& object) -> int {
        if (object.ifgroup != 0 || object.rows.empty()) return object.ifgroup;
        const int row = object.rows.front();
        return row >= 0 && row < skinfileLines.count
            ? ((const SKINFILELINEREAD*)skinfileLines.data)[row].ifgroup : 0;
    };
    std::string sourceOwner;
    std::string targetOwner;
    if (!GetObjectReorderOwner(*this, sourceModelIndex, sourceOwner) ||
        !GetObjectReorderOwner(*this, targetModelIndex, targetOwner))
        return false;

    const int sourceBranch = objectBranch(objects[sourceModelIndex]);
    const int targetBranch = objectBranch(objects[targetModelIndex]);
    for (int row : objects[sourceModelIndex].rows) {
        if (row < 0 || row >= skinfileLines.count) return false;
        const SKINFILELINEREAD& line =
            ((const SKINFILELINEREAD*)skinfileLines.data)[row];
        const char* owner = line.filename.body ? line.filename.body : mainpath;
        if (_stricmp(owner, sourceOwner.c_str()) != 0 ||
            line.ifgroup != sourceBranch) return false;
    }
    for (int row : objects[targetModelIndex].rows) {
        if (row < 0 || row >= skinfileLines.count) return false;
        const SKINFILELINEREAD& line =
            ((const SKINFILELINEREAD*)skinfileLines.data)[row];
        const char* owner = line.filename.body ? line.filename.body : mainpath;
        if (_stricmp(owner, targetOwner.c_str()) != 0 ||
            line.ifgroup != targetBranch) return false;
    }
    return true;
}

bool WORKSPACE::ObjectReorderRequiresConfirmation(int sourceModelIndex,
    int targetModelIndex) const {
    std::string sourceOwner;
    std::string targetOwner;
    return CanReorderObject(sourceModelIndex, targetModelIndex) &&
        GetObjectReorderOwner(*this, sourceModelIndex, sourceOwner) &&
        GetObjectReorderOwner(*this, targetModelIndex, targetOwner) &&
        _stricmp(sourceOwner.c_str(), targetOwner.c_str()) != 0;
}

bool WORKSPACE::QueueObjectReorder(int sourceModelIndex, int targetModelIndex,
    bool placeAfter) {
    if (!CanReorderObject(sourceModelIndex, targetModelIndex)) return false;
    CancelPendingObjectReorder();
    pendingObjectReorderSource = MakeObjectSelectionKey(sourceModelIndex);
    pendingObjectReorderTarget = MakeObjectSelectionKey(targetModelIndex);
    if (!pendingObjectReorderSource.IsValid() ||
        !pendingObjectReorderTarget.IsValid()) {
        CancelPendingObjectReorder();
        return false;
    }
    if (!GetObjectReorderOwner(*this, sourceModelIndex,
            pendingObjectReorderSourceOwner) ||
        !GetObjectReorderOwner(*this, targetModelIndex,
            pendingObjectReorderTargetOwner)) {
        CancelPendingObjectReorder();
        return false;
    }
    pendingObjectReorderAfter = placeAfter;
    if (_stricmp(pendingObjectReorderSourceOwner.c_str(),
            pendingObjectReorderTargetOwner.c_str()) != 0) {
        objectReorderConfirmationPending = true;
        objectReorderConfirmDialogRequested = true;
    } else {
        pendingObjectReorder = true;
    }
    return true;
}

bool WORKSPACE::ConfirmPendingObjectReorder() {
    if (!objectReorderConfirmationPending) return false;
    const int sourceModelIndex =
        ResolveObjectSelectionKey(pendingObjectReorderSource);
    const int targetModelIndex =
        ResolveObjectSelectionKey(pendingObjectReorderTarget);
    if (!CanReorderObject(sourceModelIndex, targetModelIndex)) {
        CancelPendingObjectReorder();
        return false;
    }
    objectReorderConfirmationPending = false;
    objectReorderConfirmDialogRequested = false;
    pendingObjectReorder = true;
    return true;
}

void WORKSPACE::CancelPendingObjectReorder() {
    pendingObjectReorder = false;
    pendingObjectReorderAfter = false;
    pendingObjectReorderSource = SEObjectSelectionKey();
    pendingObjectReorderTarget = SEObjectSelectionKey();
    objectReorderConfirmationPending = false;
    objectReorderConfirmDialogRequested = false;
    pendingObjectReorderSourceOwner.clear();
    pendingObjectReorderTargetOwner.clear();
}

int WORKSPACE::ApplyPendingObjectReorder() {
    if (!pendingObjectReorder) return 0;
    pendingObjectReorder = false;

    const int sourceModelIndex =
        ResolveObjectSelectionKey(pendingObjectReorderSource);
    const int targetModelIndex =
        ResolveObjectSelectionKey(pendingObjectReorderTarget);
    if (!CanReorderObject(sourceModelIndex, targetModelIndex)) {
        CancelPendingObjectReorder();
        return -1;
    }

    std::string sourceOwner;
    std::string targetOwner;
    if (!GetObjectReorderOwner(*this, sourceModelIndex, sourceOwner) ||
        !GetObjectReorderOwner(*this, targetModelIndex, targetOwner)) {
        CancelPendingObjectReorder();
        return -1;
    }
    const bool changesOwner =
        _stricmp(sourceOwner.c_str(), targetOwner.c_str()) != 0;

    const std::vector<SEObjectInstance>& objects = objectEditorModel.Objects();
    const SEObjectInstance& sourceObject = objects[sourceModelIndex];
    const SEObjectInstance& targetObject = objects[targetModelIndex];
    if (sourceObject.rows.empty() || targetObject.rows.empty()) {
        CancelPendingObjectReorder();
        return -1;
    }

    auto collectObjectRows = [&](const SEObjectInstance& object) {
        std::vector<int> rows = object.rows;
        const int firstObjectRow = *std::min_element(object.rows.begin(), object.rows.end());
        for (int row = firstObjectRow - 1; row >= 0; --row) {
            const SKINFILELINEREAD& metadata =
                ((const SKINFILELINEREAD*)skinfileLines.data)[row];
            const char* text = metadata.line.body ? metadata.line.body : "";
            if (strncmp(text, "$SE_OBJECT_ID,", 14) == 0 ||
                strncmp(text, "$SE_OBJECT_NAME,", 16) == 0) {
                rows.push_back(row);
                continue;
            }
            if (*text == '\0' || strncmp(text, "//", 2) == 0) continue;
            break;
        }
        std::sort(rows.begin(), rows.end());
        rows.erase(std::unique(rows.begin(), rows.end()), rows.end());
        return rows;
    };

    const std::vector<int> sourceRows = collectObjectRows(sourceObject);
    const std::vector<int> targetRows = collectObjectRows(targetObject);
    if (sourceRows.empty() || targetRows.empty()) {
        CancelPendingObjectReorder();
        return -1;
    }

    std::vector<bool> moved(skinfileLines.count, false);
    for (int row : sourceRows) {
        if (row < 0 || row >= skinfileLines.count) {
            CancelPendingObjectReorder();
            return -1;
        }
        moved[row] = true;
    }
    const int targetBoundary = pendingObjectReorderAfter
        ? targetRows.back() + 1 : targetRows.front();

    std::vector<int> order;
    order.reserve(skinfileLines.count);
    int insertAt = 0;
    for (int row = 0; row < skinfileLines.count; ++row) {
        if (row < targetBoundary && !moved[row]) ++insertAt;
        if (!moved[row]) order.push_back(row);
    }
    order.insert(order.begin() + insertAt, sourceRows.begin(), sourceRows.end());
    bool changed = false;
    for (int row = 0; row < skinfileLines.count; ++row) {
        if (order[row] != row) { changed = true; break; }
    }
    if (!changed) {
        CancelPendingObjectReorder();
        return 0;
    }

    SkinDocumentSnapshot before = CaptureDocumentSnapshot();
    SkinDocumentSnapshot reordered;
    reordered.lines.reserve(before.lines.size());
    std::vector<int> oldToNew(order.size(), -1);
    for (int newRow = 0; newRow < (int)order.size(); ++newRow) {
        const int oldRow = order[newRow];
        reordered.lines.push_back(before.lines[oldRow]);
        if (changesOwner && moved[oldRow]) {
            reordered.lines.back().filename = targetOwner;
            reordered.lines.back().modified = true;
        }
        oldToNew[oldRow] = newRow;
    }
    reordered.selection = before.selection;
    auto remapKey = [&](SEObjectSelectionKey& key) {
        if (!key.editorId.empty() || key.anchorRow < 0 ||
            key.anchorRow >= (int)oldToNew.size()) return;
        key.anchorRow = oldToNew[key.anchorRow];
    };
    for (SEObjectSelectionKey& key : reordered.selection.selected) remapKey(key);
    remapKey(reordered.selection.active);
    remapKey(reordered.selection.anchor);
    reordered.selection.focusBrowserRequested = true;

    const int snapshotIndex = (int)historyDocumentSnapshots.size();
    historyDocumentSnapshots.push_back(before);
    HISTORY* history = (HISTORY*)arr_history.Get_new();
    history->op = moveLine;
    history->target = snapshotIndex;
    CancelPendingObjectReorder();
    return RestoreDocumentSnapshot(reordered);
}

void WORKSPACE::NotifyDocumentChanged(unsigned int changes) {
    // A new user edit starts a new history branch. Undo/redo replays set
    // replayingHistory so restoring an older document does not discard the
    // remaining forward states.
    if (!replayingHistory) redoDocumentSnapshots.clear();
    ++documentRevision;
    InvalidateSimpleModeProjection();
    const unsigned long long now = GetTickCount64();
    if (changes & (DOCUMENT_CHANGE_VALUE | DOCUMENT_CHANGE_STRUCTURE)) {
        editorDerivedRebuildPending = true;
        editorDerivedRebuildRequestedAt =
            (changes & DOCUMENT_CHANGE_STRUCTURE) ? 0 : now;
        previewReloadPending = true;
        previewReloadRequestedAt = now;
    }
    if (changes & (DOCUMENT_CHANGE_VALUE | DOCUMENT_CHANGE_STRUCTURE |
        DOCUMENT_CHANGE_OBJECT_METADATA))
        objectModelRebuildPending = true;
}

bool WORKSPACE::IsDocumentDirty() const {
    return documentRevision != savedDocumentRevision;
}

void WORKSPACE::MarkDocumentSaved() {
    savedDocumentRevision = documentRevision;
    for (int row = 0; row < skinfileLines.count; ++row)
        ((SKINFILELINEREAD*)skinfileLines.data)[row].modified = false;
    lastSaveState = 1;
    lastSaveMessage = "Saved";
    lastSaveMessageAt = GetTickCount64();
}

int WORKSPACE::SaveCurrentSkin() {
    if (!loaded || !mainpath[0]) {
        lastSaveState = -1;
        lastSaveMessage = "No loaded skin to save";
        lastSaveMessageAt = GetTickCount64();
        return -1;
    }

    // Normal Save preserves the current include structure. Save As remains the
    // explicit place to choose whether scripts should be merged.
    if (SaveSkinScript(mainpath, true, false) != 0) {
        lastSaveState = -1;
        lastSaveMessage = "Save failed; original files were preserved";
        lastSaveMessageAt = GetTickCount64();
        return -1;
    }
    MarkDocumentSaved();
    return 0;
}

int WORKSPACE::SetObjectName(int modelIndex, const char* name) {
    const std::vector<SEObjectInstance>& objects = objectEditorModel.Objects();
    if (!name || modelIndex < 0 || modelIndex >= (int)objects.size() ||
        objects[modelIndex].rows.empty()) return -1;

    const SEObjectInstance object = objects[modelIndex];
    const int firstRow = object.rows.front();
    int nameRow = -1;
    int idRow = -1;
    for (int row = firstRow - 1; row >= 0; --row) {
        SKINFILELINEREAD& metadata = ((SKINFILELINEREAD*)skinfileLines.data)[row];
        const char* text = metadata.line.body ? metadata.line.outstr() : "";
        if (strncmp(text, "$SE_OBJECT_NAME,", 16) == 0) nameRow = row;
        else if (strncmp(text, "$SE_OBJECT_ID,", 14) == 0) idRow = row;
        else if (*text == '#') break;
        else if (*text && *text != '$' && strncmp(text, "//", 2) != 0) break;
    }

    CSTR owner(((SKINFILELINEREAD*)skinfileLines.data)[firstRow].filename);
    const std::string nameLine = std::string("$SE_OBJECT_NAME,") + name;
    int finalNameRow = nameRow;
    if (nameRow >= 0) {
        CSTR oldLine(((SKINFILELINEREAD*)skinfileLines.data)[nameRow].line);
        if (EditLine(nameRow, oldLine, CSTR(nameLine.c_str())) != 0) return -1;
    } else {
        finalNameRow = idRow >= 0 ? idRow : firstRow;
        if (InsertLine(finalNameRow) != 0) return -1;
        ((SKINFILELINEREAD*)skinfileLines.data)[finalNameRow].filename.assign(owner);
        CSTR placeholder(((SKINFILELINEREAD*)skinfileLines.data)[finalNameRow].line);
        if (EditLine(finalNameRow, placeholder, CSTR(nameLine.c_str())) != 0) return -1;
    }

    if (object.editorId.empty()) {
        char generatedId[64];
        snprintf(generatedId, sizeof(generatedId), "obj_%08llX_name", GetTickCount64());
        const int idInsertAt = finalNameRow + 1;
        if (InsertLine(idInsertAt) != 0) return -1;
        ((SKINFILELINEREAD*)skinfileLines.data)[idInsertAt].filename.assign(owner);
        const std::string idLine = std::string("$SE_OBJECT_ID,") + generatedId;
        CSTR idPlaceholder(((SKINFILELINEREAD*)skinfileLines.data)[idInsertAt].line);
        if (EditLine(idInsertAt, idPlaceholder, CSTR(idLine.c_str())) != 0) return -1;
        objectSelection.active.editorId = generatedId;
        objectSelection.active.group = object.group;
        objectSelection.active.anchorRow = firstRow + 2;
        objectSelection.selected.clear();
        objectSelection.selected.push_back(objectSelection.active);
    } else {
        objectSelection.active = MakeObjectSelectionKey(modelIndex);
    }
    return 0;
}

int WORKSPACE::DeleteObject(int modelIndex) {
    const std::vector<SEObjectInstance>& allObjects = objectEditorModel.Objects();
    if (modelIndex < 0 || modelIndex >= (int)allObjects.size()) return -1;

    // Copy the model entry before removing source rows. DeleteLine shifts every
    // following row and schedules the same editor/Preview rebuild used by the
    // existing CSV editing path.
    const SEObjectInstance deleting = allObjects[modelIndex];
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
    if (deleteRows.empty()) return -1;
    for (std::vector<int>::reverse_iterator it = deleteRows.rbegin();
        it != deleteRows.rend(); ++it) {
        if (DeleteLine(*it) != 0) return -1;
    }

    ClearObjectSelection();
    RebuildObjectModel();
    selected_object_editor = 0;
    return 0;
}

int WORKSPACE::DeleteLine(int pos) {

    if (pos < 0 || pos >= skinfileLines.count) return -1;

    if (!applyingHistory) {
        HISTORY* hs = (HISTORY*)arr_history.Get_new();
        hs->op = removeLine;
        hs->target = pos;
        hs->older.line.assign(((SKINFILELINEREAD*)skinfileLines.data)[pos].line);
    }

    skinfileLines.DeleteAt(pos);

    for (int i = 0; i < skinfileLines.count; ++i)
        ((SKINFILELINEREAD*)skinfileLines.data)[i].numTotal = i;
    for (int i = 0; i < arr_SRC.count; ++i) {
        SRC& src = ((SRC*)arr_SRC.data)[i];
        if (src.declare == pos) src.declare = -1;
        else if (src.declare > pos) --src.declare;
    }
    for (int i = 0; i < arr_DST.count; ++i) {
        DST& dst = ((DST*)arr_DST.data)[i];
        if (dst.declare == pos) dst.declare = -1;
        else if (dst.declare > pos) --dst.declare;
    }
    for (int i = 0; i < arr_SRCGR.count; ++i) {
        SRCGR& graphic = ((SRCGR*)arr_SRCGR.data)[i];
        if (graphic.declare == pos) graphic.declare = -1;
        else if (graphic.declare > pos) --graphic.declare;
    }
    for (int i = 0; i < arr_IMG.count; ++i) {
        IMG& image = ((IMG*)arr_IMG.data)[i];
        if (image.sourceDeclare == pos) image.sourceDeclare = -1;
        else if (image.sourceDeclare > pos) --image.sourceDeclare;
        if (image.editorDeclare == pos) image.editorDeclare = -1;
        else if (image.editorDeclare > pos) --image.editorDeclare;
    }
    NotifyDocumentChanged(DOCUMENT_CHANGE_STRUCTURE);

    return 0;
}
int WORKSPACE::EditLine(int pos, CSTR oldlinebody, CSTR newlinebody) {
    if (pos < 0 || pos >= skinfileLines.count) return -1;
    const char* oldText = oldlinebody.body ? oldlinebody.outstr() : "";
    const char* newText = newlinebody.body ? newlinebody.outstr() : "";
    const bool editorMetadata =
        strncmp(oldText, "$SE_", 4) == 0 && strncmp(newText, "$SE_", 4) == 0;
    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[pos];
    line.line.assign(newlinebody);

    line.isComment = (*line.line.atPos(0) != '#');
    line.isSEcomment = (*line.line.atPos(0) == '$');

    if (!line.isComment) {
        SplitCSV(line.line, &line.csv, ",");
        line.csvColumnCount = CountCsvColumns(line.line);
    } else {
        line.csvColumnCount = 0;
    }
    line.modified = true;

    HISTORY* hs = applyingHistory ? NULL : (HISTORY*)arr_history.Get_new();
    if (hs) {
        hs->op = overwriteLine;
        hs->target = pos;
    }
    CsvToLine(pos);
    if (hs) {
        hs->older.line.assign(oldlinebody);
        hs->newer.line.assign(newlinebody);
    }

    NotifyDocumentChanged(editorMetadata
        ? DOCUMENT_CHANGE_OBJECT_METADATA : DOCUMENT_CHANGE_STRUCTURE);

    return 0;
}

namespace {

bool IsCommandIndexField(const char* command, int column) {
    if (!command || !*command || column <= 0 || column >= 30) return false;
    CSTR help = GetCommandHelp(command, column);
    help.trimWhiteSpace();
    const char* label = help.body ? help.outstr() : "";
    if (*label == '$') ++label;
    return _stricmp(label, "index") == 0;
}

int FindCommandIndexColumn(const char* command) {
    for (int column = 1; column < 30; ++column)
        if (IsCommandIndexField(command, column)) return column;
    return -1;
}

bool CommandBelongsToObjectGroup(const SEObjectGroupDef* group,
    const char* command) {
    if (!group || !command || !*command) return false;
    return std::find(group->commands.begin(), group->commands.end(),
        command) != group->commands.end();
}

void CollectLinkedObjectIndexTargets(WORKSPACE& workspace, int sourceRow,
    int sourceColumn, std::vector<std::pair<int, int> >& targets) {
    targets.clear();
    targets.push_back(std::make_pair(sourceRow, sourceColumn));

    SKINFILELINEREAD& source =
        ((SKINFILELINEREAD*)workspace.skinfileLines.data)[sourceRow];
    const char* sourceCommand = source.csv.str[0].body
        ? source.csv.str[0].outstr() : "";
    if (!IsCommandIndexField(sourceCommand, sourceColumn)) return;

    const std::vector<SEObjectInstance>& objects =
        workspace.objectEditorModel.Objects();
    const int modelIndex = SEFindObjectForRow(objects, sourceRow);
    if (modelIndex < 0 || modelIndex >= (int)objects.size()) return;

    const SEObjectInstance& object = objects[modelIndex];
    const auto appendTarget = [&](int row) {
        if (row < 0 || row >= workspace.skinfileLines.count) return;
        SKINFILELINEREAD& candidate =
            ((SKINFILELINEREAD*)workspace.skinfileLines.data)[row];
        const char* command = candidate.csv.str[0].body
            ? candidate.csv.str[0].outstr() : "";
        const int indexColumn = FindCommandIndexColumn(command);
        if (indexColumn < 0) return;
        const std::pair<int, int> target(row, indexColumn);
        if (std::find(targets.begin(), targets.end(), target) == targets.end())
            targets.push_back(target);
    };
    for (int row : object.rows) appendTarget(row);

    // A document edited by an older build may already have a SRC index which
    // no longer matches its following DST rows. In that state the Object model
    // necessarily exposes a source-only instance. Recover the immediately
    // following destination run from the same command group so changing the
    // SRC index again repairs, rather than further splits, the copied Object.
    if (targets.size() == 1 && strncmp(sourceCommand, "#SRC", 4) == 0) {
        const SEObjectGroupDef* group =
            workspace.objectEditorModel.Group(object.group);
        const std::string owner = source.filename.body
            ? source.filename.outstr() : "";
        for (int row = sourceRow + 1; row < workspace.skinfileLines.count;
            ++row) {
            SKINFILELINEREAD& candidate =
                ((SKINFILELINEREAD*)workspace.skinfileLines.data)[row];
            const std::string candidateOwner = candidate.filename.body
                ? candidate.filename.outstr() : "";
            if (_stricmp(owner.c_str(), candidateOwner.c_str()) != 0 ||
                candidate.ifgroup != source.ifgroup)
                break;

            const char* text = candidate.line.body
                ? candidate.line.outstr() : "";
            const char* command = candidate.csv.str[0].body
                ? candidate.csv.str[0].outstr() : "";
            if (!*text || strncmp(text, "//", 2) == 0) continue;
            if (strncmp(text, "$SE_OBJECT_ID,", 14) == 0 ||
                strncmp(text, "$SE_OBJECT_NAME,", 16) == 0)
                break;
            if (!CommandBelongsToObjectGroup(group, command)) break;
            if (strncmp(command, "#SRC", 4) == 0) break;
            if (strncmp(command, "#DST", 4) != 0) break;
            appendTarget(row);
        }
    }
}

} // namespace

int WORKSPACE::EditValue(int pos, int column, const char* newVal) {
    if (pos < 0 || pos >= skinfileLines.count || column < 0 || column >= 30 ||
        newVal == NULL)
        return -1;

    std::vector<std::pair<int, int> > targets;
    CollectLinkedObjectIndexTargets(*this, pos, column, targets);
    int historyEditCount = 0;
    bool structuralChange = false;
    for (const std::pair<int, int>& target : targets) {
        SKINFILELINEREAD& line =
            ((SKINFILELINEREAD*)skinfileLines.data)[target.first];
        CSTR oldLine(line.line);

        line.csv.str[target.second].assign(newVal);
        line.csv.val[target.second] = atol(newVal);
        if (line.csvColumnCount < target.second + 1)
            line.csvColumnCount = target.second + 1;
        line.modified = true;
        CsvToLine(target.first);

        if (!applyingHistory) {
            HISTORY* history = (HISTORY*)arr_history.Get_new();
            if (history) {
                history->op = overwriteLine;
                history->target = target.first;
                history->older.line.assign(oldLine);
                history->newer.line.assign(line.line);
                ++historyEditCount;
            }
        }
        if (target.second == 0) structuralChange = true;
    }
    if (!applyingHistory && historyEditCount > 1) {
        HISTORY* grouped = (HISTORY*)arr_history.Get_new();
        if (grouped) {
            grouped->op = group;
            grouped->target = historyEditCount;
        }
    }

    NotifyDocumentChanged(structuralChange
        ? DOCUMENT_CHANGE_STRUCTURE : DOCUMENT_CHANGE_VALUE);
    return 0;
}

int WORKSPACE::EditValue(int pos, int column, int newVal) {
    char value[16];
    _snprintf_s(value, sizeof(value), _TRUNCATE, "%d", newVal);
    return EditValue(pos, column, value);
}

static int UndoHistoryEntry(WORKSPACE& workspace) {
    ARR& arr_history = workspace.arr_history;
    if (arr_history.count <= 0) return -1;

    HISTORY& history = ((HISTORY*)arr_history.data)[arr_history.count - 1];
    const HISTORYOP operation = history.op;
    const int target = history.target;
    CSTR oldLine(history.older.line);
    --arr_history.count;

    // Batch commands append a group marker after their normal line edits.
    // Undo the recorded operations in reverse order so one Ctrl+Z restores
    // the complete action while each primitive keeps its existing logic.
    if (operation == group) {
        if (target <= 0 || target > arr_history.count) return -1;
        for (int edit = 0; edit < target; ++edit) {
            if (UndoHistoryEntry(workspace) != 0) return -1;
        }
        return 0;
    }

    workspace.applyingHistory = true;
    int result = 0;
    if (operation == overwriteLine) {
        if (target < 0 || target >= workspace.skinfileLines.count) result = -1;
        else {
            CSTR currentLine(
                ((SKINFILELINEREAD*)workspace.skinfileLines.data)[target].line);
            result = workspace.EditLine(target, currentLine, oldLine);
        }
    } else if (operation == insertLine) {
        result = workspace.DeleteLine(target);
    } else if (operation == removeLine) {
        result = workspace.InsertLine(target);
        if (result == 0) {
            CSTR insertedLine(
                ((SKINFILELINEREAD*)workspace.skinfileLines.data)[target].line);
            result = workspace.EditLine(target, insertedLine, oldLine);
        }
    } else if (operation == moveLine || operation == restoreDocument) {
        if (target >= 0 &&
            target < (int)workspace.historyDocumentSnapshots.size()) {
            // Undo can be requested after Preview/ImageManager have already
            // submitted texture commands this frame. Restore at the next
            // frame boundary so the derived texture arrays can be rebuilt
            // before any window draws.
            workspace.pendingHistorySnapshotRestore = target;
            workspace.pendingHistorySnapshotPreservesRedo = true;
            result = 0;
        } else {
            result = -1;
        }
    } else {
        result = -1;
    }
    workspace.applyingHistory = false;

    if (result == 0 && operation != moveLine && operation != restoreDocument)
        workspace.RestoreObjectSelection();
    return result;
}

int WORKSPACE::UndoLastEdit() {
    if (arr_history.count <= 0 || pendingHistorySnapshotRestore >= 0)
        return -1;

    const SkinDocumentSnapshot redoState = CaptureDocumentSnapshot();
    const bool previousReplayState = replayingHistory;
    replayingHistory = true;
    const int result = UndoHistoryEntry(*this);
    replayingHistory = previousReplayState;
    if (result == 0) redoDocumentSnapshots.push_back(redoState);
    return result;
}

int WORKSPACE::RedoLastEdit() {
    if (redoDocumentSnapshots.empty() || pendingHistorySnapshotRestore >= 0)
        return -1;

    const SkinDocumentSnapshot undoState = CaptureDocumentSnapshot();
    const SkinDocumentSnapshot redoState = redoDocumentSnapshots.back();
    redoDocumentSnapshots.pop_back();

    const int undoSnapshotIndex = (int)historyDocumentSnapshots.size();
    historyDocumentSnapshots.push_back(undoState);
    HISTORY* history = (HISTORY*)arr_history.Get_new();
    if (!history) {
        redoDocumentSnapshots.push_back(redoState);
        historyDocumentSnapshots.pop_back();
        return -1;
    }
    history->op = restoreDocument;
    history->target = undoSnapshotIndex;

    pendingHistorySnapshotRestore = (int)historyDocumentSnapshots.size();
    historyDocumentSnapshots.push_back(redoState);
    pendingHistorySnapshotPreservesRedo = true;
    return 0;
}

int WORKSPACE::ApplyPendingHistorySnapshotRestore() {
    if (pendingHistorySnapshotRestore < 0) return -1;
    const int snapshotIndex = pendingHistorySnapshotRestore;
    const bool preserveRedo = pendingHistorySnapshotPreservesRedo;
    pendingHistorySnapshotRestore = -1;
    pendingHistorySnapshotPreservesRedo = false;
    if (snapshotIndex >= (int)historyDocumentSnapshots.size()) return -1;

    const bool previousReplayState = replayingHistory;
    if (preserveRedo) replayingHistory = true;
    const int result = RestoreDocumentSnapshot(
        historyDocumentSnapshots[snapshotIndex]);
    replayingHistory = previousReplayState;
    return result;
}

int WORKSPACE::CsvToLine(int pos) {
    if (pos < 0 || pos >= skinfileLines.count) return -1;
    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[pos];

    line.isComment = (*line.line.atPos(0) != '#');
    line.isSEcomment = (*line.line.atPos(0) == '$');

    if (line.isComment || line.isSEcomment) {
        return 0;
    }

    CSTR buf("");

    cstrSprintf(&buf, "%s", line.csv.str[0]);
    int columnCount = line.csvColumnCount;
    if (columnCount < 1) columnCount = 1;
    if (columnCount > 30) columnCount = 30;
    for (int i = 1; i < columnCount; i++) {
        const char* value = (line.csv.str[i].body == NULL) ? "" : line.csv.str[i].outstr();
        if (line.csv.val[i] < 0 && value[0] != '-' && value[0] != '!')
            cstrSprintf(&buf, "%s,!%s", buf, value);
        else
            cstrSprintf(&buf, "%s,%s", buf, value);
    }

    line.line.assign(buf);
    return 0;
}

int CsvToCSTR(CSVbuf& csv, CSTR& line) {
    CSTR buf("");
    cstrSprintf(&buf, "%s", csv.str[0]);
    int columnCount = 30;
    while (columnCount > 1 && csv.str[columnCount - 1].body == NULL) --columnCount;
    for (int i = 1; i < columnCount; i++) {
        const char* value = (csv.str[i].body == NULL) ? "" : csv.str[i].outstr();
        if (csv.val[i] < 0 && value[0] != '-' && value[0] != '!')
            cstrSprintf(&buf, "%s,!%s", buf, value);
        else
            cstrSprintf(&buf, "%s,%s", buf, value);
    }

    line.assign(buf);
    return 0;
}
