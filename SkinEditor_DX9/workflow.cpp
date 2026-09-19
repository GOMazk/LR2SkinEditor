#include "winWorkspace.h"
#include "seHelper.h"
#include "inputwrap.h"
#include "seLocalization.h"
#include "seUI.h"
#include "uiCatalog.h"
#include "winWorkspaceUiHelpers.h"

namespace {
void ContinueRowIfFits(const char* label) {
    const float right = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;
    if (ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x +
        ImGui::CalcTextSize(label).x + 2 * ImGui::GetStyle().FramePadding.x <= right)
        ImGui::SameLine();
}
}

std::vector<SEPendingWork> WORKSPACE::PendingWork() const {
    std::vector<SEPendingWork> items;
    if (IsDocumentDirty()) items.push_back({SEPendingKind::Script, mainpath});
    if (!codeEditorBuffer.empty() && codeEditorBase != codeEditorBuffer.data())
        items.push_back({SEPendingKind::TextDraft, codeEditorOwner});
    if (customFileDraftDirty)
        items.push_back({SEPendingKind::CustomFilesDraft, customFileDraftDocument});
    for (const auto& image : imagePixelPaintDirtyPaths)
        items.push_back({SEPendingKind::Image, image.first});
    if (imageFontEditor.Dirty())
        items.push_back({SEPendingKind::Font, imageFontEditor.document.path});
    return items;
}

bool WORKSPACE::SavePendingWork(const SEPendingWork& item, std::string& error) {
    error.clear();
    // Never silently apply text/custom-file drafts as a side effect of Save.
    if (item.IsDraft()) {
        error = "Review and Apply the draft in its editor, then save the skin.";
        return false;
    }
    if (item.kind == SEPendingKind::Script) {
        const bool ok = SaveCurrentSkin() == 0;
        error = lastSaveMessage;
        return ok;
    }
    if (item.kind == SEPendingKind::Font) {
        const bool ok = SaveImageFont();
        error = imageFontEditor.status;
        return ok;
    }
    return SavePaintImage(item.path, error);
}

void WORKSPACE::ReviewPendingWork(const SEPendingWork& item) {
    alive = true;
    switch (item.kind) {
    case SEPendingKind::Script:
        wObjectInspector = true; objectInspectorRevealRequested = true; break;
    case SEPendingKind::TextDraft:
        wCodeEditor = true; codeEditorRevealRequested = true; break;
    case SEPendingKind::CustomFilesDraft:
        wCustomFiles = true; customFilesRevealRequested = true; break;
    case SEPendingKind::Font:
        wImageFontEditor = true; imageFontRevealRequested = true; break;
    case SEPendingKind::Image:
        wImgManager = true; imageManagerRevealRequested = true;
        for (int i = 0; i < arr_SRCGR.count; ++i) {
            auto& image = ((SRCGR*)arr_SRCGR.data)[i];
            if (!image.path.body || _stricmp(image.path.body, item.path.c_str())) continue;
            int target = -1;
            for (int asset = 0; asset < arr_IMG.count; ++asset) {
                const auto& crop = ((IMG*)arr_IMG.data)[asset];
                if (crop.gr != image.grID) continue;
                if (target < 0) target = asset;
                if (crop.ifGroup == image.isIf) { target = asset; break; }
            }
            if (target >= 0) SelectIMGAsset(target, true);
            gr_selected = i; grID_selected = image.grID;
            imageManagerManualTexturePath = image.path.body;
            imageManagerManualTextureGr = image.grID;
            imagePixelPaintMode = true;
            break;
        }
        break;
    }
}

bool SECanExitWorkspaces(const std::vector<std::unique_ptr<WORKSPACE>>& workspaces) {
    for (const auto& workspace : workspaces)
        if (!workspace->PendingWork().empty() || workspace->pendingHistorySnapshotRestore >= 0)
            return false; // Hidden workspaces and external-only drafts also count.
    return true;
}

