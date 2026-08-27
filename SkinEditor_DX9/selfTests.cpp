#include "selfTests.h"

#include "../LR2/En_timer.h"
#include "../LR2/LR2_skinmanage.h"
#include "olrSkin.h"
#include "seHelper.h"
#include "seObjectEditor.h"
#include "skin.h"
#include "skinBrowser.h"
#include "skinPathResolver.h"
#include "skinResolution.h"
#include "uiCatalog.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <set>
#include <string>
#include <vector>

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

int RunResolutionEstimatorSelfTest() {
    SESkinResolutionDecision decision = SEResolveSkinResolution({
        "#INFORMATION,0,title,maker,,,1920,1080",
        "#RESOLUTION,1280,720",
        "#DST_IMAGE,0,0,0,0,3840,2160"
    });
    if (decision.width != 1920 || decision.height != 1080 ||
        decision.source != SESkinResolutionSource::Information)
        return 1;

    decision = SEResolveSkinResolution({
        "#INFORMATION,0,title,maker,,,,",
        "#RESOLUTION,1280,720",
        "#DST_IMAGE,0,0,0,0,1920,1080"
    });
    if (decision.width != 1280 || decision.height != 720 ||
        decision.source != SESkinResolutionSource::ResolutionCommand)
        return 2;

    decision = SEResolveSkinResolution({ "#RESOLUTION,1" });
    if (decision.width != 1280 || decision.height != 720 ||
        decision.source != SESkinResolutionSource::ResolutionCommand)
        return 3;

    // TenRiff regression: lanes left of x=960 look SD by themselves, but the
    // corner-anchored 1280x720 backdrop identifies the HD authoring canvas.
    decision = SEResolveSkinResolution({
        "#DST_IMAGE,0,0,0,0,1280,720",
        "#DST_NOTE,0,0,200,500,60,33,0,255",
        "#DST_NOTE,1,0,260,500,60,33,0,255"
    });
    if (decision.width != 1280 || decision.height != 720 ||
        decision.source != SESkinResolutionSource::TenRiffAuto ||
        decision.destinationEvidenceCount != 3)
        return 4;

    // TenRiff regression: large animation panels parked away from the origin
    // must not promote an ordinary SD playfield to HD or FHD.
    decision = SEResolveSkinResolution({
        "#DST_IMAGE,0,0,0,1335,1280,364",
        "#DST_IMAGE,1,0,0,-341,1280,341",
        "#DST_NOTE,0,0,100,400,30,22,0,255",
        "#DST_NOTE,1,0,130,400,30,22,0,255"
    });
    if (decision.width != 640 || decision.height != 480 ||
        decision.source != SESkinResolutionSource::TenRiffAuto ||
        decision.destinationEvidenceCount != 2)
        return 5;

    decision = SEResolveSkinResolution({
        "#DST_NOTE,0,0,1200,500,60,33,0,255"
    });
    if (decision.width != 1280 || decision.height != 720 ||
        decision.source != SESkinResolutionSource::TenRiffAuto)
        return 6;

    // TenRiff recognises LR2's three authoring families; even a larger
    // backdrop resolves to the highest supported FHD family.
    decision = SEResolveSkinResolution({
        "#DST_IMAGE,0,0,0,0,3840,2160"
    });
    if (decision.width != 1920 || decision.height != 1080 ||
        decision.source != SESkinResolutionSource::TenRiffAuto)
        return 7;

    // The final alpha-zero destination removes an otherwise off-screen lane,
    // matching TenRiff's visible-lane map rather than accumulating keyframes.
    decision = SEResolveSkinResolution({
        "#DST_NOTE,0,0,1700,500,60,33,0,255",
        "#DST_NOTE,0,100,1700,500,60,33,0,0",
        "#DST_NOTE,1,0,100,400,30,22,0,255"
    });
    if (decision.width != 640 || decision.height != 480 ||
        decision.source != SESkinResolutionSource::TenRiffAuto ||
        decision.destinationEvidenceCount != 1)
        return 8;

    decision = SEResolveSkinResolution({ "#IMAGE,background.png" });
    if (decision.width != 640 || decision.height != 480 ||
        decision.source != SESkinResolutionSource::Default640x480 ||
        decision.destinationEvidenceCount != 0)
        return 9;

    return 0;
}

