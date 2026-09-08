#include "seUI.h"

#include <algorithm>

namespace {
    ImVec4 WithAlpha(const ImVec4& color, float alpha) {
        return ImVec4(color.x, color.y, color.z, alpha);
    }
}

namespace SEUI {
    ImVec4 Colors::Accent() { return ImVec4(0.31f, 0.62f, 0.98f, 1.00f); }
    ImVec4 Colors::Success() { return ImVec4(0.24f, 0.78f, 0.55f, 1.00f); }
    ImVec4 Colors::Warning() { return ImVec4(0.96f, 0.70f, 0.28f, 1.00f); }
    ImVec4 Colors::Danger() { return ImVec4(0.95f, 0.36f, 0.42f, 1.00f); }
    ImVec4 Colors::Muted() { return ImVec4(0.55f, 0.61f, 0.70f, 1.00f); }

    void ApplyModernTheme(float dpiScale) {
        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();

        style.WindowPadding = ImVec2(10.0f, 8.0f);
        style.FramePadding = ImVec2(8.0f, 4.0f);
        style.CellPadding = ImVec2(7.0f, 4.0f);
        style.ItemSpacing = ImVec2(7.0f, 5.0f);
        style.ItemInnerSpacing = ImVec2(5.0f, 4.0f);
        style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
        style.IndentSpacing = 16.0f;
        style.ScrollbarSize = 10.0f;
        style.GrabMinSize = 9.0f;

        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.TabBorderSize = 0.0f;

        style.WindowRounding = 6.0f;
        style.ChildRounding = 5.0f;
        style.FrameRounding = 4.0f;
        style.PopupRounding = 6.0f;
        style.ScrollbarRounding = 6.0f;
        style.GrabRounding = 4.0f;
        style.TabRounding = 4.0f;

        const ImVec4 accent = Colors::Accent();
        ImVec4* c = style.Colors;
        c[ImGuiCol_Text] = ImVec4(0.91f, 0.93f, 0.96f, 1.00f);
        c[ImGuiCol_TextDisabled] = ImVec4(0.47f, 0.52f, 0.61f, 1.00f);
        c[ImGuiCol_WindowBg] = ImVec4(0.055f, 0.067f, 0.090f, 1.00f);
        c[ImGuiCol_ChildBg] = ImVec4(0.066f, 0.080f, 0.106f, 1.00f);
        c[ImGuiCol_PopupBg] = ImVec4(0.075f, 0.090f, 0.120f, 0.99f);
        c[ImGuiCol_Border] = ImVec4(0.16f, 0.19f, 0.25f, 1.00f);
        c[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
        c[ImGuiCol_FrameBg] = ImVec4(0.105f, 0.125f, 0.165f, 1.00f);
        c[ImGuiCol_FrameBgHovered] = ImVec4(0.145f, 0.175f, 0.225f, 1.00f);
        c[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.22f, 0.29f, 1.00f);
        c[ImGuiCol_TitleBg] = ImVec4(0.060f, 0.073f, 0.098f, 1.00f);
        c[ImGuiCol_TitleBgActive] = ImVec4(0.080f, 0.098f, 0.132f, 1.00f);
        c[ImGuiCol_TitleBgCollapsed] = c[ImGuiCol_TitleBg];
        c[ImGuiCol_MenuBarBg] = ImVec4(0.045f, 0.056f, 0.076f, 1.00f);
        c[ImGuiCol_ScrollbarBg] = ImVec4(0.045f, 0.055f, 0.075f, 0.85f);
        c[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.24f, 0.31f, 1.00f);
        c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.34f, 0.43f, 1.00f);
        c[ImGuiCol_ScrollbarGrabActive] = accent;
        c[ImGuiCol_CheckMark] = accent;
        c[ImGuiCol_SliderGrab] = WithAlpha(accent, 0.82f);
        c[ImGuiCol_SliderGrabActive] = accent;
        c[ImGuiCol_Button] = ImVec4(0.115f, 0.140f, 0.185f, 1.00f);
        c[ImGuiCol_ButtonHovered] = ImVec4(0.16f, 0.21f, 0.29f, 1.00f);
        c[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.29f, 0.42f, 1.00f);
        c[ImGuiCol_Header] = WithAlpha(accent, 0.25f);
        c[ImGuiCol_HeaderHovered] = WithAlpha(accent, 0.38f);
        c[ImGuiCol_HeaderActive] = WithAlpha(accent, 0.50f);
        c[ImGuiCol_Separator] = ImVec4(0.16f, 0.19f, 0.25f, 1.00f);
        c[ImGuiCol_SeparatorHovered] = WithAlpha(accent, 0.70f);
        c[ImGuiCol_SeparatorActive] = accent;
        c[ImGuiCol_ResizeGrip] = WithAlpha(accent, 0.20f);
        c[ImGuiCol_ResizeGripHovered] = WithAlpha(accent, 0.55f);
        c[ImGuiCol_ResizeGripActive] = accent;
        c[ImGuiCol_Tab] = ImVec4(0.075f, 0.090f, 0.120f, 1.00f);
        c[ImGuiCol_TabHovered] = WithAlpha(accent, 0.55f);
        c[ImGuiCol_TabSelected] = ImVec4(0.13f, 0.19f, 0.28f, 1.00f);
        c[ImGuiCol_TabSelectedOverline] = accent;
        c[ImGuiCol_TabDimmed] = ImVec4(0.060f, 0.070f, 0.095f, 1.00f);
        c[ImGuiCol_TabDimmedSelected] = ImVec4(0.095f, 0.12f, 0.17f, 1.00f);
        c[ImGuiCol_DockingPreview] = WithAlpha(accent, 0.70f);
        c[ImGuiCol_DockingEmptyBg] = ImVec4(0.045f, 0.055f, 0.075f, 1.00f);
        c[ImGuiCol_TableHeaderBg] = ImVec4(0.095f, 0.115f, 0.155f, 1.00f);
        c[ImGuiCol_TableBorderStrong] = ImVec4(0.18f, 0.21f, 0.28f, 1.00f);
        c[ImGuiCol_TableBorderLight] = ImVec4(0.13f, 0.16f, 0.21f, 1.00f);
        c[ImGuiCol_TableRowBgAlt] = ImVec4(1, 1, 1, 0.018f);
        c[ImGuiCol_TextSelectedBg] = WithAlpha(accent, 0.35f);
        c[ImGuiCol_NavCursor] = accent;
        c[ImGuiCol_ModalWindowDimBg] = ImVec4(0.01f, 0.015f, 0.025f, 0.72f);

        style.ScaleAllSizes((std::max)(0.75f, dpiScale));
    }