bool SEDrawPendingChanges(std::vector<std::unique_ptr<WORKSPACE>>& workspaces, bool& exitRequested) {
    static std::string status;
    static bool discardConfirmation = false;
    bool requested = false;
    for (auto& workspace : workspaces) {
        requested |= workspace->pendingChangesRequested;
        workspace->pendingChangesRequested = false;
    }
    char title[128];
    FormatSEUISurfaceTitle(title, sizeof(title), SEUISurfaceId::PendingChanges);
    if (exitRequested && SECanExitWorkspaces(workspaces)) return true;
    if (requested || (exitRequested && !ImGui::IsPopupOpen(title))) {
        status.clear(); discardConfirmation = false;
        ImGui::OpenPopup(title);
    }
    const auto* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Appearing, ImVec2(.5f, .5f));
    ImGui::SetNextWindowSize(ImVec2((std::min)(760.0f, viewport->WorkSize.x - 32),
        (std::min)(510.0f, viewport->WorkSize.y - 32)), ImGuiCond_Appearing);
    if (!ImGui::BeginPopupModal(title, nullptr)) return false;
    ImGui::TextWrapped("%s", exitRequested
        ? SEText("Save your work before exiting. Hidden workspaces are included.", u8"\uC885\uB8CC \uC804 \uC791\uC5C5\uC744 \uC800\uC7A5\uD558\uC138\uC694. \uC228\uACA8\uC9C4 \uC791\uC5C5\uACF5\uAC04\uB3C4 \uD3EC\uD568\uB429\uB2C8\uB2E4.")
        : SEText("Each file saves independently. Text drafts must be reviewed and applied first.", u8"\uAC01 \uD30C\uC77C\uC740 \uAC1C\uBCC4 \uC800\uC7A5\uB429\uB2C8\uB2E4. \uD14D\uC2A4\uD2B8 \uCD08\uC548\uC740 \uBA3C\uC800 \uAC80\uD1A0\uD558\uACE0 \uC801\uC6A9\uD558\uC138\uC694."));
    ImGui::Separator();
    bool review = false;
    ImGui::BeginChild("PendingFiles", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 4));
    int total = 0;
    for (size_t index = 0; index < workspaces.size(); ++index) {
        auto& workspace = *workspaces[index];
        const auto items = workspace.PendingWork();
        if (items.empty()) continue;
        total += (int)items.size();
        ImGui::PushID((int)index);
        ImGui::SeparatorText(workspace.title);
        for (size_t row = 0; row < items.size(); ++row) {
            const auto& item = items[row];
            ImGui::PushID((int)row);
            const char* kind = item.kind == SEPendingKind::Script ? "Skin CSV (+ includes)" :
                item.kind == SEPendingKind::TextDraft ? "Text Editor - unapplied draft" :
                item.kind == SEPendingKind::CustomFilesDraft ? "Custom Files - unapplied draft" :
                item.kind == SEPendingKind::Font ? "Image font" : "Painted image";
            ImGui::TextColored(item.IsDraft() ? SEUI::Colors::Warning() : SEUI::Colors::Accent(), "%s", kind);
            ImGui::TextWrapped("%s", Cp932ToUtf8(item.path.c_str()).c_str());
            if (ImGui::Button(SEText("Review", u8"\uAC80\uD1A0"))) {
                workspace.ReviewPendingWork(item);
                review = true;
            }
            if (!item.IsDraft()) {
                ImGui::SameLine();
                if (ImGui::Button(SEText("Save file", u8"\uD30C\uC77C \uC800\uC7A5"))) {
                    std::string message;
                    const bool saved = workspace.SavePendingWork(item, message);
                    status = std::string(saved ? "Saved: " : "Not saved: ") + message;
                }
            }
            ImGui::Separator();
            ImGui::PopID();
        }
        ImGui::PopID();
    }
    if (!total) ImGui::TextUnformatted(SEText("No pending changes.", u8"\uBBF8\uC800\uC7A5 \uBCC0\uACBD\uC774 \uC5C6\uC2B5\uB2C8\uB2E4."));
    ImGui::EndChild();
    ImGui::BeginChild("SaveDetails", ImVec2(0, ImGui::GetFrameHeightWithSpacing() * 2));
    if (!status.empty()) {
        ImGui::TextWrapped("%s", status.c_str());
    }
    ImGui::EndChild();
    bool exitNow = false;
    if (ImGui::Button(SEText("Keep working", u8"\uACC4\uC18D \uC791\uC5C5")) ||
        ImGui::IsKeyPressed(ImGuiKey_Escape) || review) {
        exitRequested = false; discardConfirmation = false;
        ImGui::CloseCurrentPopup();
    } else if (exitRequested) {
        if (!discardConfirmation) {
            ContinueRowIfFits(SEText("Exit without saving...", u8"\uC800\uC7A5\uD558\uC9C0 \uC54A\uACE0 \uC885\uB8CC\u2026"));
            if (ImGui::Button(SEText("Exit without saving...", u8"\uC800\uC7A5\uD558\uC9C0 \uC54A\uACE0 \uC885\uB8CC\u2026"))) discardConfirmation = true;
        } else {
            ContinueRowIfFits(SEText("Confirm discard and exit", u8"\uBCC0\uACBD\uC744 \uBC84\uB9AC\uACE0 \uC885\uB8CC \uD655\uC778"));
            if (ImGui::Button(SEText("Confirm discard and exit", u8"\uBCC0\uACBD\uC744 \uBC84\uB9AC\uACE0 \uC885\uB8CC \uD655\uC778"))) exitNow = true;
            ContinueRowIfFits(SEText("Back", u8"\uB4A4\uB85C"));
            if (ImGui::Button(SEText("Back", u8"\uB4A4\uB85C"))) discardConfirmation = false;
            ImGui::TextColored(SEUI::Colors::Warning(), "%s",
                SEText("All remaining unsaved changes will be lost. Saved files stay unchanged.", u8"\uB0A8\uC740 \uBBF8\uC800\uC7A5 \uBCC0\uACBD\uC740 \uC0AC\uB77C\uC9D1\uB2C8\uB2E4. \uC774\uBBF8 \uC800\uC7A5\uB41C \uD30C\uC77C\uC740 \uC720\uC9C0\uB429\uB2C8\uB2E4."));
        }
    }
    ImGui::EndPopup();
    return exitNow;
}

