#include "winWorkspace.h"

#include "winWorkspaceUiHelpers.h"
#include "seHelper.h"
#include "olrSkin.h"
#include "inputwrap.h"
#include "uiCatalog.h"
#include "seUI.h"
#include "../LR2/En_fileutil.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <objidl.h>
#include <gdiplus.h>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#pragma comment(lib, "gdiplus.lib")

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
    // Plain LR2 Objects may have no editor ID, so selection falls back to a row.
    // Translate those anchors with the inserted markers before rebuilding.
    const auto remapSelection = [first, last](SEObjectSelectionKey& key) {
        if (key.anchorRow >= first)
            key.anchorRow += key.anchorRow <= last ? 2 : 3;
    };
    for (auto& key : after.selection.selected) remapSelection(key);
    remapSelection(after.selection.active);
    remapSelection(after.selection.anchor);
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
        const std::string report = scriptSaveReport;
        rollback(); error = "Save failed; the in-memory split was rolled back.\n" + report;
        return false;
    }
    const int snapshot = (int)historyDocumentSnapshots.size();
    historyDocumentSnapshots.push_back(before);
    auto* history = (HISTORY*)arr_history.Get_new();
    history->op = restoreDocument;
    history->target = snapshot;
    MarkDocumentSaved();
    if (!scriptSaveReport.empty()) lastSaveMessage += "\n" + scriptSaveReport;
    return true;
}

namespace {
constexpr size_t codeEditorCapacity = 4 * 1024 * 1024;
bool CodeFileMarker(const SkinLineSnapshot& line) {
    return line.line.rfind("$FILE ", 0) == 0;
}
std::string CodeCommand(const std::string& line) {
    const size_t first = line.find_first_not_of(" \t");
    if (first == std::string::npos) return "";
    const size_t end = line.find_first_of(", \t", first);
    return line.substr(first, end == std::string::npos ? end : end - first);
}
bool EncodeCodeDraft(const char* utf8, std::string& cp932) {
    const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, nullptr, 0);
    if (length <= 0) return false;
    std::vector<wchar_t> wide(length);
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, wide.data(), length)) return false;
    BOOL substituted = FALSE;
    const int bytes = WideCharToMultiByte(932, WC_NO_BEST_FIT_CHARS, wide.data(), -1,
        nullptr, 0, nullptr, &substituted);
    if (bytes <= 0 || substituted) return false;
    std::vector<char> encoded(bytes);
    if (!WideCharToMultiByte(932, WC_NO_BEST_FIT_CHARS, wide.data(), -1,
        encoded.data(), bytes, nullptr, &substituted) || substituted) return false;
    cp932.assign(encoded.data());
    return true;
}
}

bool WORKSPACE::ApplyCustomFileDraft() {
    if (!loaded || applyingHistory || pendingHistorySnapshotRestore >= 0 ||
        customFileDraftDocument != mainpath || customFileDraftRevision != documentRevision ||
        customFileDraftRow < 0 || customFileDraftRow >= skinfileLines.count) {
        customFileStatus = "Document changed; discard the draft and reselect the declaration.";
        return false;
    }
    SKINFILELINEREAD& row = ((SKINFILELINEREAD*)skinfileLines.data)[customFileDraftRow];
    if (!row.line.body || customFileDraftOriginal != row.line.body ||
        !row.csv.str[0].body || !row.csv.str[0].isSame("#CUSTOMFILE")) return false;
    std::string title, pattern, defaultValue;
    if (!EncodeCodeDraft(customFileTitle, title) || !EncodeCodeDraft(customFilePattern, pattern) ||
        !EncodeCodeDraft(customFileDefault, defaultValue)) {
        customFileStatus = "A field cannot be represented in CP932."; return false;
    }
    for (const std::string* value : { &title, &pattern, &defaultValue }) {
        if (value->find_first_of(",\r\n") != std::string::npos) {
            customFileStatus = "CSV fields cannot contain commas or newlines."; return false;
        }
    }
    if (title.empty() || pattern.empty()) {
        customFileStatus = "Title and pattern are required."; return false;
    }
    const std::string replacement = "#CUSTOMFILE," + title + "," + pattern + "," + defaultValue;
    if (replacement != customFileDraftOriginal &&
        EditLine(customFileDraftRow, CSTR(customFileDraftOriginal.c_str()), CSTR(replacement.c_str())) != 0) {
        customFileStatus = "Could not edit the declaration."; return false;
    }
    customFileDraftDirty = false;
    customFileStatus = "Applied to document (Undo available). Use Save to write CSV.";
    return true;
}

bool WORKSPACE::LoadCodeEditorFile(const std::string& owner) {
    std::string text;
    bool found = false;
    for (const auto& line : CaptureDocumentSnapshot().lines) {
        if (_stricmp(line.filename.c_str(), owner.c_str())) continue;
        found = true;
        if (CodeFileMarker(line)) continue;
        text += Cp932ToUtf8(line.line.c_str());
        text += '\n';
    }
    if (!found || text.size() >= codeEditorCapacity) {
        codeEditorStatus = "File unavailable or larger than the 4 MiB draft limit.";
        return false;
    }
    codeEditorOwner = owner;
    codeEditorDocument = mainpath;
    codeEditorBase = text;
    codeEditorRevision = documentRevision;
    codeEditorBuffer.assign(codeEditorCapacity, '\0');
    memcpy(codeEditorBuffer.data(), text.c_str(), text.size());
    codeEditorAssist = {};
    codeEditorStatus.clear();
    return true;
}

void WORKSPACE::RebuildScriptDirectoryTree() {
    namespace fs = std::filesystem;
    scriptDirectoryTree.clear();
    scriptDirectoryTree.push_back({"Scripts", "", {}});
    std::error_code pathError;
    fs::path main = fs::absolute(fs::path(mainpath), pathError);
    if (pathError) main = fs::path(mainpath);
    fs::path base = main.parent_path().lexically_normal();
    bool rootedAtLr2Files = false;
    for (fs::path parent = base; !parent.empty(); parent = parent.parent_path()) {
        if (!_stricmp(parent.filename().string().c_str(), "LR2files")) {
            base = parent;
            rootedAtLr2Files = true;
            break;
        }
        if (parent == parent.parent_path()) break;
    }
    std::string basePrefix = base.string();
    if (!basePrefix.empty() && basePrefix.back() != '\\' && basePrefix.back() != '/') basePrefix += '\\';
    std::set<std::string> seen;
    for (int row = 0; row < skinfileLines.count; ++row) {
        const auto& source = ((SKINFILELINEREAD*)skinfileLines.data)[row];
        if (!source.filename.body || !*source.filename.body) continue;
        const std::string owner = source.filename.body;
        std::string key = owner;
        std::transform(key.begin(), key.end(), key.begin(), [](unsigned char c) {
            return c >= 'A' && c <= 'Z' ? char(c + 'a' - 'A') : char(c);
        });
        if (!seen.insert(key).second) continue;
        pathError.clear();
        fs::path ownerPath = fs::absolute(fs::path(owner), pathError);
        if (pathError) ownerPath = fs::path(owner);
        ownerPath = ownerPath.lexically_normal();
        const std::string ownerPathText = ownerPath.string();
        const bool inside = !basePrefix.empty() &&
            !_strnicmp(ownerPathText.c_str(), basePrefix.c_str(), basePrefix.size());
        fs::path relative = inside ? fs::path(ownerPathText.substr(basePrefix.size())) : ownerPath;
        std::vector<std::string> parts;
        if (!inside) {
            parts.push_back("External files");
        } else if (rootedAtLr2Files) parts.push_back("LR2files");
        for (const auto& part : relative) {
            if (part != "." && !part.empty()) parts.push_back(part.string());
        }
        if (parts.empty()) continue;
        int parent = 0;
        for (size_t i = 0; i < parts.size(); ++i) {
            const bool leaf = i + 1 == parts.size();
            int child = -1;
            for (int candidate : scriptDirectoryTree[parent].children) {
                const auto& node = scriptDirectoryTree[candidate];
                if (!_stricmp(node.label.c_str(), parts[i].c_str()) &&
                    node.owner.empty() == !leaf) { child = candidate; break; }
            }
            if (child < 0) {
                child = (int)scriptDirectoryTree.size();
                scriptDirectoryTree.push_back({parts[i], leaf ? owner : "", {}});
                scriptDirectoryTree[parent].children.push_back(child);
            }
            parent = child;
        }
    }
    for (auto& node : scriptDirectoryTree)
        std::sort(node.children.begin(), node.children.end(), [&](int a, int b) {
            const auto& left = scriptDirectoryTree[a];
            const auto& right = scriptDirectoryTree[b];
            const bool leftMain = !left.owner.empty() && !_stricmp(left.owner.c_str(), mainpath);
            const bool rightMain = !right.owner.empty() && !_stricmp(right.owner.c_str(), mainpath);
            if (leftMain != rightMain) return leftMain;
            if (left.owner.empty() != right.owner.empty()) return left.owner.empty();
            return _stricmp(left.label.c_str(), right.label.c_str()) < 0;
        });
    scriptDirectoryRevision = documentRevision;
    scriptDirectoryDocument = mainpath;
}

bool WORKSPACE::OpenScriptInCodeEditor(const std::string& owner) {
    wCodeEditor = true;
    if (!codeEditorBuffer.empty() && codeEditorBase != codeEditorBuffer.data()) {
        if (codeEditorOwner == owner && codeEditorDocument == mainpath) {
            codeEditorRevealRequested = true;
            return true;
        }
        scriptDirectoryStatus = "Text Editor has an unapplied draft. Apply or discard it before opening another file.";
        return false;
    }
    if (!LoadCodeEditorFile(owner)) { scriptDirectoryStatus = codeEditorStatus; return false; }
    codeEditorRevealRequested = true;
    scriptDirectoryStatus.clear();
    return true;
}

