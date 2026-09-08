#include "winWorkspace.h"

#include "winWorkspaceUiHelpers.h"
#include "seHelper.h"
#include "olrSkin.h"
#include "../LR2/En_fileutil.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

namespace {

struct CopiedObjectLine {
    std::string text;
    int sourceLineNumber = 0;
};

struct CopiedObject {
    std::string name;
    std::vector<CopiedObjectLine> lines;
};

struct ObjectClipboard {
    std::vector<CopiedObject> objects;
};

ObjectClipboard processObjectClipboard;
unsigned int generatedObjectIdSerial = 0;

std::vector<int> ResolveSelectedObjectModels(WORKSPACE& workspace) {
    std::vector<int> modelIndices;
    for (const SEObjectSelectionKey& key : workspace.objectSelection.selected) {
        const int modelIndex = workspace.ResolveObjectSelectionKey(key);
        if (modelIndex >= 0) modelIndices.push_back(modelIndex);
    }
    if (modelIndices.empty()) {
        const int active = workspace.ResolveObjectSelectionKey(
            workspace.objectSelection.active);
        if (active >= 0) modelIndices.push_back(active);
    }
    if (modelIndices.empty())
        modelIndices = workspace.preview_selected_object_model_indices;

    const std::vector<SEObjectInstance>& objects =
        workspace.objectEditorModel.Objects();
    modelIndices.erase(std::remove_if(modelIndices.begin(), modelIndices.end(),
        [&](int index) {
            return index < 0 || index >= (int)objects.size() ||
                objects[index].rows.empty();
        }), modelIndices.end());
    std::sort(modelIndices.begin(), modelIndices.end(),
        [&](int left, int right) {
            return objects[left].rows.front() < objects[right].rows.front();
        });
    modelIndices.erase(std::unique(modelIndices.begin(), modelIndices.end()),
        modelIndices.end());
    return modelIndices;
}

bool CaptureObjects(WORKSPACE& workspace, const std::vector<int>& modelIndices,
    ObjectClipboard& clipboard) {
    clipboard.objects.clear();
    const std::vector<SEObjectInstance>& objects =
        workspace.objectEditorModel.Objects();
    for (int modelIndex : modelIndices) {
        if (modelIndex < 0 || modelIndex >= (int)objects.size()) continue;
        const SEObjectInstance& sourceObject = objects[modelIndex];
        CopiedObject copied;
        copied.name = sourceObject.name;
        for (int row : sourceObject.rows) {
            if (row < 0 || row >= workspace.skinfileLines.count) continue;
            SKINFILELINEREAD& source =
                ((SKINFILELINEREAD*)workspace.skinfileLines.data)[row];
            CopiedObjectLine line;
            line.text = source.line.body ? source.line.outstr() : "";
            line.sourceLineNumber = source.num;
            copied.lines.push_back(line);
        }
        if (!copied.lines.empty()) clipboard.objects.push_back(copied);
    }
    return !clipboard.objects.empty();
}

std::string GenerateObjectId(const WORKSPACE& workspace) {
    std::set<std::string> existingIds;
    for (const SEObjectInstance& object : workspace.objectEditorModel.Objects())
        existingIds.insert(object.editorId);

    char candidate[64];
    do {
        ++generatedObjectIdSerial;
        snprintf(candidate, sizeof(candidate), "obj_%08llX_%04X",
            GetTickCount64(), generatedObjectIdSerial & 0xffffu);
    } while (existingIds.find(candidate) != existingIds.end());
    return candidate;
}

int PasteObjects(WORKSPACE& workspace, const ObjectClipboard& clipboard) {
    if (clipboard.objects.empty()) return -1;

    CSTR targetOwner(workspace.mainpath);
    int targetIfgroup = 0;
    int insertPosition = -1;
    const int activeModel = workspace.ResolveObjectSelectionKey(
        workspace.objectSelection.active);
    const std::vector<SEObjectInstance>& objects =
        workspace.objectEditorModel.Objects();
    if (activeModel >= 0 && activeModel < (int)objects.size() &&
        !objects[activeModel].rows.empty()) {
        const SEObjectInstance& target = objects[activeModel];
        const int firstRow = target.rows.front();
        insertPosition = target.rows.back() + 1;
        targetIfgroup = target.ifgroup;
        SKINFILELINEREAD& ownerRow =
            ((SKINFILELINEREAD*)workspace.skinfileLines.data)[firstRow];
        if (ownerRow.filename.body && *ownerRow.filename.outstr())
            targetOwner.assign(ownerRow.filename.outstr());
    } else {
        AssignRootFileOwner(workspace.skinfileLines, workspace.mainpath,
            targetOwner);
        insertPosition = FindOwnerFileEndRow(workspace.skinfileLines,
            targetOwner.body ? targetOwner.outstr() : workspace.mainpath);
    }
    if (insertPosition < 0) insertPosition = workspace.skinfileLines.count;

    const SkinDocumentSnapshot before = workspace.CaptureDocumentSnapshot();
    const bool previousApplyingHistory = workspace.applyingHistory;
    workspace.applyingHistory = true;
    std::vector<std::string> generatedIds;
    bool failed = false;

    const auto insertLine = [&](const std::string& text, int sourceLineNumber) {
        if (workspace.InsertLine(insertPosition) != 0) return false;
        SKINFILELINEREAD& inserted =
            ((SKINFILELINEREAD*)workspace.skinfileLines.data)[insertPosition];
        CSTR placeholder(inserted.line);
        if (workspace.EditLine(insertPosition, placeholder,
            CSTR(text.c_str())) != 0) return false;
        inserted.filename.assign(targetOwner.body && *targetOwner.outstr()
            ? targetOwner.outstr() : workspace.mainpath);
        inserted.num = sourceLineNumber;
        inserted.ifgroup = targetIfgroup;
        inserted.modified = true;
        ++insertPosition;
        return true;
    };

    for (const CopiedObject& copied : clipboard.objects) {
        const std::string generatedId = GenerateObjectId(workspace);
        generatedIds.push_back(generatedId);
        const int sourceLineNumber = copied.lines.empty()
            ? 0 : copied.lines.front().sourceLineNumber;
        if (!insertLine("$SE_OBJECT_ID," + generatedId, sourceLineNumber)) {
            failed = true;
            break;
        }
        if (!copied.name.empty() &&
            !insertLine("$SE_OBJECT_NAME," + copied.name, sourceLineNumber)) {
            failed = true;
            break;
        }
        for (const CopiedObjectLine& line : copied.lines) {
            if (!insertLine(line.text, line.sourceLineNumber)) {
                failed = true;
                break;
            }
        }
        if (failed) break;
    }
    workspace.applyingHistory = previousApplyingHistory;

    if (failed) {
        const bool previousReplayState = workspace.replayingHistory;
        workspace.replayingHistory = true;
        workspace.RestoreDocumentSnapshot(before);
        workspace.replayingHistory = previousReplayState;
        return -1;
    }

    const int snapshotIndex = (int)workspace.historyDocumentSnapshots.size();
    workspace.historyDocumentSnapshots.push_back(before);
    HISTORY* history = (HISTORY*)workspace.arr_history.Get_new();
    if (!history) return -1;
    history->op = restoreDocument;
    history->target = snapshotIndex;

    workspace.RebuildObjectModel();
    std::vector<int> pastedModels;
    const std::vector<SEObjectInstance>& rebuilt =
        workspace.objectEditorModel.Objects();
    for (const std::string& generatedId : generatedIds) {
        for (int index = 0; index < (int)rebuilt.size(); ++index) {
            if (rebuilt[index].editorId == generatedId) {
                pastedModels.push_back(index);
                break;
            }
        }
    }
    if (!pastedModels.empty()) {
        workspace.SetObjectSelection(pastedModels, pastedModels.back(),
            pastedModels.front(), true);
        workspace.preview_object_dragging = false;
        workspace.preview_object_resizing = false;
        workspace.RefreshPreviewSelectionBounds();
    }
    return (int)pastedModels.size();
}

} // namespace

