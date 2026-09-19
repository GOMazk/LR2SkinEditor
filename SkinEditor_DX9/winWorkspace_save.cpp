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
#include "inputwrap.h"
#include "scriptFileSave.h"
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>

static bool SamePath(const char* a, const char* b) {
    return a && b && _stricmp(a, b) == 0;
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

namespace {
struct PendingSkinSave {
    std::string outputPath, tempPath, backupPath;
    bool hadOriginal = false;
    bool backupOwned = false;
    bool restoreFailed = false;
    unsigned long long expectedSize = 0;
    unsigned int expectedHash = 0;
};

// Injectable only inside this translation unit's self-test; normal saves always
// use real Win32 operations. No global fault switch or user-data test paths.
struct SkinSaveFileOps {
    std::function<BOOL(const char*, const char*, BOOL)> copy = CopyFileA;
    std::function<BOOL(const char*, const char*, DWORD)> move = MoveFileExA;
    std::function<BOOL(const char*)> erase = DeleteFileA;
    std::function<bool(const char*, unsigned long long&, unsigned int&)> fingerprint = FileFingerprint;
};

void AppendSaveReport(std::string& report, const std::string& text) {
    if (!report.empty()) report += '\n';
    report += text;
}

void ReportWindowsSaveError(std::string& report, const char* action,
    const std::string& path, DWORD code) {
    // Windows code and UTF-8 path remain available even if message lookup fails.
    std::string detail = std::string(action) + ": " + Cp932ToUtf8(path.c_str()) +
        " (Windows error " + std::to_string(code) + ")";
    wchar_t message[512] = {};
    if (FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, code, 0, message, 512, nullptr)) {
        char utf8[2048] = {};
        if (WideCharToMultiByte(CP_UTF8, 0, message, -1, utf8, sizeof(utf8), nullptr, nullptr)) {
            std::string text(utf8);
            while (!text.empty() && (text.back() == '\r' || text.back() == '\n')) text.pop_back();
            detail += " - " + text;
        }
    }
    AppendSaveReport(report, detail);
}

bool RecoveryBackupAvailable(const std::string& backup, std::string& report) {
    if (GetFileAttributesA(backup.c_str()) != INVALID_FILE_ATTRIBUTES) {
        AppendSaveReport(report, "Save stopped: recovery backup already exists: " + Cp932ToUtf8(backup.c_str()) +
            ". Restore or move it to a safe location before saving again. It was not overwritten.");
        return false;
    }
    const DWORD error = GetLastError();
    if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) return true;
    ReportWindowsSaveError(report, "Cannot check recovery backup", backup, error);
    return false;
}

void CleanupSkinSave(std::vector<PendingSkinSave>& pending, std::string& report, const SkinSaveFileOps& ops) {
    for (auto& save : pending) {
        if (!ops.erase(save.tempPath.c_str())) {
            const DWORD error = GetLastError();
            if (error != ERROR_FILE_NOT_FOUND && error != ERROR_PATH_NOT_FOUND)
                ReportWindowsSaveError(report, "Temporary file retained", save.tempPath, error);
        }
        // Never remove an unowned backup or one needed for failed rollback.
        if (save.backupOwned && !save.restoreFailed) {
            if (!ops.erase(save.backupPath.c_str())) {
                const DWORD error = GetLastError();
                if (error != ERROR_FILE_NOT_FOUND && error != ERROR_PATH_NOT_FOUND)
                    ReportWindowsSaveError(report, "Backup retained; move it to safety before the next save", save.backupPath, error);
            }
            save.backupOwned = false;
        }
    }
}

