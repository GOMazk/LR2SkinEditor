#pragma once

#include "imgui/imgui.h"

// Shared presentation layer for SkinEditor.
// Keep editor/domain state out of this module: components return user intent
// and the owning WORKSPACE applies it. This makes UI changes low-risk and
// keeps new tools visually consistent.
namespace SEUI {
    struct Colors {
        static ImVec4 Accent();
        static ImVec4 Success();
        static ImVec4 Warning();
        static ImVec4 Danger();
        static ImVec4 Muted();
    };

    void ApplyModernTheme(float dpiScale);

    bool BeginToolbar(const char* id);
    void EndToolbar();
    bool ActionButton(const char* label, const char* tooltip = nullptr, bool enabled = true);
    void ToolbarSeparator();

    void SectionHeader(const char* title, const char* description = nullptr);
    void StatusPill(const char* label, const ImVec4& color);
    void EmptyState(const char* title, const char* description);
    void HelpMarker(const char* description);

    float ToolbarHeight();
    float FooterHeight();
}