bool WORKSPACE::ApplyCodeEditorDraft() {
    if (!loaded || codeEditorBuffer.empty() || applyingHistory || pendingHistorySnapshotRestore >= 0 ||
        codeEditorDocument != mainpath || codeEditorRevision != documentRevision) {
        codeEditorStatus = "Document changed elsewhere. Copy your draft, then discard/reload before applying.";
        return false;
    }
    if (codeEditorBase == codeEditorBuffer.data()) return true;
    std::string text;
    if (!EncodeCodeDraft(codeEditorBuffer.data(), text)) {
        codeEditorStatus = "Some characters cannot be stored as Shift-JIS (CP932). Nothing was applied.";
        return false;
    }
    const auto before = CaptureDocumentSnapshot();
    int start = -1, finish = -1, starts = 0;
    for (int i = 0; i < (int)before.lines.size(); ++i) {
        const auto& line = before.lines[i];
        if (_stricmp(line.filename.c_str(), codeEditorOwner.c_str()) || !CodeFileMarker(line)) continue;
        if (line.line.size() >= 6 && line.line.compare(line.line.size() - 6, 6, " start") == 0) {
            start = i; ++starts;
        } else if (line.line.size() >= 4 && line.line.compare(line.line.size() - 4, 4, " end") == 0) finish = i;
    }
    if (starts != 1 || start < 0 || finish <= start) {
        codeEditorStatus = "This file has no unique include instance. Repeated includes are read-only here.";
        return false;
    }
    // Preserve expanded child files; only this owner's source lines are replaced.
    std::vector<std::string> includeLines;
    std::vector<std::vector<SkinLineSnapshot>> children;
    for (int i = start + 1; i < finish; ++i) {
        const auto& line = before.lines[i];
        if (_stricmp(line.filename.c_str(), codeEditorOwner.c_str())) {
            codeEditorStatus = "Unrecognized include expansion; nothing was applied."; return false;
        }
        if (!_stricmp(CodeCommand(line.line).c_str(), "#INCLUDE")) {
            includeLines.push_back(line.line);
            children.emplace_back();
            while (i + 1 < finish && _stricmp(before.lines[i + 1].filename.c_str(), codeEditorOwner.c_str()))
                children.back().push_back(before.lines[++i]);
        }
    }
    std::vector<SkinLineSnapshot> replacement;
    size_t offset = 0, includeIndex = 0;
    int sourceNumber = 0;
    std::vector<bool> conditionElse;
    while (offset < text.size()) {
        const size_t end = text.find('\n', offset);
        std::string line = text.substr(offset, end == std::string::npos ? end : end - offset);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        offset = end == std::string::npos ? text.size() : end + 1;
        const auto command = CodeCommand(line);
        if (command == "$FILE") { codeEditorStatus = "$FILE markers are internal and cannot be entered."; return false; }
        if (!_stricmp(command.c_str(), "#IF")) conditionElse.push_back(false);
        else if (!_stricmp(command.c_str(), "#ELSE") || !_stricmp(command.c_str(), "#ELSEIF")) {
            if (conditionElse.empty() || conditionElse.back()) { codeEditorStatus = "Invalid ELSE/ELSEIF nesting."; return false; }
            if (!_stricmp(command.c_str(), "#ELSE")) conditionElse.back() = true;
        } else if (!_stricmp(command.c_str(), "#ENDIF")) {
            if (conditionElse.empty()) { codeEditorStatus = "Unmatched ENDIF in this file."; return false; }
            conditionElse.pop_back();
        }
        SkinLineSnapshot source;
        source.filename = codeEditorOwner;
        source.line = line;
        source.num = ++sourceNumber;
        source.modified = true;
        replacement.push_back(source);
        if (!_stricmp(command.c_str(), "#INCLUDE")) {
            if (includeIndex >= includeLines.size() || line != includeLines[includeIndex]) {
                codeEditorStatus = "Keep INCLUDE lines and their order unchanged in this first version."; return false;
            }
            replacement.insert(replacement.end(), children[includeIndex].begin(), children[includeIndex].end());
            ++includeIndex;
        }
    }
    if (!conditionElse.empty() || includeIndex != includeLines.size()) {
        codeEditorStatus = "Close all IF blocks and keep every original INCLUDE line."; return false;
    }
    auto after = before;
    after.lines.erase(after.lines.begin() + start + 1, after.lines.begin() + finish);
    after.lines.insert(after.lines.begin() + start + 1, replacement.begin(), replacement.end());
    const auto oldRedo = redoDocumentSnapshots;
    const auto oldRevision = documentRevision;
    if (RestoreDocumentSnapshot(after) != 0 || RebuildEditorDerivedState() != 0) {
        const bool oldReplay = replayingHistory;
        replayingHistory = true;
        RestoreDocumentSnapshot(before);
        replayingHistory = oldReplay;
        redoDocumentSnapshots = oldRedo;
        documentRevision = oldRevision;
        RebuildEditorDerivedState();
        RebuildObjectModel();
        codeEditorStatus = "Rebuild failed; document restored. Draft kept.";
        return false;
    }
    RebuildObjectModel();
    const int snapshot = (int)historyDocumentSnapshots.size();
    historyDocumentSnapshots.push_back(before);
    auto* history = (HISTORY*)arr_history.Get_new();
    history->op = restoreDocument;
    history->target = snapshot;
    LoadCodeEditorFile(codeEditorOwner);
    codeEditorStatus = "Applied to Workspace. Use Save to write files.";
    return true;
}

void WORKSPACE::drawCodeEditor() {
    char title[128];
    FormatSEUIWindowTitle(title, sizeof(title), SEUIWindowId::CodeEditor, num);
    ImGui::SetNextWindowSize(ImVec2(760, 520), ImGuiCond_FirstUseEver);
    if (codeEditorRevealRequested) {
        SEUI::RevealWindowTab(title);
        codeEditorRevealRequested = false;
    }
    if (!ImGui::Begin(title, &wCodeEditor)) { ImGui::End(); return; }
    if (!loaded) { ImGui::TextUnformatted("Open a skin first."); ImGui::End(); return; }
    if (codeEditorBuffer.empty()) LoadCodeEditorFile(objectBrowserFile.empty() ? mainpath : objectBrowserFile);
    const bool dirty = !codeEditorBuffer.empty() && codeEditorBase != codeEditorBuffer.data();
    ImGui::BeginDisabled(dirty);
    const std::string filePreview = Cp932ToUtf8(codeEditorOwner.c_str());
    if (ImGui::BeginCombo("File", filePreview.c_str())) {
        std::set<std::string> owners;
        for (const auto& line : CaptureDocumentSnapshot().lines) owners.insert(line.filename);
        for (const auto& owner : owners) {
            if (owner.empty()) continue;
            if (ImGui::Selectable(Cp932ToUtf8(owner.c_str()).c_str(), owner == codeEditorOwner)) LoadCodeEditorFile(owner);
        }
        ImGui::EndCombo();
    }
    ImGui::EndDisabled();
    if (ImGui::Button("Apply")) ApplyCodeEditorDraft();
    ImGui::SameLine();
    if (ImGui::Button(dirty ? "Discard draft" : "Reload from Workspace")) {
        const std::string owner = codeEditorDocument == mainpath ? codeEditorOwner : std::string(mainpath);
        LoadCodeEditorFile(owner);
    }
    ImGui::SameLine();
    ImGui::TextDisabled(dirty ? "Draft (not saved)" : "Workspace copy");
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Ctrl+Z inside the field undoes text edits. Apply is one Workspace undo step. Closing this window keeps the draft.");
    if (!codeEditorStatus.empty()) ImGui::TextWrapped("%s", codeEditorStatus.c_str());
    if (codeEditorRevision != documentRevision || codeEditorDocument != mainpath)
        ImGui::TextWrapped("Workspace changed. Copy the draft before reloading; Apply is blocked.");
    if (!codeEditorBuffer.empty())
        SEDrawCodeEditorInput("##CodeDraft", codeEditorBuffer.data(), codeEditorBuffer.size(),
            ImVec2(-FLT_MIN, (std::max)(80.0f, ImGui::GetContentRegionAvail().y)),
            codeEditorAssist, num);
    ImGui::End();
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
    std::string& errorText, int afterObject, const SELayoutImageOptions& options, bool createImageNow) {
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
    std::string objectId;
    std::string generatedPath;
    if (createImageNow) {
        const fs::path directory = fs::absolute(fs::path(mainpath), pathError)
            .parent_path();
        if (pathError || !fs::is_directory(directory, pathError)) {
            errorText = "The skin directory is unavailable.";
            return false;
        }
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
    } else {
        objectId = GenerateObjectId(*this);
    }
    const SkinDocumentSnapshot before = CaptureDocumentSnapshot();
    const auto oldRedo = redoDocumentSnapshots;
    const auto oldRevision = documentRevision;
    const int oldGraphicFocus = imageManagerGeneratedGrFocusRequest;
    applyingHistory = true;
    const int oldCount = skinfileLines.count;
    // An empty crop of LR2's built-in white slot is a valid, non-rendering SRC.
    // It does not reference an arbitrary user texture or consume a #IMAGE slot.
    const int graphicId = createImageNow ? RegisterGeneratedImage(generatedPath.c_str(),
        sheetWidth, sheetHeight, errorText, options.divX, options.divY, options.cycle) : 111;
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
            if (!strcmp(field, "w")) value = source ? (createImageNow ? sheetWidth : 0) : width;
            else if (!strcmp(field, "h")) value = source ? (createImageNow ? sheetHeight : 0) : height;
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
        if (!generatedPath.empty()) fs::remove(generatedPath, pathError);
        if (errorText.empty()) errorText = "Could not create the IMAGE Object.";
        if (pathError) errorText += " The unused PNG could not be removed: " + generatedPath;
        return false;
    }
    const int snapshotIndex = (int)historyDocumentSnapshots.size();
    historyDocumentSnapshots.push_back(before);
    HISTORY* history = (HISTORY*)arr_history.Get_new();
    history->op = restoreDocument;
    history->target = snapshotIndex;
    if (!createImageNow) previewLayoutMode = true;
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

namespace {
struct DstAssetPlan {
    int source = -1, width = 0, height = 0, columns = 1, rows = 1, cycle = 0;
    int sheetWidth = 0, sheetHeight = 0;
    int tileWidth = 0, tileHeight = 0;
    int atlasX = 0, atlasY = 0;
    int model = -1;
    std::string name;
};

bool ReadDstAssetPlan(WORKSPACE& workspace, int model, DstAssetPlan& plan, std::string& error) {
    error.clear();
    const auto& objects = workspace.objectEditorModel.Objects();
    if (!workspace.loaded || model < 0 || model >= (int)objects.size()) {
        error = "Select an image-backed Object in an open skin."; return false;
    }
    plan.model = model;
    plan.name = objects[model].name;
    int destination = -1;
    std::string type;
    for (int row : objects[model].rows) {
        if (row < 0 || row >= workspace.skinfileLines.count) continue;
        auto& line = ((SKINFILELINEREAD*)workspace.skinfileLines.data)[row];
        const std::string command = line.csv.str[0].body ? line.csv.str[0].outstr() : "";
        if (command.compare(0, 5, "#SRC_") == 0) {
            if (plan.source >= 0) { error = "Objects with multiple SRC commands are not supported yet."; return false; }
            plan.source = row;
            type = command.substr(5);
        }
    }
    if (plan.source < 0 || (type != "IMAGE" && type != "NUMBER" && type != "SLIDER" &&
        type != "BUTTON" && type != "BARGRAPH")) {
        error = "Supported types: IMAGE, NUMBER, SLIDER, BUTTON and BARGRAPH (one SRC)."; return false;
    }
    for (int row : objects[model].rows) {
        auto& line = ((SKINFILELINEREAD*)workspace.skinfileLines.data)[row];
        if (line.csv.str[0].isSame(("#DST_" + type).c_str())) { destination = row; break; }
    }
    if (destination < 0) { error = "This Object has no matching DST rectangle."; return false; }
    auto& src = ((SKINFILELINEREAD*)workspace.skinfileLines.data)[plan.source];
    auto& dst = ((SKINFILELINEREAD*)workspace.skinfileLines.data)[destination];
    auto field = [](SKINFILELINEREAD& line, const char* name, int fallback) {
        const int column = FindCommandFieldColumn(line.csv.str[0].outstr(), name);
        return column > 0 ? line.csv.val[column] : fallback;
    };
    const long long w = field(dst, "w", 0), h = field(dst, "h", 0);
    const long long aw = w < 0 ? -w : w, ah = h < 0 ? -h : h;
    if (aw <= 0 || ah <= 0 || aw > 16384 || ah > 16384) {
        error = "The first DST needs nonzero W/H, at most 16384 pixels per side."; return false;
    }
    plan.width = (int)aw; plan.height = (int)ah;
    plan.columns = field(src, "div_x", 1); plan.rows = field(src, "div_y", 1);
    plan.cycle = field(src, "cycle", 0);
    SELayoutImageOptions options;
    options.divX = plan.columns; options.divY = plan.rows;
    if (!SELayoutImageSize(plan.width, plan.height, options, plan.sheetWidth, plan.sheetHeight)) {
        error = "SRC divisions must be positive; the sheet limit is 16384 per side and 16 megapixels."; return false;
    }
    return true;
}
}

bool WORKSPACE::GetDstAssetSize(int model, int& width, int& height,
    int& columns, int& rows, std::string& error) {
    DstAssetPlan plan;
    if (!ReadDstAssetPlan(*this, model, plan, error)) return false;
    width = plan.width; height = plan.height; columns = plan.columns; rows = plan.rows;
    return true;
}

bool WORKSPACE::IsLayoutOnlyObject(int model) {
    DstAssetPlan plan; std::string error;
    if (!ReadDstAssetPlan(*this, model, plan, error)) return false;
    auto& line = ((SKINFILELINEREAD*)skinfileLines.data)[plan.source];
    const char* command = line.csv.str[0].outstr();
    const int gr = FindCommandFieldColumn(command, "gr");
    const int w = FindCommandFieldColumn(command, "w"), h = FindCommandFieldColumn(command, "h");
    return gr > 0 && w > 0 && h > 0 && line.csv.val[gr] == 111 &&
        line.csv.val[w] == 0 && line.csv.val[h] == 0;
}