int RunOlrPackageSelfTest() {
    char temporaryRoot[MAX_PATH] = {};
    char testRoot[MAX_PATH] = {};
    if (!GetTempPathA(MAX_PATH, temporaryRoot)) return 1;
    if (!GetTempFileNameA(temporaryRoot, "OLR", 0, testRoot)) return 2;
    DeleteFileA(testRoot);
    if (!CreateDirectoryA(testRoot, NULL)) return 3;

    const std::string root = testRoot;
    const std::string virtualRoot = root + "\\Test";
    const std::string virtualNoteFolder = virtualRoot + "\\note";
    const std::string assetPath = virtualNoteFolder + "\\blue.png";
    const std::string packagePath = root + "\\test.olrskin";
    const std::string tamperedPath = root + "\\tampered.olrskin";
    const std::string extractedPath = root + "\\imported";
    const std::string materializedPath = root + "\\materialized";
    const unsigned char assetBytes[] = {
        'O', 'L', 'R', '_', 'A', 'S', 'S', 'E', 'T', '_', 'T', 'E', 'S', 'T', '!'
    };
    int result = 0;
    if (!CreateDirectoryA(virtualRoot.c_str(), NULL) ||
        !CreateDirectoryA(virtualNoteFolder.c_str(), NULL))
        result = 4;
    if (result == 0) {
        FILE* assetFile = fopen(assetPath.c_str(), "wb");
        if (!assetFile) result = 5;
        else {
            if (fwrite(assetBytes, 1, sizeof(assetBytes), assetFile) !=
                sizeof(assetBytes))
                result = 6;
            fclose(assetFile);
        }
    }

    SEOLRSkinDocument document;
    document.title = "OLR self test";
    document.maker = "SkinEditor";
    document.scene = "PLAY 7KEYS";
    document.canvasWidth = 1280;
    document.canvasHeight = 720;
    document.resolutionSource = "#RESOLUTION";
    document.lr2Script =
        "#INFORMATION,0,OLR self test,SkinEditor,,,1280,720\r\n"
        "#CUSTOMFILE,NOTE,vfs/LR2files/Theme/Test/note/*.png,blue\r\n"
        "#IMAGE,vfs/LR2files/Theme/Test/note/blue.png\r\n"
        "#SRC_IMAGE,0,0,0,0,16,16,1,1,0,0\r\n"
        "#DST_IMAGE,0,0,100,200,16,16,0,255,255,255,255,1,0,0,0,0\r\n";
    document.lr2ExportMainPath = "LR2files/Theme/Test/play.lr2skin";
    document.virtualRoots.push_back({ "LR2files/Theme/Test", virtualRoot });
    SEOLRSemanticObject semanticObject;
    semanticObject.id = "obj_test";
    semanticObject.category = "misc";
    semanticObject.name = "Test image";
    semanticObject.group = "IMAGE";
    semanticObject.sourceCommand = "#SRC_IMAGE";
    semanticObject.destinationCommand = "#DST_IMAGE";
    semanticObject.sourceRows = { 2, 3 };
    semanticObject.hasDestination = true;
    semanticObject.x = 100;
    semanticObject.y = 200;
    semanticObject.width = 16;
    semanticObject.height = 16;
    document.objects.push_back(semanticObject);
    document.sourceMap.push_back({ 0, 0, "main.lr2skin" });

    SEOLRPackageInfo packageInfo;
    std::string errorMessage;
    if (result == 0 && !SEWriteOLRSkinPackage(packagePath.c_str(), document,
        packageInfo, errorMessage)) result = 7;
    if (result == 0 && (packageInfo.entries.size() != 7 ||
        packageInfo.objectCount != 1 || packageInfo.assetCount != 1 ||
        packageInfo.virtualRootCount != 1 || packageInfo.virtualFileCount != 1))
        result = 8;

    std::string extractedMain;
    if (result == 0 && !SEExtractOLRSkinPackage(packagePath.c_str(),
        extractedPath.c_str(), extractedMain, packageInfo, errorMessage))
        result = 9;
    if (result == 0 && extractedMain != extractedPath + "\\main.lr2skin")
        result = 10;
    if (result == 0) {
        std::ifstream script(extractedMain, std::ios::binary);
        const std::string scriptBytes((std::istreambuf_iterator<char>(script)),
            std::istreambuf_iterator<char>());
        if (scriptBytes != document.lr2Script) result = 11;
    }
    if (result == 0) {
        std::ifstream asset(extractedPath +
            "\\vfs\\LR2files\\Theme\\Test\\note\\blue.png",
            std::ios::binary);
        const std::vector<unsigned char> extractedBytes(
            (std::istreambuf_iterator<char>(asset)),
            std::istreambuf_iterator<char>());
        if (extractedBytes.size() != sizeof(assetBytes) ||
            memcmp(extractedBytes.data(), assetBytes, sizeof(assetBytes)) != 0)
            result = 12;
    }

    if (result == 0) {
        std::string resolved;
        if (!SEResolveSkinResourcePath(
            "vfs/LR2files/Theme/Test/note/*.png",
            extractedMain.c_str(), extractedMain.c_str(), resolved) ||
            _stricmp(resolved.c_str(),
                (extractedPath +
                    "\\vfs\\LR2files\\Theme\\Test\\note\\*.png").c_str()) != 0)
            result = 22;
    }

    SEOLRLr2ExportInfo exportInfo;
    if (result == 0 && (!SEIsOLRVirtualWorkspace(extractedMain.c_str()) ||
        !SEExportOLRWorkspaceToLR2(extractedMain.c_str(), materializedPath.c_str(),
            exportInfo, errorMessage)))
        result = 13;
    if (result == 0 && exportInfo.copiedFileCount != 1) result = 14;
    if (result == 0 && std::filesystem::path(exportInfo.mainSkinPath) !=
        std::filesystem::path(materializedPath) /
            "LR2files/Theme/Test/play.lr2skin")
        result = 21;
    if (result == 0) {
        std::ifstream compiled(exportInfo.mainSkinPath, std::ios::binary);
        const std::string compiledBytes((std::istreambuf_iterator<char>(compiled)),
            std::istreambuf_iterator<char>());
        if (compiledBytes.find("vfs/LR2files/") != std::string::npos ||
            compiledBytes.find("LR2files\\Theme\\Test\\note\\*.png") ==
                std::string::npos ||
            compiledBytes.find("LR2files\\Theme\\Test\\note\\blue.png") ==
                std::string::npos)
            result = 15;
    }

    if (result == 0) {
        SEOLRSkinDocument unsafeDocument = document;
        unsafeDocument.virtualRoots[0].logicalRoot = "LR2files/Theme/../escape";
        if (SEWriteOLRSkinPackage((root + "\\unsafe.olrskin").c_str(),
            unsafeDocument, packageInfo, errorMessage))
            result = 16;
    }

    if (result == 0) {
        std::ifstream input(packagePath, std::ios::binary);
        std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(input)),
            std::istreambuf_iterator<char>());
        const auto match = std::search(bytes.begin(), bytes.end(),
            std::begin(assetBytes), std::end(assetBytes));
        if (match == bytes.end()) result = 17;
        else {
            *match ^= 0x20;
            std::ofstream output(tamperedPath, std::ios::binary | std::ios::trunc);
            output.write((const char*)bytes.data(), bytes.size());
            output.close();
            if (SEInspectOLRSkinPackage(tamperedPath.c_str(), packageInfo,
                errorMessage)) result = 18;
        }
    }

    if (result == 0) {
        std::string resolved;
        const std::string owner = virtualRoot + "\\play.lr2skin";
        if (!SEResolveSkinResourcePath(
            ".\\LR2files\\Theme\\Test\\note\\*.png",
            owner.c_str(), owner.c_str(), resolved) ||
            _stricmp(resolved.c_str(),
                (virtualNoteFolder + "\\*.png").c_str()) != 0)
            result = 20;
    }

    std::error_code cleanupError;
    std::filesystem::remove_all(root, cleanupError);
    if (result == 0 && cleanupError) result = 19;
    return result;
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
        "left-browser") != 0) return 14;
    if (std::strcmp(SEUIWindowSpecFor(SEUIWindowId::ObjectInspector).defaultDock,
        "left-inspector") != 0) return 15;
    if (std::strcmp(SEUIWindowSpecFor(SEUIWindowId::TextEditor).defaultDock,
        "center-tabs") != 0) return 16;
    if (std::strcmp(SEUIWindowSpecFor(SEUIWindowId::ObjectProperty).group,
        "Advanced") != 0) return 17;
    FormatSEUIWindowTitle(title, sizeof(title), SEUIWindowId::TimerControl, 5);
    if (std::strcmp(title, "Timer Control##timer-control-5") != 0) return 18;
    if (std::strcmp(SEUIWindowSpecFor(SEUIWindowId::TimerControl).defaultDock,
        "right-lower") != 0) return 19;

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

    if (result == 0) {
        FILE* skinFile = fopen(rootSkin.c_str(), "wb");
        if (!skinFile) result = 13;
        else {
            fputs("#INFORMATION,0,Resolution test,Self test,,,,\r\n", skinFile);
            fputs("#RESOLUTION,1280,720\r\n", skinFile);
            fclose(skinFile);

            SkinManage parsedSkin = {};
            if (!InitSkinData(&parsedSkin)) result = 14;
            else {
                char originalWorkingDirectory[MAX_PATH] = {};
                if (!GetCurrentDirectoryA(MAX_PATH, originalWorkingDirectory) ||
                    !SetCurrentDirectoryA(scanRoot)) {
                    result = 15;
                }
                if (result == 0)
                    ParseLR2SkinCustom(&parsedSkin, CSTR(rootSkin.c_str()));
                if (*originalWorkingDirectory)
                    SetCurrentDirectoryA(originalWorkingDirectory);
                SESkinResolutionDecision browserResolution;
                if (result == 0 && (parsedSkin.Count != 1 ||
                    !SEResolveSkinResolutionFile(rootSkin.c_str(),
                        browserResolution) ||
                    browserResolution.width != 1280 ||
                    browserResolution.height != 720)) result = 16;
            }
        }
    }

    DeleteFileA(ignored.c_str());
    DeleteFileA(nestedSkin.c_str());
    DeleteFileA(rootSkin.c_str());
    RemoveDirectoryA(nested.c_str());
    RemoveDirectoryA(scanRoot);

    if (result == 0) {
        SkinManage skinData = {};
        if (!InitSkinData(&skinData)) result = 17;
        else {
            skinData.Data[0].skinFile.assign("first.lr2skin");
            skinData.Data[0].customs[0].title.assign("Option");
            skinData.Data[0].customs[0].op_label[0].assign("Choice");
            skinData.Data[0].customs[0].dst_op_count = 1;
            skinData.Data[0].custom_count = 1;
            skinData.Count = 1;
            CSTR* const labelTable = skinData.Data[0].customs[0].op_label;
            if (!ResetSkinData(&skinData)) result = 18;
            else if (skinData.Count != 0 || skinData.Data[0].skinFile.body ||
                skinData.Data[0].customs[0].title.body ||
                skinData.Data[0].customs[0].op_label != labelTable ||
                skinData.Data[0].customs[0].op_label[0].body ||
                skinData.Data[0].customs[0].dst_op_count != 0) result = 19;
        }
    }
    return result;
}