int CommitSkinSave(std::vector<PendingSkinSave>& pending, std::string& report,
    const SkinSaveFileOps& ops = SkinSaveFileOps()) {
    // Check every backup before creating any. A later retry must not erase a
    // recovery copy left by this process, another Workspace or an interrupted run.
    for (auto& save : pending) {
        if (!RecoveryBackupAvailable(save.backupPath, report)) {
            CleanupSkinSave(pending, report, ops); return -1;
        }
        const DWORD attributes = GetFileAttributesA(save.outputPath.c_str());
        if (attributes == INVALID_FILE_ATTRIBUTES) {
            const DWORD error = GetLastError();
            if (error != ERROR_FILE_NOT_FOUND && error != ERROR_PATH_NOT_FOUND) {
                ReportWindowsSaveError(report, "Cannot inspect destination", save.outputPath, error);
                CleanupSkinSave(pending, report, ops); return -1;
            }
        }
        save.hadOriginal = attributes != INVALID_FILE_ATTRIBUTES;
    }
    for (auto& save : pending) {
        if (!save.hadOriginal) continue;
        // Fail-if-exists closes the check/copy race without clobbering backups.
        if (!ops.copy(save.outputPath.c_str(), save.backupPath.c_str(), TRUE)) {
            const DWORD error = GetLastError();
            ReportWindowsSaveError(report, "Could not create backup", save.backupPath, error);
            AppendSaveReport(report, "No destination files were replaced. Any backup at the failed path was left untouched.");
            CleanupSkinSave(pending, report, ops); return -1;
        }
        save.backupOwned = true;
    }
    std::size_t replaced = 0;
    for (; replaced < pending.size(); ++replaced) {
        auto& save = pending[replaced];
        if (!ops.move(save.tempPath.c_str(), save.outputPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            ReportWindowsSaveError(report, "Could not replace destination", save.outputPath, GetLastError());
            break;
        }
    }
    bool verified = replaced == pending.size();
    if (verified) for (const auto& save : pending) {
        unsigned long long size = 0; unsigned int hash = 0;
        if (!ops.fingerprint(save.outputPath.c_str(), size, hash) ||
            size != save.expectedSize || hash != save.expectedHash) {
            AppendSaveReport(report, "Saved file verification failed (read/size/hash): " + Cp932ToUtf8(save.outputPath.c_str()));
            verified = false; break;
        }
    }
    if (!verified) {
        bool rollbackOk = true;
        for (std::size_t i = 0; i < replaced; ++i) {
            auto& save = pending[i];
            if (save.hadOriginal) {
                if (ops.move(save.backupPath.c_str(), save.outputPath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
                    save.backupOwned = false; // backup was consumed by successful restoration
                else {
                    const DWORD error = GetLastError();
                    rollbackOk = false; save.restoreFailed = true;
                    ReportWindowsSaveError(report, "RESTORE FAILED", save.outputPath, error);
                    AppendSaveReport(report, "Recovery backup kept: " + Cp932ToUtf8(save.backupPath.c_str()));
                }
            } else if (!ops.erase(save.outputPath.c_str())) {
                const DWORD error = GetLastError();
                if (error != ERROR_FILE_NOT_FOUND && error != ERROR_PATH_NOT_FOUND) {
                    rollbackOk = false;
                    ReportWindowsSaveError(report, "Could not remove newly created destination", save.outputPath, error);
                }
            }
        }
        CleanupSkinSave(pending, report, ops);
        AppendSaveReport(report, rollbackOk ? "Save failed; destination changes were rolled back."
            : "Recovery is incomplete. Files may contain mixed versions. Keep the listed backups and restore them before retrying.");
        return -1;
    }
    CleanupSkinSave(pending, report, ops);
    return 0;
}
}

bool SESaveExternalTextFile(const std::string& path, const std::string& original,
    const std::string& edited, std::string& report) {
    report.clear();
    if (path.empty()) { report = "No external file is open."; return false; }
    std::ifstream source(path, std::ios::binary);
    // Read at most the expected length plus one byte, even if an external
    // editor replaced the source with a very large file since it was opened.
    std::string current(original.size() + 1, '\0');
    source.read(current.data(), (std::streamsize)current.size());
    const auto count = source.gcount();
    current.resize((size_t)count);
    if (!source.is_open() || source.bad() || current != original) {
        report = "File changed outside the editor or could not be read. Reload before saving: " + Cp932ToUtf8(path.c_str());
        return false;
    }
    source.close();
    PendingSkinSave save;
    save.outputPath = path; save.tempPath = path + ".skineditor.tmp"; save.backupPath = path + ".skineditor.bak";
    if (!RecoveryBackupAvailable(save.backupPath, report)) return false;
    FILE* output = fopen(save.tempPath.c_str(), "wb");
    if (!output) { report = "Cannot create temporary file: " + Cp932ToUtf8(save.tempPath.c_str()); return false; }
    bool ok = fwrite(edited.data(), 1, edited.size(), output) == edited.size();
    if (fclose(output) != 0) ok = false;
    if (!ok || !FileFingerprint(save.tempPath.c_str(), save.expectedSize, save.expectedHash)) {
        report = "Cannot write/verify temporary file: " + Cp932ToUtf8(save.tempPath.c_str());
        std::vector<PendingSkinSave> pending{save}; CleanupSkinSave(pending, report, SkinSaveFileOps()); return false;
    }
    std::vector<PendingSkinSave> pending{save};
    return CommitSkinSave(pending, report) == 0;
}

int WORKSPACE::SaveSkinScript(char* path, bool split, bool nocomment) {
    scriptSaveReport.clear();
    if (!path || !*path) { scriptSaveReport = "No output path was specified."; return -1; }
    std::vector<PendingSkinSave> pending;

    auto persistedAssetIsUsed = [&](SKINFILELINEREAD& metadata) {
        const char* text = metadata.line.body ? metadata.line.outstr() : "";
        if (strncmp(text, "$SRC_IMAGE,", 11) != 0) return false;
        CSVbuf asset;
        SplitCSV(metadata.line, &asset, ",");
        for (int row = 0; row < skinfileLines.count; ++row) {
            SKINFILELINEREAD& candidate =
                ((SKINFILELINEREAD*)skinfileLines.data)[row];
            if (candidate.isComment || !candidate.csv.str[0].body ||
                strncmp(candidate.csv.str[0].outstr(), "#SRC", 4) != 0 ||
                candidate.csv.str[0].isSame("#SRC_TEXT")) continue;
            if (candidate.ifgroup == metadata.ifgroup &&
                candidate.csv.val[2] == asset.val[2] &&
                candidate.csv.val[3] == asset.val[3] &&
                candidate.csv.val[4] == asset.val[4] &&
                candidate.csv.val[5] == asset.val[5] &&
                candidate.csv.val[6] == asset.val[6]) return true;
        }
        return false;
    };

    auto prepareOneFile = [&](const char* outputPath, const char* sourcePath, bool merged) -> int {
        const std::string backupPath = std::string(outputPath) + ".skineditor.bak";
        if (!RecoveryBackupAvailable(backupPath, scriptSaveReport)) return -1;
        std::string tempPath(outputPath);
        tempPath += ".skineditor.tmp";
        FILE* pFile = fopen(tempPath.c_str(), "wb");
        if (pFile == NULL) {
            scriptSaveReport = "Could not create temporary file: " + Cp932ToUtf8(tempPath.c_str()) +
                " (errno " + std::to_string(errno) + ")";
            return -1;
        }
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
            const bool isAssetMetadata = line.isSEcomment &&
                line.line.left(11).isSame("$SRC_IMAGE,");
            if (isAssetMetadata && persistedAssetIsUsed(line)) continue;
            const bool isEditorMetadata = line.isSEcomment &&
                (line.line.left(4).isSame("$SE_") || isAssetMetadata);
            if (nocomment && line.isComment && !isEditorMetadata) continue;
            if (fputs(line.line, pFile) < 0 || fputs("\n", pFile) < 0) { ok = false; break; }
        }
        if (fclose(pFile) != 0) ok = false;
        if (!ok) {
            scriptSaveReport = "Could not write temporary file: " + Cp932ToUtf8(tempPath.c_str());
            remove(tempPath.c_str()); return -1;
        }
        PendingSkinSave save;
        save.outputPath = outputPath;
        save.tempPath = tempPath;
        save.backupPath = backupPath;
        if (!FileFingerprint(tempPath.c_str(), save.expectedSize, save.expectedHash)) {
            scriptSaveReport = "Could not verify temporary file: " + Cp932ToUtf8(tempPath.c_str());
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
                CleanupSkinSave(pending, scriptSaveReport, SkinSaveFileOps());
                return -1;
            }
        }
        if (prepareOneFile(path, mainpath, false) != 0) {
            CleanupSkinSave(pending, scriptSaveReport, SkinSaveFileOps());
            return -1;
        }
    }

    return CommitSkinSave(pending, scriptSaveReport);
}

