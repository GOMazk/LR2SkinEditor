#include "winWorkspace.h"
#include "seHelper.h"
#include "uiCatalog.h"
#include "seUI.h"
#include "imgui/imgui_internal.h"
#include "agentDiagnostics.h"
#include <filesystem>
#include <fstream>

namespace {
int TestCompactWorkflowPanels() {
    auto storage = std::make_unique<WORKSPACE>();
    auto& ws = *storage;
    if (ws.ResetEditorDocumentForLoad() != 0 || !ws.objectEditorModel.LoadGroups(nullptr)) return 70;
    SkinDocumentSnapshot snapshot;
    snapshot.lines = {
        {"compact.csv", "$SE_OBJECT_ID,compact-object"},
        {"compact.csv", "#SRC_IMAGE,0,111,0,0,40,50,1,1,0,0"},
        {"compact.csv", "#DST_IMAGE,0,0,20,30,40,50,0,255,255,255,255,1,0,0,0,0,0,0,0,0"}
    };
    for (int i = 1; i < 20; ++i)
        snapshot.lines.push_back({"compact.csv", "$SRC_IMAGE,111," + std::to_string(i * 40) + ",0,40,50,1,1,0,0"});
    if (ws.RestoreDocumentSnapshot(snapshot) != 0 || ws.RebuildEditorDerivedState() != 0 || ws.arr_IMG.count < 10) return 71;
    ws.RebuildObjectModel();
    ws.SetObjectSelection({0}, 0, 0, false);
    ws.loaded = true;
    ws.wAssetBrowser = ws.wObjectInspector = true;
    ws.objectInspectorRevealRequested = false;
    const auto revision = ws.documentRevision;
    const int history = ws.arr_history.count;

    ImGui::CreateContext();
    struct Cleanup {
        WORKSPACE& ws;
        ~Cleanup() { ImGui::DestroyContext(); ws.loaded = false; }
    } cleanup{ws};
    auto& io = ImGui::GetIO();
    io.IniFilename = nullptr; io.DisplaySize = ImVec2(1920, 1080); io.DeltaTime = 1.0f / 60;
    unsigned char* pixels; int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    SEUI::ApplyModernTheme(1.0f);

    // The last section used to disappear behind scrolling tab arrows. Test
    // actual mouse selection, resizing and changing frame-count labels.
    int section = 0;
    ImVec2 lastMin, lastMax;
    bool navFits = true;
    auto navigationFrame = [&](float paneWidth, const char* timeline) {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(paneWidth, 220));
        ImGui::Begin("Compact navigation");
        const char* labels[] = {"SRC", "Layout", timeline, "Conditions", "Advanced LR2"};
        section = SEUI::SectionSelector("sections", labels, 5, section);
        lastMin = ImGui::GetItemRectMin(); lastMax = ImGui::GetItemRectMax();
        navFits = lastMax.x <= ImGui::GetWindowPos().x + paneWidth - ImGui::GetStyle().WindowPadding.x + 1;
        ImGui::End(); ImGui::Render();
    };
    for (int i = 0; i < 3; ++i) navigationFrame(280, "Timeline (1)");
    if (!navFits) return 72;
    io.AddMousePosEvent((lastMin.x + lastMax.x) * .5f, (lastMin.y + lastMax.y) * .5f);
    navigationFrame(280, "Timeline (1)");
    io.AddMouseButtonEvent(0, true); navigationFrame(280, "Timeline (1)");
    io.AddMouseButtonEvent(0, false); navigationFrame(280, "Timeline (1)");
    if (section != 4) return 73;
    navigationFrame(720, "Timeline (123)");
    if (!navFits || section != 4) return 74;
    io.FontGlobalScale = 1.25f;
    for (int i = 0; i < 3; ++i) navigationFrame(280, "Timeline (123)");
    if (!navFits || section != 4) return 75;
    io.FontGlobalScale = 1;
    io.AddMousePosEvent(-100, -100);

