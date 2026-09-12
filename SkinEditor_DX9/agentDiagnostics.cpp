#include "agentDiagnostics.h"
#include "agentUtility.h"
#include "winWorkspace.h"
#include "winWorkspaceUiHelpers.h"
#include "inputwrap.h"
#include "../LR2/LR2_skinload.h"
#include "../LR2/LR2_skindraw.h"
#include "../LR2/LR2_skinobject.h"
#include "../LR2/En_timer.h"

#include <cmath>
#include <climits>
#include <cstdlib>
#include <cstring>
#include <locale>
#include <sstream>

namespace {
std::string Quote(const std::string& text) {
    static const char* hex = "0123456789abcdef";
    std::string result = "\"";
    for (unsigned char c : text) {
        if (c == '"' || c == '\\') { result += '\\'; result += c; }
        else if (c < 32) { result += "\\u00"; result += hex[c >> 4]; result += hex[c & 15]; }
        else result += c;
    }
    return result + '"';
}

const char* Command(const SKINFILELINEREAD& row) {
    return row.csv.str[0].body ? row.csv.str[0].body : "";
}

struct Issue {
    int object = -1;
    int row = -1;
    std::string code, severity, hint;
    const char* scope = "current_preview";
    bool hidden = false;
};

struct ObjectResult {
    std::string status = "unknown";
    int timelines = 0;
    int hiddenTimelines = 0;
    std::vector<int> issues;
};

// ReadDST is the LR2 parser used by ReadSkinSE. Keep its allocations local;
// arr_DST is an editor cache and may cross special SRC/Object boundaries.
struct LocalTimeline {
    DSTstruct dst{};
    LocalTimeline() { InitDST(&dst); }
    ~LocalTimeline() { free(dst.draw); }
    LocalTimeline(const LocalTimeline&) = delete;
    LocalTimeline& operator=(const LocalTimeline&) = delete;
};

bool HasCommonTimeline(const char* command) {
    // FindCommandFieldColumn resolves the canonical GetCommandHelp schema,
    // including fields such as TEXT's size. Do not maintain a CSV column map.
    return FindCommandFieldColumn(command, "time") >= 0 &&
        FindCommandFieldColumn(command, "x") >= 0 &&
        FindCommandFieldColumn(command, "y") >= 0 &&
        FindCommandFieldColumn(command, "w") >= 0 &&
        (FindCommandFieldColumn(command, "h") >= 0 ||
            FindCommandFieldColumn(command, "size") >= 0);
}

bool HasAbsoluteBounds(const std::string& command) {
    return command == "#DST_IMAGE" || command == "#DST_NUMBER" ||
        command == "#DST_TEXT" || command == "#DST_BUTTON" ||
        command == "#DST_GROOVEGAUGE" || command == "#DST_MASK" ||
        command == "#DST_BARGRAPH" || command == "#DST_BGA";
}

bool HasCommonVisibility(const std::string& command) {
    // BAR rows, relative combos and chart primitives are interpreted by scene
    // code. Their raw DST is not sufficient proof of an invisible final draw.
    return HasAbsoluteBounds(command) || command == "#DST_SLIDER" ||
        command == "#DST_ONMOUSE" || command == "#DST_LINE" ||
        command == "#DST_JUDGELINE";
}

DST_ANIMATION EditorFrame(const DSTdraw& frame) {
    DST_ANIMATION result{};
    result.time = frame.time;
    result.x = frame.x; result.y = frame.y;
    result.w = frame.w; result.h = frame.h;
    result.acc = frame.acc;
    result.a = frame.a; result.r = frame.r;
    result.g = frame.g; result.b = frame.b;
    result.blend = frame.blend; result.filter = frame.filter;
    result.angle = frame.angle; result.center = frame.center;
    return result;
}
} // namespace

