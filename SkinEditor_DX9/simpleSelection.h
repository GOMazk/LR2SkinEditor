#pragma once

#include <string>
#include <vector>

// Editor-only grouping. These values are never serialized into OLRskin.
enum class SESelectionGroup {
    SongList, Highlight, SongInfo, Graphs, Controls, Decoration, SelectedObjects
};
enum class SESelectionEditKind { Layout, Tint, Animation };
enum class SESelectionEffect { FadeIn, FadeOut, SlideIn, Pulse };

struct SESelectionEdit {
    SESelectionGroup group = SESelectionGroup::SongInfo;
    SESelectionEditKind kind = SESelectionEditKind::Layout;
    bool selectedBranchOnly = false;
    int offsetX = 0;
    int offsetY = 0;
    float scalePercent = 100.0f;
    float tint[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; // RGBA multipliers
    SESelectionEffect effect = SESelectionEffect::FadeIn;
    int timer = -1; // -1 preserves each timeline's existing trigger
    int delayMs = 0;
    int durationMs = 400;
    int slideX = -40;
    int slideY = 0;
    int minimumOpacityPercent = 35;
    bool replaceAnimation = false;
};

// Derived rows and model indices are valid only until the next invalidation.
// UI stores a group enum and uses WORKSPACE's stable Object selection instead.
struct SESelectionTimeline {
    std::string command;
    std::string label;
    std::vector<int> rows;
    int objectIndex = -1;
    int ifgroup = 0;
    bool isBarAnchor = false;
    bool isBarRelative = false;
};

const char* SESelectionGroupTitle(SESelectionGroup group);
bool SESelectionMatchesGroup(const SESelectionTimeline& timeline, SESelectionGroup group);
int RunSimpleSelectionSelfTest();
