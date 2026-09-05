#include "winWorkspace.h"
#include "seHelper.h"
#include "olrSkin.h"
#include "../LR2/En_fileutil.h"

#include <cstdio>
#include <filesystem>
#include <fstream>

int RunSimpleFontApplySelfTest() {
    if (arr_CommandHelp.count <= 0 && LoadCommandHelp(nullptr) != 0) return 1;
    char tempDirectory[MAX_PATH] = {}, uniquePath[MAX_PATH] = {};
    if (!GetTempPathA(MAX_PATH, tempDirectory) || !GetTempFileNameA(tempDirectory, "SEF", 0, uniquePath)) return 2;
    if (!DeleteFileA(uniquePath) || !CreateDirectoryA(uniquePath, nullptr)) return 3;
    const auto root = std::filesystem::path(uniquePath);
    const auto skinFolder = root / "LR2files" / "Theme" / "FontTest";
    std::filesystem::create_directories(skinFolder);
    const auto mainPath = skinFolder / "main.lr2skin";
    const auto originalImage = skinFolder / "original.png";
    char imageError[256] = {};
    if (!CreateSolidImageFileAtomic(originalImage.string().c_str(), 100, 30, 0xffffffff, imageError, sizeof(imageError))) return 4;

    auto workspace = std::make_unique<WORKSPACE>();
    workspace->skinfileLines.Alloc(sizeof(SKINFILELINEREAD), 32);
    workspace->arr_CustomFile.Alloc(sizeof(CSTR), 2);
    workspace->arr_SRCGR.Alloc(sizeof(SRCGR), 2);
    workspace->arr_IMG.Alloc(sizeof(IMG), 4);
    workspace->arr_SRC.Alloc(sizeof(SRC), 4);
    workspace->arr_DST.Alloc(sizeof(DST), 4);
    workspace->arr_seobj.Alloc(sizeof(SEOBJ), 4);
    workspace->arr_ifunit.Alloc(sizeof(IFUNIT), 4);
    workspace->arr_history.Alloc(sizeof(HISTORY), 4);
    strncpy_s(workspace->mainpath, mainPath.string().c_str(), _TRUNCATE);
    workspace->loaded = true;
    workspace->meta.type = SKINTYPE_7KEYS;
    workspace->meta.targetX = 1280;
    workspace->meta.targetY = 720;
    workspace->meta.title.assign("Font test");
    workspace->meta.maker.assign("Self-test");
    const auto append = [&](const std::string& text) {
        auto* row = (SKINFILELINEREAD*)workspace->skinfileLines.Get_new();
        row->line.assign(text.c_str());
        row->filename.assign(workspace->mainpath);
        row->numTotal = workspace->skinfileLines.count - 1;
        row->num = row->numTotal;
        row->isComment = text[0] != '#';
        row->isSEcomment = text[0] == '$';
        row->ifgroup = 0;
        SplitCSV(row->line, &row->csv, ",");
        row->csvColumnCount = CountCsvColumns(row->line);
    };
    append("$FILE '" + mainPath.string() + "' start");
    append("#INFORMATION,0,Font test,Self-test");
    append("#RESOLUTION,1280,720");
    append("#IMAGE," + originalImage.string());
    append("#SRC_NOWJUDGE_1P,5,0,0,0,100,30,1,1,1000,46,0,0,0");
    append("#DST_NOWJUDGE_1P,5,0,100,200,100,30,0,255,255,255,255,0,0,0,46,0,0,0,0");
    append("#SRC_NOWJUDGE_2P,5,0,0,0,100,30,1,1,1000,47,1,0,0");
    append("#SRC_NOWJUDGE_1P,4,0,0,0,100,30,1,1,1000,46,0,0,0");
    append("$SRC_IMAGE,0,0,0,0,100,30,1,1,0,0,0,0,0");
    append("$FILE '" + mainPath.string() + "' end");
    const auto lineAt = [&](int index) { return std::string(((SKINFILELINEREAD*)workspace->skinfileLines.data)[index].line.outstr()); };
    std::vector<std::string> originalRows;
    for (int row = 0; row < workspace->skinfileLines.count; ++row) originalRows.push_back(lineAt(row));
    { std::ofstream file(mainPath, std::ios::binary); for (const auto& line : originalRows) file << line << "\r\n"; }
    auto slots = workspace->GetSimpleModeSlots();
    if (slots.size() != 3) return 5;
    const std::string slotId = slots.front().id;
    if (workspace->GetSimpleModeApplyTargets(slotId, 1).size() != 2) return 6;
    wchar_t windowsDirectory[MAX_PATH] = {};
    if (!GetWindowsDirectoryW(windowsDirectory, MAX_PATH)) return 7;
    SEFontAtlasRequest request;
    request.fontPath = std::wstring(windowsDirectory) + L"\\Fonts\\arial.ttf";
    request.isNumber = false;
    request.columns = request.rows = 1;
    request.cellWidth = 256;
    request.cellHeight = 64;
    SEFontAtlasBitmap bitmap;
    std::string message;
    if (!RenderSEFontAtlas(request, bitmap, message)) return 8;
    if (workspace->ApplySimpleModeFontBitmap(slotId, bitmap, 1, message) != 0) {
        fprintf(stderr, "Font apply: %s\n", message.c_str()); return 9;
    }
    if (workspace->arr_history.count != 1 || workspace->skinfileLines.count != (int)originalRows.size() + 2) return 10;
    auto* rows = (SKINFILELINEREAD*)workspace->skinfileLines.data;
    if (rows[6].csv.val[2] != 0 || rows[8].csv.val[2] != 0 || rows[9].csv.val[2] != 1 ||
        rows[6].csv.val[5] != 256 || rows[6].csv.val[6] != 64 ||
        rows[6].csv.val[9] != 1000 || rows[6].csv.val[10] != 46 ||
        rows[8].csv.val[10] != 47 || rows[8].csv.val[11] != 1 ||
        lineAt(7) != originalRows[5] || rows[9].csv.val[5] != 100 || rows[9].csv.val[6] != 30 ||
        rows[10].csv.val[2] != 1 || lineAt(10).find("$SRC_IMAGE,0,1,") != 0) return 11;
    int precedingImages = 0;
    for (int row = 0; row < workspace->skinfileLines.count; ++row) {
        if (rows[row].csv.str[0].isSame("#IMAGE")) ++precedingImages;
        if (rows[row].csv.str[0].body && !strncmp(rows[row].csv.str[0].outstr(), "#SRC_NOWJUDGE_", 14) &&
            rows[row].csv.val[2] >= precedingImages) return 22;
    }
    std::filesystem::path generatedImage;
    for (const auto& entry : std::filesystem::directory_iterator(skinFolder / "simple-assets"))
        if (entry.path().extension() == ".png") generatedImage = entry.path();
    if (generatedImage.empty()) return 12;
    int width = 0, height = 0;
    if (!GetImageSizeFromFile(generatedImage.string().c_str(), &width, &height) || width != 256 || height != 64) return 13;

    // Exercise the unchanged Workspace -> OLRskin 0.9 packager with a generated PNG.
    const auto packagePath = root / "font-test.olrskin";
    if (workspace->ExportOlrSkin(packagePath.string().c_str(), message) != 0) {
        fprintf(stderr, "Font package: %s\n", message.c_str()); return 14;
    }
    SEOLRPackageInfo packageInfo;
    if (!SEInspectOLRSkinPackage(packagePath.string().c_str(), packageInfo, message) ||
        packageInfo.formatVersion != 9) return 15;
    for (const std::string& entry : packageInfo.entries)
        if (entry.find(".ttf") != std::string::npos || entry.find(".otf") != std::string::npos) return 25;
    if (workspace->UndoLastEdit() != 0 || workspace->ApplyPendingHistorySnapshotRestore() != 0) return 16;
    if (workspace->skinfileLines.count != (int)originalRows.size()) return 17;
    for (int row = 0; row < (int)originalRows.size(); ++row)
        if (lineAt(row) != originalRows[row]) return 18;
    if (!std::filesystem::exists(generatedImage)) return 19;
    if (workspace->RedoLastEdit() != 0 || workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        ((SKINFILELINEREAD*)workspace->skinfileLines.data)[6].csv.val[2] != 0) return 20;
    // Invalid scope and bitmap must leave the current document/history intact.
    const int historyCount = workspace->arr_history.count;
    const int rowCount = workspace->skinfileLines.count;
    if (workspace->ApplySimpleModeFontBitmap(slotId, bitmap, 4, message) == 0 ||
        workspace->ApplySimpleModeFontBitmap(slotId, SEFontAtlasBitmap(), 0, message) == 0 ||
        workspace->arr_history.count != historyCount || workspace->skinfileLines.count != rowCount) return 21;
    const SESimpleModeSlot imageTarget = workspace->GetSimpleModeSlots().front();
    if (workspace->ImportSimpleModeImage(imageTarget.row, originalImage.string().c_str(), 0, true, message) != 0) {
        fprintf(stderr, "Font image import: %s\n", message.c_str()); return 23;
    }
    rows = (SKINFILELINEREAD*)workspace->skinfileLines.data;
    if (rows[8].csv.val[2] != 0 || rows[8].csv.val[5] != 100 || rows[10].csv.val[2] != 1 ||
        rows[11].csv.val[2] != 2 || workspace->arr_history.count != historyCount + 1) return 24;
    if (workspace->EditValue(8, 2, 99) != 0) return 26;
    const auto blockedTarget = workspace->GetSimpleModeSlots().front();
    const int blockedHistoryCount = workspace->arr_history.count;
    if (workspace->ApplySimpleModeFontBitmap(blockedTarget.id, bitmap, 0, message) == 0 ||
        workspace->arr_history.count != blockedHistoryCount ||
        ((SKINFILELINEREAD*)workspace->skinfileLines.data)[8].csv.val[2] != 99) return 27;
    fprintf(stdout, "Font fixture (PNG and OLRskin): %s\n", root.string().c_str());
    return 0;
}
