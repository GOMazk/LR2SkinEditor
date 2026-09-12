#pragma once
#include <iostream>
#include "../LR2/structure.h"

#define MAX_IFDEPTH 99





int LR2SEInit(game* g, bool initializeCore);

inline bool LR2SEPreviewDrawVisible(int order, const unsigned char* mask, int count) {
    return !mask || (order >= 0 && order < count && mask[order] != 0);
}
inline void LR2SEFilterPreviewDrawBuffer(DrawingBuf& buffer,
    const unsigned char* mask, int count) {
    if (!mask) return;
    int kept = 0;
    for (int i = 0; i < buffer.count; ++i)
        if (LR2SEPreviewDrawVisible(buffer.dstd[i].sourceOrder, mask, count))
            buffer.dstd[kept++] = buffer.dstd[i];
    buffer.count = kept;
}
// Explicit CLI visual-test sample. A null pointer preserves the normal editor
// preview. Runtime skin/CSV data is never changed to manufacture test frames.
struct LR2SEPreviewSample {
    int gauge = 100;
    int combo = 123;
    int judge = -1; // -1: no judgement; 0..5: actual LR2 judgement slot
    int notes = 0; // 0 all, 1 normal, 2 long, 3 mine, 4 none
};
int LR2SEDrawLoop(game* g, int gHandle, int sizeX, int sizeY, bool staticSpecialPreview,
    const unsigned char* mask = nullptr, int maskCount = 0,
    const LR2SEPreviewSample* sample = nullptr);
int LR2SEDrawLoopSafe(game* g, int gHandle, int sizeX, int sizeY, bool staticSpecialPreview,
    const unsigned char* mask = nullptr, int maskCount = 0,
    const LR2SEPreviewSample* sample = nullptr);
void LR2SEResetRenderFault();
void LR2SEPreparePreviewState(game* g, int type);

enum LR2SEPreviewNoteKind {
	LR2SE_PREVIEW_NOTE_NORMAL = 0,
	LR2SE_PREVIEW_NOTE_LONG = 1,
	LR2SE_PREVIEW_NOTE_MINE = 2,
};

enum LR2SEPreviewChartMode {
	LR2SE_PREVIEW_CHART_SIMPLE = 0,
	LR2SE_PREVIEW_CHART_FULL = 1,
};

struct LR2SEPreviewChartNote {
	int lane;
	unsigned int timingMs;
	unsigned int endTimingMs;
	LR2SEPreviewNoteKind kind;
};

struct LR2SEPreviewTimelineEvent {
	unsigned int timingMs;
	int op;
	double value;
	bool terminal;
};

int LR2SEBuildPreviewChart(int type, LR2SEPreviewChartMode mode,
	LR2SEPreviewChartNote* notes, int capacity);
int LR2SEGetSamplePreviewScratchSide(int type, int scratchSide1,
	int scratchSide2);
void LR2SEResetPreviewCourseState(gameplay* preview);
int LR2SEBuildPreviewTimeline(LR2SEPreviewTimelineEvent* events, int capacity);
bool LR2SEShouldDrawStaticNormalSample(int sampleIndex, bool longNoteVisible);
int LR2SEGetSamplePreviewScratchSide(int type, int scratchSide1,
	int scratchSide2);

int LR2SESceneInitSafe(game* g, int type, LR2SEPreviewChartMode mode);
int LR2SESceneProcSafe(game* g, int type, LR2SEPreviewChartMode mode);

int LR2SESceneInit(game* g, int type, LR2SEPreviewChartMode mode);

int LR2SESceneProc(game* g, int type, LR2SEPreviewChartMode mode);
