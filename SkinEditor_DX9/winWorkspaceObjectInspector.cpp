#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include "../LR2/structure.h"
#include "../LR2/LR2_skinmanage.h"
#include "../LR2/LR2_skindraw.h"
#include "../LR2/LR2_skinobject.h"
#include "../LR2/En_timer.h"
#include "../LR2/En_value.h"
#include "../lib/DxLib/DxLib.h"

#include "winWorkspace.h"
#include "skin.h"
#include "op.h"
#include "seHelper.h"
#include "seUI.h"
#include "skinPathResolver.h"
#include "inputwrap.h"
#include "uiCatalog.h"
#include "winWorkspaceUiHelpers.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>
int WORKSPACE::drawObjectInspector() {
    char inspectorTitle[128];
    FormatSEUIWindowTitle(inspectorTitle, sizeof(inspectorTitle),
        SEUIWindowId::ObjectInspector, num);
    if (!ImGui::Begin(inspectorTitle, &wObjectInspector)) {
        ImGui::End();
        return 0;
    }
    if (objectEditorModel.Groups().empty()) {
        ImGui::TextUnformatted("skinObjGroup.txt not loaded.");
        ImGui::End();
        return 0;
    }
    if (!loaded || skinfileLines.count <= 0) {
        ImGui::TextUnformatted("No skin loaded.");
        ImGui::End();
        return 0;
    }
    if (objectEditorLastLineCount != skinfileLines.count)
        RebuildObjectModel();

    int inspectorModelIndex = ResolveObjectSelectionKey(objectSelection.active);
    if (inspectorModelIndex < 0)
        inspectorModelIndex = preview_selected_object_model_index;
    auto formatConditionHeader = [](SKINFILELINEREAD& row) {
        return FormatObjectConditionHeader(row);
    };
    int renameObjectModelIndex = -1;
    std::string renamedObjectName;

    // Object Inspector: actual Object data. #IF/#ELSEIF/#ELSE are context only.
    const bool drawProperties = ImGui::BeginChild("ObjectProperties", ImVec2(0, 0), true);
    if (drawProperties) {
        if (inspectorModelIndex >= 0 && inspectorModelIndex < (int)objectEditorModel.Objects().size()) {
            SEObjectInstance& obj = objectEditorModel.ObjectsMutable()[inspectorModelIndex];
            const SEObjectGroupDef* def = objectEditorModel.Group(obj.group);
            if (def) {
                ImGui::Text("%s", def->name.c_str());

                int ifgroup = obj.ifgroup;
                if (ifgroup == 0 && !obj.rows.empty())
                    ifgroup = ((SKINFILELINEREAD*)skinfileLines.data)[obj.rows[0]].ifgroup;
                ImGui::SameLine();
                if (ifgroup == 0) {
                    ImGui::TextDisabled("[ALWAYS]");
                } else {
                    std::string flowName = "IF";
                    if (ifgroup < arr_ifunit.count) {
                        IFUNIT& unit = ((IFUNIT*)arr_ifunit.data)[ifgroup];
                        if (unit.order > 0) flowName = "ELSE";
                        for (int rno = 0; rno < skinfileLines.count; ++rno) {
                            SKINFILELINEREAD& r = ((SKINFILELINEREAD*)skinfileLines.data)[rno];
                            if (r.ifgroup != ifgroup || !r.isIfGroupHead || !r.csv.str[0].body) continue;
                            flowName = formatConditionHeader(r);
                            break;
                        }
                    }
                    ImGui::TextDisabled("[%s]", flowName.c_str());
                }
                ImGui::Separator();

                char objectName[256];
                const std::string objectNameUtf8 = Cp932ToUtf8(obj.name.c_str());
                strncpy(objectName, objectNameUtf8.c_str(), sizeof(objectName) - 1);
                objectName[sizeof(objectName) - 1] = '\0';
                ImGui::SetNextItemWidth(260.0f);
                const bool objectNameChanged =
                    ImGui::InputText("Name", objectName, sizeof(objectName));
                if (objectNameChanged)
                    obj.name = Utf8ToCp932(objectName);
                const bool commitObjectName = ImGui::IsItemDeactivatedAfterEdit() ||
                    (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter));
                if (commitObjectName) {
                    renameObjectModelIndex = inspectorModelIndex;
                    renamedObjectName = obj.name;
                }

                auto drawTaggedImageSelector = [&](int row, const char* command) {
                    if (!command || strncmp(command, "#SRC", 4) != 0 || arr_IMG.count <= 0) return;
                    SKINFILELINEREAD& srcLine = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    int columns[5] = { -1, -1, -1, -1, -1 };
                    if (!ResolveImageCropColumns(command, columns)) return;

                    const int currentTag = FindImageAssetForRow(row);
                    char preview[260];
                    if (currentTag >= 0 && currentTag < arr_IMG.count) {
                        IMG& tag = ((IMG*)arr_IMG.data)[currentTag];
                        const std::string tagNameUtf8 = Cp932ToUtf8(
                            tag.name.body ? tag.name.outstr() : "noname");
                        snprintf(preview, sizeof(preview), "%03d  %s", currentTag,
                            tagNameUtf8.c_str());
                    } else snprintf(preview, sizeof(preview), "Custom coordinates");

                    ImGui::SetNextItemWidth(300.0f);
                    if (ImGui::BeginCombo("Tagged image", preview)) {
                        for (int tagIndex = 0; tagIndex < arr_IMG.count; ++tagIndex) {
                            IMG& tag = ((IMG*)arr_IMG.data)[tagIndex];
                            char tagLabel[260];
                            const std::string tagNameUtf8 = Cp932ToUtf8(
                                tag.name.body ? tag.name.outstr() : "noname");
                            snprintf(tagLabel, sizeof(tagLabel), "%03d  G%02d  %s", tagIndex, tag.gr,
                                tagNameUtf8.c_str());
                            if (ImGui::Selectable(tagLabel, tagIndex == currentTag)) {
                                ApplyImageAssetToObjectSource(tagIndex,
                                    inspectorModelIndex, row,
                                    false);
                            }
                            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone) &&
                                tag.gr >= 0) {
                                int tagImageIndex = -1;
                                // Preview the tag against the branch of the
                                // SRC currently being edited. Coordinate tags
                                // themselves are shared across branches.
                                for (int candidate = 0; candidate < arr_SRCGR.count; ++candidate) {
                                    SRCGR& source = ((SRCGR*)arr_SRCGR.data)[candidate];
                                    if (source.grID == tag.gr && source.isIf == srcLine.ifgroup) {
                                        tagImageIndex = candidate;
                                        break;
                                    }
                                }
                                if (tagImageIndex < 0) {
                                    for (int candidate = 0; candidate < arr_SRCGR.count; ++candidate) {
                                        if (((SRCGR*)arr_SRCGR.data)[candidate].grID == tag.gr) {
                                            tagImageIndex = candidate;
                                            break;
                                        }
                                    }
                                }
                                if (tagImageIndex < 0) continue;
                                EnsureSRCGRTexture(tagImageIndex);
                                SRCGR& sourceImage = ((SRCGR*)arr_SRCGR.data)[tagImageIndex];
                                const int tagW = tag.w == -1 ? sourceImage.sizeX - tag.x : tag.w;
                                const int tagH = tag.h == -1 ? sourceImage.sizeY - tag.y : tag.h;
                                if (sourceImage.texture && sourceImage.sizeX > 0 && sourceImage.sizeY > 0 &&
                                    tagW > 0 && tagH > 0 && ImGui::BeginTooltip()) {
                                    float scale = 1.0f;
                                    if (tagW > 240 || tagH > 180) {
                                        const float sx = 240.0f / tagW, sy = 180.0f / tagH;
                                        scale = sx < sy ? sx : sy;
                                    }
                                    ImGui::Image(sourceImage.texture, ImVec2(tagW * scale, tagH * scale),
                                        ImVec2(tag.x / (float)sourceImage.sizeX, tag.y / (float)sourceImage.sizeY),
                                        ImVec2((tag.x + tagW) / (float)sourceImage.sizeX,
                                            (tag.y + tagH) / (float)sourceImage.sizeY));
                                    ImGui::Text("gr %d  x %d  y %d  w %d  h %d", tag.gr, tag.x, tag.y, tag.w, tag.h);
                                    ImGui::EndTooltip();
                                }
                            }
                        }
                        ImGui::EndCombo();
                    }
                };

                auto drawDstColorEditor = [&](int row, const char* command,
                    const char* label) {
                    if (row < 0 || row >= skinfileLines.count) return false;
                    int columns[4];
                    if (!ResolveDstArgbColumns(command, columns)) return false;

                    SKINFILELINEREAD& line =
                        ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    auto clampByte = [](int value) {
                        return value < 0 ? 0 : (value > 255 ? 255 : value);
                    };
                    // ImGui uses RGBA floats while LR2 stores A,R,G,B integers.
                    float color[4] = {
                        clampByte(line.csv.val[columns[1]]) / 255.0f,
                        clampByte(line.csv.val[columns[2]]) / 255.0f,
                        clampByte(line.csv.val[columns[3]]) / 255.0f,
                        clampByte(line.csv.val[columns[0]]) / 255.0f
                    };
                    const ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs |
                        ImGuiColorEditFlags_AlphaBar |
                        ImGuiColorEditFlags_AlphaPreviewHalf |
                        ImGuiColorEditFlags_InputRGB |
                        ImGuiColorEditFlags_Uint8;
                    const bool changed = ImGui::ColorEdit4(label, color, flags);
                    if (ImGui::IsItemActivated()) EndDstArgbEdit();

                    if (changed) {
                        const int edited[4] = {
                            (int)std::round(color[3] * 255.0f),
                            (int)std::round(color[0] * 255.0f),
                            (int)std::round(color[1] * 255.0f),
                            (int)std::round(color[2] * 255.0f)
                        };
                        ApplyDstArgbEdit(row, edited);
                    }
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
                        ImGui::SetTooltip("A %d  R %d  G %d  B %d",
                            line.csv.val[columns[0]], line.csv.val[columns[1]],
                            line.csv.val[columns[2]], line.csv.val[columns[3]]);
                    }
                    if (ImGui::IsItemDeactivatedAfterEdit()) EndDstArgbEdit();
                    return true;
                };

                auto drawObjectPropertyRow = [&](int row, int compactDstIndex) {
                    if (row < 0 || row >= skinfileLines.count) return;
                    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    const char* command = line.csv.str[0].body ? line.csv.str[0].outstr() : "";
                    const bool compactDst = compactDstIndex >= 0;
                    ImGui::PushID(row);
                    if (compactDst) {
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 2.0f));
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 2.0f));
                    }
                    if (compactDst || ImGui::CollapsingHeader(command, ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (!compactDst) drawTaggedImageSelector(row, command);
                        int maxcol = 30;
                        int dstColorColumns[4];
                        const bool hasDstColor = ResolveDstArgbColumns(command,
                            dstColorColumns);
                        for (int col = 1; col < maxcol; ++col) {
                            CSTR help = GetCommandHelp(command, col);
                            help.trimWhiteSpace();
                            if (!line.csv.str[col].body && !help.body) continue;
                            const char* label = help.body ? help.outstr() : "value";
                            if (!label || !*label) continue;
                            // In animation frames after the first DST, these values are inherited
                            // from the first row and are not used independently. Match by field
                            // name because their column positions differ between DST commands.
                            const bool isSharedDstField = _strnicmp(label, "loop", 4) == 0 ||
                                _strnicmp(label, "timer", 5) == 0 || _strnicmp(label, "$timer", 6) == 0 ||
                                _strnicmp(label, "op1", 3) == 0 || _strnicmp(label, "$op1", 4) == 0 ||
                                _strnicmp(label, "op2", 3) == 0 || _strnicmp(label, "$op2", 4) == 0 ||
                                _strnicmp(label, "op3", 3) == 0 || _strnicmp(label, "$op3", 4) == 0;
                            if (compactDstIndex > 0 && isSharedDstField) continue;
                            if (hasDstColor && col > dstColorColumns[0] &&
                                col <= dstColorColumns[3]) continue;
                            ImGui::PushID(col);
                            const bool isDstColor = hasDstColor &&
                                col == dstColorColumns[0];
                            const bool isRelativeCoordinate =
                                IsNowComboRelativeField(command, label);
                            const char* displayLabel = isDstColor ? "ARGB" :
                                (isRelativeCoordinate
                                    ? (_stricmp(label, "x") == 0
                                        ? "Offset X" : "Offset Y")
                                    : label);
                            const char* widgetLabel = displayLabel;
                            if (compactDst) {
                                if (compactDstIndex == 0) {
                                    ImGui::AlignTextToFramePadding();
                                    ImGui::TextDisabled("%s", displayLabel);
                                    ImGui::SameLine(62.0f);
                                }
                                widgetLabel = "##value";
                                ImGui::SetNextItemWidth(-FLT_MIN);
                            }
                            if (!isDstColor ||
                                !drawDstColorEditor(row, command, widgetLabel)) {
                                const int current = line.csv.val[col];
                                int selectedValue = current;
                                if (DrawCommandValueCombo(widgetLabel, command,
                                    help.body ? help.outstr() : "", current, selectedValue)) {
                                    if (selectedValue != current) EditValue(row, col, selectedValue);
                                } else {
                                    CSTR before(line.csv.str[col]);
                                    CstrInputText(widgetLabel, &line.csv.str[col], ImGuiInputTextFlags_EnterReturnsTrue);
                                    if (before.isDiff(line.csv.str[col])) {
                                        CSTR newValue(line.csv.str[col]);
                                        line.csv.str[col] = before;
                                        EditValue(row, col, newValue.outstr());
                                    }
                                }
                            }
                            if (isRelativeCoordinate &&
                                ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                                ImGui::SetTooltip("%s", NowComboCoordinateHelp());
                            ImGui::PopID();
                        }
                    }
                    if (compactDst) ImGui::PopStyleVar(2);
                    ImGui::PopID();
                };

                std::vector<int> srcRows;
                std::vector<int> dstRows;
                std::vector<int> semanticDstRows;
                std::string semanticDestinationCommand;
                bool requestAddDstFrame = false;
                bool requestRemoveDstFrame = false;
                for (std::size_t ri = 0; ri < obj.rows.size(); ++ri) {
                    const int row = obj.rows[ri];
                    if (row < 0 || row >= skinfileLines.count) continue;
                    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    const char* command = line.csv.str[0].body ? line.csv.str[0].outstr() : "";
                    if (strncmp(command, "#DST", 4) == 0)
                        dstRows.push_back(row);
                    else
                        srcRows.push_back(row);
                }

                // V0.5-V0.7 intentionally own one destination command family.
                // A later family is a variant (V0.9) and remains available in
                // Advanced LR2 without being mixed into this timeline.
                for (int row : dstRows) {
                    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    const char* command = line.csv.str[0].body ? line.csv.str[0].outstr() : "";
                    const bool hasHeight = FindCommandFieldColumn(command, "h") >= 0 ||
                        FindCommandFieldColumn(command, "size") >= 0;
                    const bool hasSemanticContract =
                        FindCommandFieldColumn(command, "time") >= 0 &&
                        FindCommandFieldColumn(command, "x") >= 0 &&
                        FindCommandFieldColumn(command, "y") >= 0 &&
                        FindCommandFieldColumn(command, "w") >= 0 && hasHeight &&
                        FindCommandFieldColumn(command, "a") >= 0 &&
                        FindCommandFieldColumn(command, "angle") >= 0 &&
                        FindCommandFieldColumn(command, "blend") >= 0;
                    if (semanticDestinationCommand.empty()) {
                        if (!hasSemanticContract) continue;
                        semanticDestinationCommand = command;
                    }
                    if (semanticDestinationCommand == command && hasSemanticContract)
                        semanticDstRows.push_back(row);
                }

                auto drawSemanticField = [&](int row, const char* label,
                    const char* fieldName, const char* fallbackField = nullptr) {
                    if (row < 0 || row >= skinfileLines.count) return false;
                    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    const char* command = line.csv.str[0].body ? line.csv.str[0].outstr() : "";
                    int column = FindCommandFieldColumn(command, fieldName);
                    if (column < 0 && fallbackField)
                        column = FindCommandFieldColumn(command, fallbackField);
                    if (column < 0) {
                        ImGui::TextDisabled("%s: unsupported by %s", label, command);
                        return false;
                    }
                    int value = line.csv.val[column];
                    ImGui::SetNextItemWidth(120.0f);
                    ImGui::PushID(row);
                    ImGui::PushID(column);
                    const bool changed = ImGui::InputInt(label, &value);
                    if (changed) EditValue(row, column, value);
                    if (IsNowComboRelativeField(command, fieldName) &&
                        ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                        ImGui::SetTooltip("%s", NowComboCoordinateHelp());
                    ImGui::PopID();
                    ImGui::PopID();
                    return true;
                };

                auto drawTimelineCell = [&](int row, const char* fieldName,
                    const char* fallbackField = nullptr) {
                    if (row < 0 || row >= skinfileLines.count) return;
                    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    const char* command = line.csv.str[0].body ? line.csv.str[0].outstr() : "";
                    int column = FindCommandFieldColumn(command, fieldName);
                    if (column < 0 && fallbackField)
                        column = FindCommandFieldColumn(command, fallbackField);
                    if (column < 0) {
                        ImGui::TextDisabled("--");
                        return;
                    }
                    int value = line.csv.val[column];
                    ImGui::PushID(row);
                    ImGui::PushID(column);
                    ImGui::SetNextItemWidth(76.0f);
                    if (ImGui::InputInt("##semantic", &value, 0, 0))
                        EditValue(row, column, value);
                    if (IsNowComboRelativeField(command, fieldName) &&
                        ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
                        ImGui::SetTooltip("%s", NowComboCoordinateHelp());
                    ImGui::PopID();
                    ImGui::PopID();
                };

                const bool hasRelativeNowComboCoordinates =
                    IsNowComboDestinationCommand(
                        semanticDestinationCommand.c_str());
                if (hasRelativeNowComboCoordinates) {
                    ImGui::TextColored(SEUI::Colors::Warning(),
                        "Relative NOWCOMBO position");
                    ImGui::SameLine();
                    SEUI::HelpMarker(NowComboCoordinateHelp());
                    ImGui::TextWrapped(
                        "Offset X/Y are added to the matching NOWJUDGE position; they are not canvas X/Y.");
                    ImGui::Separator();
                }

                if (ImGui::BeginTabBar("ObjectPropertyTabs")) {
                    if (ImGui::BeginTabItem("SRC")) {
                        ImGui::SeparatorText("basic");
                        if (srcRows.empty()) ImGui::TextDisabled("No SRC properties.");
                        for (std::size_t i = 0; i < srcRows.size(); ++i)
                            drawObjectPropertyRow(srcRows[i], -1);
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Layout")) {
                        ImGui::TextDisabled("The first destination rectangle is the static layout authority.");
                        if (semanticDstRows.empty()) {
                            ImGui::TextDisabled("No DST layout is available.");
                        } else {
                            const int firstDestinationRow = semanticDstRows.front();
                            if (ImGui::BeginTable("SemanticLayout", 2,
                                ImGuiTableFlags_SizingStretchSame)) {
                                const char* absoluteLabels[] = { "X", "Y", "Width", "Height", "Rotation", "Blend" };
                                const char* relativeLabels[] = { "Offset X", "Offset Y", "Width", "Height", "Rotation", "Blend" };
                                const char** labels = hasRelativeNowComboCoordinates
                                    ? relativeLabels : absoluteLabels;
                                const char* fields[] = { "x", "y", "w", "h", "angle", "blend" };
                                for (int field = 0; field < 6; ++field) {
                                    ImGui::TableNextColumn();
                                    drawSemanticField(firstDestinationRow, labels[field], fields[field],
                                        field == 3 ? "size" : nullptr);
                                }
                                ImGui::EndTable();
                            }
                            ImGui::Separator();
                            ImGui::TextDisabled("Canvas: drag to move all frames; drag the white handle to resize frame 0.");
                        }
                        ImGui::EndTabItem();
                    }

                    char timelineTabLabel[64];
                    snprintf(timelineTabLabel, sizeof(timelineTabLabel),
                        "Timeline (%d)###ObjectTimelineTab", (int)semanticDstRows.size());
                    if (ImGui::BeginTabItem(timelineTabLabel)) {
                        ImGui::BeginDisabled(semanticDstRows.empty());
                        if (ImGui::Button("+ Frame")) requestAddDstFrame = true;
                        ImGui::EndDisabled();
                        ImGui::SameLine();
                        ImGui::BeginDisabled(semanticDstRows.size() <= 1);
                        if (ImGui::Button("- Last Frame")) requestRemoveDstFrame = true;
                        ImGui::EndDisabled();
                        if (semanticDstRows.empty()) {
                            ImGui::TextDisabled("No animation frames are available.");
                        } else if (ImGui::BeginTable("SemanticTimeline", 9,
                            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_ScrollX | ImGuiTableFlags_SizingFixedFit)) {
                            ImGui::TableSetupScrollFreeze(1, 1);
                            const char* absoluteHeadings[] = { "Frame", "ms", "X", "Y", "W", "H", "Alpha", "Rotation", "Blend" };
                            const char* relativeHeadings[] = { "Frame", "ms", "dX", "dY", "W", "H", "Alpha", "Rotation", "Blend" };
                            const char** headings = hasRelativeNowComboCoordinates
                                ? relativeHeadings : absoluteHeadings;
                            for (int headingIndex = 0; headingIndex < 9;
                                ++headingIndex) {
                                const char* heading = headings[headingIndex];
                                ImGui::TableSetupColumn(heading, ImGuiTableColumnFlags_WidthFixed,
                                    !strcmp(heading, "Frame") ? 62.0f : 92.0f);
                            }
                            ImGui::TableHeadersRow();
                            for (int frameIndex = 0; frameIndex < (int)semanticDstRows.size(); ++frameIndex) {
                                ImGui::TableNextRow();
                                ImGui::TableSetColumnIndex(0);
                                ImGui::Text("%d%s", frameIndex,
                                    frameIndex == 0 ? " (layout)" : "");
                                const char* fields[] = { "time", "x", "y", "w", "h", "a", "angle", "blend" };
                                for (int field = 0; field < 8; ++field) {
                                    ImGui::TableSetColumnIndex(field + 1);
                                    drawTimelineCell(semanticDstRows[frameIndex], fields[field],
                                        field == 4 ? "size" : nullptr);
                                }
                            }
                            ImGui::EndTable();
                        }
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Conditions")) {
                        ImGui::TextDisabled("All OP terms must pass. Unknown/custom OP values remain raw.");
                        if (semanticDstRows.empty()) {
                            ImGui::TextDisabled("No destination condition is available.");
                        } else {
                            const int row = semanticDstRows.front();
                            SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                            const char* command = line.csv.str[0].body ? line.csv.str[0].outstr() : "";
                            const int timerColumn = FindCommandFieldColumn(command, "timer");
                            if (timerColumn >= 0) {
                                const int currentTimer = line.csv.val[timerColumn];
                                int timerValue = currentTimer;
                                CSTR timerHelp = GetCommandHelp(command, timerColumn);
                                timerHelp.trimWhiteSpace();
                                ImGui::SetNextItemWidth(300.0f);
                                if (DrawCommandValueCombo("Trigger", command,
                                    timerHelp.body ? timerHelp.outstr() : "$timer",
                                    currentTimer, timerValue) && timerValue != currentTimer)
                                    EditValue(row, timerColumn, timerValue);
                                const char* triggerName = timerName((unsigned)std::abs(timerValue));
                                if (!triggerName[0])
                                    ImGui::TextColored(SEUI::Colors::Warning(),
                                        "Raw LR2 TIMER: %d", timerValue);
                                else ImGui::TextDisabled("Timer: %s", triggerName);
                            }
                            drawSemanticField(row, "Loop (ms)", "loop");
                            bool allConditionsPass = true;
                            int conditionCount = 0;
                            for (int conditionIndex = 0; conditionIndex < 3; ++conditionIndex) {
                                const std::string fieldName = "op" + std::to_string(conditionIndex + 1);
                                const int column = FindCommandFieldColumn(command, fieldName.c_str());
                                if (column < 0) continue;
                                const int currentOption = line.csv.val[column];
                                int option = currentOption;
                                CSTR optionHelp = GetCommandHelp(command, column);
                                optionHelp.trimWhiteSpace();
                                char label[32];
                                snprintf(label, sizeof(label), "Condition %d", conditionIndex + 1);
                                ImGui::SetNextItemWidth(300.0f);
                                if (DrawCommandValueCombo(label, command,
                                    optionHelp.body ? optionHelp.outstr() : "$op",
                                    currentOption, option) && option != currentOption)
                                    EditValue(row, column, option);
                                if (option == 0) continue;
                                ++conditionCount;
                                const int absoluteOption = std::abs(option);
                                const char* optionName = dstName((unsigned)absoluteOption);
                                const bool isRaw = absoluteOption >= 900 || !optionName[0];
                                const bool passes = GetOptionFlag_dst(&g, option);
                                allConditionsPass = allConditionsPass && passes;
                                if (isRaw) {
                                    ImGui::TextColored(SEUI::Colors::Warning(),
                                        "%s Raw LR2 OP: %d  [%s]", passes ? "PASS" : "FAIL",
                                        option, optionName[0] ? optionName : "unknown");
                                } else {
                                    ImGui::TextColored(passes ? SEUI::Colors::Success() : SEUI::Colors::Danger(),
                                        "%s  %s%s", passes ? "PASS" : "FAIL",
                                        option < 0 ? "NOT " : "", optionName);
                                }
                            }
                            ImGui::Separator();
                            ImGui::TextColored(allConditionsPass ? SEUI::Colors::Success() : SEUI::Colors::Danger(),
                                "Simulator: %s (%d active condition%s)",
                                allConditionsPass ? "VISIBLE" : "HIDDEN", conditionCount,
                                conditionCount == 1 ? "" : "s");
                        }
                        ImGui::EndTabItem();
                    }

                    char dstTabLabel[64];
                    // Keep the tab identity stable while its visible count
                    // changes, so +DST/-DST does not switch back to SRC.
                    snprintf(dstTabLabel, sizeof(dstTabLabel), "Advanced LR2###ObjectDstTab");
                    if (ImGui::BeginTabItem(dstTabLabel)) {
                        ImGui::TextDisabled("Raw LR2 fields for compatibility and unsupported commands.");
                        if (hasRelativeNowComboCoordinates) {
                            ImGui::SameLine();
                            SEUI::HelpMarker(NowComboCoordinateHelp());
                        }
                        ImGui::SeparatorText(dstRows.size() > 1 ? "animation frames" : "basic");
                        if (dstRows.empty()) {
                            ImGui::TextDisabled("No DST properties.");
                        } else if (dstRows.size() == 1) {
                            drawObjectPropertyRow(dstRows[0], -1);
                        } else {
                            // Most DST values are short integers. Keep animation
                            // columns dense so several frames fit on screen.
                            const float labelWidth = 50.0f;
                            const float valueWidth = 82.0f;
                            const float cellPaddingWidth = ImGui::GetStyle().CellPadding.x * 2.0f;
                            const float innerWidth = labelWidth + valueWidth * (float)dstRows.size() +
                                cellPaddingWidth * ((float)dstRows.size() + 1.0f) + 16.0f;
                            const ImGuiTableFlags flags = ImGuiTableFlags_Borders |
                                ImGuiTableFlags_ScrollX | ImGuiTableFlags_SizingFixedFit |
                                ImGuiTableFlags_RowBg;
                            if (ImGui::BeginTable("DSTHorizontalRows", (int)dstRows.size() + 1, flags,
                                ImVec2(0.0f, 0.0f), innerWidth)) {
                                // Keep property names and the authoritative first DST visible;
                                // only animation frames after DST 1 scroll horizontally.
                                ImGui::TableSetupScrollFreeze(2, 1);
                                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, labelWidth);
                                for (int i = 0; i < (int)dstRows.size(); ++i) {
                                    char columnLabel[32];
                                    snprintf(columnLabel, sizeof(columnLabel), "DST %d", i + 1);
                                    ImGui::TableSetupColumn(columnLabel, ImGuiTableColumnFlags_WidthFixed, valueWidth);
                                }
                                ImGui::TableHeadersRow();

                                SKINFILELINEREAD& firstDst = ((SKINFILELINEREAD*)skinfileLines.data)[dstRows[0]];
                                const char* firstCommand = firstDst.csv.str[0].body ? firstDst.csv.str[0].outstr() : "";
                                int firstColorColumns[4];
                                const bool hasDstColor = ResolveDstArgbColumns(
                                    firstCommand, firstColorColumns);
                                for (int col = 1; col < 30; ++col) {
                                    CSTR firstHelp = GetCommandHelp(firstCommand, col);
                                    firstHelp.trimWhiteSpace();
                                    // Multi-DST rows follow the documented command fields only.
                                    // Ignore trailing CSV values and CR/LF-only helper entries.
                                    if (!firstHelp.body) continue;
                                    const char* rowLabel = firstHelp.outstr();
                                    if (!rowLabel || !*rowLabel) continue;
                                    if (hasDstColor && col > firstColorColumns[0] &&
                                        col <= firstColorColumns[3]) continue;
                                    const bool isDstColor = hasDstColor &&
                                        col == firstColorColumns[0];
                                    if (isDstColor) rowLabel = "ARGB";

                                    const bool sharedField = _strnicmp(rowLabel, "loop", 4) == 0 ||
                                        _strnicmp(rowLabel, "timer", 5) == 0 || _strnicmp(rowLabel, "$timer", 6) == 0 ||
                                        _strnicmp(rowLabel, "op1", 3) == 0 || _strnicmp(rowLabel, "$op1", 4) == 0 ||
                                        _strnicmp(rowLabel, "op2", 3) == 0 || _strnicmp(rowLabel, "$op2", 4) == 0 ||
                                        _strnicmp(rowLabel, "op3", 3) == 0 || _strnicmp(rowLabel, "$op3", 4) == 0;

                                    ImGui::TableNextRow();
                                    ImGui::TableSetColumnIndex(0);
                                    ImGui::AlignTextToFramePadding();
                                    ImGui::TextDisabled("%s", rowLabel);

                                    for (int i = 0; i < (int)dstRows.size(); ++i) {
                                        ImGui::TableSetColumnIndex(i + 1);
                                        if (i > 0 && sharedField) continue;

                                        const int row = dstRows[i];
                                        SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                                        CSTR help = GetCommandHelp(line.csv.str[0].outstr(), col);
                                        help.trimWhiteSpace();
                                        ImGui::PushID(row);
                                        ImGui::PushID(col);
                                        ImGui::SetNextItemWidth(-FLT_MIN);
                                        if (!isDstColor || !drawDstColorEditor(row,
                                            line.csv.str[0].outstr(), "##value")) {
                                            const int current = line.csv.val[col];
                                            int selectedValue = current;
                                            const SECommandValueKind valueKind = GetCommandValueKind(
                                                line.csv.str[0].outstr(), help.body ? help.outstr() : "");
                                            if (DrawCommandValueCombo("##value", line.csv.str[0].outstr(),
                                                help.body ? help.outstr() : "", current, selectedValue)) {
                                                if (selectedValue != current) EditValue(row, col, selectedValue);
                                                if ((valueKind == SE_VALUE_OPTION || valueKind == SE_VALUE_TIMER) &&
                                                    ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
                                                    const char* valueName = GetCommandValueName(valueKind, current);
                                                    ImGui::SetTooltip("%03d:%s", current, valueName ? valueName : "");
                                                }
                                            } else {
                                                CSTR before(line.csv.str[col]);
                                                CstrInputText("##value", &line.csv.str[col], ImGuiInputTextFlags_EnterReturnsTrue);
                                                if (before.isDiff(line.csv.str[col])) {
                                                    CSTR newValue(line.csv.str[col]);
                                                    line.csv.str[col] = before;
                                                    EditValue(row, col, newValue.outstr());
                                                }
                                            }
                                        }
                                        ImGui::PopID();
                                        ImGui::PopID();
                                    }
                                }
                                ImGui::EndTable();
                            }
                        }
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }

                // Apply structural edits after all widgets referencing the
                // current rows have been submitted. InsertLine/DeleteLine
                // update source-row indices and are already integrated with
                // Ctrl+Z history.
                const std::string editedObjectId = obj.editorId;
                const int editedObjectGroup = obj.group;
                const int editedObjectAnchorRow = obj.rows.empty() ? -1 : obj.rows.front();
                auto restoreEditedObjectSelection = [&]() {
                    const std::vector<SEObjectInstance>& rebuilt = objectEditorModel.Objects();
                    int restoredModel = -1;
                    for (int modelIndex = 0; modelIndex < (int)rebuilt.size(); ++modelIndex) {
                        const SEObjectInstance& candidate = rebuilt[modelIndex];
                        if (!editedObjectId.empty() && candidate.editorId == editedObjectId) {
                            restoredModel = modelIndex;
                            break;
                        }
                        if (editedObjectId.empty() && candidate.group == editedObjectGroup &&
                            std::find(candidate.rows.begin(), candidate.rows.end(), editedObjectAnchorRow) !=
                            candidate.rows.end()) {
                            restoredModel = modelIndex;
                            break;
                        }
                    }
                    if (restoredModel >= 0) {
                        SetObjectSelection(std::vector<int>(1, restoredModel),
                            restoredModel, restoredModel, true);
                    }
                };
                if (requestAddDstFrame && !semanticDstRows.empty()) {
                    const int sourceRow = semanticDstRows.back();
                    const int insertAt = sourceRow + 1;
                    SKINFILELINEREAD& source = ((SKINFILELINEREAD*)skinfileLines.data)[sourceRow];
                    CSTR duplicatedLine(source.line);
                    CSTR sourceOwner(source.filename);
                    const int sourceFileLine = source.num;
                    const int sourceIfgroup = source.ifgroup;
                    const int sourceObjID = source.objID;
                    const int sourceObjType = source.objType;
                    const int sourceObjInTypeID = source.objInTypeID;
                    if (InsertLine(insertAt) == 0) {
                        // Keep the insertion as the single undo record. The
                        // content initialization is part of that insertion.
                        applyingHistory = true;
                        CSTR placeholder(((SKINFILELINEREAD*)skinfileLines.data)[insertAt].line);
                        EditLine(insertAt, placeholder, duplicatedLine);
                        applyingHistory = false;
                        SKINFILELINEREAD& inserted =
                            ((SKINFILELINEREAD*)skinfileLines.data)[insertAt];
                        inserted.filename.assign(sourceOwner.body ? sourceOwner.outstr() : mainpath);
                        inserted.num = sourceFileLine;
                        inserted.ifgroup = sourceIfgroup;
                        inserted.isDST = true;
                        inserted.isSRC = false;
                        inserted.isOther = false;
                        inserted.objID = sourceObjID;
                        inserted.objType = sourceObjType;
                        inserted.objInTypeID = sourceObjInTypeID;
                        RebuildObjectModel();
                        restoreEditedObjectSelection();
                        previewReloadPending = true;
                        previewReloadRequestedAt = GetTickCount64();
                    }
                } else if (requestRemoveDstFrame && semanticDstRows.size() > 1) {
                    if (DeleteLine(semanticDstRows.back()) == 0) {
                        RebuildObjectModel();
                        restoreEditedObjectSelection();
                        previewReloadPending = true;
                        previewReloadRequestedAt = GetTickCount64();
                    }
                }
            }
        } else {
            ImGui::Text("No object.");
        }
    }
    ImGui::EndChild();

    // Name insertion can shift every following CSV row. Apply it only after
    // all Inspector widgets for this frame have released their row references.
    if (renameObjectModelIndex >= 0)
        SetObjectName(renameObjectModelIndex, renamedObjectName.c_str());

    ImGui::End();
    return 0;
}