int WORKSPACE::CopySelectedObjects() {
    const std::vector<int> selected = ResolveSelectedObjectModels(*this);
    ObjectClipboard copied;
    if (!CaptureObjects(*this, selected, copied)) return -1;
    processObjectClipboard = copied;
    return (int)processObjectClipboard.objects.size();
}

int WORKSPACE::PasteCopiedObjects() {
    return PasteObjects(*this, processObjectClipboard);
}

int WORKSPACE::DuplicateSelectedObjects() {
    const std::vector<int> selected = ResolveSelectedObjectModels(*this);
    ObjectClipboard copied;
    if (!CaptureObjects(*this, selected, copied)) return -1;
    return PasteObjects(*this, copied);
}

bool WORKSPACE::SplitSelectedObjects(const char* filename, std::string& error) {
    namespace fs = std::filesystem;
    error.clear();
    if (!loaded || applyingHistory || pendingHistorySnapshotRestore >= 0 ||
        SEIsOLRVirtualWorkspace(mainpath)) {
        error = "Open an editable LR2 skin first (not a virtual OLR workspace).";
        return false;
    }
    // A sibling filename keeps all moved commands in the same resource directory.
    std::string name = filename ? filename : "";
    if (name.size() < 5 || name.size() > 120 ||
        _stricmp(name.c_str() + name.size() - 4, ".csv") != 0 ||
        name.find("..") != std::string::npos || name[0] == '.') {
        error = "Enter a new sibling filename such as notes.csv."; return false;
    }
    for (unsigned char c : name) {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.')) {
            error = "Use letters, digits, underscore or dash in the CSV filename.";
            return false;
        }
    }
    const auto selected = ResolveSelectedObjectModels(*this);
    if (selected.empty()) { error = "Select Objects first."; return false; }
    const auto& objects = objectEditorModel.Objects();
    std::set<int> selectedRows;
    for (int index : selected)
        selectedRows.insert(objects[index].rows.begin(), objects[index].rows.end());
    int first = *selectedRows.begin(), last = *selectedRows.rbegin();
    auto* lines = (SKINFILELINEREAD*)skinfileLines.data;
    const std::string owner = lines[first].filename.outstr();
    const int branch = lines[first].ifgroup;
    int ownerInstances = 0;
    for (int row = 0; row < skinfileLines.count; ++row) {
        const std::string text = lines[row].line.body ? lines[row].line.outstr() : "";
        if (!_stricmp(lines[row].filename.outstr(), owner.c_str()) &&
            text.rfind("$FILE ", 0) == 0 && text.size() >= 6 &&
            text.compare(text.size() - 6, 6, " start") == 0) ++ownerInstances;
    }
    if (ownerInstances != 1) {
        error = "A repeatedly included source file cannot be split here."; return false;
    }
    // Include attached identity/name metadata, never the enclosing IF or group.
    while (first > 0) {
        auto& previous = lines[first - 1];
        const char* text = previous.line.body ? previous.line.outstr() : "";
        if (_stricmp(previous.filename.outstr(), owner.c_str()) ||
            strncmp(text, "$SE_OBJECT_", 11)) break;
        --first;
    }
    for (int row = first; row <= last; ++row) {
        auto& line = lines[row];
        const char* text = line.line.body ? line.line.outstr() : "";
        if (_stricmp(line.filename.outstr(), owner.c_str()) || line.ifgroup != branch ||
            (!line.isComment && selectedRows.count(row) == 0) ||
            (line.isSEcomment && strncmp(text, "$SE_OBJECT_", 11))) {
            error = "Select consecutive Objects in one file and IF branch, without intervening commands/groups.";
            return false;
        }
    }
    std::error_code ec;
    const fs::path target = fs::absolute(fs::path(owner).parent_path() / name, ec).lexically_normal();
    if (ec || target.string().size() >= MAX_PATH || fs::exists(target, ec) || ec) {
        error = "The target already exists or the path is invalid."; return false;
    }
    for (int row = 0; row < skinfileLines.count; ++row) {
        if (!_stricmp(lines[row].filename.outstr(), target.string().c_str())) {
            error = "This filename is already part of the document."; return false;
        }
    }
    const auto before = CaptureDocumentSnapshot();
    auto after = before;
    const std::string targetText = target.string();
    for (int row = first; row <= last; ++row) {
        after.lines[row].filename = targetText;
        after.lines[row].modified = true;
    }
    SkinLineSnapshot include, begin, end;
    include.filename = owner;
    include.line = "#INCLUDE," + name;
    include.modified = true;
    begin.filename = end.filename = targetText;
    begin.line = "$FILE '" + targetText + "' start";
    end.line = "$FILE '" + targetText + "' end";
    after.lines.insert(after.lines.begin() + last + 1, end);
    after.lines.insert(after.lines.begin() + first, begin);
    after.lines.insert(after.lines.begin() + first, include);
    const auto oldRedo = redoDocumentSnapshots;
    const auto oldRevision = documentRevision;
    const bool oldReplay = replayingHistory;
    const auto rollback = [&]() {
        replayingHistory = true;
        RestoreDocumentSnapshot(before);
        replayingHistory = oldReplay;
        redoDocumentSnapshots = oldRedo;
        documentRevision = oldRevision;
        RebuildEditorDerivedState();
        RebuildObjectModel();
    };
    if (RestoreDocumentSnapshot(after) != 0 || RebuildEditorDerivedState() != 0) {
        rollback(); error = "Could not rebuild split document; changes rolled back."; return false;
    }
    RebuildObjectModel();
    if (SaveSkinScript(mainpath, true, false) != 0) {
        rollback(); error = "Save failed; split was rolled back."; return false;
    }
    const int snapshot = (int)historyDocumentSnapshots.size();
    historyDocumentSnapshots.push_back(before);
    auto* history = (HISTORY*)arr_history.Get_new();
    history->op = restoreDocument;
    history->target = snapshot;
    MarkDocumentSaved();
    return true;
}

