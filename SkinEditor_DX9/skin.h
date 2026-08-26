#pragma once
#include <iostream>
#include "../LR2/structure.h"

#define MAX_IFDEPTH 99





int LR2SEInit(game* g);

int LR2SEDrawLoop(game* g, int gHandle, int sizeX, int sizeY, bool staticSpecialPreview);
int LR2SEDrawLoopSafe(game* g, int gHandle, int sizeX, int sizeY, bool staticSpecialPreview);
int LR2SESceneInitSafe(game* g, int type);
int LR2SESceneProcSafe(game* g, int type);
void LR2SEResetRenderFault();
void LR2SEPreparePreviewState(game* g, int type);

enum LR2SEPreviewNoteKind {
	LR2SE_PREVIEW_NOTE_NORMAL = 0,
	LR2SE_PREVIEW_NOTE_LONG = 1,
	LR2SE_PREVIEW_NOTE_MINE = 2,
};

struct LR2SEPreviewChartNote {
	int lane;
	unsigned int timingMs;
	unsigned int endTimingMs;
	LR2SEPreviewNoteKind kind;
};

int LR2SEBuildPreviewChart(int type, LR2SEPreviewChartNote* notes, int capacity);

int LR2SESceneInit(game* g, int type);

int LR2SESceneProc(game* g, int type);
