#include "imgui/imgui.h"

#include "../LR2/structure.h"
#include "../LR2/LR2_skinmanage.h"
#include "../LR2/LR2_skinload.h"
#include "../LR2/LR2_skindraw.h"
#include "../LR2/LR2_skinobject.h"
#include "../LR2/LR2_configsave.h"
#include "../LR2/En_fileutil.h"
#include "../LR2/Scene07_Skinselect.h"
#include "../LR2/En_timer.h"
#include "../LR2/En_value.h" //for ByTime

#include "../lib/DxLib/DxLib.h"
#include "winWorkspace.h"

#include "skin.h"
#include "op.h"
#include "arr.hpp"
#include "seHelper.h"

static bool SamePath(const char* a, const char* b) {
    return a && b && _stricmp(a, b) == 0;
}

static int ReplaceFileAtomically(const char* tempPath, const char* finalPath) {
    if (MoveFileExA(tempPath, finalPath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) return 0;
    remove(tempPath);
    return -1;
}

static bool FileFingerprint(const char* path, unsigned long long& size, unsigned int& hash) {
    FILE* fp = fopen(path, "rb");
    if (!fp) return false;
    size = 0;
    hash = 2166136261u;
    unsigned char buffer[8192];
    size_t count = 0;
    while ((count = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        size += count;
        for (size_t i = 0; i < count; ++i) {
            hash ^= buffer[i];
            hash *= 16777619u;
        }
    }
    const bool ok = !ferror(fp);
    fclose(fp);
    return ok;
}

//TODO apply split
int WORKSPACE::SaveSkinScript(char* path, bool split, bool nocomment) {
    if (!path || !*path) return -1;

    struct PendingSave {
        std::string outputPath;
        std::string tempPath;
        std::string backupPath;
        bool hadOriginal = false;
        unsigned long long expectedSize = 0;
        unsigned int expectedHash = 0;
    };
    std::vector<PendingSave> pending;

    auto prepareOneFile = [&](const char* outputPath, const char* sourcePath, bool merged) -> int {
        std::string tempPath(outputPath);
        tempPath += ".skineditor.tmp";
        FILE* pFile = fopen(tempPath.c_str(), "wb");
        if (pFile == NULL) return -1;
        bool ok = true;
        for (int i = 0; i < skinfileLines.count; i++) {
            SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[i];
            if (line.isSEcomment && line.line.left(5).isSame("$FILE")) continue;
            if (!merged) {
                const char* owner = line.filename.body ? line.filename.outstr() : mainpath;
                if (!SamePath(owner, sourcePath)) continue;
            } else if (!line.isComment && line.csv.str[0].isSame("#INCLUDE")) {
                // Included files are already expanded in skinfileLines.
                continue;
            }
            const bool isEditorMetadata = line.isSEcomment && line.line.left(4).isSame("$SE_");
            if (nocomment && line.isComment && !isEditorMetadata) continue;
            if (fputs(line.line, pFile) < 0 || fputs("\n", pFile) < 0) { ok = false; break; }
        }
        if (fclose(pFile) != 0) ok = false;
        if (!ok) { remove(tempPath.c_str()); return -1; }
        PendingSave save;
        save.outputPath = outputPath;
        save.tempPath = tempPath;
        save.backupPath = std::string(outputPath) + ".skineditor.bak";
        if (!FileFingerprint(tempPath.c_str(), save.expectedSize, save.expectedHash)) {
            remove(tempPath.c_str());
            return -1;
        }
        pending.push_back(save);
        return 0;
    };

    if (!split) {
        if (prepareOneFile(path, NULL, true) != 0) return -1;
    } else {
        std::vector<std::string> sources;
        sources.push_back(mainpath);
        for (int i = 0; i < skinfileLines.count; ++i) {
            SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[i];
            if (!line.filename.body) continue;
            const char* filename = line.filename.outstr();
            bool exists = false;
            for (std::size_t s = 0; s < sources.size(); ++s) {
                if (SamePath(sources[s].c_str(), filename)) { exists = true; break; }
            }
            if (!exists) sources.push_back(filename);
        }

        for (std::size_t s = 1; s < sources.size(); ++s) {
            if (prepareOneFile(sources[s].c_str(), sources[s].c_str(), false) != 0) {
                for (const PendingSave& save : pending) remove(save.tempPath.c_str());
                return -1;
            }
        }
        if (prepareOneFile(path, mainpath, false) != 0) {
            for (const PendingSave& save : pending) remove(save.tempPath.c_str());
            return -1;
        }
    }

    for (PendingSave& save : pending) {
        save.hadOriginal = GetFileAttributesA(save.outputPath.c_str()) != INVALID_FILE_ATTRIBUTES;
        remove(save.backupPath.c_str());
        if (save.hadOriginal && !CopyFileA(save.outputPath.c_str(), save.backupPath.c_str(), FALSE)) {
            for (const PendingSave& cleanup : pending) {
                remove(cleanup.tempPath.c_str());
                remove(cleanup.backupPath.c_str());
            }
            return -1;
        }
    }

    std::size_t replaced = 0;
    for (; replaced < pending.size(); ++replaced) {
        if (ReplaceFileAtomically(pending[replaced].tempPath.c_str(), pending[replaced].outputPath.c_str()) != 0)
            break;
    }
    bool verified = replaced == pending.size();
    if (verified) {
        for (const PendingSave& save : pending) {
            unsigned long long actualSize = 0;
            unsigned int actualHash = 0;
            if (!FileFingerprint(save.outputPath.c_str(), actualSize, actualHash) ||
                actualSize != save.expectedSize || actualHash != save.expectedHash) {
                verified = false;
                break;
            }
        }
    }
    if (!verified) {
        const std::size_t rollbackCount = replaced < pending.size() ? replaced : pending.size();
        for (std::size_t i = 0; i < rollbackCount; ++i) {
            if (pending[i].hadOriginal)
                MoveFileExA(pending[i].backupPath.c_str(), pending[i].outputPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
            else
                remove(pending[i].outputPath.c_str());
        }
        for (PendingSave& save : pending) {
            remove(save.tempPath.c_str());
            remove(save.backupPath.c_str());
        }
        return -1;
    }
    for (PendingSave& save : pending) remove(save.backupPath.c_str());
    return 0;
}

//save for object mode
int WORKSPACE::SaveSkinScript2(char* path, bool split, bool nocomment) {
    // Object Editor writes directly to skinfileLines. Keep one authoritative
    // save path so exported files contain the latest edits and all directives.
    return SaveSkinScript(path, split, nocomment);
}
