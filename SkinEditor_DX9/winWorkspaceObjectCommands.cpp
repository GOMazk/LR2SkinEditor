#include "winWorkspace.h"

#include "winWorkspaceUiHelpers.h"

#include <algorithm>
#include <cstdio>
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

bool WORKSPACE::HasCopiedObjects() const {
    return !processObjectClipboard.objects.empty();
}
