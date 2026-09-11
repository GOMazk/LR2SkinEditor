#pragma once
#include "imgui/imgui.h"
#include <string>
#include <string_view>
#include <vector>

struct SECodeContext {
    int start = 0, end = 0, lineStart = 0, lineNumber = 1, column = 0;
    bool valid = false, known = false;
    std::string command, query, field, signature;
};

struct SECodeSuggestion {
    std::string text, label, detail;
};

// Presentation-only state. It never owns Workspace rows, selection or History.
struct SECodeAssistState {
    SECodeContext context;
    std::vector<SECodeSuggestion> suggestions;
    int cursor = -1, textLength = -1, selected = 0;
    bool open = false, dismissed = false, force = false, accept = false;
    bool focus = false, restoreCursor = false, scrollSelection = false;
    int savedCursor = 0, savedStart = 0, savedEnd = 0;
};

SECodeContext SEReadCodeContext(std::string_view text, int cursor);
std::vector<SECodeSuggestion> SEFindCodeSuggestions(const SECodeContext& context);
bool SEApplyCodeSuggestion(ImGuiInputTextCallbackData& data,
    const SECodeContext& context, const std::string& replacement);
bool SEDrawCodeEditorInput(const char* label, char* buffer, size_t capacity,
    ImVec2 size, SECodeAssistState& state, int workspaceNumber);
int RunCodeEditorAssistSelfTest();
