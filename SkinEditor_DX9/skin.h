#pragma once
#include <iostream>
#include "../LR2/structure.h"

#define MAX_IFDEPTH 99





int LR2SEInit(game* g);

int LR2SEDrawLoop(game* g, int gHandle, int sizeX, int sizeY);

int LR2SESceneInit(game* g, int type);

int LR2SESceneProc(game* g, int type);

int LR2SE_I_Play(game* g);
