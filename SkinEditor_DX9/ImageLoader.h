#pragma once
#include "imgui/imgui.h"
#include <D3dx9tex.h>
#include <d3d9.h>
#pragma comment(lib, "D3dx9")

typedef struct D3Image {
	int width = 0;
	int height = 0;
	PDIRECT3DTEXTURE9 texture = NULL;
}D3Image;
extern LPDIRECT3DDEVICE9 g_pd3dDevice;
bool LoadTextureFromFile(const char* filename, PDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height);
bool GetImageSizeFromFile(const char* filename, int* out_width, int* out_height);
bool LoadTextureFromFile(const char* filename, D3Image* d3);
bool LoadTextureFromMemory(void* data, size_t size, PDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height);
bool LoadTextureFromRawMemory(void* data, size_t size, int width, int height, PDIRECT3DTEXTURE9* out_texture);
bool RefreshTextureByRawMemory(void* data, size_t size, int width, int height, PDIRECT3DTEXTURE9* out_texture);
bool ReadTexturePixel(PDIRECT3DTEXTURE9 texture, int x, int y, D3DCOLOR* color);
bool PaintTextureLine(PDIRECT3DTEXTURE9 texture, int x0, int y0, int x1, int y1,
    D3DCOLOR color);
bool SaveTextureToImageFileAtomic(const char* filename, PDIRECT3DTEXTURE9 texture,
    char* errorText, size_t errorTextSize);
bool CreateSolidImageFileAtomic(const char* filename, int width, int height,
    D3DCOLOR color, char* errorText, size_t errorTextSize);
bool MergeTextureRegionToImageFileAtomic(const char* filename,
    PDIRECT3DTEXTURE9 baseTexture, PDIRECT3DTEXTURE9 overlayTexture,
    int overlaySourceX, int overlaySourceY, int overlaySourceWidth,
    int overlaySourceHeight, int overlayDestinationX, int overlayDestinationY,
    bool expandCanvas, int* outputWidth, int* outputHeight,
    char* errorText, size_t errorTextSize);
bool MergeTextureRegionAutoToImageFileAtomic(const char* filename,
    PDIRECT3DTEXTURE9 baseTexture, PDIRECT3DTEXTURE9 overlayTexture,
    int overlaySourceX, int overlaySourceY, int overlaySourceWidth,
    int overlaySourceHeight, int* placedX, int* placedY, bool* canvasExpanded,
    int* outputWidth, int* outputHeight, char* errorText, size_t errorTextSize);
int RunPixelPaintSelfTest();
