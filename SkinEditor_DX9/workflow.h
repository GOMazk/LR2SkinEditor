#pragma once
#include <memory>
#include <string>
#include <vector>

struct WORKSPACE;
enum class SEPendingKind { Script, TextDraft, CustomFilesDraft, Image, Font };
// A transient projection, never another editable document or saved dirty flag.
struct SEPendingWork {
    SEPendingKind kind;
    std::string path;
    bool IsDraft() const {
        return kind == SEPendingKind::TextDraft || kind == SEPendingKind::CustomFilesDraft;
    }
};
bool SECanExitWorkspaces(const std::vector<std::unique_ptr<WORKSPACE>>& workspaces);
bool SEDrawPendingChanges(std::vector<std::unique_ptr<WORKSPACE>>& workspaces, bool& exitRequested);
int RunWorkflowSelfTest();
