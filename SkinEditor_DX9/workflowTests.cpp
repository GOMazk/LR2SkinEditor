#include "winWorkspace.h"
#include "seHelper.h"
#include "uiCatalog.h"
#include "seUI.h"
#include "imgui/imgui_internal.h"
#include <filesystem>
#include <fstream>

namespace {
int TestWorkflowDockNavigation(WORKSPACE& ws) {
    ImGui::CreateContext();
    struct ContextCleanup { ~ContextCleanup() { ImGui::DestroyContext(); } } cleanup;
    auto& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.DisplaySize = ImVec2(1000, 700);
    io.DeltaTime = 1.0f / 60;
    unsigned char* pixels; int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    char preview[260], simple[260], image[260], font[260];
    FormatSEUIWindowTitle(preview, sizeof(preview), SEUIWindowId::Preview, ws.num);
    FormatSEUIWindowTitle(simple, sizeof(simple), SEUIWindowId::SimpleMode, ws.num);
    FormatSEUIWindowTitle(image, sizeof(image), SEUIWindowId::ImageManager, ws.num);
    FormatSEUIWindowTitle(font, sizeof(font), SEUIWindowId::ImageFontEditor, ws.num);
    const char* tools[] = {simple, image, font};
    const char* side = "Browser navigation fixture";
    const ImGuiID dock = 0x53455746;
    ImGuiWindow* sideChild = nullptr;
    ImGuiID sideActiveId = 0;
    ImVec2 backPosition;
    bool setup = false, previewVisible = false;
    ws.previewRevealRequested = false;
    auto frame = [&](const char* focus = nullptr) {
        ImGui::NewFrame();
        if (!setup) {
            ImGui::DockBuilderAddNode(dock, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dock, io.DisplaySize);
            ImGuiID mainNode, sideNode;
            ImGui::DockBuilderSplitNode(dock, ImGuiDir_Right, 0.3f, &sideNode, &mainNode);
            ImGui::DockBuilderDockWindow(preview, mainNode);
            for (const auto* tool : tools) ImGui::DockBuilderDockWindow(tool, mainNode);
            ImGui::DockBuilderDockWindow(side, sideNode);
            ImGui::DockBuilderFinish(dock);
            setup = true;
        }
        ImGui::DockSpaceOverViewport(dock);
        if (focus) ImGui::SetWindowFocus(focus);
        // Match the real submission order: Preview precedes its source tool.
        if (ws.previewRevealRequested) {
            SEUI::RevealWindowTab(preview);
            ws.previewRevealRequested = false;
        }
        previewVisible = ImGui::Begin(preview);
        ImGui::End();
        for (const auto* tool : tools) {
            if (ImGui::Begin(tool)) {
                if (ImGui::SmallButton("Back to Preview")) ws.RequestPreview();
                backPosition = ImGui::GetItemRectMin();
                backPosition.x += 5; backPosition.y += 5;
            }
            ImGui::End();
        }
        ImGui::Begin(side);
        ImGui::BeginChild("Inputs");
        sideChild = ImGui::GetCurrentWindow();
        sideActiveId = ImGui::GetID("active browser drag");
        ImGui::KeepAliveID(sideActiveId);
        ImGui::EndChild();
        ImGui::End();
        ImGui::Render();
    };
    for (int i = 0; i < 6; ++i) frame();
    const auto revision = ws.documentRevision;
    const int history = ws.arr_history.count;
    for (const auto* tool : tools) {
        frame(tool); frame(); frame();
        auto* source = ImGui::FindWindowByName(tool);
        auto* target = ImGui::FindWindowByName(preview);
        if (!source || !target || !source->DockNode || source->DockNode != target->DockNode ||
            source->DockNode->VisibleWindow != source) return 26;
        io.AddMousePosEvent(backPosition.x, backPosition.y);
        frame();
        io.AddMouseButtonEvent(0, true); frame();
        io.AddMouseButtonEvent(0, false); frame();
        if (!ws.previewRevealRequested) return 27;
        // One-frame tab changes aren't sufficient: it must stay in Preview.
        for (int i = 0; i < 8; ++i) {
            frame();
            if (i >= 2 && (!previewVisible || target->DockNode->VisibleWindow != target ||
                ImGui::GetCurrentContext()->NavWindow->RootWindow != target)) return 28;
        }
    }
    // Cross-pane reveal must not steal an active Browser child/drag focus.
    frame(simple); frame(); frame();
    ImGui::FocusWindow(sideChild);
    ImGui::SetActiveID(sideActiveId, sideChild);
    ws.RequestPreview();
    for (int i = 0; i < 8; ++i) {
        frame();
        auto* context = ImGui::GetCurrentContext();
        if (context->NavWindow != sideChild || context->ActiveId != sideActiveId ||
            (i >= 2 && !previewVisible)) return 29;
    }
    if (ws.documentRevision != revision || ws.arr_history.count != history) return 30;
    return 0;
}
}