namespace {
bool PlanDstAtlas(WORKSPACE& workspace, const std::vector<int>& models,
    std::vector<DstAssetPlan>& plans, int& width, int& height, std::string& error) {
    plans.clear(); error.clear();
    width = height = 0;
    if (models.empty() || models.size() > 512) { error = "Select 1 to 512 supported Objects."; return false; }
    std::set<int> seen;
    long long area = 0, cells = 0;
    int widest = 1;
    for (int model : models) {
        if (!seen.insert(model).second) continue;
        DstAssetPlan plan;
        if (!ReadDstAssetPlan(workspace, model, plan, error)) return false;
        // Reserve a caption band outside SRC crops so guides can be overlaid 1:1.
        const int captionWidth = (int)(std::min)(size_t(512), (std::max)(size_t(128), plan.name.size() * 10));
        plan.tileWidth = (std::max)(plan.sheetWidth, captionWidth);
        plan.tileHeight = plan.sheetHeight + 28;
        if (plan.tileHeight > 16384) { error = "Leave 28 pixels below the sheet for its name."; return false; }
        area += (long long)(plan.tileWidth + 8) * (plan.tileHeight + 8);
        cells += (long long)plan.columns * plan.rows;
        widest = (std::max)(widest, plan.tileWidth);
        plans.push_back(plan);
    }
    if (cells > 8192) { error = "The drawing guide supports at most 8192 frames per batch."; return false; }
    // Deterministic shelf packing. Each animation sheet stays contiguous.
    std::stable_sort(plans.begin(), plans.end(), [](const DstAssetPlan& a, const DstAssetPlan& b) {
        return a.sheetHeight > b.sheetHeight;
    });
    int limit = widest;
    while ((long long)limit * limit < area && limit < 16384) limit = (std::min)(16384, limit * 2);
    for (;;) {
        int x = 0, y = 0, shelfHeight = 0; width = 0;
        for (auto& plan : plans) {
            if (x && x + plan.tileWidth > limit) { y += shelfHeight + 8; x = shelfHeight = 0; }
            plan.atlasX = x; plan.atlasY = y;
            width = (std::max)(width, x + plan.tileWidth);
            x += plan.tileWidth + 8;
            shelfHeight = (std::max)(shelfHeight, plan.tileHeight);
        }
        height = y + shelfHeight;
        if (height <= 16384 && (long long)width * height <= 16 * 1024 * 1024) return true;
        if (limit == 16384) { error = "Packed atlas exceeds 16384 per side or 16 megapixels. Select fewer Objects."; return false; }
        limit = (std::min)(16384, limit * 2);
    }
}
std::string DstGuideXml(const std::string& cp932) {
    const std::string utf8 = Cp932ToUtf8(cp932.c_str());
    std::string result;
    for (unsigned char ch : utf8) {
        if (ch == '&') result += "&amp;";
        else if (ch == '<') result += "&lt;";
        else if (ch == '>') result += "&gt;";
        else if (ch == '"') result += "&quot;";
        else if (ch >= 32 || ch == '\t') result += (char)ch;
    }
    return result;
}

bool RenderDstGuide(const std::vector<DstAssetPlan>& plans, int width, int height,
    std::vector<D3DCOLOR>& pixels, std::string& error) {
    struct Session {
        ULONG_PTR token = 0;
        ~Session() { if (token) Gdiplus::GdiplusShutdown(token); }
    } session;
    Gdiplus::GdiplusStartupInput startup;
    if (Gdiplus::GdiplusStartup(&session.token, &startup, nullptr) != Gdiplus::Ok) {
        error = "Could not initialize drawing-guide text rendering."; return false;
    }
    try { pixels.assign((size_t)width * height, 0); }
    catch (const std::exception&) { error = "Not enough memory for the drawing guide."; return false; }
    // PixelFormat32bppARGB is straight alpha, matching the existing PNG writer.
    Gdiplus::Bitmap bitmap(width, height, width * 4, PixelFormat32bppARGB, (BYTE*)pixels.data());
    Gdiplus::Graphics graphics(&bitmap);
    Gdiplus::Font font(L"Segoe UI", 18.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    if (bitmap.GetLastStatus() != Gdiplus::Ok || graphics.GetLastStatus() != Gdiplus::Ok ||
        font.GetLastStatus() != Gdiplus::Ok) { error = "Could not allocate the drawing guide or font."; return false; }
    graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
    Gdiplus::SolidBrush red(Gdiplus::Color(255, 240, 30, 45));
    Gdiplus::StringFormat format;
    format.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
    format.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
    for (const auto& plan : plans) {
        // Each frame owns a 1px inner border. Adjacent right/left (or
        // bottom/top) edges occupy separate pixels, making a 2px shared seam.
        for (int y = 0; y < plan.sheetHeight; ++y) for (int x = 0; x < plan.sheetWidth; ++x) {
            const int frameX = x % plan.width, frameY = y % plan.height;
            const bool border = frameX == 0 || frameY == 0 ||
                frameX == plan.width - 1 || frameY == plan.height - 1;
            if (border) pixels[(size_t)(plan.atlasY + y) * width + plan.atlasX + x] =
                D3DCOLOR_ARGB(255, 240, 30, 45);
        }
        const std::string name = plan.name.empty() ? "Unnamed Object" : plan.name;
        const int size = MultiByteToWideChar(932, 0, name.c_str(), -1, nullptr, 0);
        if (size <= 0) { error = "Could not decode an Object name."; return false; }
        std::vector<wchar_t> label(size);
        MultiByteToWideChar(932, 0, name.c_str(), -1, label.data(), size);
        const Gdiplus::RectF box((float)plan.atlasX, (float)(plan.atlasY + plan.sheetHeight + 3),
            (float)plan.tileWidth, 25.0f);
        if (graphics.DrawString(label.data(), -1, &font, box, &format, &red) != Gdiplus::Ok) {
            error = "Could not draw an Object name."; return false;
        }
        graphics.Flush(Gdiplus::FlushIntentionSync);
    }
    return true;
}

void WriteDstGuideFrameBorder(std::ostream& svg, int x, int y, int width, int height) {
    if (width == 1 || height == 1) {
        // A one-pixel-wide/high frame consists entirely of border pixels.
        // SVG ignores zero-sized rect strokes, so use a filled strip here.
        svg << "<rect x=\"" << x << "\" y=\"" << y << "\" width=\"" << width
            << "\" height=\"" << height << "\" fill=\"#f01e2d\" shape-rendering=\"crispEdges\"/>\n";
    } else {
        // SVG strokes straddle the path. Inset the path by half a pixel so
        // its 1px stroke stays inside the same pixels as the PNG border.
        svg << "<rect x=\"" << x + 0.5 << "\" y=\"" << y + 0.5
            << "\" width=\"" << width - 1 << "\" height=\"" << height - 1
            << "\" fill=\"none\" stroke=\"#f01e2d\" stroke-width=\"1\" shape-rendering=\"crispEdges\"/>\n";
    }
}
}

bool WORKSPACE::GetDstAtlasSize(const std::vector<int>& models, int& width, int& height, std::string& error) {
    std::vector<DstAssetPlan> plans;
    return PlanDstAtlas(*this, models, plans, width, height, error);
}

bool WORKSPACE::CreateAssetFromDst(int model, std::string& imagePath, std::string& error, bool paintableGuide) {
    return CreateAtlasFromDst(std::vector<int>(1, model), imagePath, error, paintableGuide);
}

bool WORKSPACE::CreateAtlasFromDst(const std::vector<int>& models, std::string& imagePath, std::string& error, bool paintableGuide) {
    imagePath.clear(); error.clear();
    if (!imagePixelPaintDirtyPaths.empty()) { error = "Save or revert Pixel Paint edits first."; return false; }
    if (applyingHistory || pendingHistorySnapshotRestore >= 0 || !*mainpath) {
        error = "Finish the pending document operation first."; return false;
    }
    std::vector<DstAssetPlan> plans;
    int width = 0, height = 0;
    if (!PlanDstAtlas(*this, models, plans, width, height, error)) return false;
    namespace fs = std::filesystem;
    std::error_code ec;
    const fs::path directory = fs::absolute(fs::path(mainpath), ec).parent_path();
    if (ec || !fs::is_directory(directory, ec)) { error = "The skin directory is unavailable."; return false; }
    std::string path, guide, annotated;
    do {
        const std::string stem = "dst_" + GenerateObjectId(*this);
        path = (directory / (stem + ".png")).string();
        guide = (directory / (stem + "_guide.png")).string();
        annotated = (directory / (stem + "_guide.svg")).string();
    } while (!ec && (fs::exists(path, ec) || fs::exists(guide, ec) || fs::exists(annotated, ec)));
    if (ec || annotated.size() >= MAX_PATH || path.find(',') != std::string::npos) {
        error = "The new image path cannot be represented in LR2."; return false;
    }
    // Cleanup is restricted to fresh outputs owned by this invocation.
    const auto cleanup = [&]() { fs::remove(path, ec); fs::remove(guide, ec); fs::remove(annotated, ec); };
    std::vector<D3DCOLOR> pixels;
    if (!RenderDstGuide(plans, width, height, pixels, error)) return false;
    std::ofstream svg(annotated, std::ios::binary);
    if (!svg) { error = "Could not create the annotated guide."; return false; }
    const int guideWidth = width;
    const int guideHeight = height;
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << guideWidth
        << "\" height=\"" << guideHeight << "\" viewBox=\"0 0 " << guideWidth << " " << guideHeight << "\">\n"
        << "<g font-family=\"Segoe UI,sans-serif\" fill=\"#f01e2d\">\n";
    int ordinal = 0;
    for (const auto& plan : plans) {
        ++ordinal;
        svg << "<g><title>" << DstGuideXml(plan.name) << " | Frame " << plan.width << " x " << plan.height
            << " | Grid " << plan.columns << " x " << plan.rows << " | Frames 0.." << plan.columns * plan.rows - 1
            << " | IF group " << objectEditorModel.Objects()[plan.model].ifgroup << "</title>\n";
        for (int row = 0; row < plan.rows; ++row) for (int col = 0; col < plan.columns; ++col) {
            const int x = plan.atlasX + col * plan.width, y = plan.atlasY + row * plan.height;
            const int frame = row * plan.columns + col;
            const float font = (std::max)(1.0f, (std::min)(12.0f, (std::min)(plan.width / 7.0f, plan.height / 3.0f)));
            WriteDstGuideFrameBorder(svg, x, y, plan.width, plan.height);
            svg << "<text x=\"" << x + 1 << "\" y=\"" << y + font << "\" font-size=\"" << font << "\">"
                << ordinal << ":" << frame << "</text>\n";
        }
        svg << "<svg x=\"" << plan.atlasX << "\" y=\"" << plan.atlasY + plan.sheetHeight
            << "\" width=\"" << plan.tileWidth << "\" height=\"28\" overflow=\"hidden\">"
            << "<text x=\"0\" y=\"21\" font-size=\"18\">"
            << DstGuideXml(plan.name.empty() ? "Unnamed Object" : plan.name) << "</text></svg></g>\n";
    }
    svg << "</g></svg>\n"; svg.close();
    if (!svg) { cleanup(); error = "Could not finish the annotated guide."; return false; }
    char fileError[256] = {};
    // Default: edit this PNG directly, painting over the red borders. No PNG
    // sidecar in this mode, so a later reload cannot resurrect erased borders
    // through Image Manager's guide overlay. Names stay outside every SRC crop.
    const bool written = paintableGuide
        ? CreateArgbImageFileAtomic(path.c_str(), width, height, pixels.data(), pixels.size(), fileError, sizeof(fileError))
        : (CreateSolidImageFileAtomic(path.c_str(), width, height, D3DCOLOR_ARGB(0, 0, 0, 0), fileError, sizeof(fileError)) &&
            CreateArgbImageFileAtomic(guide.c_str(), width, height, pixels.data(), pixels.size(), fileError, sizeof(fileError)));
    if (!written) {
        cleanup(); error = fileError; return false;
    }
    CSTR rootOwner;
    AssignRootFileOwner(skinfileLines, mainpath, rootOwner);
    const int insertAt = FindOwnerFileEndRow(skinfileLines, rootOwner.outstr());
    const auto before = CaptureDocumentSnapshot();
    const auto oldRedo = redoDocumentSnapshots;
    const auto oldRevision = documentRevision;
    const int oldFocus = imageManagerGeneratedGrFocusRequest;
    const int oldRows = skinfileLines.count;
    applyingHistory = true;
    // Object SRC rows already define the crops. Keep the #IMAGE/gr binding
    // without adding an unused full-canvas Asset (including the caption padding).
    const int gr = RegisterGeneratedImage(path.c_str(), width, height, error, 1, 1, 0, 0, 0, false);
    bool ok = gr >= 0;
    std::vector<int> sources;
    for (const auto& plan : plans) {
        if (!ok) break;
        const int source = plan.source + (plan.source >= insertAt ? skinfileLines.count - oldRows : 0);
        if (source < 0 || source >= skinfileLines.count) { ok = false; break; }
        auto& line = ((SKINFILELINEREAD*)skinfileLines.data)[source];
        CSVbuf values; SplitCSV(line.line, &values, ",");
        const std::string command = values.str[0].outstr();
        const char* names[] = { "gr", "x", "y", "w", "h" };
        const int data[] = { gr, plan.atlasX, plan.atlasY, plan.sheetWidth, plan.sheetHeight };
        for (int i = 0; i < 5; ++i) {
            const int column = FindCommandFieldColumn(command.c_str(), names[i]);
            if (column <= 0) { ok = false; break; }
            values.str[column].assign(std::to_string(data[i]).c_str()); values.val[column] = data[i];
        }
        if (ok) {
            CSTR replacement; CsvToCSTR(values, replacement);
            CSTR previous(line.line);
            ok = EditLine(source, previous, replacement) == 0;
            sources.push_back(source);
        }
    }
    applyingHistory = false;
    if (!ok) {
        const bool replay = replayingHistory; replayingHistory = true;
        RestoreDocumentSnapshot(before); replayingHistory = replay;
        redoDocumentSnapshots = oldRedo; documentRevision = oldRevision;
        imageManagerGeneratedGrFocusRequest = oldFocus; cleanup();
        if (error.empty()) error = "Could not bind the shared atlas. No Objects were changed.";
        return false;
    }
    const int snapshot = (int)historyDocumentSnapshots.size();
    historyDocumentSnapshots.push_back(before);
    auto* history = (HISTORY*)arr_history.Get_new(); history->op = restoreDocument; history->target = snapshot;
    RebuildObjectModel();
    std::vector<int> selected;
    for (int i = 0; i < (int)objectEditorModel.Objects().size(); ++i)
        for (int source : sources) {
            const auto& rows = objectEditorModel.Objects()[i].rows;
            if (std::find(rows.begin(), rows.end(), source) != rows.end()) { selected.push_back(i); break; }
        }
    if (!selected.empty()) SetObjectSelection(selected, selected.front(), selected.front(), true);
    imagePath = path; return true;
}

bool WORKSPACE::CreateImagesFromDst(const std::vector<int>& models, bool separate,
    std::vector<std::string>& paths, std::string& error, bool paintableGuide) {
    paths.clear(); error.clear();
    if (!imagePixelPaintDirtyPaths.empty() || !*mainpath) {
        error = "Open a skin and save or revert Pixel Paint edits first."; return false;
    }
    if (!separate) {
        std::string path;
        if (!CreateAtlasFromDst(models, path, error, paintableGuide)) return false;
        paths.push_back(path); return true;
    }
    if (models.empty() || models.size() > 512 || applyingHistory || pendingHistorySnapshotRestore >= 0) {
        error = "Select 1 to 512 Objects and finish pending document operations."; return false;
    }
    // Validate the entire selection before creating any files. Keep stable keys,
    // adjusting row fallbacks when root #IMAGE metadata is inserted before an include.
    std::vector<SEObjectSelectionKey> keys;
    std::set<int> seen;
    for (int model : models) {
        if (!seen.insert(model).second) continue;
        int w, h;
        if (!GetDstAtlasSize({model}, w, h, error)) return false;
        keys.push_back(MakeObjectSelectionKey(model));
    }
    const auto before = CaptureDocumentSnapshot();
    const auto oldRedo = redoDocumentSnapshots;
    const auto oldRevision = documentRevision;
    const int oldFocus = imageManagerGeneratedGrFocusRequest;
    const int historyCount = arr_history.count;
    const size_t snapshotCount = historyDocumentSnapshots.size();
    bool success = true;
    for (size_t i = 0; i < keys.size(); ++i) {
        const int model = ResolveObjectSelectionKey(keys[i]);
        CSTR owner; AssignRootFileOwner(skinfileLines, mainpath, owner);
        const int insertion = FindOwnerFileEndRow(skinfileLines, owner.outstr());
        const int oldRows = skinfileLines.count;
        std::string path;
        if (!CreateAssetFromDst(model, path, error, paintableGuide)) { success = false; break; }
        paths.push_back(path);
        const int added = skinfileLines.count - oldRows;
        for (auto& key : keys) if (key.anchorRow >= insertion) key.anchorRow += added;
    }
    arr_history.count = historyCount;
    historyDocumentSnapshots.resize(snapshotCount);
    if (!success) {
        const bool replay = replayingHistory; replayingHistory = true;
        RestoreDocumentSnapshot(before); replayingHistory = replay;
        redoDocumentSnapshots = oldRedo; documentRevision = oldRevision;
        imageManagerGeneratedGrFocusRequest = oldFocus;
        // Only files generated by this operation are removed on failure.
        std::error_code ec;
        for (const auto& path : paths) {
            const std::string stem = path.substr(0, path.size() - 4);
            std::filesystem::remove(path, ec);
            std::filesystem::remove(stem + "_guide.png", ec);
            std::filesystem::remove(stem + "_guide.svg", ec);
        }
        paths.clear(); return false;
    }
    historyDocumentSnapshots.push_back(before);
    auto* history = (HISTORY*)arr_history.Get_new();
    history->op = restoreDocument; history->target = (int)snapshotCount;
    std::vector<int> selected;
    for (const auto& key : keys) {
        const int model = ResolveObjectSelectionKey(key);
        if (model >= 0) selected.push_back(model);
    }
    if (!selected.empty()) SetObjectSelection(selected, selected.front(), selected.front(), true);
    return true;
}

static int TestDstGuideFrameBorders() {
    // Horizontal BUTTON/NUMBER sheets, a 2D animation and very small cells.
    const int grids[][4] = { {8, 6, 2, 1}, {8, 6, 10, 1}, {8, 6, 3, 2},
        {1, 1, 2, 2}, {1, 5, 3, 2}, {5, 1, 2, 3}, {2, 2, 3, 2} };
    for (const auto& grid : grids) {
        DstAssetPlan plan;
        plan.width = grid[0]; plan.height = grid[1];
        plan.columns = grid[2]; plan.rows = grid[3];
        plan.sheetWidth = plan.width * plan.columns;
        plan.sheetHeight = plan.height * plan.rows;
        plan.atlasX = 3; plan.atlasY = 4;
        plan.tileWidth = (std::max)(128, plan.sheetWidth);
        plan.name = "Frame borders";
        const int width = plan.tileWidth + 6, height = plan.sheetHeight + 32;
        std::vector<D3DCOLOR> pixels;
        std::string error;
        if (!RenderDstGuide({plan}, width, height, pixels, error)) return 400;
        // Construct expectations as four distinct edges per frame, independently
        // of the renderer's modulo-based implementation. Include clear gutters.
        const int checkedHeight = plan.atlasY + plan.sheetHeight + 1;
        std::vector<D3DCOLOR> expected((size_t)width * checkedHeight, 0);
        const D3DCOLOR red = D3DCOLOR_ARGB(255, 240, 30, 45);
        for (int row = 0; row < plan.rows; ++row) for (int col = 0; col < plan.columns; ++col) {
            const int x0 = plan.atlasX + col * plan.width, y0 = plan.atlasY + row * plan.height;
            const int x1 = x0 + plan.width - 1, y1 = y0 + plan.height - 1;
            for (int x = x0; x <= x1; ++x) expected[(size_t)y0 * width + x] = expected[(size_t)y1 * width + x] = red;
            for (int y = y0; y <= y1; ++y) expected[(size_t)y * width + x0] = expected[(size_t)y * width + x1] = red;
        }
        if (!std::equal(expected.begin(), expected.end(), pixels.begin())) return 401;
        std::ostringstream svg;
        WriteDstGuideFrameBorder(svg, 3, 4, plan.width, plan.height);
        const std::string text = svg.str();
        if (plan.width == 1 || plan.height == 1) {
            const std::string rectangle = "x=\"3\" y=\"4\" width=\"" + std::to_string(plan.width) +
                "\" height=\"" + std::to_string(plan.height) + "\" fill=\"#f01e2d\"";
            if (text.find(rectangle) == std::string::npos || text.find("stroke=") != std::string::npos) return 402;
        } else {
            const std::string rectangle = "x=\"3.5\" y=\"4.5\" width=\"" + std::to_string(plan.width - 1) +
                "\" height=\"" + std::to_string(plan.height - 1) + "\"";
            if (text.find(rectangle) == std::string::npos ||
                text.find("stroke-width=\"1\"") == std::string::npos) return 403;
        }
    }
    return 0;
}

static int TestAtlasCropDeletion(const std::filesystem::path& directory) {
    // The active gr count is 2, but the all-branch editor count is 4. This is
    // the important case: losing the legacy full-crop binding silently selects
    // a different texture after reparsing (consecutive IFs as used by tricoro).
    auto workspace = std::make_unique<WORKSPACE>();
    if (workspace->ResetEditorDocumentForLoad() != 0) return 380;
    const std::string root = (directory / "atlas-delete.lr2skin").string();
    strncpy_s(workspace->mainpath, root.c_str(), _TRUNCATE);
    const auto append = [&](const std::string& text) {
        auto* row = (SKINFILELINEREAD*)workspace->skinfileLines.Get_new();
        row->line.assign(text.c_str()); row->filename.assign(root.c_str());
        row->numTotal = workspace->skinfileLines.count - 1;
        row->isComment = text[0] != '#'; row->isSEcomment = text[0] == '$';
        SplitCSV(row->line, &row->csv, ","); row->csvColumnCount = CountCsvColumns(row->line);
    };
    append("$FILE '" + root + "' start");
    append("#INFORMATION,0,Atlas deletion,test");
    append("#IMAGE,missing-base.png");
    append("#IF,900"); append("#IMAGE,missing-left.png"); append("#ENDIF");
    append("#IF,901"); append("#IMAGE,missing-right-a.png");
    append("#IMAGE,missing-right-b.png"); append("#ENDIF");
    append("$FILE '" + root + "' end");
    workspace->g.skstruct.op[0] = workspace->g.skstruct.op[900] = 1;
    workspace->g.skstruct.op[901] = 0;
    if (workspace->RebuildEditorDerivedState() != 0 ||
        !workspace->objectEditorModel.LoadGroups(nullptr)) return 381;
    workspace->RebuildObjectModel(); workspace->loaded = true;
    std::vector<SEObjectSelectionKey> keys;
    std::vector<std::string> destinations;
    std::string path, error;
    for (int index = 0; index < 3; ++index) {
        if (!workspace->CreateImageObjectFromLayout(index * 20, 17, 8 + index, 12,
            ("Box " + std::to_string(index)).c_str(), path, error, -1, SELayoutImageOptions(), false) ||
            !path.empty() || workspace->RebuildEditorDerivedState() != 0) return 382;
        workspace->RebuildObjectModel(); keys.push_back(workspace->objectSelection.active);
        const auto object = workspace->objectEditorModel.Objects()[workspace->ResolveObjectSelectionKey(keys.back())];
        destinations.emplace_back(((SKINFILELINEREAD*)workspace->skinfileLines.data)[object.rows.back()].line.outstr());
    }
    std::vector<int> models;
    for (const auto& key : keys) models.push_back(workspace->ResolveObjectSelectionKey(key));
    if (!workspace->CreateAtlasFromDst(models, path, error) ||
        workspace->RebuildEditorDerivedState() != 0) return 383;
    workspace->RebuildObjectModel();
    const auto validateObjects = [&](WORKSPACE& ws) {
        if (ws.objectEditorModel.Objects().size() != 3) return false;
        for (int index = 0; index < 3; ++index) {
            const int model = ws.ResolveObjectSelectionKey(keys[index]);
            if (model < 0) return false;
            const auto& object = ws.objectEditorModel.Objects()[model];
            auto* rows = (SKINFILELINEREAD*)ws.skinfileLines.data;
            if (rows[object.rows.front()].csv.val[2] != 2 ||
                destinations[index] != rows[object.rows.back()].line.outstr()) return false;
            const int asset = ws.FindImageAssetForObject(model);
            if (asset < 0) return false;
            const int texture = ws.ResolveIMGTextureIndex(asset);
            if (texture < 0 || !((SRCGR*)ws.arr_SRCGR.data)[texture].path.isSame(path.c_str())) return false;
        }
        return true;
    };
    // New atlases expose only the three real crops, not an enclosing fourth.
    if (workspace->arr_IMG.count != 3 || !validateObjects(*workspace)) return 384;
    int bindingRow = -1;
    for (int row = 0; row < workspace->skinfileLines.count; ++row) {
        auto& line = ((SKINFILELINEREAD*)workspace->skinfileLines.data)[row];
        if (line.line.isSame("$SE_IMAGE_GR,2")) bindingRow = row;
    }
    if (bindingRow < 0) return 385;
    int width, height;
    if (!GetImageSizeFromFile(path.c_str(), &width, &height)) return 386;
    const std::string legacy = "$SRC_IMAGE,0,2,0,0," + std::to_string(width) + "," +
        std::to_string(height) + ",1,1,0,0,0,0,0";
    CSTR previous(((SKINFILELINEREAD*)workspace->skinfileLines.data)[bindingRow].line);
    if (workspace->EditLine(bindingRow, previous, CSTR(legacy.c_str())) != 0 ||
        workspace->RebuildEditorDerivedState() != 0) return 387;
    workspace->RebuildObjectModel();
    const int enclosing = workspace->FindIMG(2, 0, 0, width, height, 0);
    if (workspace->arr_IMG.count != 4 || enclosing >= workspace->arr_IMG.count ||
        !workspace->CanDeleteIMG(enclosing) || !validateObjects(*workspace)) return 388;
    const auto beforeDelete = workspace->CaptureDocumentSnapshot();
    const int history = workspace->arr_history.count;
    if (workspace->DeleteIMG(enclosing) != 0 || workspace->arr_history.count != history + 1 ||
        workspace->RebuildEditorDerivedState() != 0) return 389;
    workspace->RebuildObjectModel();
    if (workspace->arr_IMG.count != 3 || !validateObjects(*workspace)) return 390;
    const auto afterDelete = workspace->CaptureDocumentSnapshot();
    if (beforeDelete.lines.size() != afterDelete.lines.size()) return 391;
    for (int row = 0; row < (int)beforeDelete.lines.size(); ++row)
        if (row != bindingRow && (beforeDelete.lines[row].line != afterDelete.lines[row].line ||
            beforeDelete.lines[row].filename != afterDelete.lines[row].filename)) return 392;
    if (workspace->UndoLastEdit() != 0 || workspace->RebuildEditorDerivedState() != 0) return 393;
    workspace->RebuildObjectModel();
    if (workspace->arr_IMG.count != 4 || !validateObjects(*workspace)) return 394;
    if (workspace->RedoLastEdit() != 0 || workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        workspace->RebuildEditorDerivedState() != 0) return 395;
    workspace->RebuildObjectModel();
    if (workspace->arr_IMG.count != 3 || !validateObjects(*workspace) ||
        workspace->SaveSkinScript(workspace->mainpath, true, true) != 0) return 396;
    auto reopened = std::make_unique<WORKSPACE>();
    if (reopened->ResetEditorDocumentForLoad() != 0) return 397;
    strncpy_s(reopened->mainpath, root.c_str(), _TRUNCATE);
    if (reopened->LoadSkinScript(reopened->mainpath) < 0 || reopened->RebuildEditorDerivedState() != 0 ||
        !reopened->objectEditorModel.LoadGroups(nullptr)) return 398;
    reopened->RebuildObjectModel(); reopened->loaded = true;
    if (reopened->arr_IMG.count != 3 || !validateObjects(*reopened)) return 399;
    return 0;
}

static int TestExternalImageReload(const std::filesystem::path& directory) {
    const std::string path = (directory / "watched.png").string();
    const std::string staged = (directory / "staged.png").string();
    auto workspace = std::make_unique<WORKSPACE>();
    if (workspace->ResetEditorDocumentForLoad() != 0) return 410;
    struct ReleaseTextures { WORKSPACE& ws; ~ReleaseTextures() { ws.ResetEditorDocumentForLoad(); } } release{*workspace};
    char error[256] = {};
    FILETIME initialTime{}; GetSystemTimeAsFileTime(&initialTime);
    unsigned long long stamp = ((unsigned long long)initialTime.dwHighDateTime << 32) | initialTime.dwLowDateTime;
    const auto writeFile = [&](int width, D3DCOLOR color, bool valid = true) {
        if (valid) {
            if (!CreateSolidImageFileAtomic(staged.c_str(), width, 4, color, error, sizeof(error)) ||
                !MoveFileExA(staged.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) return false;
        } else {
            std::ofstream broken(path, std::ios::binary | std::ios::trunc);
            broken << "incomplete external PNG"; broken.close();
            if (!broken) return false;
        }
        HANDLE file = CreateFileA(path.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_EXISTING, 0, nullptr);
        if (file == INVALID_HANDLE_VALUE) return false;
        stamp += 10000000;
        FILETIME time{(DWORD)stamp, (DWORD)(stamp >> 32)};
        const bool ok = SetFileTime(file, nullptr, nullptr, &time) != 0;
        CloseHandle(file); return ok;
    };
    const D3DCOLOR red = D3DCOLOR_ARGB(255, 255, 0, 0), blue = D3DCOLOR_ARGB(255, 0, 0, 255);
    if (!writeFile(4, red)) return 411;
    // Relative components and filename case must resolve to the same image.
    const std::string alias = (directory / "." / "WATCHED.PNG").string();
    auto* first = (SRCGR*)workspace->arr_SRCGR.Get_new(); first->path.assign(path.c_str());
    auto* second = (SRCGR*)workspace->arr_SRCGR.Get_new(); second->path.assign(alias.c_str());
    if (!workspace->EnsureSRCGRTexture(0) || !workspace->EnsureSRCGRTexture(1) ||
        workspace->externalImageStates.size() != 1) return 412;
    const auto matches = [&](int width, D3DCOLOR expected) {
        for (int index = 0; index < 2; ++index) {
            const auto& image = ((SRCGR*)workspace->arr_SRCGR.data)[index]; D3DCOLOR pixel = 0;
            if (image.sizeX != width || image.sizeY != 4 || !ReadTexturePixel(image.texture, 0, 0, &pixel) || pixel != expected) return false;
        }
        return true;
    };
    // A caption fixture checks invalidation of the runtime cache and the common
    // safe rebuild request without starting/changing a real user's Scene.
    workspace->g.skstruct.count = 1;
    workspace->g.skstruct.GrHandle[0] = -1;
    workspace->g.skstruct.caption[0].assign(alias.c_str());
    workspace->objectSelection.active.editorId = "keep-selection";
    const auto revision = workspace->documentRevision;
    const int history = workspace->arr_history.count;
    workspace->PollExternalImageChanges(1000);
    if (!writeFile(6, blue)) return 413;
    workspace->PollExternalImageChanges(1500);
    workspace->PollExternalImageChanges(1750);
    if (!matches(4, red) || workspace->previewReloadPending) return 414;
    workspace->PollExternalImageChanges(2000);
    if (!matches(6, blue) || !workspace->previewReloadPending ||
        !workspace->g.skstruct.caption[0].isSame("") || workspace->documentRevision != revision ||
        workspace->arr_history.count != history || workspace->objectSelection.active.editorId != "keep-selection") return 415;
    workspace->previewReloadPending = false;
    // Unsaved paint blocks auto/manual reload, including differently spelled aliases.
    workspace->imagePixelPaintDirtyPaths[alias] = true;
    if (!writeFile(7, red)) return 416;
    workspace->PollExternalImageChanges(2500); workspace->PollExternalImageChanges(3000);
    if (!matches(6, blue) || workspace->ReloadImageFile(path.c_str()) ||
        !workspace->HasUnsavedImageEdits(path.c_str())) return 417;
    if (!workspace->ReloadImageFile(path.c_str(), true) || !matches(7, red) ||
        !workspace->imagePixelPaintDirtyPaths.empty()) return 418;
    // Partial writes and failed Revert preserve both the valid texture and edits.
    if (!writeFile(0, 0, false)) return 419;
    workspace->imagePixelPaintDirtyPaths[alias] = true;
    if (workspace->ReloadImageFile(path.c_str(), true) || !matches(7, red) ||
        workspace->imagePixelPaintDirtyPaths.empty()) return 420;
    workspace->imagePixelPaintDirtyPaths.clear();
    workspace->PollExternalImageChanges(3500); workspace->PollExternalImageChanges(4000);
    if (!matches(7, red) || workspace->previewReloadPending) return 421;
    if (!DeleteFileA(path.c_str())) return 422;
    workspace->PollExternalImageChanges(4500);
    if (!matches(7, red) || !writeFile(8, blue)) return 423;
    workspace->PollExternalImageChanges(5000); workspace->PollExternalImageChanges(5500);
    if (!matches(8, blue)) return 424;
    // No reload/release loop when the disk image has not changed.
    auto* texture = ((SRCGR*)workspace->arr_SRCGR.data)[0].texture;
    workspace->PollExternalImageChanges(6000);
    if (texture != ((SRCGR*)workspace->arr_SRCGR.data)[0].texture) return 425;
    // Saving one painted file must retain the other unsaved image and all
    // textures referenced by this frame. Alias refresh is deferred to draw().
    const std::string otherPath = (directory / "other-paint.png").string();
    if (!CreateSolidImageFileAtomic(otherPath.c_str(), 4, 4, red, error, sizeof(error))) return 426;
    auto* other = (SRCGR*)workspace->arr_SRCGR.Get_new();
    other->path.assign(otherPath.c_str());
    if (!workspace->EnsureSRCGRTexture(2)) return 427;
    auto* otherTexture = ((SRCGR*)workspace->arr_SRCGR.data)[2].texture;
    for (int i = 0; i < 3; ++i) {
        auto& image = ((SRCGR*)workspace->arr_SRCGR.data)[i];
        if (!PaintTextureLine(image.texture, 0, 0, 0, 0, red)) return 428;
    }
    workspace->imagePixelPaintDirtyPaths[path] = true;
    workspace->imagePixelPaintDirtyPaths[alias] = true;
    workspace->imagePixelPaintDirtyPaths[otherPath] = true;
    std::string saveError;
    HANDLE lock = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (lock == INVALID_HANDLE_VALUE) return 429;
    const bool lockedSave = workspace->SavePaintImage(path, saveError);
    CloseHandle(lock);
    if (lockedSave || !workspace->HasUnsavedImageEdits(path.c_str())) return 430;
    if (!workspace->SavePaintImage(path, saveError) || workspace->HasUnsavedImageEdits(path.c_str()) ||
        !workspace->HasUnsavedImageEdits(otherPath.c_str()) || workspace->editorDerivedRebuildPending ||
        texture != ((SRCGR*)workspace->arr_SRCGR.data)[0].texture ||
        otherTexture != ((SRCGR*)workspace->arr_SRCGR.data)[2].texture ||
        workspace->imageManagerReloadPathRequest != path || workspace->documentRevision != revision) return 431;
    if (!workspace->ReloadImageFile(path.c_str()) ||
        !workspace->HasUnsavedImageEdits(otherPath.c_str()) ||
        otherTexture != ((SRCGR*)workspace->arr_SRCGR.data)[2].texture) return 432;
    workspace->g.skstruct.count = 0;
    return 0;
}

int RunLayoutFirstObjectSelfTest() {
    namespace fs = std::filesystem;
    if (LoadCommandHelp(nullptr) != 0) return 1;
    const int borderResult = TestDstGuideFrameBorders();
    if (borderResult != 0) return borderResult;
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

    // Use a separate directory: the older layout-only test asserts its output
    // directory stays completely empty until explicit image generation.
    const fs::path deletionDirectory = directory / "deletion-test";
    fs::create_directory(deletionDirectory);
    const int deletionResult = TestAtlasCropDeletion(deletionDirectory);
    if (deletionResult != 0) return deletionResult;
    const int reloadResult = TestExternalImageReload(deletionDirectory);
    if (reloadResult != 0) return reloadResult;
    std::error_code deletionError;
    fs::remove_all(deletionDirectory, deletionError);

    auto workspace = std::make_unique<WORKSPACE>();
    if (workspace->imageAddAutoCrops || workspace->imageAddCropsAttempted ||
        workspace->imageAddCropsReady) return 120;
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

    // Layout-only Objects must not allocate PNGs or logical image slots. They
    // retain ordinary SRC/DST, stable IDs and include/IF ownership across Save.
    std::vector<SEObjectSelectionKey> layoutKeys;
    std::vector<std::string> layoutDestinations;
    for (int kind = 0; kind < 5; ++kind) {
        SELayoutImageOptions options;
        options.kind = kind;
        options.divX = kind == 1 ? 10 : 2;
        options.divY = kind == 1 ? 1 : 2;
        options.cycle = 600;
        options.digits = 4;
        options.align = 1;
        if (!workspace->CreateImageObjectFromLayout(21, 32, 8, 12,
            ("Layout " + std::to_string(kind)).c_str(), imagePath, error, 0, options, false) ||
            !imagePath.empty() || !fs::is_empty(directory) || !workspace->previewLayoutMode ||
            workspace->skinfileLines.count != originalRows + (kind + 1) * 4 ||
            workspace->arr_history.count != kind + 1 ||
            workspace->RebuildEditorDerivedState() != 0) return 340 + kind;
        workspace->RebuildObjectModel();
        layoutKeys.push_back(workspace->objectSelection.active);
        const int model = workspace->ResolveObjectSelectionKey(layoutKeys.back());
        if (!workspace->IsLayoutOnlyObject(model)) return 345 + kind;
        const auto object = workspace->objectEditorModel.Objects()[model];
        auto* lines = (SKINFILELINEREAD*)workspace->skinfileLines.data;
        auto& src = lines[object.rows.front()];
        auto& dst = lines[object.rows.back()];
        float x, y, w, h;
        if (object.ifgroup != anchorBranch || !src.filename.isSame(include.c_str()) ||
            !dst.filename.isSame(include.c_str()) || src.csv.val[7] != options.divX ||
            src.csv.val[8] != options.divY || src.csv.val[9] != 600 ||
            !workspace->GetObjectLayoutBounds(model, x, y, w, h) ||
            x != 21 || y != 32 || w != (kind == 1 ? 32 : 8) || h != 12) return 350 + kind;
        layoutDestinations.emplace_back(dst.line.outstr());
        for (int row = 0; row < workspace->skinfileLines.count; ++row)
            if (lines[row].csv.str[0].isSame("#IMAGE") ||
                lines[row].csv.str[0].isSame("$SRC_IMAGE")) return 355;
    }
    if (workspace->SaveSkinScript(workspace->mainpath, true, false) != 0) return 356;
    {
        auto layoutReopened = std::make_unique<WORKSPACE>();
        if (layoutReopened->ResetEditorDocumentForLoad() != 0) return 357;
        strncpy_s(layoutReopened->mainpath, root.c_str(), _TRUNCATE);
        if (layoutReopened->LoadSkinScript(layoutReopened->mainpath) < 0 ||
            layoutReopened->RebuildEditorDerivedState() != 0 ||
            !layoutReopened->objectEditorModel.LoadGroups(nullptr)) return 358;
        layoutReopened->RebuildObjectModel();
        layoutReopened->loaded = true;
        for (int kind = 0; kind < 5; ++kind) {
            const int model = layoutReopened->ResolveObjectSelectionKey(layoutKeys[kind]);
            if (!layoutReopened->IsLayoutOnlyObject(model)) return 359;
            const auto object = layoutReopened->objectEditorModel.Objects()[model];
            auto* lines = (SKINFILELINEREAD*)layoutReopened->skinfileLines.data;
            if (!lines[object.rows.front()].filename.isSame(include.c_str()) ||
                object.ifgroup != anchorBranch ||
                layoutDestinations[kind] != lines[object.rows.back()].line.outstr()) return 360;
        }
    }
    // One shared image is generated only after placement, without new Objects
    // or changes to their DST. Undo returns to the no-image layout state.
    std::vector<int> layoutModels;
    for (const auto& key : layoutKeys) layoutModels.push_back(workspace->ResolveObjectSelectionKey(key));
    std::vector<std::string> layoutPaths;
    if (!workspace->CreateImagesFromDst(layoutModels, false, layoutPaths, error) ||
        layoutPaths.size() != 1 || !fs::exists(layoutPaths[0]) ||
        workspace->skinfileLines.count != originalRows + 22 ||
        workspace->arr_history.count != 6 || workspace->RebuildEditorDerivedState() != 0) return 361;
    unsigned char bakedAlpha = 0;
    int bakedWidth = 0, bakedHeight = 0;
    const std::string bakedGuide = layoutPaths[0].substr(0, layoutPaths[0].size() - 4) + "_guide.png";
    if (!workspace->dstAssetPaintableGuide || fs::exists(bakedGuide) ||
        !ReadImageFilePixelAlpha(layoutPaths[0].c_str(), 0, 0, &bakedAlpha) || bakedAlpha != 255 ||
        !ReadImageFilePixelAlpha(layoutPaths[0].c_str(), 1, 1, &bakedAlpha) || bakedAlpha != 0 ||
        !GetImageSizeFromFile(layoutPaths[0].c_str(), &bakedWidth, &bakedHeight) ||
        workspace->UpdateImageManagerGuide(layoutPaths[0].c_str(), bakedWidth, bakedHeight)) return 426;
    workspace->RebuildObjectModel();
    for (int kind = 0; kind < 5; ++kind) {
        const int model = workspace->ResolveObjectSelectionKey(layoutKeys[kind]);
        if (model < 0 || workspace->IsLayoutOnlyObject(model)) return 362;
        const auto object = workspace->objectEditorModel.Objects()[model];
        auto* lines = (SKINFILELINEREAD*)workspace->skinfileLines.data;
        if (layoutDestinations[kind] != lines[object.rows.back()].line.outstr()) return 363;
    }
    if (workspace->UndoLastEdit() != 0 || workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        workspace->RebuildEditorDerivedState() != 0) return 364;
    workspace->RebuildObjectModel();
    for (const auto& key : layoutKeys)
        if (!workspace->IsLayoutOnlyObject(workspace->ResolveObjectSelectionKey(key))) return 365;
    if (workspace->RedoLastEdit() != 0 || workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        workspace->RebuildEditorDerivedState() != 0) return 366;
    for (int undo = 0; undo < 6; ++undo)
        if (workspace->UndoLastEdit() != 0 || workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
            workspace->RebuildEditorDerivedState() != 0) return 367;
    workspace->RebuildObjectModel();
    if (workspace->skinfileLines.count != originalRows || workspace->arr_history.count != 0) return 368;
    if (workspace->RedoLastEdit() != 0 || workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        workspace->RebuildEditorDerivedState() != 0) return 369;
    workspace->RebuildObjectModel();
    if (!workspace->IsLayoutOnlyObject(workspace->ResolveObjectSelectionKey(layoutKeys.front())) ||
        workspace->UndoLastEdit() != 0 || workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        workspace->RebuildEditorDerivedState() != 0) return 370;
    workspace->RebuildObjectModel();
    workspace->previewLayoutMode = false;

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
    int cropDeclaration = -1;
    for (int i = workspace->skinfileLines.count - 1; i >= 0; --i)
        if (((SKINFILELINEREAD*)workspace->skinfileLines.data)[i].csv.str[0].isSame("#IMAGE")) {
            cropDeclaration = i; break;
        }
    const int dedupRows = workspace->skinfileLines.count;
    const int dedupHistory = workspace->arr_history.count;
    if (cropDeclaration < 0 || workspace->RegisterImageWithTransparentCrops(
        cropDeclaration, imagePath.c_str(), 32, 16, detected, error) >= 0 ||
        workspace->skinfileLines.count != dedupRows ||
        workspace->arr_history.count != dedupHistory) return 221;
    // An existing full-size Asset must not prevent adding smaller regions.
    std::vector<TransparentAssetCrop> fullCrop = {{0, 0, 32, 16, true}};
    const bool addedFullCrop = workspace->RegisterImageWithTransparentCrops(
        cropDeclaration, imagePath.c_str(), 32, 16, fullCrop, error) >= 0;
    if (!addedFullCrop && error.find("No new crops selected") != 0) return 222;
    if (workspace->RebuildEditorDerivedState() != 0) return 225;
    std::vector<TransparentAssetCrop> mixedCrops = {{0, 0, 4, 4, true},
        {24, 8, 2, 2, true}, {24, 8, 2, 2, true}};
    if (workspace->RegisterImageWithTransparentCrops(cropDeclaration, imagePath.c_str(),
        32, 16, mixedCrops, error) < 0 ||
        workspace->skinfileLines.count != dedupRows + 1 + (addedFullCrop ? 1 : 0) ||
        workspace->RebuildEditorDerivedState() != 0) {
        printf("Auto asset mixed: %s; rows %d expected %d\n", error.c_str(),
            workspace->skinfileLines.count, dedupRows + 1 + (addedFullCrop ? 1 : 0));
        return 223;
    }
    for (int i = 0; i < 1 + (addedFullCrop ? 1 : 0); ++i)
        if (workspace->UndoLastEdit() != 0 || workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
            workspace->RebuildEditorDerivedState() != 0) return 224;
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
        const auto assetKey = workspace->MakeObjectSelectionKey(model);
        const std::string oldSource = lines[object.rows[0]].line.outstr();
        const std::string oldDst = lines[object.rows[1]].line.outstr();
        const int beforeAssetHistory = workspace->arr_history.count;
        const size_t beforeAssetObjects = workspace->objectEditorModel.Objects().size();
        int fw = 0, fh = 0, dx = 0, dy = 0;
        if (!workspace->GetDstAssetSize(model, fw, fh, dx, dy, error) || fw != 8 || fh != 9 ||
            dx != options.divX || dy != options.divY) return 230 + kind;
        std::string blankPath;
        int canvasWidth = 0, canvasHeight = 0;
        if (!workspace->GetDstAtlasSize({model}, canvasWidth, canvasHeight, error)) return 334;
        if (!workspace->CreateAssetFromDst(model, blankPath, error, false) ||
            workspace->arr_history.count != beforeAssetHistory + 1 ||
            !GetImageSizeFromFile(blankPath.c_str(), &width, &height) ||
            width != canvasWidth || height != canvasHeight ||
            !ReadImageFilePixelAlpha(blankPath.c_str(), 0, 0, &alpha) || alpha != 0 ||
            workspace->RebuildEditorDerivedState() != 0) return 240 + kind;
        const std::string guidePath = blankPath.substr(0, blankPath.size() - 4) + "_guide.png";
        if (!GetImageSizeFromFile(guidePath.c_str(), &width, &height) ||
            width != canvasWidth || height != canvasHeight ||
            !ReadImageFilePixelAlpha(guidePath.c_str(), 0, 0, &alpha) || alpha != 255 ||
            !ReadImageFilePixelAlpha(guidePath.c_str(), 1, 1, &alpha) || alpha != 0 ||
            !ReadImageFilePixelAlpha(guidePath.c_str(), 3, 3, &alpha) || alpha != 0) return 250 + kind;
        if (dx > 1 && (!ReadImageFilePixelAlpha(guidePath.c_str(), fw - 1, 3, &alpha) || alpha != 255 ||
            !ReadImageFilePixelAlpha(guidePath.c_str(), fw, 3, &alpha) || alpha != 255 ||
            !ReadImageFilePixelAlpha(guidePath.c_str(), fw + 1, 3, &alpha) || alpha != 0)) return 404;
        if (dy > 1 && (!ReadImageFilePixelAlpha(guidePath.c_str(), 3, fh - 1, &alpha) || alpha != 255 ||
            !ReadImageFilePixelAlpha(guidePath.c_str(), 3, fh, &alpha) || alpha != 255 ||
            !ReadImageFilePixelAlpha(guidePath.c_str(), 3, fh + 1, &alpha) || alpha != 0)) return 405;
        // Check the exact guide pixel buffer as well as encoded PNG alpha.
        std::vector<DstAssetPlan> guidePlans; std::vector<D3DCOLOR> guidePixels;
        if (!PlanDstAtlas(*workspace, {model}, guidePlans, width, height, error) ||
            !RenderDstGuide(guidePlans, width, height, guidePixels, error) ||
            guidePixels[0] != D3DCOLOR_ARGB(255, 240, 30, 45) ||
            !std::any_of(guidePixels.begin() + (size_t)(fh * dy) * width, guidePixels.end(),
                [](D3DCOLOR pixel) { return (pixel >> 24) != 0; })) return 335;
        // The editor must expose the sidecar without binding it as skin artwork.
        if (!workspace->UpdateImageManagerGuide(blankPath.c_str(), width, height) ||
            !workspace->imageManagerGuideTexture) return 336;
        auto* cachedGuide = workspace->imageManagerGuideTexture.get();
        D3DCOLOR guideColor = 0;
        if (!ReadTexturePixel(cachedGuide, 0, 0, &guideColor) || guideColor != D3DCOLOR_ARGB(255, 240, 30, 45) ||
            !workspace->UpdateImageManagerGuide(blankPath.c_str(), width, height) ||
            cachedGuide != workspace->imageManagerGuideTexture.get() ||
            !ReadImageFilePixelAlpha(blankPath.c_str(), 0, 0, &alpha) || alpha != 0) return 337;
        if (workspace->UpdateImageManagerGuide(blankPath.c_str(), width + 1, height, true) ||
            workspace->imageManagerGuideTexture || workspace->imageManagerGuideStatus.empty()) return 338;
        if (!workspace->UpdateImageManagerGuide(blankPath.c_str(), width, height, true) ||
            workspace->UpdateImageManagerGuide(imagePath.c_str(), width, height) ||
            workspace->imageManagerGuideTexture || !workspace->imageManagerGuideStatus.empty()) return 339;
        workspace->RebuildObjectModel();
        const int boundModel = workspace->ResolveObjectSelectionKey(assetKey);
        if (boundModel < 0 || workspace->objectEditorModel.Objects().size() != beforeAssetObjects) return 260 + kind;
        const auto bound = workspace->objectEditorModel.Objects()[boundModel];
        auto* boundLines = (SKINFILELINEREAD*)workspace->skinfileLines.data;
        auto& boundSrc = boundLines[bound.rows[0]].csv;
        if (oldDst != boundLines[bound.rows[1]].line.outstr() ||
            oldSource == boundLines[bound.rows[0]].line.outstr() ||
            boundSrc.val[7] != options.divX || boundSrc.val[8] != options.divY ||
            boundSrc.val[9] != options.cycle ||
            (kind == 1 && (boundSrc.val[12] != 1 || boundSrc.val[13] != 4))) return 270 + kind;
        if (workspace->UndoLastEdit() != 0 || workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
            workspace->RebuildEditorDerivedState() != 0 || !fs::exists(blankPath) || !fs::exists(guidePath)) return 280 + kind;
        workspace->RebuildObjectModel();
        const int restored = workspace->ResolveObjectSelectionKey(assetKey);
        if (restored < 0 || oldSource != ((SKINFILELINEREAD*)workspace->skinfileLines.data)
            [workspace->objectEditorModel.Objects()[restored].rows[0]].line.outstr()) return 290 + kind;
        if (workspace->UndoLastEdit() != 0 ||
            workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
            workspace->skinfileLines.count != rowCount || !fs::exists(imagePath) ||
            workspace->RebuildEditorDerivedState() != 0) return 42 + kind;
        workspace->RebuildObjectModel();
    }
    // Batch atlas: contiguous animation sheets, one gr, annotated guide and one Undo.
    const int batchStartRows = workspace->skinfileLines.count;
    SELayoutImageOptions aOptions; aOptions.divX = aOptions.divY = 2; aOptions.cycle = 400;
    if (!workspace->CreateImageObjectFromLayout(21, 32, 20, 12, "Atlas A <&>", imagePath, error, -1, aOptions) ||
        workspace->RebuildEditorDerivedState() != 0) return 310;
    workspace->RebuildObjectModel();
    const auto aKey = workspace->objectSelection.active;
    SELayoutImageOptions bOptions; bOptions.kind = 1; bOptions.divX = 10; bOptions.digits = 4;
    if (!workspace->CreateImageObjectFromLayout(100, 60, 6, 10, "Atlas B", imagePath, error, -1, bOptions) ||
        workspace->RebuildEditorDerivedState() != 0) return 311;
    workspace->RebuildObjectModel();
    const auto bKey = workspace->objectSelection.active;
    const int aModel = workspace->ResolveObjectSelectionKey(aKey), bModel = workspace->ResolveObjectSelectionKey(bKey);
    if (aModel < 0 || bModel < 0) return 323;
    float bx, by, bw, bh;
    if (!workspace->GetObjectLayoutBounds(bModel, bx, by, bw, bh) || bx != 100 || by != 60 || bw != 24 || bh != 10) return 312;
    workspace->previewLayoutMode = true;
    workspace->SetObjectSelection({aModel, bModel}, bModel, bModel, false);
    if (workspace->RefreshPreviewSelectionBounds() != 0 || workspace->preview_selected_obj.x != 21 ||
        workspace->preview_selected_obj.y != 32 || workspace->preview_selected_obj.w != 103 ||
        workspace->preview_selected_obj.h != 38 || workspace->preview_selected_obj_last_valid) return 313;
    workspace->previewLayoutMode = false;
    const auto aObject = workspace->objectEditorModel.Objects()[aModel];
    const auto bObject = workspace->objectEditorModel.Objects()[bModel];
    const std::string aDst = ((SKINFILELINEREAD*)workspace->skinfileLines.data)[aObject.rows.back()].line.outstr();
    const std::string bDst = ((SKINFILELINEREAD*)workspace->skinfileLines.data)[bObject.rows.back()].line.outstr();
    const int batchRows = workspace->skinfileLines.count, batchHistory = workspace->arr_history.count;
    std::string batchPath;
    if (workspace->GetDstAtlasSize({aModel, -1}, width, height, error) ||
        workspace->CreateAtlasFromDst({aModel, -1}, batchPath, error) ||
        workspace->skinfileLines.count != batchRows) return 314;
    if (!workspace->CreateAtlasFromDst({aModel, bModel, aModel}, batchPath, error) ||
        workspace->skinfileLines.count != batchRows + 2 || workspace->arr_history.count != batchHistory + 1 ||
        workspace->RebuildEditorDerivedState() != 0) return 315;
    workspace->RebuildObjectModel();
    const int packedAModel = workspace->ResolveObjectSelectionKey(aKey), packedBModel = workspace->ResolveObjectSelectionKey(bKey);
    if (packedAModel < 0 || packedBModel < 0) return 324;
    const auto packedA = workspace->objectEditorModel.Objects()[packedAModel];
    const auto packedB = workspace->objectEditorModel.Objects()[packedBModel];
    auto* packedLines = (SKINFILELINEREAD*)workspace->skinfileLines.data;
    auto& ac = packedLines[packedA.rows.front()].csv;
    auto& bc = packedLines[packedB.rows.front()].csv;
    if (ac.val[2] != bc.val[2] || ac.val[5] != 40 || ac.val[6] != 24 ||
        bc.val[5] != 60 || bc.val[6] != 10 || ac.val[7] != 2 || ac.val[8] != 2 || ac.val[9] != 400 ||
        bc.val[7] != 10 || bc.val[13] != 4 ||
        aDst != packedLines[packedA.rows.back()].line.outstr() || bDst != packedLines[packedB.rows.back()].line.outstr()) return 316;
    if (!(ac.val[3] + ac.val[5] <= bc.val[3] || bc.val[3] + bc.val[5] <= ac.val[3] ||
        ac.val[4] + ac.val[6] <= bc.val[4] || bc.val[4] + bc.val[6] <= ac.val[4])) return 317;
    const std::string svgPath = batchPath.substr(0, batchPath.size() - 4) + "_guide.svg";
    std::ifstream svgInput(svgPath, std::ios::binary);
    const std::string svgText((std::istreambuf_iterator<char>(svgInput)), std::istreambuf_iterator<char>());
    if (svgText.find("Atlas A &lt;&amp;&gt;") == std::string::npos || svgText.find("Frame 6 x 10") == std::string::npos ||
        svgText.find("#181a1e") != std::string::npos || svgText.find("#f01e2d") == std::string::npos ||
        svgText.find("stroke-width=\"2\"") != std::string::npos ||
        svgText.find("stroke-opacity=") != std::string::npos ||
        svgText.find("shape-rendering=\"crispEdges\"") == std::string::npos ||
        svgText.find("fill=\"none\"") == std::string::npos ||
        svgText.find(":9</text>") == std::string::npos || svgText.find("Frames 0..3") == std::string::npos) return 318;
    if (workspace->UndoLastEdit() != 0 || workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        workspace->skinfileLines.count != batchRows || workspace->RebuildEditorDerivedState() != 0 ||
        !fs::exists(batchPath) || !fs::exists(svgPath)) return 319;
    if (workspace->RedoLastEdit() != 0 || workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        workspace->skinfileLines.count != batchRows + 2 || workspace->RebuildEditorDerivedState() != 0) return 320;
    if (workspace->UndoLastEdit() != 0 || workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        workspace->RebuildEditorDerivedState() != 0) return 325;
    workspace->RebuildObjectModel();
    const int separateA = workspace->ResolveObjectSelectionKey(aKey), separateB = workspace->ResolveObjectSelectionKey(bKey);
    if (separateA < 0 || separateB < 0) return 326;
    workspace->SetObjectSelection({separateA, separateB}, separateB, separateB, false);
    workspace->RequestDstAssetDialog();
    if (!workspace->dstAssetRequested || workspace->dstAtlasTargets.size() != 2 || workspace->dstAssetSeparate ||
        workspace->dstAssetRevision != workspace->documentRevision) return 327;
    workspace->dstAssetRequested = false;
    const int separateHistory = workspace->arr_history.count;
    std::vector<std::string> separatePaths;
    if (workspace->CreateImagesFromDst({separateA, -1}, true, separatePaths, error) || !separatePaths.empty() ||
        workspace->skinfileLines.count != batchRows) return 328;
    if (!workspace->CreateImagesFromDst({separateA, separateB, separateA}, true, separatePaths, error, false) ||
        separatePaths.size() != 2 || separatePaths[0] == separatePaths[1] ||
        workspace->skinfileLines.count != batchRows + 4 || workspace->arr_history.count != separateHistory + 1 ||
        workspace->objectSelection.selected.size() != 2 || workspace->RebuildEditorDerivedState() != 0) return 329;
    for (const auto& path : separatePaths)
        if (!ReadImageFilePixelAlpha(path.c_str(), 0, 0, &alpha) || alpha != 0 ||
            !fs::exists(path.substr(0, path.size() - 4) + "_guide.png")) return 427;
    workspace->RebuildObjectModel();
    const int imageA = workspace->ResolveObjectSelectionKey(aKey), imageB = workspace->ResolveObjectSelectionKey(bKey);
    if (imageA < 0 || imageB < 0) return 330;
    auto* separateLines = (SKINFILELINEREAD*)workspace->skinfileLines.data;
    const auto sa = workspace->objectEditorModel.Objects()[imageA];
    const auto sb = workspace->objectEditorModel.Objects()[imageB];
    if (separateLines[sa.rows.front()].csv.val[2] == separateLines[sb.rows.front()].csv.val[2] ||
        aDst != separateLines[sa.rows.back()].line.outstr() || bDst != separateLines[sb.rows.back()].line.outstr() ||
        !GetImageSizeFromFile(separatePaths[0].c_str(), &width, &height) || width < 40 || height != 24 + 28 ||
        !GetImageSizeFromFile(separatePaths[1].c_str(), &width, &height) || width < 60 || height != 10 + 28) return 331;
    if (workspace->UndoLastEdit() != 0 || workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        workspace->skinfileLines.count != batchRows || workspace->RebuildEditorDerivedState() != 0 ||
        !fs::exists(separatePaths[0]) || !fs::exists(separatePaths[1])) return 332;
    if (workspace->RedoLastEdit() != 0 || workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
        workspace->skinfileLines.count != batchRows + 4 || workspace->RebuildEditorDerivedState() != 0) return 333;
    for (int undo = 0; undo < 3; ++undo)
        if (workspace->UndoLastEdit() != 0 || workspace->ApplyPendingHistorySnapshotRestore() != 0 ||
            workspace->RebuildEditorDerivedState() != 0) return 321;
    if (workspace->skinfileLines.count != batchStartRows) return 322;
    workspace->RebuildObjectModel();
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
    if (!reopened->LoadCodeEditorFile(splitOwner)) return 87;
    const auto codeBefore = reopened->CaptureDocumentSnapshot();
    const int codeHistory = reopened->arr_history.count;
    const auto setDraft = [&](const std::string& value) {
        memcpy(reopened->codeEditorBuffer.data(), value.c_str(), value.size() + 1);
    };
    setDraft(reopened->codeEditorBase + "// multiline draft\n// second line\n");
    if (!reopened->ApplyCodeEditorDraft() || reopened->arr_history.count != codeHistory + 1 ||
        runtimeLines(reopened->CaptureDocumentSnapshot()) != runtimeLines(codeBefore)) return 88;
    if (reopened->UndoLastEdit() != 0 || reopened->ApplyPendingHistorySnapshotRestore() != 0 ||
        reopened->CaptureDocumentSnapshot().lines.size() != codeBefore.lines.size()) return 89;
    if (!reopened->LoadCodeEditorFile(splitOwner)) return 90;
    setDraft(reopened->codeEditorBase + "#IF,900\n");
    if (reopened->ApplyCodeEditorDraft()) return 91;
    setDraft(reopened->codeEditorBase + "// \xF0\x9F\x98\x80\n");
    if (reopened->ApplyCodeEditorDraft()) return 92;
    setDraft(reopened->codeEditorBase + "// conflict\n");
    ++reopened->documentRevision;
    if (reopened->ApplyCodeEditorDraft()) return 93;
    if (!reopened->LoadCodeEditorFile(root)) return 94;
    const auto rootBefore = reopened->CaptureDocumentSnapshot();
    setDraft(reopened->codeEditorBase + "// parent edit\n");
    if (!reopened->ApplyCodeEditorDraft() ||
        runtimeLines(reopened->CaptureDocumentSnapshot()) != runtimeLines(rootBefore)) return 95;
    auto alteredInclude = reopened->codeEditorBase;
    const auto includePos = alteredInclude.find("#INCLUDE,parts.csv");
    if (includePos == std::string::npos) return 96;
    alteredInclude.replace(includePos, strlen("#INCLUDE,parts.csv"), "#INCLUDE,missing.csv");
    setDraft(alteredInclude);
    if (reopened->ApplyCodeEditorDraft()) return 97;
    // Tree navigation must not overwrite an unapplied Text Editor draft.
    const std::string keptDraft = reopened->codeEditorBuffer.data();
    if (reopened->OpenScriptInCodeEditor(splitOwner) ||
        keptDraft != reopened->codeEditorBuffer.data()) return 98;
    if (!reopened->LoadCodeEditorFile(root) ||
        !reopened->OpenScriptInCodeEditor(splitOwner)) return 99;
    const auto addTreeOwner = [&](const fs::path& path) {
        auto* line = (SKINFILELINEREAD*)reopened->skinfileLines.Get_new();
        line->filename.assign(path.string().c_str());
    };
    addTreeOwner(directory / "left" / "notes.csv");
    addTreeOwner(directory / "right" / "notes.csv");
    addTreeOwner(directory / "left" / "notes.csv");
    reopened->RebuildScriptDirectoryTree();
    int notes = 0, left = 0, right = 0, mainFiles = 0;
    for (const auto& node : reopened->scriptDirectoryTree) {
        if (node.label == "notes.csv" && !node.owner.empty()) ++notes;
        if (node.label == "left" && node.owner.empty() && node.children.size() == 1) ++left;
        if (node.label == "right" && node.owner.empty() && node.children.size() == 1) ++right;
        if (node.owner == root) ++mainFiles;
    }
    if (notes != 2 || left != 1 || right != 1 || mainFiles != 1 ||
        reopened->scriptDirectoryRevision != reopened->documentRevision) return 100;
    auto* custom = (SKINFILELINEREAD*)reopened->skinfileLines.Get_new();
    custom->filename.assign(root.c_str());
    custom->line.assign("#CUSTOMFILE,Notes,notes/*.png,white");
    SplitCSV(custom->line, &custom->csv, ",");
    custom->csvColumnCount = 4;
    reopened->customFileDraftRow = reopened->skinfileLines.count - 1;
    reopened->customFileDraftDocument = reopened->mainpath;
    reopened->customFileDraftRevision = reopened->documentRevision;
    reopened->customFileDraftOriginal = custom->line.body;
    strcpy_s(reopened->customFileTitle, "Notes");
    strcpy_s(reopened->customFilePattern, "notes/*.png");
    strcpy_s(reopened->customFileDefault, "blue");
    const int customHistory = reopened->arr_history.count;
    if (!reopened->ApplyCustomFileDraft() || reopened->arr_history.count != customHistory + 1 ||
        !custom->csv.str[3].isSame("blue") || !custom->filename.isSame(root.c_str())) return 101;
    if (reopened->ApplyCustomFileDraft()) return 102; // stale revision rejected
    reopened->customFileDraftRevision = reopened->documentRevision;
    reopened->customFileDraftOriginal = custom->line.body;
    strcpy_s(reopened->customFileTitle, "bad,title");
    if (reopened->ApplyCustomFileDraft()) return 103;
    strcpy_s(reopened->customFileTitle, "\xF0\x9F\x98\x80");
    if (reopened->ApplyCustomFileDraft()) return 104;
    if (reopened->UndoLastEdit() != 0 || !custom->csv.str[3].isSame("white")) return 105;
    const auto visibilityRevision = reopened->documentRevision;
    const int visibilityHistory = reopened->arr_history.count;
    reopened->SetObjectBrowserFile(splitOwner);
    reopened->SetPreviewFileVisible(root, false);
    reopened->SetPreviewFileVisible(root, false);
    if (reopened->previewHiddenFiles.size() != 1 || reopened->IsPreviewFileVisible(root.c_str()) ||
        !reopened->IsPreviewFileVisible(splitOwner.c_str()) || reopened->objectBrowserFile != splitOwner) return 106;
    reopened->SetPreviewFileVisible(splitOwner, false);
    reopened->previewSelectedFileOnly = true;
    if (reopened->IsPreviewFileVisible(splitOwner.c_str())) return 107;
    reopened->SetPreviewFileVisible(splitOwner, true);
    if (!reopened->IsPreviewFileVisible(splitOwner.c_str()) || reopened->IsPreviewFileVisible(root.c_str())) return 108;
    reopened->previewSelectedFileOnly = false;
    std::string rootCase = root;
    for (char& c : rootCase) if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
    if (!reopened->IsPreviewFileHidden(rootCase.c_str())) return 109;
    reopened->SetPreviewFileVisible(rootCase, true);
    if (reopened->IsPreviewFileHidden(root.c_str())) return 110;
    reopened->SetPreviewFileVisible(root, false);
    if (reopened->IsPreviewRowVisible(reopened->customFileDraftRow)) return 111;
    reopened->ShowAllPreviewFiles();
    if (!reopened->IsPreviewRowVisible(reopened->customFileDraftRow) || reopened->previewSelectedFileOnly ||
        !reopened->previewHiddenFiles.empty() || reopened->documentRevision != visibilityRevision ||
        reopened->arr_history.count != visibilityHistory) return 112;
    auto tree = std::make_unique<WORKSPACE>();
    strcpy_s(tree->mainpath, "C:\\tree-test\\LR2files\\Theme\\Demo\\main.lr2skin");
    tree->skinfileLines.Alloc(sizeof(SKINFILELINEREAD), 4);
    for (const char* path : { "C:\\tree-test\\LR2files\\Theme\\Demo\\main.lr2skin",
        "C:\\tree-test\\lr2files\\Sound\\shared.csv", "C:\\external\\other.csv",
        "C:\\tree-test\\LR2files\\Theme\\Demo\\csv\\notes.csv" }) {
        auto* source = (SKINFILELINEREAD*)tree->skinfileLines.Get_new();
        source->filename.assign(path);
    }
    tree->RebuildScriptDirectoryTree();
    int lr2Root = -1, externalRoot = -1;
    for (int child : tree->scriptDirectoryTree[0].children) {
        if (tree->scriptDirectoryTree[child].label == "LR2files") lr2Root = child;
        if (tree->scriptDirectoryTree[child].label == "External files") externalRoot = child;
    }
    if (lr2Root < 0 || externalRoot < 0 || tree->scriptDirectoryTree[lr2Root].children.size() != 2) return 113;
    bool themeFound = false, soundFound = false;
    for (int child : tree->scriptDirectoryTree[lr2Root].children) {
        themeFound |= tree->scriptDirectoryTree[child].label == "Theme";
        soundFound |= tree->scriptDirectoryTree[child].label == "Sound";
    }
    if (!themeFound || !soundFound) return 114;
    bool mainFirst = false;
    for (const auto& node : tree->scriptDirectoryTree) {
        if (node.label != "Demo" || node.children.size() != 2) continue;
        mainFirst = tree->scriptDirectoryTree[node.children[0]].owner == tree->mainpath &&
            tree->scriptDirectoryTree[node.children[1]].label == "csv";
    }
    if (!mainFirst) return 115;
    reopened->wObjectInspector = false;
    reopened->objectInspectorRevealRequested = false;
    reopened->SetObjectSelection({0}, 0, 0, false);
    if (!reopened->wObjectInspector || !reopened->objectInspectorRevealRequested) return 116;
    reopened->objectInspectorRevealRequested = false;
    reopened->RestoreObjectSelection();
    if (reopened->objectInspectorRevealRequested) return 117;
    reopened->codeEditorRevealRequested = false;
    if (!reopened->OpenScriptInCodeEditor(splitOwner) || !reopened->codeEditorRevealRequested) return 118;

    // Existing LR2 skins need not contain $SE_OBJECT_ID. Splitting two such
    // Objects must retain both choices, the active Object and the range anchor.
    auto plain = std::make_unique<WORKSPACE>();
    if (plain->ResetEditorDocumentForLoad() != 0) return 119;
    const std::string plainRoot = (directory / "plain.lr2skin").string();
    const std::string plainSplit = (directory / "plain-split.csv").string();
    strncpy_s(plain->mainpath, plainRoot.c_str(), _TRUNCATE);
    SkinDocumentSnapshot plainDocument;
    for (const auto& text : std::vector<std::string>{
        "$FILE '" + plainRoot + "' start",
        "#INFORMATION,0,Plain split test,test",
        "#SRC_IMAGE,0,0,0,0,1,1,1,1,0,0,0,0,0",
        "#DST_IMAGE,0,0,11,0,1,1,0,255,255,255,255,0,0,0,0,0,0,0,0,0",
        "#SRC_IMAGE,0,0,0,0,1,1,1,1,0,0,0,0,0",
        "#DST_IMAGE,0,0,22,0,1,1,0,255,255,255,255,0,0,0,0,0,0,0,0,0",
        "$FILE '" + plainRoot + "' end" }) {
        SkinLineSnapshot line;
        line.filename = plainRoot;
        line.line = text;
        plainDocument.lines.push_back(line);
    }
    if (plain->RestoreDocumentSnapshot(plainDocument) != 0 ||
        plain->RebuildEditorDerivedState() != 0 ||
        !plain->objectEditorModel.LoadGroups(nullptr)) return 120;
    plain->RebuildObjectModel();
    plain->loaded = true;
    if (plain->objectEditorModel.Objects().size() != 2) return 121;
    plain->SetObjectSelection({0, 1}, 1, 0, false);
    const auto plainBefore = plain->CaptureDocumentSnapshot();
    const int plainHistory = plain->arr_history.count;
    const auto plainSelectionMatches = [&](int offset, const std::string& owner) {
        const auto& selection = plain->objectSelection;
        if (selection.selected.size() != 2 ||
            plain->preview_selected_object_model_indices != std::vector<int>{0, 1}) return false;
        const auto matches = [&](const SEObjectSelectionKey& key, int expectedIndex) {
            const int expectedRow = plainBefore.selection.selected[expectedIndex].anchorRow + offset;
            if (!key.editorId.empty() || key.anchorRow != expectedRow ||
                plain->ResolveObjectSelectionKey(key) != expectedIndex) return false;
            const auto& object = plain->objectEditorModel.Objects()[expectedIndex];
            if (!object.editorId.empty() || object.rows.size() != 2 ||
                object.rows.front() != expectedRow) return false;
            const auto* lines = (const SKINFILELINEREAD*)plain->skinfileLines.data;
            return !strcmp(lines[object.rows.front()].filename.body, owner.c_str()) &&
                !strcmp(lines[object.rows.back()].filename.body, owner.c_str()) &&
                lines[object.rows.back()].csv.val[3] == (expectedIndex + 1) * 11;
        };
        return matches(selection.selected[0], 0) && matches(selection.selected[1], 1) &&
            matches(selection.active, 1) && matches(selection.anchor, 0) &&
            plain->preview_selected_object_model_index == 1 &&
            plain->preview_selection_anchor_model_index == 0;
    };
    if (!plainSelectionMatches(0, plainRoot)) return 122;
    if (!plain->SplitSelectedObjects("plain-split.csv", error) ||
        plain->arr_history.count != plainHistory + 1 ||
        !plainSelectionMatches(2, plainSplit)) return 123;
    if (plain->UndoLastEdit() != 0 || plain->ApplyPendingHistorySnapshotRestore() != 0 ||
        plain->skinfileLines.count != (int)plainBefore.lines.size() ||
        plain->RebuildEditorDerivedState() != 0) return 124;
    plain->RebuildObjectModel();
    if (!plainSelectionMatches(0, plainRoot)) return 124;
    if (plain->RedoLastEdit() != 0 || plain->ApplyPendingHistorySnapshotRestore() != 0 ||
        plain->skinfileLines.count != (int)plainBefore.lines.size() + 3 ||
        plain->RebuildEditorDerivedState() != 0) return 125;
    plain->RebuildObjectModel();
    if (!plainSelectionMatches(2, plainSplit)) return 125;
    return 0;
}
