#include "skinResolution.h"

#include "../LR2/En_fileutil.h"
#include "seHelper.h"

#include <algorithm>
#include <cctype>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <unordered_map>
#include <vector>

namespace {

constexpr int kMinimumWidth = 64;
constexpr int kMinimumHeight = 64;
constexpr int kMaximumWidth = 7680;
constexpr int kMaximumHeight = 4320;
constexpr int kFallbackWidth = 640;
constexpr int kFallbackHeight = 480;

struct ResolutionPreset {
    int width;
    int height;
};

constexpr ResolutionPreset kResolutionPresets[] = {
    { 640, 480 },
    { 800, 600 },
    { 960, 540 },
    { 1024, 576 },
    { 1024, 768 },
    { 1280, 720 },
    { 1366, 768 },
    { 1280, 960 },
    { 1600, 900 },
    { 1920, 1080 },
    { 2560, 1440 },
    { 3840, 2160 },
    { 7680, 4320 },
};

struct DestinationExtent {
    int right;
    int bottom;
    int x;
    int y;
    int width;
    int height;
};

struct DestinationSchema {
    int xColumn = -1;
    int yColumn = -1;
    int widthColumn = -1;
    int heightColumn = -1;
    int sizeColumn = -1;
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

    // Older LR2 skins may use a preset id instead of explicit dimensions.
    if (csv.val[2] != 0) return false;
    switch (csv.val[1]) {
    case 0: width = 640; height = 480; return true;
    case 1: width = 1280; height = 720; return true;
    case 2: width = 1920; height = 1080; return true;
    case 3: width = 3840; height = 2160; return true;
    default: return false;
    }
}

int FindSchemaColumn(const char* command, const char* fieldName) {
    if (!command || !fieldName) return -1;
    for (int column = 1; column < 30; ++column) {
        CSTR field = GetCommandHelp(command, column);
        field.trimWhiteSpace();
        if (field.body && _stricmp(field.outstr(), fieldName) == 0)
            return column;
    }
    return -1;
}

int PositiveMagnitude(int value) {
    if (value == INT_MIN) return INT_MAX;
    return value < 0 ? -value : value;
}

const DestinationSchema& DestinationSchemaFor(const char* command,
    std::unordered_map<std::string, DestinationSchema>& schemaCache) {
    std::string normalizedCommand = command ? command : "";
    std::transform(normalizedCommand.begin(), normalizedCommand.end(),
        normalizedCommand.begin(), [](unsigned char ch) {
            return (char)std::toupper(ch);
        });

    const auto existing = schemaCache.find(normalizedCommand);
    if (existing != schemaCache.end()) return existing->second;

    DestinationSchema schema;
    schema.xColumn = FindSchemaColumn(normalizedCommand.c_str(), "x");
    schema.yColumn = FindSchemaColumn(normalizedCommand.c_str(), "y");
    schema.widthColumn = FindSchemaColumn(normalizedCommand.c_str(), "w");
    schema.heightColumn = FindSchemaColumn(normalizedCommand.c_str(), "h");
    schema.sizeColumn = FindSchemaColumn(normalizedCommand.c_str(), "size");
    return schemaCache.emplace(normalizedCommand, schema).first->second;
}

bool TryReadDestinationExtent(CSVbuf& csv,
    std::unordered_map<std::string, DestinationSchema>& schemaCache,
    DestinationExtent& extent) {
    const char* command = csv.str[0].body ? csv.str[0].outstr() : "";
    if (_strnicmp(command, "#DST_", 5) != 0) return false;

    const DestinationSchema& schema =
        DestinationSchemaFor(command, schemaCache);
    if (schema.xColumn < 0 || schema.yColumn < 0) return false;

    const int x = csv.val[schema.xColumn];
    const int y = csv.val[schema.yColumn];
    const int width = schema.widthColumn >= 0
        ? PositiveMagnitude(csv.val[schema.widthColumn]) : 0;
    int height = schema.heightColumn >= 0
        ? PositiveMagnitude(csv.val[schema.heightColumn]) : 0;
    if (height == 0 && schema.sizeColumn >= 0)
        height = PositiveMagnitude(csv.val[schema.sizeColumn]);
    if (width == 0 && height == 0) return false;

    const long long right = (long long)x + (std::max)(1, width);
    const long long bottom = (long long)y + (std::max)(1, height);
    if (right <= 0 || bottom <= 0) return false;

    // Coordinates this far outside the editor's supported canvas are usually
    // transition sentinels or malformed rows, not usable resolution evidence.
    if (right > (long long)kMaximumWidth * 2 ||
        bottom > (long long)kMaximumHeight * 2)
        return false;

    extent.right = (int)right;
    extent.bottom = (int)bottom;
    extent.x = x;
    extent.y = y;
    extent.width = width;
    extent.height = height;
    return true;
}

int RobustUpperBound(std::vector<int> values) {
    if (values.empty()) return 0;
    std::sort(values.begin(), values.end());
    if (values.size() < 10) return values.back();

    // The 90th percentile keeps a few off-canvas animation frames from
    // turning an otherwise ordinary skin into a 4K canvas.
    const size_t index = ((values.size() - 1) * 9) / 10;
    return values[index];
}

bool MatchesPreset(const DestinationExtent& extent,
    const ResolutionPreset& preset) {
    return std::abs(extent.x) <= 2 && std::abs(extent.y) <= 2 &&
        std::abs(extent.width - preset.width) <= 2 &&
        std::abs(extent.height - preset.height) <= 2;
}

ResolutionPreset SmallestContainingPreset(int requiredWidth,
    int requiredHeight) {
    ResolutionPreset best = { 0, 0 };
    long long bestArea = (std::numeric_limits<long long>::max)();
    for (const ResolutionPreset& preset : kResolutionPresets) {
        if (preset.width < requiredWidth || preset.height < requiredHeight)
            continue;
        const long long area = (long long)preset.width * preset.height;
        if (area < bestArea) {
            best = preset;
            bestArea = area;
        }
    }
    return best;
}

int RoundUpToEight(int value) {
    return value > INT_MAX - 7 ? INT_MAX : ((value + 7) & ~7);
}

}

