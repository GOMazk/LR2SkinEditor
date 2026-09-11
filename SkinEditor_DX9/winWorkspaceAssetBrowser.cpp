#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "../LR2/structure.h"
#include "../LR2/LR2_skinmanage.h"
#include "../LR2/LR2_skindraw.h"
#include "../LR2/LR2_skinobject.h"
#include "../LR2/En_timer.h"
#include "../LR2/En_value.h"
#include "../lib/DxLib/DxLib.h"

#include "winWorkspace.h"
#include "skin.h"
#include "op.h"
#include "seHelper.h"
#include "seUI.h"
#include "skinPathResolver.h"
#include "inputwrap.h"
#include "uiCatalog.h"
#include "winWorkspaceUiHelpers.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>

extern PDIRECT3DTEXTURE9 transBackground;

void WORKSPACE::drawLayoutFirstImageDialog() {
    const SEUISurfaceSpec& spec = SEUISurfaceSpecFor(SEUISurfaceId::LayoutFirstImage);
    char popup[128];
    snprintf(popup, sizeof(popup), "%s##%s-%d", spec.title, spec.key, num);
    if (layoutFirstDialogPending) {
        layoutFirstDialogPending = false;
        layoutFirstPlacement = layoutFirstDragging = false;
        if (!layoutFirstResume) {
            layoutFirstOptions = SELayoutImageOptions();
            layoutFirstName[0] = '\0';
            layoutFirstPosition[0] = layoutFirstPosition[1] = 0;
            layoutFirstSize[0] = layoutFirstSize[1] = 64;
            layoutFirstUseSelection = false;
            layoutFirstOpenPaint = true;
            layoutFirstAnchor = objectSelection.active;
            layoutFirstError.clear();
        }
        layoutFirstResume = false;
        ImGui::OpenPopup(popup);
    }
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(),
        ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(460.0f, 0.0f), ImGuiCond_Appearing);
    if (!ImGui::BeginPopupModal(popup, nullptr,
        ImGuiWindowFlags_AlwaysAutoResize)) return;
    ImGui::InputText("Name", layoutFirstName, sizeof(layoutFirstName));
    if (ImGui::Combo("Type", &layoutFirstOptions.kind,
        "IMAGE\0NUMBER\0SLIDER\0BUTTON\0BARGRAPH\0")) {
        const int kind = layoutFirstOptions.kind;
        layoutFirstOptions = SELayoutImageOptions();
        layoutFirstOptions.kind = kind;
        if (kind == 1) layoutFirstOptions.divX = 10;
        if (kind == 3) layoutFirstOptions.divX = 2;
    }
    ImGui::InputInt2("Position (X, Y)", layoutFirstPosition);
    ImGui::InputInt2("Size (W, H)", layoutFirstSize);
    if (layoutFirstOptions.kind == 1) {
        ImGui::TextWrapped("W/H is one digit. Paint 0 through 9 from left to right.");
        ImGui::InputInt("Digits (keta)", &layoutFirstOptions.digits);
        ImGui::Combo("Align", &layoutFirstOptions.align, "Right\0Left\0Center\0");
    } else {
        ImGui::InputInt("Columns (div_x)", &layoutFirstOptions.divX);
        ImGui::InputInt("Rows (div_y)", &layoutFirstOptions.divY);
        ImGui::InputInt("Cycle (ms)", &layoutFirstOptions.cycle);
    }
    if (layoutFirstOptions.kind != 0) {
        const std::string command = std::string("#SRC_") + SELayoutImageType(layoutFirstOptions.kind);
        int selected = layoutFirstOptions.value;
        DrawCommandValueCombo("Value", command.c_str(),
            layoutFirstOptions.kind == 1 ? "$num" : "$type", selected, layoutFirstOptions.value);
    }
    if (layoutFirstOptions.kind == 2 || layoutFirstOptions.kind == 4) {
        ImGui::Combo("Direction", &layoutFirstOptions.direction,
            "0\0" "1\0" "2\0" "3\0");
        if (layoutFirstOptions.kind == 2) ImGui::InputInt("Range", &layoutFirstOptions.range);
    }
    int sheetWidth = 0, sheetHeight = 0;
    const bool validSize = SELayoutImageSize(layoutFirstSize[0], layoutFirstSize[1],
        layoutFirstOptions, sheetWidth, sheetHeight);
    ImGui::TextDisabled("Transparent PNG: %d x %d pixels", sheetWidth, sheetHeight);
    if (ImGui::Button("Draw rectangle in Preview")) {
        layoutFirstPlacement = true;
        layoutFirstDragging = false;
        wPreview = true;
        preview_object_dragging = preview_object_resizing = false;
        char previewTitle[96];
        FormatSEUIWindowTitle(previewTitle, sizeof(previewTitle), SEUIWindowId::Preview, num);
        ImGui::CloseCurrentPopup();
        ImGui::SetWindowFocus(previewTitle);
    }
    const int anchor = ResolveObjectSelectionKey(layoutFirstAnchor);
    ImGui::BeginDisabled(anchor < 0);
    ImGui::Checkbox("Use selected Object's file / IF branch", &layoutFirstUseSelection);
    ImGui::EndDisabled();
    if (layoutFirstUseSelection && anchor >= 0) {
        const SEObjectInstance& object = objectEditorModel.Objects()[anchor];
        ImGui::TextWrapped("After: %s", Cp932ToUtf8(object.name.c_str()).c_str());
    } else if (!layoutFirstUseSelection) {
        ImGui::TextDisabled("Destination: main skin / ALWAYS");
    }
    ImGui::Checkbox("Open Pixel Paint after creation", &layoutFirstOpenPaint);
    if (!validSize)
        ImGui::TextWrapped("Use a positive size, up to 16384 per side and 16 megapixels total.");
    if (layoutFirstUseSelection && anchor < 0)
        ImGui::TextWrapped("The target Object is unavailable. Reopen this dialog to choose a target.");
    if (!layoutFirstError.empty())
        ImGui::TextWrapped("%s", layoutFirstError.c_str());
    ImGui::Separator();
    ImGui::BeginDisabled(!loaded || !validSize ||
        (layoutFirstUseSelection && anchor < 0) || pendingHistorySnapshotRestore >= 0);
    if (ImGui::Button("Create", ImVec2(100.0f, 0.0f))) {
        const std::string name = Utf8ToCp932(layoutFirstName);
        std::string path;
        if (Cp932ToUtf8(name.c_str()) != layoutFirstName) {
            layoutFirstError = "The name contains characters unavailable in Shift-JIS.";
        } else if (CreateImageObjectFromLayout(layoutFirstPosition[0],
            layoutFirstPosition[1], layoutFirstSize[0], layoutFirstSize[1],
            name.c_str(), path, layoutFirstError, layoutFirstUseSelection ? anchor : -1,
            layoutFirstOptions)) {
            assetShowUnusedOnly = false;
            wPreview = wObjectBrowser = wObjectInspector = true;
            char focusTitle[96];
            if (layoutFirstOpenPaint) {
                wImgManager = true;
                imageManagerRevealRequested = true;
                imagePixelPaintMode = true;
                imagePixelPaintLastX = imagePixelPaintLastY = imagePixelPaintLastButton = -1;
            }
            FormatSEUIWindowTitle(focusTitle, sizeof(focusTitle),
                layoutFirstOpenPaint ? SEUIWindowId::ImageManager : SEUIWindowId::Preview, num);
            ImGui::CloseCurrentPopup();
            ImGui::SetWindowFocus(focusTitle);
        }
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(100.0f, 0.0f)) ||
        ImGui::IsKeyPressed(ImGuiKey_Escape)) ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
}

