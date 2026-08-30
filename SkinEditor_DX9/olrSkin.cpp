#include "olrSkin.h"
#include "op.h"
#include "seHelper.h"

#include <Windows.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <vector>

namespace {

constexpr uint32_t kLocalFileHeaderSignature = 0x04034b50u;
constexpr uint32_t kCentralFileHeaderSignature = 0x02014b50u;
constexpr uint32_t kEndOfCentralDirectorySignature = 0x06054b50u;
constexpr uint16_t kStoredMethod = 0;
constexpr uint64_t kMaximumClassicZipSize = 0xFFFFFFFFull;
constexpr size_t kCopyBufferSize = 64 * 1024;
constexpr int kOlrFormatVersion = 8;
constexpr const char* kSimpleModeAuthority = "lr2-source-v0.4";
constexpr const char* kLegacySemanticObjectAuthority = "lr2-destination-v0.7";
constexpr const char* kSemanticObjectAuthority = "lr2-destination-parts-v0.8";

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

bool IsRawLr2Option(int option) {
    const int absolute = std::abs(option);
    return absolute >= 900 || !dstName((unsigned)absolute)[0];
}

void DescribeLr2Option(int option, std::string& key, std::string& value) {
    const int absolute = std::abs(option);
    switch (absolute) {
    case 42: key = "Gauge"; value = "1P GROOVE"; return;
    case 43: key = "Gauge"; value = "1P RED"; return;
    case 44: key = "Gauge"; value = "2P GROOVE"; return;
    case 45: key = "Gauge"; value = "2P RED"; return;
    case 118: key = "Gauge"; value = "GROOVE"; return;
    case 119: key = "Gauge"; value = "HARD"; return;
    case 120: key = "Gauge"; value = "EX-HARD"; return;
    case 121: key = "Gauge"; value = "EASY"; return;
    case 66:
    case 105:
    case 645: key = "FullCombo"; value = "true"; return;
    default:
        key = "LR2 Option";
        value = dstName((unsigned)absolute);
        return;
    }
}

void WriteConditionTerm(std::ostringstream& json, int slot, int option,
    const char* indent) {
    const int absolute = std::abs(option);
    if (IsRawLr2Option(option)) {
        json << indent << "{\"slot\": " << slot
            << ", \"kind\": \"raw\", \"lr2_op\": " << option
            << ", \"label\": \"Raw LR2 OP " << option << "\"}";
        return;
    }
    std::string key;
    std::string value;
    DescribeLr2Option(option, key, value);
    json << indent << "{\"slot\": " << slot
        << ", \"kind\": \"semantic\", \"key\": \""
        << JsonEscape(key) << "\", \"value\": \"" << JsonEscape(value)
        << "\", \"lr2_name\": \"" << JsonEscape(dstName((unsigned)absolute))
        << "\", \"negated\": " << (option < 0 ? "true" : "false") << "}";
}

void WriteTransform(std::ostringstream& json,
    const SEOLRSemanticObject::Transform& transform) {
    json << "{\"x\": " << transform.x << ", \"y\": " << transform.y
        << ", \"width\": " << transform.width << ", \"height\": "
        << transform.height << ", \"rotation\": " << transform.rotation
        << ", \"blend\": " << transform.blend << "}";
}

bool IsSemanticDestination(
    const SEOLRSemanticObject::Destination& destination) {
    return !destination.destinationCommand.empty() &&
        !destination.animationFrames.empty();
}

size_t SemanticDestinationCount(const SEOLRSemanticObject::Part& part) {
    return (size_t)std::count_if(part.destinations.begin(),
        part.destinations.end(), IsSemanticDestination);
}

size_t SemanticPartCount(const SEOLRSemanticObject& object) {
    return (size_t)std::count_if(object.parts.begin(), object.parts.end(),
        [](const SEOLRSemanticObject::Part& part) {
            return SemanticDestinationCount(part) > 0;
        });
}

bool IsSemanticObject(const SEOLRSemanticObject& object) {
    return SemanticPartCount(object) > 0;
}

size_t SemanticObjectCount(const SEOLRSkinDocument& document) {
    return (size_t)std::count_if(document.objects.begin(),
        document.objects.end(), IsSemanticObject);
}

size_t SemanticPartCount(const SEOLRSkinDocument& document) {
    size_t count = 0;
    for (const SEOLRSemanticObject& object : document.objects)
        count += SemanticPartCount(object);
    return count;
}

size_t SemanticDestinationCount(const SEOLRSkinDocument& document) {
    size_t count = 0;
    for (const SEOLRSemanticObject& object : document.objects)
        for (const SEOLRSemanticObject::Part& part : object.parts)
            count += SemanticDestinationCount(part);
    return count;
}

void WriteSemanticDestination(std::ostringstream& json,
    const SEOLRSemanticObject::Destination& destination,
    const char* indent) {
    json << indent << "{\n";
    json << indent << "  \"id\": \"" << JsonEscape(destination.id)
        << "\",\n";
    json << indent << "  \"destination_command\": \""
        << JsonEscape(destination.destinationCommand) << "\",\n";
    json << indent << "  \"layout\": {\"destination_row\": "
        << destination.animationFrames.front().destinationRow
        << ", \"transform\": ";
    WriteTransform(json, destination.layout);
    json << "},\n";
    json << indent << "  \"animation\": {\"frames\": [";
    for (size_t index = 0; index < destination.animationFrames.size(); ++index) {
        const SEOLRSemanticObject::AnimationFrame& frame =
            destination.animationFrames[index];
        json << (index ? ",\n" : "\n") << indent
            << "    {\"destination_row\": " << frame.destinationRow
            << ", \"time_ms\": " << frame.timeMs << ", \"alpha\": "
            << frame.alpha << ", \"transform\": ";
        WriteTransform(json, frame.transform);
        json << "}";
    }
    json << "\n" << indent << "  ]},\n";

    json << indent << "  \"condition\": {\"mode\": \"all\", \"timer\": ";
    if (!destination.hasTimer) {
        json << "null";
    }
    else {
        const int absoluteTimer = std::abs(destination.timer);
        const char* timerLabel = timerName((unsigned)absoluteTimer);
        const bool rawTimer = destination.timer < 0 || !timerLabel[0];
        if (rawTimer) {
            json << "{\"kind\": \"raw\", \"lr2_timer\": "
                << destination.timer << ", \"label\": \"Raw LR2 TIMER "
                << destination.timer << "\"}";
        }
        else {
            json << "{\"kind\": \"semantic\", \"lr2_name\": \""
                << JsonEscape(timerLabel) << "\"}";
        }
    }
    json << ", \"loop\": ";
    if (destination.hasLoop) json << destination.loop;
    else json << "null";
    json << ", \"all\": [";
    bool firstCondition = true;
    for (int optionIndex = 0; optionIndex < 3; ++optionIndex) {
        if (!destination.hasOptions[optionIndex]) continue;
        json << (firstCondition ? "\n" : ",\n");
        WriteConditionTerm(json, optionIndex + 1,
            destination.options[optionIndex],
            (std::string(indent) + "    ").c_str());
        firstCondition = false;
    }
    if (!firstCondition) json << "\n" << indent << "  ";
    json << "]}\n";
    json << indent << "}";
}

void WriteSemanticPart(std::ostringstream& json,
    const SEOLRSemanticObject::Part& part, const char* indent) {
    json << indent << "{\n";
    json << indent << "  \"id\": \"" << JsonEscape(part.id) << "\",\n";
    json << indent << "  \"sources\": [";
    for (size_t index = 0; index < part.sources.size(); ++index) {
        const SEOLRSemanticObject::SourceBinding& source = part.sources[index];
        json << (index ? ",\n" : "\n") << indent
            << "    {\"source_row\": " << source.sourceRow
            << ", \"source_command\": \""
            << JsonEscape(source.sourceCommand) << "\"}";
    }
    if (!part.sources.empty()) json << "\n" << indent << "  ";
    json << "],\n";
    json << indent << "  \"destinations\": [";
    bool firstDestination = true;
    for (const SEOLRSemanticObject::Destination& destination :
        part.destinations) {
        if (!IsSemanticDestination(destination)) continue;
        json << (firstDestination ? "\n" : ",\n");
        WriteSemanticDestination(json, destination,
            (std::string(indent) + "    ").c_str());
        firstDestination = false;
    }
    if (!firstDestination) json << "\n" << indent << "  ";
    json << "]\n";
    json << indent << "}";
}

void WriteSemanticObject(std::ostringstream& json,
    const SEOLRSemanticObject& object, const char* indent) {
    json << indent << "{\n";
    json << indent << "  \"id\": \"" << JsonEscape(object.id) << "\",\n";
    json << indent << "  \"name\": \"" << JsonEscape(object.name) << "\",\n";
    json << indent << "  \"group\": \"" << JsonEscape(object.group) << "\",\n";
    json << indent << "  \"parts\": [";
    bool firstPart = true;
    for (const SEOLRSemanticObject::Part& part : object.parts) {
        if (SemanticDestinationCount(part) == 0) continue;
        json << (firstPart ? "\n" : ",\n");
        WriteSemanticPart(json, part, (std::string(indent) + "    ").c_str());
        firstPart = false;
    }
    if (!firstPart) json << "\n" << indent << "  ";
    json << "]\n";
    json << indent << "}";
}

void WriteSimpleSlot(std::ostringstream& json, const SEOLRSimpleSlot& slot,
    const char* indent) {
    json << indent << "{\"id\": \"" << JsonEscape(slot.id)
        << "\", \"category\": \"" << JsonEscape(slot.category)
        << "\", \"label\": \"" << JsonEscape(slot.label)
        << "\", \"object_id\": \"" << JsonEscape(slot.objectId)
        << "\", \"source_command\": \"" << JsonEscape(slot.sourceCommand)
        << "\", \"source_row\": " << slot.sourceRow
        << ", \"asset\": {\"gr\": " << slot.graphicId
        << ", \"x\": " << slot.x << ", \"y\": " << slot.y
        << ", \"width\": " << slot.width << ", \"height\": "
        << slot.height << ", \"div_x\": " << slot.divX
        << ", \"div_y\": " << slot.divY << ", \"cycle\": "
        << slot.cycle << "}}";
}

std::string BuildSkinJson(const SEOLRSkinDocument& document) {
    static const char* categories[] = {
        "gear", "notes", "judge", "combo", "gauge", "bga",
        "effects", "texts", "ui", "misc"
    };
    std::ostringstream json;
    json << "{\n";
    json << "  \"format\": \"olrskin-semantic\",\n";
    json << "  \"version\": 8,\n";
    json << "  \"metadata\": {\"title\": \"" << JsonEscape(document.title)
        << "\", \"maker\": \"" << JsonEscape(document.maker)
        << "\", \"scene\": \"" << JsonEscape(document.scene) << "\"},\n";
    json << "  \"canvas\": {\"width\": " << document.canvasWidth
        << ", \"height\": " << document.canvasHeight
        << ", \"source\": \"" << JsonEscape(document.resolutionSource)
        << "\", \"inferred\": " << (document.resolutionInferred ? "true" : "false")
        << "},\n";
    json << "  \"objects\": {\"authority\": \""
        << kSemanticObjectAuthority << "\", \"items\": [";
    bool firstObject = true;
    for (const SEOLRSemanticObject& object : document.objects) {
        if (!IsSemanticObject(object)) continue;
        json << (firstObject ? "\n" : ",\n");
        WriteSemanticObject(json, object, "    ");
        firstObject = false;
    }
    if (!firstObject) json << "\n  ";
    json << "]},\n";
    json << "  \"sections\": {\n";
    for (size_t categoryIndex = 0;
        categoryIndex < sizeof(categories) / sizeof(categories[0]);
        ++categoryIndex) {
        const char* category = categories[categoryIndex];
        json << "    \"" << category << "\": [";
        bool first = true;
        for (const SEOLRSemanticObject& object : document.objects) {
            if (!IsSemanticObject(object) || object.category != category)
                continue;
            json << (first ? "\n" : ",\n") << "      \""
                << JsonEscape(object.id) << "\"";
            first = false;
        }
        if (!first) json << "\n    ";
        json << "]";
        json << (categoryIndex + 1 < sizeof(categories) / sizeof(categories[0])
            ? ",\n" : "\n");
    }
    json << "  },\n";
    json << "  \"simple_mode\": {\"authority\": \""
        << kSimpleModeAuthority << "\", \"slots\": [";
    for (size_t index = 0; index < document.simpleSlots.size(); ++index) {
        json << (index ? ",\n" : "\n");
        WriteSimpleSlot(json, document.simpleSlots[index], "    ");
    }
    if (!document.simpleSlots.empty()) json << "\n  ";
    json << "]},\n";
    json << "  \"compatibility\": {\"authority\": \"lr2/main.lr2skin\", "
        << "\"source_map\": \"compatibility/source-map.json\", "
        << "\"path_map\": \"compatibility/path-map.json\"}\n";
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

struct VirtualRootPackageStats {
    std::string logicalRoot;
    int fileCount = 0;
    int skippedFileCount = 0;
};

std::string BuildPathMapJson(const SEOLRSkinDocument& document,
    const std::vector<VirtualRootPackageStats>& rootStats) {
    std::ostringstream json;
    json << "{\n  \"format\": \"olrskin-path-map\",\n  \"version\": 1,\n"
        << "  \"workspace_prefix\": \"vfs/\",\n"
        << "  \"export_main\": \"" << JsonEscape(document.lr2ExportMainPath)
        << "\",\n  \"roots\": [";
    for (size_t index = 0; index < rootStats.size(); ++index) {
        const VirtualRootPackageStats& root = rootStats[index];
        json << (index ? ",\n" : "\n")
            << "    {\"logical\": \"" << JsonEscape(root.logicalRoot)
            << "\", \"virtual\": \"vfs/" << JsonEscape(root.logicalRoot)
            << "\", \"file_count\": " << root.fileCount
            << ", \"skipped_file_count\": " << root.skippedFileCount << "}";
    }
    if (!rootStats.empty()) json << "\n  ";
    json << "],\n  \"unresolved_resource_count\": "
        << document.unresolvedResourceCount << "\n}\n";
    return json.str();
}

std::string BuildManifestJson(const SEOLRSkinDocument& document,
    int bundledAssetCount, int virtualFileCount, int skippedVirtualFileCount) {
    std::ostringstream json;
    json << "{\n"
        << "  \"format\": \"olrskin\",\n"
        << "  \"version\": " << kOlrFormatVersion << ",\n"
        << "  \"profile\": \"lr2-semantic-v0.8\",\n"
        << "  \"semantic_authority\": \"object parts + simple_mode\",\n"
        << "  \"lr2_entry\": \"lr2/main.lr2skin\",\n"
        << "  \"skin_entry\": \"skin.json\",\n"
        << "  \"path_map_entry\": \"compatibility/path-map.json\",\n"
        << "  \"object_count\": " << SemanticObjectCount(document) << ",\n"
        << "  \"part_count\": " << SemanticPartCount(document) << ",\n"
        << "  \"destination_count\": " << SemanticDestinationCount(document) << ",\n"
        << "  \"simple_slot_count\": " << document.simpleSlots.size() << ",\n"
        << "  \"asset_count\": " << bundledAssetCount << ",\n"
        << "  \"virtual_root_count\": " << document.virtualRoots.size() << ",\n"
        << "  \"virtual_file_count\": " << virtualFileCount << ",\n"
        << "  \"skipped_virtual_file_count\": " << skippedVirtualFileCount << ",\n"
        << "  \"unresolved_image_count\": " << document.unresolvedImageCount << ",\n"
        << "  \"unresolved_resource_count\": " << document.unresolvedResourceCount << ",\n"
        << "  \"limitations\": [\"SRC fields outside simple_mode remain compatibility-owned\", "
        << "\"IF/ELSE control flow remains compatibility-owned\", "
        << "\"resources outside captured LR2 roots remain external\"]\n"
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
        (asset.packagePath.rfind("lr2/assets/", 0) != 0 &&
            asset.packagePath.rfind("lr2/vfs/LR2files/", 0) != 0)) {
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

std::string LowerPathKey(const std::string& value) {
    std::string key = value;
    std::transform(key.begin(), key.end(), key.begin(),
        [](unsigned char ch) { return (char)std::tolower(ch); });
    return key;
}

bool IsOLRContainerArtifact(const std::filesystem::path& path) {
    const std::string filename = LowerPathKey(path.filename().string());
    const std::string packageSuffix = ".olrskin";
    const std::string temporarySuffix = ".olrskin.skineditor.tmp";
    return (filename.size() >= packageSuffix.size() &&
        filename.compare(filename.size() - packageSuffix.size(),
            packageSuffix.size(), packageSuffix) == 0) ||
        (filename.size() >= temporarySuffix.size() &&
            filename.compare(filename.size() - temporarySuffix.size(),
                temporarySuffix.size(), temporarySuffix) == 0);
}

bool AppendVirtualRootEntries(const SEOLRSkinDocument& document,
    std::vector<PackageEntrySource>& assetEntries,
    std::vector<VirtualRootPackageStats>& rootStats,
    int& virtualFileCount, int& skippedVirtualFileCount,
    std::string& errorMessage) {
    std::set<std::string> packageNames;
    for (const PackageEntrySource& entry : assetEntries)
        packageNames.insert(LowerPathKey(entry.name));

    for (const SEOLRVirtualRootInput& input : document.virtualRoots) {
        if (!IsSafeArchivePath(input.logicalRoot) ||
            input.logicalRoot.rfind("LR2files/", 0) != 0) {
            errorMessage = "An OLR virtual root was unsafe: " + input.logicalRoot;
            return false;
        }
        std::error_code filesystemError;
        const std::filesystem::path sourceRoot(input.sourceDirectory);
        if (!std::filesystem::is_directory(sourceRoot, filesystemError) ||
            filesystemError) {
            errorMessage = "An OLR virtual root could not be opened: " +
                input.sourceDirectory;
            return false;
        }

        VirtualRootPackageStats stats;
        stats.logicalRoot = input.logicalRoot;
        std::filesystem::recursive_directory_iterator iterator(sourceRoot,
            std::filesystem::directory_options::skip_permission_denied,
            filesystemError);
        const std::filesystem::recursive_directory_iterator end;
        while (!filesystemError && iterator != end) {
            const std::filesystem::directory_entry entry = *iterator;
            iterator.increment(filesystemError);
            std::error_code entryError;
            if (entry.is_symlink(entryError) || entryError ||
                !entry.is_regular_file(entryError) || entryError)
                continue;
            // OLR packages are transport containers, never LR2 runtime assets.
            // Excluding them prevents Save OLRskin -> import -> Save OLRskin
            // from recursively embedding the previous package.
            if (IsOLRContainerArtifact(entry.path())) {
                ++stats.skippedFileCount;
                ++skippedVirtualFileCount;
                continue;
            }
            const std::filesystem::path relative =
                entry.path().lexically_relative(sourceRoot);
            const std::string packagePath = "lr2/vfs/" + input.logicalRoot +
                "/" + relative.generic_string();
            if (!IsSafeArchivePath(packagePath)) {
                ++stats.skippedFileCount;
                ++skippedVirtualFileCount;
                continue;
            }
            if (!packageNames.insert(LowerPathKey(packagePath)).second) {
                errorMessage = "Two OLR virtual files map to the same Windows path: " +
                    packagePath;
                return false;
            }
            SEOLRAssetInput asset;
            asset.sourcePath = entry.path().string();
            asset.packagePath = packagePath;
            PackageEntrySource packageEntry;
            if (!PrepareFileEntry(asset, packageEntry, errorMessage)) return false;
            assetEntries.push_back(std::move(packageEntry));
            ++stats.fileCount;
            ++virtualFileCount;
        }
        if (filesystemError) {
            errorMessage = "An OLR virtual root could not be enumerated: " +
                input.sourceDirectory;
            return false;
        }
        rootStats.push_back(std::move(stats));
    }
    std::sort(assetEntries.begin(), assetEntries.end(),
        [](const PackageEntrySource& left, const PackageEntrySource& right) {
            return left.name < right.name;
        });
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
        if (!IsSafeArchivePath(entry.name) ||
            !names.emplace(LowerPathKey(entry.name), true).second) {
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
            !names.emplace(LowerPathKey(entry.name), true).second) {
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

struct SimpleModeCompileSlot {
    std::string id;
    std::string category;
    std::string sourceCommand;
    int sourceRow = -1;
    int graphicId = -1;
    int x = -1;
    int y = -1;
    int width = -1;
    int height = -1;
    int divX = -1;
    int divY = -1;
    int cycle = -1;
    bool hasCategory = false;
    bool hasSourceCommand = false;
    bool hasSourceRow = false;
    bool hasAsset = false;
};

struct SemanticCompileTransform {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    int rotation = 0;
    int blend = 0;
};

struct SemanticCompileFrame {
    int destinationRow = -1;
    int timeMs = 0;
    int alpha = 255;
    SemanticCompileTransform transform;
};

struct SemanticCompileConditionTerm {
    bool isRaw = false;
    bool isNegated = false;
    bool hasSlot = false;
    int slot = 0;
    int rawOption = 0;
    std::string lr2Name;
};

struct SemanticCompileSourceBinding {
    int sourceRow = -1;
    std::string sourceCommand;
};

struct SemanticCompileObject {
    std::string id;
    std::string parentId;
    std::string partKey;
    std::vector<SemanticCompileSourceBinding> sources;
    std::string destinationCommand;
    int layoutRow = -1;
    SemanticCompileTransform layout;
    std::vector<SemanticCompileFrame> frames;
    bool timerIsRaw = false;
    bool hasTimer = false;
    int rawTimer = 0;
    std::string timerName;
    bool hasLoop = false;
    int loop = 0;
    std::vector<SemanticCompileConditionTerm> conditions;
};

struct SemanticCompilePart {
    std::string id;
    std::vector<SemanticCompileSourceBinding> sources;
    std::vector<SemanticCompileObject> destinations;
};

struct ParsedOlrManifest {
    std::string format;
    std::string profile;
    std::string semanticAuthority;
    std::string lr2Entry;
    std::string skinEntry;
    std::string pathMapEntry;
    int version = -1;
    int objectCount = 0;
    int partCount = 0;
    int destinationCount = 0;
    int simpleSlotCount = 0;
    int assetCount = 0;
    int unresolvedImageCount = 0;
    int virtualRootCount = 0;
    int virtualFileCount = 0;
    int skippedVirtualFileCount = 0;
    int unresolvedResourceCount = 0;
    bool hasFormat = false;
    bool hasProfile = false;
    bool hasSemanticAuthority = false;
    bool hasVersion = false;
    bool hasLr2Entry = false;
    bool hasSkinEntry = false;
    bool hasPathMapEntry = false;
    bool hasObjectCount = false;
    bool hasPartCount = false;
    bool hasDestinationCount = false;
    bool hasSimpleSlotCount = false;
    bool hasAssetCount = false;
    bool hasUnresolvedImageCount = false;
    bool hasVirtualRootCount = false;
    bool hasVirtualFileCount = false;
    bool hasSkippedVirtualFileCount = false;
    bool hasUnresolvedResourceCount = false;
};

class SimpleModeJsonReader {
public:
    explicit SimpleModeJsonReader(const std::string& text) : text_(text) {}

    bool Read(std::string& authority, std::vector<SimpleModeCompileSlot>& slots,
        std::string& objectAuthority, std::vector<SemanticCompileObject>& objects,
        bool& hasObjects, std::string& errorMessage) {
        authority.clear();
        slots.clear();
        objectAuthority.clear();
        objects.clear();
        hasObjects = false;
        hasDocumentFormat_ = false;
        documentFormat_.clear();
        hasDocumentVersion_ = false;
        documentVersion_ = -1;
        SkipWhitespace();
        if (!Take('{')) return Fail("skin.json root must be an object.", errorMessage);
        bool foundSimpleMode = false;
        SkipWhitespace();
        if (!Take('}')) {
            for (;;) {
                std::string key;
                if (!ReadString(key, errorMessage) || !Require(':', errorMessage))
                    return false;
                if (key == "format") {
                    if (hasDocumentFormat_)
                        return Fail("skin.json contains duplicate format fields.", errorMessage);
                    hasDocumentFormat_ = true;
                    if (!ReadString(documentFormat_, errorMessage)) return false;
                }
                else if (key == "version") {
                    if (hasDocumentVersion_)
                        return Fail("skin.json contains duplicate version fields.", errorMessage);
                    hasDocumentVersion_ = true;
                    if (!ReadInteger(documentVersion_, errorMessage)) return false;
                }
                else if (key == "simple_mode") {
                    if (foundSimpleMode)
                        return Fail("skin.json contains duplicate simple_mode objects.", errorMessage);
                    foundSimpleMode = true;
                    if (!ReadSimpleMode(authority, slots, errorMessage)) return false;
                }
                else if (key == "objects") {
                    if (hasObjects)
                        return Fail("skin.json contains duplicate objects authorities.", errorMessage);
                    hasObjects = true;
                    if (!ReadSemanticObjects(objectAuthority, objects, errorMessage)) return false;
                }
                else if (!SkipValue(0, errorMessage)) return false;
                SkipWhitespace();
                if (Take('}')) break;
                if (!Require(',', errorMessage)) return false;
            }
        }
        SkipWhitespace();
        if (at_ != text_.size())
            return Fail("skin.json has trailing data after its root object.", errorMessage);
        if (!foundSimpleMode)
            return Fail("skin.json has no simple_mode object.", errorMessage);
        return true;
    }

    bool ReadManifest(ParsedOlrManifest& manifest,
        std::string& errorMessage) {
        manifest = ParsedOlrManifest();
        SkipWhitespace();
        if (!Take('{'))
            return Fail("manifest.json root must be an object.", errorMessage);
        std::set<std::string> keys;
        SkipWhitespace();
        if (!Take('}')) {
            for (;;) {
                std::string key;
                if (!ReadString(key, errorMessage) || !Require(':', errorMessage))
                    return false;
                if (!keys.insert(key).second)
                    return Fail("manifest.json contains a duplicate top-level field.",
                        errorMessage);
                int* count = nullptr;
                if (key == "format") {
                    manifest.hasFormat = true;
                    if (!ReadString(manifest.format, errorMessage)) return false;
                }
                else if (key == "version") {
                    manifest.hasVersion = true;
                    if (!ReadInteger(manifest.version, errorMessage)) return false;
                }
                else if (key == "profile") {
                    manifest.hasProfile = true;
                    if (!ReadString(manifest.profile, errorMessage)) return false;
                }
                else if (key == "semantic_authority") {
                    manifest.hasSemanticAuthority = true;
                    if (!ReadString(manifest.semanticAuthority, errorMessage)) return false;
                }
                else if (key == "lr2_entry") {
                    manifest.hasLr2Entry = true;
                    if (!ReadString(manifest.lr2Entry, errorMessage)) return false;
                }
                else if (key == "skin_entry") {
                    manifest.hasSkinEntry = true;
                    if (!ReadString(manifest.skinEntry, errorMessage)) return false;
                }
                else if (key == "path_map_entry") {
                    manifest.hasPathMapEntry = true;
                    if (!ReadString(manifest.pathMapEntry, errorMessage)) return false;
                }
                else if (key == "object_count") {
                    manifest.hasObjectCount = true;
                    count = &manifest.objectCount;
                }
                else if (key == "part_count") {
                    manifest.hasPartCount = true;
                    count = &manifest.partCount;
                }
                else if (key == "destination_count") {
                    manifest.hasDestinationCount = true;
                    count = &manifest.destinationCount;
                }
                else if (key == "simple_slot_count") {
                    manifest.hasSimpleSlotCount = true;
                    count = &manifest.simpleSlotCount;
                }
                else if (key == "asset_count") {
                    manifest.hasAssetCount = true;
                    count = &manifest.assetCount;
                }
                else if (key == "unresolved_image_count") {
                    manifest.hasUnresolvedImageCount = true;
                    count = &manifest.unresolvedImageCount;
                }
                else if (key == "virtual_root_count") {
                    manifest.hasVirtualRootCount = true;
                    count = &manifest.virtualRootCount;
                }
                else if (key == "virtual_file_count") {
                    manifest.hasVirtualFileCount = true;
                    count = &manifest.virtualFileCount;
                }
                else if (key == "skipped_virtual_file_count") {
                    manifest.hasSkippedVirtualFileCount = true;
                    count = &manifest.skippedVirtualFileCount;
                }
                else if (key == "unresolved_resource_count") {
                    manifest.hasUnresolvedResourceCount = true;
                    count = &manifest.unresolvedResourceCount;
                }

                if (count) {
                    if (!ReadInteger(*count, errorMessage)) return false;
                    if (*count < 0)
                        return Fail("manifest.json counts cannot be negative.",
                            errorMessage);
                }
                else if (key != "format" && key != "version" &&
                    key != "profile" && key != "semantic_authority" &&
                    key != "lr2_entry" && key != "skin_entry" &&
                    key != "path_map_entry" &&
                    !SkipValue(0, errorMessage)) return false;

                if (Take('}')) break;
                if (!Require(',', errorMessage)) return false;
            }
        }
        SkipWhitespace();
        if (at_ != text_.size())
            return Fail("manifest.json has trailing data after its root object.",
                errorMessage);
        if (!manifest.hasFormat || !manifest.hasVersion ||
            !manifest.hasLr2Entry || !manifest.hasSkinEntry)
            return Fail("manifest.json is missing a required field.", errorMessage);
        return true;
    }

    bool HasDocumentFormat() const { return hasDocumentFormat_; }
    const std::string& DocumentFormat() const { return documentFormat_; }
    bool HasDocumentVersion() const { return hasDocumentVersion_; }
    int DocumentVersion() const { return documentVersion_; }

private:
    const std::string& text_;
    size_t at_ = 0;
    bool semanticItemsUseParts_ = false;
    bool semanticItemsUseLegacy_ = false;
    std::set<std::string> semanticParentIds_;
    bool hasDocumentFormat_ = false;
    std::string documentFormat_;
    bool hasDocumentVersion_ = false;
    int documentVersion_ = -1;

    void SkipWhitespace() {
        while (at_ < text_.size() &&
            std::isspace((unsigned char)text_[at_])) ++at_;
    }

    bool Take(char expected) {
        SkipWhitespace();
        if (at_ >= text_.size() || text_[at_] != expected) return false;
        ++at_;
        return true;
    }

    bool Fail(const char* message, std::string& errorMessage) const {
        errorMessage = std::string(message) + " At byte " + std::to_string(at_) + ".";
        return false;
    }

    bool Require(char expected, std::string& errorMessage) {
        if (Take(expected)) return true;
        const std::string message = std::string("skin.json expected '") +
            expected + "'.";
        return Fail(message.c_str(), errorMessage);
    }

    bool ReadString(std::string& value, std::string& errorMessage) {
        SkipWhitespace();
        if (at_ >= text_.size() || text_[at_] != '"')
            return Fail("skin.json expected a string.", errorMessage);
        ++at_;
        value.clear();
        while (at_ < text_.size()) {
            const unsigned char ch = (unsigned char)text_[at_++];
            if (ch == '"') return true;
            if (ch < 0x20)
                return Fail("skin.json string contains a control character.", errorMessage);
            if (ch != '\\') {
                value.push_back((char)ch);
                continue;
            }
            if (at_ >= text_.size())
                return Fail("skin.json string ends inside an escape.", errorMessage);
            const char escaped = text_[at_++];
            switch (escaped) {
            case '"': value.push_back('"'); break;
            case '\\': value.push_back('\\'); break;
            case '/': value.push_back('/'); break;
            case 'b': value.push_back('\b'); break;
            case 'f': value.push_back('\f'); break;
            case 'n': value.push_back('\n'); break;
            case 'r': value.push_back('\r'); break;
            case 't': value.push_back('\t'); break;
            case 'u': {
                if (at_ + 4 > text_.size())
                    return Fail("skin.json has a truncated Unicode escape.", errorMessage);
                unsigned int codepoint = 0;
                for (int digit = 0; digit < 4; ++digit) {
                    const char hex = text_[at_++];
                    codepoint <<= 4;
                    if (hex >= '0' && hex <= '9') codepoint += hex - '0';
                    else if (hex >= 'a' && hex <= 'f') codepoint += hex - 'a' + 10;
                    else if (hex >= 'A' && hex <= 'F') codepoint += hex - 'A' + 10;
                    else return Fail("skin.json has an invalid Unicode escape.", errorMessage);
                }
                if (codepoint <= 0x7f) value.push_back((char)codepoint);
                else if (codepoint <= 0x7ff) {
                    value.push_back((char)(0xc0 | (codepoint >> 6)));
                    value.push_back((char)(0x80 | (codepoint & 0x3f)));
                }
                else {
                    value.push_back((char)(0xe0 | (codepoint >> 12)));
                    value.push_back((char)(0x80 | ((codepoint >> 6) & 0x3f)));
                    value.push_back((char)(0x80 | (codepoint & 0x3f)));
                }
                break;
            }
            default:
                return Fail("skin.json has an unsupported string escape.", errorMessage);
            }
        }
        return Fail("skin.json has an unterminated string.", errorMessage);
    }

    bool ReadInteger(int& value, std::string& errorMessage) {
        SkipWhitespace();
        const size_t start = at_;
        if (at_ < text_.size() && text_[at_] == '-') ++at_;
        const size_t digits = at_;
        if (at_ >= text_.size() || text_[at_] < '0' || text_[at_] > '9')
            return Fail("skin.json expected an integer.", errorMessage);
        if (text_[at_] == '0') {
            ++at_;
            if (at_ < text_.size() && text_[at_] >= '0' && text_[at_] <= '9')
                return Fail("JSON integers cannot contain leading zeroes.", errorMessage);
        }
        else {
            while (at_ < text_.size() && text_[at_] >= '0' && text_[at_] <= '9')
                ++at_;
        }
        if (digits == at_)
            return Fail("skin.json expected an integer.", errorMessage);
        if (at_ < text_.size() && (text_[at_] == '.' || text_[at_] == 'e' ||
            text_[at_] == 'E'))
            return Fail("Simple Mode compiler fields must be integers.", errorMessage);
        const long long parsed = _strtoi64(text_.c_str() + start, nullptr, 10);
        if (parsed < (std::numeric_limits<int>::min)() ||
            parsed > (std::numeric_limits<int>::max)())
            return Fail("Simple Mode compiler integer is out of range.", errorMessage);
        value = (int)parsed;
        return true;
    }

    bool ReadBoolean(bool& value, std::string& errorMessage) {
        SkipWhitespace();
        if (text_.compare(at_, 4, "true") == 0) {
            at_ += 4;
            value = true;
            return true;
        }
        if (text_.compare(at_, 5, "false") == 0) {
            at_ += 5;
            value = false;
            return true;
        }
        return Fail("skin.json expected a boolean.", errorMessage);
    }

    bool ReadNull(std::string& errorMessage) {
        SkipWhitespace();
        if (text_.compare(at_, 4, "null") != 0)
            return Fail("skin.json expected null.", errorMessage);
        at_ += 4;
        return true;
    }

    bool SkipValue(int depth, std::string& errorMessage) {
        if (depth > 64) return Fail("skin.json nesting is too deep.", errorMessage);
        SkipWhitespace();
        if (at_ >= text_.size())
            return Fail("skin.json ended before a value.", errorMessage);
        if (text_[at_] == '"') {
            std::string ignored;
            return ReadString(ignored, errorMessage);
        }
        if (text_[at_] == '{') {
            ++at_;
            SkipWhitespace();
            if (Take('}')) return true;
            for (;;) {
                std::string ignored;
                if (!ReadString(ignored, errorMessage) ||
                    !Require(':', errorMessage) ||
                    !SkipValue(depth + 1, errorMessage)) return false;
                if (Take('}')) return true;
                if (!Require(',', errorMessage)) return false;
            }
        }
        if (text_[at_] == '[') {
            ++at_;
            SkipWhitespace();
            if (Take(']')) return true;
            for (;;) {
                if (!SkipValue(depth + 1, errorMessage)) return false;
                if (Take(']')) return true;
                if (!Require(',', errorMessage)) return false;
            }
        }
        const char* literals[] = { "true", "false", "null" };
        for (const char* literal : literals) {
            const size_t length = strlen(literal);
            if (text_.compare(at_, length, literal) == 0) {
                at_ += length;
                return true;
            }
        }
        const size_t start = at_;
        if (text_[at_] == '-') ++at_;
        const size_t integerDigits = at_;
        while (at_ < text_.size() && text_[at_] >= '0' && text_[at_] <= '9') ++at_;
        if (integerDigits == at_)
            return Fail("skin.json contains an invalid number.", errorMessage);
        if (at_ < text_.size() && text_[at_] == '.') {
            ++at_;
            const size_t fractionDigits = at_;
            while (at_ < text_.size() && text_[at_] >= '0' && text_[at_] <= '9') ++at_;
            if (fractionDigits == at_)
                return Fail("skin.json contains an invalid number fraction.", errorMessage);
        }
        if (at_ < text_.size() && (text_[at_] == 'e' || text_[at_] == 'E')) {
            ++at_;
            if (at_ < text_.size() && (text_[at_] == '+' || text_[at_] == '-')) ++at_;
            const size_t exponentDigits = at_;
            while (at_ < text_.size() && text_[at_] >= '0' && text_[at_] <= '9') ++at_;
            if (exponentDigits == at_)
                return Fail("skin.json contains an invalid number exponent.", errorMessage);
        }
        if (at_ > start) return true;
        return Fail("skin.json contains an invalid value.", errorMessage);
    }

    bool ReadSemanticObjects(std::string& authority,
        std::vector<SemanticCompileObject>& objects, std::string& errorMessage) {
        if (!Take('{'))
            return Fail("skin.json objects authority must be an object.", errorMessage);
        bool foundAuthority = false;
        bool foundItems = false;
        if (!Take('}')) {
            for (;;) {
                std::string key;
                if (!ReadString(key, errorMessage) || !Require(':', errorMessage))
                    return false;
                if (key == "authority") {
                    if (foundAuthority)
                        return Fail("objects contains duplicate authority fields.", errorMessage);
                    foundAuthority = true;
                    if (!ReadString(authority, errorMessage)) return false;
                }
                else if (key == "items") {
                    if (foundItems)
                        return Fail("objects contains duplicate items fields.", errorMessage);
                    foundItems = true;
                    if (!ReadSemanticObjectArray(objects, errorMessage)) return false;
                }
                else if (!SkipValue(0, errorMessage)) return false;
                if (Take('}')) break;
                if (!Require(',', errorMessage)) return false;
            }
        }
        if (!foundAuthority || !foundItems)
            return Fail("objects requires authority and items.", errorMessage);
        if (authority == kSemanticObjectAuthority && semanticItemsUseLegacy_)
            return Fail("V0.8 objects authority cannot contain legacy flat destinations.",
                errorMessage);
        if (authority == kLegacySemanticObjectAuthority && semanticItemsUseParts_)
            return Fail("V0.7 objects authority cannot contain V0.8 parts.",
                errorMessage);
        return true;
    }

    bool ReadSemanticObjectArray(std::vector<SemanticCompileObject>& objects,
        std::string& errorMessage) {
        if (!Take('[')) return Fail("objects.items must be an array.", errorMessage);
        if (Take(']')) return true;
        for (;;) {
            if (!ReadSemanticItem(objects, errorMessage)) return false;
            if (Take(']')) return true;
            if (!Require(',', errorMessage)) return false;
        }
    }

    bool ReadSemanticItem(std::vector<SemanticCompileObject>& objects,
        std::string& errorMessage) {
        if (!Take('{')) return Fail("Each semantic Object must be an object.", errorMessage);
        SemanticCompileObject legacyObject;
        std::string parentId;
        std::vector<SemanticCompilePart> parts;
        bool foundId = false;
        bool foundParts = false;
        bool foundCommand = false;
        bool foundLayout = false;
        bool foundAnimation = false;
        bool foundCondition = false;
        if (!Take('}')) {
            for (;;) {
                std::string key;
                if (!ReadString(key, errorMessage) || !Require(':', errorMessage))
                    return false;
                if (key == "id") {
                    if (foundId) return Fail("A semantic Object contains duplicate id fields.", errorMessage);
                    foundId = true;
                    if (!ReadString(parentId, errorMessage)) return false;
                    legacyObject.id = parentId;
                    legacyObject.parentId = parentId;
                }
                else if (key == "parts") {
                    if (foundParts)
                        return Fail("A semantic Object contains duplicate parts fields.", errorMessage);
                    foundParts = true;
                    if (!ReadSemanticParts(parts, errorMessage)) return false;
                }
                else if (key == "destination_command") {
                    if (foundCommand)
                        return Fail("A semantic Object contains duplicate destination_command fields.", errorMessage);
                    foundCommand = true;
                    if (!ReadString(legacyObject.destinationCommand, errorMessage)) return false;
                }
                else if (key == "layout") {
                    if (foundLayout) return Fail("A semantic Object contains duplicate layout fields.", errorMessage);
                    foundLayout = true;
                    if (!ReadLayout(legacyObject, errorMessage)) return false;
                }
                else if (key == "animation") {
                    if (foundAnimation)
                        return Fail("A semantic Object contains duplicate animation fields.", errorMessage);
                    foundAnimation = true;
                    if (!ReadAnimation(legacyObject.frames, errorMessage)) return false;
                }
                else if (key == "condition") {
                    if (foundCondition)
                        return Fail("A semantic Object contains duplicate condition fields.", errorMessage);
                    foundCondition = true;
                    if (!ReadCondition(legacyObject, errorMessage)) return false;
                }
                else if (!SkipValue(0, errorMessage)) return false;
                if (Take('}')) break;
                if (!Require(',', errorMessage)) return false;
            }
        }
        if (!foundId || parentId.empty())
            return Fail("A semantic Object requires a non-empty id.", errorMessage);
        if (!semanticParentIds_.insert(parentId).second)
            return Fail("Two semantic Objects use the same parent id.", errorMessage);
        const bool hasLegacyCompilerFields = foundCommand || foundLayout ||
            foundAnimation || foundCondition;
        if (foundParts) {
            if (hasLegacyCompilerFields)
                return Fail("A V0.8 semantic Object cannot mix parts and flat destination fields.",
                    errorMessage);
            if (parts.empty())
                return Fail("A V0.8 semantic Object requires at least one part.", errorMessage);
            semanticItemsUseParts_ = true;
            std::set<std::string> partIds;
            for (SemanticCompilePart& part : parts) {
                if (!partIds.insert(part.id).second)
                    return Fail("A semantic Object contains duplicate part ids.", errorMessage);
                if (part.destinations.empty())
                    return Fail("Each semantic part requires a destination.", errorMessage);
                std::set<std::string> destinationIds;
                const std::string partKey = parentId + "/" + part.id;
                for (SemanticCompileObject& destination : part.destinations) {
                    if (!destinationIds.insert(destination.id).second)
                        return Fail("A semantic part contains duplicate destination ids.",
                            errorMessage);
                    destination.parentId = parentId;
                    destination.partKey = partKey;
                    destination.sources = part.sources;
                    destination.id = partKey + "/" + destination.id;
                    objects.push_back(std::move(destination));
                }
            }
            return true;
        }
        if (!foundCommand || !foundLayout || !foundAnimation ||
            !foundCondition || legacyObject.destinationCommand.empty() ||
            legacyObject.frames.empty())
            return Fail("A semantic Object is missing a compiler field.", errorMessage);
        semanticItemsUseLegacy_ = true;
        objects.push_back(std::move(legacyObject));
        return true;
    }

    bool ReadSemanticParts(std::vector<SemanticCompilePart>& parts,
        std::string& errorMessage) {
        if (!Take('[')) return Fail("Object parts must be an array.", errorMessage);
        if (Take(']')) return true;
        for (;;) {
            SemanticCompilePart part;
            if (!ReadSemanticPart(part, errorMessage)) return false;
            parts.push_back(std::move(part));
            if (Take(']')) return true;
            if (!Require(',', errorMessage)) return false;
        }
    }

    bool ReadSemanticPart(SemanticCompilePart& part,
        std::string& errorMessage) {
        if (!Take('{')) return Fail("Each semantic part must be an object.", errorMessage);
        bool foundId = false;
        bool foundSources = false;
        bool foundDestinations = false;
        if (!Take('}')) {
            for (;;) {
                std::string key;
                if (!ReadString(key, errorMessage) || !Require(':', errorMessage))
                    return false;
                if (key == "id") {
                    if (foundId) return Fail("A semantic part contains duplicate id fields.", errorMessage);
                    foundId = true;
                    if (!ReadString(part.id, errorMessage)) return false;
                }
                else if (key == "sources") {
                    if (foundSources)
                        return Fail("A semantic part contains duplicate sources fields.", errorMessage);
                    foundSources = true;
                    if (!ReadSourceBindings(part.sources, errorMessage)) return false;
                }
                else if (key == "destinations") {
                    if (foundDestinations)
                        return Fail("A semantic part contains duplicate destinations fields.", errorMessage);
                    foundDestinations = true;
                    if (!ReadDestinationArray(part.destinations, errorMessage)) return false;
                }
                else if (!SkipValue(0, errorMessage)) return false;
                if (Take('}')) break;
                if (!Require(',', errorMessage)) return false;
            }
        }
        if (!foundId || part.id.empty() || !foundSources || !foundDestinations)
            return Fail("A semantic part requires id, sources and destinations.", errorMessage);
        return true;
    }

    bool ReadSourceBindings(std::vector<SemanticCompileSourceBinding>& sources,
        std::string& errorMessage) {
        if (!Take('[')) return Fail("Part sources must be an array.", errorMessage);
        if (Take(']')) return true;
        for (;;) {
            SemanticCompileSourceBinding source;
            if (!ReadSourceBinding(source, errorMessage)) return false;
            sources.push_back(std::move(source));
            if (Take(']')) return true;
            if (!Require(',', errorMessage)) return false;
        }
    }

    bool ReadSourceBinding(SemanticCompileSourceBinding& source,
        std::string& errorMessage) {
        if (!Take('{')) return Fail("Each part source must be an object.", errorMessage);
        bool foundRow = false;
        bool foundCommand = false;
        if (!Take('}')) {
            for (;;) {
                std::string key;
                if (!ReadString(key, errorMessage) || !Require(':', errorMessage))
                    return false;
                if (key == "source_row") {
                    if (foundRow) return Fail("A part source contains duplicate source_row fields.", errorMessage);
                    foundRow = true;
                    if (!ReadInteger(source.sourceRow, errorMessage)) return false;
                }
                else if (key == "source_command") {
                    if (foundCommand)
                        return Fail("A part source contains duplicate source_command fields.", errorMessage);
                    foundCommand = true;
                    if (!ReadString(source.sourceCommand, errorMessage)) return false;
                }
                else if (!SkipValue(0, errorMessage)) return false;
                if (Take('}')) break;
                if (!Require(',', errorMessage)) return false;
            }
        }
        if (!foundRow || !foundCommand || source.sourceRow <= 0 ||
            source.sourceCommand.compare(0, 5, "#SRC_") != 0)
            return Fail("A part source requires a positive row and #SRC_* command.",
                errorMessage);
        return true;
    }

    bool ReadDestinationArray(std::vector<SemanticCompileObject>& destinations,
        std::string& errorMessage) {
        if (!Take('[')) return Fail("Part destinations must be an array.", errorMessage);
        if (Take(']')) return true;
        for (;;) {
            SemanticCompileObject destination;
            if (!ReadSemanticDestination(destination, errorMessage)) return false;
            destinations.push_back(std::move(destination));
            if (Take(']')) return true;
            if (!Require(',', errorMessage)) return false;
        }
    }

    bool ReadSemanticDestination(SemanticCompileObject& destination,
        std::string& errorMessage) {
        if (!Take('{')) return Fail("Each semantic destination must be an object.", errorMessage);
        bool foundId = false;
        bool foundCommand = false;
        bool foundLayout = false;
        bool foundAnimation = false;
        bool foundCondition = false;
        if (!Take('}')) {
            for (;;) {
                std::string key;
                if (!ReadString(key, errorMessage) || !Require(':', errorMessage))
                    return false;
                if (key == "id") {
                    if (foundId) return Fail("A semantic destination contains duplicate id fields.", errorMessage);
                    foundId = true;
                    if (!ReadString(destination.id, errorMessage)) return false;
                }
                else if (key == "destination_command") {
                    if (foundCommand)
                        return Fail("A semantic destination contains duplicate destination_command fields.", errorMessage);
                    foundCommand = true;
                    if (!ReadString(destination.destinationCommand, errorMessage)) return false;
                }
                else if (key == "layout") {
                    if (foundLayout) return Fail("A semantic destination contains duplicate layout fields.", errorMessage);
                    foundLayout = true;
                    if (!ReadLayout(destination, errorMessage)) return false;
                }
                else if (key == "animation") {
                    if (foundAnimation)
                        return Fail("A semantic destination contains duplicate animation fields.", errorMessage);
                    foundAnimation = true;
                    if (!ReadAnimation(destination.frames, errorMessage)) return false;
                }
                else if (key == "condition") {
                    if (foundCondition)
                        return Fail("A semantic destination contains duplicate condition fields.", errorMessage);
                    foundCondition = true;
                    if (!ReadCondition(destination, errorMessage)) return false;
                }
                else if (!SkipValue(0, errorMessage)) return false;
                if (Take('}')) break;
                if (!Require(',', errorMessage)) return false;
            }
        }
        if (!foundId || destination.id.empty() || !foundCommand ||
            !foundLayout || !foundAnimation || !foundCondition ||
            destination.destinationCommand.empty() || destination.frames.empty())
            return Fail("A semantic destination is missing a compiler field.",
                errorMessage);
        return true;
    }

    bool ReadLayout(SemanticCompileObject& object, std::string& errorMessage) {
        if (!Take('{')) return Fail("Object layout must be an object.", errorMessage);
        bool foundRow = false;
        bool foundTransform = false;
        if (!Take('}')) {
            for (;;) {
                std::string key;
                if (!ReadString(key, errorMessage) || !Require(':', errorMessage)) return false;
                if (key == "destination_row") {
                    if (foundRow) return Fail("Object layout contains duplicate destination_row fields.", errorMessage);
                    foundRow = true;
                    if (!ReadInteger(object.layoutRow, errorMessage)) return false;
                }
                else if (key == "transform") {
                    if (foundTransform) return Fail("Object layout contains duplicate transform fields.", errorMessage);
                    foundTransform = true;
                    if (!ReadTransform(object.layout, errorMessage)) return false;
                }
                else if (!SkipValue(0, errorMessage)) return false;
                if (Take('}')) break;
                if (!Require(',', errorMessage)) return false;
            }
        }
        if (!foundRow || !foundTransform)
            return Fail("Object layout requires destination_row and transform.", errorMessage);
        return true;
    }

    bool ReadTransform(SemanticCompileTransform& transform,
        std::string& errorMessage) {
        if (!Take('{')) return Fail("A semantic transform must be an object.", errorMessage);
        bool fields[6] = {};
        if (!Take('}')) {
            for (;;) {
                std::string key;
                if (!ReadString(key, errorMessage) || !Require(':', errorMessage)) return false;
                int* destination = nullptr;
                int field = -1;
                if (key == "x") { destination = &transform.x; field = 0; }
                else if (key == "y") { destination = &transform.y; field = 1; }
                else if (key == "width") { destination = &transform.width; field = 2; }
                else if (key == "height") { destination = &transform.height; field = 3; }
                else if (key == "rotation") { destination = &transform.rotation; field = 4; }
                else if (key == "blend") { destination = &transform.blend; field = 5; }
                if (destination) {
                    if (fields[field]) return Fail("A semantic transform contains a duplicate field.", errorMessage);
                    fields[field] = true;
                    if (!ReadInteger(*destination, errorMessage)) return false;
                }
                else if (!SkipValue(0, errorMessage)) return false;
                if (Take('}')) break;
                if (!Require(',', errorMessage)) return false;
            }
        }
        for (bool present : fields)
            if (!present) return Fail("A semantic transform is missing a compiler field.", errorMessage);
        return true;
    }

    bool ReadAnimation(std::vector<SemanticCompileFrame>& frames,
        std::string& errorMessage) {
        if (!Take('{')) return Fail("Object animation must be an object.", errorMessage);
        bool foundFrames = false;
        if (!Take('}')) {
            for (;;) {
                std::string key;
                if (!ReadString(key, errorMessage) || !Require(':', errorMessage)) return false;
                if (key == "frames") {
                    if (foundFrames) return Fail("Object animation contains duplicate frames fields.", errorMessage);
                    foundFrames = true;
                    if (!ReadFrameArray(frames, errorMessage)) return false;
                }
                else if (!SkipValue(0, errorMessage)) return false;
                if (Take('}')) break;
                if (!Require(',', errorMessage)) return false;
            }
        }
        if (!foundFrames) return Fail("Object animation requires frames.", errorMessage);
        return true;
    }

    bool ReadFrameArray(std::vector<SemanticCompileFrame>& frames,
        std::string& errorMessage) {
        if (!Take('[')) return Fail("animation.frames must be an array.", errorMessage);
        if (Take(']')) return true;
        for (;;) {
            SemanticCompileFrame frame;
            if (!ReadFrame(frame, errorMessage)) return false;
            frames.push_back(frame);
            if (Take(']')) return true;
            if (!Require(',', errorMessage)) return false;
        }
    }

    bool ReadFrame(SemanticCompileFrame& frame, std::string& errorMessage) {
        if (!Take('{')) return Fail("Each animation frame must be an object.", errorMessage);
        bool foundRow = false;
        bool foundTime = false;
        bool foundAlpha = false;
        bool foundTransform = false;
        if (!Take('}')) {
            for (;;) {
                std::string key;
                if (!ReadString(key, errorMessage) || !Require(':', errorMessage)) return false;
                if (key == "destination_row") {
                    if (foundRow) return Fail("An animation frame contains duplicate destination_row fields.", errorMessage);
                    foundRow = true;
                    if (!ReadInteger(frame.destinationRow, errorMessage)) return false;
                }
                else if (key == "time_ms") {
                    if (foundTime) return Fail("An animation frame contains duplicate time_ms fields.", errorMessage);
                    foundTime = true;
                    if (!ReadInteger(frame.timeMs, errorMessage)) return false;
                }
                else if (key == "alpha") {
                    if (foundAlpha) return Fail("An animation frame contains duplicate alpha fields.", errorMessage);
                    foundAlpha = true;
                    if (!ReadInteger(frame.alpha, errorMessage)) return false;
                }
                else if (key == "transform") {
                    if (foundTransform) return Fail("An animation frame contains duplicate transform fields.", errorMessage);
                    foundTransform = true;
                    if (!ReadTransform(frame.transform, errorMessage)) return false;
                }
                else if (!SkipValue(0, errorMessage)) return false;
                if (Take('}')) break;
                if (!Require(',', errorMessage)) return false;
            }
        }
        if (!foundRow || !foundTime || !foundAlpha || !foundTransform)
            return Fail("An animation frame is missing a compiler field.", errorMessage);
        return true;
    }

    bool ReadCondition(SemanticCompileObject& object, std::string& errorMessage) {
        if (!Take('{')) return Fail("Object condition must be an object.", errorMessage);
        bool foundMode = false;
        bool foundTimer = false;
        bool foundLoop = false;
        bool foundAll = false;
        if (!Take('}')) {
            for (;;) {
                std::string key;
                if (!ReadString(key, errorMessage) || !Require(':', errorMessage)) return false;
                if (key == "mode") {
                    if (foundMode) return Fail("Object condition contains duplicate mode fields.", errorMessage);
                    foundMode = true;
                    std::string mode;
                    if (!ReadString(mode, errorMessage)) return false;
                    if (mode != "all") return Fail("Only all-mode LR2 conditions are supported.", errorMessage);
                }
                else if (key == "timer") {
                    if (foundTimer) return Fail("Object condition contains duplicate timer fields.", errorMessage);
                    foundTimer = true;
                    SkipWhitespace();
                    if (text_.compare(at_, 4, "null") == 0) {
                        if (!ReadNull(errorMessage)) return false;
                        object.hasTimer = false;
                    }
                    else if (!ReadTimer(object, errorMessage)) return false;
                }
                else if (key == "loop") {
                    if (foundLoop) return Fail("Object condition contains duplicate loop fields.", errorMessage);
                    foundLoop = true;
                    SkipWhitespace();
                    if (text_.compare(at_, 4, "null") == 0) {
                        if (!ReadNull(errorMessage)) return false;
                        object.hasLoop = false;
                    }
                    else {
                        if (!ReadInteger(object.loop, errorMessage)) return false;
                        object.hasLoop = true;
                    }
                }
                else if (key == "all") {
                    if (foundAll) return Fail("Object condition contains duplicate all fields.", errorMessage);
                    foundAll = true;
                    if (!ReadConditionTerms(object.conditions, errorMessage)) return false;
                }
                else if (!SkipValue(0, errorMessage)) return false;
                if (Take('}')) break;
                if (!Require(',', errorMessage)) return false;
            }
        }
        if (!foundMode || !foundTimer || !foundLoop || !foundAll)
            return Fail("Object condition requires mode, timer, loop and all.", errorMessage);
        if (object.conditions.size() > 3)
            return Fail("LR2 destinations support at most three OP conditions.", errorMessage);
        return true;
    }

    bool ReadTimer(SemanticCompileObject& object, std::string& errorMessage) {
        if (!Take('{')) return Fail("Condition timer must be an object.", errorMessage);
        std::string kind;
        bool foundKind = false;
        bool foundName = false;
        bool foundRaw = false;
        if (!Take('}')) {
            for (;;) {
                std::string key;
                if (!ReadString(key, errorMessage) || !Require(':', errorMessage)) return false;
                if (key == "kind") {
                    if (foundKind) return Fail("Condition timer contains duplicate kind fields.", errorMessage);
                    foundKind = true;
                    if (!ReadString(kind, errorMessage)) return false;
                }
                else if (key == "lr2_name") {
                    if (foundName) return Fail("Condition timer contains duplicate lr2_name fields.", errorMessage);
                    foundName = true;
                    if (!ReadString(object.timerName, errorMessage)) return false;
                }
                else if (key == "lr2_timer") {
                    if (foundRaw) return Fail("Condition timer contains duplicate lr2_timer fields.", errorMessage);
                    foundRaw = true;
                    if (!ReadInteger(object.rawTimer, errorMessage)) return false;
                }
                else if (!SkipValue(0, errorMessage)) return false;
                if (Take('}')) break;
                if (!Require(',', errorMessage)) return false;
            }
        }
        if (!foundKind || (kind != "semantic" && kind != "raw"))
            return Fail("Condition timer kind must be semantic or raw.", errorMessage);
        object.timerIsRaw = kind == "raw";
        if ((object.timerIsRaw && !foundRaw) || (!object.timerIsRaw && !foundName))
            return Fail("Condition timer is missing its LR2 identity.", errorMessage);
        object.hasTimer = true;
        return true;
    }

    bool ReadConditionTerms(std::vector<SemanticCompileConditionTerm>& terms,
        std::string& errorMessage) {
        if (!Take('[')) return Fail("condition.all must be an array.", errorMessage);
        if (Take(']')) return true;
        for (;;) {
            SemanticCompileConditionTerm term;
            if (!ReadConditionTerm(term, errorMessage)) return false;
            terms.push_back(term);
            if (Take(']')) return true;
            if (!Require(',', errorMessage)) return false;
        }
    }

    bool ReadConditionTerm(SemanticCompileConditionTerm& term,
        std::string& errorMessage) {
        if (!Take('{')) return Fail("Each condition term must be an object.", errorMessage);
        std::string kind;
        bool foundKind = false;
        bool foundSlot = false;
        bool foundName = false;
        bool foundRaw = false;
        bool foundNegated = false;
        if (!Take('}')) {
            for (;;) {
                std::string key;
                if (!ReadString(key, errorMessage) || !Require(':', errorMessage)) return false;
                if (key == "slot") {
                    if (foundSlot)
                        return Fail("A condition term contains duplicate slot fields.", errorMessage);
                    foundSlot = true;
                    term.hasSlot = true;
                    if (!ReadInteger(term.slot, errorMessage)) return false;
                }
                else if (key == "kind") {
                    if (foundKind) return Fail("A condition term contains duplicate kind fields.", errorMessage);
                    foundKind = true;
                    if (!ReadString(kind, errorMessage)) return false;
                }
                else if (key == "lr2_name") {
                    if (foundName) return Fail("A condition term contains duplicate lr2_name fields.", errorMessage);
                    foundName = true;
                    if (!ReadString(term.lr2Name, errorMessage)) return false;
                }
                else if (key == "lr2_op") {
                    if (foundRaw) return Fail("A condition term contains duplicate lr2_op fields.", errorMessage);
                    foundRaw = true;
                    if (!ReadInteger(term.rawOption, errorMessage)) return false;
                }
                else if (key == "negated") {
                    if (foundNegated) return Fail("A condition term contains duplicate negated fields.", errorMessage);
                    foundNegated = true;
                    if (!ReadBoolean(term.isNegated, errorMessage)) return false;
                }
                else if (!SkipValue(0, errorMessage)) return false;
                if (Take('}')) break;
                if (!Require(',', errorMessage)) return false;
            }
        }
        if (!foundKind || (kind != "semantic" && kind != "raw"))
            return Fail("Condition term kind must be semantic or raw.", errorMessage);
        term.isRaw = kind == "raw";
        if ((term.isRaw && !foundRaw) || (!term.isRaw && (!foundName || !foundNegated)))
            return Fail("A condition term is missing its LR2 identity.", errorMessage);
        return true;
    }

    bool ReadSimpleMode(std::string& authority,
        std::vector<SimpleModeCompileSlot>& slots, std::string& errorMessage) {
        if (!Take('{'))
            return Fail("skin.json simple_mode must be an object.", errorMessage);
        bool foundAuthority = false;
        bool foundSlots = false;
        if (!Take('}')) {
            for (;;) {
                std::string key;
                if (!ReadString(key, errorMessage) || !Require(':', errorMessage))
                    return false;
                if (key == "authority") {
                    if (foundAuthority)
                        return Fail("simple_mode contains duplicate authority fields.", errorMessage);
                    foundAuthority = true;
                    if (!ReadString(authority, errorMessage)) return false;
                }
                else if (key == "slots") {
                    if (foundSlots)
                        return Fail("simple_mode contains duplicate slots fields.", errorMessage);
                    foundSlots = true;
                    if (!ReadSlots(slots, errorMessage)) return false;
                }
                else if (!SkipValue(0, errorMessage)) return false;
                if (Take('}')) break;
                if (!Require(',', errorMessage)) return false;
            }
        }
        if (!foundAuthority || !foundSlots)
            return Fail("simple_mode requires authority and slots.", errorMessage);
        return true;
    }

    bool ReadSlots(std::vector<SimpleModeCompileSlot>& slots,
        std::string& errorMessage) {
        if (!Take('['))
            return Fail("simple_mode.slots must be an array.", errorMessage);
        if (Take(']')) return true;
        for (;;) {
            SimpleModeCompileSlot slot;
            if (!ReadSlot(slot, errorMessage)) return false;
            slots.push_back(std::move(slot));
            if (Take(']')) return true;
            if (!Require(',', errorMessage)) return false;
        }
    }

    bool ReadSlot(SimpleModeCompileSlot& slot, std::string& errorMessage) {
        if (!Take('{'))
            return Fail("Each simple_mode slot must be an object.", errorMessage);
        if (!Take('}')) {
            for (;;) {
                std::string key;
                if (!ReadString(key, errorMessage) || !Require(':', errorMessage))
                    return false;
                if (key == "id") {
                    if (!ReadString(slot.id, errorMessage)) return false;
                }
                else if (key == "category") {
                    if (slot.hasCategory)
                        return Fail("A simple_mode slot contains duplicate category fields.", errorMessage);
                    slot.hasCategory = true;
                    if (!ReadString(slot.category, errorMessage)) return false;
                }
                else if (key == "source_command") {
                    if (slot.hasSourceCommand)
                        return Fail("A simple_mode slot contains duplicate source_command fields.", errorMessage);
                    slot.hasSourceCommand = true;
                    if (!ReadString(slot.sourceCommand, errorMessage)) return false;
                }
                else if (key == "source_row") {
                    if (slot.hasSourceRow)
                        return Fail("A simple_mode slot contains duplicate source_row fields.", errorMessage);
                    slot.hasSourceRow = true;
                    if (!ReadInteger(slot.sourceRow, errorMessage)) return false;
                }
                else if (key == "asset") {
                    if (slot.hasAsset)
                        return Fail("A simple_mode slot contains duplicate asset fields.", errorMessage);
                    slot.hasAsset = true;
                    if (!ReadAsset(slot, errorMessage)) return false;
                }
                else if (!SkipValue(0, errorMessage)) return false;
                if (Take('}')) break;
                if (!Require(',', errorMessage)) return false;
            }
        }
        if (!slot.hasCategory || !slot.hasSourceCommand ||
            !slot.hasSourceRow || !slot.hasAsset)
            return Fail("A simple_mode slot is missing a compiler field.", errorMessage);
        return true;
    }

    bool ReadAsset(SimpleModeCompileSlot& slot, std::string& errorMessage) {
        if (!Take('{'))
            return Fail("A simple_mode slot asset must be an object.", errorMessage);
        bool fields[8] = {};
        if (!Take('}')) {
            for (;;) {
                std::string key;
                if (!ReadString(key, errorMessage) || !Require(':', errorMessage))
                    return false;
                int* destination = nullptr;
                int field = -1;
                if (key == "gr") { destination = &slot.graphicId; field = 0; }
                else if (key == "x") { destination = &slot.x; field = 1; }
                else if (key == "y") { destination = &slot.y; field = 2; }
                else if (key == "width") { destination = &slot.width; field = 3; }
                else if (key == "height") { destination = &slot.height; field = 4; }
                else if (key == "div_x") { destination = &slot.divX; field = 5; }
                else if (key == "div_y") { destination = &slot.divY; field = 6; }
                else if (key == "cycle") { destination = &slot.cycle; field = 7; }
                if (destination) {
                    if (fields[field])
                        return Fail("A simple_mode asset contains a duplicate field.", errorMessage);
                    fields[field] = true;
                    if (!ReadInteger(*destination, errorMessage)) return false;
                }
                else if (!SkipValue(0, errorMessage)) return false;
                if (Take('}')) break;
                if (!Require(',', errorMessage)) return false;
            }
        }
        for (bool present : fields)
            if (!present)
                return Fail("A simple_mode asset is missing a compiler field.", errorMessage);
        return true;
    }
};

bool ParseOLRManifestJson(const std::string& manifestJson,
    SEOLRPackageInfo& packageInfo, std::string& errorMessage) {
    packageInfo = SEOLRPackageInfo();
    errorMessage.clear();
    ParsedOlrManifest manifest;
    SimpleModeJsonReader reader(manifestJson);
    if (!reader.ReadManifest(manifest, errorMessage)) return false;
    if (manifest.format != "olrskin" || manifest.version < 1 ||
        manifest.version > kOlrFormatVersion ||
        manifest.lr2Entry != "lr2/main.lr2skin" ||
        manifest.skinEntry != "skin.json") {
        errorMessage = "The OLR manifest format, version or entry references are unsupported.";
        return false;
    }
    if (manifest.version >= 2 && (!manifest.hasPathMapEntry ||
        manifest.pathMapEntry != "compatibility/path-map.json")) {
        errorMessage = "The OLR V0.2+ manifest has no supported path map reference.";
        return false;
    }
    if (manifest.version == 8 && (!manifest.hasProfile ||
        manifest.profile != "lr2-semantic-v0.8" ||
        !manifest.hasSemanticAuthority ||
        manifest.semanticAuthority != "object parts + simple_mode" ||
        !manifest.hasObjectCount || !manifest.hasPartCount ||
        !manifest.hasDestinationCount || !manifest.hasSimpleSlotCount ||
        !manifest.hasAssetCount || !manifest.hasVirtualRootCount ||
        !manifest.hasVirtualFileCount ||
        !manifest.hasSkippedVirtualFileCount ||
        !manifest.hasUnresolvedImageCount ||
        !manifest.hasUnresolvedResourceCount)) {
        errorMessage = "The OLR V0.8 manifest profile, semantic authority or required counts are unsupported.";
        return false;
    }
    packageInfo.formatVersion = manifest.version;
    packageInfo.objectCount = manifest.objectCount;
    packageInfo.semanticPartCount = manifest.partCount;
    packageInfo.destinationCount = manifest.destinationCount;
    packageInfo.simpleSlotCount = manifest.simpleSlotCount;
    packageInfo.assetCount = manifest.assetCount;
    packageInfo.unresolvedImageCount = manifest.unresolvedImageCount;
    packageInfo.virtualRootCount = manifest.virtualRootCount;
    packageInfo.virtualFileCount = manifest.virtualFileCount;
    packageInfo.skippedVirtualFileCount = manifest.skippedVirtualFileCount;
    packageInfo.unresolvedResourceCount = manifest.unresolvedResourceCount;
    return true;
}

struct PreservedScriptLine {
    std::string content;
    std::string ending;
};

std::vector<PreservedScriptLine> SplitPreservedScriptLines(
    const std::string& script) {
    std::vector<PreservedScriptLine> lines;
    size_t start = 0;
    while (start < script.size()) {
        const size_t end = script.find_first_of("\r\n", start);
        PreservedScriptLine line;
        if (end == std::string::npos) {
            line.content = script.substr(start);
            lines.push_back(std::move(line));
            break;
        }
        line.content = script.substr(start, end - start);
        size_t next = end + 1;
        if (script[end] == '\r' && next < script.size() && script[next] == '\n')
            ++next;
        line.ending = script.substr(end, next - end);
        lines.push_back(std::move(line));
        start = next;
    }
    if (script.empty()) lines.push_back(PreservedScriptLine());
    return lines;
}

std::vector<std::string> SplitSimpleCsv(const std::string& line) {
    std::vector<std::string> fields;
    size_t start = 0;
    for (;;) {
        const size_t comma = line.find(',', start);
        fields.push_back(line.substr(start, comma == std::string::npos
            ? std::string::npos : comma - start));
        if (comma == std::string::npos) break;
        start = comma + 1;
    }
    return fields;
}

bool StartsWith(const std::string& value, const char* prefix) {
    return prefix && value.compare(0, strlen(prefix), prefix) == 0;
}

bool IsCompilableSimpleModeCommand(const SimpleModeCompileSlot& slot) {
    const std::string& command = slot.sourceCommand;
    if (slot.category == "number-fonts")
        return command == "#SRC_NUMBER" || StartsWith(command, "#SRC_NOWCOMBO_");
    if (slot.category == "judgement-fonts")
        return StartsWith(command, "#SRC_NOWJUDGE_");
    if (slot.category == "notes")
        return command == "#SRC_NOTE" || command == "#SRC_MINE" ||
            StartsWith(command, "#SRC_LN_") || command == "#SRC_AUTO_NOTE" ||
            command == "#SRC_AUTO_MINE" || StartsWith(command, "#SRC_AUTO_LN_");
    if (slot.category == "gear")
        return command == "#SRC_IMAGE" || command == "#SRC_LINE" ||
            command == "#SRC_JUDGELINE";
    if (slot.category == "gauge")
        return command == "#SRC_GROOVEGAUGE" ||
            command == "#SRC_SCORECHART" ||
            StartsWith(command, "#SRC_GAUGECHART_");
    return false;
}

bool IsSafeSimpleModeAssetValues(int graphicId, int x, int y, int width,
    int height, int divX, int divY, int cycle) {
    return graphicId >= 0 && graphicId <= 99 && x >= 0 && y >= 0 &&
        width > 0 && height > 0 && divX > 0 && divY > 0 && cycle >= 0 &&
        x <= 1000000 && y <= 1000000 && width <= 1000000 &&
        height <= 1000000 && divX <= 100000 && divY <= 100000 &&
        cycle <= 1000000000;
}

int FindCompilerFieldColumn(const std::string& command, const char* fieldName) {
    for (int column = 1; column < 30; ++column) {
        CSTR help = GetCommandHelp(command.c_str(), column);
        help.trimWhiteSpace();
        const char* label = help.body ? help.outstr() : "";
        if (*label == '$') ++label;
        if (_stricmp(label, fieldName) == 0) return column;
    }
    return -1;
}

int FindTransformFieldColumn(const std::string& command, const char* fieldName) {
    int column = FindCompilerFieldColumn(command, fieldName);
    if (column < 0 && _stricmp(fieldName, "h") == 0)
        column = FindCompilerFieldColumn(command, "size");
    return column;
}

int FindOptionByName(const std::string& name) {
    for (int option = 0; option < 900; ++option) {
        const char* candidate = dstName((unsigned)option);
        if (candidate[0] && name == candidate) return option;
    }
    return -1;
}

int FindTimerByName(const std::string& name) {
    for (int timer = 0; timer < 900; ++timer) {
        const char* candidate = timerName((unsigned)timer);
        if (candidate[0] && name == candidate) return timer;
    }
    return -1;
}

bool SameTransform(const SemanticCompileTransform& left,
    const SemanticCompileTransform& right) {
    return left.x == right.x && left.y == right.y &&
        left.width == right.width && left.height == right.height &&
        left.rotation == right.rotation && left.blend == right.blend;
}

bool IsSafeSemanticTransform(const SemanticCompileTransform& transform) {
    const int values[] = { transform.x, transform.y, transform.width,
        transform.height, transform.rotation, transform.blend };
    for (int value : values)
        if (value < -1000000 || value > 1000000) return false;
    return true;
}

bool AssignCompiledField(std::vector<std::string>& fields,
    const std::string& command, const char* fieldName, int value,
    bool required, std::string& errorMessage) {
    const int column = FindTransformFieldColumn(command, fieldName);
    if (column < 0) {
        if (!required) return true;
        errorMessage = "The LR2 destination schema has no semantic field '" +
            std::string(fieldName) + "' for " + command + ".";
        return false;
    }
    if (column >= (int)fields.size()) fields.resize((size_t)column + 1);
    if (!required && value == 0 && fields[(size_t)column].empty())
        return true;
    fields[(size_t)column] = std::to_string(value);
    return true;
}

void JoinCompiledFields(PreservedScriptLine& line,
    const std::vector<std::string>& fields) {
    std::ostringstream rebuilt;
    for (size_t field = 0; field < fields.size(); ++field) {
        if (field) rebuilt << ',';
        rebuilt << fields[field];
    }
    line.content = rebuilt.str();
}

bool WriteTextFileAtomic(const std::filesystem::path& path,
    const std::string& bytes, std::string& errorMessage) {
    const std::filesystem::path temporary = path.string() + ".olr-compile.tmp";
    DeleteFileA(temporary.string().c_str());
    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
    if (!output) {
        errorMessage = "The OLR compiler could not create its temporary LR2 script.";
        return false;
    }
    output.write(bytes.data(), (std::streamsize)bytes.size());
    output.close();
    if (!output) {
        DeleteFileA(temporary.string().c_str());
        errorMessage = "The OLR compiler could not write the complete LR2 script.";
        return false;
    }
    if (!MoveFileExA(temporary.string().c_str(), path.string().c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        DeleteFileA(temporary.string().c_str());
        errorMessage = "The OLR compiler could not atomically replace the LR2 script.";
        return false;
    }
    return true;
}

bool ValidateManifest(FILE* archive,
    const std::vector<PackageEntryRecord>& entries, SEOLRPackageInfo& info,
    std::string& errorMessage) {
    const PackageEntryRecord* manifest = nullptr;
    const PackageEntryRecord* skin = nullptr;
    bool hasSourceMap = false;
    bool hasPathMap = false;
    bool hasMain = false;
    bool hasExportMain = false;
    for (const PackageEntryRecord& entry : entries) {
        info.entries.push_back(entry.name);
        if (entry.name == "manifest.json") manifest = &entry;
        else if (entry.name == "skin.json") skin = &entry;
        else if (entry.name == "compatibility/source-map.json") hasSourceMap = true;
        else if (entry.name == "compatibility/path-map.json") hasPathMap = true;
        else if (entry.name == "lr2/main.lr2skin") hasMain = true;
        else if (entry.name == "lr2/.olr-export-main.txt") hasExportMain = true;
        if (entry.name.rfind("lr2/assets/", 0) == 0 ||
            entry.name.rfind("lr2/vfs/", 0) == 0)
            ++info.assetCount;
        if (entry.name.rfind("lr2/vfs/", 0) == 0) ++info.virtualFileCount;
    }
    if (!manifest || !skin || !hasSourceMap || !hasMain) {
        errorMessage = "The OLR package is missing a required entry.";
        return false;
    }
    std::vector<unsigned char> bytes;
    if (!CopyAndValidateEntry(archive, *manifest, nullptr, &bytes, errorMessage))
        return false;
    const std::string text(bytes.begin(), bytes.end());
    SEOLRPackageInfo manifestInfo;
    if (!ParseOLRManifestJson(text, manifestInfo, errorMessage)) return false;
    if (manifestInfo.formatVersion >= 2 &&
        (!hasPathMap || !hasExportMain)) {
        errorMessage = "The OLR V0.2+ package is missing its virtual path metadata.";
        return false;
    }
    if (manifestInfo.formatVersion == 8) {
        if (info.assetCount != manifestInfo.assetCount ||
            info.virtualFileCount != manifestInfo.virtualFileCount) {
            errorMessage = "The OLR V0.8 manifest asset counts do not match the archive.";
            return false;
        }

        std::vector<unsigned char> skinBytes;
        if (!CopyAndValidateEntry(archive, *skin, nullptr, &skinBytes,
            errorMessage)) return false;
        const std::string skinJson(skinBytes.begin(), skinBytes.end());
        std::string simpleAuthority;
        std::vector<SimpleModeCompileSlot> slots;
        std::string objectAuthority;
        std::vector<SemanticCompileObject> objects;
        bool hasObjects = false;
        SimpleModeJsonReader skinReader(skinJson);
        if (!skinReader.Read(simpleAuthority, slots, objectAuthority, objects,
            hasObjects, errorMessage)) return false;
        if (!skinReader.HasDocumentFormat() ||
            skinReader.DocumentFormat() != "olrskin-semantic" ||
            !skinReader.HasDocumentVersion() ||
            skinReader.DocumentVersion() != 8 ||
            simpleAuthority != kSimpleModeAuthority || !hasObjects ||
            objectAuthority != kSemanticObjectAuthority) {
            errorMessage = "The OLR V0.8 manifest and skin.json authorities do not match.";
            return false;
        }

        std::set<std::string> parentIds;
        std::set<std::string> partKeys;
        for (const SemanticCompileObject& object : objects) {
            parentIds.insert(object.parentId);
            partKeys.insert(object.partKey);
        }
        if (manifestInfo.objectCount != static_cast<int>(parentIds.size()) ||
            manifestInfo.semanticPartCount != static_cast<int>(partKeys.size()) ||
            manifestInfo.destinationCount != static_cast<int>(objects.size()) ||
            manifestInfo.simpleSlotCount != static_cast<int>(slots.size())) {
            errorMessage = "The OLR V0.8 manifest semantic counts do not match skin.json.";
            return false;
        }
    }
    else if (manifestInfo.formatVersion >= 4) {
        std::vector<unsigned char> skinBytes;
        if (!CopyAndValidateEntry(archive, *skin, nullptr, &skinBytes,
            errorMessage)) return false;
        const std::string skinJson(skinBytes.begin(), skinBytes.end());
        std::string simpleAuthority;
        std::vector<SimpleModeCompileSlot> slots;
        std::string objectAuthority;
        std::vector<SemanticCompileObject> objects;
        bool hasObjects = false;
        SimpleModeJsonReader skinReader(skinJson);
        if (!skinReader.Read(simpleAuthority, slots, objectAuthority, objects,
            hasObjects, errorMessage)) return false;
        // V0.4 manifests historically contain a version-3 skin document.
        // Reserve version 8+ documents for the V0.8 authority without
        // imposing equality on the older manifest/document version pairs.
        if ((skinReader.HasDocumentVersion() &&
            skinReader.DocumentVersion() >= 8) ||
            simpleAuthority != kSimpleModeAuthority ||
            (hasObjects && objectAuthority != kLegacySemanticObjectAuthority)) {
            errorMessage = "The pre-V0.8 manifest and skin.json authorities do not match.";
            return false;
        }
    }
    info.formatVersion = manifestInfo.formatVersion;
    info.objectCount = manifestInfo.objectCount;
    info.semanticPartCount = manifestInfo.semanticPartCount;
    info.destinationCount = manifestInfo.destinationCount;
    info.simpleSlotCount = manifestInfo.simpleSlotCount;
    info.unresolvedImageCount = manifestInfo.unresolvedImageCount;
    info.virtualRootCount = manifestInfo.virtualRootCount;
    info.skippedVirtualFileCount = manifestInfo.skippedVirtualFileCount;
    info.unresolvedResourceCount = manifestInfo.unresolvedResourceCount;
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

bool SEParseOLRManifestJson(const std::string& manifestJson,
    SEOLRPackageInfo& packageInfo, std::string& errorMessage) {
    return ParseOLRManifestJson(manifestJson, packageInfo, errorMessage);
}

bool SEIsOLRSimpleSlotCompilable(const SEOLRSimpleSlot& slot) {
    return IsSafeSimpleModeAssetValues(slot.graphicId, slot.x, slot.y,
        slot.width, slot.height, slot.divX, slot.divY, slot.cycle);
}

bool SECompileOLRSimpleMode(const std::string& skinJson,
    const std::string& lr2Script, std::string& compiledScript,
    int& compiledSlotCount, std::string& errorMessage) {
    compiledScript.clear();
    compiledSlotCount = 0;
    errorMessage.clear();

    std::string authority;
    std::vector<SimpleModeCompileSlot> slots;
    std::string objectAuthority;
    std::vector<SemanticCompileObject> objects;
    bool hasObjects = false;
    SimpleModeJsonReader reader(skinJson);
    if (!reader.Read(authority, slots, objectAuthority, objects,
        hasObjects, errorMessage)) return false;
    if (authority != kSimpleModeAuthority) {
        errorMessage = "skin.json simple_mode authority is not supported: " +
            authority + ".";
        return false;
    }

    std::vector<PreservedScriptLine> lines = SplitPreservedScriptLines(lr2Script);
    std::set<int> compiledRows;
    int validatedSlotCount = 0;
    for (const SimpleModeCompileSlot& slot : slots) {
        if (!IsCompilableSimpleModeCommand(slot)) {
            errorMessage = "Simple Mode slot category/command is not compilable: " +
                slot.category + " / " + slot.sourceCommand + ".";
            return false;
        }
        if (slot.sourceRow <= 0 || slot.sourceRow > (int)lines.size()) {
            errorMessage = "Simple Mode source_row is outside lr2/main.lr2skin: " +
                std::to_string(slot.sourceRow) + ".";
            return false;
        }
        if (!compiledRows.insert(slot.sourceRow).second) {
            errorMessage = "Two Simple Mode slots target LR2 row " +
                std::to_string(slot.sourceRow) + ".";
            return false;
        }

        PreservedScriptLine& line = lines[(size_t)slot.sourceRow - 1];
        std::vector<std::string> fields = SplitSimpleCsv(line.content);
        if (fields.size() < 10 || _stricmp(fields[0].c_str(),
            slot.sourceCommand.c_str()) != 0) {
            errorMessage = "Simple Mode row/command mismatch at LR2 row " +
                std::to_string(slot.sourceRow) + ": expected " +
                slot.sourceCommand + ".";
            return false;
        }
        // LR2 skins in the wild use negative source crop sizes as engine-owned
        // sentinels. After validating their row identity, keep those rows raw
        // because they are not editable Simple Mode atlas rectangles.
        if (!IsSafeSimpleModeAssetValues(slot.graphicId, slot.x, slot.y,
            slot.width, slot.height, slot.divX, slot.divY, slot.cycle))
            continue;
        const int values[] = { slot.graphicId, slot.x, slot.y, slot.width,
            slot.height, slot.divX, slot.divY, slot.cycle };
        for (int field = 0; field < 8; ++field)
            fields[(size_t)field + 2] = std::to_string(values[field]);
        std::ostringstream rebuilt;
        for (size_t field = 0; field < fields.size(); ++field) {
            if (field) rebuilt << ',';
            rebuilt << fields[field];
        }
        line.content = rebuilt.str();
        ++validatedSlotCount;
    }

    std::ostringstream output;
    for (const PreservedScriptLine& line : lines)
        output << line.content << line.ending;
    compiledScript = output.str();
    compiledSlotCount = validatedSlotCount;
    return true;
}

bool SECompileOLRSemantics(const std::string& skinJson,
    const std::string& lr2Script, std::string& compiledScript,
    int& compiledSlotCount, int& compiledObjectCount,
    int& compiledAnimationFrameCount, std::string& errorMessage) {
    compiledScript.clear();
    compiledSlotCount = 0;
    compiledObjectCount = 0;
    compiledAnimationFrameCount = 0;
    errorMessage.clear();

    std::string simpleCompiled;
    int simpleCount = 0;
    if (!SECompileOLRSimpleMode(skinJson, lr2Script, simpleCompiled,
        simpleCount, errorMessage)) return false;

    std::string simpleAuthority;
    std::vector<SimpleModeCompileSlot> slots;
    std::string objectAuthority;
    std::vector<SemanticCompileObject> objects;
    bool hasObjects = false;
    SimpleModeJsonReader reader(skinJson);
    if (!reader.Read(simpleAuthority, slots, objectAuthority, objects,
        hasObjects, errorMessage)) return false;
    if (!hasObjects) {
        compiledScript = simpleCompiled;
        compiledSlotCount = simpleCount;
        return true;
    }
    const bool usesPartAuthority = objectAuthority == kSemanticObjectAuthority;
    const bool usesLegacyAuthority =
        objectAuthority == kLegacySemanticObjectAuthority;
    if (!usesPartAuthority && !usesLegacyAuthority) {
        errorMessage = "skin.json objects authority is not supported: " +
            objectAuthority + ".";
        return false;
    }
    if (usesPartAuthority && (!reader.HasDocumentFormat() ||
        reader.DocumentFormat() != "olrskin-semantic" ||
        !reader.HasDocumentVersion() || reader.DocumentVersion() != 8)) {
        errorMessage = "V0.8 part objects require an olrskin-semantic version 8 document.";
        return false;
    }
    if (usesLegacyAuthority && reader.HasDocumentVersion() &&
        reader.DocumentVersion() >= 8) {
        errorMessage = "A version 8 semantic document cannot use the V0.7 flat object authority.";
        return false;
    }
    if (arr_CommandHelp.count <= 0 && LoadCommandHelp(nullptr) != 0) {
        errorMessage = "The embedded LR2 command schema could not be loaded.";
        return false;
    }

    std::vector<PreservedScriptLine> lines = SplitPreservedScriptLines(simpleCompiled);
    std::set<int> compiledRows;
    std::set<std::string> objectIds;
    std::set<std::string> compiledParentIds;
    std::map<int, std::string> sourcePartByRow;
    int pendingObjectCount = 0;
    int pendingAnimationFrameCount = 0;
    for (const SemanticCompileObject& object : objects) {
        if (!objectIds.insert(object.id).second) {
            errorMessage = "Two semantic Objects use the id '" + object.id + "'.";
            return false;
        }
        if (object.destinationCommand.compare(0, 5, "#DST_") != 0) {
            errorMessage = "Semantic Object destination_command is not #DST_*: " +
                object.destinationCommand + ".";
            return false;
        }
        for (const SemanticCompileSourceBinding& source : object.sources) {
            if (source.sourceRow <= 0 || source.sourceRow > (int)lines.size()) {
                errorMessage = "Semantic source_row is outside lr2/main.lr2skin: " +
                    std::to_string(source.sourceRow) + ".";
                return false;
            }
            const std::vector<std::string> sourceFields = SplitSimpleCsv(
                lines[(size_t)source.sourceRow - 1].content);
            if (sourceFields.empty() || _stricmp(sourceFields[0].c_str(),
                source.sourceCommand.c_str()) != 0) {
                errorMessage = "Semantic source row/command mismatch at LR2 row " +
                    std::to_string(source.sourceRow) + ": expected " +
                    source.sourceCommand + ".";
                return false;
            }
            const auto existingSource = sourcePartByRow.find(source.sourceRow);
            if (existingSource != sourcePartByRow.end() &&
                existingSource->second != object.partKey) {
                errorMessage = "Two semantic parts bind LR2 source row " +
                    std::to_string(source.sourceRow) + ".";
                return false;
            }
            sourcePartByRow[source.sourceRow] = object.partKey;
        }
        if (object.layoutRow != object.frames.front().destinationRow ||
            !SameTransform(object.layout, object.frames.front().transform)) {
            errorMessage = "Semantic Object layout must match animation frame 0: " +
                object.id + ".";
            return false;
        }
        if (!IsSafeSemanticTransform(object.layout) ||
            (object.hasLoop && (object.loop < -1000000 ||
                object.loop > 1000000000))) {
            errorMessage = "Semantic Object layout or loop is outside the safe LR2 range: " +
                object.id + ".";
            return false;
        }

        if (usesLegacyAuthority && (!object.hasTimer || !object.hasLoop)) {
            errorMessage = "V0.7 semantic conditions require timer and loop values.";
            return false;
        }
        int timer = object.rawTimer;
        if (object.hasTimer && !object.timerIsRaw) {
            timer = FindTimerByName(object.timerName);
            if (timer < 0) {
                errorMessage = "Semantic timer name is unknown: " + object.timerName + ".";
                return false;
            }
        }
        int options[3] = {};
        bool hasOptions[3] = {};
        for (size_t index = 0; index < object.conditions.size(); ++index) {
            const SemanticCompileConditionTerm& condition = object.conditions[index];
            int optionSlot = (int)index;
            if (usesPartAuthority) {
                if (!condition.hasSlot || condition.slot < 1 || condition.slot > 3) {
                    errorMessage = "V0.8 condition terms require a slot from 1 to 3: " +
                        object.id + ".";
                    return false;
                }
                optionSlot = condition.slot - 1;
                if (hasOptions[optionSlot]) {
                    errorMessage = "A V0.8 destination contains duplicate OP slots: " +
                        object.id + ".";
                    return false;
                }
            }
            else if (condition.hasSlot) {
                errorMessage = "V0.7 condition terms cannot use V0.8 slots.";
                return false;
            }
            int option = condition.rawOption;
            if (!condition.isRaw) {
                option = FindOptionByName(condition.lr2Name);
                if (option < 0) {
                    errorMessage = "Semantic LR2 option name is unknown: " +
                        condition.lr2Name + ".";
                    return false;
                }
                if (condition.isNegated) option = -option;
            }
            if (option < -999 || option > 999) {
                errorMessage = "Raw LR2 OP is outside the supported range for " +
                    object.id + ".";
                return false;
            }
            options[optionSlot] = option;
            hasOptions[optionSlot] = true;
        }

        for (size_t frameIndex = 0; frameIndex < object.frames.size(); ++frameIndex) {
            const SemanticCompileFrame& frame = object.frames[frameIndex];
            if (frame.destinationRow <= 0 || frame.destinationRow > (int)lines.size()) {
                errorMessage = "Semantic destination_row is outside lr2/main.lr2skin: " +
                    std::to_string(frame.destinationRow) + ".";
                return false;
            }
            if (!compiledRows.insert(frame.destinationRow).second) {
                errorMessage = "Two semantic frames target LR2 row " +
                    std::to_string(frame.destinationRow) + ".";
                return false;
            }
            if (!IsSafeSemanticTransform(frame.transform) || frame.timeMs < 0 ||
                frame.timeMs > 1000000000 || frame.alpha < 0 || frame.alpha > 255) {
                errorMessage = "Semantic animation values are outside the safe LR2 range: " +
                    object.id + ".";
                return false;
            }

            PreservedScriptLine& line = lines[(size_t)frame.destinationRow - 1];
            std::vector<std::string> fields = SplitSimpleCsv(line.content);
            if (fields.empty() || _stricmp(fields[0].c_str(),
                object.destinationCommand.c_str()) != 0) {
                errorMessage = "Semantic row/command mismatch at LR2 row " +
                    std::to_string(frame.destinationRow) + ": expected " +
                    object.destinationCommand + ".";
                return false;
            }
            if (!AssignCompiledField(fields, object.destinationCommand, "time",
                frame.timeMs, true, errorMessage) ||
                !AssignCompiledField(fields, object.destinationCommand, "x",
                    frame.transform.x, true, errorMessage) ||
                !AssignCompiledField(fields, object.destinationCommand, "y",
                    frame.transform.y, true, errorMessage) ||
                !AssignCompiledField(fields, object.destinationCommand, "w",
                    frame.transform.width, true, errorMessage) ||
                !AssignCompiledField(fields, object.destinationCommand, "h",
                    frame.transform.height, true, errorMessage) ||
                !AssignCompiledField(fields, object.destinationCommand, "a",
                    frame.alpha, true, errorMessage) ||
                !AssignCompiledField(fields, object.destinationCommand, "angle",
                    frame.transform.rotation, true, errorMessage) ||
                !AssignCompiledField(fields, object.destinationCommand, "blend",
                    frame.transform.blend, true, errorMessage))
                return false;

            if (frameIndex == 0) {
                if (object.hasLoop && !AssignCompiledField(fields,
                    object.destinationCommand, "loop", object.loop, true,
                    errorMessage)) return false;
                if (object.hasTimer && !AssignCompiledField(fields,
                    object.destinationCommand, "timer", timer, true,
                    errorMessage)) return false;
                for (int optionIndex = 0; optionIndex < 3; ++optionIndex) {
                    if (usesPartAuthority && !hasOptions[optionIndex]) continue;
                    const std::string fieldName = "op" + std::to_string(optionIndex + 1);
                    const bool required = options[optionIndex] != 0;
                    if (!AssignCompiledField(fields, object.destinationCommand,
                        fieldName.c_str(), options[optionIndex], required,
                        errorMessage)) return false;
                }
            }
            JoinCompiledFields(line, fields);
            ++pendingAnimationFrameCount;
        }
        if (usesPartAuthority) compiledParentIds.insert(object.parentId);
        else ++pendingObjectCount;
    }
    if (usesPartAuthority)
        pendingObjectCount = static_cast<int>(compiledParentIds.size());

    std::ostringstream output;
    for (const PreservedScriptLine& line : lines)
        output << line.content << line.ending;
    compiledScript = output.str();
    compiledSlotCount = simpleCount;
    compiledObjectCount = pendingObjectCount;
    compiledAnimationFrameCount = pendingAnimationFrameCount;
    return true;
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
    if (!IsSafeArchivePath(document.lr2ExportMainPath) ||
        document.lr2ExportMainPath.rfind("LR2files/", 0) != 0 ||
        document.lr2ExportMainPath.find('*') != std::string::npos ||
        document.lr2ExportMainPath.find('?') != std::string::npos) {
        errorMessage = "The OLR package has no safe LR2 export destination for its main skin.";
        return false;
    }

    std::vector<PackageEntrySource> assetEntries;
    assetEntries.reserve(document.assets.size());
    for (const SEOLRAssetInput& asset : document.assets) {
        PackageEntrySource entry;
        if (!PrepareFileEntry(asset, entry, errorMessage)) return false;
        assetEntries.push_back(std::move(entry));
    }

    std::vector<VirtualRootPackageStats> rootStats;
    int virtualFileCount = 0;
    int skippedVirtualFileCount = 0;
    if (!AppendVirtualRootEntries(document, assetEntries, rootStats,
        virtualFileCount, skippedVirtualFileCount, errorMessage))
        return false;

    std::vector<PackageEntrySource> entries;
    entries.push_back(MemoryEntry("manifest.json",
        BuildManifestJson(document, (int)assetEntries.size(), virtualFileCount,
            skippedVirtualFileCount)));
    entries.push_back(MemoryEntry("skin.json", BuildSkinJson(document)));
    entries.push_back(MemoryEntry("compatibility/source-map.json",
        BuildSourceMapJson(document)));
    entries.push_back(MemoryEntry("compatibility/path-map.json",
        BuildPathMapJson(document, rootStats)));
    entries.push_back(MemoryEntry("lr2/main.lr2skin", document.lr2Script));
    entries.push_back(MemoryEntry("lr2/.olr-export-main.txt",
        document.lr2ExportMainPath + "\n"));
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
    std::vector<unsigned char> semanticBytes;
    for (const PackageEntryRecord& entry : entries) {
        std::vector<unsigned char>* captured =
            packageInfo.formatVersion >= 4 && entry.name == "skin.json"
            ? &semanticBytes : nullptr;
        if (!CopyAndValidateEntry(archive, entry, nullptr, captured, errorMessage)) {
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
    if (packageInfo.formatVersion >= 4) {
        std::ifstream input(mainSkinPath, std::ios::binary);
        const std::string lr2Script((std::istreambuf_iterator<char>(input)),
            std::istreambuf_iterator<char>());
        if (!input && !input.eof()) {
            std::filesystem::remove_all(target, filesystemError);
            mainSkinPath.clear();
            errorMessage = "The extracted LR2 script could not be read for semantic compilation.";
            return false;
        }
        input.close();
        const std::string skinJson(semanticBytes.begin(), semanticBytes.end());
        std::string compiledScript;
        int compiledSlotCount = 0;
        int compiledObjectCount = 0;
        int compiledAnimationFrameCount = 0;
        if (!SECompileOLRSemantics(skinJson, lr2Script, compiledScript,
            compiledSlotCount, compiledObjectCount, compiledAnimationFrameCount,
            errorMessage) ||
            !WriteTextFileAtomic(mainSkinPath, compiledScript, errorMessage)) {
            std::filesystem::remove_all(target, filesystemError);
            mainSkinPath.clear();
            return false;
        }
        packageInfo.compiledSimpleSlotCount = compiledSlotCount;
        packageInfo.compiledSemanticObjectCount = compiledObjectCount;
        packageInfo.compiledAnimationFrameCount = compiledAnimationFrameCount;
    }
    return true;
}

bool SEIsOLRVirtualWorkspace(const char* mainSkinPath) {
    if (!mainSkinPath || !*mainSkinPath) return false;
    std::error_code error;
    const std::filesystem::path workspace =
        std::filesystem::path(mainSkinPath).parent_path();
    return std::filesystem::is_regular_file(
        workspace / ".olr-export-main.txt", error) && !error;
}

bool SEExportOLRWorkspaceToLR2(const char* mainSkinPath,
    const char* outputDirectory, SEOLRLr2ExportInfo& exportInfo,
    std::string& errorMessage) {
    exportInfo = SEOLRLr2ExportInfo();
    errorMessage.clear();
    if (!mainSkinPath || !*mainSkinPath || !outputDirectory || !*outputDirectory) {
        errorMessage = "Choose an extracted OLR workspace and LR2 export folder.";
        return false;
    }

    const std::filesystem::path mainPath(mainSkinPath);
    const std::filesystem::path workspace = mainPath.parent_path();
    const std::filesystem::path markerPath = workspace / ".olr-export-main.txt";
    std::ifstream marker(markerPath, std::ios::binary);
    std::string exportMain((std::istreambuf_iterator<char>(marker)),
        std::istreambuf_iterator<char>());
    while (!exportMain.empty() &&
        (exportMain.back() == '\r' || exportMain.back() == '\n'))
        exportMain.pop_back();
    if (!marker || !IsSafeArchivePath(exportMain) ||
        exportMain.rfind("LR2files/", 0) != 0 ||
        exportMain.find('*') != std::string::npos ||
        exportMain.find('?') != std::string::npos) {
        errorMessage = "The OLR workspace has invalid LR2 export metadata.";
        return false;
    }

    std::ifstream scriptFile(mainPath, std::ios::binary);
    const std::string script((std::istreambuf_iterator<char>(scriptFile)),
        std::istreambuf_iterator<char>());
    if (!scriptFile || script.empty()) {
        errorMessage = "The OLR workspace main skin could not be read.";
        return false;
    }

    std::error_code filesystemError;
    const std::filesystem::path target(outputDirectory);
    if (std::filesystem::exists(target, filesystemError)) {
        errorMessage = "The LR2 export folder already exists. Choose a new destination.";
        return false;
    }
    if (!std::filesystem::create_directories(target, filesystemError) ||
        filesystemError) {
        errorMessage = "The LR2 export folder could not be created.";
        return false;
    }

    bool ok = true;
    const std::filesystem::path vfsRoot = workspace / "vfs" / "LR2files";
    if (std::filesystem::is_directory(vfsRoot, filesystemError) && !filesystemError) {
        std::filesystem::recursive_directory_iterator iterator(vfsRoot,
            std::filesystem::directory_options::skip_permission_denied,
            filesystemError);
        const std::filesystem::recursive_directory_iterator end;
        while (!filesystemError && iterator != end) {
            const std::filesystem::directory_entry entry = *iterator;
            iterator.increment(filesystemError);
            std::error_code entryError;
            if (entry.is_symlink(entryError) || entryError ||
                !entry.is_regular_file(entryError) || entryError)
                continue;
            const std::filesystem::path relative =
                entry.path().lexically_relative(vfsRoot);
            const std::filesystem::path destination = target / "LR2files" / relative;
            std::filesystem::create_directories(destination.parent_path(), entryError);
            if (entryError || !std::filesystem::copy_file(entry.path(), destination,
                std::filesystem::copy_options::none, entryError) || entryError) {
                errorMessage = "A virtual LR2 resource could not be materialized: " +
                    relative.generic_string();
                ok = false;
                break;
            }
            ++exportInfo.copiedFileCount;
        }
        if (filesystemError && ok) {
            errorMessage = "The virtual LR2 resource tree could not be enumerated.";
            ok = false;
        }
    }
    else filesystemError.clear();

    std::string compiledScript = script;
    const auto restoreVirtualPaths = [&](const std::string& virtualPrefix) {
        const std::string lr2Prefix = "LR2files\\";
        size_t position = 0;
        while ((position = compiledScript.find(virtualPrefix, position)) !=
            std::string::npos) {
            compiledScript.replace(position, virtualPrefix.size(), lr2Prefix);
            const size_t pathStart = position + lr2Prefix.size();
            const size_t pathEnd = compiledScript.find_first_of(",\r\n", pathStart);
            const size_t stop = pathEnd == std::string::npos ?
                compiledScript.size() : pathEnd;
            for (size_t index = pathStart; index < stop; ++index) {
                if (compiledScript[index] == '/') compiledScript[index] = '\\';
            }
            position = stop;
        }
    };
    restoreVirtualPaths("vfs/LR2files/");
    restoreVirtualPaths("vfs\\LR2files\\");

    if (ok) {
        const std::filesystem::path compiledMain = target /
            std::filesystem::path(exportMain);
        // Fixed assets are stored beside the extracted compatibility script as
        // assets/*. The compiled LR2 main keeps those relative declarations,
        // so materialize the folder beside the final main skin as well.
        const std::filesystem::path assetRoot = workspace / "assets";
        if (std::filesystem::is_directory(assetRoot, filesystemError) &&
            !filesystemError) {
            std::filesystem::recursive_directory_iterator iterator(assetRoot,
                std::filesystem::directory_options::skip_permission_denied,
                filesystemError);
            const std::filesystem::recursive_directory_iterator end;
            while (!filesystemError && iterator != end) {
                const std::filesystem::directory_entry entry = *iterator;
                iterator.increment(filesystemError);
                std::error_code entryError;
                if (entry.is_symlink(entryError) || entryError ||
                    !entry.is_regular_file(entryError) || entryError)
                    continue;
                const std::filesystem::path relative =
                    entry.path().lexically_relative(assetRoot);
                const std::filesystem::path destination =
                    compiledMain.parent_path() / "assets" / relative;
                std::filesystem::create_directories(destination.parent_path(),
                    entryError);
                if (entryError || !std::filesystem::copy_file(entry.path(),
                    destination, std::filesystem::copy_options::none,
                    entryError) || entryError) {
                    errorMessage = "A packaged skin asset could not be materialized: " +
                        relative.generic_string();
                    ok = false;
                    break;
                }
                ++exportInfo.copiedFileCount;
            }
            if (filesystemError && ok) {
                errorMessage = "The packaged skin asset tree could not be enumerated.";
                ok = false;
            }
        }
        else filesystemError.clear();
        if (ok) {
            std::filesystem::create_directories(compiledMain.parent_path(), filesystemError);
            FILE* output = filesystemError ? nullptr : OpenFile(compiledMain.string().c_str(), "wb");
            bool writeOk = output != nullptr;
            if (writeOk && fwrite(compiledScript.data(), 1, compiledScript.size(), output) !=
                compiledScript.size())
                writeOk = false;
            if (output && fclose(output) != 0) writeOk = false;
            if (!writeOk) {
                errorMessage = "The compiled LR2 main skin could not be written.";
                ok = false;
            }
            else exportInfo.mainSkinPath = compiledMain.string();
        }
    }

    if (!ok) {
        std::filesystem::remove_all(target, filesystemError);
        exportInfo = SEOLRLr2ExportInfo();
        return false;
    }
    return true;
}
