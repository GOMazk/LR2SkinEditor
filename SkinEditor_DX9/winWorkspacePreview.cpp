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
#include "seLocalization.h"
#include "seUI.h"
#include "skinPathResolver.h"
#include "inputwrap.h"
#include "uiCatalog.h"
#include "winWorkspaceUiHelpers.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace {

float SnapPreviewCoordinate(float value, int gridSize) {
    const float grid = (float)(std::max)(1, gridSize);
    return std::round(value / grid) * grid;
}

void FinishPreviewHistoryGroup(WORKSPACE& workspace, int editCount) {
    if (editCount <= 1) return;
    HISTORY* history = (HISTORY*)workspace.arr_history.Get_new();
    if (!history) return;
    history->op = group;
    history->target = editCount;
}

} // namespace

int WORKSPACE::RefreshPreviewSelectionBounds() {
    bool firstBounds = true;
    bool lastBounds = true;
    float minX = 0, minY = 0, maxX = 0, maxY = 0;
    float lastMinX = 0, lastMinY = 0, lastMaxX = 0, lastMaxY = 0;
    const std::vector<SEObjectInstance>& objects = objectEditorModel.Objects();

    auto includeBounds = [](float x, float y, float w, float h, bool& empty,
        float& left, float& top, float& right, float& bottom) {
        const float frameRight = x + w;
        const float frameBottom = y + h;
        const float x1 = x < frameRight ? x : frameRight;
        const float y1 = y < frameBottom ? y : frameBottom;
        const float x2 = x > frameRight ? x : frameRight;
        const float y2 = y > frameBottom ? y : frameBottom;
        if (empty) {
            left = x1; top = y1; right = x2; bottom = y2; empty = false;
        } else {
            if (x1 < left) left = x1;
            if (y1 < top) top = y1;
            if (x2 > right) right = x2;
            if (y2 > bottom) bottom = y2;
        }
    };
    auto sameFrame = [](const DST_ANIMATION& editorFrame, const DSTdraw& runtimeFrame) {
        return std::abs(editorFrame.x - runtimeFrame.x) < 0.5f &&
            std::abs(editorFrame.y - runtimeFrame.y) < 0.5f &&
            std::abs(editorFrame.w - runtimeFrame.w) < 0.5f &&
            std::abs(editorFrame.h - runtimeFrame.h) < 0.5f;
    };

    for (int modelIndex : preview_selected_object_model_indices) {
        if (modelIndex < 0 || modelIndex >= (int)objects.size()) continue;
        const SEObjectInstance& object = objects[modelIndex];

        // The Object model is the authoritative boundary for editor
        // selection. arr_DST is a legacy sequential parser cache and can
        // absorb a following Object's DST rows when include-file reordering
        // places an unsupported/special SRC between them. Building the bounds
        // from the selected Object's own CSV rows keeps Inspector and Preview
        // synchronized even in that case.
        std::vector<DST_ANIMATION> editorFrames;
        for (int objectRow : object.rows) {
            if (objectRow < 0 || objectRow >= skinfileLines.count) continue;
            SKINFILELINEREAD& destinationLine =
                ((SKINFILELINEREAD*)skinfileLines.data)[objectRow];
            if (!destinationLine.csv.str[0].body ||
                strncmp(destinationLine.csv.str[0].outstr(), "#DST", 4) != 0)
                continue;
            DST_ANIMATION frame = {};
            frame.time = destinationLine.csv.val[2];
            frame.x = destinationLine.csv.val[3];
            frame.y = destinationLine.csv.val[4];
            frame.w = destinationLine.csv.val[5];
            frame.h = destinationLine.csv.val[6];
            editorFrames.push_back(frame);
        }
        if (editorFrames.empty()) continue;

        DST* selectedDst = NULL;
        for (int dstIndex = 0; dstIndex < arr_DST.count; ++dstIndex) {
            DST& candidate = ((DST*)arr_DST.data)[dstIndex];
            if (std::find(object.rows.begin(), object.rows.end(),
                    candidate.declare) == object.rows.end())
                continue;
            if (!selectedDst) selectedDst = &candidate;
            if (GetOptionFlag_dst(&g, candidate.op1) && GetOptionFlag_dst(&g, candidate.op2) &&
                GetOptionFlag_dst(&g, candidate.op3)) {
                selectedDst = &candidate;
                break;
            }
        }

        int textAlign = 0;
        int textFontIndex = -1;
        int textStringIndex = -1;
        int numberKeta = 1;
        bool isTextObject = false;
        bool isNumberObject = false;
        SRC* editorSlider = NULL;
        for (int objectRow : object.rows) {
            if (objectRow < 0 || objectRow >= skinfileLines.count) continue;
            SKINFILELINEREAD& sourceLine = ((SKINFILELINEREAD*)skinfileLines.data)[objectRow];
            if (sourceLine.csv.str[0].isSame("#SRC_TEXT")) {
                isTextObject = true;
                textFontIndex = sourceLine.csv.val[2];
                textStringIndex = sourceLine.csv.val[3];
                textAlign = sourceLine.csv.val[4];
            } else if (sourceLine.csv.str[0].isSame("#SRC_NUMBER")) {
                isNumberObject = true;
                numberKeta = (std::max)(1, sourceLine.csv.val[13]);
            } else if (sourceLine.csv.str[0].isSame("#SRC_SLIDER")) {
                for (int srcIndex = 0; srcIndex < arr_SRC.count; ++srcIndex) {
                    SRC& candidate = ((SRC*)arr_SRC.data)[srcIndex];
                    if (candidate.declare == objectRow) {
                        editorSlider = &candidate;
                        break;
                    }
                }
            }
        }
        auto alignedX = [&](float x, float w) {
            if (isTextObject) {
                // TEXT uses 0=left, 1=middle and 2=right. Unlike NUMBER,
                // LR2 treats DST x as an anchor and shifts by the rendered
                // string width (LRDrawText), not by the configured DST width.
                if (textAlign == 1) return x - (float)(int)(w * 0.5f);
                if (textAlign == 2) return x - (float)(int)w;
            }
            // NUMBER align controls digit placement inside the keta-wide
            // field. LR2 always advances from the DST x coordinate, so x is
            // the field's left edge for right, left and middle alignment.
            return x;
        };
        auto textFrameWidth = [&](const DST_ANIMATION& frame) {
            if (!isTextObject || textFontIndex < 0 || textFontIndex >= 10 ||
                textStringIndex < 0 || textStringIndex >= 300 ||
                frame.w == 0.0f || frame.h == 0.0f)
                return frame.w;

            CSTR& text = g.txtStruct.objectStr[textStringIndex];
            if (!text.body || text.length() < 1) return frame.w;

            float naturalWidth = 0.0f;
            float naturalHeight = 0.0f;
            ImageFont& imageFont = g.skstruct.ImageFonts[textFontIndex];
            if (imageFont.size > 0) {
                naturalWidth = (float)GetTextGraphLength(&text, &imageFont);
                naturalHeight = (float)imageFont.size;
            } else {
                const int fontHandle = g.skstruct.fontHandle[textFontIndex];
                int fontSize = 0;
                int fontThickness = 0;
                if (fontHandle != -1 &&
                    GetFontStateToHandle(NULL, &fontSize, &fontThickness, fontHandle) == 0) {
                    naturalWidth = (float)GetDrawStringWidthToHandle(
                        text.outstr(), text.length(), fontHandle, 0);
                    naturalHeight = (float)fontSize;
                }
            }
            if (naturalWidth <= 0.0f || naturalHeight <= 0.0f) return frame.w;

            // Keep the same order as LRDrawText: cap the natural width by
            // DST w first, then apply the scale selected by DST h.
            const float widthScale = naturalWidth > frame.w
                ? frame.w / naturalWidth : 1.0f;
            const float heightScale = frame.h / naturalHeight;
            return naturalWidth * widthScale * heightScale;
        };
        auto objectFrameWidth = [&](const DST_ANIMATION& frame) {
            // LR2 draws NUMBER by advancing one DST width per configured
            // digit. Keep the selection rectangle on the same keta-wide
            // field instead of highlighting only a single glyph cell.
            if (isNumberObject) return frame.w * numberKeta;
            if (isTextObject) return textFrameWidth(frame);
            return frame.w;
        };

        // SliderByTime renders the knob at the animated DST position plus
        // SRCstruct::sx/sy. Highlight that current rectangle, not the raw
        // animation endpoints stored in the CSV.
        bool includedRuntimeSlider = false;
        if (editorSlider && selectedDst) {
            SkinObject& runtimeSliders = g.skstruct.otherObject[2];
            for (int runtimeIndex = 0; runtimeIndex < runtimeSliders.srcSize; ++runtimeIndex) {
                SRCstruct& runtimeSrc = runtimeSliders.src[runtimeIndex];
                DSTstruct& runtimeDst = runtimeSliders.dst[runtimeIndex];
                if (runtimeDst.dstCount != (int)editorFrames.size() ||
                    runtimeDst.dstCount <= 0 || !runtimeDst.draw ||
                    runtimeSrc.op1 != editorSlider->muki ||
                    runtimeSrc.op2 != editorSlider->range ||
                    runtimeSrc.op3 != editorSlider->type ||
                    runtimeSrc.op4 != editorSlider->disable ||
                    runtimeDst.timer != selectedDst->timer ||
                    runtimeDst.loop != selectedDst->loop ||
                    runtimeDst.op1 != selectedDst->op1 ||
                    runtimeDst.op2 != selectedDst->op2 ||
                    runtimeDst.op3 != selectedDst->op3 ||
                    !sameFrame(editorFrames.front(), runtimeDst.draw[0]) ||
                    !sameFrame(editorFrames.back(),
                        runtimeDst.draw[runtimeDst.dstCount - 1]))
                    continue;

                DSTdraw rendered = SetDSTdrawByTime(runtimeDst,
                    GetTimeLapse(runtimeDst.timer, &g.timer1));
                if (rendered.time != -1) {
                    includeBounds(rendered.x + runtimeSrc.sx, rendered.y + runtimeSrc.sy,
                        rendered.w, rendered.h, firstBounds, minX, minY, maxX, maxY);
                    includedRuntimeSlider = true;
                }
                break;
            }
        }
        if (includedRuntimeSlider) continue;

        const DST_ANIMATION& firstFrame = editorFrames.front();
        const float firstFrameWidth = objectFrameWidth(firstFrame);
        includeBounds(alignedX(firstFrame.x, firstFrameWidth), firstFrame.y,
            firstFrameWidth, firstFrame.h,
            firstBounds, minX, minY, maxX, maxY);
        const DST_ANIMATION& lastFrame = editorFrames.back();
        const float lastFrameWidth = objectFrameWidth(lastFrame);
        includeBounds(alignedX(lastFrame.x, lastFrameWidth), lastFrame.y,
            lastFrameWidth, lastFrame.h, lastBounds,
            lastMinX, lastMinY, lastMaxX, lastMaxY);
    }

    preview_selected_obj_valid = !firstBounds;
    preview_selected_obj_last_valid = !lastBounds;
    if (!firstBounds) preview_selected_obj = { minX, minY, maxX - minX, maxY - minY };
    if (!lastBounds) preview_selected_obj_last = {
        lastMinX, lastMinY, lastMaxX - lastMinX, lastMaxY - lastMinY };
    return preview_selected_obj_valid ? 0 : -1;
}