int RunPreviewSimulatorSelfTest() {
    LR2SEPreviewChartNote notes[256] = {};
    int count = LR2SEBuildPreviewChart(SKINTYPE_7KEYS,
        LR2SE_PREVIEW_CHART_FULL, notes, 256);
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

    count = LR2SEBuildPreviewChart(SKINTYPE_14KEYS,
        LR2SE_PREVIEW_CHART_FULL, notes, 256);
    if (count != 180) return 10;
    bool firstPlayer = false;
    bool secondPlayer = false;
    for (int index = 0; index < count; ++index) {
        firstPlayer = firstPlayer || (notes[index].lane >= 0 && notes[index].lane <= 7);
        secondPlayer = secondPlayer || (notes[index].lane >= 10 && notes[index].lane <= 17);
    }
    if (!firstPlayer || !secondPlayer) return 11;

    count = LR2SEBuildPreviewChart(SKINTYPE_9KEYS,
        LR2SE_PREVIEW_CHART_FULL, notes, 256);
    if (count != 180) return 12;
    for (int index = 0; index < count; ++index) {
        if (notes[index].lane < 1 || notes[index].lane > 9) return 13;
    }

    if (LR2SEBuildPreviewChart(SKINTYPE_14KEYS,
        LR2SE_PREVIEW_CHART_FULL, notes, 5) != 5)
        return 14;
    if (LR2SEBuildPreviewChart(SKINTYPE_7KEYS,
        LR2SE_PREVIEW_CHART_FULL, nullptr, 256) != 0)
        return 15;

    count = LR2SEBuildPreviewChart(SKINTYPE_7KEYS,
        LR2SE_PREVIEW_CHART_SIMPLE, notes, 256);
    if (count != 16) return 16;
    longNotes = 0;
    mines = 0;
    previousTiming = 0;
    for (int index = 0; index < count; ++index) {
        const LR2SEPreviewChartNote& note = notes[index];
        if (note.timingMs < previousTiming) return 17;
        previousTiming = note.timingMs;
        if (note.kind == LR2SE_PREVIEW_NOTE_LONG) longNotes++;
        if (note.kind == LR2SE_PREVIEW_NOTE_MINE) mines++;
    }
    if (longNotes == 0 || mines == 0) return 18;

    LR2SEPreviewTimelineEvent timeline[32] = {};
    count = LR2SEBuildPreviewTimeline(timeline, 32);
    if (count != 23) return 19;
    for (int index = 0; index < count - 1; ++index) {
        if (timeline[index].timingMs != (unsigned int)index * 1600U ||
            timeline[index].op != 2 || timeline[index].value != 0.0 ||
            timeline[index].terminal)
            return 20;
    }
    if (timeline[count - 1].timingMs != 34000U ||
        timeline[count - 1].op != 2 || !timeline[count - 1].terminal)
        return 21;
    if (LR2SEBuildPreviewTimeline(timeline, 2) != 2 ||
        LR2SEBuildPreviewTimeline(nullptr, 32) != 0)
        return 22;

    Timer rhythmTimer = {};
    rhythmTimer.Rhythm = -1.0;
    if (SetTimeLapse(140, &rhythmTimer) != 1 ||
        GetTimeLapse(140, &rhythmTimer) != 0.0)
        return 23;
    if (ResetTimeLapse(140, &rhythmTimer) != 1 ||
        GetTimeLapse(140, &rhythmTimer) != -1.0)
        return 24;

    return 0;
}
