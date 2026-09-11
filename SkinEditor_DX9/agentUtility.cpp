#include "agentUtility.h"
#include "winWorkspace.h"
#include "seHelper.h"
#include "inputwrap.h"
#include "../LR2/LR2_skinmanage.h"
#include "../lib/DxLib/DxLib.h"
#include <shellapi.h>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {
const std::vector<std::wstring>& Arguments() {
    static const auto args = [] {
        int count = 0;
        LPWSTR* raw = CommandLineToArgvW(GetCommandLineW(), &count);
        std::vector<std::wstring> result;
        if (raw) { for (int i = 0; i < count; ++i) result.emplace_back(raw[i]); LocalFree(raw); }
        return result;
    }();
    return args;
}

std::string Encode(const std::wstring& text, UINT codePage) {
    BOOL substituted = FALSE;
    const DWORD flags = codePage == CP_UTF8 ? WC_ERR_INVALID_CHARS : WC_NO_BEST_FIT_CHARS;
    BOOL* used = codePage == CP_UTF8 ? nullptr : &substituted;
    const int count = WideCharToMultiByte(codePage, flags, text.c_str(), (int)text.size(), nullptr, 0, nullptr, used);
    if ((!count && !text.empty()) || substituted) throw std::runtime_error("Text/path cannot be represented in the required encoding.");
    std::string result(count, '\0');
    if (count && !WideCharToMultiByte(codePage, flags, text.c_str(), (int)text.size(), result.data(), count, nullptr, used))
        throw std::runtime_error("Text conversion failed.");
    return result;
}

std::string Quote(const std::string& text) {
    static const char* hex = "0123456789abcdef";
    std::string result = "\"";
    for (unsigned char c : text) {
        if (c == '"' || c == '\\') { result += '\\'; result += c; }
        else if (c < 32) { result += "\\u00"; result += hex[c >> 4]; result += hex[c & 15]; }
        else result += c;
    }
    return result + '"';
}

std::string Schema() {
    std::ostringstream out;
    out << "{\"ok\":true,\"api\":\"lr2-agent/v1\",\"commands\":[";
    bool first = true;
    for (const auto& command : GetCommandNames()) {
        if (!first) out << ',';
        first = false;
        int last = 0;
        for (int i = 1; i < 30; ++i) {
            CSTR field = GetCommandHelp(command.c_str(), i);
            if (field.body && *field.body) last = i;
        }
        out << "{\"name\":" << Quote(command) << ",\"fields\":[";
        for (int i = 1; i <= last; ++i) {
            if (i > 1) out << ',';
            CSTR field = GetCommandHelp(command.c_str(), i);
            out << "{\"column\":" << i << ",\"name\":" << Quote(Cp932ToUtf8(field.body))
                << ",\"value_kind\":" << GetCommandValueKind(command.c_str(), field.body) << '}';
        }
        out << "]}";
    }
    out << "],\"value_sets\":{";
    for (int kind = SE_VALUE_SKIN_TYPE; kind <= SE_VALUE_BARGRAPH; ++kind) {
        if (kind > SE_VALUE_SKIN_TYPE) out << ',';
        out << Quote(std::to_string(kind)) << ":[";
        bool firstValue = true;
        const auto valueKind = (SECommandValueKind)kind;
        for (int i = 0; i < GetCommandValueItemCount(valueKind); ++i) {
            const int value = GetCommandValueAt(valueKind, i);
            const char* name = GetCommandValueName(valueKind, value);
            if (!name || !*name) continue;
            if (!firstValue) out << ',';
            firstValue = false;
            out << "{\"value\":" << value << ",\"name\":" << Quote(Cp932ToUtf8(name)) << '}';
        }
        out << ']';
    }
    return out.str() + "}}";
}

std::unique_ptr<WORKSPACE> InspectWorkspace(const std::wstring& path, bool render) {
    if (!std::filesystem::is_regular_file(path)) throw std::runtime_error("Skin file does not exist.");
    auto workspace = std::make_unique<WORKSPACE>();
    const std::string nativePath = Encode(std::filesystem::absolute(path).wstring(), CP_ACP);
    if (nativePath.size() >= MAX_PATH) throw std::runtime_error("Skin path exceeds the native editor path limit.");
    strncpy_s(workspace->mainpath, nativePath.c_str(), _TRUNCATE);
    if (render) {
        if (!InitSkinData(&workspace->g.skinData)) throw std::runtime_error("Skin metadata allocation failed.");
        workspace->skinBrowserDataInitialized = true;
        ParseLR2SkinCustom(&workspace->g.skinData, CSTR(nativePath.c_str()));
        if (workspace->g.skinData.Count <= 0) throw std::runtime_error("Skin metadata could not be parsed.");
        workspace->meta = workspace->g.skinData.Data[0];
        if (workspace->LoadSkin(workspace->mainpath) != 0) throw std::runtime_error("Native runtime could not load the skin.");
    } else {
        if (workspace->ResetEditorDocumentForLoad() != 0) throw std::runtime_error("Workspace reset failed.");
        strncpy_s(workspace->mainpath, nativePath.c_str(), _TRUNCATE);
        if (workspace->LoadSkinScript(workspace->mainpath) < 0 ||
            workspace->RebuildEditorDerivedState() != 0 || !workspace->objectEditorModel.LoadGroups(nullptr))
            throw std::runtime_error("Native parser could not load the skin.");
        workspace->RebuildObjectModel();
    }
    workspace->loaded = true;
    if (workspace->objectEditorModel.Objects().empty()) throw std::runtime_error("No drawable Objects were found.");
    return workspace;
}

