#include "skinResolution.h"

#include "../LR2/En_fileutil.h"

#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unordered_map>

namespace {

constexpr int kMinimumWidth = 64;
constexpr int kMinimumHeight = 64;
constexpr int kMaximumWidth = 7680;
constexpr int kMaximumHeight = 4320;

// Automatic resolution detection below is adapted from TenRiff's
// src/app/Lr2Skin.cpp (MIT License, Copyright (c) 2026 TenRiff contributors;
// see docs/THIRD_PARTY_NOTICES.md).
// TenRiff deliberately recognises only LR2's SD, HD and FHD authoring families.
constexpr int kCanvasOriginTolerance = 4;
constexpr int kMinimumCanvasWidth = 400;
constexpr int kMinimumCanvasHeight = 300;

enum class TenRiffResolutionFamily {
    Sd,
    Hd,
    Fhd,
};

struct TenRiffResolutionEvidence {
    std::unordered_map<int, int> visibleLaneX;
    int canvasWidth = 0;
    int canvasHeight = 0;
    int canvasRowCount = 0;
};

bool IsValidResolution(int width, int height) {
    return width >= kMinimumWidth && width <= kMaximumWidth &&
        height >= kMinimumHeight && height <= kMaximumHeight;
}

bool IsCommand(CSVbuf& csv, const char* command) {
    return csv.str[0].body && _stricmp(csv.str[0].outstr(), command) == 0;
}

bool TryReadInformationResolution(CSVbuf& csv, int& width, int& height) {
    if (!IsCommand(csv, "#INFORMATION")) return false;
    if (!IsValidResolution(csv.val[6], csv.val[7])) return false;
    width = csv.val[6];
    height = csv.val[7];
    return true;
}

bool TryReadResolutionCommand(CSVbuf& csv, int& width, int& height) {
    if (!IsCommand(csv, "#RESOLUTION")) return false;

    if (IsValidResolution(csv.val[1], csv.val[2])) {
        width = csv.val[1];
        height = csv.val[2];
        return true;
    }

    // Preserve the editor's existing explicit-header contract. TenRiff's
    // algorithm is used only when neither explicit form supplies a canvas.
    if (csv.val[2] != 0) return false;
    switch (csv.val[1]) {
    case 0: width = 640; height = 480; return true;
    case 1: width = 1280; height = 720; return true;
    case 2: width = 1920; height = 1080; return true;
    case 3: width = 3840; height = 2160; return true;
    default: return false;
    }
}

int PositiveMagnitude(int value) {
    if (value == INT_MIN) return INT_MAX;
    return value < 0 ? -value : value;
}

void CollectTenRiffEvidence(CSVbuf& csv,
    TenRiffResolutionEvidence& evidence) {
    if (IsCommand(csv, "#DST_NOTE")) {
        if (csv.str[1].length() == 0 || csv.str[3].length() == 0 ||
            csv.str[4].length() == 0 || csv.str[5].length() == 0 ||
            csv.str[6].length() == 0)
            return;
        const int lane = csv.val[1];
        const int alpha = csv.str[8].length() > 0 ? csv.val[8] : 255;
        if (alpha <= 0) {
            evidence.visibleLaneX.erase(lane);
        }
        else {
            // TenRiff keeps the final visible destination for each lane.
            evidence.visibleLaneX[lane] = csv.val[3];
        }
        return;
    }

    if (!IsCommand(csv, "#DST_IMAGE")) return;
    if (csv.str[1].length() == 0 || csv.str[3].length() == 0 ||
        csv.str[4].length() == 0 || csv.str[5].length() == 0 ||
        csv.str[6].length() == 0)
        return;
    const int x = csv.val[3];
    const int y = csv.val[4];
    const int width = PositiveMagnitude(csv.val[5]);
    const int height = PositiveMagnitude(csv.val[6]);

    // A full-canvas backdrop must be anchored at the top-left. Large panels
    // parked off-screen for animation are intentionally ignored.
    if (PositiveMagnitude(x) > kCanvasOriginTolerance ||
        PositiveMagnitude(y) > kCanvasOriginTolerance ||
        width < kMinimumCanvasWidth || height < kMinimumCanvasHeight)
        return;

    evidence.canvasWidth = (std::max)(evidence.canvasWidth, width);
    evidence.canvasHeight = (std::max)(evidence.canvasHeight, height);
    ++evidence.canvasRowCount;
}

TenRiffResolutionFamily DetectLaneFamily(
    const TenRiffResolutionEvidence& evidence) {
    int maxX = 0;
    for (const auto& lane : evidence.visibleLaneX)
        maxX = (std::max)(maxX, lane.second);
    if (maxX <= 960) return TenRiffResolutionFamily::Sd;
    if (maxX <= 1600) return TenRiffResolutionFamily::Hd;
    return TenRiffResolutionFamily::Fhd;
}

TenRiffResolutionFamily DetectCanvasFamily(int width, int height) {
    const auto axisFamily = [](int value, int sdToHd, int hdToFhd) {
        if (value <= sdToHd) return TenRiffResolutionFamily::Sd;
        return value <= hdToFhd
            ? TenRiffResolutionFamily::Hd
            : TenRiffResolutionFamily::Fhd;
    };
    return (std::max)(axisFamily(width, 960, 1600),
        axisFamily(height, 600, 900));
}

TenRiffResolutionFamily DetectAutomaticFamily(
    const TenRiffResolutionEvidence& evidence) {
    const TenRiffResolutionFamily laneFamily = DetectLaneFamily(evidence);
    if (evidence.canvasWidth <= 0 || evidence.canvasHeight <= 0)
        return laneFamily;
    return (std::max)(laneFamily,
        DetectCanvasFamily(evidence.canvasWidth, evidence.canvasHeight));
}

SESkinResolutionDecision DecisionForFamily(TenRiffResolutionFamily family,
    int evidenceCount) {
    SESkinResolutionDecision decision;
    switch (family) {
    case TenRiffResolutionFamily::Hd:
        decision.width = 1280;
        decision.height = 720;
        break;
    case TenRiffResolutionFamily::Fhd:
        decision.width = 1920;
        decision.height = 1080;
        break;
    default:
        decision.width = 640;
        decision.height = 480;
        break;
    }
    decision.source = SESkinResolutionSource::TenRiffAuto;
    decision.destinationEvidenceCount = evidenceCount;
    return decision;
}

}