std::string SEAgentDiagnostics(WORKSPACE& workspace) {
    const auto& objects = workspace.objectEditorModel.Objects();
    auto* rows = (SKINFILELINEREAD*)workspace.skinfileLines.data;
    const bool hasMask = workspace.previewRuntimeLineMask.size() ==
        (size_t)workspace.skinfileLines.count;
    std::vector<int> rowObjects((size_t)workspace.skinfileLines.count, -1);
    for (int object = 0; object < (int)objects.size(); ++object)
        for (int row : objects[object].rows)
            if (row >= 0 && row < workspace.skinfileLines.count) rowObjects[row] = object;

    std::vector<Issue> issues;
    std::vector<ObjectResult> results(objects.size());
    auto add = [&](int object, int row, const char* code, const char* severity,
        const std::string& hint, bool hidden = false, const char* scope = "current_preview") {
        Issue issue;
        issue.object = object; issue.row = row;
        issue.code = code; issue.severity = severity; issue.hint = hint;
        issue.hidden = hidden; issue.scope = scope;
        if (object >= 0) results[object].issues.push_back((int)issues.size());
        issues.push_back(std::move(issue));
    };

    // This editor scan intentionally includes inactive/customization branches.
    // Missing optional resources are useful authoring warnings, not proof that
    // the selected Object is hidden in the current runtime branch.
    std::vector<SEImageDiagnostic> assets;
    workspace.BuildImageDiagnostics(assets);
    for (const auto& asset : assets) {
        const char* code = nullptr;
        const char* hint = nullptr;
        switch (asset.kind) {
        case SEImageDiagnosticKind::MissingFile:
            code = "missing_file"; hint = "Check the owning #IMAGE path and selected custom file."; break;
        case SEImageDiagnosticKind::UnloadableFile:
            code = "unloadable_file"; hint = "Check that the file is a supported, readable image."; break;
        case SEImageDiagnosticKind::CropOutOfBounds:
            code = "crop_out_of_bounds"; hint = "Keep SRC x/y/w/h inside the selected image dimensions."; break;
        case SEImageDiagnosticKind::SourceWithoutAsset:
            code = "source_without_asset"; hint = "Check the SRC graphic index and image crop. Editor asset metadata alone does not prove runtime failure."; break;
        default: continue; // Duplicate/unused crops do not explain visibility.
        }
        int row = asset.sourceRow;
        if (row < 0 && asset.assetIndex >= 0 && asset.assetIndex < workspace.arr_IMG.count) {
            const auto& image = ((IMG*)workspace.arr_IMG.data)[asset.assetIndex];
            row = image.sourceDeclare >= 0 ? image.sourceDeclare : image.editorDeclare;
        }
        if (row < 0 && asset.graphicIndex >= 0 && asset.graphicIndex < workspace.arr_SRCGR.count)
            row = ((SRCGR*)workspace.arr_SRCGR.data)[asset.graphicIndex].declare;
        const int object = row >= 0 && row < (int)rowObjects.size() ? rowObjects[row] : -1;
        add(object, row, code, "warning", asset.message + ". " + hint, false, "all_branches");
    }

    for (int objectIndex = 0; objectIndex < (int)objects.size(); ++objectIndex) {
        const auto& object = objects[objectIndex];
        ObjectResult& result = results[objectIndex];
        const int anchor = object.rows.empty() ? -1 : object.rows.front();
        std::vector<SEPreviewObjectDestination> destinations;
        workspace.CollectPreviewObjectDestinations(object, destinations);
        result.timelines = (int)destinations.size();
        bool uncertain = !hasMask;
        if (!hasMask)
            add(objectIndex, anchor, "condition_state_unknown", "info",
                "Load the native preview before diagnosing IF/include state.");

        for (int row : object.rows) {
            if (row < 0 || row >= workspace.skinfileLines.count ||
                !hasMask || !workspace.previewRuntimeLineMask[row]) continue;
            const char* sourceCommand = Command(rows[row]);
            int columns[5];
            if (strncmp(sourceCommand, "#SRC_", 5) ||
                !workspace.ResolveImageCropColumns(sourceCommand, columns)) continue;
            const int graphic = rows[row].csv.val[columns[0]];
            // 100+ are LR2 runtime resources, not normal #IMAGE declarations.
            // Read the actual active runtime table, not editor branch-local gr
            // candidates, so an optional missing image in another branch does
            // not make a working source look unavailable.
            if (graphic >= 0 && graphic < 100 &&
                (graphic >= workspace.g.skstruct.count || workspace.g.skstruct.GrHandle[graphic] < 0)) {
                add(objectIndex, row, "source_asset_unavailable", "warning",
                    "SRC graphic " + std::to_string(graphic) +
                    " has no loaded image in the active runtime table. Check its #IMAGE declaration, path and selected custom file. Other source variants may still draw.");
                uncertain = true;
            }
        }

        if (destinations.empty()) {
            add(objectIndex, anchor, "missing_dst", "warning",
                "This Object has no DST timeline. Add the corresponding destination; shared/special sources may use another Object's DST.");
            // Notes and special sources can share an indexed DST elsewhere.
            result.status = "unknown";
            continue;
        }

        for (const auto& destination : destinations) {
            const int firstRow = destination.firstRow;
            if (firstRow < 0 || firstRow >= workspace.skinfileLines.count) continue;
            const std::string command = Command(rows[firstRow]);
            bool anyActive = false;
            bool anyFileVisible = false;
            std::vector<int> timelineRows;
            for (int row : object.rows) {
                if (row < destination.firstRow || row > destination.lastRow ||
                    row < 0 || row >= workspace.skinfileLines.count || command != Command(rows[row])) continue;
                const bool active = !hasMask || workspace.previewRuntimeLineMask[row] != 0;
                if (active) {
                    anyActive = true;
                    if (workspace.IsPreviewRowVisible(row)) anyFileVisible = true;
                    timelineRows.push_back(row);
                }
            }
            if (hasMask && !anyActive) {
                add(objectIndex, firstRow, "if_inactive", "info",
                    "This timeline is excluded by the loaded #IF/#ELSE/#INCLUDE state. Select the matching customization and reload.", true);
                ++result.hiddenTimelines;
                continue;
            }
            if (!anyFileVisible) {
                add(objectIndex, firstRow, "preview_file_hidden", "info",
                    "Enable this owner file in Preview to display its Objects.", true);
                ++result.hiddenTimelines;
                continue;
            }
            if (!HasCommonTimeline(command.c_str()) || timelineRows.empty()) {
                add(objectIndex, firstRow, "timeline_unknown", "info",
                    "This command does not expose a standard DST timeline. Inspect its command help and rendered preview.");
                uncertain = true;
                continue;
            }

            LocalTimeline parsed;
            for (int row : timelineRows) ReadDST(&parsed.dst, &rows[row].csv, row);
            DSTstruct& timeline = parsed.dst;
            if (!timeline.draw || timeline.dstCount <= 0) {
                add(objectIndex, firstRow, "timeline_unknown", "warning", "The native DST parser produced no frames.");
                uncertain = true;
                continue;
            }
            const bool commonVisibility = HasCommonVisibility(command);
            bool hidden = false;
            if (!commonVisibility) {
                add(objectIndex, firstRow, "special_runtime_semantics", "info",
                    "Scene code controls this element's selection, timer and/or relative position. Raw DST bounds are not used to declare it hidden.");
                uncertain = true;
                continue;
            }
            const int options[] = { timeline.op1, timeline.op2, timeline.op3 };
            for (int option : options) {
                if (option < -999 || option > 999) {
                    add(objectIndex, firstRow, "option_unknown", "warning", "DST option is outside the supported -999..999 range.");
                    uncertain = true;
                } else if (!GetOptionFlag_dst(&workspace.g, option)) {
                    add(objectIndex, firstRow, "dst_option_false", "info",
                        "DST option " + std::to_string(option) + " is false in this preview. Check its state or choose a matching preview case.", true);
                    hidden = true;
                }
            }
            if (timeline.timer < 0 || timeline.timer >= 500) {
                add(objectIndex, firstRow, "timer_unknown", "warning", "DST timer is outside the native 0..499 timer array.");
                uncertain = true;
                if (hidden) ++result.hiddenTimelines;
                continue;
            }
            const double elapsed = GetTimeLapse((unsigned int)timeline.timer, &workspace.g.timer1);
            if (elapsed < 0.0) {
                add(objectIndex, firstRow, "timer_inactive", "info",
                    "DST timer " + std::to_string(timeline.timer) + " has not started in this preview. Trigger it or use a case with that timer active.", true);
                ++result.hiddenTimelines;
                continue;
            }
            if (!std::isfinite(elapsed) || elapsed > 2147483647.0) {
                add(objectIndex, firstRow, "timer_unknown", "warning", "Elapsed timer value cannot be safely evaluated as a native frame time.");
                uncertain = true;
                if (hidden) ++result.hiddenTimelines;
                continue;
            }
            DSTdraw frame = SetDSTdrawByTime(timeline, elapsed);
            if (frame.time == -1) {
                const bool before = elapsed < timeline.draw[0].time;
                const bool after = timeline.loop < 0 && elapsed > timeline.draw[timeline.dstCount - 1].time;
                add(objectIndex, firstRow, before ? "before_animation" : after ? "after_animation" : "animation_inactive",
                    "info", before ? "Preview time is before the first DST frame. Advance time or adjust the first frame." :
                    after ? "The non-looping DST animation has ended. Check its final time/loop value or render an earlier case." :
                    "The native interpolator produced no frame. Check ordered keyframe times and loop bounds.", true);
                ++result.hiddenTimelines;
                continue;
            }
            if (frame.a < 2) {
                const bool alphaHidden = frame.blend != 0 &&
                    (command != "#DST_TEXT" || frame.a == 0);
                add(objectIndex, firstRow, "alpha_zero", alphaHidden ? "warning" : "info",
                    alphaHidden ? "The interpolated alpha is below 2 in a blending mode. Raise alpha or change the animation time." :
                    frame.blend == 0 ? "Alpha is below 2, but LR2 blend 0 ignores alpha. This value alone does not hide the image." :
                    "Text alpha is near zero. The text renderer may still emit faint pixels; inspect the preview.", alphaHidden);
                hidden = hidden || alphaHidden;
            }
            if (frame.w == 0.0f || frame.h == 0.0f) {
                add(objectIndex, firstRow, "zero_size", "warning", "The interpolated DST width or height is zero. Give it a nonzero size.", true);
                hidden = true;
            }

            // A full digit field can fit on screen while the renderer silently
            // discards digits beyond keta. Check the same live numeric value
            // LR2 uses, independently of rectangle clipping or rotation.
            if (command == "#DST_NUMBER" && hasMask) {
                int sourceRow = -1;
                for (int row : object.rows)
                    if (row >= 0 && row < firstRow && row < workspace.skinfileLines.count &&
                        workspace.previewRuntimeLineMask[row] &&
                        !strcmp(Command(rows[row]), "#SRC_NUMBER")) sourceRow = row;
                if (sourceRow >= 0) {
                    const auto& source = rows[sourceRow];
                    const int numberColumn = FindCommandFieldColumn("#SRC_NUMBER", "num");
                    const int digitsColumn = FindCommandFieldColumn("#SRC_NUMBER", "keta");
                    const int divX = FindCommandFieldColumn("#SRC_NUMBER", "div_x");
                    const int divY = FindCommandFieldColumn("#SRC_NUMBER", "div_y");
                    const int alignColumn = FindCommandFieldColumn("#SRC_NUMBER", "align(0right1left2middle)");
                    if (numberColumn >= 0 && digitsColumn >= 0 && divX >= 0 && divY >= 0 && alignColumn >= 0) {
                        const int keta = source.csv.val[digitsColumn];
                        const int align = source.csv.val[alignColumn];
                        const long long cells = (long long)source.csv.val[divX] * source.csv.val[divY];
                        // The renderer recognizes 10/11-cell unsigned atlases
                        // and 24-cell signed atlases. Signed/sentinel values
                        // need different rules, so leave those unclassified.
                        if (keta > 0 && align >= 0 && align <= 2 &&
                            source.csv.val[divX] > 0 && source.csv.val[divY] > 0 && cells > 0 &&
                            cells % 24 != 0 && (cells % 10 == 0 || cells % 11 == 0)) {
                            const unsigned int value = SetObjectValue_Num(&workspace.g, source.csv.val[numberColumn]);
                            if (value <= INT_MAX) {
                                const std::string digits = std::to_string(value);
                                if (digits.size() > (size_t)keta)
                                    add(objectIndex, sourceRow, "number_truncated", "warning",
                                        "Current value " + digits + " needs " + std::to_string(digits.size()) +
                                        " digits, but SRC_NUMBER keta is " + std::to_string(keta) +
                                        ". LR2 truncates the value inside this field. Increase keta and check the resulting full field width.");
                            }
                        }
                    }
                }
            }

            const auto& adjustment = workspace.g.skstruct.adjust;
            const bool transformed = adjustment.rate_x != 100 || adjustment.rate_y != 100 ||
                adjustment.shift_x != 0 || adjustment.shift_y != 0 ||
                (command == "#DST_IMAGE" && (timeline.op4 == 1 || timeline.op4 == 2));
            if (!HasAbsoluteBounds(command) || frame.angle != 0.0f || transformed) {
                add(objectIndex, firstRow, "bounds_unknown", "info",
                    "This element has a runtime offset or rotation. Check the rendered preview; raw rectangle clipping would be misleading.");
                uncertain = true;
            } else {
                float x, y, w, h;
                // A semantic Object can contain several SRC/DST states. Bounds
                // must use this timeline's active SRC rather than the final
                // NUMBER/TEXT declaration in the whole Object.
                SEObjectInstance boundsObject = object;
                boundsObject.rows.clear();
                const std::string sourceCommand = "#SRC_" + command.substr(5);
                for (int row : object.rows)
                    if (row >= 0 && row < firstRow && row < workspace.skinfileLines.count &&
                        sourceCommand == Command(rows[row]) &&
                        (!hasMask || workspace.previewRuntimeLineMask[row])) {
                        boundsObject.rows.clear();
                        boundsObject.rows.push_back(row);
                    }
                if (boundsObject.rows.empty()) {
                    add(objectIndex, firstRow, "source_binding_unknown", "warning",
                        "No matching active SRC belongs to this timeline's Object. Check source ownership; a shared source may be declared elsewhere.");
                    uncertain = true;
                    if (hidden) ++result.hiddenTimelines;
                    continue;
                }
                workspace.ResolvePreviewObjectFrameBounds(boundsObject, EditorFrame(frame), x, y, w, h);
                // Signed NUMBER atlases add a sign cell before their configured
                // digits. The generic editor bounds helper currently omits it;
                // report uncertainty instead of an incorrect clipping verdict.
                if (command == "#DST_NUMBER") {
                    const auto& source = rows[boundsObject.rows.front()];
                    const int divX = FindCommandFieldColumn(sourceCommand.c_str(), "div_x");
                    const int divY = FindCommandFieldColumn(sourceCommand.c_str(), "div_y");
                    const long long cells = divX >= 0 && divY >= 0 ?
                        (long long)source.csv.val[divX] * source.csv.val[divY] : 0;
                    if (cells > 0 && cells % 24 == 0) {
                        add(objectIndex, firstRow, "bounds_unknown", "info",
                            "This signed NUMBER atlas has an extra sign cell. Inspect the rendered full digit field for clipping.");
                        uncertain = true;
                        if (hidden) ++result.hiddenTimelines;
                        continue;
                    }
                }
                const float left = (std::min)(x, x + w);
                const float top = (std::min)(y, y + h);
                const float right = (std::max)(x, x + w);
                const float bottom = (std::max)(y, y + h);
                if (!std::isfinite(left) || !std::isfinite(top) || !std::isfinite(right) || !std::isfinite(bottom)) {
                    add(objectIndex, firstRow, "bounds_unknown", "warning", "The resolved rectangle contains non-finite coordinates.");
                    uncertain = true;
                } else if (right <= 0 || bottom <= 0 || left >= workspace.skinSizeX || top >= workspace.skinSizeY) {
                    add(objectIndex, firstRow, "offscreen", "warning",
                        "The resolved rectangle is outside the skin canvas. Move it inside the configured resolution.", true);
                    hidden = true;
                } else if (left < 0 || top < 0 || right > workspace.skinSizeX || bottom > workspace.skinSizeY) {
                    add(objectIndex, firstRow, "partially_clipped", "warning",
                        "The resolved rectangle crosses the canvas edge. Check all NUMBER digits or gauge cells and move/resize the element.");
                }
            }
            if (hidden) ++result.hiddenTimelines;
        }
        result.status = result.hiddenTimelines == result.timelines ? "hidden" :
            uncertain ? "unknown" : "not_proven_hidden";
    }

    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << "{\"scope\":\"current_native_preview\",\"resource_scan_scope\":\"all_branches\","
        "\"visibility_limit\":\"Passing these checks does not prove visible pixels: draw order, texture alpha, source variants and scene logic may still suppress or cover an element.\","
        "\"width\":" << workspace.skinSizeX << ",\"height\":" << workspace.skinSizeY << ",\"issues\":[";
    for (size_t i = 0; i < issues.size(); ++i) {
        const auto& issue = issues[i];
        if (i) out << ',';
        const bool validRow = issue.row >= 0 && issue.row < workspace.skinfileLines.count;
        out << "{\"index\":" << i << ",\"object_index\":";
        if (issue.object >= 0) out << issue.object; else out << "null";
        out << ",\"object_id\":";
        if (issue.object >= 0) out << Quote(Cp932ToUtf8(objects[issue.object].editorId.c_str())); else out << "null";
        out << ",\"file\":" << Quote(validRow ? SEAgentPathToUtf8(rows[issue.row].filename.body) : "")
            << ",\"line\":";
        if (validRow && rows[issue.row].num > 0) out << rows[issue.row].num; else out << "null";
        out << ",\"command\":" << Quote(validRow ? Command(rows[issue.row]) : "")
            << ",\"code\":" << Quote(issue.code) << ",\"severity\":" << Quote(issue.severity)
            << ",\"hint\":" << Quote(issue.hint) << ",\"scope\":" << Quote(issue.scope)
            << ",\"confirmed_hidden\":" << (issue.hidden ? "true" : "false") << ",\"branch_active\":";
        if (hasMask && validRow) out << (workspace.previewRuntimeLineMask[issue.row] ? "true" : "false"); else out << "null";
        out << '}';
    }
    out << "],\"objects\":[";
    for (size_t i = 0; i < objects.size(); ++i) {
        if (i) out << ',';
        const auto& result = results[i];
        out << "{\"index\":" << i << ",\"id\":" << Quote(Cp932ToUtf8(objects[i].editorId.c_str()))
            << ",\"name\":" << Quote(Cp932ToUtf8(objects[i].name.c_str()))
            << ",\"visibility\":" << Quote(result.status)
            << ",\"timeline_count\":" << result.timelines << ",\"hidden_timeline_count\":" << result.hiddenTimelines
            << ",\"issue_indices\":[";
        for (size_t j = 0; j < result.issues.size(); ++j) { if (j) out << ','; out << result.issues[j]; }
        out << "]}";
    }
    return out.str() + "]}";
}