bool WORKSPACE::HasCopiedObjects() const {
    return !processObjectClipboard.objects.empty();
}

const char* SELayoutImageType(int kind) {
    static const char* types[] = { "IMAGE", "NUMBER", "SLIDER", "BUTTON", "BARGRAPH" };
    return kind >= 0 && kind < 5 ? types[kind] : nullptr;
}

bool WORKSPACE::RegisterImageRegion(int graphicIndex, const TransparentAssetCrop& crop,
    std::string& status) {
    if (!loaded || applyingHistory || pendingHistorySnapshotRestore >= 0 ||
        graphicIndex < 0 || graphicIndex >= arr_SRCGR.count) {
        status = "The target texture is unavailable.";
        return false;
    }
    SRCGR& graphic = ((SRCGR*)arr_SRCGR.data)[graphicIndex];
    if (crop.x < 0 || crop.y < 0 || crop.w <= 0 || crop.h <= 0 ||
        (long long)crop.x + crop.w > graphic.sizeX ||
        (long long)crop.y + crop.h > graphic.sizeY) {
        status = "The detected region is outside the texture.";
        return false;
    }
    const int existing = FindIMG(graphic.grID, crop.x, crop.y, crop.w, crop.h, graphic.isIf);
    // FindIMG's legacy not-found sentinel is arr_IMG.count, NOT -1.
    if (existing >= 0 && existing < arr_IMG.count) {
        SelectIMGAsset(existing, true);
        assetBrowserFocusRequest = existing;
        assetSearch[0] = '\0';
        assetShowUnusedOnly = false;
        wAssetBrowser = true;
        status = "Selected existing Asset " + std::to_string(existing) + ".";
        return true;
    }
    const int branch = graphic.isIf;
    CSTR owner(mainpath);
    int row = graphic.declare;
    if (row < 0 || row >= skinfileLines.count) {
        status = "The #IMAGE declaration is unavailable.";
        return false;
    }
    owner.assign(((SKINFILELINEREAD*)skinfileLines.data)[row].filename);
    ++row;
    while (row < skinfileLines.count) {
        auto& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
        if (!line.line.body || strncmp(line.line.outstr(), "$SRC_IMAGE,", 11)) break;
        ++row;
    }
    char text[256];
    snprintf(text, sizeof(text), "$SRC_IMAGE,0,%d,%d,%d,%d,%d,1,1,0,0,0,0,0,manual crop",
        graphic.grID, crop.x, crop.y, crop.w, crop.h);
    const auto before = CaptureDocumentSnapshot();
    const auto oldRedo = redoDocumentSnapshots;
    const auto oldRevision = documentRevision;
    applyingHistory = true;
    bool ok = InsertLine(row) == 0;
    if (ok) {
        auto& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
        CSTR previous(line.line);
        ok = EditLine(row, previous, CSTR(text)) == 0;
        line.filename.assign(owner);
        line.ifgroup = branch;
        SplitCSV(line.line, &line.csv, ",");
        line.csvColumnCount = CountCsvColumns(line.line);
    }
    applyingHistory = false;
    if (!ok) {
        const bool replay = replayingHistory;
        replayingHistory = true;
        RestoreDocumentSnapshot(before);
        replayingHistory = replay;
        redoDocumentSnapshots = oldRedo;
        documentRevision = oldRevision;
        status = "Could not register the Asset; document changes were rolled back.";
        return false;
    }
    const int snapshot = (int)historyDocumentSnapshots.size();
    historyDocumentSnapshots.push_back(before);
    HISTORY* history = (HISTORY*)arr_history.Get_new();
    history->op = restoreDocument;
    history->target = snapshot;
    // Resolve the metadata row after the normal deferred rebuild, not a temporary IMG index.
    imageManagerAssetDeclarationFocusRequest = row;
    assetSearch[0] = '\0';
    assetShowUnusedOnly = false;
    wAssetBrowser = true;
    status = "Added Asset: " + std::to_string(crop.x) + ", " + std::to_string(crop.y) +
        " / " + std::to_string(crop.w) + " x " + std::to_string(crop.h) + ".";
    return true;
}

