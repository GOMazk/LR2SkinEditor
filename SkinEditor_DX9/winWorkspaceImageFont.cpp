#include "winWorkspace.h"
#include "uiCatalog.h"
#include "seUI.h"
#include "inputwrap.h"
#include "skinPathResolver.h"
#include "../LR2/LR2_skinload.h"
#include "../LR2/En_fileutil.h"
#include "../lib/DxLib/DxLib.h"
#include <commdlg.h>
#include <filesystem>
#include <cmath>
#include <climits>

namespace {
size_t LastPathSeparator(const std::string& path) {
    size_t last = std::string::npos;
    for (size_t i = 0; i < path.size(); ++i) {
        if (IsDBCSLeadByteEx(932, (BYTE)path[i])) { ++i; continue; }
        if (path[i] == '/' || path[i] == '\\') last = i;
    }
    return last;
}

std::string FindFontInPattern(const std::string& pattern) {
    const size_t slash = LastPathSeparator(pattern);
    const std::string directory = slash == std::string::npos ? "" : pattern.substr(0, slash + 1);
    // Keep the existing directory-wildcard choice behavior. Once a directory
    // has been selected, file lookup must still go through DxLib for DXA leaves.
    if (directory.find_first_of("*?") != std::string::npos) {
        CSTR selected = GetRandomFileNoError(CSTR(pattern.c_str()), CSTR(""));
        if (selected.isSame("ERROR") || selected.findStrPos("*") >= 0 || selected.findStrPos("?") >= 0) return "";
        return FindFontInPattern(selected.outstr());
    }
    FILEINFO info{};
    const DWORD_PTR handle = DxLib::FileRead_findFirst(pattern.c_str(), &info);
    if (handle == (DWORD_PTR)-1) return "";
    struct CloseSearch { DWORD_PTR handle; ~CloseSearch() { DxLib::FileRead_findClose(handle); } } close{handle};
    // Reservoir selection keeps RANDOM behavior without storing all matches.
    std::string chosen;
    int count = 0;
    do {
        if (!info.DirFlag && *info.Name && ++count <= 65536 &&
            (count == 1 || DxLib::GetRand(count - 1) == 0)) chosen = directory + info.Name;
    } while (count < 65536 && DxLib::FileRead_findNext(handle, &info) == 0);
    return chosen;
}

void PointSampler(const ImDrawList*, const ImDrawCmd*) {
    if (!g_pd3dDevice) return;
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
}
std::string ChooseFontFile(std::string& error) {
    wchar_t path[32768] = {};
    OPENFILENAMEW dialog{};
    dialog.lStructSize = sizeof(dialog); dialog.hwndOwner = GetActiveWindow();
    dialog.lpstrFilter = L"LR2 image font / DXA (*.lr2font;*.dxa)\0*.lr2font;*.dxa\0\0";
    dialog.lpstrFile = path; dialog.nMaxFile = 32768;
    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if (!GetOpenFileNameW(&dialog)) return "";
    char utf8[32768 * 3] = {};
    std::string result;
    if (!WideCharToMultiByte(CP_UTF8, 0, path, -1, utf8, sizeof(utf8), nullptr, nullptr) ||
        !SEImageFontEncode(utf8, result)) error = "The font path must be representable in CP932 for LR2.";
    return result;
}
SEImageFontTexture* Texture(SEImageFontEditor& state, int page) {
    const std::string path = SEImageFontPagePath(state.document, page);
    if (path.empty()) return nullptr;
    auto found = state.textures.find(page);
    if (found != state.textures.end()) return &found->second;
    // Bounded lazy cache: do not load thousands of font pages at once. Reload
    // images clears it explicitly; COM ownership lasts through frame rendering.
    if (state.textures.size() >= 16) return nullptr;
    auto& image = state.textures[page]; image.path = path;
    std::string bytes, error;
    D3DXIMAGE_INFO info{};
    if (g_pd3dDevice && SEReadFontResource(path, 64*1024*1024, bytes, error) &&
        SUCCEEDED(D3DXGetImageInfoFromFileInMemory(bytes.data(), (UINT)bytes.size(), &info)) &&
        info.Width > 0 && info.Height > 0 && (uint64_t)info.Width * info.Height <= 4 * 1024 * 1024) {
        IDirect3DTexture9* texture = nullptr;
        if (LoadTextureFromMemory(bytes.data(), bytes.size(), &texture, &image.width, &image.height))
            image.texture.reset(texture, [](IDirect3DTexture9* owned) { owned->Release(); });
    }
    return &image;
}
bool InBounds(const SEImageFontGlyph& g, const SEImageFontTexture* image) {
    return image && image->texture && g.x >= 0 && g.y >= 0 && g.w > 0 && g.h > 0 &&
        (long long)g.x + g.w <= image->width && (long long)g.y + g.h <= image->height;
}
void GlyphImage(ImDrawList* draw, const SEImageFontGlyph& g, SEImageFontTexture& image,
    ImVec2 position, float zoom) {
    draw->AddImage((ImTextureID)image.texture.get(), position,
        ImVec2(position.x + g.w * zoom, position.y + g.h * zoom),
        ImVec2((float)g.x / image.width, (float)g.y / image.height),
        ImVec2((float)(g.x + g.w) / image.width, (float)(g.y + g.h) / image.height));
}
void Sample(SEImageFontEditor& state) {
    ImGui::InputText("Sample text", state.sample, sizeof(state.sample));
    ImGui::SetNextItemWidth(150);
    ImGui::SliderFloat("Text zoom", &state.sampleZoom, 0.25f, 4.0f, "%.2fx");
    ImGui::BeginChild("##fontSample", ImVec2(0, 100), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
    std::string encoded;
    if (!SEImageFontEncode(state.sample, encoded)) ImGui::TextUnformatted("This text contains characters not supported by LR2 / CP932.");
    else {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        draw->AddCallback(PointSampler, nullptr);
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        float x = 0, height = (float)state.document.size * state.sampleZoom;
        int missing = 0;
        for (size_t i = 0; i < encoded.size();) {
            const size_t length = IsDBCSLeadByteEx(932, (BYTE)encoded[i]) ? 2 : 1;
            const std::string character = Cp932ToUtf8(encoded.substr(i, length).c_str()); i += length;
            int code = 0;
            if (!SEImageFontCode(character.c_str(), code)) { ++missing; continue; }
            auto glyph = state.document.glyphs.find(code);
            if (glyph == state.document.glyphs.end()) { ++missing; glyph = state.document.glyphs.find(63); }
            if (glyph == state.document.glyphs.end()) { x += height * 0.5f; continue; }
            const auto& g = glyph->second;
            SEImageFontTexture* image = Texture(state, g.page);
            if (code != 32 && code != 9 && code != 10 && InBounds(g, image))
                GlyphImage(draw, g, *image, ImVec2(origin.x + x, origin.y), state.sampleZoom);
            else if (code != 32 && code != 9 && code != 10) ++missing;
            x += (g.w + state.document.margin) * state.sampleZoom;
            height = (std::max)(height, g.h * state.sampleZoom);
        }
        draw->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
        ImGui::Dummy(ImVec2((std::max)(1.0f, x), height));
        if (missing) ImGui::TextDisabled("%d missing/unavailable glyph(s)", missing);
    }
    ImGui::EndChild();
}
}

std::string SEFindPreviewImageFontPath(const char* requestedPath,
    const char* ownerFilePath, const char* mainSkinPath) {
    if (!requestedPath || !*requestedPath || !_stricmp(requestedPath, "ERROR")) return "";
    std::string resolved;
    const auto resolution = SEResolveSkinResourcePath(requestedPath, ownerFilePath, mainSkinPath, resolved);
    if (resolution == SESkinResourcePathResult::Rejected) return "";
    if (resolution == SESkinResourcePathResult::Resolved) return FindFontInPattern(resolved);
    // A relative virtual DXA leaf does not exist to Win32's PatternExists.
    // Try the owning include, then the main skin, then the existing CWD fallback.
    for (const char* file : {ownerFilePath, mainSkinPath, ""}) {
        const std::string owner = file ? file : "";
        const size_t slash = LastPathSeparator(owner);
        const std::string prefix = slash == std::string::npos ? "" : owner.substr(0, slash + 1);
        const std::string found = FindFontInPattern(prefix + requestedPath);
        if (!found.empty()) return found;
    }
    return "";
}

void SEImageFontEditor::SelectPage(int page) {
    if (selectedPage != page) textures.clear();
    selectedPage = page; pageIndex = page;
    const auto found = document.pages.find(page);
    snprintf(pagePath, sizeof(pagePath), "%s", found == document.pages.end() ? "" : Cp932ToUtf8(found->second.path.c_str()).c_str());
}
void SEImageFontEditor::SelectGlyph(int code) {
    selectedCode = code;
    auto found = document.glyphs.find(code);
    if (found != document.glyphs.end()) {
        glyph = found->second; SelectPage(glyph.page);
        const std::string label = SEImageFontCharacter(code);
        snprintf(character, sizeof(character), "%s", code == 9 || code == 10 || code == 32 ? "" : label.c_str());
    }
}
void SEImageFontEditor::RefreshFields() {
    height = document.size; margin = document.margin;
    if (!document.glyphs.count(selectedCode)) selectedCode = document.glyphs.empty() ? -1 : document.glyphs.begin()->first;
    if (selectedCode >= 0) SelectGlyph(selectedCode);
    else if (!document.pages.empty()) SelectPage(document.pages.begin()->first);
    fieldsDirty = metricsDirty = pageDirty = glyphDirty = false;
    characterInvalid = false;
}
bool SEImageFontEditor::ApplyFields() {
    if (!fieldsDirty) return true;
    if (characterInvalid) { status = "Enter one CP932 character, or correct LR2 code before applying."; return false; }
    // Validate every changed form before replacing the live draft.
    SEImageFontDocument edited = document;
    if (metricsDirty && !edited.SetMetrics(height, margin, status)) return false;
    if (pageDirty) {
        std::string path;
        if (!SEImageFontEncode(pagePath, path)) { status = "Texture path cannot be encoded in CP932."; return false; }
        if (!edited.SetPage(pageIndex, path, status)) return false;
    }
    if (glyphDirty && !edited.SetGlyph(selectedCode, glyph, status)) return false;
    const int pageAfter = pageDirty ? pageIndex : glyphDirty ? glyph.page : selectedPage;
    document = std::move(edited);
    if (glyphDirty) selectedCode = glyph.code;
    if (pageDirty) textures.clear();
    RefreshFields();
    if (document.pages.count(pageAfter)) SelectPage(pageAfter);
    status = "Draft updated. Save font writes this .lr2font file, not the skin CSV.";
    return true;
}
bool WORKSPACE::OpenImageFont(const std::string& path) {
    SEImageFontDocument document;
    if (!document.Open(path, imageFontEditor.status)) return false;
    imageFontEditor = SEImageFontEditor();
    imageFontEditor.document = std::move(document);
    imageFontEditor.RefreshFields();
    ReloadRuntimeImageFont(imageFontEditor.document.path);
    wImageFontEditor = true;
    return true;
}
void WORKSPACE::ReloadRuntimeImageFont(const std::string& path) {
    for (int i = 0; i < 10; ++i) {
        if (imageFontRuntimePaths[i].empty() ||
            _stricmp(std::filesystem::absolute(imageFontRuntimePaths[i]).lexically_normal().string().c_str(), path.c_str())) continue;
        ImageFont& font = g.skstruct.ImageFonts[i];
        if (!font.chars || !font.images) continue;
        // InitImageFont alone leaves old rectangle definitions behind. Clear
        // removed glyphs too, and invalidate directory-only legacy font caching.
        for (int c = 0; c < 0x3bce; ++c) {
            if (font.chars[c].grHandle >= 0) DxLib::DeleteGraph(font.chars[c].grHandle);
            font.chars[c] = {}; font.chars[c].grHandle = -1;
        }
        for (int p = 0; p < 1000; ++p) {
            if (font.images[p].grHandle >= 0) DxLib::DeleteGraph(font.images[p].grHandle);
            font.images[p] = {}; font.images[p].grHandle = -1;
        }
        font.size = font.kerning = 0; font.filepath[0] = 0;
        ReadImageFont(CSTR(path.c_str()), &font);
    }
    previewLastRenderAt = 0;
    if (loaded) RefreshPreviewSelectionBounds();
}
bool WORKSPACE::SaveImageFont() {
    auto& editor = imageFontEditor;
    if (!editor.ApplyFields() || !editor.document.Save(editor.status)) return false;
    const std::string warning = editor.status;
    ReloadRuntimeImageFont(editor.document.path);
    editor.status = "Font saved. Matching fonts in this Workspace were reloaded.";
    if (!warning.empty()) editor.status += "\n" + warning;
    return true;
}

void WORKSPACE::drawImageFontEditor() {
    auto& e = imageFontEditor;
    char title[128];
    FormatSEUIWindowTitle(title, sizeof(title), SEUIWindowId::ImageFontEditor, num);
    if (imageFontRevealRequested) {
        SEUI::RevealWindowTab(title);
        imageFontRevealRequested = false;
    }
    ImGui::SetNextWindowSize(ImVec2(900, 700), ImGuiCond_FirstUseEver);
    const bool visible = ImGui::Begin(title, &wImageFontEditor, e.Dirty() ? ImGuiWindowFlags_UnsavedDocument : 0);
    e.focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    if (!visible) { ImGui::End(); return; }
    auto requestOpen = [&](const std::string& path) {
        if (path.empty()) return;
        const std::string key = SEFontPathKey(path);
        if (key.size() >= 4 && key.compare(key.size()-4,4,".dxa") == 0) {
            SEImageFontArchive archive;
            if (!archive.Open(path,false,e.status)) return;
            e.archiveChoices=archive.Fonts(); e.archiveChoicePath=path;
            if (e.archiveChoices.empty()) { e.status="This DXA contains no .lr2font definitions."; return; }
            if (e.archiveChoices.size() > 1) { e.chooseArchiveMember=true; return; }
            e.pendingPath=SEImageFontArchiveVirtualPath(path,e.archiveChoices.front());
        } else e.pendingPath = path;
        e.pendingOpen = true;
    };
    auto canSelect = [&]() {
        if (!e.fieldsDirty) return true;
        e.status = "Apply edits or Revert fields before selecting another character/page."; return false;
    };
    auto undo = [&]() { if (e.fieldsDirty) e.RefreshFields(); else { e.document.Undo(); e.RefreshFields(); } e.textures.clear(); };
    auto redo = [&]() { if (!e.fieldsDirty) { e.document.Redo(); e.RefreshFields(); e.textures.clear(); } };
    if (e.focused && ImGui::GetIO().KeyCtrl && !ImGui::GetIO().WantTextInput) {
        if (ImGui::IsKeyPressed(ImGuiKey_S, false)) SaveImageFont();
        if (ImGui::IsKeyPressed(ImGuiKey_Z, false)) { if (ImGui::GetIO().KeyShift) redo(); else undo(); }
        if (ImGui::IsKeyPressed(ImGuiKey_Y, false)) redo();
    }
    if (ImGui::Button("Open font / DXA...")) requestOpen(ChooseFontFile(e.status));
    ImGui::SameLine();
    ImGui::SetNextItemWidth((std::max)(120.0f, ImGui::GetContentRegionAvail().x));
    if (ImGui::BeginCombo("##registeredFonts", "Fonts in current Preview")) {
        bool any = false;
        for (int i = 0; i < 10; ++i) if (!imageFontRuntimePaths[i].empty()) {
            any = true;
            const std::string& path = imageFontRuntimePaths[i];
            const size_t slash = LastPathSeparator(path);
            const size_t parent = slash == std::string::npos ? std::string::npos : LastPathSeparator(path.substr(0, slash));
            const std::string shortPath = parent == std::string::npos ? path : path.substr(parent + 1);
            const std::string label = std::to_string(i) + " : " + Cp932ToUtf8(shortPath.c_str());
            ImGui::PushID(i);
            if (ImGui::Selectable(label.c_str())) requestOpen(path);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", Cp932ToUtf8(path.c_str()).c_str());
            ImGui::PopID();
        }
        if (!any) ImGui::TextDisabled("No active #LR2FONT declarations.");
        ImGui::EndCombo();
    }
    if (e.chooseArchiveMember) { e.chooseArchiveMember=false; ImGui::OpenPopup("Choose font in DXA"); }
    ImGui::SetNextWindowSize(ImVec2(520, 320), ImGuiCond_FirstUseEver);
    if (ImGui::BeginPopupModal("Choose font in DXA", nullptr)) {
        ImGui::TextWrapped("%s", Cp932ToUtf8(e.archiveChoicePath.c_str()).c_str());
        ImGui::BeginChild("##dxaFonts", ImVec2(0,-ImGui::GetFrameHeightWithSpacing()));
        for (const auto& member : e.archiveChoices) if (ImGui::Selectable(Cp932ToUtf8(member.c_str()).c_str())) {
            e.pendingPath=SEImageFontArchiveVirtualPath(e.archiveChoicePath,member);
            e.pendingOpen=true; ImGui::CloseCurrentPopup();
        }
        ImGui::EndChild();
        if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (e.pendingOpen) {
        e.pendingOpen = false;
        if (e.Dirty()) ImGui::OpenPopup("Unsaved font changes");
        else { const std::string path = e.pendingPath; OpenImageFont(path); }
    }
    if (ImGui::BeginPopupModal("Unsaved font changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Save or discard font edits before opening/reloading another file?");
        if (ImGui::Button("Save and open")) {
            const std::string target = e.pendingPath;
            if (SaveImageFont() && OpenImageFont(target)) ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Discard and open")) {
            const std::string target = e.pendingPath;
            if (OpenImageFont(target)) ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
        if (!e.status.empty()) ImGui::TextWrapped("%s", e.status.c_str());
        ImGui::EndPopup();
    }
    if (e.document.path.empty()) {
        SEUI::EmptyState("No image font open", "Choose an active #LR2FONT, .lr2font file or legacy .dxa archive. Number/judgement atlases remain in Simple Mode.");
        if (!e.status.empty()) ImGui::TextWrapped("%s", e.status.c_str());
        ImGui::End(); return;
    }
    ImGui::TextWrapped("%s", Cp932ToUtf8(e.document.path.c_str()).c_str());
    if (e.document.archive) {
        ImGui::TextDisabled("DXA v%d: Save font updates the definition inside the archive.", e.document.archive->version);
        SEUI::HelpMarker("Other members and the archive version are preserved. No extraction or skin CSV rewrite. Custom passwords / DXA versions 5+ are not editable.");
    }
    // Wrap toolbar buttons rather than requiring a horizontal window scroll.
    auto button = [&](const char* label, bool enabled) {
        const float width = ImGui::CalcTextSize(label).x + ImGui::GetStyle().FramePadding.x * 2;
        if (ImGui::GetCursorPosX() > ImGui::GetStyle().WindowPadding.x && ImGui::GetContentRegionAvail().x < width) ImGui::NewLine();
        ImGui::BeginDisabled(!enabled); const bool pressed = ImGui::Button(label); ImGui::EndDisabled(); ImGui::SameLine(); return pressed;
    };
    if (button("Apply edits", e.fieldsDirty)) e.ApplyFields();
    if (button("Revert fields", e.fieldsDirty)) e.RefreshFields();
    if (button("Save font", e.Dirty())) SaveImageFont();
    if (button("Undo", e.fieldsDirty || !e.document.undo.empty())) undo();
    if (button("Redo", !e.fieldsDirty && !e.document.redo.empty())) redo();
    if (button("Reload", true)) requestOpen(e.document.path);
    if (button("Reload images", true)) {
        if (e.document.archive) DxLib::DXArchiveRelease(e.document.archive->path.c_str());
        e.textures.clear();
    }
    if (button("Back to Preview", loaded)) RequestPreview();
    ImGui::NewLine();
    if (!e.status.empty()) {
        ImGui::BeginChild("##fontStatus", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 2.5f));
        ImGui::TextWrapped("%s", e.status.c_str()); ImGui::EndChild();
    }
    const float leftWidth = (std::max)(120.0f, (std::min)(220.0f, ImGui::GetContentRegionAvail().x * 0.3f));
    ImGui::BeginChild("##fontCharacters", ImVec2(leftWidth, 0), ImGuiChildFlags_Borders);
    ImGui::Text("%d characters", (int)e.document.glyphs.size());
    ImGui::SetNextItemWidth(-1); ImGui::InputTextWithHint("##fontSearch", "Character / code", e.search, sizeof(e.search));
    if (ImGui::Button("New character") && canSelect()) {
        e.selectedCode = -1; e.glyph = {}; e.glyph.code = 32;
        while (e.document.glyphs.count(e.glyph.code) && e.glyph.code < 0x3bcd) ++e.glyph.code;
        e.glyph.page = (std::max)(0, e.selectedPage); e.glyph.w = e.glyph.h = e.document.size;
        e.character[0] = 0; e.fieldsDirty = e.glyphDirty = true;
    }
    ImGui::BeginChild("##fontCharacterList");
    std::vector<int> codes;
    for (const auto& entry : e.document.glyphs) {
        if (!*e.search || std::to_string(entry.first).find(e.search) != std::string::npos ||
            SEImageFontCharacter(entry.first).find(e.search) != std::string::npos) codes.push_back(entry.first);
    }
    ImGuiListClipper clipper; clipper.Begin((int)codes.size());
    while (clipper.Step()) for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index) {
        const int code = codes[index];
        const std::string label = std::to_string(code) + "  " + SEImageFontCharacter(code);
        ImGui::PushID(code);
        if (ImGui::Selectable(label.c_str(), e.selectedCode == code) && canSelect()) e.SelectGlyph(code);
        ImGui::PopID();
    }
    ImGui::EndChild(); ImGui::EndChild(); ImGui::SameLine();
    ImGui::BeginChild("##fontProperties", ImVec2(0, 0));
    if (ImGui::CollapsingHeader("Font and texture pages", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SetNextItemWidth(120);
        if (ImGui::InputInt("Height (#S)", &e.height)) e.fieldsDirty = e.metricsDirty = true;
        ImGui::SetNextItemWidth(120);
        if (ImGui::InputInt("Spacing (#M)", &e.margin)) e.fieldsDirty = e.metricsDirty = true;
        ImGui::SetNextItemWidth(-1);
        const std::string pageLabel = "Page " + std::to_string(e.selectedPage);
        if (ImGui::BeginCombo("##fontPages", pageLabel.c_str())) {
            for (const auto& page : e.document.pages) {
                std::string label = std::to_string(page.first) + " : " + Cp932ToUtf8(page.second.path.c_str());
                if (ImGui::Selectable(label.c_str(), page.first == e.selectedPage) && canSelect()) e.SelectPage(page.first);
            }
            ImGui::EndCombo();
        }
        ImGui::SetNextItemWidth(120);
        if (ImGui::InputInt("Page ID (#T)", &e.pageIndex)) e.fieldsDirty = e.pageDirty = true;
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputTextWithHint("##fontPagePath", "Image path relative to font", e.pagePath, sizeof(e.pagePath))) e.fieldsDirty = e.pageDirty = true;
        SEUI::HelpMarker("Changing Page ID adds/updates that #T page; existing pages are preserved. Image pixels are not modified by this window.");
    }
    if (ImGui::CollapsingHeader("Character rectangle (#R)", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SetNextItemWidth(120);
        if (ImGui::InputInt("LR2 code", &e.glyph.code)) {
            e.fieldsDirty = e.glyphDirty = true; e.characterInvalid = false;
            snprintf(e.character, sizeof(e.character), "%s", SEImageFontCharacter(e.glyph.code).c_str());
        }
        ImGui::SetNextItemWidth(120);
        if (ImGui::InputText("Character", e.character, sizeof(e.character))) {
            int code = 0;
            e.fieldsDirty = e.glyphDirty = true;
            e.characterInvalid = !SEImageFontCode(e.character, code);
            if (!e.characterInvalid) e.glyph.code = code;
            else e.status = "Enter exactly one CP932 character, or use LR2 code for control characters.";
        }
        SEUI::HelpMarker("LR2 code is not Unicode. Character converts a single CP932 character to LR2's index. Space/TAB/LF have codes 32/9/10.");
        ImGui::SetNextItemWidth(120);
        if (ImGui::InputInt("Texture page", &e.glyph.page)) e.fieldsDirty = e.glyphDirty = true;
        int rect[] = { e.glyph.x, e.glyph.y, e.glyph.w, e.glyph.h };
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputInt4("##fontRect", rect)) {
            e.glyph.x = rect[0]; e.glyph.y = rect[1]; e.glyph.w = rect[2]; e.glyph.h = rect[3];
            e.fieldsDirty = e.glyphDirty = true;
        }
        ImGui::TextDisabled("x / y / width / height (pixels)");
        ImGui::BeginDisabled(e.selectedCode < 0 || e.fieldsDirty);
        if (ImGui::Button("Remove character")) e.pendingDelete = true;
        ImGui::EndDisabled();
        if (e.pendingDelete) { ImGui::OpenPopup("Remove this character?"); e.pendingDelete = false; }
        if (ImGui::BeginPopupModal("Remove this character?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Remove LR2 code %d from the draft?", e.selectedCode);
            if (ImGui::Button("Remove")) { e.document.RemoveGlyph(e.selectedCode, e.status); e.RefreshFields(); ImGui::CloseCurrentPopup(); }
            ImGui::SameLine(); if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup(); ImGui::EndPopup();
        }
        auto* image = Texture(e, e.glyph.page);
        if (InBounds(e.glyph, image)) {
            const float scale = (std::min)(4.0f, 80.0f / (std::max)(e.glyph.w, e.glyph.h));
            ImDrawList* draw = ImGui::GetWindowDrawList(); draw->AddCallback(PointSampler, nullptr);
            GlyphImage(draw, e.glyph, *image, ImGui::GetCursorScreenPos(), scale);
            draw->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
            ImGui::Dummy(ImVec2(e.glyph.w * scale, e.glyph.h * scale));
        } else ImGui::TextDisabled("No pixels: empty rectangle, unavailable image or outside texture.");
    }
    Texture(e, e.selectedPage); // prioritize the selected page over sample text
    Sample(e);
    ImGui::SetNextItemWidth(140); ImGui::SliderFloat("Atlas zoom", &e.zoom, 0.1f, 16.0f, "%.2fx", ImGuiSliderFlags_Logarithmic);
    ImGui::SameLine(); if (ImGui::Button("100%")) e.zoom = 1.0f;
    ImGui::Checkbox("Show rectangles", &e.showBoxes);
    // Load the selected page first, even when sample text uses other pages.
    auto* page = Texture(e, e.selectedPage);
    ImGui::BeginChild("##fontAtlas", ImVec2(0, (std::max)(220.0f, ImGui::GetContentRegionAvail().y)), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
    if (page && page->texture) {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        draw->AddCallback(PointSampler, nullptr);
        ImGui::Image((ImTextureID)page->texture.get(), ImVec2(page->width * e.zoom, page->height * e.zoom));
        const bool hovered = ImGui::IsItemHovered();
        if (e.showBoxes) for (const auto& entry : e.document.glyphs) {
            const auto& g = entry.second;
            if (g.page != e.selectedPage || !InBounds(g, page)) continue;
            draw->AddRect(ImVec2(origin.x + g.x * e.zoom, origin.y + g.y * e.zoom),
                ImVec2(origin.x + (g.x + g.w) * e.zoom, origin.y + (g.y + g.h) * e.zoom),
                g.code == e.selectedCode ? IM_COL32(255, 150, 40, 255) : IM_COL32(100, 160, 240, 120));
        }
        draw->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && canSelect()) {
            const float x = (ImGui::GetIO().MousePos.x - origin.x) / e.zoom, y = (ImGui::GetIO().MousePos.y - origin.y) / e.zoom;
            int selected = -1; long long area = LLONG_MAX;
            for (const auto& entry : e.document.glyphs) {
                const auto& g = entry.second;
                if (g.page == e.selectedPage && x >= g.x && y >= g.y && x < g.x + g.w && y < g.y + g.h && (long long)g.w * g.h < area) {
                    selected = g.code; area = (long long)g.w * g.h;
                }
            }
            if (selected >= 0) e.SelectGlyph(selected);
        }
        if (hovered && ImGui::GetIO().KeyCtrl && ImGui::GetIO().MouseWheel != 0) {
            const ImVec2 mouse = ImGui::GetIO().MousePos;
            const float ratio = (std::clamp)(e.zoom * powf(1.15f, ImGui::GetIO().MouseWheel), 0.1f, 16.0f) / e.zoom;
            ImGui::SetScrollX(ImGui::GetScrollX() + (mouse.x - origin.x) * (ratio - 1));
            ImGui::SetScrollY(ImGui::GetScrollY() + (mouse.y - origin.y) * (ratio - 1));
            e.zoom *= ratio;
        }
    } else {
        ImGui::TextWrapped("No texture preview. Check the relative #T path and image file. Unpacked images up to 4 megapixels are supported; Reload images clears the 16-page cache.");
        ImGui::TextWrapped("%s", Cp932ToUtf8(SEImageFontPagePath(e.document, e.selectedPage).c_str()).c_str());
    }
    ImGui::EndChild(); ImGui::EndChild(); ImGui::End();
}
