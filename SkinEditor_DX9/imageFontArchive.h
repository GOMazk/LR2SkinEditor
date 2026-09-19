#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

// Legacy DXArchive file-format versions 1..4 (including LR2's DxaEncode 1.02).
// Default key only; never infer passwords or convert to a newer archive version.
class SEImageFontArchive {
public:
    struct Entry {
        std::string name;
        uint32_t record = 0, offset = 0, size = 0, packed = 0xffffffff;
    };
    std::string path;
    int version = 0;
    std::map<std::string, Entry> entries;
    std::shared_ptr<const std::string> snapshot;
    bool Open(const std::string& file, bool keepSnapshot, std::string& error);
    bool Parse(std::shared_ptr<const std::string> bytes, std::string& error);
    bool Read(const std::string& member, size_t limit, std::string& bytes, std::string& error) const;
    bool Replace(const std::string& member, const std::string& bytes,
        std::string& result, std::string& error) const;
    std::vector<std::string> Fonts() const;
private:
    std::string header, index;
    uint32_t dataStart = 0, indexStart = 0, fileTable = 0, dirTable = 0;
    uint64_t fileSize = 0, fileTime = 0;
    bool keyed = true;
    bool ParseIndex(std::string& error);
    bool ParseHeader(std::string& error);
};

// Recognizes both font/font.lr2font -> font.dxa and explicit font.dxa/font.lr2font.
// DXA wins over a same-named disk folder, matching DxLib's default priority.
bool SEFindImageFontArchive(const std::string& virtualPath, std::string& archive, std::string& member);
bool SEReadFontResource(const std::string& path, size_t limit, std::string& bytes, std::string& error);
std::string SEImageFontArchiveVirtualPath(const std::string& archive, const std::string& member);
std::string SEFontPathKey(const std::string& path);
int RunImageFontArchiveSelfTest();
int CheckImageFontArchiveCommand();
