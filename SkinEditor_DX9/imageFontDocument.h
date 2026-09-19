#pragma once
#include <map>
#include <string>
#include <vector>
#include "imageFontArchive.h"

// LR2 .lr2font draft, separate from the skin CSV. Original bytes, unknown rows,
// comments, duplicate declarations and line endings survive unedited round trips.
struct SEImageFontGlyph {
    int code = 0, page = 0, x = 0, y = 0, w = 0, h = 0;
    int row = -1;
};
struct SEImageFontPage { int row = -1; std::string path; };
struct SEImageFontDocument {
    std::string path, originalBytes, bytes;
    std::shared_ptr<const SEImageFontArchive> archive;
    std::string archiveMember;
    int size = 0, margin = 0, sizeRow = -1, marginRow = -1;
    std::map<int, SEImageFontGlyph> glyphs;
    std::map<int, SEImageFontPage> pages;
    std::vector<std::string> undo, redo;
    bool Dirty() const { return bytes != originalBytes; }
    bool Open(const std::string& file, std::string& error);
    bool Parse(const std::string& content, std::string& error);
    bool SetMetrics(int height, int spacing, std::string& error);
    bool SetPage(int index, const std::string& cp932Path, std::string& error);
    bool SetGlyph(int previousCode, const SEImageFontGlyph& glyph, std::string& error);
    bool RemoveGlyph(int code, std::string& error);
    bool Undo();
    bool Redo();
    bool Save(std::string& report);
private:
    bool Change(const std::string& content, std::string& error);
};
bool SEReadImageFontBytes(const std::string& path, std::string& bytes, std::string& error);
bool SEImageFontEncode(const char* utf8, std::string& cp932);
bool SEImageFontCode(const char* utf8, int& code);
std::string SEImageFontCharacter(int code);
std::string SEImageFontPagePath(const SEImageFontDocument& document, int page);
int RunImageFontEditorSelfTest();
