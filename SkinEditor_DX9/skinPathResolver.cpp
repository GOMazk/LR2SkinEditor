#include "skinPathResolver.h"

#include <Windows.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <set>
#include <vector>

namespace {

bool EqualsIgnoreCase(const std::string& left, const std::string& right) {
    return _stricmp(left.c_str(), right.c_str()) == 0;
}

bool StartsWithIgnoreCase(const std::string& value, const char* prefix) {
    const size_t prefixLength = strlen(prefix);
    return value.size() >= prefixLength &&
        _strnicmp(value.c_str(), prefix, prefixLength) == 0;
}

std::vector<std::string> SplitPath(const std::string& path) {
    std::vector<std::string> segments;
    size_t start = 0;
    while (start <= path.size()) {
        const size_t slash = path.find('/', start);
        const size_t end = slash == std::string::npos ? path.size() : slash;
        segments.push_back(path.substr(start, end - start));
        if (slash == std::string::npos) break;
        start = slash + 1;
    }
    return segments;
}

std::string JoinSegments(const std::vector<std::string>& segments,
    size_t begin, size_t end) {
    std::string result;
    for (size_t index = begin; index < end; ++index) {
        if (!result.empty()) result += '/';
        result += segments[index];
    }
    return result;
}

bool PatternExists(const std::filesystem::path& path) {
    WIN32_FIND_DATAA findData{};
    const std::string pattern = path.string();
    HANDLE find = FindFirstFileA(pattern.c_str(), &findData);
    if (find == INVALID_HANDLE_VALUE) return false;
    FindClose(find);
    return true;
}

bool DirectoryExists(const std::filesystem::path& path) {
    std::error_code error;
    return std::filesystem::is_directory(path, error) && !error;
}

void AddCandidate(std::vector<std::filesystem::path>& candidates,
    std::set<std::string>& keys, const std::filesystem::path& candidate) {
    if (candidate.empty()) return;
    const std::string normalized = candidate.lexically_normal().string();
    std::string key = normalized;
    std::transform(key.begin(), key.end(), key.begin(),
        [](unsigned char ch) { return (char)std::tolower(ch); });
    if (keys.insert(key).second) candidates.push_back(normalized);
}

void AddAncestorCandidates(const char* filePath,
    const std::vector<std::string>& segments, size_t rootSegmentCount,
    std::vector<std::filesystem::path>& candidates,
    std::set<std::string>& keys) {
    if (!filePath || !*filePath) return;
    std::filesystem::path cursor = std::filesystem::path(filePath).parent_path();
    const bool hasPackage = rootSegmentCount >= 3;
    const std::string scope = segments[1];
    const std::string package = hasPackage ? segments[2] : scope;

    for (int depth = 0; !cursor.empty() && depth < 16; ++depth) {
        const std::string filename = cursor.filename().string();
        if (EqualsIgnoreCase(filename, package)) AddCandidate(candidates, keys, cursor);
        if (hasPackage) AddCandidate(candidates, keys, cursor / package);
        else AddCandidate(candidates, keys, cursor / scope);
        AddCandidate(candidates, keys, cursor / "LR2files" / scope /
            (hasPackage ? std::filesystem::path(package) : std::filesystem::path()));
        const std::filesystem::path parent = cursor.parent_path();
        if (parent == cursor) break;
        cursor = parent;
    }
}

} // namespace

bool SENormalizeLr2RootedPath(const char* requestedPath,
    std::string& logicalPath) {
    logicalPath.clear();
    if (!requestedPath || !*requestedPath) return false;
    std::string value = requestedPath;
    std::replace(value.begin(), value.end(), '\\', '/');
    while (value.size() >= 2 && value[0] == '.' && value[1] == '/')
        value.erase(0, 2);
    if (!StartsWithIgnoreCase(value, "LR2files/")) return false;

    const std::vector<std::string> segments = SplitPath(value);
    if (segments.size() < 2 || !EqualsIgnoreCase(segments[0], "LR2files"))
        return false;
    for (const std::string& segment : segments) {
        if (segment.empty() || segment == "." || segment == ".." ||
            segment.find(':') != std::string::npos)
            return false;
    }
    logicalPath = JoinSegments(segments, 0, segments.size());
    logicalPath.replace(0, strlen("LR2files"), "LR2files");
    return true;
}

bool SEResolveLr2VirtualRoot(const char* requestedPath,
    const char* ownerFilePath, const char* mainSkinPath,
    SELr2VirtualRootResolution& resolution) {
    resolution = SELr2VirtualRootResolution();
    if (!SENormalizeLr2RootedPath(requestedPath, resolution.logicalPath))
        return false;

    const std::vector<std::string> segments = SplitPath(resolution.logicalPath);
    size_t rootSegmentCount = 2;
    if (segments.size() >= 3 &&
        (EqualsIgnoreCase(segments[1], "Theme") ||
            EqualsIgnoreCase(segments[1], "Sound")))
        rootSegmentCount = 3;
    if (segments.size() < rootSegmentCount) return false;
    resolution.logicalRoot = JoinSegments(segments, 0, rootSegmentCount);

    std::vector<std::filesystem::path> candidates;
    std::set<std::string> keys;
    AddCandidate(candidates, keys, std::filesystem::path(resolution.logicalRoot));
    AddAncestorCandidates(ownerFilePath, segments, rootSegmentCount, candidates, keys);
    AddAncestorCandidates(mainSkinPath, segments, rootSegmentCount, candidates, keys);

    for (const std::filesystem::path& candidate : candidates) {
        if (!DirectoryExists(candidate)) continue;
        std::error_code error;
        const std::filesystem::path absolute = std::filesystem::absolute(candidate, error);
        resolution.physicalRoot = (error ? candidate : absolute).lexically_normal().string();
        return true;
    }
    resolution = SELr2VirtualRootResolution();
    return false;
}

bool SEResolveSkinResourcePath(const char* requestedPath,
    const char* ownerFilePath, const char* mainSkinPath,
    std::string& resolvedPath) {
    resolvedPath.clear();
    if (!requestedPath || !*requestedPath) return false;

    SELr2VirtualRootResolution root;
    if (SEResolveLr2VirtualRoot(requestedPath, ownerFilePath, mainSkinPath, root)) {
        const std::string suffix = root.logicalPath.size() > root.logicalRoot.size()
            ? root.logicalPath.substr(root.logicalRoot.size() + 1) : std::string();
        std::filesystem::path candidate(root.physicalRoot);
        if (!suffix.empty()) candidate /= std::filesystem::path(suffix);
        resolvedPath = candidate.lexically_normal().string();
        return true;
    }

    const std::filesystem::path requested(requestedPath);
    std::vector<std::filesystem::path> candidates;
    std::set<std::string> keys;
    if (requested.is_absolute()) AddCandidate(candidates, keys, requested);
    else {
        if (ownerFilePath && *ownerFilePath)
            AddCandidate(candidates, keys,
                std::filesystem::path(ownerFilePath).parent_path() / requested);
        if (mainSkinPath && *mainSkinPath)
            AddCandidate(candidates, keys,
                std::filesystem::path(mainSkinPath).parent_path() / requested);
        AddCandidate(candidates, keys, requested);
    }
    for (const std::filesystem::path& candidate : candidates) {
        if (!PatternExists(candidate)) continue;
        std::error_code error;
        const std::filesystem::path absolute = std::filesystem::absolute(candidate, error);
        resolvedPath = (error ? candidate : absolute).lexically_normal().string();
        return true;
    }
    return false;
}
