#include "codeEditorAssist.h"
#include "seHelper.h"
#include "imgui/imgui_internal.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

int RunCodeEditorAssistSelfTest() {
    if (LoadCommandHelp(nullptr) != 0) return 1;
    if (!GetCommandHelp("#SRC_IMAGE_EXTRA", 0).isSame("WIP") ||
        !GetCommandHelp(nullptr, 0).isSame("WIP") ||
        !GetCommandHelp("#SRC_IMAGE", 30).isSame("WIP")) return 2;
    auto context = SEReadCodeContext("  #src_n,0,1", 8);
    auto suggestions = SEFindCodeSuggestions(context);
    if (!context.valid || context.start != 2 || context.end != 8 || suggestions.empty()) return 3;
    if (SEFindCodeSuggestions(SEReadCodeContext("// #SRC_N", 9)).size() ||
        SEFindCodeSuggestions(SEReadCodeContext("$SE_OBJECT_NAME,#SRC_N", 21)).size()) return 4;
    suggestions = SEFindCodeSuggestions(SEReadCodeContext("note", 4));
    if (std::none_of(suggestions.begin(), suggestions.end(), [](const auto& item) {
        return item.text == "#DST_NOTE";
    })) return 5;
    std::string timer = "#SRC_IMAGE,0,0,0,0,16,16,1,1,0,SongCh";
    context = SEReadCodeContext(timer, (int)timer.size());
    suggestions = SEFindCodeSuggestions(context);
    if (!context.known || context.column != 10 || context.field != "$timer" ||
        suggestions.empty() || suggestions[0].text != "11" ||
        context.signature.find("[ $timer ]") == std::string::npos) return 6;
    const std::string japanese = "// \xE6\x97\xA5\xE6\x9C\xAC\n#IF,";
    context = SEReadCodeContext(japanese, (int)japanese.size());
    if (!context.valid || context.lineNumber != 2 || context.column != 1 ||
        context.start != (int)japanese.size()) return 7;

    char buffer[512] = "  #src_n  ,0,1\n// unchanged";
    ImGuiInputTextCallbackData data;
    data.Buf = buffer;
    data.BufSize = sizeof(buffer);
    data.BufTextLen = (int)strlen(buffer);
    context = SEReadCodeContext(buffer, 8);
    if (!SEApplyCodeSuggestion(data, context, "#SRC_NOTE") ||
        strcmp(buffer, "  #SRC_NOTE  ,0,1\n// unchanged")) return 8;
    char tiny[8] = "#SRC_N";
    data.Buf = tiny;
    data.BufSize = sizeof(tiny);
    data.BufTextLen = (int)strlen(tiny);
    if (SEApplyCodeSuggestion(data, SEReadCodeContext(tiny, 6), "#SRC_NOTE") ||
        strcmp(tiny, "#SRC_N")) return 9;
    strcpy_s(buffer, timer.c_str());
    data.Buf = buffer;
    data.BufSize = sizeof(buffer);
    data.BufTextLen = (int)strlen(buffer);
    if (!SEApplyCodeSuggestion(data, SEReadCodeContext(timer, (int)timer.size()), "11") ||
        std::string(buffer) != timer.substr(0, timer.size() - 6) + "11") return 10;

    // Exercise the real ImGui text callback and key routing, without sending
    // keyboard events to the desktop or needing a native graphics device.
    struct ContextScope {
        ImGuiContext* previous = ImGui::GetCurrentContext();
        ImGuiContext* test = ImGui::CreateContext();
        ~ContextScope() { ImGui::DestroyContext(test); ImGui::SetCurrentContext(previous); }
    } scope;
    auto& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.DisplaySize = ImVec2(900, 700);
    io.DeltaTime = 1.0f / 60;
    io.ConfigInputTrickleEventQueue = false;
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    buffer[0] = 0;
    SECodeAssistState state;
    state.focus = true;
    auto frame = [&]() {
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(800, 620));
        ImGui::Begin("Code assistance test", nullptr, ImGuiWindowFlags_NoDecoration);
        SEDrawCodeEditorInput("##draft", buffer, sizeof(buffer), ImVec2(760, 570), state, 77);
        ImGui::End();
        ImGui::Render();
    };
    auto press = [&](ImGuiKey key, bool ctrl = false) {
        if (ctrl) io.AddKeyEvent(ImGuiMod_Ctrl, true);
        io.AddKeyEvent(key, true); frame();
        io.AddKeyEvent(key, false);
        if (ctrl) io.AddKeyEvent(ImGuiMod_Ctrl, false);
        frame();
    };
    frame(); frame();
    io.AddInputCharactersUTF8("#SRC_N"); frame(); frame();
    if (!state.open || state.suggestions.size() < 2 || strcmp(buffer, "#SRC_N")) return 20;
    const int cursor = state.cursor;
    press(ImGuiKey_DownArrow);
    if (state.selected != 1 || state.cursor != cursor || strcmp(buffer, "#SRC_N")) return 21;
    const std::string expected = state.suggestions[1].text + ",";
    press(ImGuiKey_Tab);
    if (buffer != expected || state.open) return 22;
    press(ImGuiKey_Z, true);
    if (strcmp(buffer, "#SRC_N")) return 23;
    press(ImGuiKey_Space, true);
    if (!state.open) return 24;
    press(ImGuiKey_Escape);
    if (state.open || strcmp(buffer, "#SRC_N")) return 25;
    press(ImGuiKey_Space, true);
    if (!state.open) return 26;
    const std::string enterExpected = state.suggestions[state.selected].text + ",";
    press(ImGuiKey_Enter);
    if (buffer != enterExpected || strchr(buffer, '\n')) return 27;
    press(ImGuiKey_Enter);
    if (std::string(buffer) != enterExpected + "\n") return 28;
    press(ImGuiKey_Tab);
    if (std::string(buffer) != enterExpected + "\n\t") return 29;
    // Completing a second line must preserve everything already in the draft.
    io.AddInputCharactersUTF8("dst_n"); frame(); frame();
    if (!state.open) return 30;
    const std::string clicked = state.suggestions[0].text + ",";
    ImGuiWindow* popup = ImGui::FindWindowByName("Text suggestions##code-suggestions-77");
    if (!popup || !popup->Active) return 31;
    const ImVec2 click(popup->Pos.x + 28, popup->Pos.y + ImGui::GetStyle().WindowPadding.y +
        ImGui::GetTextLineHeightWithSpacing() + ImGui::GetTextLineHeight() * 0.5f);
    io.AddMousePosEvent(click.x, click.y); frame();
    io.AddMouseButtonEvent(0, true); frame();
    io.AddMouseButtonEvent(0, false); frame(); frame(); frame();
    if (std::string(buffer) != enterExpected + "\n\t" + clicked) return 32;
    SECodeAssistState other;
    if (other.open || !other.suggestions.empty() || other.cursor != -1) return 33;
    return 0;
}