//save for object mode
int WORKSPACE::SaveSkinScript2(char* path, bool split, bool nocomment) {
    // Object Editor writes directly to skinfileLines. Keep one authoritative
    // save path so exported files contain the latest edits and all directives.
    return SaveSkinScript(path, split, nocomment);
}

int RunScriptSaveRecoverySelfTest() {
    namespace fs = std::filesystem;
    char temporary[MAX_PATH] = {}, unique[MAX_PATH] = {};
    if (!GetTempPathA(MAX_PATH, temporary) || !GetTempFileNameA(temporary, "ses", 0, unique)) return 1;
    if (!DeleteFileA(unique) || !CreateDirectoryA(unique, nullptr)) return 2;
    struct Cleanup {
        fs::path path;
        ~Cleanup() { std::error_code ec; fs::remove_all(path, ec); }
    } cleanup{fs::path(unique)};
    const auto write = [](const std::string& path, const std::string& bytes) {
        std::ofstream output(path, std::ios::binary | std::ios::trunc);
        output.write(bytes.data(), bytes.size()); output.close(); return !!output;
    };
    const auto read = [](const std::string& path) {
        std::ifstream input(path, std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    };
    // Each scenario owns a separate temporary directory. Original bytes include
    // CP932 text and CRLF; rollback must preserve bytes, not reserialize CSV.
    const std::string original[] = {"old-A,\x83\x65\x83\x58\x83\x67\r\n", "old-B\r\n"};
    const std::string edited[] = {"new-A\n", "new-B\n"};
    for (int scenario = 0; scenario <= 10; ++scenario) {
        const fs::path directory = cleanup.path / std::to_string(scenario);
        fs::create_directory(directory);
        std::vector<PendingSkinSave> pending(2);
        const bool firstIsNew = scenario == 8 || scenario == 9;
        for (int i = 0; i < 2; ++i) {
            auto& save = pending[i];
            save.outputPath = (directory / (i ? "main.lr2skin" : "included.csv")).string();
            save.tempPath = save.outputPath + ".skineditor.tmp";
            save.backupPath = save.outputPath + ".skineditor.bak";
            if ((i != 0 || !firstIsNew) && !write(save.outputPath, original[i])) return 3;
            if (!write(save.tempPath, edited[i]) || !FileFingerprint(save.tempPath.c_str(), save.expectedSize, save.expectedHash)) return 4;
        }
        SkinSaveFileOps ops;
        struct Locks {
            HANDLE first = INVALID_HANDLE_VALUE, second = INVALID_HANDLE_VALUE;
            ~Locks() { if (first != INVALID_HANDLE_VALUE) CloseHandle(first); if (second != INVALID_HANDLE_VALUE) CloseHandle(second); }
        } locks;
        if (scenario == 3) {
            // Real Windows sharing violations: B cannot be replaced; A is
            // locked after replacement so restoring its backup also fails.
            locks.second = CreateFileA(pending[1].outputPath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                nullptr, OPEN_EXISTING, 0, nullptr);
            if (locks.second == INVALID_HANDLE_VALUE) return 5;
        }
        ops.move = [&](const char* from, const char* to, DWORD flags) -> BOOL {
            const bool first = pending[0].outputPath == to;
            const bool replacing = pending[first ? 0 : 1].tempPath == from;
            if ((scenario == 1 && replacing && first) ||
                ((scenario == 2 || scenario == 8 || scenario == 9) && replacing && !first) ||
                (scenario == 5 && !replacing && first)) {
                SetLastError(ERROR_ACCESS_DENIED); return FALSE;
            }
            const BOOL result = MoveFileExA(from, to, flags);
            if (scenario == 3 && first && replacing && result)
                locks.first = CreateFileA(to, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
            return result;
        };
        ops.copy = [&](const char* from, const char* to, BOOL failIfExists) -> BOOL {
            if (!failIfExists) { SetLastError(ERROR_INVALID_PARAMETER); return FALSE; }
            if (scenario == 6 && pending[1].backupPath == to) {
                write(to, "partial backup"); SetLastError(ERROR_DISK_FULL); return FALSE;
            }
            if (scenario == 7 && pending[0].backupPath == to) write(to, "foreign recovery copy");
            return CopyFileA(from, to, failIfExists);
        };
        ops.fingerprint = [&](const char* path, unsigned long long& size, unsigned int& hash) {
            if (scenario == 5) return false;
            const bool ok = FileFingerprint(path, size, hash);
            if (scenario == 4 && pending[1].outputPath == path) ++hash;
            return ok;
        };
        ops.erase = [&](const char* path) -> BOOL {
            if ((scenario == 9 && pending[0].outputPath == path) ||
                (scenario == 10 && pending[0].backupPath == path)) {
                SetLastError(ERROR_ACCESS_DENIED); return FALSE;
            }
            return DeleteFileA(path);
        };
        std::string report;
        const int result = CommitSkinSave(pending, report, ops);
        if ((result == 0) != (scenario == 0 || scenario == 10)) return 10 + scenario;
        if (scenario == 3 && locks.first == INVALID_HANDLE_VALUE) return 21;
        for (int i = 0; i < 2; ++i) {
            const auto& save = pending[i];
            const bool changed = scenario == 0 || scenario == 10 ||
                (i == 0 && (scenario == 3 || scenario == 5 || scenario == 9));
            const bool missing = firstIsNew && i == 0 && !changed;
            if (missing ? fs::exists(save.outputPath) : read(save.outputPath) != (changed ? edited[i] : original[i])) return 30 + scenario;
            const bool kept = (i == 0 && (scenario == 3 || scenario == 5 || scenario == 7 || scenario == 10)) ||
                (i == 1 && scenario == 6);
            if (fs::exists(save.backupPath) != kept || fs::exists(save.tempPath)) return 50 + scenario;
            if (kept && read(save.backupPath) != (scenario == 7 ? "foreign recovery copy" :
                scenario == 6 ? "partial backup" : original[i])) return 70 + scenario;
        }
        if (scenario == 0 && !report.empty()) return 90;
        if (scenario != 0 && report.find(".lr2skin") == std::string::npos && report.find(".csv") == std::string::npos) return 91;
        if ((scenario == 3 || scenario == 5) &&
            (report.find("RESTORE FAILED") == std::string::npos || report.find("Recovery backup kept:") == std::string::npos ||
                report.find("Windows error") == std::string::npos || report.find("Recovery is incomplete") == std::string::npos)) return 92;
        if (scenario == 9 && report.find("Could not remove newly created destination") == std::string::npos) return 93;
        if (scenario == 3) {
            CloseHandle(locks.first); locks.first = INVALID_HANDLE_VALUE;
            CloseHandle(locks.second); locks.second = INVALID_HANDLE_VALUE;
            // A fresh Workspace must detect the backup on disk before any I/O.
            auto workspace = std::make_unique<WORKSPACE>();
            strncpy_s(workspace->mainpath, pending[0].outputPath.c_str(), _TRUNCATE);
            workspace->loaded = true;
            workspace->documentRevision = 1; workspace->savedDocumentRevision = 0;
            const auto history = workspace->arr_history.count;
            if (workspace->SaveCurrentSkin() == 0 || workspace->lastSaveState != -1 ||
                !workspace->IsDocumentDirty() || workspace->savedDocumentRevision != 0 ||
                workspace->arr_history.count != history ||
                workspace->lastSaveMessage.find("recovery backup already exists") == std::string::npos ||
                workspace->lastSaveMessage.find(Cp932ToUtf8(pending[0].backupPath.c_str())) == std::string::npos ||
                read(pending[0].backupPath) != original[0] || read(pending[0].outputPath) != edited[0] ||
                fs::exists(pending[0].tempPath)) return 94;
            // Retry the whole transaction: no copy/move may run and the backup
            // is unowned by this invocation, so cleanup must leave it alone.
            for (auto& save : pending) {
                save.backupOwned = save.restoreFailed = false;
                if (!write(save.tempPath, "retry")) return 95;
            }
            bool touched = false;
            ops.copy = [&](const char*, const char*, BOOL) -> BOOL { touched = true; return FALSE; };
            ops.move = [&](const char*, const char*, DWORD) -> BOOL { touched = true; return FALSE; };
            report.clear();
            if (CommitSkinSave(pending, report, ops) == 0 || touched ||
                read(pending[0].backupPath) != original[0] || read(pending[0].outputPath) != edited[0]) return 96;
        }
    }
    printf("Script save recovery: 11 success/failure cases passed, including real locked-file rollback and retry protection.\n");
    return 0;
}
