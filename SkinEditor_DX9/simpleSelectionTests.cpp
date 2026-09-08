#include "winWorkspace.h"
#include "winWorkspaceUiHelpers.h"
#include "seHelper.h"
#include "../LR2/LR2_skinload.h"
#include "../LR2/LR2_skindraw.h"
#include "../LR2/LR2_skinobject.h"
#include "../LR2/En_fileutil.h"
#include "seLocalization.h"
#include "uiCatalog.h"
#include "imgui/imgui_internal.h"

#include <cstdio>
#include <climits>
#include <limits>

namespace {
struct SelectionFixture {
    std::unique_ptr<WORKSPACE> workspace = std::make_unique<WORKSPACE>();
    SelectionFixture() {
        auto& ws = *workspace;
        ws.skinfileLines.Alloc(sizeof(SKINFILELINEREAD), 64);
        ws.arr_CustomFile.Alloc(sizeof(CSTR), 2);
        ws.arr_SRCGR.Alloc(sizeof(SRCGR), 4);
        ws.arr_IMG.Alloc(sizeof(IMG), 4);
        ws.arr_SRC.Alloc(sizeof(SRC), 8);
        ws.arr_DST.Alloc(sizeof(DST), 16);
        ws.arr_seobj.Alloc(sizeof(SEOBJ), 8);
        ws.arr_ifunit.Alloc(sizeof(IFUNIT), 8);
        ws.arr_history.Alloc(sizeof(HISTORY), 8);
        strcpy_s(ws.mainpath, "C:\\selection-self-test\\main.lr2skin");
        ws.loaded = true;
        ws.meta.type = SKINTYPE_SELECT;
        ws.skinSizeX = 640;
        ws.skinSizeY = 480;
        Append("$FILE 'C:\\selection-self-test\\main.lr2skin' start");
        Append("#INFORMATION,5,Selection test,Self-test");
        Append("$SE_OBJECT_ID,song-title");
        Append("$SE_OBJECT_NAME,Song title");
        Append("#SRC_TEXT,0,0,10,0,0,0");
        Append("#DST_TEXT,0,0,50,60,220,20,0,220,210,200,190,0,0,,,300,11,900,,0,777,778,unknown");
        Append("// preserve comment between animation frames");
        Append("#DST_TEXT,0,300,70,80,220,20,0,240,210,200,190,0,0,,,0,0,,,,,,last-frame");
        Append("$SE_OBJECT_ID,background");
        Append("#SRC_IMAGE,0,0,0,0,32,32,1,1,0,0,0,0,0");
        Append("#DST_IMAGE,0,0,0,0,640,480,0,255,255,255,255,0,0,0,0,0,0,,0,0");
        Append("#SRC_BAR_BODY,0,0,0,0,200,20,1,1,0,0");
        Append("#DST_BAR_BODY_OFF,0,0,400,20,200,20,0,255,255,255,255,0,0,0,0,0,0");
        Append("#DST_BAR_BODY_OFF,1,0,400,40,200,20,0,255,255,255,255,0,0,0,0,0,0");
        Append("#DST_BAR_BODY_ON,0,0,400,20,200,20,0,255,255,255,255,0,0,0,0,0,0");
        Append("#SRC_BAR_TITLE,0,0,10,0");
        Append("#DST_BAR_TITLE,0,0,5,8,180,16,0,255,255,255,255,0,0,,,0,0,,,");
        Append("#SRC_BAR_LAMP,0,0,0,0,8,8,1,1,0,0");
        Append("#DST_BAR_LAMP,0,0,2,3,8,8,0,255,255,255,255,0,0,0,0,0,0,0,0,0");
        Append("#SRC_BAR_FLASH,0,0,0,0,200,20,1,1,0,0");
        Append("#DST_BAR_FLASH,0,0,0,0,200,20,0,200,255,255,255,0,0,0,0,0,0,0,0,0");
        Append("#SRC_BUTTON,0,0,0,0,20,10,2,1,0,0,1,1,0");
        Append("#DST_BUTTON,0,0,10,400,20,10,0,255,255,255,255,0,0,0,0,0,0,0,0,0");
        Append("#SRC_BARGRAPH,0,0,0,0,80,10,1,1,0,0,1,0,0");
        Append("#DST_BARGRAPH,0,0,10,200,80,10,0,255,255,255,255,0,0,0,0,0,0,0,0,0");
        const char* child = "C:\\selection-self-test\\parts.csv";
        Append("$FILE 'C:\\selection-self-test\\parts.csv' start", child);
        Append("#IF,900", child);
        Append("$SE_OBJECT_ID,conditional-number", child);
        Append("#SRC_NUMBER,0,0,0,0,100,10,10,1,0,0,45,1,3", child);
        Append("#DST_NUMBER,0,0,100,200,10,10,0,255,255,255,255,0,0,0,0,0,11,900,0,0", child);
        Append("#ELSE", child);
        Append("$SE_OBJECT_ID,alternative-number", child);
        Append("#SRC_NUMBER,0,0,0,0,100,10,10,1,0,0,46,1,3", child);
        Append("#DST_NUMBER,0,0,100,200,10,10,0,255,255,255,255,0,0,0,0,0,11,-900,0,0", child);
        Append("#ENDIF", child);
        Append("$FILE 'C:\\selection-self-test\\parts.csv' end", child);
        Append("$FILE 'C:\\selection-self-test\\main.lr2skin' end");
    }
    void Append(const char* text, const char* owner = nullptr) {
        auto& ws = *workspace;
        auto* row = (SKINFILELINEREAD*)ws.skinfileLines.Get_new();
        row->line.assign(text);
        row->filename.assign(owner ? owner : ws.mainpath);
        row->isComment = text[0] != '#';
        row->isSEcomment = text[0] == '$';
        SplitCSV(row->line, &row->csv, ",");
        row->csvColumnCount = CountCsvColumns(row->line);
    }
    bool Refresh() {
        if (workspace->RebuildEditorDerivedState() != 0 || !workspace->objectEditorModel.LoadGroups(nullptr)) return false;
        workspace->RebuildObjectModel();
        return true;
    }
    int Row(const char* command, int occurrence = 0) const {
        for (int row = 0; row < workspace->skinfileLines.count; ++row) {
            const auto& line = ((SKINFILELINEREAD*)workspace->skinfileLines.data)[row];
            if (line.csv.str[0].body && !strcmp(line.csv.str[0].body, command) && occurrence-- == 0) return row;
        }
        return -1;
    }
    int Value(const char* command, const char* field, int occurrence = 0) const {
        const int row = Row(command, occurrence);
        const int column = FindCommandFieldColumn(command, field);
        return row < 0 || column < 0 ? INT_MIN :
            ((SKINFILELINEREAD*)workspace->skinfileLines.data)[row].csv.val[column];
    }
    std::string Text(const char* command, int occurrence = 0) const {
        const int row = Row(command, occurrence);
        return row < 0 ? "" : ((SKINFILELINEREAD*)workspace->skinfileLines.data)[row].line.outstr();
    }
    bool Select(const char* id) {
        const auto& objects = workspace->objectEditorModel.Objects();
        for (int model = 0; model < (int)objects.size(); ++model) if (objects[model].editorId == id) {
            workspace->SetObjectSelection({ model }, model, model, false);
            return true;
        }
        return false;
    }
};

bool SameDocument(const SkinDocumentSnapshot& a, const SkinDocumentSnapshot& b) {
    if (a.lines.size() != b.lines.size()) return false;
    for (size_t row = 0; row < a.lines.size(); ++row)
        if (a.lines[row].line != b.lines[row].line || a.lines[row].filename != b.lines[row].filename) return false;
    return true;
}

bool Undo(SelectionFixture& fixture) {
    return fixture.workspace->UndoLastEdit() == 0 &&
        fixture.workspace->ApplyPendingHistorySnapshotRestore() == 0 && fixture.Refresh();
}
} // namespace