    char assetTitle[128], inspectorTitle[128];
    FormatSEUIWindowTitle(assetTitle, sizeof(assetTitle), SEUIWindowId::AssetBrowser, ws.num);
    FormatSEUIWindowTitle(inspectorTitle, sizeof(inspectorTitle), SEUIWindowId::ObjectInspector, ws.num);
    auto panelFrame = [&](float assetWidth, float assetHeight, float inspectorWidth) {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0, 300));
        ImGui::SetNextWindowSize(ImVec2(assetWidth, assetHeight));
        ws.drawAssetBrowser();
        ImGui::SetNextWindowPos(ImVec2(1100, 0));
        ImGui::SetNextWindowSize(ImVec2(inspectorWidth, 700));
        ws.drawObjectInspector();
        ImGui::Render();
    };
    auto gridWindow = [&]() -> ImGuiWindow* {
        auto* parent = ImGui::FindWindowByName(assetTitle);
        if (parent) for (auto* child : parent->DC.ChildWindows)
            if (strstr(child->Name, "##AssetGrid")) return child;
        return nullptr;
    };
    // FHD's short lower dock should fit a complete default-size thumbnail
    // and caption, not only its upper half. No forced layout reset required.
    for (int i = 0; i < 4; ++i) panelFrame(980, 220, 320);
    auto* grid = gridWindow();
    if (!grid || grid->InnerRect.GetHeight() < 96 + 14 + ImGui::GetFontSize() + 5 ||
        grid->ScrollMax.x > 0 || ImGui::FindWindowByName(assetTitle)->ScrollMax.x > 0) return 76;
    const float compactHeight = grid->ContentSize.y;
    ws.assetDetailedCards = true;
    for (int i = 0; i < 3; ++i) panelFrame(980, 220, 320);
    if (!gridWindow() || gridWindow()->ContentSize.y <= compactHeight) return 77;
    ws.assetDetailedCards = false;
    for (int selected = 0; selected < 5; ++selected) {
        ws.objectInspectorSection = selected;
        for (int i = 0; i < 3; ++i) panelFrame(380, 300, 280);
        if (ws.objectInspectorSection != selected || !gridWindow() || gridWindow()->ScrollMax.x > 0 ||
            ImGui::FindWindowByName(assetTitle)->ScrollMax.x > 0) return 78;
    }
    // A requested large thumbnail is clamped to the pane, without changing
    // the user's slider preference. The narrow header must wrap as well.
    ws.assetThumbnailSize = 192;
    for (int i = 0; i < 3; ++i) panelFrame(190, 400, 600);
    if (!gridWindow() || gridWindow()->ScrollMax.x > 0 || ws.assetThumbnailSize != 192 ||
        ImGui::FindWindowByName(assetTitle)->ScrollMax.x > 0) return 79;
    if (ws.documentRevision != revision || ws.arr_history.count != history ||
        ws.objectSelection.active.editorId != "compact-object") return 80;
    return 0;
}