void WORKSPACE::RequestPreview() {
    wPreview = true; previewRevealRequested = true;
}

void WORKSPACE::RequestImageImport() {
    if (!loaded) return;
    wAssetBrowser = wImgManager = true;
    imageManagerRevealRequested = imageImportRequested = true;
}

bool WORKSPACE::OpenObjectImage(int modelIndex) {
    const int asset = FindImageAssetForObject(modelIndex);
    if (asset < 0 || asset >= arr_IMG.count || !SelectIMGAsset(asset, true)) {
        workflowStatus = "No image crop is available for this Object."; return false;
    }
    wImgManager = true; imageManagerRevealRequested = true;
    workflowStatus.clear(); return true;
}

bool WORKSPACE::OpenObjectFont(int modelIndex) {
    const auto& objects = objectEditorModel.Objects();
    if (modelIndex < 0 || modelIndex >= (int)objects.size()) return false;
    for (int row : objects[modelIndex].rows) {
        if (row < 0 || row >= skinfileLines.count) continue;
        auto& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
        const char* command = line.csv.str[0].body;
        if (!command || (strcmp(command, "#SRC_TEXT") && strcmp(command, "#SRC_BAR_TITLE"))) continue;
        const int column = FindCommandFieldColumn(command, "font");
        const int slot = column >= 0 ? line.csv.val[column] : -1;
        if (slot < 0 || slot >= (int)imageFontRuntimePaths.size() || imageFontRuntimePaths[slot].empty()) break;
        wImageFontEditor = true; imageFontRevealRequested = true;
        // Reuse the font editor's existing Save/discard/cancel guard.
        if (imageFontEditor.document.path != imageFontRuntimePaths[slot]) {
            imageFontEditor.pendingPath = imageFontRuntimePaths[slot];
            imageFontEditor.pendingOpen = true;
        }
        workflowStatus.clear(); return true;
    }
    workflowStatus = "This font slot is not loaded in the current Preview. Check its active IF/Customize state.";
    return false;
}