bool WORKSPACE::UpdatePreviewRuntime(unsigned long long previewNow) {
    bool previewFrameUpdated = false;
    const LR2SEPreviewChartMode chartMode = previewChartFull
        ? LR2SE_PREVIEW_CHART_FULL : LR2SE_PREVIEW_CHART_SIMPLE;

    // Object Editor changes first update the in-memory CSV. Rebuild the LR2
    // preview objects shortly after the last edit so coordinates and all DST
    // properties become visible without saving and reopening the skin.
    if (previewReloadPending && previewNow - previewReloadRequestedAt >= 80) {
        LoadSceneSE();
        LR2SEResetRenderFault();
        if (previewSimulationPlaying && LR2SESceneInitSafe(&g, meta.type, chartMode) != 0) {
            previewSimulationPlaying = false;
        }
        previewReloadPending = false;
        previewLastRenderAt = 0;
        RefreshPreviewSelectionBounds();
    }

    // Keep editing and live scene playback at the same responsive refresh
    // rate. Image candidates are now loaded lazily, so the former 15/30 FPS
    // memory-pressure workaround is no longer needed.
    const unsigned long long previewFrameInterval = 16;
    if (previewLastRenderAt == 0 || previewNow - previewLastRenderAt >= previewFrameInterval) {
        // Scene processing queues PLAY notes, judge lines and other dynamic
        // objects in skstruct.drBuf.  It must run before LR2SEDrawLoop: that
        // function draws the queued objects, captures the preview and then
        // clears the buffer.  Running this after the draw loop made every
        // preview-chart note get cleared at the start of the following frame
        // without ever reaching the preview texture.
        if (previewSimulationPlaying && LR2SESceneProcSafe(&g, meta.type, chartMode) == -1) {
            previewSimulationPlaying = false;
        }

        // SELECT still needs its editor-side bar/BGA placeholders while its
        // timers are running; the original selector loop is not driven here.
        const bool staticSpecialPreview =
            !previewSimulationPlaying || meta.type == SKINTYPE_SELECT;
        if (LR2SEDrawLoopSafe(&g, previewScreen, skinSizeX, skinSizeY, staticSpecialPreview) == 0) {
            previewFrameUpdated = true;
            previewTextureDirty = true;
        }
        previewLastRenderAt = previewNow;
    }

    return previewFrameUpdated;
}

