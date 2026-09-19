#include "imageFontDocument.h"
#include "scriptFileSave.h"
#include "inputwrap.h"
#include "../lib/DxLib/DxLib.h"
#include <Windows.h>
#include <algorithm>
#include <charconv>
#include <filesystem>
#include <sstream>

namespace {
struct Line { std::string text, ending; };
std::vector<Line> Lines(const std::string& bytes) {
    std::vector<Line> result;
    for (size_t p = 0; p < bytes.size();) {
        const size_t e = bytes.find_first_of("\r\n", p);
        if (e == std::string::npos) { result.push_back({bytes.substr(p), ""}); break; }
        size_t next = e + 1;
        if (bytes[e] == '\r' && next < bytes.size() && bytes[next] == '\n') ++next;
        result.push_back({bytes.substr(p, e - p), bytes.substr(e, next - e)}); p = next;
    }
    return result;
}
std::string Join(const std::vector<Line>& lines) {
    std::string result;
    for (const auto& line : lines) result += line.text + line.ending;
    return result;
}
std::string Trim(const std::string& s) {
    const size_t first = s.find_first_not_of(" \t");
    return first == std::string::npos ? "" : s.substr(first, s.find_last_not_of(" \t") - first + 1);
}
std::vector<std::string> Fields(const std::string& text) {
    std::vector<std::string> result;
    size_t p = 0;
    for (;;) {
        size_t end = text.find(',', p);
        result.push_back(text.substr(p, end == std::string::npos ? end : end - p));
        if (end == std::string::npos) return result;
        p = end + 1;
    }
}
bool Number(const std::string& field, int& result) {
    std::string text = Trim(field);
    if (text.empty()) { result = 0; return true; }
    if (text[0] == '+') text.erase(0, 1);
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), result);
    return parsed.ec == std::errc() && parsed.ptr == text.data() + text.size();
}
void Patch(std::vector<Line>& lines, int row, const std::vector<std::string>& values) {
    if (row < 0) {
        std::string ending = "\r\n";
        for (const auto& line : lines) if (!line.ending.empty()) { ending = line.ending; break; }
        if (!lines.empty() && lines.back().ending.empty()) lines.back().ending = ending;
        lines.push_back({"", ending}); row = (int)lines.size() - 1;
    }
    auto fields = Fields(lines[row].text);
    if (fields.size() < values.size()) fields.resize(values.size());
    for (size_t i = 0; i < values.size(); ++i) fields[i] = values[i];
    lines[row].text.clear();
    for (size_t i = 0; i < fields.size(); ++i) {
        if (i) lines[row].text += ',';
        lines[row].text += fields[i];
    }
}
bool ValidGlyph(const SEImageFontGlyph& g) {
    return g.code >= 0 && g.code < 0x3bce && g.page >= 0 && g.page < 1000 &&
        g.x >= 0 && g.y >= 0 && g.w >= 0 && g.h >= 0 &&
        (long long)g.x + g.w <= 65536 && (long long)g.y + g.h <= 65536;
}
int Index(const std::string& cp932) {
    if (cp932.size() == 1) return (unsigned char)cp932[0];
    unsigned short value = ((unsigned char)cp932[0] << 8) | (unsigned char)cp932[1];
    if (value >= 0x9ffe) value = (unsigned short)(value + 0xbfbf);
    return (unsigned short)(value + 0x7fc0);
}
}

bool SEReadImageFontBytes(const std::string& path, std::string& bytes, std::string& error) {
    return SEReadFontResource(path, 8 * 1024 * 1024, bytes, error);
}

