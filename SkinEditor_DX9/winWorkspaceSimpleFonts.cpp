#include "winWorkspace.h"
#include "seLocalization.h"
#include "seUI.h"

#include <commdlg.h>
#include <d3dx9.h>
#include <wrl/client.h>
#include <filesystem>

namespace {
std::wstring FontTextFromUtf8(const char* text) {
    const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, nullptr, 0);
    if (length <= 1) return L"";
    std::wstring wide((size_t)length, L'\0');
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text, -1, wide.data(), length)) return L"";
    wide.resize((size_t)length - 1);
    return wide;
}

bool EditFontColor(const char* label, std::uint32_t& argb) {
    float color[4] = { ((argb >> 16) & 255) / 255.0f, ((argb >> 8) & 255) / 255.0f,
        (argb & 255) / 255.0f, (argb >> 24) / 255.0f };
    if (!ImGui::ColorEdit4(label, color, ImGuiColorEditFlags_AlphaPreviewHalf)) return false;
    const auto byte = [](float value) { return (std::uint32_t)(value * 255.0f + 0.5f); };
    argb = (byte(color[3]) << 24) | (byte(color[0]) << 16) | (byte(color[1]) << 8) | byte(color[2]);
    return true;
}
} // namespace

bool ReadSEFontImage(IDirect3DTexture9* texture, int x, int y, int width, int height,
    SEFontAtlasBitmap& output, std::string& error) {
    output = SEFontAtlasBitmap();
    D3DSURFACE_DESC description = {};
    if (!texture || FAILED(texture->GetLevelDesc(0, &description))) {
        error = "The font image could not be read."; return false;
    }
    if (width == -1) width = (int)description.Width - x;
    if (height == -1) height = (int)description.Height - y;
    if (x < 0 || y < 0 || width < 1 || height < 1 || width > 4096 || height > 4096 ||
        (std::uint64_t)x + width > description.Width || (std::uint64_t)y + height > description.Height) {
        error = "Choose a font crop inside the image, no larger than 4096 x 4096."; return false;
    }
    Microsoft::WRL::ComPtr<IDirect3DTexture9> converted;
    Microsoft::WRL::ComPtr<IDirect3DSurface9> source, destination;
    RECT crop = { x, y, x + width, y + height };
    if (!g_pd3dDevice || FAILED(g_pd3dDevice->CreateTexture(width, height, 1, 0, D3DFMT_A8R8G8B8,
        D3DPOOL_MANAGED, converted.GetAddressOf(), nullptr)) ||
        FAILED(texture->GetSurfaceLevel(0, source.GetAddressOf())) ||
        FAILED(converted->GetSurfaceLevel(0, destination.GetAddressOf())) ||
        FAILED(D3DXLoadSurfaceFromSurface(destination.Get(), nullptr, nullptr, source.Get(), nullptr,
            &crop, D3DX_FILTER_NONE, 0))) {
        error = "The font crop could not be converted to RGBA pixels."; return false;
    }
    try { output.pixels.resize((size_t)width * height); }
    catch (const std::bad_alloc&) { error = "Not enough memory for this font image."; return false; }
    D3DLOCKED_RECT pixels = {};
    if (FAILED(converted->LockRect(0, &pixels, nullptr, D3DLOCK_READONLY))) {
        error = "The font pixel buffer could not be locked."; output = SEFontAtlasBitmap(); return false;
    }
    for (int row = 0; row < height; ++row)
        memcpy(output.pixels.data() + (size_t)row * width,
            (const unsigned char*)pixels.pBits + (size_t)row * pixels.Pitch, (size_t)width * 4);
    converted->UnlockRect(0);
    output.width = width;
    output.height = height;
    return true;
}

