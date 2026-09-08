#include "winWorkspace.h"
#include "winWorkspaceUiHelpers.h"
#include "seLocalization.h"
#include "seUI.h"
#include "seHelper.h"
#include "uiCatalog.h"
#include "inputwrap.h"

const char* SESelectionGroupTitle(SESelectionGroup group) {
    switch (group) {
    case SESelectionGroup::SongList: return SEText("Song list", u8"곡 목록 전체");
    case SESelectionGroup::Highlight: return SEText("Selected song highlight", u8"선택된 곡 강조");
    case SESelectionGroup::SongInfo: return SEText("Song information & numbers", u8"곡 정보·숫자");
    case SESelectionGroup::Graphs: return SEText("Difficulty & graphs", u8"난이도·그래프");
    case SESelectionGroup::Controls: return SEText("Buttons & sliders", u8"버튼·슬라이더");
    case SESelectionGroup::Decoration: return SEText("Background & decoration", u8"배경·장식");
    case SESelectionGroup::SelectedObjects: return SEText("Selected Objects", u8"선택한 Object 묶음");
    }
    return "";
}

void WORKSPACE::drawSimpleSelection() {
    ImGui::TextWrapped("%s", SEText(
        "Choose a group in the loaded skin, adjust its appearance, then play an effect at an event.",
        u8"불러온 스킨에서 묶음을 고르고 배치와 모양을 수정하거나, 원하는 시점에 효과를 재생하세요."));
    ImGui::TextUnformatted(SEText("Group", u8"대상 묶음"));
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##SelectionGroup", SESelectionGroupTitle(simpleSelectionEdit.group))) {
        for (int index = 0; index <= (int)SESelectionGroup::SelectedObjects; ++index) {
            const auto group = (SESelectionGroup)index;
            if (ImGui::Selectable(SESelectionGroupTitle(group), simpleSelectionEdit.group == group)) {
                simpleSelectionEdit.group = group;
                simpleSelectionStatus.clear();
            }
        }
        ImGui::EndCombo();
    }
    if (simpleSelectionEdit.group == SESelectionGroup::SelectedObjects) {
        ImGui::TextWrapped("%s", SEText(
            "Select multiple Objects in Object Browser or Preview. This group follows that selection.",
            u8"Object Browser나 Preview에서 여러 Object를 선택하세요. 현재 선택을 하나의 묶음으로 편집합니다."));
        if (ImGui::Button(SEText("Open Object Browser", u8"Object Browser 열기"))) wObjectBrowser = true;
    }
    ImGui::Checkbox(SEText("Same condition branch only", u8"선택 Object와 같은 조건 분기만"),
        &simpleSelectionEdit.selectedBranchOnly);

    // Copy the derived targets for this frame. A committed command may invalidate
    // the Workspace cache; no panel retains pointers into it across an edit.
    const auto targets = GetSimpleSelectionTargets(simpleSelectionEdit);
    int animated = 0, frames = 0;
    bool hasBarAnchor = false, hasRelative = false;
    for (const auto& target : targets) {
        if (target.rows.size() > 1) ++animated;
        frames += (int)target.rows.size();
        hasBarAnchor |= target.isBarAnchor;
        hasRelative |= target.isBarRelative;
    }
    ImGui::Text(SEText("%d components / %d animation frames", u8"대상 %d개 / 애니메이션 프레임 %d개"),
        (int)targets.size(), frames);
    if (ImGui::CollapsingHeader(SEText("Review affected components", u8"적용 대상 확인"))) {
        if (ImGui::BeginChild("SelectionAffectedComponents", ImVec2(0, 160), ImGuiChildFlags_Borders)) {
            ImGuiListClipper clipper;
            clipper.Begin((int)targets.size());
            while (clipper.Step()) for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index) {
                const auto& target = targets[index];
                ImGui::TextUnformatted(Cp932ToUtf8(target.label.c_str()).c_str());
                if (ImGui::IsItemHovered()) {
                    const auto& line = ((SKINFILELINEREAD*)skinfileLines.data)[target.rows.front()];
                    ImGui::SetTooltip("%s | IF %d | %d frame(s)\n%s", target.command.c_str(),
                        target.ifgroup, (int)target.rows.size(),
                        Cp932ToUtf8(line.filename.body ? line.filename.body : "").c_str());
                }
            }
        }
        ImGui::EndChild();
    }
    if (hasBarAnchor && hasRelative)
        ImGui::TextWrapped("%s", SEText(
            "Moving this group moves the list anchors once; titles and lamps keep their offsets.",
            u8"목록 이동은 막대 기준 위치에 한 번만 적용하며, 제목과 램프의 상대 배치를 유지합니다."));
    if (targets.empty())
        SEUI::EmptyState(SEText("No matching components", u8"해당하는 요소가 없습니다"),
            SEText("Choose another group or select Objects in Object Browser.",
                u8"다른 묶음을 고르거나 Object Browser에서 편집할 Object를 선택하세요."));
    const bool busy = pendingHistorySnapshotRestore >= 0 || editorDerivedRebuildPending;

    if (ImGui::CollapsingHeader(SEText("Layout", u8"배치"), ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SetNextItemWidth(150);
        ImGui::InputInt(SEText("Move X", u8"가로 이동"), &simpleSelectionEdit.offsetX);
        ImGui::SetNextItemWidth(150);
        ImGui::InputInt(SEText("Move Y", u8"세로 이동"), &simpleSelectionEdit.offsetY);
        ImGui::SetNextItemWidth(180);
        ImGui::SliderFloat(SEText("Group scale", u8"묶음 크기"), &simpleSelectionEdit.scalePercent, 10, 300, "%.0f%%");
        ImGui::TextWrapped("%s", SEText("Offsets are in skin pixels. Existing movement and spacing scale together.",
            u8"이동량은 스킨 픽셀 기준입니다. 기존 움직임과 요소 사이 간격도 함께 확대·축소됩니다."));
        ImGui::BeginDisabled(busy || targets.empty());
        const bool apply = ImGui::Button(SEText("Apply layout to group", u8"묶음에 배치 적용"));
        ImGui::EndDisabled();
        if (apply) {
            simpleSelectionEdit.kind = SESelectionEditKind::Layout;
            simpleSelectionLastSucceeded = ApplySimpleSelectionEdit(simpleSelectionEdit, simpleSelectionStatus);
            if (simpleSelectionLastSucceeded) {
                simpleSelectionEdit.offsetX = simpleSelectionEdit.offsetY = 0;
                simpleSelectionEdit.scalePercent = 100;
            }
            return;
        }
    }

    if (ImGui::CollapsingHeader(SEText("Color & opacity", u8"색조·투명도"))) {
        ImGui::ColorEdit3(SEText("Tint", u8"색조"), simpleSelectionEdit.tint);
        ImGui::TextUnformatted(SEText("Opacity multiplier", u8"불투명도 비율"));
        ImGui::SetNextItemWidth(-1);
        ImGui::SliderFloat("##SelectionOpacity", &simpleSelectionEdit.tint[3], 0, 1, "%.2f");
        ImGui::TextWrapped("%s", SEText(
            "Multiplies the current color and opacity on every frame. White and 1.0 keep the current values.",
            u8"모든 프레임의 기존 색과 불투명도에 곱합니다. 흰색과 비율 1.0은 현재 값을 유지합니다."));
        ImGui::BeginDisabled(busy || targets.empty());
        const bool apply = ImGui::Button(SEText("Apply tint to group", u8"묶음에 색조 적용"));
        ImGui::EndDisabled();
        if (apply) {
            simpleSelectionEdit.kind = SESelectionEditKind::Tint;
            simpleSelectionLastSucceeded = ApplySimpleSelectionEdit(simpleSelectionEdit, simpleSelectionStatus);
            if (simpleSelectionLastSucceeded) for (float& channel : simpleSelectionEdit.tint) channel = 1;
            return;
        }
    }

    if (ImGui::CollapsingHeader(SEText("Event animation", u8"시점별 애니메이션"), ImGuiTreeNodeFlags_DefaultOpen)) {
        const char* effects[] = {
            SEText("Fade in", u8"서서히 나타나기"), SEText("Fade out", u8"서서히 사라지기"),
            SEText("Slide & fade in", u8"이동하며 나타나기"), SEText("Pulse repeatedly", u8"반복 점멸")
        };
        int effect = (int)simpleSelectionEdit.effect;
        if (ImGui::Combo(SEText("Effect", u8"효과"), &effect, effects, 4))
            simpleSelectionEdit.effect = (SESelectionEffect)effect;
        const int eventTimers[] = { -1, 0, 11, 2, 21, 31 };
        const char* eventNames[] = {
            SEText("Keep each component's event", u8"각 요소의 기존 시작 조건"),
            SEText("Scene entry", u8"화면 진입"), SEText("Song changed", u8"선택 곡 변경"),
            SEText("Scene exit", u8"화면 종료"), SEText("Panel 1 opened", u8"패널 1 열림"),
            SEText("Panel 1 closed", u8"패널 1 닫힘")
        };
        int event = -1;
        for (int i = 0; i < 6; ++i) if (eventTimers[i] == simpleSelectionEdit.timer) event = i;
        if (ImGui::BeginCombo(SEText("Start when", u8"효과 시작 시점"),
            event >= 0 ? eventNames[event] : SEText("Custom event", u8"직접 고른 조건"))) {
            for (int i = 0; i < 6; ++i) {
                ImGui::BeginDisabled(hasBarAnchor && eventTimers[i] > 0);
                if (ImGui::Selectable(eventNames[i], event == i)) simpleSelectionEdit.timer = eventTimers[i];
                ImGui::EndDisabled();
            }
            ImGui::EndCombo();
        }
        if (hasBarAnchor)
            ImGui::TextWrapped("%s", SEText(
                "LR2 list anchors animate from scene entry. Other events can be used on title/flash Objects selected separately.",
                u8"LR2의 목록 막대는 화면 진입 시점을 사용합니다. 제목이나 강조 효과만 따로 선택하면 다른 시점을 지정할 수 있습니다."));
        if (!hasBarAnchor && ImGui::TreeNode(SEText("More event timers", u8"다른 시작 조건"))) {
            int selected = simpleSelectionEdit.timer;
            if (DrawCommandValueCombo(SEText("Timer", u8"시작 조건"), "#DST_IMAGE", "$timer",
                simpleSelectionEdit.timer, selected)) simpleSelectionEdit.timer = selected;
            ImGui::TreePop();
        }
        ImGui::SetNextItemWidth(160);
        ImGui::InputInt(SEText("Delay (ms)", u8"시작 지연 (ms)"), &simpleSelectionEdit.delayMs, 50, 100);
        ImGui::SetNextItemWidth(160);
        ImGui::InputInt(SEText("Duration (ms)", u8"재생 시간 (ms)"), &simpleSelectionEdit.durationMs, 50, 100);
        if (simpleSelectionEdit.effect == SESelectionEffect::SlideIn) {
            ImGui::SetNextItemWidth(130);
            ImGui::InputInt(SEText("Start offset X", u8"시작 위치 가로 차이"), &simpleSelectionEdit.slideX);
            ImGui::SetNextItemWidth(130);
            ImGui::InputInt(SEText("Start offset Y", u8"시작 위치 세로 차이"), &simpleSelectionEdit.slideY);
        }
        if (simpleSelectionEdit.effect == SESelectionEffect::Pulse) {
            ImGui::TextUnformatted(SEText("Minimum opacity (%)", u8"가장 흐릴 때 불투명도 (%)"));
            ImGui::SetNextItemWidth(-1);
            ImGui::SliderInt("##SelectionMinimumOpacity", &simpleSelectionEdit.minimumOpacityPercent, 0, 100, "%d%%");
        }
        ImGui::Checkbox(SEText("Replace existing animation", u8"기존 애니메이션 교체 허용"),
            &simpleSelectionEdit.replaceAnimation);
        if (animated)
            ImGui::TextWrapped(SEText(
                "%d component(s) already animate. This preset uses the last pose and replaces those timelines; Undo restores them.",
                u8"%d개 요소에 기존 애니메이션이 있습니다. 마지막 자세를 기준으로 효과를 교체하며, Undo로 원래 움직임을 복구할 수 있습니다."), animated);
        const bool incompatibleTimer = hasBarAnchor && simpleSelectionEdit.timer > 0;
        ImGui::BeginDisabled(busy || targets.empty() || incompatibleTimer ||
            (animated > 0 && !simpleSelectionEdit.replaceAnimation));
        const bool apply = ImGui::Button(SEText("Apply effect & play in Preview", u8"효과 적용하고 Preview에서 재생"));
        ImGui::EndDisabled();
        if (apply) {
            simpleSelectionEdit.kind = SESelectionEditKind::Animation;
            simpleSelectionLastSucceeded = ApplySimpleSelectionEdit(simpleSelectionEdit, simpleSelectionStatus);
            if (simpleSelectionLastSucceeded) {
                char previewTitle[96];
                FormatSEUIWindowTitle(previewTitle, sizeof(previewTitle), SEUIWindowId::Preview, num);
                ImGui::SetWindowFocus(previewTitle);
            }
            return;
        }
        if (incompatibleTimer)
            ImGui::TextWrapped("%s", SEText("Choose Scene entry for this group.", u8"이 묶음의 시작 시점을 화면 진입으로 바꾸세요."));
    }
    ImGui::Separator();
    ImGui::BeginDisabled(busy || targets.empty());
    if (ImGui::Button(SEText("Replay current effect", u8"현재 효과 다시 재생"))) ReplaySimpleSelectionEffect();
    ImGui::EndDisabled();
    ImGui::BeginDisabled(busy || arr_history.count == 0);
    if (ImGui::Button(SEText("Undo group edit", u8"마지막 편집 되돌리기"))) {
        simpleSelectionLastSucceeded = UndoLastEdit() == 0;
        simpleSelectionPreviewTimers.clear();
        simpleSelectionStatus = SEText("Restoring the previous edit.", u8"직전 편집을 복구합니다.");
    }
    ImGui::EndDisabled();
    ImGui::BeginDisabled(busy || redoDocumentSnapshots.empty());
    if (ImGui::Button(SEText("Redo", u8"다시 실행"))) {
        simpleSelectionLastSucceeded = RedoLastEdit() == 0;
        simpleSelectionStatus = SEText("Restoring the next edit.", u8"다음 편집을 복구합니다.");
    }
    ImGui::EndDisabled();
    if (!simpleSelectionStatus.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, simpleSelectionLastSucceeded
            ? SEUI::Colors::Success() : SEUI::Colors::Danger());
        ImGui::TextWrapped("%s", simpleSelectionStatus.c_str());
        ImGui::PopStyleColor();
    }
    ImGui::TextWrapped("%s", SEText(
        "Changes are visible in the loaded skin immediately. Save (Ctrl+S) keeps them; Undo restores the prior edit.",
        u8"변경은 불러온 스킨에 바로 보입니다. 저장(Ctrl+S)하면 유지되며 Undo로 이전 편집을 복구할 수 있습니다."));
}
