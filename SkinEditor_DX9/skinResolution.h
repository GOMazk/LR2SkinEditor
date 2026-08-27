#pragma once

#include <string>
#include <vector>

enum class SESkinResolutionSource {
    Information,
    ResolutionCommand,
    DestinationBounds,
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
// destination geometry from the fully expanded script is used as evidence.
SESkinResolutionDecision SEResolveSkinResolution(
    const std::vector<std::string>& csvLines);

const char* SESkinResolutionSourceText(SESkinResolutionSource source);
bool SEIsInferredSkinResolution(SESkinResolutionSource source);
