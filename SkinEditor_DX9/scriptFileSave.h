#pragma once
#include <string>

// Existing external asset (byte strings, including DXA). Reuses the script transaction's backup,
// verification and rollback rules; refuses stale drafts and leftover backups.
bool SESaveExternalTextFile(const std::string& path, const std::string& original,
    const std::string& edited, std::string& report);