int RunWorkflowSelfTest() {
    std::vector<std::unique_ptr<WORKSPACE>> workspaces;
    workspaces.push_back(std::make_unique<WORKSPACE>());
    workspaces.push_back(std::make_unique<WORKSPACE>());
    auto& ws = *workspaces.front();
    auto& hidden = *workspaces.back();
    hidden.alive = false;
    strcpy_s(ws.title, "Visible workspace"); strcpy_s(hidden.title, "Hidden workspace");
    if (!SECanExitWorkspaces(workspaces)) return 1;
    hidden.documentRevision = 1;
    if (SECanExitWorkspaces(workspaces) || hidden.PendingWork().size() != 1) return 2;
    hidden.MarkDocumentSaved();
    hidden.pendingHistorySnapshotRestore = 0;
    if (SECanExitWorkspaces(workspaces)) return 3;
    hidden.pendingHistorySnapshotRestore = -1;
    hidden.codeEditorOwner = "draft.csv";
    hidden.codeEditorBase = "original";
    hidden.codeEditorBuffer = {'e', 'd', 'i', 't', 0};
    hidden.customFileDraftDirty = true;
    hidden.imagePixelPaintDirtyPaths["missing-paint.png"] = true;
    hidden.imageFontEditor.fieldsDirty = true;
    const auto pending = hidden.PendingWork();
    if (pending.size() != 4 || SECanExitWorkspaces(workspaces)) return 4;
    const auto revision = hidden.documentRevision;
    std::string error;
    if (hidden.SavePendingWork(pending[0], error) || error.empty() ||
        hidden.SavePendingWork(pending[1], error) || hidden.documentRevision != revision ||
        hidden.codeEditorBase != "original" || !hidden.customFileDraftDirty) return 5;
    if (hidden.SavePaintImage("missing-paint.png", error) || error.empty() ||
        hidden.imagePixelPaintDirtyPaths.empty()) return 6;
    hidden.ReviewPendingWork(pending[0]);
    if (!hidden.alive || !hidden.wCodeEditor || !hidden.codeEditorRevealRequested ||
        std::string(hidden.codeEditorBuffer.data()) != "edit") return 7;
    hidden.ReviewPendingWork(pending[1]);
    if (!hidden.wCustomFiles || !hidden.customFilesRevealRequested) return 8;
    hidden.ReviewPendingWork(pending[3]);
    if (!hidden.wImageFontEditor || !hidden.imageFontRevealRequested || !hidden.imageFontEditor.fieldsDirty) return 9;
    hidden.RequestImageImport();
    if (hidden.imageImportRequested) return 10; // no loaded skin
    hidden.loaded = true;
    hidden.RequestImageImport(); hidden.RequestPreview();
    if (!hidden.wAssetBrowser || !hidden.wImgManager || !hidden.imageImportRequested ||
        !hidden.imageManagerRevealRequested || !hidden.wPreview || !hidden.previewRevealRequested) return 11;
    hidden.loaded = false;

    // Real external-font save/lock failure via the same review command; only
    // unique owned fixtures are written. Saving must not touch script History.
    namespace fs = std::filesystem;
    char temp[MAX_PATH] = {}, root[MAX_PATH] = {};
    if (!GetTempPathA(MAX_PATH, temp) || !GetTempFileNameA(temp, "sew", 0, root) ||
        !DeleteFileA(root) || !CreateDirectoryA(root, nullptr)) return 12;
    struct Cleanup { fs::path path; ~Cleanup() { std::error_code ec; fs::remove_all(path, ec); } } cleanup{fs::path(root)};
    const auto font = (cleanup.path / "font.lr2font").string();
    { std::ofstream out(font, std::ios::binary); out << "#S,16\r\n#M,0\r\n"; }
    if (!ws.OpenImageFont(font)) return 13;
    ws.imageFontEditor.height = 24;
    ws.imageFontEditor.fieldsDirty = ws.imageFontEditor.metricsDirty = true;
    HANDLE lock = CreateFileA(font.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (lock == INVALID_HANDLE_VALUE) return 14;
    const bool saved = ws.SavePendingWork({SEPendingKind::Font, font}, error);
    CloseHandle(lock);
    if (saved || !ws.imageFontEditor.Dirty() || error.empty()) return 15;
    if (!ws.SavePendingWork({SEPendingKind::Font, font}, error) || ws.imageFontEditor.Dirty() ||
        ws.IsDocumentDirty() || ws.arr_history.count != 0) return 16;

    // TEXT and BAR_TITLE use schema's font field; opening a second font must
    // queue the existing unsaved-font confirmation, not replace the draft.
    LoadCommandHelp("..\\skinHelper.txt");
    if (ws.ResetEditorDocumentForLoad() != 0) return 17;
    SkinDocumentSnapshot snapshot;
    snapshot.lines.push_back({"test.lr2skin", "#SRC_TEXT,0,3,10,1,0,0"});
    if (ws.RestoreDocumentSnapshot(snapshot) != 0) return 18;
    ws.objectEditorModel.ObjectsMutable().push_back(SEObjectInstance());
    ws.objectEditorModel.ObjectsMutable().back().rows = {0};
    ws.objectSelection.active.editorId = "keep";
    ws.imageFontRuntimePaths[3] = font;
    ws.imageFontEditor.document.path = "other.lr2font";
    ws.imageFontEditor.fieldsDirty = true;
    if (!ws.OpenObjectFont(0) || !ws.imageFontEditor.pendingOpen ||
        ws.imageFontEditor.pendingPath != font || ws.imageFontEditor.document.path != "other.lr2font" ||
        !ws.imageFontEditor.fieldsDirty || ws.objectSelection.active.editorId != "keep") return 19;
    ws.imageFontRuntimePaths[3].clear();
    if (ws.OpenObjectFont(0) || ws.workflowStatus.empty()) return 20;
    if (ws.OpenObjectImage(-1) || ws.OpenObjectAppearance(-1)) return 21;

    ImGui::CreateContext();
    auto& io = ImGui::GetIO(); io.IniFilename = nullptr;
    io.DisplaySize = ImVec2(800, 600); io.DeltaTime = 1.0f / 60;
    unsigned char* pixels; int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    bool exitRequested = true;
    for (int frame = 0; frame < 3; ++frame) {
        ImGui::NewFrame();
        if (SEDrawPendingChanges(workspaces, exitRequested)) return 22;
        ImGui::Render();
    }
    // Escape cancels close without changing any draft.
    io.AddKeyEvent(ImGuiKey_Escape, true);
    ImGui::NewFrame();
    const bool closed = SEDrawPendingChanges(workspaces, exitRequested);
    ImGui::Render();
    if (closed || exitRequested || hidden.PendingWork().size() != 4) return 23;
    ImGui::DestroyContext();
    ws.ResetEditorDocumentForLoad();
    ws.MarkDocumentSaved();
    ws.imageFontEditor = SEImageFontEditor();
    hidden.codeEditorBuffer.clear(); hidden.customFileDraftDirty = false;
    hidden.imagePixelPaintDirtyPaths.clear(); hidden.imageFontEditor = SEImageFontEditor();
    if (!SECanExitWorkspaces(workspaces)) return 24;
    if (SEUIWindowSpecFor(SEUIWindowId::ImageManager).defaultVisible ||
        SEUIWindowSpecFor(SEUIWindowId::SimpleMode).defaultVisible ||
        SEUIWindowSpecFor(SEUIWindowId::DstView).defaultVisible ||
        !SEUIWindowSpecFor(SEUIWindowId::AssetBrowser).defaultVisible) return 25;
    return TestWorkflowDockNavigation(ws);
}