bool SEImageFontDocument::Parse(const std::string& content, std::string& error) {
    error.clear();
    if (content.find('\0') != std::string::npos || content.size() > 8 * 1024 * 1024 ||
        content.compare(0, 3, "\xef\xbb\xbf") == 0) {
        error = "Expected CP932 .lr2font text (up to 8 MiB)."; return false;
    }
    int newSize = 0, newMargin = 0, newSizeRow = -1, newMarginRow = -1;
    std::map<int, SEImageFontGlyph> newGlyphs;
    std::map<int, SEImageFontPage> newPages;
    const auto lines = Lines(content);
    for (size_t row = 0; row < lines.size(); ++row) {
        const auto fields = Fields(lines[row].text);
        const std::string command = fields[0]; // LR2 requires '#' at column zero.
        bool valid = true;
        if (command == "#S" || command == "#M") {
            int value = 0;
            valid = fields.size() > 1 && Number(fields[1], value);
            if (command == "#S") { valid = valid && value > 0 && value <= 65536; newSize = value; newSizeRow = (int)row; }
            else { valid = valid && value >= -65536 && value <= 65536; newMargin = value; newMarginRow = (int)row; }
        } else if (command == "#T") {
            int page = -1;
            valid = fields.size() > 2 && Number(fields[1], page) && page >= 0 && page < 1000 &&
                !Trim(fields[2]).empty() && Trim(fields[2]).size() < 256 && lines[row].text.size() < 249;
            if (valid) newPages[page] = {(int)row, Trim(fields[2])};
        } else if (command == "#R") {
            SEImageFontGlyph g; int* values[] = {&g.code, &g.page, &g.x, &g.y, &g.w, &g.h};
            valid = fields.size() >= 7;
            for (int i = 0; valid && i < 6; ++i) valid = Number(fields[i + 1], *values[i]);
            valid = valid && ValidGlyph(g);
            if (valid) { g.row = (int)row; newGlyphs[g.code] = g; }
        }
        if (!valid) { error = "Invalid " + command + " at line " + std::to_string(row + 1) + "."; return false; }
    }
    if (newSizeRow < 0) { error = "Missing #S (font height)."; return false; }
    bytes = content; size = newSize; margin = newMargin; sizeRow = newSizeRow; marginRow = newMarginRow;
    glyphs = std::move(newGlyphs); pages = std::move(newPages); return true;
}

bool SEImageFontDocument::Open(const std::string& file, std::string& error) {
    std::string content, archivePath, member;
    std::shared_ptr<SEImageFontArchive> source;
    if (SEFindImageFontArchive(file, archivePath, member)) {
        source = std::make_shared<SEImageFontArchive>();
        DxLib::DXArchiveRelease(archivePath.c_str());
        std::string checked;
        if (!source->Open(archivePath, true, error) || !source->Read(member, 8*1024*1024, checked, error) ||
            !SEReadImageFontBytes(SEImageFontArchiveVirtualPath(archivePath,member), content, error)) return false;
        if (content != checked) { error="DXA changed while opening, or library/archive disagree. Reload before editing."; return false; }
    } else if (!SEReadImageFontBytes(file, content, error)) return false;
    if (!Parse(content, error)) return false;
    path = std::filesystem::absolute(std::filesystem::path(file)).lexically_normal().string();
    archive = source; archiveMember = member;
    originalBytes = bytes; undo.clear(); redo.clear(); return true;
}
bool SEImageFontDocument::Change(const std::string& content, std::string& error) {
    if (content == bytes) { error.clear(); return true; }
    const std::string before = bytes;
    if (!Parse(content, error)) return false;
    if (undo.size() >= 32) undo.erase(undo.begin());
    undo.push_back(before); redo.clear(); return true;
}
bool SEImageFontDocument::SetMetrics(int height, int spacing, std::string& error) {
    auto lines = Lines(bytes);
    Patch(lines, sizeRow, {"#S", std::to_string(height)});
    Patch(lines, marginRow, {"#M", std::to_string(spacing)});
    return Change(Join(lines), error);
}
bool SEImageFontDocument::SetPage(int index, const std::string& name, std::string& error) {
    if (name.find_first_of(",:\r\n") != std::string::npos || name.find('\0') != std::string::npos ||
        name.empty() || std::filesystem::path(name).is_absolute() || name[0] == '\\' || name[0] == '/') {
        error = "Use a CP932 image path relative to the .lr2font folder, without commas or newlines."; return false;
    }
    auto lines = Lines(bytes); auto found = pages.find(index);
    Patch(lines, found == pages.end() ? -1 : found->second.row, {"#T", std::to_string(index), name});
    return Change(Join(lines), error);
}
bool SEImageFontDocument::SetGlyph(int previousCode, const SEImageFontGlyph& glyph, std::string& error) {
    if (!ValidGlyph(glyph) || !pages.count(glyph.page)) { error = "Invalid code, rectangle or undeclared texture page."; return false; }
    if (previousCode != glyph.code && glyphs.count(glyph.code)) { error = "This character code already exists."; return false; }
    auto lines = Lines(bytes);
    int row = -1;
    if (previousCode >= 0) {
        const auto found = glyphs.find(previousCode);
        if (found == glyphs.end()) { error = "Select a character first."; return false; }
        row = found->second.row;
        // Changing an index must also remove shadowed declarations of the old
        // code, otherwise the old character silently reappears on reload.
        if (previousCode != glyph.code) for (int i = 0; i < (int)lines.size(); ++i) {
            auto f = Fields(lines[i].text); int code = -1;
            if (i != row && f.size() > 1 && f[0] == "#R" && Number(f[1], code) && code == previousCode)
                lines[i].text = "// " + lines[i].text;
        }
    }
    Patch(lines, row, {"#R", std::to_string(glyph.code), std::to_string(glyph.page),
        std::to_string(glyph.x), std::to_string(glyph.y), std::to_string(glyph.w), std::to_string(glyph.h)});
    return Change(Join(lines), error);
}
bool SEImageFontDocument::RemoveGlyph(int code, std::string& error) {
    if (!glyphs.count(code)) { error = "Select a character first."; return false; }
    auto lines = Lines(bytes);
    for (auto& line : lines) {
        auto f = Fields(line.text); int value = -1;
        if (f.size() > 1 && f[0] == "#R" && Number(f[1], value) && value == code) line.text = "// " + line.text;
    }
    return Change(Join(lines), error);
}
bool SEImageFontDocument::Undo() {
    if (undo.empty()) return false;
    const std::string before = bytes, target = undo.back(); std::string error;
    if (!Parse(target, error)) return false;
    undo.pop_back(); redo.push_back(before); return true;
}
bool SEImageFontDocument::Redo() {
    if (redo.empty()) return false;
    const std::string before = bytes, target = redo.back(); std::string error;
    if (!Parse(target, error)) return false;
    redo.pop_back(); undo.push_back(before); return true;
}
bool SEImageFontDocument::Save(std::string& report) {
    if (archive) {
        std::string replacement;
        if (!archive->Replace(archiveMember, bytes, replacement, report)) return false;
        auto next = std::make_shared<SEImageFontArchive>();
        if (!next->Parse(std::make_shared<const std::string>(std::move(replacement)), report)) return false;
        next->path = archive->path;
        // DxLib retains idle archive file handles. Release before atomic rename
        // so saving succeeds and subsequent runtime loads cannot use an old index.
        DxLib::DXArchiveRelease(archive->path.c_str());
        if (!SESaveExternalTextFile(archive->path, *archive->snapshot, *next->snapshot, report)) return false;
        archive = next;
    } else if (!SESaveExternalTextFile(path, originalBytes, bytes, report)) return false;
    originalBytes = bytes; return true;
}

