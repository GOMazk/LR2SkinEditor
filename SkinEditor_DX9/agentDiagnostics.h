#pragma once
#include <string>

struct WORKSPACE;

// Read-only explanation of the currently loaded native preview state.
// Returns a UTF-8 JSON object; the CLI owns the enclosing response/report.
std::string SEAgentDiagnostics(WORKSPACE& workspace);
