#pragma once

#include <string>

struct SELr2VirtualRootResolution {
    std::string logicalPath;
    std::string logicalRoot;
    std::string physicalRoot;
};

// Converts optional .\LR2files paths to a slash-normalized logical path.
// The logical value never contains a drive, dot segment, or parent traversal.
bool SENormalizeLr2RootedPath(const char* requestedPath,
    std::string& logicalPath);

// Resolves an LR2-rooted declaration against either a real LR2 installation
// or the standalone skin tree containing ownerFilePath/mainSkinPath.
bool SEResolveLr2VirtualRoot(const char* requestedPath,
    const char* ownerFilePath, const char* mainSkinPath,
    SELr2VirtualRootResolution& resolution);

// Resolves preview/import resources without changing the CSV text. Wildcard
// patterns are returned as patterns so the existing LR2 selection flow remains
// authoritative.
bool SEResolveSkinResourcePath(const char* requestedPath,
    const char* ownerFilePath, const char* mainSkinPath,
    std::string& resolvedPath);