std::string Describe(const WORKSPACE& workspace) {
    std::ostringstream out;
    const auto& objects = workspace.objectEditorModel.Objects();
    const auto* rows = (const SKINFILELINEREAD*)workspace.skinfileLines.data;
    out << "{\"ok\":true,\"api\":\"lr2-agent/v1\",\"object_count\":" << objects.size()
        << ",\"row_count\":" << workspace.skinfileLines.count << ",\"objects\":[";
    for (size_t i = 0; i < objects.size(); ++i) {
        const auto& object = objects[i];
        if (i) out << ',';
        out << "{\"id\":" << Quote(object.editorId) << ",\"name\":" << Quote(Cp932ToUtf8(object.name.c_str()))
            << ",\"commands\":[";
        for (size_t j = 0; j < object.rows.size(); ++j) {
            if (j) out << ',';
            const auto& row = rows[object.rows[j]];
            out << "{\"command\":" << Quote(row.csv.str[0].body ? row.csv.str[0].body : "") << ",\"values\":[";
            for (int column = 1; column < row.csvColumnCount; ++column) {
                if (column > 1) out << ',';
                out << Quote(Cp932ToUtf8(row.csv.str[column].body));
            }
            out << "]}";
        }
        out << "]}";
    }
    return out.str() + "]}";
}
}

bool SEAgentUtilityMode() {
    const auto& args = Arguments();
    return args.size() > 1 && args[1].rfind(L"--agent-", 0) == 0;
}

int SERunAgentUtility(bool graphicsReady) {
    const auto& args = Arguments();
    if (!SEAgentUtilityMode()) return -1;
    const std::wstring& operation = args[1];
    const bool create = operation == L"--agent-create";
    const bool render = operation == L"--agent-render";
    const size_t expected = create ? 9 : render ? 5 : operation == L"--agent-inspect" ? 4 : operation == L"--agent-schema" ? 3 : 0;
    if (!expected || args.size() != expected) return 2;
    if ((create || render) && !graphicsReady) return -1;
    // Reserve the report before doing work; an existing file is never replaced.
    HANDLE report = CreateFileW(args.back().c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (report == INVALID_HANDLE_VALUE) return 2;
    std::string result;
    int exitCode = 0;
    try {
        if (LoadCommandHelp(nullptr) != 0) throw std::runtime_error("Embedded command schema is unavailable.");
        if ((create || render) && (!g_pd3dDevice || !DxLib_IsInit())) throw std::runtime_error("Direct3D 9 is unavailable.");
        if (operation == L"--agent-schema") result = Schema();
        else if (create) {
            auto number = [](const std::wstring& text) {
                size_t end = 0;
                int value = std::stoi(text, &end);
                if (end != text.size()) throw std::runtime_error("Expected an integer argument.");
                return value;
            };
            const int type = number(args[2]), width = number(args[3]), height = number(args[4]);
            if (width < 320 || width > 1920 || height < 240 || height > 1080)
                throw std::runtime_error("Resolution must be between 320x240 and 1920x1080.");
            for (int i : {5, 6}) {
                Encode(args[i], 932); // Reject lossy title/maker conversion before writing.
                if (args[i].find_first_of(L",\r\n") != std::wstring::npos || args[i].size() > 120)
                    throw std::runtime_error("Title/maker must be a CP932-compatible single CSV field (up to 120 characters).");
            }
            const auto root = std::filesystem::absolute(args[7]);
            if (!std::filesystem::create_directory(root)) throw std::runtime_error("Output directory already exists.");
            struct DirectoryScope {
                std::filesystem::path original = std::filesystem::current_path();
                ~DirectoryScope() { std::error_code ec; std::filesystem::current_path(original, ec); }
            } restore;
            std::filesystem::current_path(root);
            std::string skin, atlas, error;
            if (!SECreateAgentPreset(type, width, height, Encode(args[5], CP_UTF8), Encode(args[6], CP_UTF8), skin, atlas, error))
                throw std::runtime_error(error);
            result = "{\"ok\":true,\"api\":\"lr2-agent/v1\",\"skin\":" + Quote(skin) + ",\"atlas\":" + Quote(atlas) + "}";
        } else {
            if (render && std::filesystem::exists(args[3])) throw std::runtime_error("Preview output already exists.");
            auto workspace = InspectWorkspace(args[2], render);
            if (render) {
                workspace->previewLastRenderAt = 0;
                if (!workspace->UpdatePreviewRuntime(GetTickCount64()) ||
                    SaveSoftImageToPng(Encode(args[3], CP_ACP).c_str(), workspace->previewScreen, 6) != 0)
                    throw std::runtime_error("Native preview rendering failed.");
            }
            result = Describe(*workspace);
        }
    } catch (const std::exception& error) {
        result = "{\"ok\":false,\"error\":" + Quote(error.what()) + "}";
        exitCode = 1;
    }
    DWORD written = 0;
    if (!WriteFile(report, result.data(), (DWORD)result.size(), &written, nullptr) || written != result.size()) exitCode = 2;
    CloseHandle(report);
    return exitCode;
}