int WORKSPACE::drawAssetBrowser() {
    char title[64];
    FormatSEUIWindowTitle(title, sizeof(title), SEUIWindowId::AssetBrowser, num);
    if (!ImGui::Begin(title, &wAssetBrowser)) {
        ImGui::End();
        return 0;
    }

    auto drawBackgroundMenu = [&](bool emptySurface = false) {
        // Empty-state children are presentation only, so their whole area
        // counts as blank space. Real cards keep their own item context menu.
        if (emptySurface &&
            ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows) &&
            ImGui::IsMouseReleased(ImGuiMouseButton_Right))
            ImGui::OpenPopup("##AssetBackground");
        const bool open = emptySurface ? ImGui::BeginPopup("##AssetBackground") :
            ImGui::BeginPopupContextWindow("##AssetBackground",
                ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems);
        if (open) {
            if (ImGui::MenuItem("New blank image Object...", nullptr, false, loaded))
                layoutFirstDialogPending = true;
            ImGui::EndPopup();
        }
    };

    if (arr_IMG.count <= 0) {
        SEUI::EmptyState("No image crops",
            "Image crops parsed from #SRC commands will appear here as reusable assets.");
        drawBackgroundMenu(true);
        ImGui::End();
        return 0;
    }

    const std::vector<std::vector<int>>& assetUsage = ImageAssetUsage();
    const std::vector<SEObjectInstance>& usageObjects =
        objectEditorModel.Objects();
    auto selectUsageObject = [&](int modelIndex) {
        if (modelIndex < 0 || modelIndex >= (int)usageObjects.size()) return;
        wObjectBrowser = true;
        wObjectInspector = true;
        SetObjectSelection(std::vector<int>(1, modelIndex), modelIndex,
            modelIndex, true);
        preview_object_dragging = false;
        preview_selected_obj_valid = false;
        preview_selected_obj_last_valid = false;
    };
    const int activeObjectIndex =
        ResolveObjectSelectionKey(objectSelection.active);
    std::vector<int> activeSourceRows;
    CollectImageAssignableSourceRows(activeObjectIndex, activeSourceRows);
    const bool hasAssignableObject = activeObjectIndex >= 0 &&
        !activeSourceRows.empty();
    auto requestAssetApply = [&](int imageIndex) {
        const int modelIndex = ResolveObjectSelectionKey(objectSelection.active);
        std::vector<int> sourceRows;
        CollectImageAssignableSourceRows(modelIndex, sourceRows);
        if (imageIndex < 0 || imageIndex >= arr_IMG.count ||
            modelIndex < 0 || sourceRows.empty()) return false;
        if (sourceRows.size() == 1) {
            return ApplyImageAssetToObjectSource(imageIndex, modelIndex,
                sourceRows.front(), assetApplyCopyAnimation);
        }
        assetApplyAssetIndex = imageIndex;
        assetApplyObject = MakeObjectSelectionKey(modelIndex);
        assetApplyDialogRequested = true;
        return true;
    };
    auto requestAssetDelete = [&](int imageIndex) {
        std::string reason;
        if (!CanDeleteIMG(imageIndex, &reason)) {
            assetDeleteStatus = reason;
            return false;
        }
        assetDeleteAssetIndex = imageIndex;
        assetDeleteDialogRequested = true;
        assetDeleteStatus.clear();
        return true;
    };
    char applyAssetPopup[96];
    snprintf(applyAssetPopup, sizeof(applyAssetPopup),
        "Apply Asset to Object##%d", num);
    auto drawAssetApplyDialog = [&]() {
        if (assetApplyDialogRequested) {
            assetApplyDialogRequested = false;
            ImGui::OpenPopup(applyAssetPopup);
        }
        ImGui::SetNextWindowSize(ImVec2(500.0f, 0.0f), ImGuiCond_Appearing);
        if (!ImGui::BeginPopupModal(applyAssetPopup, NULL,
            ImGuiWindowFlags_AlwaysAutoResize)) return;

        const int modelIndex = ResolveObjectSelectionKey(assetApplyObject);
        std::vector<int> sourceRows;
        CollectImageAssignableSourceRows(modelIndex, sourceRows);
        const bool validAsset = assetApplyAssetIndex >= 0 &&
            assetApplyAssetIndex < arr_IMG.count;
        const bool validObject = modelIndex >= 0 && !sourceRows.empty();
        if (!validAsset || !validObject) {
            ImGui::TextWrapped("The selected Asset or Object is no longer available.");
            if (ImGui::Button("Close")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return;
        }

        IMG& asset = ((IMG*)arr_IMG.data)[assetApplyAssetIndex];
        const SEObjectInstance& object = usageObjects[modelIndex];
        const std::string objectName = Cp932ToUtf8(
            object.name.empty() ? "(unnamed)" : object.name.c_str());
        ImGui::Text("Asset %03d  gr %d", assetApplyAssetIndex, asset.gr);
        ImGui::Text("Object %03d  %s", modelIndex, objectName.c_str());
        ImGui::Checkbox("Copy SRC animation fields", &assetApplyCopyAnimation);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Also replace div_x, div_y, cycle and timer. Object-specific fields remain unchanged.");
        ImGui::Separator();
        ImGui::TextDisabled("Choose the SRC row to replace:");
        for (int sourceRow : sourceRows) {
            SKINFILELINEREAD& line =
                ((SKINFILELINEREAD*)skinfileLines.data)[sourceRow];
            const char* command = line.csv.str[0].body
                ? line.csv.str[0].outstr() : "#SRC";
            int columns[5];
            if (!ResolveImageCropColumns(command, columns)) continue;
            char rowLabel[320];
            snprintf(rowLabel, sizeof(rowLabel),
                "Line %d  %s   gr %d  (%d, %d, %d, %d)",
                sourceRow + 1, command, line.csv.val[columns[0]],
                line.csv.val[columns[1]], line.csv.val[columns[2]],
                line.csv.val[columns[3]], line.csv.val[columns[4]]);
            ImGui::PushID(sourceRow);
            if (ImGui::Selectable(rowLabel)) {
                ApplyImageAssetToObjectSource(assetApplyAssetIndex,
                    modelIndex, sourceRow, assetApplyCopyAnimation);
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopID();
        }
        ImGui::Separator();
        if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    };
    char deleteAssetPopup[96];
    snprintf(deleteAssetPopup, sizeof(deleteAssetPopup),
        "Delete Asset##%d", num);
    auto drawAssetDeleteDialog = [&]() {
        bool deleted = false;
        if (assetDeleteDialogRequested) {
            assetDeleteDialogRequested = false;
            ImGui::OpenPopup(deleteAssetPopup);
        }
        ImGui::SetNextWindowSize(ImVec2(480.0f, 0.0f), ImGuiCond_Appearing);
        if (!ImGui::BeginPopupModal(deleteAssetPopup, NULL,
            ImGuiWindowFlags_AlwaysAutoResize)) return deleted;

        std::string reason;
        const bool canDelete = CanDeleteIMG(assetDeleteAssetIndex, &reason);
        if (!canDelete) {
            ImGui::TextWrapped("%s", reason.c_str());
            if (ImGui::Button("Close", ImVec2(100.0f, 0.0f)))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return deleted;
        }

        IMG& asset = ((IMG*)arr_IMG.data)[assetDeleteAssetIndex];
        const std::string assetName = Cp932ToUtf8(
            asset.name.body ? asset.name.outstr() : "(unnamed)");
        ImGui::Text("Asset %03d  gr %d", assetDeleteAssetIndex, asset.gr);
        ImGui::TextWrapped("%s", assetName.c_str());
        ImGui::Text("Crop: x %d  y %d  w %d  h %d",
            asset.x, asset.y, asset.w, asset.h);
        ImGui::Separator();
        ImGui::TextWrapped(
            "Remove this unused crop from the skin? The texture file on disk will not be deleted.");
        ImGui::Spacing();
        if (ImGui::Button("Delete", ImVec2(100.0f, 0.0f))) {
            const int deletedIndex = assetDeleteAssetIndex;
            if (DeleteIMG(deletedIndex) == 0) {
                assetDeleteStatus = "Deleted Asset " +
                    std::to_string(deletedIndex) + ".";
                assetDeleteAssetIndex = -1;
                imageManagerFocusRequest = -1;
                assetBrowserFocusRequest = -1;
                if (arr_IMG.count > 0) {
                    SelectIMGAsset((std::min)(deletedIndex,
                        arr_IMG.count - 1), false);
                } else {
                    src_selected = -1;
                    gr_selected = -1;
                    grID_selected = -1;
                }
                deleted = true;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100.0f, 0.0f)))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        return deleted;
    };

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
    ImGui::SameLine();
    ImGui::Checkbox("Unused only", &assetShowUnusedOnly);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip("Show crops that are not referenced by any Object SRC command.");
    ImGui::Spacing();
    ImGui::BeginDisabled(src_selected < 0 || src_selected >= arr_IMG.count ||
        !hasAssignableObject);
    if (ImGui::Button("Use in selected Object"))
        requestAssetApply(src_selected);
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled |
        ImGuiHoveredFlags_DelayNormal)) {
        if (activeObjectIndex < 0)
            ImGui::SetTooltip("Select an Object first.");
        else if (activeSourceRows.empty())
            ImGui::SetTooltip("The selected Object has no image-backed SRC row.");
        else
            ImGui::SetTooltip("Replace the selected Object SRC crop in one undoable edit.");
    }
    ImGui::SameLine();
    ImGui::Checkbox("Copy animation", &assetApplyCopyAnimation);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip("Also copy div_x, div_y, cycle and timer from the Asset SRC.");
    if (!assetDeleteStatus.empty()) {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", assetDeleteStatus.c_str());
    }

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
        !ImGui::GetIO().WantTextInput && !ImGui::IsAnyItemActive() &&
        ImGui::IsKeyPressed(ImGuiKey_Delete, false) &&
        src_selected >= 0 && src_selected < arr_IMG.count) {
        requestAssetDelete(src_selected);
    }

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
        const std::vector<int>& users = assetUsage[imageIndex];
        if (assetShowUnusedOnly && !users.empty()) continue;
        if (query.empty()) {
            filteredAssets.push_back(imageIndex);
            continue;
        }
        const std::string tagName = Cp932ToUtf8(tag.name.body ? tag.name.outstr() : "");
        char metadata[256];
        snprintf(metadata, sizeof(metadata),
            "%03d asset %d gr %d g%02d if %d %s %s",
            imageIndex, imageIndex, tag.gr, tag.gr, tag.ifGroup,
            tagName.c_str(), users.empty() ? "unused" : "used");
        if (query.empty() || lowercase(metadata).find(query) != std::string::npos)
            filteredAssets.push_back(imageIndex);
    }

    ImGui::SameLine();
    ImGui::TextDisabled("%d / %d", (int)filteredAssets.size(), arr_IMG.count);
    ImGui::Separator();

    if (filteredAssets.empty()) {
        SEUI::EmptyState("No matching assets", "Clear the search text to show every crop.");
        drawBackgroundMenu(true);
        if (drawAssetDeleteDialog()) {
            ImGui::End();
            return 0;
        }
        drawAssetApplyDialog();
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
            ImGui::GetTextLineHeight() * 3.0f + 5.0f;
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
                    const std::vector<int>& users = assetUsage[imageIndex];
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
                    FormatSEUIWindowTitle(managerTitle, sizeof(managerTitle),
                        SEUIWindowId::ImageManager, num);
                    if (hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        SelectIMGAsset(imageIndex, true);
                        imageManagerRevealRequested = true;
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
                    char usageLabel[64];
                    if (users.empty()) {
                        snprintf(usageLabel, sizeof(usageLabel), "Unused");
                    } else {
                        snprintf(usageLabel, sizeof(usageLabel), "%d Object%s",
                            (int)users.size(), users.size() == 1 ? "" : "s");
                    }
                    drawList->AddText(ImVec2(cardMin.x + padding,
                        thumbMax.y + 4.0f + ImGui::GetTextLineHeight() * 2.0f),
                        users.empty() ? ImGui::GetColorU32(SEUI::Colors::Warning())
                            : ImGui::GetColorU32(SEUI::Colors::Success()),
                        usageLabel);
                    drawList->PopClipRect();
                    if (selected)
                        drawList->AddRect(cardMin, cardMax,
                            ImGui::GetColorU32(ImGuiCol_NavHighlight), 0.0f, 0, 2.0f);

                    if (ImGui::BeginPopupContextItem("##AssetContext")) {
                        if (ImGui::MenuItem("Open in Image Manager")) {
                            SelectIMGAsset(imageIndex, true);
                            imageManagerRevealRequested = true;
                        }
                        if (ImGui::MenuItem("Pixel paint in Image Manager")) {
                            SelectIMGAsset(imageIndex, true);
                            imagePixelPaintMode = true;
                            imagePixelPaintLastX = -1;
                            imagePixelPaintLastY = -1;
                            imagePixelPaintLastButton = -1;
                            imageManagerRevealRequested = true;
                        }
                        if (ImGui::MenuItem("Use in selected Object", NULL,
                            false, hasAssignableObject)) {
                            requestAssetApply(imageIndex);
                        }
                        ImGui::Separator();
                        if (ImGui::MenuItem("Select first using Object", NULL,
                            false, !users.empty())) {
                            selectUsageObject(users.front());
                        }
                        char usageMenuLabel[64];
                        snprintf(usageMenuLabel, sizeof(usageMenuLabel),
                            "Used by Objects (%d)", (int)users.size());
                        if (ImGui::BeginMenu(usageMenuLabel, !users.empty())) {
                            const int shownUsers = (std::min)(32,
                                (int)users.size());
                            for (int userIndex = 0; userIndex < shownUsers;
                                ++userIndex) {
                                const int modelIndex = users[userIndex];
                                if (modelIndex < 0 ||
                                    modelIndex >= (int)usageObjects.size()) continue;
                                const SEObjectInstance& object =
                                    usageObjects[modelIndex];
                                const std::string objectName = Cp932ToUtf8(
                                    object.name.empty() ? "(unnamed)" :
                                        object.name.c_str());
                                char objectLabel[320];
                                snprintf(objectLabel, sizeof(objectLabel),
                                    "%03d  %s", modelIndex,
                                    objectName.c_str());
                                if (ImGui::MenuItem(objectLabel))
                                    selectUsageObject(modelIndex);
                            }
                            if ((int)users.size() > shownUsers)
                                ImGui::TextDisabled("... %d more",
                                    (int)users.size() - shownUsers);
                            ImGui::EndMenu();
                        }
                        ImGui::Separator();
                        std::string deleteReason;
                        const bool canDeleteAsset =
                            CanDeleteIMG(imageIndex, &deleteReason);
                        if (ImGui::MenuItem("Delete Asset...", "Delete", false,
                            canDeleteAsset)) {
                            requestAssetDelete(imageIndex);
                        }
                        if (!canDeleteAsset && ImGui::IsItemHovered(
                            ImGuiHoveredFlags_AllowWhenDisabled |
                            ImGuiHoveredFlags_DelayNormal)) {
                            ImGui::SetTooltip("%s", deleteReason.c_str());
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
                        ImGui::Text("Usage: %s", usageLabel);
                        const int tooltipUsers = (std::min)(3,
                            (int)users.size());
                        for (int userIndex = 0; userIndex < tooltipUsers;
                            ++userIndex) {
                            const int modelIndex = users[userIndex];
                            if (modelIndex < 0 ||
                                modelIndex >= (int)usageObjects.size()) continue;
                            const std::string objectName = Cp932ToUtf8(
                                usageObjects[modelIndex].name.empty()
                                    ? "(unnamed)"
                                    : usageObjects[modelIndex].name.c_str());
                            ImGui::BulletText("%03d  %s", modelIndex,
                                objectName.c_str());
                        }
                        if ((int)users.size() > tooltipUsers)
                            ImGui::TextDisabled("... %d more",
                                (int)users.size() - tooltipUsers);
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
        drawBackgroundMenu();
    }
    ImGui::EndChild();
    if (drawAssetDeleteDialog()) {
        ImGui::End();
        return 0;
    }
    drawAssetApplyDialog();
    ImGui::End();
    return 0;
}
