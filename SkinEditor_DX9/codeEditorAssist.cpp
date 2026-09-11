#include "codeEditorAssist.h"
#include "seHelper.h"
#include "seUI.h"
#include "uiCatalog.h"
#include "inputwrap.h"
#include "imgui/imgui_internal.h"
#include <algorithm>
#include <cctype>
#include <cstring>

namespace {
std::string Fold(std::string text) {
    for (char& c : text) if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
    return text;
}

std::string Signature(const std::string& command, int active = -1) {
    int last = 0;
    for (int i = 1; i < 30; ++i) {
        CSTR help = GetCommandHelp(command.c_str(), i);
        if (help.body && *help.body && !help.isSame("WIP")) last = i;
    }
    std::string result = command;
    for (int i = 1; i <= last; ++i) {
        CSTR help = GetCommandHelp(command.c_str(), i);
        result += ", ";
        if (i == active) result += "[ ";
        result += help.body ? Cp932ToUtf8(help.body) : "";
        if (i == active) result += " ]";
    }
    return result;
}

int MatchRank(const std::string& text, const std::string& query) {
    const size_t match = Fold(text).find(Fold(query));
    return match == 0 ? 0 : match != std::string::npos ? 1 : 2;
}

int AssistCallback(ImGuiInputTextCallbackData* data) {
    auto& state = *(SECodeAssistState*)data->UserData;
    if (state.restoreCursor) {
        data->CursorPos = state.savedCursor;
        data->SelectionStart = state.savedStart;
        data->SelectionEnd = state.savedEnd;
        state.restoreCursor = false;
    }
    if (state.accept) {
        state.accept = false;
        const SECodeContext current = SEReadCodeContext(
            std::string_view(data->Buf, data->BufTextLen), data->CursorPos);
        // A click is applied next frame; reject it if the text/caret changed.
        if (current.valid && current.start == state.context.start &&
            current.end == state.context.end && current.query == state.context.query &&
            current.command == state.context.command && !state.suggestions.empty())
            SEApplyCodeSuggestion(*data, current, state.suggestions[state.selected].text);
        state.open = false;
        state.dismissed = true;
        state.context = SEReadCodeContext(std::string_view(data->Buf, data->BufTextLen), data->CursorPos);
        state.cursor = data->CursorPos;
        state.textLength = data->BufTextLen;
        return 0;
    }
    const bool changed = data->EventFlag == ImGuiInputTextFlags_CallbackEdit ||
        state.cursor != data->CursorPos || state.textLength != data->BufTextLen;
    if (changed || state.force) {
        state.dismissed = false;
        state.context = SEReadCodeContext(std::string_view(data->Buf, data->BufTextLen), data->CursorPos);
        state.suggestions = SEFindCodeSuggestions(state.context);
        state.selected = 0;
        state.scrollSelection = true;
        state.open = !state.dismissed && !data->HasSelection() &&
            !state.suggestions.empty() && (state.force || !state.context.query.empty() ||
                (state.context.known && state.context.column > 0));
        state.force = false;
    }
    state.cursor = data->CursorPos;
    state.textLength = data->BufTextLen;
    return 0;
}
}

SECodeContext SEReadCodeContext(std::string_view text, int cursor) {
    SECodeContext context;
    if (cursor < 0 || (size_t)cursor > text.size()) return context;
    const size_t prior = cursor ? text.rfind('\n', cursor - 1) : std::string_view::npos;
    const size_t lineStart = prior == std::string_view::npos ? 0 : prior + 1;
    size_t lineEnd = text.find_first_of("\r\n", cursor);
    if (lineEnd == std::string_view::npos) lineEnd = text.size();
    context.lineStart = (int)lineStart;
    context.lineNumber = 1 + (int)std::count(text.begin(), text.begin() + lineStart, '\n');
    size_t commandStart = lineStart;
    while (commandStart < lineEnd && (text[commandStart] == ' ' || text[commandStart] == '\t')) ++commandStart;
    if ((size_t)cursor < commandStart || (commandStart < lineEnd &&
        (text[commandStart] == '/' || text[commandStart] == '$'))) return context;
    size_t commandEnd = text.find(',', commandStart);
    if (commandEnd == std::string_view::npos || commandEnd > lineEnd) commandEnd = lineEnd;
    size_t trimmedCommandEnd = commandEnd;
    while (trimmedCommandEnd > commandStart && (text[trimmedCommandEnd - 1] == ' ' || text[trimmedCommandEnd - 1] == '\t')) --trimmedCommandEnd;
    context.command = Fold(std::string(text.substr(commandStart, trimmedCommandEnd - commandStart)));
    if (context.command.empty() || context.command[0] != '#') context.command.insert(0, "#");
    context.known = GetCommandHelp(context.command.c_str(), 0).isSame(context.command.c_str());
    size_t start = commandStart, end = commandEnd;
    while (end < (size_t)cursor && end < lineEnd) {
        ++context.column;
        start = end + 1;
        end = text.find(',', start);
        if (end == std::string_view::npos || end > lineEnd) end = lineEnd;
    }
    while (start < end && (text[start] == ' ' || text[start] == '\t')) ++start;
    while (end > start && (text[end - 1] == ' ' || text[end - 1] == '\t')) --end;
    if ((size_t)cursor < start || context.column >= 30) return context;
    context.start = (int)start;
    context.end = (int)end;
    context.query = std::string(text.substr(start, (std::min)((size_t)cursor, end) - start));
    if (context.column == 0) {
        for (unsigned char c : context.query)
            if (!(std::isalnum(c) || c == '#' || c == '_')) return context;
    } else if (!context.known) return context;
    context.valid = true;
    if (context.known) {
        CSTR field = GetCommandHelp(context.command.c_str(), context.column);
        context.field = field.body ? Cp932ToUtf8(field.body) : "";
        context.signature = Signature(context.command, context.column);
    }
    return context;
}

