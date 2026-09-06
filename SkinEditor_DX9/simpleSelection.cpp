#include "winWorkspace.h"
#include "winWorkspaceUiHelpers.h"
#include "seHelper.h"

#include <cmath>
#include <climits>
#include <cstdlib>
#include <set>

namespace {
bool StartsWith(const std::string& text, const char* prefix) {
    return text.compare(0, strlen(prefix), prefix) == 0;
}

std::string CsvField(const std::string& text, int column) {
    size_t start = 0;
    for (int i = 0; i < column; ++i) {
        const size_t comma = text.find(',', start);
        if (comma == std::string::npos) return "";
        start = comma + 1;
    }
    return text.substr(start, text.find(',', start) - start);
}

// Change only the requested token, retaining unknown fields, empty tokens and
// byte encoding elsewhere in the imported script.
void ReplaceFieldText(std::string& text, int column, const std::string& value) {
    size_t start = 0;
    for (int i = 0; i < column; ++i) {
        size_t comma = text.find(',', start);
        if (comma == std::string::npos) {
            text.push_back(',');
            comma = text.size() - 1;
        }
        start = comma + 1;
    }
    const size_t end = text.find(',', start);
    text.replace(start, end == std::string::npos ? end : end - start, value);
}

void ReplaceField(std::string& text, int column, int value) {
    ReplaceFieldText(text, column, std::to_string(value));
}

bool ReadInteger(const std::string& text, int column, int& value) {
    if (column < 0) return false;
    const std::string token = CsvField(text, column);
    if (token.find_first_not_of(" \t\r") == std::string::npos) { value = 0; return true; }
    char* end = nullptr;
    const double number = strtod(token.c_str(), &end);
    if (end == token.c_str()) return false;
    while (*end == ' ' || *end == '\t' || *end == '\r') ++end;
    if (*end || !std::isfinite(number) || number < INT_MIN || number > INT_MAX ||
        std::trunc(number) != number) return false;
    value = (int)number;
    return true;
}

bool StoreRounded(std::string& text, int column, double value) {
    if (column < 0 || !std::isfinite(value) || value < INT_MIN || value > INT_MAX) return false;
    ReplaceField(text, column, (int)std::round(value));
    return true;
}

bool IsSupportedDestination(const std::string& command) {
    if (!StartsWith(command, "#DST_")) return false;
    return FindCommandFieldColumn(command.c_str(), "time") >= 0 &&
        FindCommandFieldColumn(command.c_str(), "x") >= 0 &&
        FindCommandFieldColumn(command.c_str(), "y") >= 0 &&
        FindCommandFieldColumn(command.c_str(), "w") >= 0 &&
        (FindCommandFieldColumn(command.c_str(), "h") >= 0 ||
            FindCommandFieldColumn(command.c_str(), "size") >= 0);
}

bool IsAnimationValid(const SESelectionEdit& edit) {
    return edit.timer >= -1 && edit.timer < 200 && edit.delayMs >= 0 &&
        edit.delayMs <= 60000 && edit.durationMs >= 20 && edit.durationMs <= 60000 &&
        edit.minimumOpacityPercent >= 0 && edit.minimumOpacityPercent <= 100 &&
        edit.effect >= SESelectionEffect::FadeIn && edit.effect <= SESelectionEffect::Pulse;
}
} // namespace

bool SESelectionMatchesGroup(const SESelectionTimeline& timeline, SESelectionGroup group) {
    const std::string& command = timeline.command;
    switch (group) {
    case SESelectionGroup::SongList: return StartsWith(command, "#DST_BAR_");
    case SESelectionGroup::Highlight:
        return command == "#DST_BAR_BODY_ON" || command == "#DST_BAR_FLASH";
    case SESelectionGroup::SongInfo:
        return command == "#DST_TEXT" || command == "#DST_NUMBER";
    case SESelectionGroup::Graphs: return command == "#DST_BARGRAPH";
    case SESelectionGroup::Controls:
        return command == "#DST_BUTTON" || command == "#DST_SLIDER" || command == "#DST_ONMOUSE";
    case SESelectionGroup::Decoration:
        return command == "#DST_IMAGE" || command == "#DST_BGA";
    case SESelectionGroup::SelectedObjects: return true;
    }
    return false;
}