bool WORKSPACE::OpenObjectAppearance(int modelIndex) {
    const auto& objects = objectEditorModel.Objects();
    if (modelIndex < 0 || modelIndex >= (int)objects.size()) return false;
    for (const auto& slot : GetSimpleModeSlots()) {
        if (std::find(objects[modelIndex].rows.begin(), objects[modelIndex].rows.end(), slot.row) == objects[modelIndex].rows.end()) continue;
        simpleModeCategory = (int)slot.category;
        simpleModeSelectedSlotId = slot.id;
        simpleModeCandidateAsset = slot.imageIndex;
        simpleModeShowSelection = false;
        wSimpleMode = simpleModeRevealRequested = true;
        workflowStatus.clear(); return true;
    }
    workflowStatus = "This Object has no semantic appearance tools. Use its source image or Advanced LR2 fields.";
    return false;
}

void WORKSPACE::drawObjectWorkflowActions(int modelIndex) {
    const auto& objects = objectEditorModel.Objects();
    if (modelIndex < 0 || modelIndex >= (int)objects.size()) return;
    bool font = false;
    for (int row : objects[modelIndex].rows) {
        if (row < 0 || row >= skinfileLines.count) continue;
        auto& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
        font |= line.csv.str[0].body &&
            (line.csv.str[0].isSame("#SRC_TEXT") || line.csv.str[0].isSame("#SRC_BAR_TITLE"));
    }
    if (IsLayoutOnlyObject(modelIndex)) {
        if (ImGui::SmallButton(SEText("Create artwork...", u8"\uADF8\uB9BC \uB9CC\uB4E4\uAE30\u2026"))) RequestDstAssetDialog();
    } else if (font) {
        if (ImGui::SmallButton(SEText("Edit text font", u8"\uBB38\uC790 \uD3F0\uD2B8 \uD3B8\uC9D1"))) OpenObjectFont(modelIndex);
    } else {
        const int asset = FindImageAssetForObject(modelIndex);
        ImGui::BeginDisabled(asset < 0 || asset >= arr_IMG.count);
        if (ImGui::SmallButton(SEText("Source image", u8"\uC6D0\uBCF8 \uC774\uBBF8\uC9C0"))) OpenObjectImage(modelIndex);
        ImGui::EndDisabled();
        bool appearance = false;
        for (const auto& slot : GetSimpleModeSlots()) {
            if (std::find(objects[modelIndex].rows.begin(), objects[modelIndex].rows.end(), slot.row) != objects[modelIndex].rows.end()) {
                appearance = true; break;
            }
        }
        if (appearance) {
            ContinueRowIfFits(SEText("Appearance...", u8"\uBAA8\uC591 \uBC14\uAFB8\uAE30\u2026"));
            if (ImGui::SmallButton(SEText("Appearance...", u8"\uBAA8\uC591 \uBC14\uAFB8\uAE30\u2026"))) OpenObjectAppearance(modelIndex);
        }
    }
    ContinueRowIfFits(SEText("Edit CSV", u8"CSV \uD3B8\uC9D1"));
    if (ImGui::SmallButton(SEText("Edit CSV", u8"CSV \uD3B8\uC9D1")) && !objects[modelIndex].rows.empty()) {
        const int row = objects[modelIndex].rows.front();
        auto& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
        if (!OpenScriptInCodeEditor(line.filename.outstr())) workflowStatus = scriptDirectoryStatus;
        else workflowStatus.clear();
    }
    if (!workflowStatus.empty()) ImGui::TextWrapped("%s", workflowStatus.c_str());
}
