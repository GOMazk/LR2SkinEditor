#pragma once

#include <Windows.h>

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

struct SESkinFolderScanResult {
    bool success = false;
    std::vector<std::string> files;
    std::string message;
    int skippedLongPaths = 0;
};

inline bool SEIsSkinScriptFilename(const char* filename) {
    if (!filename) return false;
    const char* extension = strrchr(filename, '.');
    return extension && (_stricmp(extension, ".lr2skin") == 0 ||
        _stricmp(extension, ".lr2ss") == 0);
}

// The legacy parser and CSTR path fields are MAX_PATH/ANSI based. Keep folder
// discovery on the same boundary and report paths that cannot be represented
// instead of handing the loader a truncated filename.
inline SESkinFolderScanResult SEScanSkinFolder(const char* folder) {
    SESkinFolderScanResult result;
    if (!folder || !*folder) {
        result.message = "No folder was selected.";
        return result;
    }

    char canonicalRoot[MAX_PATH] = {};
    const DWORD rootLength = GetFullPathNameA(folder, MAX_PATH, canonicalRoot, NULL);
    if (rootLength == 0 || rootLength >= MAX_PATH) {
        result.message = "The selected folder path is too long for the LR2 loader.";
        return result;
    }
    const DWORD rootAttributes = GetFileAttributesA(canonicalRoot);
    if (rootAttributes == INVALID_FILE_ATTRIBUTES ||
        !(rootAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        result.message = "The selected folder does not exist or cannot be opened.";
        return result;
    }

    std::vector<std::string> pendingDirectories = { canonicalRoot };
    while (!pendingDirectories.empty()) {
        std::string directory = pendingDirectories.back();
        pendingDirectories.pop_back();
        if (!directory.empty() && directory.back() != '\\') directory += '\\';

        const std::string search = directory + "*";
        WIN32_FIND_DATAA entry = {};
        HANDLE find = FindFirstFileA(search.c_str(), &entry);
        if (find == INVALID_HANDLE_VALUE) continue;
        do {
            if (strcmp(entry.cFileName, ".") == 0 ||
                strcmp(entry.cFileName, "..") == 0) continue;
            const std::string path = directory + entry.cFileName;
            if (path.size() >= MAX_PATH) {
                ++result.skippedLongPaths;
                continue;
            }
            if (entry.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (!(entry.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
                    pendingDirectories.push_back(path);
            } else if (SEIsSkinScriptFilename(entry.cFileName)) {
                result.files.push_back(path);
            }
        } while (FindNextFileA(find, &entry));
        FindClose(find);
    }

    std::sort(result.files.begin(), result.files.end(),
        [](const std::string& left, const std::string& right) {
            return _stricmp(left.c_str(), right.c_str()) < 0;
        });
    result.success = true;
    if (result.skippedLongPaths > 0) {
        result.message = "Some skin paths were skipped because they exceed MAX_PATH.";
    }
    return result;
}
