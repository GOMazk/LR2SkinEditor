#include "imageFontDocument.h"
#include "winWorkspace.h"
#include "uiCatalog.h"
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <iterator>

int RunImageFontEditorSelfTest() {
    SEImageFontDocument doc;
    std::string error;
    const std::string original = "// CP932: \x83\x65\x83\x58\x83\x67\r\n#S,18,keep\r\n#M,-3,\n"
        "#T,0,font.png,extra\r\n#UNKNOWN,retain,\r\n#R,65,0,0,0,5,18,first\r\n"
        "#R,65,0,5,0,6,18,last\r\n#R,32,0,0,0,4,18,\r\n#R,330,0,11,0,8,18,";
    if (!doc.Parse(original, error) || doc.bytes != original || doc.glyphs.size() != 3 ||
        doc.glyphs.at(65).x != 5 || doc.margin != -3) return 1;
    doc.originalBytes = original;
    if (doc.Dirty() || !doc.SetMetrics(24, -2, error) || doc.bytes.find("#S,24,keep\r\n#M,-2,\n") == std::string::npos ||
        doc.bytes.find("#UNKNOWN,retain,") == std::string::npos || !doc.Dirty()) return 2;
    if (!doc.Undo() || doc.bytes != original || doc.Dirty() || !doc.Redo() || doc.size != 24) return 3;
    doc.Undo();
    auto glyph = doc.glyphs.at(65); glyph.code = 66; glyph.x = 20; glyph.w = 10;
    if (!doc.SetGlyph(65, glyph, error) || doc.glyphs.count(65) || doc.glyphs.at(66).x != 20 ||
        doc.bytes.find("10,18,last") == std::string::npos) return 4;
    if (!doc.RemoveGlyph(66, error) || doc.glyphs.count(66) || !doc.Undo() || !doc.glyphs.count(66)) return 5;
    doc.Undo();
    const std::string before = doc.bytes;
    glyph.code = 32;
    if (doc.SetGlyph(65, glyph, error) || doc.bytes != before) return 6;
    glyph.code = 0x3bce;
    if (doc.SetGlyph(65, glyph, error) || doc.bytes != before) return 7;
    glyph.code = 67; glyph.page = 999;
    if (doc.SetGlyph(-1, glyph, error) || doc.bytes != before) return 8;
    if (!doc.SetPage(999, "pages/last.png", error)) return 9;
    if (!doc.SetGlyph(-1, glyph, error) || !doc.glyphs.count(67)) return 10;
    const std::string valid = doc.bytes;
    if (doc.SetPage(1000, "bad.png", error) || doc.SetPage(1, "bad,comma.png", error) ||
        doc.SetPage(1, "C:\\outside.png", error) || doc.SetMetrics(0, 0, error) || doc.bytes != valid) return 11;
    for (const char* invalid : {"#S,18\n#R,-1,0,0,0,1,1", "#S,18\n#T,1000,x.png",
        "#S,18\n#R,1,0,2147483647,0,2,2", "#S,18\n#R,abc,0,0,0,1,1", "#M,3"}) {
        if (doc.Parse(invalid, error) || doc.bytes != valid) return 12;
    }
    int code = 0;
    if (!SEImageFontCode("A", code) || code != 65 || SEImageFontCharacter(code) != "A") return 13;
    if (!SEImageFontCode("\xe3\x81\x82", code) || code != 608 || SEImageFontCharacter(code) != "\xe3\x81\x82") return 14;
    if (!SEImageFontCode("\xef\xbd\xb1", code) || code != 0xb1 || SEImageFontCharacter(code) != "\xef\xbd\xb1") return 15;
    if (SEImageFontCode("AB", code) || SEImageFontCode("\xf0\x9f\x99\x82", code) || SEImageFontCode("", code)) return 16;
    for (int i = 33; i < 0x3bce; ++i) {
        const std::string label = SEImageFontCharacter(i);
        if (label.empty()) continue;
        if (!SEImageFontCode(label.c_str(), code)) return 17;
        // CP932 duplicate Unicode mappings may encode to a canonical alias.
        if (SEImageFontCharacter(code) != label) return 18;
    }

    namespace fs = std::filesystem;
    char temp[MAX_PATH] = {}, root[MAX_PATH] = {};
    if (!GetTempPathA(MAX_PATH, temp) || !GetTempFileNameA(temp, "sef", 0, root) ||
        !DeleteFileA(root) || !CreateDirectoryA(root, nullptr)) return 20;
    struct Cleanup { fs::path path; ~Cleanup() { std::error_code ec; fs::remove_all(path, ec); } } cleanup{fs::path(root)};
    const std::string path = (cleanup.path / "font.lr2font").string();
    const auto write = [](const std::string& file, const std::string& bytes) {
        std::ofstream out(file, std::ios::binary); out << bytes; out.close(); return !!out;
    };
    const auto read = [](const std::string& file) { std::ifstream in(file, std::ios::binary); return std::string(std::istreambuf_iterator<char>(in), {}); };
    if (!write(path, original) || !doc.Open(path, error) || doc.Dirty()) return 21;
    if (SEFontPathKey(SEFindPreviewImageFontPath(path.c_str(), "", "")) != SEFontPathKey(path)) return 41;
    const std::string wildcardPath = (cleanup.path / "*.lr2font").string();
    if (SEFontPathKey(SEFindPreviewImageFontPath(wildcardPath.c_str(), "", "")) != SEFontPathKey(path)) return 42;
    if (SEImageFontPagePath(doc, 0) != (cleanup.path / "font.png").string()) return 22;
    if (!doc.SetMetrics(24, -2, error) || !doc.Save(error) || doc.Dirty() || read(path) != doc.bytes ||
        fs::exists(path + ".skineditor.bak") || fs::exists(path + ".skineditor.tmp")) return 23;
    SEImageFontDocument reloaded;
    if (!reloaded.Open(path, error) || reloaded.size != 24 || reloaded.bytes.find("#UNKNOWN,retain,") == std::string::npos) return 24;
    if (!doc.SetMetrics(30, 1, error) || !write(path, original) || doc.Save(error) || !doc.Dirty() ||
        read(path) != original || error.find("changed outside") == std::string::npos) return 25;
    if (!doc.Open(path, error) || !doc.SetMetrics(30, 0, error) || !write(path + ".skineditor.bak", "recover") ||
        doc.Save(error) || read(path + ".skineditor.bak") != "recover" || read(path) != original) return 26;
    if (!DeleteFileA((path + ".skineditor.bak").c_str())) return 27;
    HANDLE lock = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (lock == INVALID_HANDLE_VALUE) return 28;
    const bool savedLocked = doc.Save(error); CloseHandle(lock);
    if (savedLocked || !doc.Dirty() || read(path) != original) return 29;
    if (!doc.Save(error) || read(path) != doc.bytes) return 30;
    const std::string saved = doc.bytes;
    if (!doc.Undo() || !doc.Dirty() || !doc.Save(error) || read(path) == saved) return 31;
    if (doc.Open((cleanup.path / "archive.dxa/font.lr2font").string(), error) || doc.path != path ||
        error.find("DXA") == std::string::npos) return 32;

    auto workspace = std::make_unique<WORKSPACE>();
    if (!workspace->OpenImageFont(path)) return 33;
    auto& editor = workspace->imageFontEditor;
    editor.height = 0; editor.metricsDirty = editor.fieldsDirty = true;
    const std::string unchanged = editor.document.bytes;
    if (editor.ApplyFields() || editor.document.bytes != unchanged) return 34;
    editor.characterInvalid = true;
    editor.height = 32;
    if (editor.ApplyFields() || editor.document.bytes != unchanged) return 38;
    editor.characterInvalid = false;
    // Use LR2's actual reader after Save: cache invalidation must refresh the
    // same-directory font and clear rectangles for removed/old characters.
    std::vector<FontChar> runtimeChars(0x3bce);
    std::vector<FontImage> runtimePages(1000);
    for (auto& c : runtimeChars) c.grHandle = -1;
    for (auto& p : runtimePages) p.grHandle = -1;
    runtimeChars[100].width = 123;
    auto& runtimeFont = workspace->g.skstruct.ImageFonts[0];
    runtimeFont.chars = runtimeChars.data(); runtimeFont.images = runtimePages.data();
    runtimeFont.size = 16;
    workspace->imageFontRuntimePaths[0] = path;
    strcpy_s(runtimeFont.filepath, (cleanup.path.string() + "\\").c_str());
    if (!editor.ApplyFields() || !workspace->SaveImageFont() || editor.Dirty() ||
        workspace->documentRevision != 0 || workspace->arr_history.count != 0) return 35;
    if (runtimeFont.size != 32 || runtimeChars[65].srcX != 5 || runtimeChars[65].width != 6 ||
        runtimeChars[100].width != 0 || std::string(runtimePages[0].filename) != "font.png") return 39;
    SEImageFontDocument external = editor.document;
    if (!external.SetMetrics(40, 0, error) || !write(path, external.bytes) ||
        !workspace->OpenImageFont(path) || runtimeFont.size != 40) return 40;
    runtimeFont.chars = nullptr; runtimeFont.images = nullptr;
    const auto& spec = SEUIWindowSpecFor(SEUIWindowId::ImageFontEditor);
    if (std::string(spec.group) != "Assets" || spec.defaultVisible || std::string(spec.defaultDock) != "center-tabs") return 36;
    char title[128]; FormatSEUIWindowTitle(title, sizeof(title), SEUIWindowId::ImageFontEditor, 7);
    if (std::string(title) != "Image Font Editor##image-font-editor-7") return 37;

    // Exercise Begin/End, empty + populated windows and clipped glyph lists
    // without D3D or touching any real user font/image.
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); io.IniFilename = nullptr; io.DisplaySize = ImVec2(1000, 800); io.DeltaTime = 1.0f / 60;
    unsigned char* pixels; int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    for (int frame = 0; frame < 3; ++frame) {
        ImGui::NewFrame(); workspace->drawImageFontEditor(); ImGui::Render();
    }
    workspace->imageFontEditor = SEImageFontEditor();
    ImGui::NewFrame(); workspace->drawImageFontEditor(); ImGui::Render();
    ImGui::DestroyContext();
    return 0;
}