std::vector<SECodeSuggestion> SEFindCodeSuggestions(const SECodeContext& context) {
    std::vector<std::pair<int, SECodeSuggestion>> ranked;
    if (!context.valid) return {};
    if (context.column == 0) {
        std::string query = context.query;
        if (!query.empty() && query[0] == '#') query.erase(0, 1);
        for (const std::string& command : GetCommandNames()) {
            const int rank = MatchRank(command.substr(1), query);
            if (rank < 2) ranked.push_back({rank, {command, command, Signature(command)}});
        }
    } else {
        CSTR field = GetCommandHelp(context.command.c_str(), context.column);
        const auto kind = GetCommandValueKind(context.command.c_str(), field.body);
        for (int i = 0; i < GetCommandValueItemCount(kind); ++i) {
            const int value = GetCommandValueAt(kind, i);
            const char* name = GetCommandValueName(kind, value);
            if (!name || !*name || !strcmp(name, "?")) continue;
            const std::string token = value < 0 ? "!" + std::to_string(-value) : std::to_string(value);
            const std::string display = (value < 0 ? "NOT " : "") + Cp932ToUtf8(name);
            const int rank = (std::min)(MatchRank(token, context.query), MatchRank(display, context.query));
            if (rank < 2) ranked.push_back({rank, {token, token + "  " + display, context.field + ": " + display}});
        }
    }
    std::stable_sort(ranked.begin(), ranked.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    std::vector<SECodeSuggestion> result;
    for (auto& item : ranked) {
        if (result.size() == 64) break;
        if (std::none_of(result.begin(), result.end(), [&](const auto& old) { return old.text == item.second.text; }))
            result.push_back(std::move(item.second));
    }
    return result;
}

bool SEApplyCodeSuggestion(ImGuiInputTextCallbackData& data,
    const SECodeContext& context, const std::string& replacement) {
    if (!context.valid || context.start < 0 || context.end < context.start ||
        context.end > data.BufTextLen) return false;
    std::string text = replacement;
    int next = context.end;
    while (next < data.BufTextLen && (data.Buf[next] == ' ' || data.Buf[next] == '\t')) ++next;
    if (context.column == 0 && (next == data.BufTextLen || data.Buf[next] != ',') &&
        Signature(replacement).find(',') != std::string::npos) text += ',';
    if ((size_t)(data.BufTextLen - (context.end - context.start)) + text.size() >= (size_t)data.BufSize)
        return false;
    // One atomic callback edit lets InputText reconcile a single native Undo.
    memmove(data.Buf + context.start + text.size(), data.Buf + context.end,
        (size_t)(data.BufTextLen - context.end) + 1);
    memcpy(data.Buf + context.start, text.data(), text.size());
    data.BufTextLen += (int)text.size() - (context.end - context.start);
    data.BufDirty = true;
    data.CursorPos = data.SelectionStart = data.SelectionEnd = context.start + (int)text.size();
    return true;
}

bool SEDrawCodeEditorInput(const char* label, char* buffer, size_t capacity, ImVec2 size, SECodeAssistState& state, int workspaceNumber) {
    const ImGuiID inputId = ImGui::GetID(label);
    const ImGuiID assistId = ImGui::GetID("##CodeSuggestionKeys");
    char title[128];
    snprintf(title, sizeof(title), "%s##code-suggestions-%d", SEUISurfaceSpecFor(SEUISurfaceId::CodeSuggestions).title, workspaceNumber);
    const ImGuiWindow* popup = ImGui::FindWindowByName(title);
    const bool overSuggestions = popup && popup->WasActive && popup->Rect().Contains(ImGui::GetIO().MousePos);
    bool active = ImGui::GetActiveID() == inputId;
    if (ImGui::SmallButton("Suggest")) { state.force = state.focus = true; }
    ImGui::SameLine();
    ImGui::TextDisabled("Ctrl+Space | Tab/Enter: insert | Esc: close");
    ImGuiIO& io = ImGui::GetIO();
    if (active && io.KeyCtrl && !io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_Space)) state.force = true;
    if (active && state.open && !io.KeyCtrl && !io.KeyAlt && !io.KeyShift && !io.KeySuper) {
        for (ImGuiKey key : {ImGuiKey_Tab, ImGuiKey_Enter, ImGuiKey_KeypadEnter, ImGuiKey_Escape}) {
            if (!ImGui::IsKeyPressed(key, false)) continue;
            // InputText routes these keys with its own ID. Claim them first so
            // completion cannot insert a newline or revert the entire draft.
            ImGui::SetKeyOwner(key, assistId, ImGuiInputFlags_LockThisFrame);
            if (key == ImGuiKey_Escape) { state.open = false; state.dismissed = true; }
            else state.accept = true;
        }
        const int direction = ImGui::IsKeyPressed(ImGuiKey_DownArrow) ? 1 :
            ImGui::IsKeyPressed(ImGuiKey_UpArrow) ? -1 : 0;
        if (direction && !state.suggestions.empty()) {
            state.selected = (state.selected + direction + (int)state.suggestions.size()) % (int)state.suggestions.size();
            state.scrollSelection = true;
            if (auto* input = ImGui::GetInputTextState(inputId)) {
                state.restoreCursor = true;
                state.savedCursor = input->GetCursorPos();
                state.savedStart = input->GetSelectionStart();
                state.savedEnd = input->GetSelectionEnd();
            }
        }
    }
    if (state.focus) { ImGui::SetKeyboardFocusHere(); state.focus = false; }
    const float helpHeight = ImGui::GetTextLineHeightWithSpacing() * 3.4f;
    const bool changed = ImGui::InputTextMultiline(label, buffer, capacity,
        ImVec2(size.x, (std::max)(80.0f, size.y - helpHeight - ImGui::GetFrameHeightWithSpacing())),
        ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_CallbackEdit,
        AssistCallback, &state);
    active = ImGui::IsItemActive();
    const ImVec2 inputMin = ImGui::GetItemRectMin(), inputMax = ImGui::GetItemRectMax();
    if (!active && !state.accept && !overSuggestions) state.open = false;

    if (ImGui::BeginChild("##CodeSignature", ImVec2(0, helpHeight), ImGuiChildFlags_Borders)) {
        if (state.context.known) {
            ImGui::TextColored(SEUI::Colors::Accent(), "Ln %d | %s | %s %d: %s",
                state.context.lineNumber, state.context.command.c_str(), "Argument",
                state.context.column, state.context.field.c_str());
            ImGui::TextWrapped("%s", state.context.signature.c_str());
        } else ImGui::TextWrapped("Type #SRC_, #DST_ or part of a command name. Suggestions and argument hints come from the LR2 command schema.");
    }
    ImGui::EndChild();

    if ((active || overSuggestions || state.accept) && state.open && !state.suggestions.empty()) {
        auto* input = ImGui::GetInputTextState(inputId);
        const int cursor = (std::clamp)(state.cursor, state.context.lineStart, (int)strlen(buffer));
        const ImVec2 padding = ImGui::GetStyle().FramePadding;
        ImVec2 position(inputMin.x + padding.x, inputMin.y + padding.y);
        position.x += ImGui::CalcTextSize(buffer + state.context.lineStart, buffer + cursor).x - (input ? input->Scroll.x : 0);
        position.y += state.context.lineNumber * ImGui::GetTextLineHeight() - (input ? input->Scroll.y : 0);
        if (position.y < inputMin.y || position.y > inputMax.y + ImGui::GetTextLineHeight()) return changed;
        const ImGuiViewport* viewport = ImGui::GetWindowViewport();
        const float width = (std::min)(520.0f, viewport->WorkSize.x - 16.0f);
        const float height = ImGui::GetTextLineHeightWithSpacing() * 11 + 28;
        position.x = (std::clamp)(position.x, viewport->WorkPos.x + 4, viewport->WorkPos.x + viewport->WorkSize.x - width - 4);
        if (position.y + height > viewport->WorkPos.y + viewport->WorkSize.y)
            position.y -= height + ImGui::GetTextLineHeight();
        position.y = (std::max)(viewport->WorkPos.y + 4, position.y);
        ImGui::SetNextWindowPos(position);
        ImGui::SetNextWindowSize(ImVec2(width, height));
        if (ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNavFocus)) {
            ImGui::TextDisabled("%d suggestions  |  Up / Down", (int)state.suggestions.size());
            if (ImGui::BeginChild("##Suggestions", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 7))) {
                for (int i = 0; i < (int)state.suggestions.size(); ++i) {
                    ImGui::PushID(i);
                    if (ImGui::Selectable(state.suggestions[i].label.c_str(), i == state.selected)) {
                        state.selected = i;
                        state.accept = state.focus = true;
                    }
                    if (i == state.selected && state.scrollSelection) ImGui::SetScrollHereY();
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", state.suggestions[i].detail.c_str());
                    ImGui::PopID();
                }
            }
            ImGui::EndChild();
            state.scrollSelection = false;
            ImGui::Separator();
            ImGui::TextWrapped("%s", state.suggestions[state.selected].detail.c_str());
        }
        ImGui::End();
    }
    return changed;
}