bool SELayoutImageSize(int width, int height, const SELayoutImageOptions& options,
    int& sheetWidth, int& sheetHeight) {
    sheetWidth = sheetHeight = 0;
    if (!SELayoutImageType(options.kind) || width <= 0 || height <= 0 ||
        width > 16384 || height > 16384 || options.divX <= 0 || options.divY <= 0 ||
        options.divX > 16384 || options.divY > 16384 || options.cycle < 0 ||
        options.digits < 1 || options.digits > 32 || options.align < 0 || options.align > 2 ||
        options.direction < 0 || options.direction > 3 || options.range < 0 ||
        (options.kind == 1 && (options.divX != 10 || options.divY != 1))) return false;
    const long long w = (long long)width * options.divX;
    const long long h = (long long)height * options.divY;
    if (w > 16384 || h > 16384 || w * h > 16 * 1024 * 1024) return false;
    sheetWidth = (int)w;
    sheetHeight = (int)h;
    return true;
}

bool WORKSPACE::CreateImageObjectFromLayout(int x, int y, int width,
    int height, const char* name, std::string& imagePath,
    std::string& errorText, int afterObject, const SELayoutImageOptions& options) {
    imagePath.clear();
    errorText.clear();
    int sheetWidth = 0, sheetHeight = 0;
    if (!loaded || !*mainpath || pendingHistorySnapshotRestore >= 0 ||
        applyingHistory || !SELayoutImageSize(width, height, options, sheetWidth, sheetHeight)) {
        errorText = "Open a skin and specify a positive image size (maximum 16 megapixels).";
        return false;
    }
    const std::string objectName = name && *name ? name : "Untitled image";
    if (objectName.find_first_of(",\r\n") != std::string::npos) {
        errorText = "The Object name cannot contain commas or line breaks.";
        return false;
    }
    // Check the installed schema before writing a file or any document row.
    const std::string sourceCommand = std::string("#SRC_") + SELayoutImageType(options.kind);
    const std::string destinationCommand = std::string("#DST_") + SELayoutImageType(options.kind);
    CSTR sourceHelp = GetCommandHelp(sourceCommand.c_str(), 0);
    CSTR destinationHelp = GetCommandHelp(destinationCommand.c_str(), 0);
    if (!sourceHelp.isSame(sourceCommand.c_str()) ||
        !destinationHelp.isSame(destinationCommand.c_str())) {
        errorText = "The IMAGE command schema is unavailable.";
        return false;
    }
    CSTR rootOwner;
    AssignRootFileOwner(skinfileLines, mainpath, rootOwner);
    const int rootEnd = FindOwnerFileEndRow(skinfileLines, rootOwner.outstr());
    int insertAt = rootEnd;
    int branch = 0;
    CSTR owner(rootOwner);
    const auto& objects = objectEditorModel.Objects();
    if (afterObject != -1) {
        if (afterObject < 0 || afterObject >= (int)objects.size() ||
            objects[afterObject].rows.empty()) {
            errorText = "The target Object no longer exists.";
            return false;
        }
        const SEObjectInstance& target = objects[afterObject];
        insertAt = target.rows.back() + 1;
        branch = target.ifgroup;
        owner.assign(((SKINFILELINEREAD*)skinfileLines.data)
            [target.rows.front()].filename);
    }
    if (insertAt < 0 || insertAt > skinfileLines.count) {
        errorText = "Could not locate the target file boundary.";
        return false;
    }

    namespace fs = std::filesystem;
    std::error_code pathError;
    const fs::path directory = fs::absolute(fs::path(mainpath), pathError)
        .parent_path();
    if (pathError || !fs::is_directory(directory, pathError)) {
        errorText = "The skin directory is unavailable.";
        return false;
    }
    std::string objectId;
    std::string generatedPath;
    do {
        objectId = GenerateObjectId(*this);
        generatedPath = (directory / ("layout_" + objectId + ".png")).string();
    } while (fs::exists(generatedPath, pathError) && !pathError);
    if (pathError || generatedPath.size() >= MAX_PATH ||
        generatedPath.find(',') != std::string::npos) {
        errorText = "The generated PNG path cannot be represented in LR2 CSV.";
        return false;
    }

    char imageError[256] = {};
    if (!CreateSolidImageFileAtomic(generatedPath.c_str(), sheetWidth, sheetHeight,
        D3DCOLOR_ARGB(0, 0, 0, 0), imageError, sizeof(imageError))) {
        errorText = imageError;
        return false;
    }
    const SkinDocumentSnapshot before = CaptureDocumentSnapshot();
    const auto oldRedo = redoDocumentSnapshots;
    const auto oldRevision = documentRevision;
    const int oldGraphicFocus = imageManagerGeneratedGrFocusRequest;
    applyingHistory = true;
    const int oldCount = skinfileLines.count;
    const int graphicId = RegisterGeneratedImage(generatedPath.c_str(),
        sheetWidth, sheetHeight, errorText, options.divX, options.divY, options.cycle);
    // RegisterGeneratedImage inserts #IMAGE and its reusable crop at root end.
    if (insertAt >= rootEnd) insertAt += skinfileLines.count - oldCount;

    const auto append = [&](const std::string& text) {
        if (InsertLine(insertAt) != 0) return false;
        SKINFILELINEREAD& line =
            ((SKINFILELINEREAD*)skinfileLines.data)[insertAt];
        CSTR previous(line.line);
        if (EditLine(insertAt, previous, CSTR(text.c_str())) != 0) return false;
        line.filename.assign(owner);
        line.ifgroup = branch;
        ++insertAt;
        return true;
    };
    const auto makeCommand = [&](const char* command, bool source) {
        CSVbuf values;
        SplitCSV("", &values, ",");
        values.str[0].assign(command);
        for (int column = 1; column < 30; ++column) {
            CSTR help = GetCommandHelp(command, column);
            if (!help.body || !*help.outstr()) continue;
            help.trimWhiteSpace();
            const char* field = help.outstr();
            int value = 0;
            if (!strcmp(field, "w")) value = source ? sheetWidth : width;
            else if (!strcmp(field, "h")) value = source ? sheetHeight : height;
            else if (source && !strcmp(field, "gr")) value = graphicId;
            else if (source && !strcmp(field, "div_x")) value = options.divX;
            else if (source && !strcmp(field, "div_y")) value = options.divY;
            else if (source && !strcmp(field, "cycle")) value = options.cycle;
            else if (source && (!strcmp(field, "$type") || !strcmp(field, "type") ||
                !strcmp(field, "$num") || !strcmp(field, "num"))) value = options.value;
            else if (source && !strncmp(field, "align", 5)) value = options.align;
            else if (source && !strcmp(field, "keta")) value = options.digits;
            else if (source && !strcmp(field, "muki")) value = options.direction;
            else if (source && !strcmp(field, "range")) value = options.range;
            else if (!source && !strcmp(field, "x")) value = x;
            else if (!source && !strcmp(field, "y")) value = y;
            else if (!source && (!strcmp(field, "a") || !strcmp(field, "r") ||
                !strcmp(field, "g") || !strcmp(field, "b"))) value = 255;
            values.str[column].assign(std::to_string(value).c_str());
            values.val[column] = value;
        }
        CSTR text;
        CsvToCSTR(values, text);
        return std::string(text.outstr());
    };
    const bool succeeded = graphicId >= 0 &&
        append("$SE_OBJECT_ID," + objectId) &&
        append("$SE_OBJECT_NAME," + objectName) &&
        append(makeCommand(sourceCommand.c_str(), true)) &&
        append(makeCommand(destinationCommand.c_str(), false));
    applyingHistory = false;
    if (!succeeded) {
        const bool previousReplay = replayingHistory;
        replayingHistory = true;
        RestoreDocumentSnapshot(before);
        replayingHistory = previousReplay;
        redoDocumentSnapshots = oldRedo;
        documentRevision = oldRevision;
        imageManagerGeneratedGrFocusRequest = oldGraphicFocus;
        // Only this call's newly created file is removed on failed creation.
        fs::remove(generatedPath, pathError);
        if (errorText.empty()) errorText = "Could not create the IMAGE Object.";
        if (pathError) errorText += " The unused PNG could not be removed: " + generatedPath;
        return false;
    }
    const int snapshotIndex = (int)historyDocumentSnapshots.size();
    historyDocumentSnapshots.push_back(before);
    HISTORY* history = (HISTORY*)arr_history.Get_new();
    history->op = restoreDocument;
    history->target = snapshotIndex;
    RebuildObjectModel();
    const auto& rebuilt = objectEditorModel.Objects();
    for (int model = 0; model < (int)rebuilt.size(); ++model) {
        if (rebuilt[model].editorId != objectId) continue;
        SetObjectSelection(std::vector<int>(1, model), model, model, true);
        RefreshPreviewSelectionBounds();
        break;
    }
    imagePath = generatedPath;
    return true;
}

