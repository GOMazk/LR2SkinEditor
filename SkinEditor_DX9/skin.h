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

int LR2SESceneInit(game* g, int type);

int LR2SESceneProc(game* g, int type);

int LR2SE_I_Play(game* g);