const std::vector<SESelectionTimeline>& WORKSPACE::GetSimpleSelectionTimelines() {
    if (!simpleSelectionProjectionDirty) return simpleSelectionProjection;
    simpleSelectionProjection.clear();
    simpleSelectionProjectionDirty = false;
    if (!loaded || meta.type != SKINTYPE_SELECT) return simpleSelectionProjection;

    std::vector<int> rowObjects(skinfileLines.count, -1);
    const auto& objects = objectEditorModel.Objects();
    for (int model = 0; model < (int)objects.size(); ++model)
        for (int row : objects[model].rows)
            if (row >= 0 && row < skinfileLines.count) rowObjects[row] = model;

    std::string previousCommand, previousOwner, previousIndex;
    int previousBranch = -1;
    // Source/control boundaries start a new timeline, while ordinary comments
    // between animation frames do not. ON/OFF and indexed bars stay separate.
    for (int row = 0; row < skinfileLines.count; ++row) {
        const auto& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
        const std::string text = line.line.body ? line.line.body : "";
        const std::string command = line.csv.str[0].body ? line.csv.str[0].body : "";
        if (StartsWith(text, "$FILE") || StartsWith(text, "$SE_OBJECT_ID")) previousCommand.clear();
        if (line.isComment) continue;
        if (!IsSupportedDestination(command)) { previousCommand.clear(); continue; }
        const std::string owner = line.filename.body ? line.filename.body : "";
        const int indexColumn = FindCommandFieldColumn(command.c_str(), "index");
        const std::string index = indexColumn >= 0 ? CsvField(text, indexColumn) : "";
        if (command != previousCommand || owner != previousOwner ||
            index != previousIndex || line.ifgroup != previousBranch) {
            SESelectionTimeline timeline;
            timeline.command = command;
            timeline.objectIndex = rowObjects[row];
            timeline.ifgroup = line.ifgroup;
            timeline.isBarAnchor = command == "#DST_BAR_BODY_OFF" || command == "#DST_BAR_BODY_ON";
            timeline.isBarRelative = StartsWith(command, "#DST_BAR_") && !timeline.isBarAnchor;
            timeline.label = timeline.objectIndex >= 0
                ? objects[timeline.objectIndex].name : command.substr(5);
            if (!index.empty()) timeline.label += " [" + index + "]";
            simpleSelectionProjection.push_back(std::move(timeline));
        }
        simpleSelectionProjection.back().rows.push_back(row);
        previousCommand = command;
        previousOwner = owner;
        previousIndex = index;
        previousBranch = line.ifgroup;
    }
    return simpleSelectionProjection;
}

std::vector<SESelectionTimeline> WORKSPACE::GetSimpleSelectionTargets(const SESelectionEdit& edit) {
    std::set<int> selectedModels;
    for (const auto& key : objectSelection.selected) {
        const int model = ResolveObjectSelectionKey(key);
        if (model >= 0) selectedModels.insert(model);
    }
    const int activeModel = ResolveObjectSelectionKey(objectSelection.active);
    const auto& objects = objectEditorModel.Objects();
    if (edit.selectedBranchOnly && (activeModel < 0 || activeModel >= (int)objects.size())) return {};
    std::vector<SESelectionTimeline> result;
    for (const auto& timeline : GetSimpleSelectionTimelines()) {
        if (!SESelectionMatchesGroup(timeline, edit.group)) continue;
        if (edit.group == SESelectionGroup::SelectedObjects &&
            selectedModels.find(timeline.objectIndex) == selectedModels.end()) continue;
        if (edit.selectedBranchOnly && timeline.ifgroup != objects[activeModel].ifgroup) continue;
        result.push_back(timeline);
    }
    return result;
}