int TestWorkflowVisibility() {
    auto storage = std::make_unique<WORKSPACE>();
    auto& ws = *storage;
    if (ws.ResetEditorDocumentForLoad() != 0 || !ws.objectEditorModel.LoadGroups(nullptr)) return 40;
    SkinDocumentSnapshot snapshot;
    const char* lines[] = {
        "$SE_OBJECT_ID,visibility-image", "#SRC_IMAGE,0,111,0,0,40,50,1,1,0,0",
        "#DST_IMAGE,0,0,20,30,40,50,0,255,255,255,255,1,0,0,0,0,41,900,0,0",
        "#DST_IMAGE,0,1000,20,30,40,50,0,255,255,255,255,1,0,0,0,0,41,900,0,0",
        "$SE_OBJECT_ID,visibility-second", "#SRC_IMAGE,0,111,0,0,40,50,1,1,0,0",
        "#DST_IMAGE,0,0,20,30,40,50,0,255,255,255,255,1,0,0,0,0,0,0,0,0",
        "$SE_OBJECT_ID,visibility-combo", "#SRC_NOWCOMBO_1P,0,111,0,0,100,20,10,1,0,0,0,0,5",
        "#DST_NOWCOMBO_1P,0,0,20,30,10,20,0,255,255,255,255,1,0,0,0,0,41",
        "$SE_OBJECT_ID,visibility-no-dst", "#SRC_IMAGE,0,111,0,0,40,50,1,1,0,0"
    };
    for (const auto* line : lines) snapshot.lines.push_back({"visibility.csv", line});
    if (ws.RestoreDocumentSnapshot(snapshot) != 0) return 41;
    ws.RebuildObjectModel();
    const int model = SEFindObjectForRow(ws.objectEditorModel.Objects(), 1);
    const int second = SEFindObjectForRow(ws.objectEditorModel.Objects(), 5);
    const int combo = SEFindObjectForRow(ws.objectEditorModel.Objects(), 8);
    const int noDst = SEFindObjectForRow(ws.objectEditorModel.Objects(), 11);
    if (model < 0 || second < 0 || combo < 0 || noDst < 0) return 42;
    ws.loaded = true;
    ws.previewReloadPending = ws.editorDerivedRebuildPending = ws.objectModelRebuildPending = false;
    ws.previewRuntimeLineMask.assign(ws.skinfileLines.count, 1);
    ws.g.skstruct.adjust.rate_x = ws.g.skstruct.adjust.rate_y = 100;
    ws.g.skstruct.op[0] = ws.g.skstruct.op[900] = 1;
    for (auto& clock : ws.g.timer1.clock) clock = -1;
    ws.g.timer1.flagMovieTimer = true; ws.g.timer1.movieFramerate = 60;
    ws.g.timer1.movieTimer = 500;
    ws.g.timer1.clock[0] = ws.g.timer1.clock[41] = 0;
    auto has = [](const SEObjectPreviewVisibility& result, const char* code) {
        for (const auto& issue : result.issues) if (issue.code == code) return true;
        return false;
    };
    auto inspect = [&]() { return SEObjectPreviewDiagnostics(ws, model); };
    const auto revision = ws.documentRevision;
    const int history = ws.arr_history.count;
    ws.objectSelection.active.editorId = "keep-selection";
    if (inspect().status != "not_proven_hidden" || !inspect().issues.empty() ||
        SEObjectPreviewDiagnostics(ws, second).status != "not_proven_hidden") return 43;
    // Use the loaded native mask, not a second IF/ELSE/include evaluator.
    ws.previewRuntimeLineMask[2] = ws.previewRuntimeLineMask[3] = 0;
    if (!has(inspect(), "if_inactive") || inspect().status != "hidden") return 44;
    ws.previewRuntimeLineMask[2] = ws.previewRuntimeLineMask[3] = 1;
    ws.previewHiddenFiles = {"VISIBILITY.CSV"};
    if (!has(inspect(), "preview_file_hidden") || inspect().status != "hidden") return 45;
    ws.previewHiddenFiles.clear();
    ws.previewSelectedFileOnly = true; ws.objectBrowserFile = "another.csv";
    if (!has(inspect(), "preview_file_hidden")) return 46;
    ws.previewSelectedFileOnly = false;
    ws.g.skstruct.op[900] = 0;
    if (!has(inspect(), "dst_option_false") || inspect().status != "hidden") return 47;
    auto* rows = (SKINFILELINEREAD*)ws.skinfileLines.data;
    rows[2].csv.val[18] = -900;
    if (has(inspect(), "dst_option_false")) return 48;
    ws.g.skstruct.op[900] = 1;
    if (!has(inspect(), "dst_option_false")) return 49;
    rows[2].csv.val[18] = 900;
    ws.g.timer1.clock[41] = -1;
    if (!has(inspect(), "timer_inactive") || inspect().status != "hidden") return 50;
    // NOWCOMBO is scene-controlled; an inactive raw timer must not prove it hidden.
    const auto special = SEObjectPreviewDiagnostics(ws, combo);
    if (!has(special, "special_runtime_semantics") || special.status != "unknown") return 51;
    ws.g.timer1.clock[41] = 0;
    rows[2].csv.val[17] = 140; ws.g.timer1.Rhythm = -1;
    if (!has(inspect(), "timer_inactive")) return 63;
    ws.g.timer1.Rhythm = 500;
    if (has(inspect(), "timer_inactive")) return 64;
    rows[2].csv.val[17] = 41;
    rows[2].csv.val[2] = 600;
    if (!has(inspect(), "before_animation")) return 52;
    rows[2].csv.val[2] = 0; rows[2].csv.val[16] = -1;
    ws.g.timer1.movieTimer = 2000;
    if (!has(inspect(), "after_animation")) return 53;
    rows[2].csv.val[16] = 0; ws.g.timer1.movieTimer = 500;
    rows[2].csv.val[8] = rows[3].csv.val[8] = 0;
    if (!has(inspect(), "alpha_zero") || inspect().status != "hidden") return 54;
    rows[2].csv.val[12] = rows[3].csv.val[12] = 0;
    if (inspect().status == "hidden") return 65; // blend 0 ignores alpha
    rows[2].csv.val[12] = rows[3].csv.val[12] = 1;
    rows[2].csv.val[8] = rows[3].csv.val[8] = 255;
    rows[2].csv.val[5] = rows[3].csv.val[5] = 0;
    if (!has(inspect(), "zero_size") || inspect().status != "hidden") return 66;
    rows[2].csv.val[5] = rows[3].csv.val[5] = 40;
    rows[2].csv.val[3] = rows[3].csv.val[3] = 1000;
    if (!has(inspect(), "offscreen") || inspect().status != "hidden") return 67;
    rows[2].csv.val[3] = rows[3].csv.val[3] = 20;
    auto& objectRows = ws.objectEditorModel.ObjectsMutable()[model].rows;
    const auto originalRows = objectRows;
    objectRows.push_back(5); objectRows.push_back(6);
    ws.g.skstruct.op[900] = 0;
    const auto multiple = inspect();
    if (multiple.timelines != 2 || multiple.hiddenTimelines != 1 || multiple.status == "hidden") return 68;
    objectRows = originalRows; ws.g.skstruct.op[900] = 1;
    ws.previewReloadPending = true;
    if (!has(inspect(), "preview_refresh_pending") || inspect().status != "unknown") return 55;
    ws.previewReloadPending = false;
    ws.previewRuntimeLineMask.clear();
    if (!has(inspect(), "condition_state_unknown") || inspect().status != "unknown") return 56;
    ws.previewRuntimeLineMask.assign(ws.skinfileLines.count, 1);
    ws.previewLayoutMode = true; ws.g.timer1.clock[41] = -1; ws.g.skstruct.op[900] = 0;
    if (inspect().status != "layout" || has(inspect(), "timer_inactive") || has(inspect(), "dst_option_false")) return 57;
    ws.previewHiddenFiles = {"visibility.csv"};
    if (!has(inspect(), "preview_file_hidden") || inspect().status != "hidden") return 58;
    ws.previewHiddenFiles.clear(); ws.previewLayoutMode = false;
    if (!has(SEObjectPreviewDiagnostics(ws, noDst), "missing_dst")) return 59;
    if (!SEObjectPreviewDiagnostics(ws, -1).issues.empty()) return 60;
    // The CLI still serializes the same shared analysis; no JSON parsing in UI.
    const std::string json = SEAgentDiagnostics(ws);
    if (json.find("\"scope\":\"current_native_preview\"") == std::string::npos ||
        json.find("\"code\":\"dst_option_false\"") == std::string::npos) return 61;
    if (ws.documentRevision != revision || ws.arr_history.count != history ||
        ws.objectSelection.active.editorId != "keep-selection" || ws.g.timer1.clock[41] != -1 ||
        ws.g.skstruct.op[900] != 0 || !ws.previewHiddenFiles.empty()) return 62;
    // Exercise tooltip Begin/End/wrapping against the same workspace projection.
    ImGui::CreateContext();
    auto& io = ImGui::GetIO(); io.IniFilename = nullptr;
    io.DisplaySize = ImVec2(800, 600); io.DeltaTime = 1.0f / 60;
    unsigned char* pixels; int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    for (int i = 0; i < 3; ++i) {
        ImGui::NewFrame();
        if (ImGui::BeginTooltip()) { ws.DrawObjectPreviewVisibilityDetails(model); ImGui::EndTooltip(); }
        ImGui::Render();
    }
    ImGui::DestroyContext();
    ws.loaded = false;
    return 0;
}

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
    if (!SECanExitWorkspaces(workspaces) || SEPendingWorkCount(workspaces) != 0) return 1;
    hidden.documentRevision = 1;
    if (SECanExitWorkspaces(workspaces) || hidden.PendingWork().size() != 1 ||
        SEPendingWorkCount(workspaces) != 1) return 2;
    hidden.MarkDocumentSaved();
    if (SEPendingWorkCount(workspaces) != 0) return 31;
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
    if (pending.size() != 4 || SECanExitWorkspaces(workspaces) ||
        SEPendingWorkCount(workspaces) != 4) return 4;
    ws.documentRevision = 1;
    if (SEPendingWorkCount(workspaces) != 5) return 32;
    ws.MarkDocumentSaved();
    if (SEPendingWorkCount(workspaces) != 4) return 33;
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
    if (SEPendingWorkCount(workspaces) != 4) return 34;

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
    if (!SECanExitWorkspaces(workspaces) || SEPendingWorkCount(workspaces) != 0) return 24;
    if (SEUIWindowSpecFor(SEUIWindowId::ImageManager).defaultVisible ||
        SEUIWindowSpecFor(SEUIWindowId::SimpleMode).defaultVisible ||
        SEUIWindowSpecFor(SEUIWindowId::DstView).defaultVisible ||
        !SEUIWindowSpecFor(SEUIWindowId::AssetBrowser).defaultVisible) return 25;
    const int visibility = TestWorkflowVisibility();
    if (visibility) return visibility;
    const int compact = TestCompactWorkflowPanels();
    if (compact) return compact;
    return TestWorkflowDockNavigation(ws);
}