void WORKSPACE::drawSimpleModeFontTools(const SESimpleModeSlot& slot) {
    if (slot.category != SESimpleModeCategory::NumberFonts &&
        slot.category != SESimpleModeCategory::JudgementFonts) return;
    // The projection can rebuild after another control edits the document.
    const SESimpleModeSlot target = slot;
    if (!ImGui::CollapsingHeader(SEText("Generate from TTF###SimpleFontTools",
        u8"TTF\uB85C \uD3F0\uD2B8 \uB9CC\uB4E4\uAE30###SimpleFontTools"), ImGuiTreeNodeFlags_DefaultOpen)) return;

    if (simpleFontSlotId != target.id) {
        simpleFontSlotId = target.id;
        simpleFontPreparedGeneration = 0;
        simpleFontSettings.cellWidth = (std::max)(8, (std::min)(1024,
            target.width > 0 ? target.width / target.divX : 32));
        simpleFontSettings.cellHeight = (std::max)(8, (std::min)(1024,
            target.height > 0 ? target.height / target.divY : 48));
        // LR2 indexes: miss-poor, poor, bad, good, great, perfect-great.
        const char* labels[] = { "MISS", "POOR", "BAD", "GOOD", "GREAT", "PGREAT" };
        snprintf(simpleFontJudgementText, sizeof(simpleFontJudgementText), "%s",
            target.sourceIndex >= 0 && target.sourceIndex < 6 ? labels[target.sourceIndex] : "JUDGE");
    }
    simpleFontSettings.isNumber = target.category == SESimpleModeCategory::NumberFonts;
    simpleFontSettings.columns = target.divX;
    simpleFontSettings.rows = target.divY;
    ImGui::TextWrapped("%s", SEText("Choose a font, generate a preview, then apply. Existing placement stays unchanged.",
        u8"\uAE00\uAF34\uC744 \uACE0\uB974\uACE0 \uBBF8\uB9AC\uBCF4\uAE30\uB97C \uB9CC\uB4E0 \uB4A4 \uC801\uC6A9\uD558\uC138\uC694. \uAE30\uC874 \uBC30\uCE58\uB294 \uC720\uC9C0\uB429\uB2C8\uB2E4."));
    if (ImGui::Button(SEText("Choose font file...", u8"\uAE00\uAF34 \uD30C\uC77C \uC120\uD0DD..."))) {
        wchar_t fontPath[32768] = {};
        OPENFILENAMEW dialog = {};
        dialog.lStructSize = sizeof(dialog);
        dialog.hwndOwner = GetActiveWindow();
        dialog.lpstrFilter = L"TrueType / OpenType (*.ttf;*.otf;*.ttc)\0*.ttf;*.otf;*.ttc\0\0";
        dialog.lpstrFile = fontPath;
        dialog.nMaxFile = (DWORD)(sizeof(fontPath) / sizeof(fontPath[0]));
        dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
        if (GetOpenFileNameW(&dialog)) simpleFontSettings.fontPath = fontPath;
        else if (CommDlgExtendedError()) {
            simpleModeStatus = "The font file picker could not be opened.";
            simpleModeStatusState = -1;
        }
    }
    if (!simpleFontSettings.fontPath.empty()) {
        const auto filename = std::filesystem::path(simpleFontSettings.fontPath).filename().u8string();
        ImGui::TextWrapped("%s", reinterpret_cast<const char*>(filename.c_str()));
    }
    if (!simpleFontSettings.isNumber)
        ImGui::InputText(SEText("Judgement text", u8"\uD310\uC815 \uBB38\uAD6C"),
            simpleFontJudgementText, sizeof(simpleFontJudgementText));
    simpleFontSettings.judgementText = FontTextFromUtf8(simpleFontJudgementText);

    ImGui::SetNextItemWidth(120.0f);
    ImGui::InputInt(SEText("Cell width (px)", u8"\uAE00\uC790 \uCE78 \uB108\uBE44 (px)"), &simpleFontSettings.cellWidth);
    ImGui::SetNextItemWidth(120.0f);
    ImGui::InputInt(SEText("Cell height (px)", u8"\uAE00\uC790 \uCE78 \uB192\uC774 (px)"), &simpleFontSettings.cellHeight);
    ImGui::SliderInt(SEText("Padding", u8"\uC5EC\uBC31"), &simpleFontSettings.padding, 0, 32);
    ImGui::SliderInt(SEText("Outline", u8"\uD14C\uB450\uB9AC"), &simpleFontSettings.outlinePixels, 0, 8);
    ImGui::SliderInt(SEText("Shadow", u8"\uADF8\uB9BC\uC790"), &simpleFontSettings.shadowPixels, 0, 8);
    EditFontColor(SEText("Text color", u8"\uAE00\uC790 \uC0C9"), simpleFontSettings.fillArgb);
    EditFontColor(SEText("Outline color", u8"\uD14C\uB450\uB9AC \uC0C9"), simpleFontSettings.outlineArgb);
    ImGui::TextDisabled("Atlas %d x %d cells | cycle %d ms", target.divX, target.divY, target.cycle);
    if (target.cycle > 0)
        ImGui::TextWrapped("%s", SEText("Generated frames repeat static text; the original animation timing is retained.",
            u8"\uC0DD\uC131\uD55C \uAE00\uC790\uB97C \uAC01 \uD504\uB808\uC784\uC5D0 \uBC18\uBCF5\uD569\uB2C8\uB2E4. \uAE30\uC874 \uC560\uB2C8\uBA54\uC774\uC158 \uC2DC\uAC04\uC740 \uC720\uC9C0\uB429\uB2C8\uB2E4."));

    if (ImGui::Button(SEText("Generate preview", u8"\uBBF8\uB9AC\uBCF4\uAE30 \uC0DD\uC131"))) {
        simpleFontPreparedGeneration = 0;
        SEFontAtlasBitmap raster;
        if (RenderSEFontAtlas(simpleFontSettings, raster, simpleModeStatus)) {
            PDIRECT3DTEXTURE9 texture = nullptr;
            if (LoadTextureFromRawMemory(raster.pixels.data(), raster.pixels.size() * sizeof(std::uint32_t),
                raster.width, raster.height, &texture)) {
                simpleFontTexture.reset(texture, [](IDirect3DTexture9* owned) { owned->Release(); });
                simpleFontBitmap = std::move(raster);
                simpleFontPreparedRequest = simpleFontSettings;
                simpleFontPreparedGeneration = simpleModeProjectionGeneration;
                simpleModeStatus = "Preview ready. Review the target list before applying.";
                simpleModeStatusState = 1;
            } else {
                if (texture) texture->Release();
                simpleModeStatus = "The font preview texture could not be created. Reduce the cell size.";
                simpleModeStatusState = -1;
            }
        } else simpleModeStatusState = -1;
    }
    ImGui::Checkbox(SEText("Also replace matching 1P / 2P", u8"\uB300\uC751\uD558\uB294 1P / 2P\uB3C4 \uAD50\uCCB4"), &simpleFontApplyToPair);
    const int scope = simpleFontApplyToPair ? 1 : 0;
    const auto targets = GetSimpleModeApplyTargets(target.id, scope);
    bool matchingGrids = true;
    ImGui::Text(SEText("Apply to %d component(s):", u8"%d\uAC1C \uB300\uC0C1\uC5D0 \uC801\uC6A9:"), (int)targets.size());
    for (const SESimpleModeSlot& affected : targets) {
        ImGui::BulletText("%s | index %d | branch %d | row %d",
            affected.command.c_str(), affected.sourceIndex, affected.ifgroup, affected.row + 1);
        matchingGrids &= affected.divX == target.divX && affected.divY == target.divY;
    }
    const bool hasCurrentPreview = simpleFontTexture && simpleFontPreparedGeneration == simpleModeProjectionGeneration &&
        simpleFontPreparedGeneration != 0 && simpleFontSettings == simpleFontPreparedRequest;
    if (!matchingGrids) ImGui::TextWrapped("%s", SEText("The player pair uses different grids. Apply separately.",
        u8"1P/2P\uC758 \uBD84\uD560 \uAD6C\uC870\uAC00 \uB2E4\uB985\uB2C8\uB2E4. \uAC01\uAC01 \uC801\uC6A9\uD558\uC138\uC694."));
    if (SEUI::ActionButton(SEText("Apply generated font", u8"\uC0DD\uC131\uD55C \uD3F0\uD2B8 \uC801\uC6A9"),
        SEText("Writes a new PNG. Undo restores all affected source rows.",
            u8"\uC0C8 PNG\uB97C \uC800\uC7A5\uD569\uB2C8\uB2E4. \uC2E4\uD589 \uCDE8\uC18C\uB85C \uC774\uC804 \uC0C1\uD0DC\uB97C \uBCF5\uC6D0\uD569\uB2C8\uB2E4."),
        hasCurrentPreview && matchingGrids && !targets.empty())) {
        simpleModeStatusState = ApplySimpleModeFontBitmap(target.id, simpleFontBitmap, scope, simpleModeStatus) == 0 ? 1 : -1;
        simpleFontPreparedGeneration = 0;
    }
    if (simpleFontTexture) {
        const float scale = (std::min)(1.0f, (std::min)(ImGui::GetContentRegionAvail().x / simpleFontBitmap.width,
            180.0f / simpleFontBitmap.height));
        ImGui::Image(simpleFontTexture.get(), ImVec2(simpleFontBitmap.width * scale, simpleFontBitmap.height * scale));
        if (!hasCurrentPreview) ImGui::TextWrapped("%s", SEText("Settings or target changed. Generate the preview again before applying.",
            u8"\uC124\uC815 \uB610\uB294 \uB300\uC0C1\uC774 \uBC14\uB00C\uC5C8\uC2B5\uB2C8\uB2E4. \uBBF8\uB9AC\uBCF4\uAE30\uB97C \uB2E4\uC2DC \uC0DD\uC131\uD558\uC138\uC694."));
    }
    ImGui::Separator();
}
