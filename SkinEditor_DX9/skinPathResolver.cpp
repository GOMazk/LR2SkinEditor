#include "skinPathResolver.h"
#include "olrSkin.h"

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

bool IsAbsoluteWindowsBytePath(const char* path) {
    if (!path || !*path) return false;
    const bool hasDrive = std::isalpha((unsigned char)path[0]) &&
        path[1] == ':' && (path[2] == '\\' || path[2] == '/');
    const bool isUnc = (path[0] == '\\' && path[1] == '\\') ||
        (path[0] == '/' && path[1] == '/');
    return hasDrive || isUnc;
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

enum class ImportedWorkspacePathKind {
    Other,
    Valid,
    Invalid,
};

std::string NormalizeWin32SegmentAlias(const std::string& segment) {
    size_t length = segment.size();
    while (length > 0 &&
        (segment[length - 1] == '.' || segment[length - 1] == ' '))
        --length;
    return segment.substr(0, length);
}

ImportedWorkspacePathKind NormalizeImportedWorkspacePath(const char* requestedPath,
    std::string& normalizedPath) {
    normalizedPath.clear();
    if (!requestedPath || !*requestedPath)
        return ImportedWorkspacePathKind::Other;

    std::string value = requestedPath;
    std::replace(value.begin(), value.end(), '\\', '/');
    while (value.size() >= 2 && value[0] == '.' && value[1] == '/')
        value.erase(0, 2);

    const std::vector<std::string> segments = SplitPath(value);
    if (segments.empty() || !EqualsIgnoreCase(
        NormalizeWin32SegmentAlias(segments[0]), "vfs"))
        return ImportedWorkspacePathKind::Other;

    // Win32 ignores trailing dots and spaces in ordinary path segments. Treat
    // every alias of the imported workspace's vfs namespace as reserved, then
    // accept only its exact spelling. Otherwise vfs. or LR2files. could reach
    // the real directories after this validator had delegated to legacy path
    // fallback, including paths containing parent traversal.
    if (segments.size() < 3 ||
        !EqualsIgnoreCase(segments[0], "vfs") ||
        !EqualsIgnoreCase(segments[1], "LR2files"))
        return ImportedWorkspacePathKind::Invalid;
    for (const std::string& segment : segments) {
        if (segment.empty() || segment == "." || segment == ".." ||
            segment.find(':') != std::string::npos)
            return ImportedWorkspacePathKind::Invalid;
    }

    normalizedPath = JoinSegments(segments, 0, segments.size());
    normalizedPath.replace(0, strlen("vfs/LR2files"), "vfs/LR2files");
    return ImportedWorkspacePathKind::Valid;
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

bool ResolveImportedWorkspacePath(const std::string& normalizedPath,
    const char* ownerFilePath, const char* mainSkinPath,
    std::string& resolvedPath) {
    std::vector<std::filesystem::path> candidates;
    std::set<std::string> keys;
    const auto addWorkspaceCandidate = [&](const char* filePath) {
        if (!filePath || !*filePath) return;
        AddCandidate(candidates, keys,
            std::filesystem::path(filePath).parent_path() / "vfs" / "LR2files");
    };
    addWorkspaceCandidate(ownerFilePath);
    addWorkspaceCandidate(mainSkinPath);

    const std::string suffix = normalizedPath.substr(strlen("vfs/LR2files/"));
    for (const std::filesystem::path& candidate : candidates) {
        if (!DirectoryExists(candidate)) continue;
        std::error_code error;
        const std::filesystem::path absolute =
            std::filesystem::absolute(candidate, error);
        resolvedPath = (error ? candidate : absolute).lexically_normal().string();
        if (!resolvedPath.empty() && resolvedPath.back() != '\\' &&
            resolvedPath.back() != '/')
            resolvedPath += '\\';
        // Keep the legacy CP932 suffix as bytes. Constructing a filesystem path
        // from it can reinterpret valid LR2 text through the active code page.
        resolvedPath += suffix;
        std::replace(resolvedPath.begin(), resolvedPath.end(), '/', '\\');
        return true;
    }
    return false;
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
    // The opened document owns its virtual LR2 namespace. Resolve from its
    // declaring include/main skin before consulting the process working
    // directory; otherwise an unrelated editor-side LR2files tree can shadow
    // a portable standalone skin with the same Theme name.
    AddAncestorCandidates(ownerFilePath, segments, rootSegmentCount, candidates, keys);
    AddAncestorCandidates(mainSkinPath, segments, rootSegmentCount, candidates, keys);
    AddCandidate(candidates, keys, std::filesystem::path(resolution.logicalRoot));

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

SESkinResourcePathResult SEResolveSkinResourcePath(const char* requestedPath,
    const char* ownerFilePath, const char* mainSkinPath,
    std::string& resolvedPath) {
    resolvedPath.clear();
    if (!requestedPath || !*requestedPath)
        return SESkinResourcePathResult::Unresolved;

    // Callers may pass a path already resolved by an earlier parser phase.
    // Reconstructing std::filesystem::path from that CP932 byte stream can
    // throw on non-Japanese Windows. Absolute paths need no further root
    // discovery, so keep the original bytes intact.
    if (IsAbsoluteWindowsBytePath(requestedPath)) {
        resolvedPath = requestedPath;
        std::replace(resolvedPath.begin(), resolvedPath.end(), '/', '\\');
        return SESkinResourcePathResult::Resolved;
    }

    // Imported OLR workspaces deliberately keep portable resources below
    // vfs/LR2files until the explicit install-ready export. Resolve that
    // namespace from the workspace root even when the requested leaf exists
    // only inside a sibling .dxa archive and Win32 cannot enumerate it.
    if (SEIsOLRVirtualWorkspace(mainSkinPath) ||
        SEIsOLRVirtualWorkspace(ownerFilePath)) {
        std::string importedWorkspacePath;
        const ImportedWorkspacePathKind importedWorkspaceKind =
            NormalizeImportedWorkspacePath(requestedPath, importedWorkspacePath);
        if (importedWorkspaceKind == ImportedWorkspacePathKind::Invalid)
            return SESkinResourcePathResult::Rejected;
        if (importedWorkspaceKind == ImportedWorkspacePathKind::Valid)
            return ResolveImportedWorkspacePath(importedWorkspacePath,
                ownerFilePath, mainSkinPath, resolvedPath)
                    ? SESkinResourcePathResult::Resolved
                    : SESkinResourcePathResult::Unresolved;
    }

    SELr2VirtualRootResolution root;
    if (SEResolveLr2VirtualRoot(requestedPath, ownerFilePath, mainSkinPath, root)) {
        const std::string suffix = root.logicalPath.size() > root.logicalRoot.size()
            ? root.logicalPath.substr(root.logicalRoot.size() + 1) : std::string();
        // LR2 CSV files are commonly CP932 byte streams even on a machine
        // whose active ANSI code page is not Japanese. std::filesystem::path
        // converts narrow text through that active code page, and malformed
        // legacy bytes can throw while loading an otherwise valid skin. The
        // logical suffix is already traversal-checked, so joining its bytes is
        // both sufficient and faithful to LR2's original char-path behavior.
        resolvedPath = root.physicalRoot;
        if (!suffix.empty()) {
            if (!resolvedPath.empty() && resolvedPath.back() != '\\' &&
                resolvedPath.back() != '/')
                resolvedPath += '\\';
            resolvedPath += suffix;
        }
        std::replace(resolvedPath.begin(), resolvedPath.end(), '/', '\\');
        return SESkinResourcePathResult::Resolved;
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
        return SESkinResourcePathResult::Resolved;
    }
    return SESkinResourcePathResult::Unresolved;
}
