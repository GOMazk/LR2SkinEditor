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

int CountCsvColumns(CSTR& line) {
    if (!line.body || line.length() <= 0) return 0;
    int count = 1;
    const char* text = line.outstr();
    for (int i = 0; text[i] != '\0'; ++i) {
        if (text[i] == ',') ++count;
    }
    return count > 30 ? 30 : count;
}

int RunAssetMetadataSelfTest() {
    if (arr_CommandHelp.count <= 0 &&
        LoadCommandHelp("..\\skinHelper.txt") != 0) return 9;
    char tempDirectory[MAX_PATH] = {};
    if (!GetTempPathA(MAX_PATH, tempDirectory)) return 10;
    char outputPath[MAX_PATH] = {};
    snprintf(outputPath, sizeof(outputPath), "%sSkinEditor_asset_%lu.lr2skin",
        tempDirectory, GetCurrentProcessId());
    remove(outputPath);

    WORKSPACE workspace;
    workspace.skinfileLines.Alloc(sizeof(SKINFILELINEREAD), 8);
    workspace.arr_IMG.Alloc(sizeof(IMG), 8);
    workspace.arr_SRC.Alloc(sizeof(SRC), 4);
    workspace.arr_seobj.Alloc(sizeof(SEOBJ), 8);
    workspace.arr_ifunit.Alloc(sizeof(IFUNIT), 4);
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

    if (workspace.ParseSkinConditions() != 0) return 11;
    const int assetIfgroup =
        ((SKINFILELINEREAD*)workspace.skinfileLines.data)[1].ifgroup;
    if (assetIfgroup <= 0 ||
        ((SKINFILELINEREAD*)workspace.skinfileLines.data)[2].ifgroup != assetIfgroup)
        return 12;
    if (workspace.ParseSkinLegacyObjectsAndAssets() != 0 ||
        workspace.arr_IMG.count != 2) return 11;
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
        strstr(workspace.newObjectName.outstr(), "Fast_P1") == NULL) return 40;
    workspace.nCsv.str[11].assign("214");
    workspace.SynchronizeNewObjectAutoName("#SRC_NUMBER", false);
    if (!workspace.newObjectName.body ||
        strstr(workspace.newObjectName.outstr(), "Slow_P1") == NULL) return 41;
    workspace.newObjectName.assign("My counter");
    workspace.newObjectNameManuallyEdited = true;
    workspace.SynchronizeNewObjectAutoName("#SRC_NUMBER", false);
    if (!workspace.newObjectName.isSame("My counter")) return 42;

    const int manualIndex = workspace.NewIMG(17, 1, 2, 3, 4, 23);
    if (manualIndex != 2) return 14;
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

    // Generated images must be appended after the widest IF branch. Inserting
    // them beside the current texture would renumber every following gr ID.
    std::unique_ptr<WORKSPACE> generatedWorkspace(new WORKSPACE());
    generatedWorkspace->skinfileLines.Alloc(sizeof(SKINFILELINEREAD), 12);
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
    appendGeneratedLine("#IMAGE,left.png");
    appendGeneratedLine("#ELSE");
    appendGeneratedLine("#IMAGE,right_a.png");
    appendGeneratedLine("#IMAGE,right_b.png");
    appendGeneratedLine("#ENDIF");
    appendGeneratedLine(fileEnd.c_str());
    char generatedImagePath[MAX_PATH] = {};
    snprintf(generatedImagePath, sizeof(generatedImagePath),
        "%sSkinEditor_generated_%lu.png", tempDirectory, GetCurrentProcessId());
    std::string registrationError;
    if (generatedWorkspace->RegisterGeneratedImage(generatedImagePath, 64, 32,
        registrationError) != 3) return 33;
    if (generatedWorkspace->skinfileLines.count != 11) return 34;
    SKINFILELINEREAD& generatedImage =
        ((SKINFILELINEREAD*)generatedWorkspace->skinfileLines.data)[8];
    SKINFILELINEREAD& generatedAsset =
        ((SKINFILELINEREAD*)generatedWorkspace->skinfileLines.data)[9];
    SKINFILELINEREAD& generatedEnd =
        ((SKINFILELINEREAD*)generatedWorkspace->skinfileLines.data)[10];
    if (!generatedImage.csv.str[0].isSame("#IMAGE") ||
        !generatedAsset.line.body ||
        strstr(generatedAsset.line.outstr(), "$SRC_IMAGE,0,3,0,0,64,32") !=
            generatedAsset.line.outstr() ||
        !generatedEnd.line.body || strstr(generatedEnd.line.outstr(), "$FILE ") !=
            generatedEnd.line.outstr()) return 35;

    remove(outputPath);
    remove((std::string(outputPath) + ".skineditor.tmp").c_str());
    remove((std::string(outputPath) + ".skineditor.bak").c_str());
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
            for (int imageIndex = 0; imageIndex < arr_IMG.count; ++imageIndex) {
                IMG& image = ((IMG*)arr_IMG.data)[imageIndex];
                if (image.editorDeclare > metadataRow) --image.editorDeclare;
            }
        }
    }
    arr_IMG.DeleteAt(pos);

    //TODO:history here

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

    return 0;
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
    }
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
    objectEditorLastLineCount = skinfileLines.count;
    objectModelRebuildPending = false;
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
    if ((int)snapshot.lines.size() != skinfileLines.count) return -1;

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
    NotifyDocumentChanged(DOCUMENT_CHANGE_STRUCTURE);
    return 0;
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
    if (objectBranch(objects[sourceModelIndex]) !=
        objectBranch(objects[targetModelIndex])) return false;

    auto objectOwner = [&](const SEObjectInstance& object) -> const char* {
        if (object.rows.empty()) return "";
        const int row = object.rows.front();
        if (row < 0 || row >= skinfileLines.count) return "";
        const SKINFILELINEREAD& line =
            ((const SKINFILELINEREAD*)skinfileLines.data)[row];
        return line.filename.body ? line.filename.body : mainpath;
    };
    const char* sourceOwner = objectOwner(objects[sourceModelIndex]);
    const char* targetOwner = objectOwner(objects[targetModelIndex]);
    if (_stricmp(sourceOwner, targetOwner) != 0) return false;

    for (int row : objects[sourceModelIndex].rows) {
        if (row < 0 || row >= skinfileLines.count) return false;
        const SKINFILELINEREAD& line =
            ((const SKINFILELINEREAD*)skinfileLines.data)[row];
        const char* owner = line.filename.body ? line.filename.body : mainpath;
        if (_stricmp(owner, sourceOwner) != 0 ||
            line.ifgroup != objectBranch(objects[sourceModelIndex])) return false;
    }
    for (int row : objects[targetModelIndex].rows) {
        if (row < 0 || row >= skinfileLines.count) return false;
        const SKINFILELINEREAD& line =
            ((const SKINFILELINEREAD*)skinfileLines.data)[row];
        const char* owner = line.filename.body ? line.filename.body : mainpath;
        if (_stricmp(owner, targetOwner) != 0 ||
            line.ifgroup != objectBranch(objects[targetModelIndex])) return false;
    }
    return true;
}

