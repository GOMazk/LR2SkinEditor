#pragma once
#include <string>
#include <vector>

struct WORKSPACE;

// Read-only explanation of the currently loaded native preview state.
// Returns a UTF-8 JSON object; the CLI owns the enclosing response/report.
std::string SEAgentDiagnostics(WORKSPACE& workspace);

struct SEPreviewVisibilityIssue {
    int row = -1;
    std::string code, hint;
    bool hidden = false;
};

struct SEObjectPreviewVisibility {
    std::string status = "unknown";
    int timelines = 0;
    int hiddenTimelines = 0;
    std::vector<SEPreviewVisibilityIssue> issues;
};

// On-demand single-Object projection for UI tooltips. Uses the same native
// visibility analysis as the CLI, without its whole-skin resource scan.
SEObjectPreviewVisibility SEObjectPreviewDiagnostics(WORKSPACE& workspace, int modelIndex);