int RunLayoutFirstObjectSelfTest() {
    namespace fs = std::filesystem;
    if (LoadCommandHelp(nullptr) != 0) return 1;
    char tempRoot[MAX_PATH] = {};
    char uniqueFile[MAX_PATH] = {};
    if (!GetTempPathA(MAX_PATH, tempRoot) ||
        !GetTempFileNameA(tempRoot, "sel", 0, uniqueFile)) return 2;
    // GetTempFileName reserves a unique path; use that name for our test folder.
    if (!DeleteFileA(uniqueFile) || !CreateDirectoryA(uniqueFile, nullptr)) return 3;
    const fs::path directory(uniqueFile);
    struct Cleanup {
        fs::path directory;
        ~Cleanup() { std::error_code ec; fs::remove_all(directory, ec); }
    } cleanup{directory};

    auto workspace = std::make_unique<WORKSPACE>();
    workspace->skinfileLines.Alloc(sizeof(SKINFILELINEREAD), 16);
    workspace->arr_CustomFile.Alloc(sizeof(CSTR), 2);
    workspace->arr_SRCGR.Alloc(sizeof(SRCGR), 2);
    workspace->arr_IMG.Alloc(sizeof(IMG), 2);
    workspace->arr_SRC.Alloc(sizeof(SRC), 2);
    workspace->arr_DST.Alloc(sizeof(DST), 2);
    workspace->arr_seobj.Alloc(sizeof(SEOBJ), 2);
    workspace->arr_ifunit.Alloc(sizeof(IFUNIT), 2);
    workspace->arr_history.Alloc(sizeof(HISTORY), 2);
    const std::string root = (directory / "main.lr2skin").string();
    const std::string include = (directory / "parts.csv").string();
    strncpy_s(workspace->mainpath, root.c_str(), _TRUNCATE);
    const auto append = [&](const std::string& text, const std::string& owner) {
        auto* line = (SKINFILELINEREAD*)workspace->skinfileLines.Get_new();
        line->line.assign(text.c_str());
        line->filename.assign(owner.c_str());
        line->isComment = text[0] != '#';
        line->isSEcomment = text[0] == '$';
        SplitCSV(line->line, &line->csv, ",");
        line->csvColumnCount = CountCsvColumns(line->line);
    };
    append("$FILE '" + root + "' start", root);
    append("#INFORMATION,0,Layout test,test", root);
    append("#INCLUDE,parts.csv", root);
    append("$FILE '" + include + "' start", include);
    append("#IF,900", include);
    append("$SE_OBJECT_ID,layout-anchor", include);
    append("#SRC_IMAGE,0,0,0,0,1,1,1,1,0,0,0,0,0", include);
    append("#DST_IMAGE,0,0,0,0,1,1,0,255,255,255,255,0,0,0,0,0,0,0,0,0", include);
    append("#ENDIF", include);
    append("$FILE '" + include + "' end", include);
    append("$FILE '" + root + "' end", root);
    if (workspace->RebuildEditorDerivedState() != 0 ||
        !workspace->objectEditorModel.LoadGroups(nullptr)) return 4;
    workspace->RebuildObjectModel();
    workspace->loaded = true;
    const int originalRows = workspace->skinfileLines.count;
    const int anchorBranch = workspace->objectEditorModel.Objects()[0].ifgroup;
    std::string imagePath, error;
    if (workspace->CreateImageObjectFromLayout(0, 0, 0, 16, "Bad",
        imagePath, error) || workspace->skinfileLines.count != originalRows ||
        workspace->arr_history.count != 0) return 5;
    if (!workspace->CreateImageObjectFromLayout(-7, 23, 32, 16, "Draw later",
        imagePath, error, 0)) return 6;
    if (workspace->arr_history.count != 1 ||
        workspace->skinfileLines.count != originalRows + 6) return 7;
    int width = 0, height = 0;
    unsigned char alpha = 255;
    if (!GetImageSizeFromFile(imagePath.c_str(), &width, &height) ||
        width != 32 || height != 16 ||
        !ReadImageFilePixelAlpha(imagePath.c_str(), 31, 15, &alpha) || alpha != 0)
        return 8;
    const std::string id = workspace->objectSelection.active.editorId;
    const auto validate = [&]() {
        const int model = workspace->ResolveObjectSelectionKey(workspace->objectSelection.active);
        if (model < 0) return false;
        const auto& object = workspace->objectEditorModel.Objects()[model];
        if (object.editorId != id || object.rows.size() != 2 ||
            object.ifgroup != anchorBranch) return false;
        auto* lines = (SKINFILELINEREAD*)workspace->skinfileLines.data;
        auto& src = lines[object.rows[0]];
        auto& dst = lines[object.rows[1]];
        return src.csv.str[0].isSame("#SRC_IMAGE") &&
            dst.csv.str[0].isSame("#DST_IMAGE") &&
            src.csv.val[5] == 32 && src.csv.val[6] == 16 &&
            src.csv.val[7] == 1 && src.csv.val[8] == 1 &&
            dst.csv.val[3] == -7 && dst.csv.val[4] == 23 &&
            dst.csv.val[5] == 32 && dst.csv.val[6] == 16 &&
            dst.csv.val[8] == 255 && dst.csv.val[17] == 0 &&
            src.filename.isSame(include.c_str()) && dst.filename.isSame(include.c_str());
    };
    if (!validate() || workspace->RebuildEditorDerivedState() != 0) return 9;
    workspace->RebuildObjectModel();
    if (!validate() || workspace->FindImageAssetForObject(
        workspace->preview_selected_object_model_index) < 0) return 10;
    const int selectedAsset = workspace->src_selected;
    const int manualGraphicIndex = workspace->arr_SRCGR.count;
    const int logicalGr = ((IMG*)workspace->arr_IMG.data)[selectedAsset].gr;
    auto* alternate = (SRCGR*)workspace->arr_SRCGR.Get_new();
    alternate->grID = logicalGr;
    alternate->isIf = 999;
    alternate->path.assign((directory / "manual.png").string().c_str());
    workspace->imageManagerManualTexturePath = alternate->path.outstr();
    workspace->imageManagerManualTextureGr = logicalGr;
    if (!workspace->SelectIMGAsset(selectedAsset, false) ||
        workspace->gr_selected != manualGraphicIndex ||
        !workspace->SelectIMGAsset(selectedAsset, false) ||
        workspace->gr_selected != manualGraphicIndex) return 52;
    workspace->imageManagerManualTexturePath.clear();
    workspace->imageManagerManualTextureGr = -1;
    if (workspace->RebuildEditorDerivedState() != 0) return 53;
    workspace->RebuildObjectModel();

    // Register a gesture result through the same command as Image Manager.
    const TransparentAssetCrop gestureCrop{1, 1, 4, 3, true};
    const int gestureGraphic = workspace->ResolveIMGTextureIndex(workspace->src_selected);
    if (gestureGraphic < 0) return 54;
    const int gestureGr = ((SRCGR*)workspace->arr_SRCGR.data)[gestureGraphic].grID;
    const int gestureBranch = ((SRCGR*)workspace->arr_SRCGR.data)[gestureGraphic].isIf;
    const int gestureRows = workspace->skinfileLines.count;
    const int gestureHistory = workspace->arr_history.count;
    const auto gestureObjects = workspace->objectEditorModel.Objects().size();
    const auto findGesture = [&]() {
        return workspace->FindIMG(gestureGr, 1, 1, 4, 3, gestureBranch);
    };
    if (findGesture() != workspace->arr_IMG.count ||
        !workspace->RegisterImageRegion(gestureGraphic, gestureCrop, error) ||
        workspace->skinfileLines.count != gestureRows + 1 ||
        workspace->arr_history.count != gestureHistory + 1 ||
        workspace->RebuildEditorDerivedState() != 0) return 55;
    workspace->RebuildObjectModel();
    const int gestureAsset = findGesture();
    if (gestureAsset < 0 || gestureAsset >= workspace->arr_IMG.count ||
        workspace->objectEditorModel.Objects().size() != gestureObjects) return 56;
    const int gestureDeclaration = ((IMG*)workspace->arr_IMG.data)[gestureAsset].editorDeclare;
    if (gestureDeclaration < 0 ||
        workspace->imageManagerAssetDeclarationFocusRequest != gestureDeclaration ||
        !workspace->RegisterImageRegion(workspace->ResolveIMGTextureIndex(gestureAsset),
            gestureCrop, error) || workspace->skinfileLines.count != gestureRows + 1 ||
        workspace->arr_history.count != gestureHistory + 1) return 57;
    if (workspace->UndoLastEdit() != 0 ||
        workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        workspace->RebuildEditorDerivedState() != 0 ||
        workspace->skinfileLines.count != gestureRows ||
        findGesture() != workspace->arr_IMG.count) return 58;
    workspace->RebuildObjectModel();
    if (workspace->RedoLastEdit() != 0 ||
        workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        workspace->RebuildEditorDerivedState() != 0 ||
        findGesture() >= workspace->arr_IMG.count) return 59;
    workspace->RebuildObjectModel();
    if (workspace->UndoLastEdit() != 0 ||
        workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        workspace->RebuildEditorDerivedState() != 0) return 60;
    workspace->RebuildObjectModel();

    // The generated asset is a real paintable PNG. Undo/Redo must not erase
    // artwork added after creation, so only document references are undone.
    PDIRECT3DTEXTURE9 texture = nullptr;
    if (!LoadTextureFromFile(imagePath.c_str(), &texture, &width, &height)) return 11;
    char paintError[256] = {};
    const bool painted = PaintTextureLine(texture, 2, 3, 2, 3,
        D3DCOLOR_ARGB(255, 255, 0, 0)) && SaveTextureToImageFileAtomic(
            imagePath.c_str(), texture, paintError, sizeof(paintError));
    texture->Release();
    if (!painted) return 12;
    if (workspace->UndoLastEdit() != 0 ||
        workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        workspace->skinfileLines.count != originalRows || !fs::exists(imagePath)) return 13;
    if (workspace->RebuildEditorDerivedState() != 0) return 14;
    workspace->RebuildObjectModel();
    if (workspace->RedoLastEdit() != 0 ||
        workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        workspace->RebuildEditorDerivedState() != 0) return 15;
    workspace->RebuildObjectModel();
    if (!validate() || !ReadImageFilePixelAlpha(imagePath.c_str(), 2, 3, &alpha) ||
        alpha != 255) return 16;
    const int cropRowsBefore = workspace->skinfileLines.count;
    const int cropHistoryBefore = workspace->arr_history.count;
    const int cropObjectsBefore = (int)workspace->objectEditorModel.Objects().size();
    std::vector<TransparentAssetCrop> detected = {{0, 0, 4, 4, true}, {10, 8, 2, 3, true}, {20, 0, 1, 1, false}};
    if (workspace->RegisterImageWithTransparentCrops(-1, imagePath.c_str(), 32, 16,
        detected, error) < 0 || workspace->arr_history.count != cropHistoryBefore + 1 ||
        workspace->skinfileLines.count != cropRowsBefore + 3 ||
        workspace->RebuildEditorDerivedState() != 0) return 50;
    workspace->RebuildObjectModel();
    if ((int)workspace->objectEditorModel.Objects().size() != cropObjectsBefore ||
        workspace->UndoLastEdit() != 0 || workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        workspace->skinfileLines.count != cropRowsBefore ||
        workspace->RebuildEditorDerivedState() != 0) return 51;
    workspace->RebuildObjectModel();
    for (int kind = 0; kind < 5; ++kind) {
        SELayoutImageOptions options;
        options.kind = kind;
        options.divX = kind == 1 ? 10 : 3;
        options.divY = kind == 1 ? 1 : 2;
        options.cycle = kind == 1 ? 0 : 600;
        options.digits = 4;
        const int rowCount = workspace->skinfileLines.count;
        const int historyCount = workspace->arr_history.count;
        if (!workspace->CreateImageObjectFromLayout(11, 12, 8, 9, "Sprite",
            imagePath, error, -1, options)) return 17 + kind;
        if (workspace->arr_history.count != historyCount + 1 ||
            !GetImageSizeFromFile(imagePath.c_str(), &width, &height) ||
            width != 8 * options.divX || height != 9 * options.divY ||
            workspace->RebuildEditorDerivedState() != 0) return 22 + kind;
        workspace->RebuildObjectModel();
        const int model = workspace->ResolveObjectSelectionKey(workspace->objectSelection.active);
        if (model < 0) return 27 + kind;
        const auto& object = workspace->objectEditorModel.Objects()[model];
        if (object.rows.size() != 2 || object.ifgroup != 0) return 32 + kind;
        auto* lines = (SKINFILELINEREAD*)workspace->skinfileLines.data;
        auto& src = lines[object.rows[0]].csv;
        const auto& dst = lines[object.rows[1]].csv;
        const std::string command = std::string("#SRC_") + SELayoutImageType(kind);
        if (!src.str[0].isSame(command.c_str()) || src.val[5] != width ||
            src.val[6] != height || src.val[7] != options.divX ||
            src.val[8] != options.divY || src.val[9] != options.cycle ||
            dst.val[5] != 8 || dst.val[6] != 9 ||
            (kind == 1 && (src.val[12] != 1 || src.val[13] != 4))) return 37 + kind;
        if (workspace->UndoLastEdit() != 0 ||
            workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
            workspace->skinfileLines.count != rowCount || !fs::exists(imagePath) ||
            workspace->RebuildEditorDerivedState() != 0) return 42 + kind;
        workspace->RebuildObjectModel();
    }
    SELayoutImageOptions invalid;
    invalid.divX = INT_MAX;
    if (SELayoutImageSize(64, 64, invalid, width, height)) return 47;
    invalid.divX = 0;
    if (SELayoutImageSize(64, 64, invalid, width, height)) return 48;
    invalid.divX = 2;
    if (SELayoutImageSize(16384, 16384, invalid, width, height)) return 49;
    // Split an object inside an include/IF. The disk expansion must retain
    // its command order, owner metadata and a single snapshot undo boundary.
    int splitModel = -1;
    for (int i = 0; i < (int)workspace->objectEditorModel.Objects().size(); ++i)
        if (workspace->objectEditorModel.Objects()[i].editorId == id) splitModel = i;
    if (splitModel < 0) return 70;
    workspace->SetObjectSelection({splitModel}, splitModel, splitModel, false);
    const auto splitBefore = workspace->CaptureDocumentSnapshot();
    const int splitHistory = workspace->arr_history.count;
    if (workspace->SplitSelectedObjects("../bad.csv", error)) return 71;
    if (!workspace->SplitSelectedObjects("extracted.csv", error)) return 72;
    if (!fs::exists(directory / "extracted.csv") ||
        workspace->arr_history.count != splitHistory + 1) return 73;
    const auto readText = [](const fs::path& file) {
        std::ifstream input(file, std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(input), {});
    };
    const std::string extracted = readText(directory / "extracted.csv");
    const std::string parent = readText(include);
    if (extracted.find("#SRC_IMAGE") == std::string::npos ||
        extracted.find("#DST_IMAGE") == std::string::npos ||
        extracted.find("$FILE") != std::string::npos ||
        parent.find("#INCLUDE,extracted.csv") == std::string::npos ||
        parent.find("#IF,900") > parent.find("#INCLUDE,extracted.csv") ||
        parent.find("#ENDIF") < parent.find("#INCLUDE,extracted.csv")) return 74;
    if (workspace->SplitSelectedObjects("extracted.csv", error) ||
        workspace->arr_history.count != splitHistory + 1) return 75;
    if (workspace->UndoLastEdit() != 0 ||
        workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        workspace->CaptureDocumentSnapshot().lines.size() != splitBefore.lines.size()) return 76;
    const auto undone = workspace->CaptureDocumentSnapshot();
    for (size_t i = 0; i < undone.lines.size(); ++i)
        if (undone.lines[i].line != splitBefore.lines[i].line ||
            undone.lines[i].filename != splitBefore.lines[i].filename) return 77;
    if (workspace->RedoLastEdit() != 0 ||
        workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        workspace->SaveSkinScript(workspace->mainpath, true, false) != 0) return 78;
    auto reopened = std::make_unique<WORKSPACE>();
    if (reopened->ResetEditorDocumentForLoad() != 0) return 79;
    strncpy_s(reopened->mainpath, root.c_str(), _TRUNCATE);
    if (reopened->LoadSkinScript(reopened->mainpath) < 0 ||
        reopened->RebuildEditorDerivedState() != 0 ||
        !reopened->objectEditorModel.LoadGroups(nullptr)) return 80;
    reopened->RebuildObjectModel();
    const auto runtimeLines = [](const SkinDocumentSnapshot& snapshot) {
        std::vector<std::string> result;
        for (const auto& line : snapshot.lines)
            if (!line.line.empty() && line.line[0] == '#' &&
                line.line.rfind("#INCLUDE", 0) != 0) result.push_back(line.line);
        return result;
    };
    if (runtimeLines(reopened->CaptureDocumentSnapshot()) != runtimeLines(splitBefore)) return 81;
    bool foundSplit = false;
    for (const auto& object : reopened->objectEditorModel.Objects()) {
        if (object.editorId != id) continue;
        foundSplit = object.rows.size() == 2 && object.ifgroup == anchorBranch &&
            ((SKINFILELINEREAD*)reopened->skinfileLines.data)[object.rows[0]].filename.isSame(
                (directory / "extracted.csv").string().c_str());
    }
    if (!foundSplit) return 82;
    reopened->loaded = true;
    const auto fileRevision = reopened->documentRevision;
    const std::string splitOwner = (directory / "extracted.csv").string();
    if (!reopened->SetObjectBrowserFile(splitOwner)) return 83;
    int filteredCount = 0;
    for (int i = 0; i < (int)reopened->objectEditorModel.Objects().size(); ++i)
        if (reopened->ObjectMatchesFile(i)) ++filteredCount;
    if (filteredCount != 1 || !reopened->PrepareNewObjectInBrowserFile() ||
        !reopened->newObjectOwner.isSame(splitOwner.c_str()) ||
        reopened->newObjectIfgroup != anchorBranch ||
        reopened->documentRevision != fileRevision ||
        reopened->SetObjectBrowserFile("missing.csv")) return 84;
    if (!reopened->SetObjectBrowserFile("")) return 85;
    for (int i = 0; i < (int)reopened->objectEditorModel.Objects().size(); ++i)
        if (!reopened->ObjectMatchesFile(i)) return 86;
    return 0;
}