SESkinResolutionDecision SEResolveSkinResolution(
    const std::vector<std::string>& csvLines) {
    int informationWidth = 0;
    int informationHeight = 0;
    int resolutionCommandWidth = 0;
    int resolutionCommandHeight = 0;
    TenRiffResolutionEvidence evidence;

    for (const std::string& line : csvLines) {
        if (line.empty() || line[0] != '#') continue;
        CSTR csvText(line.c_str());
        CSVbuf csv = {};
        SplitCSV(csvText, &csv, ",");

        int declaredWidth = 0;
        int declaredHeight = 0;
        if (informationWidth == 0 &&
            TryReadInformationResolution(csv, declaredWidth, declaredHeight)) {
            informationWidth = declaredWidth;
            informationHeight = declaredHeight;
        }
        if (resolutionCommandWidth == 0 &&
            TryReadResolutionCommand(csv, declaredWidth, declaredHeight)) {
            resolutionCommandWidth = declaredWidth;
            resolutionCommandHeight = declaredHeight;
        }
        CollectTenRiffEvidence(csv, evidence);
    }

    if (informationWidth > 0) {
        return { informationWidth, informationHeight,
            SESkinResolutionSource::Information, 0 };
    }
    if (resolutionCommandWidth > 0) {
        return { resolutionCommandWidth, resolutionCommandHeight,
            SESkinResolutionSource::ResolutionCommand, 0 };
    }

    const int evidenceCount = (int)evidence.visibleLaneX.size() +
        evidence.canvasRowCount;
    if (evidenceCount == 0) return {};
    return DecisionForFamily(DetectAutomaticFamily(evidence), evidenceCount);
}

bool SEResolveSkinResolutionFile(const char* path,
    SESkinResolutionDecision& decision) {
    if (!path || !*path) return false;
    FILE* file = fopen(path, "rb");
    if (!file) return false;

    std::vector<std::string> csvLines;
    char line[4096] = {};
    while (fgets(line, sizeof(line), file))
        csvLines.emplace_back(line);
    fclose(file);

    decision = SEResolveSkinResolution(csvLines);
    return true;
}

const char* SESkinResolutionSourceText(SESkinResolutionSource source) {
    switch (source) {
    case SESkinResolutionSource::Information: return "#INFORMATION";
    case SESkinResolutionSource::ResolutionCommand: return "#RESOLUTION";
    case SESkinResolutionSource::TenRiffAuto: return "TenRiff auto inference";
    default: return "640 x 480 fallback";
    }
}

bool SEIsInferredSkinResolution(SESkinResolutionSource source) {
    return source == SESkinResolutionSource::TenRiffAuto;
}
