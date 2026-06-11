#include "En_graphic.h"
#include "DxLib/DxLib.h"

//49e780
int ReloadImage(CSTR filename, int *grHandle) {
	return ReloadGraph(filename, *grHandle, 0);
}

//49ed30
int ScreenCapture(uint iGrHandle, int x, int y){
	GetDrawScreenGraph(0, 0, x, y, iGrHandle, 1);
	return 0;
}

int hBackImage;
//4a9ff0
int SetBackground(int hImage) {
	hBackImage = hImage;
	return 1;
}

int screenSizeX, screenSizeY;
int skinSizeX, skinSizeY;
int Resize(game* g, double skinX, double skinY, bool bit16) {
	int oldXpos = 320, oldYpos = 240;

	GetWindowPosition(&oldXpos, &oldYpos);
	SetGraphMode(skinX, skinY, bit16? 16 : 32, 60);
	SetWindowSizeExtendRate((double)g->config.system.windowsize_x / skinX, (double)g->config.system.windowsize_y / skinY);
	SetDrawScreen(DX_SCREEN_BACK);
	SetWindowPosition(oldXpos, oldYpos);

	skinSizeX = skinX;
	skinSizeY = skinY;
	return 0;
}