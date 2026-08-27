#include "olrSkin.h"

#include <Windows.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <sstream>
#include <vector>

namespace {

constexpr uint32_t kLocalFileHeaderSignature = 0x04034b50u;
constexpr uint32_t kCentralFileHeaderSignature = 0x02014b50u;
constexpr uint32_t kEndOfCentralDirectorySignature = 0x06054b50u;
constexpr uint16_t kStoredMethod = 0;
constexpr uint64_t kMaximumClassicZipSize = 0xFFFFFFFFull;
constexpr size_t kCopyBufferSize = 64 * 1024;

struct PackageEntrySource {
    std::string name;
    std::vector<unsigned char> memory;
    std::string filePath;
    bool isFile = false;
    uint32_t size = 0;
    uint32_t crc = 0;
    uint32_t localHeaderOffset = 0;
};

struct PackageEntryRecord {
    std::string name;
    uint16_t flags = 0;
    uint16_t method = 0;
    uint32_t crc = 0;
    uint32_t compressedSize = 0;
    uint32_t uncompressedSize = 0;
    uint32_t localHeaderOffset = 0;
};

FILE* OpenFile(const char* path, const char* mode) {
    FILE* file = nullptr;
    return fopen_s(&file, path, mode) == 0 ? file : nullptr;
}

uint16_t ReadU16(const unsigned char* bytes) {
    return (uint16_t)(bytes[0] | (bytes[1] << 8));
}

uint32_t ReadU32(const unsigned char* bytes) {
    return (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) |
        ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
}

bool WriteU16(FILE* file, uint16_t value) {
    const unsigned char bytes[2] = {
        (unsigned char)(value & 0xFF),
        (unsigned char)((value >> 8) & 0xFF)
    };
    return fwrite(bytes, 1, sizeof(bytes), file) == sizeof(bytes);
}

bool WriteU32(FILE* file, uint32_t value) {
    const unsigned char bytes[4] = {
        (unsigned char)(value & 0xFF),
        (unsigned char)((value >> 8) & 0xFF),
        (unsigned char)((value >> 16) & 0xFF),
        (unsigned char)((value >> 24) & 0xFF)
    };
    return fwrite(bytes, 1, sizeof(bytes), file) == sizeof(bytes);
}

uint32_t UpdateCrc32(uint32_t crc, const unsigned char* bytes, size_t count) {
    static uint32_t table[256] = {};
    static bool initialized = false;
    if (!initialized) {
        for (uint32_t index = 0; index < 256; ++index) {
            uint32_t value = index;
            for (int bit = 0; bit < 8; ++bit)
                value = (value & 1) ? (value >> 1) ^ 0xEDB88320u : value >> 1;
            table[index] = value;
        }
        initialized = true;
    }
    for (size_t index = 0; index < count; ++index)
        crc = table[(crc ^ bytes[index]) & 0xFF] ^ (crc >> 8);
    return crc;
}

uint32_t ComputeMemoryCrc(const std::vector<unsigned char>& bytes) {
    return UpdateCrc32(0xFFFFFFFFu, bytes.data(), bytes.size()) ^ 0xFFFFFFFFu;
}

bool IsSafeArchivePath(const std::string& path) {
    if (path.empty() || path.size() > 240 || path.front() == '/' ||
        path.back() == '/' || path.find('\\') != std::string::npos ||
        path.find(':') != std::string::npos)
        return false;
    size_t start = 0;
    while (start < path.size()) {
        const size_t slash = path.find('/', start);
        const size_t end = slash == std::string::npos ? path.size() : slash;
        const std::string segment = path.substr(start, end - start);
        if (segment.empty() || segment == "." || segment == "..") return false;
        if (slash == std::string::npos) break;
        start = slash + 1;
    }
    return true;
}

std::string JsonEscape(const std::string& value) {
    std::ostringstream output;
    for (unsigned char ch : value) {
        switch (ch) {
        case '"': output << "\\\""; break;
        case '\\': output << "\\\\"; break;
        case '\b': output << "\\b"; break;
        case '\f': output << "\\f"; break;
        case '\n': output << "\\n"; break;
        case '\r': output << "\\r"; break;
        case '\t': output << "\\t"; break;
        default:
            if (ch < 0x20) {
                const char hex[] = "0123456789ABCDEF";
                output << "\\u00" << hex[(ch >> 4) & 0xF] << hex[ch & 0xF];
            }
            else output << (char)ch;
            break;
        }
    }
    return output.str();
}

void WriteSemanticObject(std::ostringstream& json,
    const SEOLRSemanticObject& object, const char* indent) {
    json << indent << "{\n";
    json << indent << "  \"id\": \"" << JsonEscape(object.id) << "\",\n";
    json << indent << "  \"name\": \"" << JsonEscape(object.name) << "\",\n";
    json << indent << "  \"group\": \"" << JsonEscape(object.group) << "\",\n";
    json << indent << "  \"source_command\": \""
        << JsonEscape(object.sourceCommand) << "\",\n";
    json << indent << "  \"destination_command\": \""
        << JsonEscape(object.destinationCommand) << "\",\n";
    json << indent << "  \"source_rows\": [";
    for (size_t index = 0; index < object.sourceRows.size(); ++index) {
        if (index) json << ", ";
        json << object.sourceRows[index];
    }
    json << "],\n";
    if (object.hasDestination) {
        json << indent << "  \"destination\": {\"x\": " << object.x
            << ", \"y\": " << object.y << ", \"width\": "
            << object.width << ", \"height\": " << object.height
            << ", \"timer\": " << object.timer << ", \"loop\": "
            << object.loop << ", \"ops\": [" << object.op1 << ", "
            << object.op2 << ", " << object.op3 << "]}\n";
    }
    else json << indent << "  \"destination\": null\n";
    json << indent << "}";
}

std::string BuildSkinJson(const SEOLRSkinDocument& document) {
    static const char* categories[] = {
        "gear", "notes", "judge", "combo", "gauge", "bga",
        "effects", "texts", "ui", "misc"
    };
    std::ostringstream json;
    json << "{\n";
    json << "  \"format\": \"olrskin-semantic\",\n";
    json << "  \"version\": 1,\n";
    json << "  \"metadata\": {\"title\": \"" << JsonEscape(document.title)
        << "\", \"maker\": \"" << JsonEscape(document.maker)
        << "\", \"scene\": \"" << JsonEscape(document.scene) << "\"},\n";
    json << "  \"canvas\": {\"width\": " << document.canvasWidth
        << ", \"height\": " << document.canvasHeight
        << ", \"source\": \"" << JsonEscape(document.resolutionSource)
        << "\", \"inferred\": " << (document.resolutionInferred ? "true" : "false")
        << "},\n";
    json << "  \"sections\": {\n";
    for (size_t categoryIndex = 0;
        categoryIndex < sizeof(categories) / sizeof(categories[0]);
        ++categoryIndex) {
        const char* category = categories[categoryIndex];
        json << "    \"" << category << "\": [";
        bool first = true;
        for (const SEOLRSemanticObject& object : document.objects) {
            if (object.category != category) continue;
            json << (first ? "\n" : ",\n");
            WriteSemanticObject(json, object, "      ");
            first = false;
        }
        if (!first) json << "\n    ";
        json << "]";
        json << (categoryIndex + 1 < sizeof(categories) / sizeof(categories[0])
            ? ",\n" : "\n");
    }
    json << "  },\n";
    json << "  \"compatibility\": {\"authority\": \"lr2/main.lr2skin\", "
        << "\"source_map\": \"compatibility/source-map.json\"}\n";
    json << "}\n";
    return json.str();
}

std::string BuildSourceMapJson(const SEOLRSkinDocument& document) {
    std::ostringstream json;
    json << "{\n  \"format\": \"olrskin-source-map\",\n  \"version\": 1,\n"
        << "  \"rows\": [";
    for (size_t index = 0; index < document.sourceMap.size(); ++index) {
        const SEOLRSourceMapEntry& row = document.sourceMap[index];
        json << (index ? ",\n" : "\n");
        json << "    {\"expanded_row\": " << row.expandedRow
            << ", \"packaged_row\": " << row.packagedRow
            << ", \"owner\": \"" << JsonEscape(row.owner) << "\"}";
    }
    if (!document.sourceMap.empty()) json << "\n  ";
    json << "]\n}\n";
    return json.str();
}

std::string BuildManifestJson(const SEOLRSkinDocument& document,
    int bundledAssetCount) {
    std::ostringstream json;
    json << "{\n"
        << "  \"format\": \"olrskin\",\n"
        << "  \"version\": 1,\n"
        << "  \"profile\": \"lr2-compat-v0.1\",\n"
        << "  \"semantic_authority\": \"descriptive\",\n"
        << "  \"lr2_entry\": \"lr2/main.lr2skin\",\n"
        << "  \"skin_entry\": \"skin.json\",\n"
        << "  \"object_count\": " << document.objects.size() << ",\n"
        << "  \"asset_count\": " << bundledAssetCount << ",\n"
        << "  \"unresolved_image_count\": " << document.unresolvedImageCount << ",\n"
        << "  \"limitations\": [\"semantic edits are descriptive in v0.1\", "
        << "\"wildcard/custom/font/video/sound resources may remain external\"]\n"
        << "}\n";
    return json.str();
}

PackageEntrySource MemoryEntry(const char* name, const std::string& text) {
    PackageEntrySource entry;
    entry.name = name;
    entry.memory.assign(text.begin(), text.end());
    entry.size = (uint32_t)entry.memory.size();
    entry.crc = ComputeMemoryCrc(entry.memory);
    return entry;
}

bool PrepareFileEntry(const SEOLRAssetInput& asset, PackageEntrySource& entry,
    std::string& errorMessage) {
    if (!IsSafeArchivePath(asset.packagePath) ||
        asset.packagePath.rfind("lr2/assets/", 0) != 0) {
        errorMessage = "An asset package path was unsafe: " + asset.packagePath;
        return false;
    }
    FILE* file = OpenFile(asset.sourcePath.c_str(), "rb");
    if (!file) {
        errorMessage = "An OLR asset could not be opened: " + asset.sourcePath;
        return false;
    }
    uint64_t size = 0;
    uint32_t crc = 0xFFFFFFFFu;
    std::vector<unsigned char> buffer(kCopyBufferSize);
    size_t count = 0;
    while ((count = fread(buffer.data(), 1, buffer.size(), file)) > 0) {
        size += count;
        if (size > kMaximumClassicZipSize) {
            fclose(file);
            errorMessage = "An OLR asset exceeds the V0.1 ZIP size limit.";
            return false;
        }
        crc = UpdateCrc32(crc, buffer.data(), count);
    }
    const bool readOk = !ferror(file);
    fclose(file);
    if (!readOk) {
        errorMessage = "An OLR asset could not be read completely: " + asset.sourcePath;
        return false;
    }
    entry.name = asset.packagePath;
    entry.filePath = asset.sourcePath;
    entry.isFile = true;
    entry.size = (uint32_t)size;
    entry.crc = crc ^ 0xFFFFFFFFu;
    return true;
}

bool WriteEntryData(FILE* archive, const PackageEntrySource& entry) {
    if (!entry.isFile)
        return entry.memory.empty() ||
            fwrite(entry.memory.data(), 1, entry.memory.size(), archive) ==
            entry.memory.size();

    FILE* source = OpenFile(entry.filePath.c_str(), "rb");
    if (!source) return false;
    std::vector<unsigned char> buffer(kCopyBufferSize);
    size_t count = 0;
    bool ok = true;
    while ((count = fread(buffer.data(), 1, buffer.size(), source)) > 0) {
        if (fwrite(buffer.data(), 1, count, archive) != count) {
            ok = false;
            break;
        }
    }
    if (ferror(source)) ok = false;
    fclose(source);
    return ok;
}

bool WriteArchive(const char* packagePath,
    std::vector<PackageEntrySource>& entries, std::string& errorMessage) {
    if (!packagePath || !*packagePath) {
        errorMessage = "No OLR package path was provided.";
        return false;
    }
    if (entries.empty() || entries.size() > 0xFFFF) {
        errorMessage = "The OLR package has an unsupported entry count.";
        return false;
    }
    std::map<std::string, bool> names;
    for (const PackageEntrySource& entry : entries) {
        if (!IsSafeArchivePath(entry.name) || !names.emplace(entry.name, true).second) {
            errorMessage = "The OLR package contains an unsafe or duplicate entry name.";
            return false;
        }
    }

    const std::string temporaryPath = std::string(packagePath) + ".skineditor.tmp";
    remove(temporaryPath.c_str());
    FILE* archive = OpenFile(temporaryPath.c_str(), "wb");
    if (!archive) {
        errorMessage = "The OLR temporary package could not be created.";
        return false;
    }

    bool ok = true;
    for (PackageEntrySource& entry : entries) {
        const __int64 offset = _ftelli64(archive);
        if (offset < 0 || (uint64_t)offset > kMaximumClassicZipSize ||
            entry.name.size() > 0xFFFF) {
            ok = false;
            break;
        }
        entry.localHeaderOffset = (uint32_t)offset;
        ok = WriteU32(archive, kLocalFileHeaderSignature) &&
            WriteU16(archive, 20) && WriteU16(archive, 0) &&
            WriteU16(archive, kStoredMethod) && WriteU16(archive, 0) &&
            WriteU16(archive, 0) && WriteU32(archive, entry.crc) &&
            WriteU32(archive, entry.size) && WriteU32(archive, entry.size) &&
            WriteU16(archive, (uint16_t)entry.name.size()) &&
            WriteU16(archive, 0) &&
            fwrite(entry.name.data(), 1, entry.name.size(), archive) == entry.name.size() &&
            WriteEntryData(archive, entry);
        if (!ok) break;
    }

    const __int64 centralStartValue = ok ? _ftelli64(archive) : -1;
    if (centralStartValue < 0 || (uint64_t)centralStartValue > kMaximumClassicZipSize)
        ok = false;
    const uint32_t centralStart = ok ? (uint32_t)centralStartValue : 0;
    if (ok) {
        for (const PackageEntrySource& entry : entries) {
            ok = WriteU32(archive, kCentralFileHeaderSignature) &&
                WriteU16(archive, 20) && WriteU16(archive, 20) &&
                WriteU16(archive, 0) && WriteU16(archive, kStoredMethod) &&
                WriteU16(archive, 0) && WriteU16(archive, 0) &&
                WriteU32(archive, entry.crc) && WriteU32(archive, entry.size) &&
                WriteU32(archive, entry.size) &&
                WriteU16(archive, (uint16_t)entry.name.size()) &&
                WriteU16(archive, 0) && WriteU16(archive, 0) &&
                WriteU16(archive, 0) && WriteU16(archive, 0) &&
                WriteU32(archive, 0) && WriteU32(archive, entry.localHeaderOffset) &&
                fwrite(entry.name.data(), 1, entry.name.size(), archive) == entry.name.size();
            if (!ok) break;
        }
    }

    const __int64 centralEndValue = ok ? _ftelli64(archive) : -1;
    if (centralEndValue < centralStartValue || centralEndValue < 0 ||
        (uint64_t)centralEndValue > kMaximumClassicZipSize)
        ok = false;
    const uint32_t centralSize = ok
        ? (uint32_t)(centralEndValue - centralStartValue) : 0;
    if (ok) {
        const uint16_t entryCount = (uint16_t)entries.size();
        ok = WriteU32(archive, kEndOfCentralDirectorySignature) &&
            WriteU16(archive, 0) && WriteU16(archive, 0) &&
            WriteU16(archive, entryCount) && WriteU16(archive, entryCount) &&
            WriteU32(archive, centralSize) && WriteU32(archive, centralStart) &&
            WriteU16(archive, 0);
    }
    if (fclose(archive) != 0) ok = false;
    if (!ok) {
        remove(temporaryPath.c_str());
        errorMessage = "The OLR package could not be written completely.";
        return false;
    }
    if (!MoveFileExA(temporaryPath.c_str(), packagePath,
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        remove(temporaryPath.c_str());
        errorMessage = "The completed OLR package could not replace the destination.";
        return false;
    }
    return true;
}

bool ReadArchiveDirectory(FILE* archive, std::vector<PackageEntryRecord>& entries,
    std::string& errorMessage) {
    if (_fseeki64(archive, 0, SEEK_END) != 0) return false;
    const __int64 fileSizeValue = _ftelli64(archive);
    if (fileSizeValue < 22 || (uint64_t)fileSizeValue > kMaximumClassicZipSize) {
        errorMessage = "The OLR package size is invalid or requires ZIP64.";
        return false;
    }
    const uint64_t fileSize = (uint64_t)fileSizeValue;
    const size_t tailSize = (size_t)(std::min<uint64_t>)(fileSize, 22 + 0xFFFF);
    std::vector<unsigned char> tail(tailSize);
    if (_fseeki64(archive, (__int64)(fileSize - tailSize), SEEK_SET) != 0 ||
        fread(tail.data(), 1, tail.size(), archive) != tail.size()) {
        errorMessage = "The OLR package footer could not be read.";
        return false;
    }
    size_t endOffset = tail.size();
    for (size_t cursor = tail.size() - 22;; --cursor) {
        if (ReadU32(tail.data() + cursor) == kEndOfCentralDirectorySignature) {
            endOffset = cursor;
            break;
        }
        if (cursor == 0) break;
    }
    if (endOffset == tail.size() || endOffset + 22 > tail.size()) {
        errorMessage = "The OLR ZIP footer was not found.";
        return false;
    }
    const unsigned char* end = tail.data() + endOffset;
    const uint16_t disk = ReadU16(end + 4);
    const uint16_t centralDisk = ReadU16(end + 6);
    const uint16_t diskEntries = ReadU16(end + 8);
    const uint16_t totalEntries = ReadU16(end + 10);
    const uint32_t centralSize = ReadU32(end + 12);
    const uint32_t centralOffset = ReadU32(end + 16);
    const uint16_t commentLength = ReadU16(end + 20);
    if (disk != 0 || centralDisk != 0 || diskEntries != totalEntries ||
        totalEntries == 0 || endOffset + 22 + commentLength != tail.size() ||
        (uint64_t)centralOffset + centralSize > fileSize) {
        errorMessage = "The OLR central directory is invalid.";
        return false;
    }
    if (_fseeki64(archive, centralOffset, SEEK_SET) != 0) return false;
    entries.clear();
    entries.reserve(totalEntries);
    std::map<std::string, bool> names;
    uint64_t consumed = 0;
    for (uint16_t index = 0; index < totalEntries; ++index) {
        unsigned char header[46];
        if (fread(header, 1, sizeof(header), archive) != sizeof(header) ||
            ReadU32(header) != kCentralFileHeaderSignature) {
            errorMessage = "An OLR central directory entry is invalid.";
            return false;
        }
        consumed += sizeof(header);
        PackageEntryRecord entry;
        entry.flags = ReadU16(header + 8);
        entry.method = ReadU16(header + 10);
        entry.crc = ReadU32(header + 16);
        entry.compressedSize = ReadU32(header + 20);
        entry.uncompressedSize = ReadU32(header + 24);
        const uint16_t nameLength = ReadU16(header + 28);
        const uint16_t extraLength = ReadU16(header + 30);
        const uint16_t entryCommentLength = ReadU16(header + 32);
        const uint16_t entryDisk = ReadU16(header + 34);
        entry.localHeaderOffset = ReadU32(header + 42);
        if (nameLength == 0 || nameLength > 240 || entryDisk != 0 ||
            (entry.flags & 1) != 0 || entry.method != kStoredMethod ||
            entry.compressedSize != entry.uncompressedSize) {
            errorMessage = "An OLR entry uses unsupported ZIP features.";
            return false;
        }
        std::vector<char> name(nameLength);
        if (fread(name.data(), 1, name.size(), archive) != name.size() ||
            _fseeki64(archive, (uint64_t)extraLength + entryCommentLength,
                SEEK_CUR) != 0) {
            errorMessage = "An OLR entry name could not be read.";
            return false;
        }
        consumed += nameLength + extraLength + entryCommentLength;
        entry.name.assign(name.begin(), name.end());
        if (!IsSafeArchivePath(entry.name) ||
            !names.emplace(entry.name, true).second) {
            errorMessage = "An OLR entry path is unsafe or duplicated.";
            return false;
        }
        entries.push_back(entry);
    }
    if (consumed != centralSize) {
        errorMessage = "The OLR central directory size does not match its entries.";
        return false;
    }
    return true;
}

bool EntryDataOffset(FILE* archive, const PackageEntryRecord& entry,
    uint64_t& dataOffset, std::string& errorMessage) {
    if (_fseeki64(archive, entry.localHeaderOffset, SEEK_SET) != 0) return false;
    unsigned char header[30];
    if (fread(header, 1, sizeof(header), archive) != sizeof(header) ||
        ReadU32(header) != kLocalFileHeaderSignature ||
        ReadU16(header + 6) != entry.flags ||
        ReadU16(header + 8) != entry.method ||
        ReadU32(header + 14) != entry.crc ||
        ReadU32(header + 18) != entry.compressedSize ||
        ReadU32(header + 22) != entry.uncompressedSize) {
        errorMessage = "An OLR local file header does not match its directory.";
        return false;
    }
    const uint16_t nameLength = ReadU16(header + 26);
    const uint16_t extraLength = ReadU16(header + 28);
    std::vector<char> name(nameLength);
    if (nameLength != entry.name.size() ||
        fread(name.data(), 1, name.size(), archive) != name.size() ||
        std::string(name.begin(), name.end()) != entry.name) {
        errorMessage = "An OLR local entry name does not match its directory.";
        return false;
    }
    dataOffset = (uint64_t)entry.localHeaderOffset + sizeof(header) +
        nameLength + extraLength;
    return true;
}

bool CopyAndValidateEntry(FILE* archive, const PackageEntryRecord& entry,
    FILE* output, std::vector<unsigned char>* captured,
    std::string& errorMessage) {
    uint64_t dataOffset = 0;
    if (!EntryDataOffset(archive, entry, dataOffset, errorMessage) ||
        _fseeki64(archive, dataOffset, SEEK_SET) != 0)
        return false;
    if (captured) {
        captured->clear();
        captured->reserve(entry.uncompressedSize);
    }
    uint32_t crc = 0xFFFFFFFFu;
    uint32_t remaining = entry.uncompressedSize;
    std::vector<unsigned char> buffer(kCopyBufferSize);
    while (remaining > 0) {
        const size_t requested = (std::min<size_t>)(buffer.size(), remaining);
        const size_t count = fread(buffer.data(), 1, requested, archive);
        if (count != requested) {
            errorMessage = "An OLR entry ended before its declared size.";
            return false;
        }
        crc = UpdateCrc32(crc, buffer.data(), count);
        if (output && fwrite(buffer.data(), 1, count, output) != count) {
            errorMessage = "An extracted OLR file could not be written.";
            return false;
        }
        if (captured) captured->insert(captured->end(), buffer.begin(), buffer.begin() + count);
        remaining -= (uint32_t)count;
    }
    if ((crc ^ 0xFFFFFFFFu) != entry.crc) {
        errorMessage = "An OLR entry failed its CRC check: " + entry.name;
        return false;
    }
    return true;
}

bool ValidateManifest(FILE* archive,
    const std::vector<PackageEntryRecord>& entries, SEOLRPackageInfo& info,
    std::string& errorMessage) {
    const PackageEntryRecord* manifest = nullptr;
    bool hasSkin = false;
    bool hasSourceMap = false;
    bool hasMain = false;
    for (const PackageEntryRecord& entry : entries) {
        info.entries.push_back(entry.name);
        if (entry.name == "manifest.json") manifest = &entry;
        else if (entry.name == "skin.json") hasSkin = true;
        else if (entry.name == "compatibility/source-map.json") hasSourceMap = true;
        else if (entry.name == "lr2/main.lr2skin") hasMain = true;
        if (entry.name.rfind("lr2/assets/", 0) == 0) ++info.assetCount;
    }
    if (!manifest || !hasSkin || !hasSourceMap || !hasMain) {
        errorMessage = "The OLR package is missing a required entry.";
        return false;
    }
    std::vector<unsigned char> bytes;
    if (!CopyAndValidateEntry(archive, *manifest, nullptr, &bytes, errorMessage))
        return false;
    const std::string text(bytes.begin(), bytes.end());
    if (text.find("\"format\": \"olrskin\"") == std::string::npos ||
        text.find("\"version\": 1") == std::string::npos ||
        text.find("\"lr2_entry\": \"lr2/main.lr2skin\"") == std::string::npos) {
        errorMessage = "The OLR manifest format or version is unsupported.";
        return false;
    }
    const char* objectKey = "\"object_count\": ";
    const char* unresolvedKey = "\"unresolved_image_count\": ";
    const size_t objectAt = text.find(objectKey);
    const size_t unresolvedAt = text.find(unresolvedKey);
    if (objectAt != std::string::npos)
        info.objectCount = atoi(text.c_str() + objectAt + strlen(objectKey));
    if (unresolvedAt != std::string::npos)
        info.unresolvedImageCount = atoi(text.c_str() + unresolvedAt + strlen(unresolvedKey));
    return true;
}

bool OpenAndValidateArchive(const char* packagePath, FILE*& archive,
    std::vector<PackageEntryRecord>& entries, SEOLRPackageInfo& info,
    std::string& errorMessage) {
    archive = OpenFile(packagePath, "rb");
    if (!archive) {
        errorMessage = "The OLR package could not be opened.";
        return false;
    }
    if (!ReadArchiveDirectory(archive, entries, errorMessage)) {
        fclose(archive);
        archive = nullptr;
        return false;
    }
    info = SEOLRPackageInfo();
    if (!ValidateManifest(archive, entries, info, errorMessage)) {
        fclose(archive);
        archive = nullptr;
        return false;
    }
    return true;
}

}

bool SEWriteOLRSkinPackage(const char* packagePath,
    const SEOLRSkinDocument& document, SEOLRPackageInfo& packageInfo,
    std::string& errorMessage) {
    packageInfo = SEOLRPackageInfo();
    errorMessage.clear();
    if (document.lr2Script.empty()) {
        errorMessage = "The OLR package has no compiled LR2 compatibility script.";
        return false;
    }

    std::vector<PackageEntrySource> assetEntries;
    assetEntries.reserve(document.assets.size());
    for (const SEOLRAssetInput& asset : document.assets) {
        PackageEntrySource entry;
        if (!PrepareFileEntry(asset, entry, errorMessage)) return false;
        assetEntries.push_back(std::move(entry));
    }

    std::vector<PackageEntrySource> entries;
    entries.push_back(MemoryEntry("manifest.json",
        BuildManifestJson(document, (int)assetEntries.size())));
    entries.push_back(MemoryEntry("skin.json", BuildSkinJson(document)));
    entries.push_back(MemoryEntry("compatibility/source-map.json",
        BuildSourceMapJson(document)));
    entries.push_back(MemoryEntry("lr2/main.lr2skin", document.lr2Script));
    for (PackageEntrySource& asset : assetEntries)
        entries.push_back(std::move(asset));

    if (!WriteArchive(packagePath, entries, errorMessage)) return false;
    return SEInspectOLRSkinPackage(packagePath, packageInfo, errorMessage);
}

bool SEInspectOLRSkinPackage(const char* packagePath,
    SEOLRPackageInfo& packageInfo, std::string& errorMessage) {
    FILE* archive = nullptr;
    std::vector<PackageEntryRecord> entries;
    if (!OpenAndValidateArchive(packagePath, archive, entries, packageInfo,
        errorMessage))
        return false;
    for (const PackageEntryRecord& entry : entries) {
        if (!CopyAndValidateEntry(archive, entry, nullptr, nullptr, errorMessage)) {
            fclose(archive);
            return false;
        }
    }
    fclose(archive);
    return true;
}

bool SEExtractOLRSkinPackage(const char* packagePath,
    const char* outputDirectory, std::string& mainSkinPath,
    SEOLRPackageInfo& packageInfo, std::string& errorMessage) {
    mainSkinPath.clear();
    if (!outputDirectory || !*outputDirectory) {
        errorMessage = "No OLR import directory was provided.";
        return false;
    }
    std::error_code filesystemError;
    const std::filesystem::path target(outputDirectory);
    if (std::filesystem::exists(target, filesystemError)) {
        errorMessage = "The OLR import directory already exists.";
        return false;
    }

    FILE* archive = nullptr;
    std::vector<PackageEntryRecord> entries;
    if (!OpenAndValidateArchive(packagePath, archive, entries, packageInfo,
        errorMessage))
        return false;

    // Validate the complete package before creating any output. Import only
    // extracts lr2/, but a corrupt semantic or source-map entry still makes
    // the container invalid.
    for (const PackageEntryRecord& entry : entries) {
        if (!CopyAndValidateEntry(archive, entry, nullptr, nullptr, errorMessage)) {
            fclose(archive);
            return false;
        }
    }

    if (!std::filesystem::create_directories(target, filesystemError) ||
        filesystemError) {
        fclose(archive);
        errorMessage = "The OLR import directory could not be created.";
        return false;
    }

    bool ok = true;
    for (const PackageEntryRecord& entry : entries) {
        if (entry.name.rfind("lr2/", 0) != 0) continue;
        const std::string relative = entry.name.substr(4);
        if (!IsSafeArchivePath(relative)) {
            errorMessage = "The OLR LR2 output path is unsafe.";
            ok = false;
            break;
        }
        const std::filesystem::path outputPath = target /
            std::filesystem::path(relative);
        std::filesystem::create_directories(outputPath.parent_path(),
            filesystemError);
        if (filesystemError) {
            errorMessage = "An OLR output directory could not be created.";
            ok = false;
            break;
        }
        FILE* output = OpenFile(outputPath.string().c_str(), "wb");
        if (!output) {
            errorMessage = "An OLR output file could not be created.";
            ok = false;
            break;
        }
        ok = CopyAndValidateEntry(archive, entry, output, nullptr, errorMessage);
        if (fclose(output) != 0) ok = false;
        if (!ok) break;
    }
    fclose(archive);

    if (!ok) {
        std::filesystem::remove_all(target, filesystemError);
        return false;
    }
    mainSkinPath = (target / "main.lr2skin").string();
    if (!std::filesystem::exists(mainSkinPath, filesystemError)) {
        std::filesystem::remove_all(target, filesystemError);
        mainSkinPath.clear();
        errorMessage = "The extracted OLR package had no main LR2 script.";
        return false;
    }
    return true;
}