bool WORKSPACE::ApplySimpleSelectionEdit(const SESelectionEdit& edit, std::string& message) {
    message.clear();
    if (!loaded || meta.type != SKINTYPE_SELECT || applyingHistory || pendingHistorySnapshotRestore >= 0) {
        message = "Open a Selection skin and finish the pending Undo first."; return false;
    }
    if (edit.kind < SESelectionEditKind::Layout || edit.kind > SESelectionEditKind::Animation ||
        (edit.kind == SESelectionEditKind::Layout && (!std::isfinite(edit.scalePercent) ||
            edit.scalePercent < 1 || edit.scalePercent > 1000)) ||
        (edit.kind == SESelectionEditKind::Animation && !IsAnimationValid(edit))) {
        message = "Use scale 1-1000%, duration 20-60000 ms and a valid event timer."; return false;
    }
    for (float channel : edit.tint) if (edit.kind == SESelectionEditKind::Tint &&
        (!std::isfinite(channel) || channel < 0 || channel > 1)) {
        message = "Color and opacity must be between 0 and 100%."; return false;
    }
    // Re-resolve selection and rows for every command; never apply a cached row
    // from the preceding ImGui frame to a newly loaded or reparsed document.
    const auto targets = GetSimpleSelectionTargets(edit);
    if (targets.empty()) { message = "This group has no editable components. Choose Objects or another group."; return false; }
    const bool hasBarAnchors = std::any_of(targets.begin(), targets.end(),
        [](const auto& target) { return target.isBarAnchor; });
    if (edit.kind == SESelectionEditKind::Animation && hasBarAnchors && edit.timer > 0) {
        message = "LR2 song-list row anchors use scene entry. Choose Scene entry or select only the title/flash Objects.";
        return false;
    }
    const SkinDocumentSnapshot before = CaptureDocumentSnapshot();
    std::map<int, std::vector<SkinLineSnapshot>> replacements;
    std::set<int> removed;
    std::set<int> eventTimers;
    double pivotX = 0, pivotY = 0;
    bool hasPivot = false;
    for (const auto& target : targets) {
        if (edit.kind != SESelectionEditKind::Layout || target.isBarRelative) continue;
        int x = 0, y = 0;
        const std::string& text = before.lines[target.rows.front()].line;
        if (!ReadInteger(text, FindCommandFieldColumn(target.command.c_str(), "x"), x) ||
            !ReadInteger(text, FindCommandFieldColumn(target.command.c_str(), "y"), y)) {
            message = "A target has non-integer coordinates. No components were changed."; return false;
        }
        pivotX = hasPivot ? (std::min)(pivotX, (double)x) : x;
        pivotY = hasPivot ? (std::min)(pivotY, (double)y) : y;
        hasPivot = true;
    }

    for (const auto& target : targets) {
        const char* command = target.command.c_str();
        const int xCol = FindCommandFieldColumn(command, "x");
        const int yCol = FindCommandFieldColumn(command, "y");
        const int wCol = FindCommandFieldColumn(command, "w");
        int hCol = FindCommandFieldColumn(command, "h");
        if (hCol < 0) hCol = FindCommandFieldColumn(command, "size");
        const int alphaCol = FindCommandFieldColumn(command, "a");
        const int timeCol = FindCommandFieldColumn(command, "time");
        const int timerCol = FindCommandFieldColumn(command, "timer");
        const int loopCol = FindCommandFieldColumn(command, "loop");
        const auto fail = [&]() {
            message = "Cannot safely edit " + target.command + " near row " +
                std::to_string(target.rows.front() + 1) + ". No components were changed.";
            return false;
        };
        if (edit.kind == SESelectionEditKind::Animation) {
            if (target.rows.size() > 1 && !edit.replaceAnimation) {
                message = "Existing animation found. Enable Replace existing animation to apply this preset.";
                return false;
            }
            SkinLineSnapshot pose = before.lines[target.rows.front()];
            // The first row also owns extension/unknown condition columns.
            // Copy only known per-frame pose fields from the settled last row.
            for (const char* field : { "x", "y", "w", "h", "size", "a", "r", "g", "b",
                "blend", "filter", "angle", "center" }) {
                const int column = FindCommandFieldColumn(command, field);
                if (column >= 0) ReplaceFieldText(pose.line, column,
                    CsvField(before.lines[target.rows.back()].line, column));
            }
            int alpha = 0, x = 0, y = 0, timer = 0;
            if (timerCol < 0 || loopCol < 0 ||
                !ReadInteger(pose.line, alphaCol, alpha) || alpha < 0 || alpha > 255 ||
                !ReadInteger(pose.line, xCol, x) || !ReadInteger(pose.line, yCol, y) ||
                !ReadInteger(before.lines[target.rows.front()].line, timerCol, timer)) return fail();
            if (edit.timer >= 0) timer = edit.timer;
            if (target.isBarAnchor) timer = 0; // LR2 hard-codes this clock for row anchors.
            if (timer < 0 || timer >= 200) return fail();
            eventTimers.insert(timer);
            ReplaceField(pose.line, timerCol, timer);
            ReplaceField(pose.line, loopCol, edit.effect == SESelectionEffect::Pulse
                ? edit.delayMs : edit.delayMs + edit.durationMs);
            const int accCol = FindCommandFieldColumn(command, "acc");
            if (accCol >= 0) ReplaceField(pose.line, accCol, 0);
            pose.modified = true;
            std::vector<SkinLineSnapshot> frames(edit.effect == SESelectionEffect::Pulse ? 3 : 2, pose);
            ReplaceField(frames.front().line, timeCol, edit.delayMs);
            ReplaceField(frames.back().line, timeCol, edit.delayMs + edit.durationMs);
            if (edit.effect == SESelectionEffect::FadeIn || edit.effect == SESelectionEffect::SlideIn)
                ReplaceField(frames.front().line, alphaCol, 0);
            if (edit.effect == SESelectionEffect::FadeOut)
                ReplaceField(frames.back().line, alphaCol, 0);
            if (edit.effect == SESelectionEffect::SlideIn && !(hasBarAnchors && target.isBarRelative)) {
                if (!StoreRounded(frames.front().line, xCol, (double)x + edit.slideX) ||
                    !StoreRounded(frames.front().line, yCol, (double)y + edit.slideY)) return fail();
            }
            if (edit.effect == SESelectionEffect::Pulse) {
                ReplaceField(frames[1].line, timeCol, edit.delayMs + edit.durationMs / 2);
                ReplaceField(frames[1].line, alphaCol, alpha * edit.minimumOpacityPercent / 100);
            }
            replacements[target.rows.front()] = std::move(frames);
            removed.insert(target.rows.begin() + 1, target.rows.end());
        } else for (int row : target.rows) {
            SkinLineSnapshot changed = before.lines[row];
            if (edit.kind == SESelectionEditKind::Layout) {
                const double scale = edit.scalePercent / 100.0;
                const int columns[] = { xCol, yCol, wCol, hCol };
                for (int field = 0; field < 4; ++field) {
                    int original = 0;
                    if (!ReadInteger(changed.line, columns[field], original)) return fail();
                    const double pivot = target.isBarRelative ? 0 : (field == 0 ? pivotX : pivotY);
                    const int offset = target.isBarRelative && hasBarAnchors ? 0 :
                        (field == 0 ? edit.offsetX : edit.offsetY);
                    const double value = field < 2 ? pivot + (original - pivot) * scale + offset : original * scale;
                    if (!StoreRounded(changed.line, columns[field], value)) return fail();
                }
            } else {
                const char* fields[] = { "r", "g", "b", "a" };
                for (int channel = 0; channel < 4; ++channel) {
                    const int column = FindCommandFieldColumn(command, fields[channel]);
                    int original = 0;
                    if (!ReadInteger(changed.line, column, original) || original < 0 || original > 255 ||
                        !StoreRounded(changed.line, column, original * edit.tint[channel])) return fail();
                }
            }
            changed.modified = changed.line != before.lines[row].line || changed.modified;
            replacements[row] = { std::move(changed) };
        }
    }
    SkinDocumentSnapshot after;
    after.selection = before.selection;
    for (int row = 0; row < (int)before.lines.size(); ++row) {
        const auto replacement = replacements.find(row);
        if (replacement != replacements.end())
            after.lines.insert(after.lines.end(), replacement->second.begin(), replacement->second.end());
        else if (removed.find(row) == removed.end()) after.lines.push_back(before.lines[row]);
    }
    const bool unchanged = after.lines.size() == before.lines.size() &&
        std::equal(after.lines.begin(), after.lines.end(), before.lines.begin(),
            [](const auto& a, const auto& b) { return a.line == b.line; });
    if (unchanged) { message = "These settings make no changes."; return true; }

    const auto savedRedo = redoDocumentSnapshots;
    const auto savedRevision = documentRevision;
    if (RestoreDocumentSnapshot(after) != 0) {
        RestoreDocumentSnapshot(before);
        redoDocumentSnapshots = savedRedo;
        documentRevision = savedRevision;
        message = "Could not apply the group; the previous document was restored."; return false;
    }
    historyDocumentSnapshots.push_back(before);
    HISTORY* history = (HISTORY*)arr_history.Get_new();
    if (!history) {
        historyDocumentSnapshots.pop_back();
        RestoreDocumentSnapshot(before);
        redoDocumentSnapshots = savedRedo;
        documentRevision = savedRevision;
        message = "Could not record Undo; the previous document was restored."; return false;
    }
    history->op = restoreDocument;
    history->target = (int)historyDocumentSnapshots.size() - 1;
    simpleSelectionPreviewTimers.assign(eventTimers.begin(), eventTimers.end());
    wPreview = true;
    message = "Applied to " + std::to_string(targets.size()) + " component timelines. Undo restores the entire group.";
    return true;
}

void WORKSPACE::ReplaySimpleSelectionEffect() {
    std::set<int> timers;
    for (const auto& timeline : GetSimpleSelectionTargets(simpleSelectionEdit)) {
        const auto& line = ((SKINFILELINEREAD*)skinfileLines.data)[timeline.rows.front()];
        int timer = 0;
        if (timeline.isBarAnchor || ReadInteger(line.line.body ? line.line.body : "",
            FindCommandFieldColumn(timeline.command.c_str(), "timer"), timer)) {
            if (timer >= 0 && timer < 200) timers.insert(timer);
        }
    }
    simpleSelectionPreviewTimers.assign(timers.begin(), timers.end());
    wPreview = true;
}