int WORKSPACE::drawPreview() {
    // Preview texture is owned by this workspace and recreated whenever
    // the loaded skin resolution changes. ImGui::Begin returns false for an
    // inactive dock tab, but a running scene must still tick and consume its
    // Workspace draw buffer before the presentation-only early return.
    char title[260];
    if (ImGui::IsKeyPressed(ImGuiKey_F11, false))
        previewCanvasFullscreen = !previewCanvasFullscreen;
    if (previewCanvasFullscreen && ImGui::IsKeyPressed(ImGuiKey_Escape, false))
        previewCanvasFullscreen = false;

    ImGuiWindowFlags previewWindowFlags = ImGuiWindowFlags_HorizontalScrollbar;
    if (previewCanvasFullscreen) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
        ImGui::SetNextWindowViewport(viewport->ID);
        snprintf(title, sizeof(title), "Preview Canvas###PreviewFullscreen%d", num);
        previewWindowFlags |= ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings;
    } else {
        FormatSEUIWindowTitle(title, sizeof(title), SEUIWindowId::Preview, num);
    }
    const bool previewWindowVisible = ImGui::Begin(
        title, previewCanvasFullscreen ? NULL : &wPreview, previewWindowFlags);

    const bool previewFrameUpdated = previewWindowVisible ||
        previewSimulationPlaying
        ? UpdatePreviewRuntime(GetTickCount64()) : false;
    if (!previewWindowVisible) {
        ImGui::End();
        return 0;
    }

    float previewCanvasScale = 1.0f / zoom;
    float zoomPercent = previewCanvasScale * 100.0f;
    ImGui::SetNextItemWidth(210.0f);
    if (ImGui::SliderFloat(SEText("Zoom##zoom", u8"\uD655\uB300/\uCD95\uC18C##zoom"),
        &zoomPercent, 25.0f, 1600.0f,
        "%.0f%%", ImGuiSliderFlags_Logarithmic)) {
        if (zoomPercent < 1.0f) zoomPercent = 1.0f;
        previewCanvasScale = zoomPercent / 100.0f;
        zoom = 1.0f / previewCanvasScale;
    }
    ImGui::SameLine();
    if (ImGui::Button(previewCanvasFullscreen
        ? SEText("Exit Fullscreen (F11)", u8"\uC804\uCCB4\uD654\uBA74 \uC885\uB8CC (F11)")
        : SEText("Fullscreen (F11)", u8"\uC804\uCCB4\uD654\uBA74 (F11)")))
        previewCanvasFullscreen = !previewCanvasFullscreen;
    ImGui::SameLine();
    ImGui::SetNextItemWidth(110.0f);
    char snapLabel[32];
    snprintf(snapLabel, sizeof(snapLabel),
        SEText("Snap %d px", u8"\uC2A4\uB0C5 %d px"), previewSnapGridSize);
    if (ImGui::BeginCombo("##PreviewSnapGrid", snapLabel)) {
        const int gridSizes[] = { 1, 2, 4, 8, 10, 16, 32 };
        for (int gridSize : gridSizes) {
            const bool selected = previewSnapGridSize == gridSize;
            char gridLabel[24];
            snprintf(gridLabel, sizeof(gridLabel), "%d px", gridSize);
            if (ImGui::Selectable(gridLabel, selected))
                previewSnapGridSize = gridSize;
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
        ImGui::SetTooltip("%s", SEText(
            "Hold Shift while moving or resizing to snap to this grid.",
            u8"\uC774\uB3D9\uD558\uAC70\uB098 \uD06C\uAE30\uB97C \uBC14\uAFC0 \uB54C Shift\uB97C \uB204\uB974\uBA74 \uC774 \uAC04\uACA9\uC5D0 \uB9DE\uCDB0\uC9D1\uB2C8\uB2E4."));
    ImVec2 p = ImGui::GetCursorScreenPos();
    const ImVec2 oldPreviewCanvasSize(skinSizeX * previewCanvasScale,
        skinSizeY * previewCanvasScale);
    if (ApplyMouseCenteredWheelZoom(previewCanvasScale, 0.25f, 16.0f,
        p, oldPreviewCanvasSize))
        zoom = 1.0f / previewCanvasScale;
    //TODO init zoom value
    //TODO support HD skins

    void* previewPixels = NULL;
    if (previewTextureDirty || !texture_preview)
        previewPixels = GetImageAddressSoftImage(previewScreen);
    const size_t previewBytes = (size_t)skinSizeX * (size_t)skinSizeY * 4;
    if (texture_preview &&
        (texture_preview_width != skinSizeX || texture_preview_height != skinSizeY)) {
        texture_preview->Release();
        texture_preview = NULL;
    }
    if (!texture_preview && previewPixels) {
        if (LoadTextureFromRawMemory(previewPixels, previewBytes,
            skinSizeX, skinSizeY, &texture_preview)) {
            texture_preview_width = skinSizeX;
            texture_preview_height = skinSizeY;
            previewTextureDirty = false;
        }
    }
    else if (previewPixels && previewFrameUpdated) {
        RefreshTextureByRawMemory(previewPixels, previewBytes,
            skinSizeX, skinSizeY, &texture_preview);
        previewTextureDirty = false;
    }

    // An Image does not claim mouse input, so dragging an Object over it can
    // also start ImGui's window move behavior. Use a transparent button as the
    // preview canvas so the canvas owns the drag from its first frame.
    const ImVec2 previewCanvasSize(skinSizeX * previewCanvasScale,
        skinSizeY * previewCanvasScale);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 0, 0, 0));
    const bool sharpPreview = BeginSharpMagnifiedCanvas(previewCanvasScale);
    ImGui::ImageButton("##preview_canvas", texture_preview, previewCanvasSize,
        ImVec2(0, 0), ImVec2(1, 1));
    EndSharpMagnifiedCanvas(sharpPreview);
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    // Asset Browser sends a stable IMG index. Preview owns coordinate
    // conversion because it is the only window that knows the current canvas
    // origin, scroll and zoom. Delivery opens the existing New Object form;
    // no CSV or runtime Object is mutated until the user confirms that form.
    if (ImGui::BeginDragDropTarget()) {
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(
            "SKINEDITOR_IMG_ASSET",
            ImGuiDragDropFlags_AcceptBeforeDelivery |
            ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
        if (payload && payload->DataSize == sizeof(int)) {
            const int assetIndex = *(const int*)payload->Data;
            if (assetIndex >= 0 && assetIndex < arr_IMG.count) {
                IMG& asset = ((IMG*)arr_IMG.data)[assetIndex];
                const int textureIndex = ResolveIMGTextureIndex(assetIndex);
                SRCGR* source = textureIndex >= 0 && textureIndex < arr_SRCGR.count
                    ? &((SRCGR*)arr_SRCGR.data)[textureIndex] : NULL;
                int cropW = asset.w;
                int cropH = asset.h;
                if (source && source->sizeX > 0 && source->sizeY > 0) {
                    if (cropW == -1) cropW = source->sizeX - asset.x;
                    if (cropH == -1) cropH = source->sizeY - asset.y;
                }
                cropW = (std::max)(1, cropW);
                cropH = (std::max)(1, cropH);

                int divX = 1;
                int divY = 1;
                int cycle = 0;
                int timer = 0;
                ResolveIMGDivision(assetIndex, divX, divY, cycle, timer);
                (void)timer;
                const long long frameCount64 = (long long)divX * (long long)divY;
                const int frameCount = frameCount64 > 0 && frameCount64 <= INT_MAX
                    ? (int)frameCount64 : 1;
                int frame = 0;
                if (assetAnimateSrc && cycle > 0 && frameCount > 1) {
                    const long long elapsedMs =
                        (long long)(ImGui::GetTime() * 1000.0);
                    frame = (int)(((elapsedMs % cycle) * frameCount) / cycle);
                    if (frame >= frameCount) frame = frameCount - 1;
                }
                const int frameX = frame % divX;
                const int frameY = frame / divX;
                const int frameCropX0 = asset.x + (cropW * frameX) / divX;
                const int frameCropY0 = asset.y + (cropH * frameY) / divY;
                const int frameCropX1 = asset.x + (cropW * (frameX + 1)) / divX;
                const int frameCropY1 = asset.y + (cropH * (frameY + 1)) / divY;
                const int frameW = (std::max)(1, frameCropX1 - frameCropX0);
                const int frameH = (std::max)(1, frameCropY1 - frameCropY0);

                const ImVec2 mouse = ImGui::GetIO().MousePos;
                const int dropX = (std::max)(0, (std::min)(skinSizeX,
                    (int)floorf((mouse.x - p.x) / previewCanvasScale)));
                const int dropY = (std::max)(0, (std::min)(skinSizeY,
                    (int)floorf((mouse.y - p.y) / previewCanvasScale)));
                const ImVec2 ghostMin(p.x + dropX * previewCanvasScale,
                    p.y + dropY * previewCanvasScale);
                const ImVec2 ghostMax(ghostMin.x + frameW * previewCanvasScale,
                    ghostMin.y + frameH * previewCanvasScale);
                const ImVec2 previewMax(p.x + previewCanvasSize.x,
                    p.y + previewCanvasSize.y);
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                drawList->PushClipRect(p, previewMax, true);
                if (source && source->texture && source->sizeX > 0 && source->sizeY > 0) {
                    const int cropX0 = (std::max)(0,
                        (std::min)(source->sizeX, frameCropX0));
                    const int cropY0 = (std::max)(0,
                        (std::min)(source->sizeY, frameCropY0));
                    const int cropX1 = (std::max)(cropX0,
                        (std::min)(source->sizeX, frameCropX1));
                    const int cropY1 = (std::max)(cropY0,
                        (std::min)(source->sizeY, frameCropY1));
                    const ImVec2 uv0(cropX0 / (float)source->sizeX,
                        cropY0 / (float)source->sizeY);
                    const ImVec2 uv1(cropX1 / (float)source->sizeX,
                        cropY1 / (float)source->sizeY);
                    const bool sharpGhost = BeginSharpMagnifiedCanvas(previewCanvasScale);
                    drawList->AddImage(source->texture, ghostMin, ghostMax, uv0, uv1,
                        IM_COL32(255, 255, 255, 180));
                    EndSharpMagnifiedCanvas(sharpGhost);
                } else {
                    drawList->AddRectFilled(ghostMin, ghostMax,
                        IM_COL32(76, 151, 255, 75));
                }
                drawList->AddRect(ghostMin, ghostMax,
                    IM_COL32(76, 190, 255, 255), 0.0f, 0, 2.0f);
                drawList->PopClipRect();

                if (payload->IsDelivery())
                    OpenNewObjectFromAsset(assetIndex, dropX, dropY);
            }
        }
        ImGui::EndDragDropTarget();
    }

    // Slider positions can change every scene frame without editing the CSV.
    // Refresh only slider selections here so the highlight follows the knob,
    // while an active preview drag keeps its own temporary coordinates.
    if (!preview_object_dragging && !preview_object_resizing &&
        !preview_selected_object_model_indices.empty()) {
        bool sliderSelected = false;
        const std::vector<SEObjectInstance>& selectedObjects = objectEditorModel.Objects();
        for (int modelIndex : preview_selected_object_model_indices) {
            if (modelIndex < 0 || modelIndex >= (int)selectedObjects.size()) continue;
            for (int row : selectedObjects[modelIndex].rows) {
                if (row < 0 || row >= skinfileLines.count) continue;
                SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                if (line.csv.str[0].isSame("#SRC_SLIDER")) {
                    sliderSelected = true;
                    break;
                }
            }
            if (sliderSelected) break;
        }
        if (sliderSelected) RefreshPreviewSelectionBounds();
    }

    // Drag the highlighted Object. All DST animation rows receive the same
    // delta, preserving animation while EditValue records CSV and History.
    if (preview_selected_obj_valid) {
        const float previewScale = 1.0f / zoom;
        float hitX1 = preview_selected_obj.x;
        float hitY1 = preview_selected_obj.y;
        float hitX2 = preview_selected_obj.x + preview_selected_obj.w;
        float hitY2 = preview_selected_obj.y + preview_selected_obj.h;
        if (hitX1 > hitX2) std::swap(hitX1, hitX2);
        if (hitY1 > hitY2) std::swap(hitY1, hitY2);
        // Keep at least a 20x20 screen-space grab area. Very small or thin
        // LR2 objects are otherwise nearly impossible to catch with a mouse.
        const float grabPadding = 10.0f;
        const ImVec2 hitMin(p.x + hitX1 * previewScale - grabPadding,
            p.y + hitY1 * previewScale - grabPadding);
        const ImVec2 hitMax(p.x + hitX2 * previewScale + grabPadding,
            p.y + hitY2 * previewScale + grabPadding);
        const ImVec2 imageMax(p.x + skinSizeX * previewScale,
            p.y + skinSizeY * previewScale);
        const ImVec2 mousePos = ImGui::GetIO().MousePos;
        const bool overPreviewImage = mousePos.x >= p.x && mousePos.x <= imageMax.x &&
            mousePos.y >= p.y && mousePos.y <= imageMax.y;
        const bool overSelectedObject = overPreviewImage &&
            mousePos.x >= hitMin.x && mousePos.x <= hitMax.x &&
            mousePos.y >= hitMin.y && mousePos.y <= hitMax.y;
        const bool canResizeObject = preview_selected_object_model_indices.size() == 1;
        const ImVec2 resizeHandle(
            p.x + (preview_selected_obj.x + preview_selected_obj.w) * previewScale,
            p.y + (preview_selected_obj.y + preview_selected_obj.h) * previewScale);
        const float resizeHandleRadius = 8.0f;
        const bool overResizeHandle = canResizeObject && overPreviewImage &&
            fabsf(mousePos.x - resizeHandle.x) <= resizeHandleRadius &&
            fabsf(mousePos.y - resizeHandle.y) <= resizeHandleRadius;

        if (canResizeObject) {
            ImGui::GetWindowDrawList()->AddRectFilled(
                ImVec2(resizeHandle.x - 4.0f, resizeHandle.y - 4.0f),
                ImVec2(resizeHandle.x + 4.0f, resizeHandle.y + 4.0f),
                IM_COL32(255, 255, 255, 245));
            ImGui::GetWindowDrawList()->AddRect(
                ImVec2(resizeHandle.x - 5.0f, resizeHandle.y - 5.0f),
                ImVec2(resizeHandle.x + 5.0f, resizeHandle.y + 5.0f),
                IM_COL32(32, 210, 255, 255));
        }

        if (overResizeHandle || preview_object_resizing)
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
        else if (overSelectedObject || preview_object_dragging)
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        if (overResizeHandle && !preview_object_dragging && !preview_object_resizing)
            ImGui::SetTooltip("Drag to resize the first destination rectangle");
        else if (overSelectedObject && !preview_object_dragging && !preview_object_resizing)
            ImGui::SetTooltip("Drag to move Object");

        // Read MouseDown directly. The preview Image may already own the
        // frame's click, so relying on IsMouseClicked can miss the drag start.
        if (!preview_object_dragging && !preview_object_resizing && overResizeHandle &&
            ImGui::GetIO().MouseDown[ImGuiMouseButton_Left]) {
            preview_object_resizing = true;
            preview_drag_mouse_start = mousePos;
            preview_resize_object_start_w = preview_selected_obj.w;
            preview_resize_object_start_h = preview_selected_obj.h;
        }
        else if (!preview_object_dragging && !preview_object_resizing &&
            overSelectedObject && ImGui::GetIO().MouseDown[ImGuiMouseButton_Left]) {
            preview_object_dragging = true;
            preview_drag_mouse_start = mousePos;
            preview_drag_object_start_x = preview_selected_obj.x;
            preview_drag_object_start_y = preview_selected_obj.y;
            preview_drag_last_start_x = preview_selected_obj_last.x;
            preview_drag_last_start_y = preview_selected_obj_last.y;
        }

        if (preview_object_resizing) {
            const ImVec2 mouseNow = ImGui::GetIO().MousePos;
            const float deltaX = (mouseNow.x - preview_drag_mouse_start.x) * zoom;
            const float deltaY = (mouseNow.y - preview_drag_mouse_start.y) * zoom;
            float proposedWidth = preview_resize_object_start_w + deltaX;
            float proposedHeight = preview_resize_object_start_h + deltaY;
            if (ImGui::GetIO().KeyShift) {
                proposedWidth = SnapPreviewCoordinate(proposedWidth,
                    previewSnapGridSize);
                proposedHeight = SnapPreviewCoordinate(proposedHeight,
                    previewSnapGridSize);
            }
            preview_selected_obj.w = preview_resize_object_start_w < 0.0f
                ? (std::min)(-1.0f, proposedWidth) : (std::max)(1.0f, proposedWidth);
            preview_selected_obj.h = preview_resize_object_start_h < 0.0f
                ? (std::min)(-1.0f, proposedHeight) : (std::max)(1.0f, proposedHeight);
            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                const int resizedWidth = (int)std::round(preview_selected_obj.w);
                const int resizedHeight = (int)std::round(preview_selected_obj.h);
                const int modelIndex = preview_selected_object_model_indices.front();
                int historyEditCount = 0;
                if (modelIndex >= 0 && modelIndex < (int)objectEditorModel.Objects().size()) {
                    const SEObjectInstance& selectedObject = objectEditorModel.Objects()[modelIndex];
                    for (int row : selectedObject.rows) {
                        if (row < 0 || row >= skinfileLines.count) continue;
                        SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                        const char* command = line.csv.str[0].body ? line.csv.str[0].outstr() : "";
                        if (strncmp(command, "#DST_", 5) != 0) continue;
                        const int widthColumn = FindCommandFieldColumn(command, "w");
                        int heightColumn = FindCommandFieldColumn(command, "h");
                        if (heightColumn < 0) heightColumn = FindCommandFieldColumn(command, "size");
                        if (widthColumn >= 0 && heightColumn >= 0) {
                            if (EditValue(row, widthColumn, resizedWidth) == 0)
                                ++historyEditCount;
                            if (EditValue(row, heightColumn, resizedHeight) == 0)
                                ++historyEditCount;
                            previewReloadPending = true;
                            previewReloadRequestedAt = GetTickCount64();
                            break;
                        }
                    }
                }
                FinishPreviewHistoryGroup(*this, historyEditCount);
                preview_selected_obj.w = (float)resizedWidth;
                preview_selected_obj.h = (float)resizedHeight;
                preview_object_resizing = false;
            }
        }

        if (preview_object_dragging) {
            const ImVec2 mouseNow = ImGui::GetIO().MousePos;
            float deltaX = (mouseNow.x - preview_drag_mouse_start.x) * zoom;
            float deltaY = (mouseNow.y - preview_drag_mouse_start.y) * zoom;
            preview_selected_obj.x = preview_drag_object_start_x + deltaX;
            preview_selected_obj.y = preview_drag_object_start_y + deltaY;
            if (ImGui::GetIO().KeyShift) {
                preview_selected_obj.x = SnapPreviewCoordinate(
                    preview_selected_obj.x, previewSnapGridSize);
                preview_selected_obj.y = SnapPreviewCoordinate(
                    preview_selected_obj.y, previewSnapGridSize);
                deltaX = preview_selected_obj.x - preview_drag_object_start_x;
                deltaY = preview_selected_obj.y - preview_drag_object_start_y;
            }
            if (preview_selected_obj_last_valid) {
                preview_selected_obj_last.x = preview_drag_last_start_x + deltaX;
                preview_selected_obj_last.y = preview_drag_last_start_y + deltaY;
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                const int moveX = (int)std::round(
                    preview_selected_obj.x - preview_drag_object_start_x);
                const int moveY = (int)std::round(
                    preview_selected_obj.y - preview_drag_object_start_y);
                int historyEditCount = 0;
                if ((moveX != 0 || moveY != 0) && !preview_selected_object_model_indices.empty()) {
                    for (int selectedModelIndex : preview_selected_object_model_indices) {
                    if (selectedModelIndex < 0 ||
                        selectedModelIndex >= (int)objectEditorModel.Objects().size()) continue;
                    const SEObjectInstance& selectedObject = objectEditorModel.Objects()[selectedModelIndex];
                    for (int rowIndex = 0; rowIndex < (int)selectedObject.rows.size(); ++rowIndex) {
                        const int row = selectedObject.rows[rowIndex];
                        if (row < 0 || row >= skinfileLines.count) continue;
                        SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                        if (!line.csv.str[0].body || strncmp(line.csv.str[0].outstr(), "#DST", 4) != 0) continue;
                        if (EditValue(row, 3, line.csv.val[3] + moveX) == 0)
                            ++historyEditCount;
                        if (EditValue(row, 4, line.csv.val[4] + moveY) == 0)
                            ++historyEditCount;
                    }
                    for (int dstIndex = 0; dstIndex < arr_DST.count; ++dstIndex) {
                        DST& dst = ((DST*)arr_DST.data)[dstIndex];
                        bool belongsToObject = false;
                        for (int rowIndex = 0; rowIndex < (int)selectedObject.rows.size(); ++rowIndex) {
                            if (dst.declare == selectedObject.rows[rowIndex]) { belongsToObject = true; break; }
                        }
                        if (!belongsToObject) continue;
                        for (int frameIndex = 0; frameIndex < dst.arr_animation.count; ++frameIndex) {
                            DST_ANIMATION& frame = ((DST_ANIMATION*)dst.arr_animation.data)[frameIndex];
                            frame.x += moveX;
                            frame.y += moveY;
                        }
                    }
                    }
                }
                FinishPreviewHistoryGroup(*this, historyEditCount);
                preview_selected_obj.x = preview_drag_object_start_x + moveX;
                preview_selected_obj.y = preview_drag_object_start_y + moveY;
                if (preview_selected_obj_last_valid) {
                    preview_selected_obj_last.x = preview_drag_last_start_x + moveX;
                    preview_selected_obj_last.y = preview_drag_last_start_y + moveY;
                }
                preview_object_dragging = false;
                preview_object_resizing = false;
            }
        }

        // Fine positioning: move every selected Object by exactly one skin
        // pixel per arrow-key step. Key repeat is enabled for held keys, but
        // text/property editing keeps ownership of the arrows.
        if (!preview_object_dragging && !preview_object_resizing &&
            !ImGui::GetIO().WantTextInput && !ImGui::IsAnyItemActive()) {
            int keyMoveX = 0;
            int keyMoveY = 0;
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, true)) --keyMoveX;
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, true)) ++keyMoveX;
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, true)) --keyMoveY;
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, true)) ++keyMoveY;
            if (ImGui::GetIO().KeyShift) {
                keyMoveX *= previewSnapGridSize;
                keyMoveY *= previewSnapGridSize;
            }
            if ((keyMoveX != 0 || keyMoveY != 0) &&
                !preview_selected_object_model_indices.empty()) {
                int historyEditCount = 0;
                for (int selectedModelIndex : preview_selected_object_model_indices) {
                    if (selectedModelIndex < 0 ||
                        selectedModelIndex >= (int)objectEditorModel.Objects().size()) continue;
                    const SEObjectInstance& selectedObject =
                        objectEditorModel.Objects()[selectedModelIndex];
                    for (int rowIndex = 0; rowIndex < (int)selectedObject.rows.size(); ++rowIndex) {
                        const int row = selectedObject.rows[rowIndex];
                        if (row < 0 || row >= skinfileLines.count) continue;
                        SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                        if (!line.csv.str[0].body ||
                            strncmp(line.csv.str[0].outstr(), "#DST", 4) != 0) continue;
                        if (EditValue(row, 3, line.csv.val[3] + keyMoveX) == 0)
                            ++historyEditCount;
                        if (EditValue(row, 4, line.csv.val[4] + keyMoveY) == 0)
                            ++historyEditCount;
                    }
                    for (int dstIndex = 0; dstIndex < arr_DST.count; ++dstIndex) {
                        DST& dst = ((DST*)arr_DST.data)[dstIndex];
                        bool belongsToObject = false;
                        for (int rowIndex = 0; rowIndex < (int)selectedObject.rows.size(); ++rowIndex) {
                            if (dst.declare == selectedObject.rows[rowIndex]) {
                                belongsToObject = true;
                                break;
                            }
                        }
                        if (!belongsToObject) continue;
                        for (int frameIndex = 0; frameIndex < dst.arr_animation.count; ++frameIndex) {
                            DST_ANIMATION& frame = ((DST_ANIMATION*)dst.arr_animation.data)[frameIndex];
                            frame.x += keyMoveX;
                            frame.y += keyMoveY;
                        }
                    }
                }
                FinishPreviewHistoryGroup(*this, historyEditCount);
                preview_selected_obj.x += keyMoveX;
                preview_selected_obj.y += keyMoveY;
                if (preview_selected_obj_last_valid) {
                    preview_selected_obj_last.x += keyMoveX;
                    preview_selected_obj_last.y += keyMoveY;
                }
            }
        }
    }
    // Flash the selected Object's destination rectangle over the preview.
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if (preview_selected_obj_valid && ((int)GetTimeLapse(1, &g.timer1) % 500 < 300)) {
        const float previewScale = 1.0f / zoom;
        auto drawDstBounds = [&](const PreviewSelectionBounds& bounds, ImU32 color) {
            float x1 = bounds.x;
            float y1 = bounds.y;
            float x2 = bounds.x + bounds.w;
            float y2 = bounds.y + bounds.h;
            if (x1 > x2) std::swap(x1, x2);
            if (y1 > y2) std::swap(y1, y2);
            ImVec2 dstTopLeft(p.x + x1 * previewScale - 2.0f,
                p.y + y1 * previewScale - 2.0f);
            ImVec2 dstBottomRight(p.x + x2 * previewScale + 2.0f,
                p.y + y2 * previewScale + 2.0f);
            draw_list->AddRect(dstTopLeft, dstBottomRight, color,
                0.0f, ImDrawFlags_Closed, 3.0f);
            draw_list->AddRect(ImVec2(dstTopLeft.x + 2, dstTopLeft.y + 2),
                ImVec2(dstBottomRight.x - 2, dstBottomRight.y - 2),
                IM_COL32(255, 255, 255, 210), 0.0f, ImDrawFlags_Closed, 1.0f);
        };
        // Cyan marks the starting DST frame; red marks a distinct final
        // destination. A one-frame Object intentionally shows only cyan.
        drawDstBounds(preview_selected_obj, IM_COL32(32, 210, 255, 255));
        const bool distinctLastBounds = preview_selected_obj_last_valid &&
            (std::abs(preview_selected_obj.x - preview_selected_obj_last.x) >= 0.5f ||
                std::abs(preview_selected_obj.y - preview_selected_obj_last.y) >= 0.5f ||
                std::abs(preview_selected_obj.w - preview_selected_obj_last.w) >= 0.5f ||
                std::abs(preview_selected_obj.h - preview_selected_obj_last.h) >= 0.5f);
        if (distinctLastBounds)
            drawDstBounds(preview_selected_obj_last, IM_COL32(255, 48, 48, 255));
    }

    // Screen-space coordinate meter: its size is independent of Preview zoom,
    // and the dark fill + black shadow + light border stay legible over both
    // bright and dark skins. Anchor it to the visible Preview panel rather
    // than the potentially scrolled canvas extent.
    if (preview_selected_obj_valid) {
        const ImVec2 windowPosition = ImGui::GetWindowPos();
        const ImVec2 contentRegionMax = ImGui::GetWindowContentRegionMax();
        const ImVec2 contentMax(windowPosition.x + contentRegionMax.x,
            windowPosition.y + contentRegionMax.y);
        const ImVec2 meterSize(126.0f, 58.0f);
        const ImVec2 meterMax(contentMax.x - 12.0f, contentMax.y - 12.0f);
        const ImVec2 meterMin(meterMax.x - meterSize.x,
            meterMax.y - meterSize.y);
        draw_list->AddRectFilled(meterMin, meterMax,
            IM_COL32(12, 16, 24, 232), 6.0f);
        draw_list->AddRect(ImVec2(meterMin.x - 1.0f, meterMin.y - 1.0f),
            ImVec2(meterMax.x + 1.0f, meterMax.y + 1.0f),
            IM_COL32(0, 0, 0, 255), 7.0f, 0, 3.0f);
        draw_list->AddRect(meterMin, meterMax,
            IM_COL32(238, 244, 255, 230), 6.0f, 0, 1.0f);

        char xValue[32];
        char yValue[32];
        snprintf(xValue, sizeof(xValue), "%d",
            (int)std::round(preview_selected_obj.x));
        snprintf(yValue, sizeof(yValue), "%d",
            (int)std::round(preview_selected_obj.y));
        const ImVec2 labelX(meterMin.x + 12.0f, meterMin.y + 9.0f);
        const ImVec2 labelY(meterMin.x + 12.0f, meterMin.y + 31.0f);
        const ImVec2 valueX(meterMin.x + 42.0f, meterMin.y + 9.0f);
        const ImVec2 valueY(meterMin.x + 42.0f, meterMin.y + 31.0f);
        draw_list->AddText(labelX, IM_COL32(54, 220, 255, 255), "X");
        draw_list->AddText(labelY, IM_COL32(255, 210, 72, 255), "Y");
        draw_list->AddText(ImVec2(valueX.x + 1.0f, valueX.y + 1.0f),
            IM_COL32(0, 0, 0, 255), xValue);
        draw_list->AddText(ImVec2(valueY.x + 1.0f, valueY.y + 1.0f),
            IM_COL32(0, 0, 0, 255), yValue);
        draw_list->AddText(valueX, IM_COL32(255, 255, 255, 255), xValue);
        draw_list->AddText(valueY, IM_COL32(255, 255, 255, 255), yValue);
    }

    //test objects on cursor
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGuiIO& io = ImGui::GetIO();
        clickPos = { io.MousePos.x,io.MousePos.y };
        drawRightClick = true;
    }
    // Do not clear drawRightClick on a left press here.  The popup items are
    // processed later in this function; clearing it first skipped the entire
    // popup block on the exact frame a Selectable was clicked.
    preview_hover_obj_valid = false;
    if(drawRightClick){
        ImGui::PushID(num);
        if (ImGui::BeginPopupContextWindow()) {
            auto branchConditionMatches = [&](int ifgroup) -> bool {
                if (ifgroup <= 0 || ifgroup >= arr_ifunit.count) return ifgroup == 0;
                for (int row = 0; row < skinfileLines.count; ++row) {
                    SKINFILELINEREAD& header = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    if (header.ifgroup != ifgroup || !header.isIfGroupHead ||
                        !header.csv.str[0].body) continue;
                    const char* command = header.csv.str[0].outstr();
                    if (!strcmp(command, "#ELSE")) return true;
                    if (strcmp(command, "#IF") && strcmp(command, "#ELSEIF")) return false;
                    for (int col = 1; col <= 10; ++col) {
                        const int option = header.csv.val[col];
                        if (option != 0 && !GetOptionFlag_dst(&g, option)) return false;
                    }
                    return true;
                }
                return false;
            };
            auto rootIfgroup = [&](int ifgroup) -> int {
                if (ifgroup <= 0 || ifgroup >= arr_ifunit.count) return 0;
                IFUNIT& unit = ((IFUNIT*)arr_ifunit.data)[ifgroup];
                if (unit.order == 0) return ifgroup;
                for (int group = ifgroup - 1; group > 0; --group) {
                    IFUNIT& candidate = ((IFUNIT*)arr_ifunit.data)[group];
                    if (candidate.depth == unit.depth && candidate.parentID == unit.parentID &&
                        candidate.order == 0) return group;
                }
                return ifgroup;
            };
            std::function<bool(int)> branchIsActive = [&](int ifgroup) -> bool {
                if (ifgroup == 0) return true;
                if (ifgroup < 0 || ifgroup >= arr_ifunit.count ||
                    !branchConditionMatches(ifgroup)) return false;
                IFUNIT& unit = ((IFUNIT*)arr_ifunit.data)[ifgroup];
                if (!branchIsActive(unit.parentID)) return false;
                const int chainRoot = rootIfgroup(ifgroup);
                for (int sibling = 1; sibling < arr_ifunit.count; ++sibling) {
                    IFUNIT& previous = ((IFUNIT*)arr_ifunit.data)[sibling];
                    if (previous.parentID == unit.parentID && previous.depth == unit.depth &&
                        previous.order < unit.order && rootIfgroup(sibling) == chainRoot &&
                        branchConditionMatches(sibling)) return false;
                }
                return true;
            };

            ImGui::TextDisabled("Front-most Object first");
            ImGui::Separator();
            std::vector<int> listedObjectModels;
            // LR2 sorts draw commands by the source CSV order: later DST rows
            // are drawn over earlier ones. Walk the editor DST cache backwards
            // so the context menu presents the visible/front-most Object first.
            for (int i = arr_DST.count - 1; i >= 0; --i) {
                DST& dst = ((DST*)arr_DST.data)[i];
                if (dst.arr_animation.count <= 0 ||
                    !GetOptionFlag_dst(&g, dst.op1) ||
                    !GetOptionFlag_dst(&g, dst.op2) ||
                    !GetOptionFlag_dst(&g, dst.op3)) continue;
                DST_ANIMATION& dstd = ((DST_ANIMATION*)dst.arr_animation.data)[dst.arr_animation.count - 1];
                const float hitScale = 1.0f / zoom;
                float hitX1 = dstd.x, hitY1 = dstd.y;
                float hitX2 = dstd.x + dstd.w, hitY2 = dstd.y + dstd.h;
                if (hitX1 > hitX2) std::swap(hitX1, hitX2);
                if (hitY1 > hitY2) std::swap(hitY1, hitY2);
                ImVec2 dstposLU = { p.x + hitX1 * hitScale, p.y + hitY1 * hitScale };
                ImVec2 dstposRB = { p.x + hitX2 * hitScale, p.y + hitY2 * hitScale };

                if (dstposLU.x <= clickPos.x && clickPos.x <= dstposRB.x && dstposLU.y <= clickPos.y && clickPos.y <= dstposRB.y) {
                    int objectModelIndex = -1;
                    const std::vector<SEObjectInstance>& objects = objectEditorModel.Objects();
                    for (int modelIndex = 0; modelIndex < (int)objects.size(); ++modelIndex) {
                        if (std::find(objects[modelIndex].rows.begin(), objects[modelIndex].rows.end(),
                            dst.declare) != objects[modelIndex].rows.end()) {
                            objectModelIndex = modelIndex;
                            break;
                        }
                    }
                    if (objectModelIndex < 0 ||
                        !branchIsActive(objects[objectModelIndex].ifgroup)) continue;
                    if (std::find(listedObjectModels.begin(), listedObjectModels.end(), objectModelIndex) !=
                        listedObjectModels.end()) continue;
                    listedObjectModels.push_back(objectModelIndex);

                    // Resolve the thumbnail from this Object's matching SRC
                    // row, rather than trusting DST::src's parser-global
                    // sequence number.  Indexed object families can contain
                    // several SRC/DST command pairs, so also match the command
                    // suffix and index column.
                    int thumbnailSrc = -1;
                    const int dstRow = dst.declare;
                    SKINFILELINEREAD* dstLine = dstRow >= 0 && dstRow < skinfileLines.count
                        ? &((SKINFILELINEREAD*)skinfileLines.data)[dstRow] : NULL;
                    std::string wantedSrcCommand;
                    int wantedKey = 0;
                    bool hasWantedKey = false;
                    if (dstLine && dstLine->csv.str[0].body) {
                        const char* dstCommand = dstLine->csv.str[0].outstr();
                        if (!strncmp(dstCommand, "#DST_", 5))
                            wantedSrcCommand = std::string("#SRC_") + (dstCommand + 5);
                        if (dstLine->csv.str[1].body) {
                            wantedKey = dstLine->csv.val[1];
                            hasWantedKey = true;
                        }
                    }
                    int fallbackSrc = -1;
                    for (int objectRow : objects[objectModelIndex].rows) {
                        if (objectRow < 0 || objectRow >= skinfileLines.count) continue;
                        SKINFILELINEREAD& sourceLine =
                            ((SKINFILELINEREAD*)skinfileLines.data)[objectRow];
                        const char* sourceCommand = sourceLine.csv.str[0].body
                            ? sourceLine.csv.str[0].outstr() : "";
                        if (strncmp(sourceCommand, "#SRC_", 5)) continue;
                        int sourceArrayIndex = -1;
                        for (int sourceIndex = 0; sourceIndex < arr_SRC.count; ++sourceIndex) {
                            if (((SRC*)arr_SRC.data)[sourceIndex].declare == objectRow) {
                                sourceArrayIndex = sourceIndex;
                                break;
                            }
                        }
                        if (sourceArrayIndex < 0) continue;
                        if (fallbackSrc < 0) fallbackSrc = sourceArrayIndex;
                        const bool commandMatches = wantedSrcCommand.empty() ||
                            wantedSrcCommand == sourceCommand;
                        const bool keyMatches = !hasWantedKey || !sourceLine.csv.str[1].body ||
                            sourceLine.csv.val[1] == wantedKey;
                        if (commandMatches && keyMatches) {
                            thumbnailSrc = sourceArrayIndex;
                            break;
                        }
                    }
                    if (thumbnailSrc < 0) thumbnailSrc = fallbackSrc;
                    if (thumbnailSrc < 0 && dst.src >= 0 && dst.src < arr_SRC.count)
                        thumbnailSrc = dst.src;

                    char hitLabel[256];
                    const std::string hitName = Cp932ToUtf8(objects[objectModelIndex].name.c_str());
                    snprintf(hitLabel, sizeof(hitLabel), "Object %03d  %s##PreviewHit%d",
                        objectModelIndex, hitName.c_str(), objectModelIndex);
                    const ImVec2 hitRowStart = ImGui::GetCursorScreenPos();
                    bool chooseObject = ImGui::Selectable(hitLabel, false,
                        ImGuiSelectableFlags_AllowDoubleClick, ImVec2(310.0f, 64.0f));
                    bool hoverObject = ImGui::IsItemHovered();
                    ImGui::SetCursorScreenPos(ImVec2(hitRowStart.x + 215.0f, hitRowStart.y + 2.0f));
                    if (thumbnailSrc >= 0 && thumbnailSrc < arr_SRC.count &&
                        ((SRC*)arr_SRC.data)[thumbnailSrc].gr != 110 &&
                        ((SRC*)arr_SRC.data)[thumbnailSrc].gr != 111) {
                        printSrcImgEx(((SRC*)arr_SRC.data)[thumbnailSrc], 90, 60);
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) chooseObject = true;
                        if (ImGui::IsItemHovered()) hoverObject = true;
                    } else {
                        ImGui::TextDisabled("Special source");
                        if (ImGui::IsItemHovered()) hoverObject = true;
                    }
                    // SetCursorScreenPos() alone no longer contributes an item to the
                    // parent layout (Dear ImGui 1.89+).  Submit a zero-size item at
                    // the restored row end so the popup's content bounds and next
                    // row cursor are both finalized without triggering the layout
                    // assertion.
                    ImGui::SetCursorScreenPos(ImVec2(hitRowStart.x, hitRowStart.y + 66.0f));
                    ImGui::Dummy(ImVec2(0.0f, 0.0f));
                    if (hoverObject) {
                        preview_hover_obj = { dstd.x, dstd.y, dstd.w, dstd.h };
                        preview_hover_obj_valid = true;
                    }
                    if (chooseObject && objectModelIndex >= 0) {
                        wObjectEditor = true;
                        selected_object_group = -1;
                        selected_user_object_group = -1;
                        selected_object_editor = objectModelIndex;
                        SetObjectSelection(std::vector<int>(1, objectModelIndex),
                            objectModelIndex, objectModelIndex, true);
                        preview_object_dragging = false;
                        preview_object_resizing = false;

                        RefreshPreviewSelectionBounds();
                        ImGui::CloseCurrentPopup();
                    }
                }
            }

            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    if (preview_hover_obj_valid && ((int)GetTimeLapse(1, &g.timer1) % 400 < 250)) {
        float x1 = preview_hover_obj.x;
        float y1 = preview_hover_obj.y;
        float x2 = preview_hover_obj.x + preview_hover_obj.w;
        float y2 = preview_hover_obj.y + preview_hover_obj.h;
        if (x1 > x2) std::swap(x1, x2);
        if (y1 > y2) std::swap(y1, y2);
        const float previewScale = 1.0f / zoom;
        draw_list->AddRect(
            ImVec2(p.x + x1 * previewScale, p.y + y1 * previewScale),
            ImVec2(p.x + x2 * previewScale, p.y + y2 * previewScale),
            IM_COL32(255, 230, 40, 235), 0.0f, ImDrawFlags_Closed, 1.0f);
    }

    ImGui::End();

    if (!wPreview) SetWindowVisibleFlag(0);
    return 0;
}
