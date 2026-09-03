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
#include "seLocalization.h"
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
int WORKSPACE::drawObjectBrowser() {
    char browserTitle[128];
    FormatSEUIWindowTitle(browserTitle, sizeof(browserTitle), SEUIWindowId::ObjectBrowser, num);

    if (object_editor_select_request >= 0 && wObjectBrowser)
        ImGui::SetNextWindowFocus();
    const bool browserWasBegun = wObjectBrowser;
    const bool drawBrowser = browserWasBegun ? ImGui::Begin(browserTitle, &wObjectBrowser) : false;

    if (objectEditorModel.Groups().empty()) {
        if (drawBrowser) ImGui::Text("skinObjGroup.txt not loaded.");
        if (browserWasBegun) ImGui::End();
        return 0;
    }
    if (!loaded || skinfileLines.count <= 0) {
        if (drawBrowser) ImGui::Text("No skin loaded.");
        if (browserWasBegun) ImGui::End();
        return 0;
    }

    if (objectEditorLastLineCount != skinfileLines.count) {
        RebuildObjectModel();
        selected_object_editor = 0;
    }

    const std::vector<SEObjectGroupDef>& groups = objectEditorModel.Groups();
    if (selected_object_group >= (int)groups.size()) selected_object_group = -1;

    auto formatConditionHeader = [](SKINFILELINEREAD& row) {
        return FormatObjectConditionHeader(row);
    };

    const int requestedObjectModel = object_editor_select_request;
    if (requestedObjectModel >= 0) {
        // A Preview hit must remain reachable even when the Object Editor was
        // showing a type/user-group/search filter that excludes it.
        selected_object_group = -1;
        selected_user_object_group = -1;
        objectBrowserActiveOnly = false;
        objectSearch[0] = '\0';
    }
    // Object Browser: filters on top, condition/object list below.
    if (drawBrowser) {
        // Image Manager is drawn before Object Browser, so it consumes the
        // previous frame's hover and this frame refreshes or clears it.
        SetImageManagerHoveredObject(-1, ImGui::GetFrameCount());
        const float filterPanelHeight = ImGui::GetFrameHeightWithSpacing() * 4.0f +
            ImGui::GetStyle().WindowPadding.y * 2.0f + 18.0f;
        const bool drawFilters = ImGui::BeginChild("ObjectFilters",
            ImVec2(0, filterPanelHeight), true,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        if (drawFilters) {
        ImGui::TextDisabled("Type");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        const char* typePreview = selected_user_object_group < 0 && selected_object_group >= 0
            ? groups[selected_object_group].name.c_str() : "All Objects";
        if (ImGui::BeginCombo("##ObjectTypeFilter", typePreview)) {
            if (ImGui::Selectable("All Objects", selected_user_object_group < 0 && selected_object_group < 0)) {
                selected_object_group = -1;
                selected_user_object_group = -1;
                selected_object_editor = 0;
            }
            for (int gidx = 0; gidx < (int)groups.size(); ++gidx) {
                std::vector<int> objs = objectEditorModel.ObjectsForGroup(gidx);
                if (objs.empty()) continue;
                char label[128];
                snprintf(label, sizeof(label), "%s (%d)", groups[gidx].name.c_str(), (int)objs.size());
                if (ImGui::Selectable(label, selected_user_object_group < 0 && selected_object_group == gidx)) {
                    selected_object_group = gidx;
                    selected_user_object_group = -1;
                    selected_object_editor = 0;
                }
            }
            ImGui::EndCombo();
        }

        const std::vector<SEUserObjectGroup>& userGroups = objectEditorModel.UserGroups();
        ImGui::TextDisabled("Group");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        const std::string groupPreviewUtf8 = selected_user_object_group >= 0 &&
            selected_user_object_group < (int)userGroups.size()
            ? Cp932ToUtf8(userGroups[selected_user_object_group].name.c_str()) : "No My Group";
        if (ImGui::BeginCombo("##ObjectUserGroupFilter", groupPreviewUtf8.c_str())) {
            if (ImGui::Selectable("No My Group", selected_user_object_group < 0))
                selected_user_object_group = -1;
            for (int gidx = 0; gidx < (int)userGroups.size(); ++gidx) {
                std::vector<int> members = objectEditorModel.ObjectsForUserGroup(gidx);
                char label[160];
                const std::string nameUtf8 = Cp932ToUtf8(userGroups[gidx].name.c_str());
                snprintf(label, sizeof(label), "%s (%d)##user%d", nameUtf8.c_str(),
                    (int)members.size(), gidx);
                if (ImGui::Selectable(label, selected_user_object_group == gidx)) {
                    selected_user_object_group = gidx;
                    selected_object_editor = 0;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputTextWithHint("##ObjectSearch", "Search objects...", objectSearch, sizeof(objectSearch));
        ImGui::Checkbox("Active objects only", &objectBrowserActiveOnly);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal))
            ImGui::SetTooltip("Show objects whose IF branch and DST options are currently enabled.");

        if (requestCreateGroupPopup) {
            ImGui::OpenPopup("Create Object Group");
            requestCreateGroupPopup = false;
        }
        if (ImGui::BeginPopupModal("Create Object Group", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputText("Group name", newObjectGroupName, sizeof(newObjectGroupName));
            if (ImGui::Button("Create")) {
                struct PendingId { int row; int sourceNum; std::string owner; std::string id; };
                std::vector<PendingId> pendingIds;
                std::vector<std::string> memberIds;
                const std::vector<SEObjectInstance>& allObjects = objectEditorModel.Objects();
                const unsigned long long stamp = GetTickCount64();
                for (int modelIndex : preview_selected_object_model_indices) {
                    if (modelIndex < 0 || modelIndex >= (int)allObjects.size()) continue;
                    const SEObjectInstance& object = allObjects[modelIndex];
                    std::string id = object.editorId;
                    if (id.empty() && !object.rows.empty()) {
                        char generated[64];
                        snprintf(generated, sizeof(generated), "obj_%08llX_%04d", stamp, modelIndex);
                        id = generated;
                        const int firstRow = object.rows.front();
                        SKINFILELINEREAD& source = ((SKINFILELINEREAD*)skinfileLines.data)[firstRow];
                        PendingId pending;
                        pending.row = firstRow;
                        pending.sourceNum = source.num;
                        pending.owner = source.filename.body ? source.filename.outstr() : mainpath;
                        pending.id = id;
                        pendingIds.push_back(pending);
                    }
                    if (!id.empty()) memberIds.push_back(id);
                }

                std::sort(pendingIds.begin(), pendingIds.end(), [](const PendingId& a, const PendingId& b) {
                    return a.row > b.row;
                });
                auto insertMetaLine = [&](int position, const char* text, const char* owner, int sourceNum) {
                    InsertLine(position);
                    SKINFILELINEREAD& inserted = ((SKINFILELINEREAD*)skinfileLines.data)[position];
                    CSTR oldLine(inserted.line);
                    EditLine(position, oldLine, CSTR(text));
                    inserted.filename.assign(owner && *owner ? owner : mainpath);
                    inserted.num = sourceNum;
                    inserted.numTotal = position;
                };
                for (const PendingId& pending : pendingIds) {
                    std::string line = "$SE_OBJECT_ID," + pending.id;
                    insertMetaLine(pending.row, line.c_str(), pending.owner.c_str(), pending.sourceNum);
                }

                const int groupNumber = (int)userGroups.size();
                std::string begin = std::string("$SE_GROUP_BEGIN,") +
                    (newObjectGroupName[0] ? newObjectGroupName : "New Group");
                insertMetaLine(skinfileLines.count, begin.c_str(), mainpath, skinfileLines.count);
                for (const std::string& id : memberIds) {
                    std::string member = "$SE_GROUP_MEMBER," + id;
                    insertMetaLine(skinfileLines.count, member.c_str(), mainpath, skinfileLines.count);
                }
                insertMetaLine(skinfileLines.count, "$SE_GROUP_END", mainpath, skinfileLines.count);

                RebuildObjectModel();
                selected_user_object_group = groupNumber;
                selected_object_editor = 0;
                preview_selected_object_model_indices.clear();
                const std::vector<SEObjectInstance>& rebuiltObjects = objectEditorModel.Objects();
                for (int objectIndex = 0; objectIndex < (int)rebuiltObjects.size(); ++objectIndex) {
                    if (std::find(memberIds.begin(), memberIds.end(), rebuiltObjects[objectIndex].editorId) != memberIds.end())
                        preview_selected_object_model_indices.push_back(objectIndex);
                }
                preview_selected_object_model_index = preview_selected_object_model_indices.empty()
                    ? -1 : preview_selected_object_model_indices.front();
                SetObjectSelection(preview_selected_object_model_indices,
                    preview_selected_object_model_index,
                    preview_selected_object_model_index, false);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        }
        ImGui::EndChild();
    }

    std::vector<int> groupObjects;
    if (selected_user_object_group >= 0)
        groupObjects = objectEditorModel.ObjectsForUserGroup(selected_user_object_group);
    else if (selected_object_group >= 0)
        groupObjects = objectEditorModel.ObjectsForGroup(selected_object_group);
    else {
        const std::vector<SEObjectInstance>& allObjects = objectEditorModel.Objects();
        for (int i = 0; i < (int)allObjects.size(); ++i) groupObjects.push_back(i);
    }

    auto syncDstSelectionForObject = [&](int objectModelIndex, bool focusPreview) {
        const std::vector<SEObjectInstance>& allObjects = objectEditorModel.Objects();
        if (objectModelIndex < 0 || objectModelIndex >= (int)allObjects.size()) return;
        const SEObjectInstance& object = allObjects[objectModelIndex];

        int firstDst = -1;
        int enabledDst = -1;
        bool currentDstBelongs = false;
        for (int dstIndex = 0; dstIndex < arr_DST.count; ++dstIndex) {
            DST& candidate = ((DST*)arr_DST.data)[dstIndex];
            if (std::find(object.rows.begin(), object.rows.end(), candidate.declare) == object.rows.end())
                continue;
            if (firstDst < 0) firstDst = dstIndex;
            if (dstIndex == selected_dst) currentDstBelongs = true;
            if (enabledDst < 0 && GetOptionFlag_dst(&g, candidate.op1) &&
                GetOptionFlag_dst(&g, candidate.op2) && GetOptionFlag_dst(&g, candidate.op3))
                enabledDst = dstIndex;
        }
        if (!currentDstBelongs) {
            const int matchingDst = enabledDst >= 0 ? enabledDst : firstDst;
            if (matchingDst >= 0) {
                selected_dst = matchingDst;
                dst_view_scroll_request = matchingDst;
                SetTimeLapse(1, &g.timer1);
            }
        } else {
            dst_view_scroll_request = selected_dst;
        }
        if (focusPreview) {
            wPreview = true;
            char previewWindowTitle[64];
            FormatSEUIWindowTitle(previewWindowTitle,
                sizeof(previewWindowTitle), SEUIWindowId::Preview, num);
            ImGui::SetWindowFocus(previewWindowTitle);
        }
    };

    if (requestedObjectModel >= 0) {
        const std::vector<int>::const_iterator requestedIt = std::find(
            groupObjects.begin(), groupObjects.end(), requestedObjectModel);
        if (requestedIt != groupObjects.end())
            selected_object_editor = (int)(requestedIt - groupObjects.begin());
        syncDstSelectionForObject(requestedObjectModel, false);
        object_editor_select_request = -1;
    }

    if (objectSearch[0]) {
        auto containsText = [](const char* text, const char* query) {
            if (!text || !query || !*query) return true;
            const size_t queryLength = strlen(query);
            for (const char* cursor = text; *cursor; ++cursor)
                if (_strnicmp(cursor, query, queryLength) == 0) return true;
            return false;
        };
        const std::vector<SEObjectInstance>& allObjects = objectEditorModel.Objects();
        groupObjects.erase(std::remove_if(groupObjects.begin(), groupObjects.end(), [&](int objectIndex) {
            if (objectIndex < 0 || objectIndex >= (int)allObjects.size()) return true;
            const SEObjectInstance& object = allObjects[objectIndex];
            if (containsText(object.name.c_str(), objectSearch) ||
                containsText(object.editorId.c_str(), objectSearch)) return false;
            const SEObjectGroupDef* def = objectEditorModel.Group(object.group);
            if (def && containsText(def->name.c_str(), objectSearch)) return false;
            for (int row : object.rows) {
                if (row < 0 || row >= skinfileLines.count) continue;
                SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                if (line.line.body && containsText(line.line.outstr(), objectSearch)) return false;
            }
            return true;
        }), groupObjects.end());
    }
    int contextCopyModel = -1;
    int contextPasteModel = -1;
    int contextDuplicateModel = -1;
    int contextDeleteModel = -1;

    // Middle: control-flow is metadata, never an Object command.
    // Sibling #IF/#ELSEIF/#ELSE branches are displayed together.
    if (drawBrowser) {
        const bool drawObjectList = ImGui::BeginChild("ObjectList", ImVec2(0, 0), true);
        if (drawObjectList) {
        struct BranchBlock {
            int ifgroup;
            int order;
            std::string label;
            std::vector<int> localObjectIndices;
        };
        struct ConditionBlock {
            int rootIfgroup;
            int depth;
            std::string label;
            std::vector<BranchBlock> branches;
        };
        std::vector<ConditionBlock> conditions;
        std::map<int, int> conditionIndexByRoot;
        std::map<std::pair<int, int>, int> branchIndexByCondition;

        std::vector<std::string> conditionHeaders(
            (std::max)(0, arr_ifunit.count));
        std::vector<int> conditionOrders((std::max)(0, arr_ifunit.count), 0);
        for (int group = 1; group < arr_ifunit.count; ++group)
            conditionOrders[group] = ((IFUNIT*)arr_ifunit.data)[group].order;
        for (int row = 0; row < skinfileLines.count; ++row) {
            SKINFILELINEREAD& header =
                ((SKINFILELINEREAD*)skinfileLines.data)[row];
            if (!header.isIfGroupHead || header.ifgroup <= 0 ||
                header.ifgroup >= arr_ifunit.count ||
                !header.csv.str[0].body ||
                !conditionHeaders[header.ifgroup].empty()) continue;
            conditionHeaders[header.ifgroup] = formatConditionHeader(header);
        }

        std::vector<int> conditionRoots((std::max)(0, arr_ifunit.count), 0);
        std::map<std::pair<int, int>, int> latestRootByParentDepth;
        for (int group = 1; group < arr_ifunit.count; ++group) {
            IFUNIT& unit = ((IFUNIT*)arr_ifunit.data)[group];
            const std::pair<int, int> chainKey(unit.parentID, unit.depth);
            if (unit.order == 0) {
                conditionRoots[group] = group;
                latestRootByParentDepth[chainKey] = group;
            } else {
                const std::map<std::pair<int, int>, int>::const_iterator root =
                    latestRootByParentDepth.find(chainKey);
                conditionRoots[group] = root == latestRootByParentDepth.end()
                    ? group : root->second;
            }
        }

        auto getHeader = [&](int ifgroup, std::string& label, int& order) -> bool {
            label = "";
            order = 0;
            if (ifgroup <= 0 || ifgroup >= arr_ifunit.count) return false;
            order = conditionOrders[ifgroup];
            label = conditionHeaders[ifgroup];
            return !label.empty();
        };

        auto getRootIfgroup = [&](int ifgroup) -> int {
            if (ifgroup <= 0 || ifgroup >= arr_ifunit.count) return 0;
            return conditionRoots[ifgroup];
        };

        // Cache the exact active branch state. A branch is active only when
        // its parent is active, its own IF/ELSEIF matches, and no earlier
        // sibling branch has already matched.
        const unsigned long long statusNow = GetTickCount64();
        const bool refreshBranchStatus =
            objectStatusCacheLineCount != skinfileLines.count ||
            objectStatusCacheIfCount != arr_ifunit.count ||
            statusNow - objectStatusCacheAt >= 200;
        if (refreshBranchStatus) {
            std::vector<bool> branchMatches(arr_ifunit.count, true);
            objectBranchActive.assign(arr_ifunit.count, false);
            if (!objectBranchActive.empty()) objectBranchActive[0] = true;
            for (int row = 0; row < skinfileLines.count; ++row) {
                SKINFILELINEREAD& header = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                if (!header.isIfGroupHead || header.ifgroup <= 0 ||
                    header.ifgroup >= arr_ifunit.count || !header.csv.str[0].body) continue;
                const char* command = header.csv.str[0].outstr();
                if (!strcmp(command, "#ELSE")) {
                    branchMatches[header.ifgroup] = true;
                } else {
                    bool matches = !strcmp(command, "#IF") || !strcmp(command, "#ELSEIF");
                    for (int col = 1; matches && col <= 10; ++col) {
                        const int option = header.csv.val[col];
                        const int optionId = option < 0 ? -option : option;
                        if (optionId > 999) {
                            matches = false;
                        } else {
                            const bool optionEnabled = optionId == 0 || g.skstruct.op[optionId] != 0;
                            if ((option >= 0) != optionEnabled) matches = false;
                        }
                    }
                    branchMatches[header.ifgroup] = matches;
                }
            }
            std::map<int, bool> earlierBranchTaken;
            for (int group = 1; group < arr_ifunit.count; ++group) {
                IFUNIT& unit = ((IFUNIT*)arr_ifunit.data)[group];
                const int chainRoot = getRootIfgroup(group);
                const bool earlierTaken = earlierBranchTaken[chainRoot];
                const bool parentActive = unit.parentID >= 0 &&
                    unit.parentID < (int)objectBranchActive.size()
                    ? objectBranchActive[unit.parentID] : false;
                objectBranchActive[group] = parentActive && !earlierTaken && branchMatches[group];
                if (branchMatches[group]) earlierBranchTaken[chainRoot] = true;
            }
            objectStatusCacheLineCount = skinfileLines.count;
            objectStatusCacheIfCount = arr_ifunit.count;
            objectStatusCacheAt = statusNow;
        }

        // Command metadata never changes while the helper definition is
        // loaded. Cache the $op column positions instead of linearly searching
        // the whole help table for every DST, every UI frame.
        static std::map<std::string, std::vector<int> > dstOptionColumns;
        static std::map<std::string, std::vector<int> > commandTimerColumns;
        auto optionColumnsFor = [&](const char* command) -> const std::vector<int>& {
            const std::string key = command ? command : "";
            std::map<std::string, std::vector<int> >::iterator found = dstOptionColumns.find(key);
            if (found != dstOptionColumns.end()) return found->second;
            std::vector<int> columns;
            for (int col = 1; col < 30; ++col) {
                CSTR optionHelp = GetCommandHelp(key.c_str(), col);
                optionHelp.trimWhiteSpace();
                if (optionHelp.left(3).isSame("$op")) columns.push_back(col);
            }
            return dstOptionColumns.insert(std::make_pair(key, columns)).first->second;
        };
        auto timerColumnsFor = [&](const char* command) -> const std::vector<int>& {
            const std::string key = command ? command : "";
            std::map<std::string, std::vector<int> >::iterator found = commandTimerColumns.find(key);
            if (found != commandTimerColumns.end()) return found->second;
            std::vector<int> columns;
            for (int col = 1; col < 30; ++col) {
                CSTR help = GetCommandHelp(key.c_str(), col);
                help.trimWhiteSpace();
                if (help.left(6).isSame("$timer")) columns.push_back(col);
            }
            return commandTimerColumns.insert(std::make_pair(key, columns)).first->second;
        };

        // Use the same definition as the green/red Object Browser rows:
        // at least one DST, every DST option enabled, and the owning IF branch active.
        auto objectIsActive = [&](const SEObjectInstance& object) -> bool {
            bool hasDst = false;
            bool allDstEnabled = true;
            for (int row : object.rows) {
                if (row < 0 || row >= skinfileLines.count) continue;
                SKINFILELINEREAD& dstRow = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                if (!dstRow.csv.str[0].body) continue;
                const char* command = dstRow.csv.str[0].outstr();
                if (strncmp(command, "#DST", 4) != 0) continue;
                hasDst = true;
                const std::vector<int>& optionColumns = optionColumnsFor(command);
                for (int col : optionColumns) {
                    if (!GetOptionFlag_dst(&g, dstRow.csv.val[col])) {
                        allDstEnabled = false;
                        break;
                    }
                }
                if (!allDstEnabled) break;
            }

            int ifgroup = object.ifgroup;
            if (ifgroup == 0 && !object.rows.empty())
                ifgroup = ((SKINFILELINEREAD*)skinfileLines.data)[object.rows.front()].ifgroup;
            const bool conditionActive = ifgroup <= 0 ||
                (ifgroup < (int)objectBranchActive.size() && objectBranchActive[ifgroup]);
            return hasDst && allDstEnabled && conditionActive;
        };

        for (int oi = 0; oi < (int)groupObjects.size(); ++oi) {
            const SEObjectInstance& o = objectEditorModel.Objects()[groupObjects[oi]];
            if (objectBrowserActiveOnly && !objectIsActive(o)) continue;
            int ifgroup = o.ifgroup;
            if (ifgroup == 0 && !o.rows.empty())
                ifgroup = ((SKINFILELINEREAD*)skinfileLines.data)[o.rows[0]].ifgroup;

            if (ifgroup == 0) {
                int ci = -1;
                const std::map<int, int>::const_iterator existingCondition =
                    conditionIndexByRoot.find(0);
                if (existingCondition != conditionIndexByRoot.end())
                    ci = existingCondition->second;
                if (ci < 0) {
                    ConditionBlock c;
                    c.rootIfgroup = 0; c.depth = 0; c.label = "ALWAYS";
                    conditions.push_back(c); ci = (int)conditions.size() - 1;
                    conditionIndexByRoot[0] = ci;
                }
                if (conditions[ci].branches.empty()) {
                    BranchBlock b; b.ifgroup = 0; b.order = 0; b.label = "ALWAYS";
                    conditions[ci].branches.push_back(b);
                }
                conditions[ci].branches[0].localObjectIndices.push_back(oi);
                continue;
            }

            int root = getRootIfgroup(ifgroup);
            int ci = -1;
            const std::map<int, int>::const_iterator existingCondition =
                conditionIndexByRoot.find(root);
            if (existingCondition != conditionIndexByRoot.end())
                ci = existingCondition->second;
            if (ci < 0) {
                ConditionBlock c;
                c.rootIfgroup = root;
                c.depth = ((IFUNIT*)arr_ifunit.data)[root].depth;
                int dummyOrder = 0;
                getHeader(root, c.label, dummyOrder);
                if (c.label.empty()) c.label = "#IF";
                conditions.push_back(c); ci = (int)conditions.size() - 1;
                conditionIndexByRoot[root] = ci;
            }

            int bi = -1;
            const std::pair<int, int> branchKey(root, ifgroup);
            const std::map<std::pair<int, int>, int>::const_iterator
                existingBranch = branchIndexByCondition.find(branchKey);
            if (existingBranch != branchIndexByCondition.end())
                bi = existingBranch->second;
            if (bi < 0) {
                BranchBlock b;
                b.ifgroup = ifgroup;
                getHeader(ifgroup, b.label, b.order);
                if (b.label.empty()) b.label = (b.order == 0) ? "#IF" : "#ELSE";
                conditions[ci].branches.push_back(b); bi = (int)conditions[ci].branches.size() - 1;
                branchIndexByCondition[branchKey] = bi;
            }
            conditions[ci].branches[bi].localObjectIndices.push_back(oi);
        }

        // A condition containing only nested conditions has no direct object,
        // so the object-driven pass above cannot discover it. Add every
        // missing ancestor chain to keep the navigator rooted at the actual
        // top-level #IF instead of starting midway through the source tree.
        std::vector<int> visibleConditionRoots;
        for (const ConditionBlock& condition : conditions)
            if (condition.rootIfgroup > 0) visibleConditionRoots.push_back(condition.rootIfgroup);
        for (int visibleRoot : visibleConditionRoots) {
            int ancestorBranch = visibleRoot > 0 && visibleRoot < arr_ifunit.count
                ? ((IFUNIT*)arr_ifunit.data)[visibleRoot].parentID : 0;
            while (ancestorBranch > 0 && ancestorBranch < arr_ifunit.count) {
                const int ancestorRoot = getRootIfgroup(ancestorBranch);
                const bool alreadyPresent = conditionIndexByRoot.find(
                    ancestorRoot) != conditionIndexByRoot.end();
                if (!alreadyPresent) {
                    ConditionBlock ancestor;
                    ancestor.rootIfgroup = ancestorRoot;
                    ancestor.depth = ((IFUNIT*)arr_ifunit.data)[ancestorRoot].depth;
                    int dummyOrder = 0;
                    getHeader(ancestorRoot, ancestor.label, dummyOrder);
                    if (ancestor.label.empty()) ancestor.label = "#IF";
                    for (int group = ancestorRoot; group < arr_ifunit.count; ++group) {
                        if (getRootIfgroup(group) != ancestorRoot) continue;
                        BranchBlock branch;
                        branch.ifgroup = group;
                        getHeader(group, branch.label, branch.order);
                        if (branch.label.empty()) branch.label = branch.order == 0 ? "#IF" : "#ELSE";
                        ancestor.branches.push_back(branch);
                    }
                    conditions.push_back(ancestor);
                    conditionIndexByRoot[ancestorRoot] =
                        (int)conditions.size() - 1;
                }
                ancestorBranch = ((IFUNIT*)arr_ifunit.data)[ancestorRoot].parentID;
            }
        }
        std::stable_sort(conditions.begin(), conditions.end(), [](const ConditionBlock& a, const ConditionBlock& b) {
            if (a.rootIfgroup == b.rootIfgroup) return false;
            if (a.rootIfgroup == 0) return true;
            if (b.rootIfgroup == 0) return false;
            return a.rootIfgroup < b.rootIfgroup;
        });

        if (objectBranchTreeIfCount != arr_ifunit.count) {
            objectBranchTreeOpen.clear();
            objectBranchTreeIfCount = arr_ifunit.count;
        }

        int requestedIfgroup = 0;
        if (requestedObjectModel >= 0 && requestedObjectModel <
            (int)objectEditorModel.Objects().size()) {
            const SEObjectInstance& requestedObject =
                objectEditorModel.Objects()[requestedObjectModel];
            requestedIfgroup = requestedObject.ifgroup;
            if (requestedIfgroup == 0 && !requestedObject.rows.empty())
                requestedIfgroup = ((SKINFILELINEREAD*)skinfileLines.data)
                    [requestedObject.rows.front()].ifgroup;
            int pathBranch = requestedIfgroup;
            while (pathBranch > 0 && pathBranch < arr_ifunit.count) {
                objectBranchTreeOpen[pathBranch] = true;
                const int pathRoot = getRootIfgroup(pathBranch);
                pathBranch = pathRoot > 0 && pathRoot < arr_ifunit.count
                    ? ((IFUNIT*)arr_ifunit.data)[pathRoot].parentID : 0;
            }
        }

        auto branchIsOnRequestedPath = [&](int branchIfgroup) -> bool {
            int pathBranch = requestedIfgroup;
            while (pathBranch > 0 && pathBranch < arr_ifunit.count) {
                if (pathBranch == branchIfgroup) return true;
                const int pathRoot = getRootIfgroup(pathBranch);
                pathBranch = pathRoot > 0 && pathRoot < arr_ifunit.count
                    ? ((IFUNIT*)arr_ifunit.data)[pathRoot].parentID : 0;
            }
            return false;
        };

        for (int ci = 0; ci < (int)conditions.size(); ++ci) {
            ConditionBlock& cond = conditions[ci];
            // Conditions are stored in source order, but visually form a
            // tree. Do not draw a nested condition when its owning branch is
            // collapsed; this makes folding a parent hide the whole subtree.
            if (cond.rootIfgroup > 0 && cond.rootIfgroup < arr_ifunit.count) {
                bool ancestorCollapsed = false;
                int parentBranch = ((IFUNIT*)arr_ifunit.data)[cond.rootIfgroup].parentID;
                while (parentBranch > 0 && parentBranch < arr_ifunit.count) {
                    std::map<int, bool>::const_iterator parentOpen = objectBranchTreeOpen.find(parentBranch);
                    if (parentOpen != objectBranchTreeOpen.end() && !parentOpen->second) {
                        ancestorCollapsed = true;
                        break;
                    }
                    const int parentRoot = getRootIfgroup(parentBranch);
                    parentBranch = parentRoot > 0 && parentRoot < arr_ifunit.count
                        ? ((IFUNIT*)arr_ifunit.data)[parentRoot].parentID : 0;
                }
                if (ancestorCollapsed) continue;
            }
            const float conditionIndent = cond.depth > 1
                ? (float)(cond.depth - 1) * ImGui::GetStyle().IndentSpacing : 0.0f;
            if (conditionIndent > 0.0f) ImGui::Indent(conditionIndent);
            ImGui::PushID(cond.rootIfgroup);

            auto drawBranchObjects = [&](BranchBlock& branch) {
                ImGuiListClipper objectClipper;
                objectClipper.Begin((int)branch.localObjectIndices.size());
                if (requestedObjectModel >= 0) {
                    for (int k = 0; k < (int)branch.localObjectIndices.size(); ++k) {
                        const int requestedOi = branch.localObjectIndices[k];
                        if (requestedOi >= 0 && requestedOi < (int)groupObjects.size() &&
                            groupObjects[requestedOi] == requestedObjectModel) {
                            objectClipper.IncludeItemByIndex(k);
                            break;
                        }
                    }
                }
                while (objectClipper.Step()) {
                for (int k = objectClipper.DisplayStart;
                    k < objectClipper.DisplayEnd; ++k) {
                    int oi = branch.localObjectIndices[k];
                    const SEObjectInstance& o = objectEditorModel.Objects()[groupObjects[oi]];
                    char label[256];
                    int firstRow = o.rows.empty() ? -1 : o.rows[0];
                    int key = -1;
                    if (firstRow >= 0) {
                        SKINFILELINEREAD& r = ((SKINFILELINEREAD*)skinfileLines.data)[firstRow];
                        if (r.csv.str[1].body) key = r.csv.val[1];
                    }
                    std::vector<int> labelOps;
                    for (int rowIndex = 0; rowIndex < (int)o.rows.size() && labelOps.size() < 2; ++rowIndex) {
                        const int row = o.rows[rowIndex];
                        if (row < 0 || row >= skinfileLines.count) continue;
                        SKINFILELINEREAD& opRow = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                        const char* opCommand = opRow.csv.str[0].body ? opRow.csv.str[0].outstr() : "";
                        const std::vector<int>& labelOptionColumns = optionColumnsFor(opCommand);
                        for (int col : labelOptionColumns) {
                            if (labelOps.size() >= 2) break;
                            const int op = opRow.csv.val[col];
                            if (op == 0) continue;
                            bool duplicate = false;
                            for (std::size_t existing = 0; existing < labelOps.size(); ++existing)
                                if (labelOps[existing] == op) { duplicate = true; break; }
                            if (!duplicate) labelOps.push_back(op);
                        }
                    }

                    std::string approximateName;
                    if (!o.name.empty()) {
                        approximateName = Cp932ToUtf8(o.name.c_str());
                    } else {
                        for (std::size_t opIndex = 0; opIndex < labelOps.size(); ++opIndex) {
                            const int op = labelOps[opIndex];
                            const int optionId = op < 0 ? -op : op;
                            const char* optionName = dstName(optionId);
                            if (!optionName || !*optionName) continue;
                            if (!approximateName.empty()) approximateName += " / ";
                            if (op < 0) approximateName += "!";
                            approximateName += optionName;
                        }
                        // Automatic label priority for unnamed objects is
                        // option first, then timer. Timer 0 is the ubiquitous
                        // MainTimer and is intentionally not descriptive.
                        if (approximateName.empty()) {
                            std::vector<int> labelTimers;
                            for (int row : o.rows) {
                                if (row < 0 || row >= skinfileLines.count || labelTimers.size() >= 2) continue;
                                SKINFILELINEREAD& timerRow = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                                const char* command = timerRow.csv.str[0].body
                                    ? timerRow.csv.str[0].outstr() : "";
                                const std::vector<int>& timerColumns = timerColumnsFor(command);
                                for (int col : timerColumns) {
                                    const int timer = timerRow.csv.val[col];
                                    if (timer == 0) continue;
                                    if (std::find(labelTimers.begin(), labelTimers.end(), timer) == labelTimers.end())
                                        labelTimers.push_back(timer);
                                    if (labelTimers.size() >= 2) break;
                                }
                            }
                            for (int timer : labelTimers) {
                                const char* name = timerName(timer, true);
                                if (!name || !*name) continue;
                                if (!approximateName.empty()) approximateName += " / ";
                                approximateName += name;
                            }
                        }
                    }

                    const SEObjectGroupDef* objectTypeDef = objectEditorModel.Group(o.group);
                    const char* objectTypeName = objectTypeDef ? objectTypeDef->name.c_str() : "OBJECT";
                    if (key >= 0 && !approximateName.empty())
                        snprintf(label, sizeof(label), "%03d  [%s]  #%d  %s", oi, objectTypeName, key, approximateName.c_str());
                    else if (key >= 0)
                        snprintf(label, sizeof(label), "%03d  [%s]  #%d", oi, objectTypeName, key);
                    else if (!approximateName.empty())
                        snprintf(label, sizeof(label), "%03d  [%s]  %s", oi, objectTypeName, approximateName.c_str());
                    else
                        snprintf(label, sizeof(label), "%03d  [%s]", oi, objectTypeName);
                    bool hasDst = false;
                    for (int rowIndex = 0; rowIndex < (int)o.rows.size(); ++rowIndex) {
                        int row = o.rows[rowIndex];
                        if (row < 0 || row >= skinfileLines.count) continue;
                        SKINFILELINEREAD& dstRow = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                        if (!dstRow.csv.str[0].body) continue;
                        const char* command = dstRow.csv.str[0].outstr();
                        if (strncmp(command, "#DST", 4) != 0) continue;
                        hasDst = true;
                    }
                    const bool objectEnabled = objectIsActive(o);

                    ImGui::PushID(oi);
                    ImVec4 objectStatusColor(0, 0, 0, 0);
                    if (hasDst) {
                        const ImVec4 rowColor = objectEnabled
                            ? ImVec4(0.18f, 0.48f, 0.23f, 0.22f)
                            : ImVec4(0.55f, 0.18f, 0.18f, 0.22f);
                        const ImVec4 hoverColor = objectEnabled
                            ? ImVec4(0.24f, 0.62f, 0.30f, 0.36f)
                            : ImVec4(0.68f, 0.22f, 0.22f, 0.36f);
                        objectStatusColor = objectEnabled
                            ? ImVec4(0.35f, 0.82f, 0.42f, 0.95f)
                            : ImVec4(0.92f, 0.32f, 0.30f, 0.95f);
                        ImGui::PushStyleColor(ImGuiCol_Header, rowColor);
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, hoverColor);
                        ImGui::PushStyleColor(ImGuiCol_HeaderActive, hoverColor);
                    }
                    const int modelIndex = groupObjects[oi];
                    const bool isMultiSelected = std::find(preview_selected_object_model_indices.begin(),
                        preview_selected_object_model_indices.end(), modelIndex) !=
                        preview_selected_object_model_indices.end();
                    const float objectRowWidth = (std::min)(
                        ImGui::GetContentRegionAvail().x,
                        ImGui::CalcTextSize(label).x +
                            ImGui::GetStyle().FramePadding.x * 2.0f + 8.0f);
                    const bool clicked = ImGui::Selectable(label, isMultiSelected,
                        ImGuiSelectableFlags_None, ImVec2(objectRowWidth, 0.0f));
                    const bool objectRowHovered =
                        ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone);
                    if (objectRowHovered) {
                        SetImageManagerHoveredObject(
                            isMultiSelected ? -1 : modelIndex,
                            ImGui::GetFrameCount());
                    }
                    const ImVec2 reorderRowMin = ImGui::GetItemRectMin();
                    const ImVec2 reorderRowMax = ImGui::GetItemRectMax();
                    if (hasDst) {
                        ImGui::GetWindowDrawList()->AddRectFilled(
                            reorderRowMin,
                            ImVec2(reorderRowMin.x + 3.0f, reorderRowMax.y),
                            ImGui::GetColorU32(objectStatusColor));
                    }
                    if (modelIndex == requestedObjectModel)
                        ImGui::SetScrollHereY(0.5f);
                    if (hasDst) ImGui::PopStyleColor(3);
                    if (clicked) {
                        selected_object_editor = oi;
                        preview_selected_object_model_index = modelIndex;
                        syncDstSelectionForObject(modelIndex, true);
                        preview_object_dragging = false;
                        preview_object_resizing = false;
                        preview_selected_obj_valid = false;
                        preview_selected_obj_last_valid = false;

                        if (ImGui::GetIO().KeyShift && preview_selection_anchor_model_index >= 0) {
                            std::vector<int>::const_iterator anchorIt = std::find(groupObjects.begin(), groupObjects.end(),
                                preview_selection_anchor_model_index);
                            std::vector<int>::const_iterator clickedIt = std::find(groupObjects.begin(), groupObjects.end(), modelIndex);
                            if (anchorIt != groupObjects.end() && clickedIt != groupObjects.end()) {
                                int anchorPosition = (int)(anchorIt - groupObjects.begin());
                                int clickedPosition = (int)(clickedIt - groupObjects.begin());
                                if (anchorPosition > clickedPosition) std::swap(anchorPosition, clickedPosition);
                                if (!ImGui::GetIO().KeyCtrl) preview_selected_object_model_indices.clear();
                                for (int rangePosition = anchorPosition; rangePosition <= clickedPosition; ++rangePosition) {
                                    const int rangeModelIndex = groupObjects[rangePosition];
                                    if (std::find(preview_selected_object_model_indices.begin(),
                                        preview_selected_object_model_indices.end(), rangeModelIndex) ==
                                        preview_selected_object_model_indices.end())
                                        preview_selected_object_model_indices.push_back(rangeModelIndex);
                                }
                            } else {
                                preview_selected_object_model_indices.clear();
                                preview_selected_object_model_indices.push_back(modelIndex);
                                preview_selection_anchor_model_index = modelIndex;
                            }
                        } else if (ImGui::GetIO().KeyCtrl) {
                            std::vector<int>::iterator selectedIt = std::find(
                                preview_selected_object_model_indices.begin(),
                                preview_selected_object_model_indices.end(), modelIndex);
                            if (selectedIt == preview_selected_object_model_indices.end())
                                preview_selected_object_model_indices.push_back(modelIndex);
                            else
                                preview_selected_object_model_indices.erase(selectedIt);
                            preview_selection_anchor_model_index = modelIndex;
                        } else {
                            preview_selected_object_model_indices.clear();
                            preview_selected_object_model_indices.push_back(modelIndex);
                            preview_selection_anchor_model_index = modelIndex;
                        }

                        const bool clickedRemainsSelected = std::find(
                            preview_selected_object_model_indices.begin(),
                            preview_selected_object_model_indices.end(), modelIndex) !=
                            preview_selected_object_model_indices.end();
                        const int activeModelIndex = clickedRemainsSelected ? modelIndex :
                            (preview_selected_object_model_indices.empty() ? -1 :
                                preview_selected_object_model_indices.front());
                        SetObjectSelection(preview_selected_object_model_indices,
                            activeModelIndex, preview_selection_anchor_model_index, false);

                        RefreshPreviewSelectionBounds();
                    }
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                        ImGui::SetDragDropPayload("SKINEDITOR_OBJECT_REORDER", &modelIndex,
                            sizeof(modelIndex), ImGuiCond_Once);
                        ImGui::TextUnformatted("Move Object");
                        ImGui::TextDisabled("%s", label);
                        ImGui::Separator();
                        ImGui::TextDisabled("Drop above or below another Object.");
                        ImGui::TextDisabled("A different IF branch becomes the new branch.");
                        ImGui::TextDisabled("Moving to another include file asks for confirmation.");
                        ImGui::EndDragDropSource();
                    }
                    if (ImGui::BeginDragDropTarget()) {
                        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(
                            "SKINEDITOR_OBJECT_REORDER",
                            ImGuiDragDropFlags_AcceptBeforeDelivery);
                        if (payload && payload->DataSize == sizeof(int)) {
                            const int sourceModelIndex = *(const int*)payload->Data;
                            const bool placeAfter = ImGui::GetIO().MousePos.y >
                                (reorderRowMin.y + reorderRowMax.y) * 0.5f;
                            const bool validTarget =
                                CanReorderObject(sourceModelIndex, modelIndex);
                            const bool confirmTarget = validTarget &&
                                ObjectReorderRequiresConfirmation(
                                    sourceModelIndex, modelIndex);
                            const float markerY = placeAfter
                                ? reorderRowMax.y : reorderRowMin.y;
                            ImGui::GetWindowDrawList()->AddLine(
                                ImVec2(reorderRowMin.x, markerY),
                                ImVec2(reorderRowMax.x, markerY),
                                ImGui::GetColorU32(validTarget
                                    ? (confirmTarget
                                        ? ImVec4(0.95f, 0.62f, 0.20f, 1.0f)
                                        : ImVec4(0.30f, 0.78f, 1.0f, 1.0f))
                                    : ImVec4(0.95f, 0.32f, 0.32f, 1.0f)),
                                2.0f);
                            if (payload->IsDelivery() && validTarget)
                                QueueObjectReorder(sourceModelIndex, modelIndex, placeAfter);
                        }
                        ImGui::EndDragDropTarget();
                    }
                    if (ImGui::BeginPopupContextItem("ObjectContext")) {
                        ImGui::TextDisabled("Object %03d", oi);
                        if (ImGui::MenuItem("Create Object (new)")) {
                            AssignRootFileOwner(skinfileLines, mainpath, newObjectOwner);
                            newObjectInsertPosition = o.rows.empty()
                                ? FindOwnerFileEndRow(skinfileLines,
                                    newObjectOwner.body ? newObjectOwner.outstr() : mainpath)
                                : o.rows.back() + 1;
                            if (!o.rows.empty()) {
                                SKINFILELINEREAD& ownerRow =
                                    ((SKINFILELINEREAD*)skinfileLines.data)[o.rows.front()];
                                if (ownerRow.filename.body && *ownerRow.filename.outstr())
                                    newObjectOwner.assign(ownerRow.filename.outstr());
                            }
                            newObjectIfgroup = o.ifgroup;
                            newCommandIncludeAll = false;
                            newObjectName.assign("");
                            wNewObject = true;
                        }
                        if (ImGui::MenuItem("Create Command / Setting")) {
                            AssignRootFileOwner(skinfileLines, mainpath, newObjectOwner);
                            newObjectInsertPosition = o.rows.empty()
                                ? FindOwnerFileEndRow(skinfileLines,
                                    newObjectOwner.body ? newObjectOwner.outstr() : mainpath)
                                : o.rows.back() + 1;
                            if (!o.rows.empty()) {
                                SKINFILELINEREAD& ownerRow =
                                    ((SKINFILELINEREAD*)skinfileLines.data)[o.rows.front()];
                                if (ownerRow.filename.body && *ownerRow.filename.outstr())
                                    newObjectOwner.assign(ownerRow.filename.outstr());
                            }
                            newObjectIfgroup = o.ifgroup;
                            newCommandIncludeAll = true;
                            newObjectName.assign("");
                            wNewObject = true;
                        }
                        ImGui::Separator();
                        if (ImGui::MenuItem(SEText("Copy", u8"\uBCF5\uC0AC"), "Ctrl+C"))
                            contextCopyModel = modelIndex;
                        if (ImGui::MenuItem(SEText("Paste", u8"\uBD99\uC5EC\uB123\uAE30"), "Ctrl+V", false,
                            HasCopiedObjects()))
                            contextPasteModel = modelIndex;
                        if (ImGui::MenuItem(SEText("Duplicate", u8"\uBCF5\uC81C"), "Ctrl+D"))
                            contextDuplicateModel = modelIndex;
                        if (ImGui::MenuItem("Remove Object")) contextDeleteModel = modelIndex;
                        ImGui::Separator();
                        if (ImGui::MenuItem("Create Group from Selection", NULL, false,
                            !preview_selected_object_model_indices.empty()))
                            requestCreateGroupPopup = true;
                        ImGui::EndPopup();
                    }
                    ImGui::PopID();
                }
                }
                objectClipper.End();
            };

            if (cond.rootIfgroup == 0) {
                // Unconditional objects are not part of a condition block.
                if (requestedObjectModel >= 0 && requestedIfgroup == 0)
                    ImGui::SetNextItemOpen(true, ImGuiCond_Always);
                bool open = ImGui::TreeNodeEx("always", ImGuiTreeNodeFlags_DefaultOpen, "ALWAYS");
                if (open) {
                    for (int bi = 0; bi < (int)cond.branches.size(); ++bi)
                        drawBranchObjects(cond.branches[bi]);
                    ImGui::TreePop();
                }
            }
            else {
                // The real #IF is the condition node; no synthetic CONDITION
                // wrapper is needed. ELSEIF/ELSE remain sibling branches.
                std::stable_sort(cond.branches.begin(), cond.branches.end(),
                    [](const BranchBlock& a, const BranchBlock& b) { return a.order < b.order; });
                for (int bi = 0; bi < (int)cond.branches.size(); ++bi) {
                    BranchBlock& branch = cond.branches[bi];
                    ImGui::PushID(branch.ifgroup);
                    const bool isBranchActive = branch.ifgroup > 0 &&
                        branch.ifgroup < (int)objectBranchActive.size() &&
                        objectBranchActive[branch.ifgroup];
                    bool isParentActive = false;
                    if (branch.ifgroup > 0 && branch.ifgroup < arr_ifunit.count) {
                        const int parent = ((IFUNIT*)arr_ifunit.data)[branch.ifgroup].parentID;
                        isParentActive = parent >= 0 &&
                            parent < (int)objectBranchActive.size() && objectBranchActive[parent];
                    }
                    const ImVec4 branchColor = !isParentActive
                        ? ImVec4(0.28f, 0.28f, 0.30f, 0.22f)
                        : (isBranchActive
                            ? ImVec4(0.18f, 0.48f, 0.23f, 0.22f)
                            : ImVec4(0.55f, 0.18f, 0.18f, 0.22f));
                    const ImVec4 branchHoverColor = !isParentActive
                        ? ImVec4(0.38f, 0.38f, 0.41f, 0.36f)
                        : (isBranchActive
                            ? ImVec4(0.24f, 0.62f, 0.30f, 0.36f)
                            : ImVec4(0.68f, 0.22f, 0.22f, 0.36f));
                    ImGui::PushStyleColor(ImGuiCol_Header, branchColor);
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, branchHoverColor);
                    ImGui::PushStyleColor(ImGuiCol_HeaderActive, branchHoverColor);
                    if (branchIsOnRequestedPath(branch.ifgroup))
                        ImGui::SetNextItemOpen(true, ImGuiCond_Always);
                    bool branchOpen = ImGui::TreeNodeEx("branch", ImGuiTreeNodeFlags_DefaultOpen,
                        "%s  (%d)", branch.label.c_str(), (int)branch.localObjectIndices.size());
                    const bool showElseContext = branch.label == "#ELSE" &&
                        ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip);
                    objectBranchTreeOpen[branch.ifgroup] = branchOpen;
                    ImGui::PopStyleColor(3);
                    if (ImGui::BeginPopupContextItem("BranchContext")) {
                        int insertPosition = skinfileLines.count;
                        CSTR branchOwner(mainpath);
                        for (int row = 0; row < skinfileLines.count; ++row) {
                            SKINFILELINEREAD& candidate = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                            if (candidate.ifgroup != branch.ifgroup) continue;
                            if (candidate.filename.body && *candidate.filename.outstr())
                                branchOwner.assign(candidate.filename.outstr());
                            if (candidate.isIfGroupEnd) {
                                insertPosition = row;
                                break;
                            }
                            insertPosition = row + 1;
                        }
                        if (ImGui::MenuItem("Create Object in this branch")) {
                            newObjectInsertPosition = insertPosition;
                            newObjectOwner.assign(branchOwner);
                            newObjectIfgroup = branch.ifgroup;
                            newCommandIncludeAll = false;
                            newObjectName.assign("");
                            wNewObject = true;
                        }
                        if (ImGui::MenuItem("Create Command / Setting in this branch")) {
                            newObjectInsertPosition = insertPosition;
                            newObjectOwner.assign(branchOwner);
                            newObjectIfgroup = branch.ifgroup;
                            newCommandIncludeAll = true;
                            newObjectName.assign("");
                            wNewObject = true;
                        }
                        ImGui::EndPopup();
                    }
                    if (showElseContext) {
                        std::vector<std::pair<int, std::string> > precedingConditions;
                        for (int group = 1; group < arr_ifunit.count; ++group) {
                            if (getRootIfgroup(group) != cond.rootIfgroup) continue;
                            std::string siblingLabel;
                            int siblingOrder = 0;
                            if (!getHeader(group, siblingLabel, siblingOrder) ||
                                siblingOrder >= branch.order || siblingLabel == "#ELSE") continue;
                            precedingConditions.push_back(std::make_pair(siblingOrder, siblingLabel));
                        }
                        std::stable_sort(precedingConditions.begin(), precedingConditions.end(),
                            [](const std::pair<int, std::string>& a,
                                const std::pair<int, std::string>& b) {
                                return a.first < b.first;
                            });

                        if (ImGui::BeginTooltip()) {
                            ImGui::TextDisabled("ELSE fallback for");
                            for (const std::pair<int, std::string>& condition : precedingConditions) {
                                ImGui::Bullet();
                                ImGui::SameLine();
                                ImGui::TextUnformatted(condition.second.c_str());
                            }
                            if (precedingConditions.empty())
                                ImGui::TextDisabled("(condition header not found)");
                            ImGui::Separator();
                            ImGui::TextDisabled("Used when none of the conditions above match.");
                            ImGui::EndTooltip();
                        }
                    }
                    if (branchOpen) {
                        drawBranchObjects(branch);
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
            }
            ImGui::PopID();
            if (conditionIndent > 0.0f) ImGui::Unindent(conditionIndent);
        }
        }
        ImGui::EndChild();
    }

    if (contextDeleteModel >= 0) {
        pendingObjectDelete = MakeObjectSelectionKey(contextDeleteModel);
        objectDeleteDialogRequested = pendingObjectDelete.IsValid();
    }

    const auto selectContextObject = [&](int modelIndex, bool preserveMulti) {
        const bool alreadySelected = std::find(
            preview_selected_object_model_indices.begin(),
            preview_selected_object_model_indices.end(), modelIndex) !=
            preview_selected_object_model_indices.end();
        if (!preserveMulti || !alreadySelected)
            SetObjectSelection(std::vector<int>(1, modelIndex), modelIndex,
                modelIndex, true);
    };
    if (contextCopyModel >= 0) {
        selectContextObject(contextCopyModel, true);
        CopySelectedObjects();
    }
    if (contextPasteModel >= 0) {
        selectContextObject(contextPasteModel, false);
        PasteCopiedObjects();
    }
    if (contextDuplicateModel >= 0) {
        selectContextObject(contextDuplicateModel, true);
        DuplicateSelectedObjects();
    }
    if (browserWasBegun) ImGui::End();

    return 0;
}