bool WORKSPACE::QueueObjectReorder(int sourceModelIndex, int targetModelIndex,
    bool placeAfter) {
    if (!CanReorderObject(sourceModelIndex, targetModelIndex)) return false;
    pendingObjectReorderSource = MakeObjectSelectionKey(sourceModelIndex);
    pendingObjectReorderTarget = MakeObjectSelectionKey(targetModelIndex);
    if (!pendingObjectReorderSource.IsValid() ||
        !pendingObjectReorderTarget.IsValid()) return false;
    pendingObjectReorderAfter = placeAfter;
    pendingObjectReorder = true;
    return true;
}

int WORKSPACE::ApplyPendingObjectReorder() {
    if (!pendingObjectReorder) return 0;
    pendingObjectReorder = false;

    const int sourceModelIndex =
        ResolveObjectSelectionKey(pendingObjectReorderSource);
    const int targetModelIndex =
        ResolveObjectSelectionKey(pendingObjectReorderTarget);
    if (!CanReorderObject(sourceModelIndex, targetModelIndex)) return -1;

    const std::vector<SEObjectInstance>& objects = objectEditorModel.Objects();
    const SEObjectInstance& sourceObject = objects[sourceModelIndex];
    const SEObjectInstance& targetObject = objects[targetModelIndex];
    if (sourceObject.rows.empty() || targetObject.rows.empty()) return -1;

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
    if (sourceRows.empty() || targetRows.empty()) return -1;

    std::vector<bool> moved(skinfileLines.count, false);
    for (int row : sourceRows) {
        if (row < 0 || row >= skinfileLines.count) return -1;
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
    if (!changed) return 0;

    SkinDocumentSnapshot before = CaptureDocumentSnapshot();
    SkinDocumentSnapshot reordered;
    reordered.lines.reserve(before.lines.size());
    std::vector<int> oldToNew(order.size(), -1);
    for (int newRow = 0; newRow < (int)order.size(); ++newRow) {
        reordered.lines.push_back(before.lines[order[newRow]]);
        oldToNew[order[newRow]] = newRow;
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
    return RestoreDocumentSnapshot(reordered);
}

void WORKSPACE::NotifyDocumentChanged(unsigned int changes) {
    ++documentRevision;
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

int WORKSPACE::EditValue(int pos, int column, const char* newVal) {

    if (pos < 0 || pos >= skinfileLines.count || column < 0 || column >= 30 || newVal == NULL) return -1;

    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[pos];
    CSTR oldLine(line.line);

    line.csv.str[column].assign(newVal);
    line.csv.val[column] = atol(newVal);
    if (line.csvColumnCount < column + 1) line.csvColumnCount = column + 1;
    line.modified = true;
    CsvToLine(pos);

    if (!applyingHistory) {
        HISTORY* hs = (HISTORY*)arr_history.Get_new();
        hs->op = overwriteLine;
        hs->target = pos;
        hs->older.line.assign(oldLine);
        hs->newer.line.assign(line.line);
    }

    NotifyDocumentChanged(column == 0
        ? DOCUMENT_CHANGE_STRUCTURE : DOCUMENT_CHANGE_VALUE);

    return 0;
}
int WORKSPACE::EditValue(int pos, int column, int newVal) {

    if (pos < 0 || pos >= skinfileLines.count || column < 0 || column >= 30) return -1;

    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[pos];
    CSTR oldLine(line.line);
    line.csv.str[column].resize(12);
    ltoa(newVal, line.csv.str[column], 10);
    line.csv.val[column] = newVal;
    if (line.csvColumnCount < column + 1) line.csvColumnCount = column + 1;
    line.modified = true;
    CsvToLine(pos);

    if (!applyingHistory) {
        HISTORY* hs = (HISTORY*)arr_history.Get_new();
        hs->op = overwriteLine;
        hs->target = pos;
        hs->older.line.assign(oldLine);
        hs->newer.line.assign(line.line);
    }

    NotifyDocumentChanged(column == 0
        ? DOCUMENT_CHANGE_STRUCTURE : DOCUMENT_CHANGE_VALUE);

    return 0;
}

int WORKSPACE::UndoLastEdit() {
    if (arr_history.count <= 0) return -1;

    HISTORY& history = ((HISTORY*)arr_history.data)[arr_history.count - 1];
    const HISTORYOP operation = history.op;
    const int target = history.target;
    CSTR oldLine(history.older.line);
    --arr_history.count;

    applyingHistory = true;
    int result = 0;
    if (operation == overwriteLine) {
        if (target < 0 || target >= skinfileLines.count) result = -1;
        else {
            CSTR currentLine(((SKINFILELINEREAD*)skinfileLines.data)[target].line);
            result = EditLine(target, currentLine, oldLine);
        }
    } else if (operation == insertLine) {
        result = DeleteLine(target);
    } else if (operation == removeLine) {
        result = InsertLine(target);
        if (result == 0) {
            CSTR insertedLine(((SKINFILELINEREAD*)skinfileLines.data)[target].line);
            result = EditLine(target, insertedLine, oldLine);
        }
    } else if (operation == moveLine) {
        if (target >= 0 && target < (int)historyDocumentSnapshots.size()) {
            // Undo can be requested after Preview/ImageManager have already
            // submitted texture commands this frame. Restore at the next
            // frame boundary so the derived texture arrays can be rebuilt
            // before any window draws.
            pendingHistorySnapshotRestore = target;
            result = 0;
        } else {
            result = -1;
        }
    } else {
        result = -1;
    }
    applyingHistory = false;

    if (result == 0 && operation != moveLine) RestoreObjectSelection();
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