int RunSimpleSelectionSelfTest() {
    if (LoadCommandHelp(nullptr) != 0) return 1;
    SelectionFixture fixture;
    if (!fixture.Refresh()) return 2;
    auto& ws = *fixture.workspace;
    const auto original = ws.CaptureDocumentSnapshot();
    SESelectionEdit edit;
    std::string message;
    const auto check = [&](bool condition, int code) {
        if (!condition) fprintf(stderr, "Simple Selection check %d: %s\n", code, message.c_str());
        return condition;
    };
    edit.group = SESelectionGroup::SongList;
    if (!check(ws.GetSimpleSelectionTargets(edit).size() == 6, 3)) return 3;
    // Legacy BAR_TITLE need not belong to the Object Model to appear in a group.
    bool foundTitle = false;
    for (const auto& target : ws.GetSimpleSelectionTargets(edit))
        if (target.command == "#DST_BAR_TITLE") foundTitle = true;
    if (!foundTitle) return 4;
    edit.offsetX = 30; edit.offsetY = -10;
    if (!check(ws.ApplySimpleSelectionEdit(edit, message) && fixture.Refresh(), 5)) return 5;
    if (ws.arr_history.count != 1 || fixture.Value("#DST_BAR_BODY_OFF", "x") != 430 ||
        fixture.Value("#DST_BAR_BODY_OFF", "y", 1) != 30 ||
        fixture.Value("#DST_BAR_TITLE", "x") != 5 || fixture.Value("#DST_BAR_LAMP", "y") != 3 ||
        fixture.Value("#DST_TEXT", "x") != 50) return 6;
    if (!Undo(fixture) || !SameDocument(original, ws.CaptureDocumentSnapshot())) return 7;
    if (ws.RedoLastEdit() != 0 || ws.ApplyPendingHistorySnapshotRestore() != 0 || !fixture.Refresh() ||
        fixture.Value("#DST_BAR_BODY_OFF", "x") != 430 || !Undo(fixture)) return 8;

    edit.offsetX = edit.offsetY = 0; edit.scalePercent = 200;
    if (!ws.ApplySimpleSelectionEdit(edit, message) || !fixture.Refresh() ||
        fixture.Value("#DST_BAR_BODY_OFF", "x") != 400 ||
        fixture.Value("#DST_BAR_BODY_OFF", "y", 1) != 60 ||
        fixture.Value("#DST_BAR_TITLE", "x") != 10 ||
        fixture.Value("#DST_BAR_TITLE", "size") != 32 ||
        fixture.Value("#DST_BAR_BODY_OFF", "w") != 400 || !Undo(fixture)) return 9;

    edit = SESelectionEdit();
    edit.group = SESelectionGroup::SongInfo;
    edit.kind = SESelectionEditKind::Animation;
    edit.timer = 11;
    if (ws.ApplySimpleSelectionEdit(edit, message) || !SameDocument(original, ws.CaptureDocumentSnapshot()) ||
        ws.arr_history.count != 0) return 10; // Existing animation requires explicit replacement.
    edit.replaceAnimation = true;
    edit.durationMs = 400; edit.delayMs = 100;
    if (!check(ws.ApplySimpleSelectionEdit(edit, message) && fixture.Refresh(), 11)) return 11;
    if (ws.arr_history.count != 1 || ws.simpleSelectionPreviewTimers != std::vector<int>{11} ||
        fixture.Value("#DST_TEXT", "time") != 100 || fixture.Value("#DST_TEXT", "time", 1) != 500 ||
        fixture.Value("#DST_TEXT", "a") != 0 || fixture.Value("#DST_TEXT", "a", 1) != 240 ||
        fixture.Value("#DST_TEXT", "x") != 70 || fixture.Value("#DST_TEXT", "size") != 20 ||
        fixture.Value("#DST_TEXT", "timer") != 11 || fixture.Value("#DST_TEXT", "loop") != 500 ||
        fixture.Text("#DST_TEXT").find(",900,,0,777,778,unknown") == std::string::npos) return 12;
    const auto faded = ws.CaptureDocumentSnapshot();
    for (const auto& line : original.lines) {
        if (line.line.rfind("#DST_TEXT", 0) == 0 || line.line.rfind("#DST_NUMBER", 0) == 0) continue;
        if (std::none_of(faded.lines.begin(), faded.lines.end(), [&](const auto& saved) {
            return line.line == saved.line && line.filename == saved.filename;
        })) return 13;
    }
    // Feed the generated rows into LR2's real DST reader and interpolator.
    DSTstruct runtime = {};
    InitDST(&runtime);
    for (int row = 0; row < ws.skinfileLines.count; ++row) {
        auto& line = ((SKINFILELINEREAD*)ws.skinfileLines.data)[row];
        if (line.csv.str[0].isSame("#DST_TEXT")) ReadDST(&runtime, &line.csv, row);
    }
    const DSTdraw beforeStart = SetDSTdrawByTime(runtime, 50);
    const DSTdraw midpoint = SetDSTdrawByTime(runtime, 300);
    const DSTdraw afterEnd = SetDSTdrawByTime(runtime, 1500);
    const bool fadeWorks = beforeStart.time == -1 && midpoint.a == 120 && afterEnd.a == 240 &&
        runtime.timer == 11 && runtime.op1 == 900 && runtime.op4 == 777 && runtime.op5 == 778;
    free(runtime.draw);
    if (!fadeWorks || !Undo(fixture) || !SameDocument(original, ws.CaptureDocumentSnapshot())) return 14;

    edit.effect = SESelectionEffect::SlideIn;
    edit.slideX = -25; edit.slideY = 15;
    if (!fixture.Select("conditional-number")) return 15;
    edit.group = SESelectionGroup::SelectedObjects;
    if (!ws.ApplySimpleSelectionEdit(edit, message) || !fixture.Refresh() ||
        fixture.Value("#DST_NUMBER", "x") != 75 || fixture.Value("#DST_NUMBER", "x", 1) != 100 ||
        fixture.Value("#DST_NUMBER", "y") != 215 || fixture.Value("#DST_NUMBER", "x", 2) != 100 ||
        fixture.Value("#DST_TEXT", "a") != 220 ||
        ws.objectSelection.active.editorId != "conditional-number") return 16;
    const auto& inserted = ((SKINFILELINEREAD*)ws.skinfileLines.data)[fixture.Row("#DST_NUMBER", 1)];
    if (!inserted.filename.body || strcmp(inserted.filename.body, "C:\\selection-self-test\\parts.csv") || !Undo(fixture)) return 17;

    edit.group = SESelectionGroup::SongInfo;
    edit.selectedBranchOnly = true;
    edit.effect = SESelectionEffect::Pulse;
    edit.minimumOpacityPercent = 20;
    if (!fixture.Select("conditional-number") || ws.GetSimpleSelectionTargets(edit).size() != 1 ||
        !ws.ApplySimpleSelectionEdit(edit, message) || !fixture.Refresh() ||
        fixture.Value("#DST_NUMBER", "a", 1) != 51 ||
        fixture.Value("#DST_NUMBER", "loop") != 100 ||
        fixture.Value("#DST_NUMBER", "time", 2) != 500 || !Undo(fixture)) return 18;

    edit = SESelectionEdit();
    edit.group = SESelectionGroup::SongInfo;
    edit.kind = SESelectionEditKind::Tint;
    edit.tint[0] = 0.5f; edit.tint[3] = 0.5f;
    if (!ws.ApplySimpleSelectionEdit(edit, message) || !fixture.Refresh() ||
        fixture.Value("#DST_TEXT", "r") != 105 || fixture.Value("#DST_TEXT", "a") != 110 ||
        fixture.Value("#DST_TEXT", "a", 1) != 120 ||
        fixture.Value("#DST_TEXT", "time", 1) != 300 || !Undo(fixture)) return 19;

    const size_t redoBefore = ws.redoDocumentSnapshots.size();
    edit.kind = SESelectionEditKind::Layout;
    edit.offsetX = INT_MAX;
    if (ws.ApplySimpleSelectionEdit(edit, message) || ws.redoDocumentSnapshots.size() != redoBefore ||
        !SameDocument(original, ws.CaptureDocumentSnapshot())) return 20;
    edit.offsetX = 0; edit.scalePercent = std::numeric_limits<float>::quiet_NaN();
    if (ws.ApplySimpleSelectionEdit(edit, message)) return 21;
    edit = SESelectionEdit(); edit.group = SESelectionGroup::SongList;
    edit.kind = SESelectionEditKind::Animation; edit.timer = 11;
    if (ws.ApplySimpleSelectionEdit(edit, message) || !SameDocument(original, ws.CaptureDocumentSnapshot())) return 22;
    edit.timer = 0; edit.effect = SESelectionEffect::FadeOut;
    if (!ws.ApplySimpleSelectionEdit(edit, message) || !fixture.Refresh() ||
        fixture.Value("#DST_BAR_BODY_OFF", "a", 1) != 0 || !Undo(fixture)) return 23;

    SelectionFixture other;
    if (!other.Refresh()) return 24;
    edit = SESelectionEdit(); edit.offsetX = 1;
    const auto& projection = ws.GetSimpleSelectionTimelines();
    const size_t projectionSize = projection.size();
    if (!ws.ApplySimpleSelectionEdit(edit, message) || !fixture.Refresh() ||
        fixture.Value("#DST_TEXT", "x") != 51 || other.Value("#DST_TEXT", "x") != 50 ||
        ws.GetSimpleSelectionTimelines().size() != projectionSize || !Undo(fixture)) return 25;
    ws.meta.type = SKINTYPE_7KEYS;
    ws.InvalidateSimpleModeProjection();
    if (!ws.GetSimpleSelectionTimelines().empty() || ws.ApplySimpleSelectionEdit(edit, message)) return 26;
    ws.meta.type = SKINTYPE_SELECT;
    ws.InvalidateSimpleModeProjection();
    edit = SESelectionEdit(); edit.kind = SESelectionEditKind::Animation;
    edit.timer = 11; edit.replaceAnimation = true;
    if (!ws.ApplySimpleSelectionEdit(edit, message) || !fixture.Refresh() ||
        ws.simpleSelectionPreviewTimers.empty()) return 27;
    if (ws.EditValue(fixture.Row("#DST_TEXT"), FindCommandFieldColumn("#DST_TEXT", "x"), 90) != 0 ||
        !ws.simpleSelectionPreviewTimers.empty() || !fixture.Refresh()) return 28;

    // Submit the real Simple Mode view in both languages and narrow/wide docks.
    // This verifies ImGui ownership/layout, not native font rendering or mouse UX.
    ImGuiContext* previousContext = ImGui::GetCurrentContext();
    ImGuiContext* uiContext = ImGui::CreateContext();
    const SEUILanguage previousLanguage = SEGetUILanguage();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.DisplaySize = ImVec2(1200, 1000);
    io.DeltaTime = 1.0f / 60.0f;
    unsigned char* fontPixels = nullptr;
    int fontWidth = 0, fontHeight = 0;
    io.Fonts->GetTexDataAsRGBA32(&fontPixels, &fontWidth, &fontHeight);
    ws.simpleModeShowSelection = true;
    ws.wSimpleMode = true;
    ws.simpleSelectionEdit = SESelectionEdit();
    ws.simpleSelectionEdit.effect = SESelectionEffect::Pulse;
    char title[260];
    FormatSEUIWindowTitle(title, sizeof(title), SEUIWindowId::SimpleMode, ws.num);
    bool uiPassed = true;
    for (const auto language : { SEUILanguage::English, SEUILanguage::Korean }) {
        SESetUILanguage(language, false);
        for (float width : { 320.0f, 700.0f }) for (int frame = 0; frame < 3; ++frame) {
            ImGui::NewFrame();
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(width, 920));
            ws.drawSimpleMode();
            ImGui::Render();
            const ImGuiWindow* window = ImGui::FindWindowByName(title);
            if (frame == 2 && (!window || ImGui::GetDrawData()->TotalVtxCount == 0 ||
                window->ContentSize.x > window->InnerRect.GetWidth() + 1)) uiPassed = false;
        }
    }
    SESetUILanguage(previousLanguage, false);
    ImGui::DestroyContext(uiContext);
    ImGui::SetCurrentContext(previousContext);
    if (!uiPassed) return 29;
    return 0;
}
