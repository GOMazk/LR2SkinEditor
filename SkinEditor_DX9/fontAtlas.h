#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Editor-only rasterization input. Never serialized into an OLRskin package.
struct SEFontAtlasRequest {
    std::wstring fontPath;
    bool isNumber = true;
    std::wstring judgementText = L"PGREAT";
    int columns = 10;
    int rows = 1;
    int cellWidth = 32;
    int cellHeight = 48;
    int padding = 2;
    int outlinePixels = 1;
    int shadowPixels = 0;
    std::uint32_t fillArgb = 0xffffffff;
    std::uint32_t outlineArgb = 0xff101010;
    std::uint32_t shadowArgb = 0x90000000;
    bool operator==(const SEFontAtlasRequest&) const = default;
};

struct SEFontAtlasBitmap {
    int width = 0;
    int height = 0;
    std::vector<std::uint32_t> pixels;
};

// Uses the same 10/11/24-frame precedence as LR2's number renderer.
int SEFontNumberFrameCount(int totalFrames);
std::wstring SEFontAtlasCellText(const SEFontAtlasRequest& request, int cellIndex);
bool RenderSEFontAtlas(const SEFontAtlasRequest& request,
    SEFontAtlasBitmap& output, std::string& error);
int RunFontAtlasSelfTest();
int RunSimpleFontApplySelfTest();

struct IDirect3DTexture9;
bool ReadSEFontImage(IDirect3DTexture9* texture, int x, int y, int width, int height,
    SEFontAtlasBitmap& output, std::string& error);
