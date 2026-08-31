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

// Produces a main script whose #INFORMATION fields 6/7 and single active
// #RESOLUTION row carry the same canvas. Existing active or previously inert
// rows are reused without changing row addresses; when neither exists, a new
// row is inserted immediately after #INFORMATION. insertedRow receives that
// one-based row address, or 0 when no row was inserted.
bool SEPrepareLr2ExportResolution(const std::string& script,
    int width, int height, std::string& preparedScript,
    std::string& errorMessage, int* insertedRow = nullptr);

const char* SESkinResolutionSourceText(SESkinResolutionSource source);
bool SEIsInferredSkinResolution(SESkinResolutionSource source);