SESkinResolutionDecision SEResolveSkinResolution(
    const std::vector<std::string>& csvLines) {
    int informationWidth = 0;
    int informationHeight = 0;
    int resolutionCommandWidth = 0;
    int resolutionCommandHeight = 0;
    std::vector<DestinationExtent> destinationExtents;
    std::unordered_map<std::string, DestinationSchema> destinationSchemas;

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

        DestinationExtent extent = {};
        if (TryReadDestinationExtent(csv, destinationSchemas, extent))
            destinationExtents.push_back(extent);
    }

    if (informationWidth > 0) {
        return { informationWidth, informationHeight,
            SESkinResolutionSource::Information, 0 };
    }
    if (resolutionCommandWidth > 0) {
        return { resolutionCommandWidth, resolutionCommandHeight,
            SESkinResolutionSource::ResolutionCommand, 0 };
    }
    if (destinationExtents.empty()) return {};

    std::vector<int> rightEdges;
    std::vector<int> bottomEdges;
    rightEdges.reserve(destinationExtents.size());
    bottomEdges.reserve(destinationExtents.size());
    int anchoredWidth = 0;
    int anchoredHeight = 0;
    long long anchoredArea = 0;
    for (const DestinationExtent& extent : destinationExtents) {
        rightEdges.push_back(extent.right);
        bottomEdges.push_back(extent.bottom);
        for (const ResolutionPreset& preset : kResolutionPresets) {
            if (!MatchesPreset(extent, preset)) continue;
            const long long area = (long long)preset.width * preset.height;
            if (area > anchoredArea) {
                anchoredWidth = preset.width;
                anchoredHeight = preset.height;
                anchoredArea = area;
            }
        }
    }

    int requiredWidth = (std::max)(kFallbackWidth,
        RobustUpperBound(rightEdges));
    int requiredHeight = (std::max)(kFallbackHeight,
        RobustUpperBound(bottomEdges));
    requiredWidth = (std::max)(requiredWidth, anchoredWidth);
    requiredHeight = (std::max)(requiredHeight, anchoredHeight);

    const ResolutionPreset preset = SmallestContainingPreset(requiredWidth,
        requiredHeight);
    SESkinResolutionDecision decision;
    if (preset.width > 0) {
        decision.width = preset.width;
        decision.height = preset.height;
    }
    else {
        decision.width = (std::min)(kMaximumWidth,
            RoundUpToEight(requiredWidth));
        decision.height = (std::min)(kMaximumHeight,
            RoundUpToEight(requiredHeight));
    }
    decision.source = SESkinResolutionSource::DestinationBounds;
    decision.destinationEvidenceCount = (int)destinationExtents.size();
    return decision;
}

const char* SESkinResolutionSourceText(SESkinResolutionSource source) {
    switch (source) {
    case SESkinResolutionSource::Information: return "#INFORMATION";
    case SESkinResolutionSource::ResolutionCommand: return "#RESOLUTION";
    case SESkinResolutionSource::DestinationBounds: return "inferred DST bounds";
    default: return "640 x 480 fallback";
    }
}

bool SEIsInferredSkinResolution(SESkinResolutionSource source) {
    return source == SESkinResolutionSource::DestinationBounds;
}