    float ToolbarHeight() { return ImGui::GetFrameHeight() + ImGui::GetStyle().WindowPadding.y * 1.35f; }
    float FooterHeight() { return ImGui::GetFrameHeight() + 6.0f; }

    bool BeginToolbar(const char* id) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.070f, 0.085f, 0.115f, 1.00f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 5.0f));
        return ImGui::BeginChild(id, ImVec2(0.0f, ToolbarHeight()), ImGuiChildFlags_Borders,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    }

    void EndToolbar() {
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }

    bool BeginStatusBar(const char* id) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.050f, 0.062f, 0.084f, 1.00f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 3.0f));
        return ImGui::BeginChild(id, ImVec2(0.0f, FooterHeight()), ImGuiChildFlags_Borders,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    }

    void EndStatusBar() {
        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }

    bool ActionButton(const char* label, const char* tooltip, bool enabled) {
        if (!enabled) ImGui::BeginDisabled();
        const bool clicked = ImGui::Button(label);
        if (!enabled) ImGui::EndDisabled();
        if (tooltip && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal | ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("%s", tooltip);
        return clicked && enabled;
    }

    void ToolbarSeparator() {
        ImGui::SameLine();
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::GetWindowDrawList()->AddLine(
            ImVec2(pos.x, pos.y + 3.0f), ImVec2(pos.x, pos.y + ImGui::GetFrameHeight() - 3.0f),
            ImGui::GetColorU32(ImGuiCol_Separator));
        ImGui::Dummy(ImVec2(8.0f, ImGui::GetFrameHeight()));
        ImGui::SameLine();
    }

    void SectionHeader(const char* title, const char* description) {
        ImGui::TextUnformatted(title);
        if (description) {
            ImGui::SameLine();
            ImGui::TextDisabled("%s", description);
        }
        ImGui::Separator();
    }

    void StatusPill(const char* label, const ImVec4& color) {
        const ImVec2 text = ImGui::CalcTextSize(label);
        const ImVec2 padding(7.0f, 3.0f);
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const ImVec2 size(text.x + padding.x * 2.0f, text.y + padding.y * 2.0f);
        ImGui::PushID("##StatusPill");
        ImGui::InvisibleButton(label, size);
        ImGui::PopID();
        ImGui::GetWindowDrawList()->AddRectFilled(pos,
            ImVec2(pos.x + size.x, pos.y + size.y),
            ImGui::GetColorU32(WithAlpha(color, 0.18f)), 10.0f);
        ImGui::GetWindowDrawList()->AddRect(pos,
            ImVec2(pos.x + size.x, pos.y + size.y),
            ImGui::GetColorU32(WithAlpha(color, 0.45f)), 10.0f);
        ImGui::GetWindowDrawList()->AddText(ImVec2(pos.x + padding.x, pos.y + padding.y),
            ImGui::GetColorU32(color), label);
    }

    void EmptyState(const char* title, const char* description) {
        const ImVec2 available = ImGui::GetContentRegionAvail();
        const float cardWidth = (std::min)(520.0f, (std::max)(280.0f, available.x - 40.0f));
        const float cardHeight = 150.0f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (std::max)(0.0f, (available.x - cardWidth) * 0.5f));
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (std::max)(12.0f, (available.y - cardHeight) * 0.35f));
        ImGui::BeginChild("##EmptyState", ImVec2(cardWidth, cardHeight), ImGuiChildFlags_Borders);
        ImGui::Dummy(ImVec2(0.0f, 18.0f));
        const float titleWidth = ImGui::CalcTextSize(title).x;
        ImGui::SetCursorPosX((cardWidth - titleWidth) * 0.5f);
        ImGui::TextUnformatted(title);
        ImGui::Dummy(ImVec2(0.0f, 6.0f));
        ImGui::PushTextWrapPos(cardWidth - 30.0f);
        const float descWidth = ImGui::CalcTextSize(description, nullptr, false, cardWidth - 40.0f).x;
        ImGui::SetCursorPosX((std::max)(15.0f, (cardWidth - descWidth) * 0.5f));
        ImGui::TextDisabled("%s", description);
        ImGui::PopTextWrapPos();
        ImGui::EndChild();
    }

    void HelpMarker(const char* description) {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 32.0f);
            ImGui::TextUnformatted(description);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
}