bool SEImageFontEncode(const char* utf8, std::string& cp932) {
    const int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, nullptr, 0);
    if (count <= 0) return false;
    std::wstring wide((size_t)count, 0);
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8, -1, wide.data(), count);
    BOOL substituted = FALSE;
    const int bytes = WideCharToMultiByte(932, WC_NO_BEST_FIT_CHARS, wide.data(), count, nullptr, 0, nullptr, &substituted);
    if (bytes <= 0 || substituted) return false;
    std::string result((size_t)bytes, 0);
    if (!WideCharToMultiByte(932, WC_NO_BEST_FIT_CHARS, wide.data(), count, result.data(), bytes, nullptr, &substituted) || substituted) return false;
    result.pop_back(); cp932 = result; return true;
}
bool SEImageFontCode(const char* utf8, int& code) {
    std::string cp;
    if (!SEImageFontEncode(utf8, cp) || cp.empty() || cp.size() > 2 ||
        (cp.size() == 2 && !IsDBCSLeadByteEx(932, (BYTE)cp[0])) ||
        (cp.size() == 1 && IsDBCSLeadByteEx(932, (BYTE)cp[0]))) return false;
    code = Index(cp); return code >= 0 && code < 0x3bce;
}
std::string SEImageFontCharacter(int code) {
    if (code == 9) return "[TAB]";
    if (code == 10) return "[LF]";
    if (code == 32) return "[SPACE]";
    if (code < 32 || code >= 0x3bce) return "";
    for (int mode = 0; mode < 2; ++mode) {
        std::string cp;
        if (code <= 255) cp += (char)code;
        else {
            const unsigned short sjis = (unsigned short)(code - 0x7fc0 - (mode ? 0xbfbf : 0));
            cp += (char)(sjis >> 8); cp += (char)(sjis & 255);
            if (!IsDBCSLeadByteEx(932, (BYTE)cp[0])) continue;
        }
        wchar_t wide[3] = {};
        if (MultiByteToWideChar(932, MB_ERR_INVALID_CHARS, cp.data(), (int)cp.size(), wide, 3) != 1 || Index(cp) != code) continue;
        char utf8[12] = {};
        if (WideCharToMultiByte(CP_UTF8, 0, wide, 1, utf8, 11, nullptr, nullptr)) return utf8;
    }
    return "";
}
std::string SEImageFontPagePath(const SEImageFontDocument& document, int page) {
    const auto found = document.pages.find(page);
    if (found == document.pages.end() || document.path.empty()) return "";
    return (std::filesystem::path(document.path).parent_path() / found->second.path).lexically_normal().string();
}
