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
bool LoadTextureFromFile(const char* filename, D3Image* d3);
bool LoadTextureFromMemory(void* data, size_t size, PDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height);
bool LoadTextureFromRawMemory(void* data, size_t size, int width, int height, PDIRECT3DTEXTURE9* out_texture);
bool RefreshTextureByRawMemory(void* data, size_t size, PDIRECT3DTEXTURE9* out_texture);