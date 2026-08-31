#pragma once

#include <string>

struct SELr2VirtualRootResolution {
    std::string logicalPath;
    std::string logicalRoot;
    std::string physicalRoot;
};

enum class SESkinResourcePathResult {
    Unresolved,
    Resolved,
    Rejected,
};

// Converts optional .\LR2files paths to a slash-normalized logical path.
// The logical value never contains a drive, dot segment, or parent traversal.
bool SENormalizeLr2RootedPath(const char* requestedPath,
    std::string& logicalPath);

// Resolves an LR2-rooted declaration against the standalone/real skin tree
// containing ownerFilePath/mainSkinPath, then the process LR2 tree fallback.
bool SEResolveLr2VirtualRoot(const char* requestedPath,
    const char* ownerFilePath, const char* mainSkinPath,
    SELr2VirtualRootResolution& resolution);

// Resolves preview/import resources without changing the CSV text. Wildcard
// patterns are returned as patterns so the existing LR2 selection flow remains
// authoritative. Rejected means a reserved imported-workspace path was unsafe;
// callers must not retry it through a legacy relative-path fallback.
SESkinResourcePathResult SEResolveSkinResourcePath(const char* requestedPath,
    const char* ownerFilePath, const char* mainSkinPath,
    std::string& resolvedPath);
