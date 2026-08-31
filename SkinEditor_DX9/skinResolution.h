#pragma once

#include <string>
#include <vector>

enum class SESkinResolutionSource {
    Information,
    ResolutionCommand,
    TenRiffAuto,
    Default640x480,
};

struct SESkinResolutionDecision {
    int width = 640;
    int height = 480;
    SESkinResolutionSource source = SESkinResolutionSource::Default640x480;
    int destinationEvidenceCount = 0;
};

// Resolves the canvas size without mutating the skin document. Explicit
// #INFORMATION dimensions win, followed by #RESOLUTION. When both are absent,
// TenRiff's LR2 lane/backdrop heuristic resolves the SD, HD or FHD family from
// the fully expanded script without changing the shared LR2 parser.
SESkinResolutionDecision SEResolveSkinResolution(
    const std::vector<std::string>& csvLines);

// Reads one script file for browser metadata. Workspace loading still passes
// its already-expanded include tree to SEResolveSkinResolution().
bool SEResolveSkinResolutionFile(const char* path,
    SESkinResolutionDecision& decision);

// Produces an LR2-safe main script without changing row addresses. LR2 reads
// the target canvas from #INFORMATION fields 6 and 7; some LR2 builds corrupt
// the next skin-list slot when a separate #RESOLUTION row is present. This
// function therefore stores the requested canvas in #INFORMATION and turns
// active #RESOLUTION rows into inert OLR compatibility comments.
bool SEPrepareLr2ExportResolution(const std::string& script,
    int width, int height, std::string& preparedScript,
    std::string& errorMessage);

const char* SESkinResolutionSourceText(SESkinResolutionSource source);
bool SEIsInferredSkinResolution(SESkinResolutionSource source);
