#include <D3dx9tex.h>
#pragma comment(lib, "D3dx9")

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "ImageLoader.h"


LPDIRECT3DDEVICE9 g_pd3dDevice;

static bool ReadImageFileBytes(const char* filename,
    std::vector<unsigned char>& bytes)
{
    bytes.clear();
    if (!filename || !*filename) return false;

    // D3DX's filename overload converts the legacy byte path internally and
    // can terminate the process for CP932 names on a non-Japanese code page.
    // Win32's byte-path API fails normally instead, and the in-memory D3DX
    // overload never needs to interpret the filename.
    HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (file == INVALID_HANDLE_VALUE) return false;

    LARGE_INTEGER fileSize = {};
    const bool hasSize = GetFileSizeEx(file, &fileSize) != FALSE;
    if (!hasSize || fileSize.QuadPart <= 0 || fileSize.QuadPart > MAXDWORD) {
        CloseHandle(file);
        return false;
    }

    bytes.resize((size_t)fileSize.QuadPart);
    DWORD bytesRead = 0;
    const bool didRead = ReadFile(file, bytes.data(), (DWORD)bytes.size(),
        &bytesRead, NULL) != FALSE;
    CloseHandle(file);
    if (!didRead || bytesRead != bytes.size()) {
        bytes.clear();
        return false;
    }
    return true;
}

// Simple helper function to load an image into a DX9 texture with common settings
bool LoadTextureFromFile(const char* filename, PDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height)
{
    if (!out_texture || !out_width || !out_height) return false;
    std::vector<unsigned char> bytes;
    if (!ReadImageFileBytes(filename, bytes)) return false;

    PDIRECT3DTEXTURE9 texture = NULL;
    D3DXIMAGE_INFO info = {};
    HRESULT hr = D3DXCreateTextureFromFileInMemoryEx(
            g_pd3dDevice, bytes.data(), (UINT)bytes.size(),
            D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2,
            D3DX_DEFAULT,
            0,
            D3DFMT_UNKNOWN,
            D3DPOOL_MANAGED,
            D3DX_DEFAULT, D3DX_DEFAULT,
            0, &info, NULL, &texture);
    if (hr != S_OK)
        return false;

    // Retrieve description of the texture surface so we can access its size
    D3DSURFACE_DESC my_image_desc;
    texture->GetLevelDesc(0, &my_image_desc);
    *out_texture = texture;
    *out_width = (int)my_image_desc.Width;
    *out_height = (int)my_image_desc.Height;
    return true;
}

bool GetImageSizeFromFile(const char* filename, int* out_width, int* out_height)
{
    if (!filename || !out_width || !out_height) return false;
    std::vector<unsigned char> bytes;
    if (!ReadImageFileBytes(filename, bytes)) return false;
    D3DXIMAGE_INFO info = {};
    if (FAILED(D3DXGetImageInfoFromFileInMemory(bytes.data(),
        (UINT)bytes.size(), &info))) return false;
    *out_width = (int)info.Width;
    *out_height = (int)info.Height;
    return true;
}

bool LoadTextureFromFile(const char* filename, D3Image* d3)
{
    if (!d3) return false;
    return LoadTextureFromFile(filename, &d3->texture, &d3->width,
        &d3->height);
}
bool LoadTextureFromMemory(void* data, size_t size, PDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height)
{
    PDIRECT3DTEXTURE9 texture;
    D3DXIMAGE_INFO info;

    //HRESULT hr = D3DXCreateTextureFromFileInMemory(g_pd3dDevice, data, size, &texture);
    HRESULT hr = D3DXCreateTextureFromFileInMemoryEx(
        g_pd3dDevice, data, size,
        D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2,
        D3DX_DEFAULT,
        0,
        D3DFMT_UNKNOWN,
        D3DPOOL_MANAGED,
        D3DX_DEFAULT, D3DX_DEFAULT,
        0, &info, NULL, &texture);
    if (hr != S_OK)
        return false;
    // Retrieve description of the texture surface so we can access its size
    D3DSURFACE_DESC my_image_desc;
    texture->GetLevelDesc(0, &my_image_desc);
    *out_texture = texture;
    *out_width = (int)my_image_desc.Width;
    *out_height = (int)my_image_desc.Height;

    return true;
}

char TGAhead[18+1] = "\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x20\x03\x58\x02\x20\x00";
bool LoadTextureFromRawMemory(void* data, size_t size, int width, int height, PDIRECT3DTEXTURE9* out_texture)
{
    PDIRECT3DTEXTURE9 texture;
    D3DXIMAGE_INFO info;

    //make TGA header
    *(WORD*)&TGAhead[0x0C] = width;
    *(WORD*)&TGAhead[0x0E] = height;

    BYTE* tgafile = (BYTE*)malloc(size + 18);
    memcpy(tgafile, TGAhead, 18);
    memcpy(tgafile + 18, data, size);

    HRESULT hr = D3DXCreateTextureFromFileInMemoryEx(
        g_pd3dDevice, tgafile, size+18,
        D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2,
        D3DX_DEFAULT,
        0,
        D3DFMT_UNKNOWN,
        D3DPOOL_MANAGED,
        D3DX_DEFAULT, D3DX_DEFAULT,
        0, &info, NULL, &texture);
    free(tgafile);

    if (hr != S_OK)
        return false;
    // Retrieve description of the texture surface so we can access its size
    D3DSURFACE_DESC my_image_desc;
    texture->GetLevelDesc(0, &my_image_desc);
    *out_texture = texture;

    return true;
}

bool RefreshTextureByRawMemory(void* data, size_t size, int width, int height, PDIRECT3DTEXTURE9* out_texture)
{
    if (!data || !out_texture || !*out_texture || width <= 0 || height <= 0)
        return false;

    IDirect3DTexture9* pTexture = *out_texture;
    D3DSURFACE_DESC desc;
    if (FAILED(pTexture->GetLevelDesc(0, &desc)) ||
        desc.Width != (UINT)width || desc.Height != (UINT)height)
        return false;

    const size_t sourcePitch = (size_t)width * 4;
    if (size < sourcePitch * (size_t)height)
        return false;

    D3DLOCKED_RECT lockedRect;

    if (SUCCEEDED(pTexture->LockRect(0, &lockedRect, NULL, 0)))
    {        
        BYTE* pDestBits = (BYTE*)lockedRect.pBits;
        const BYTE* pSourceBits = (const BYTE*)data;

        // A D3D texture row may contain padding. Copying the whole image in
        // one memcpy corrupts rows whenever Pitch differs from width * 4.
        for (int y = 0; y < height; ++y)
            memcpy(pDestBits + (size_t)y * lockedRect.Pitch,
                pSourceBits + (size_t)y * sourcePitch, sourcePitch);

        pTexture->UnlockRect(0);
        return true;
    }
    return false;
}

static bool IsEditable32BitTextureFormat(D3DFORMAT format)
{
    return format == D3DFMT_A8R8G8B8 || format == D3DFMT_X8R8G8B8 ||
        format == D3DFMT_A8B8G8R8 || format == D3DFMT_X8B8G8R8;
}

static DWORD EncodeTexturePixel(D3DFORMAT format, D3DCOLOR color)
{
    const DWORD a = (color >> 24) & 0xff;
    const DWORD r = (color >> 16) & 0xff;
    const DWORD g = (color >> 8) & 0xff;
    const DWORD b = color & 0xff;
    if (format == D3DFMT_A8B8G8R8 || format == D3DFMT_X8B8G8R8)
        return ((format == D3DFMT_X8B8G8R8 ? 0xff : a) << 24) |
            (b << 16) | (g << 8) | r;
    return ((format == D3DFMT_X8R8G8B8 ? 0xff : a) << 24) |
        (r << 16) | (g << 8) | b;
}

static D3DCOLOR DecodeTexturePixel(D3DFORMAT format, DWORD pixel)
{
    const DWORD a = (pixel >> 24) & 0xff;
    if (format == D3DFMT_A8B8G8R8 || format == D3DFMT_X8B8G8R8) {
        const DWORD b = (pixel >> 16) & 0xff;
        const DWORD g = (pixel >> 8) & 0xff;
        const DWORD r = pixel & 0xff;
        return D3DCOLOR_ARGB(
            format == D3DFMT_X8B8G8R8 ? 0xff : a, r, g, b);
    }
    const DWORD r = (pixel >> 16) & 0xff;
    const DWORD g = (pixel >> 8) & 0xff;
    const DWORD b = pixel & 0xff;
    return D3DCOLOR_ARGB(format == D3DFMT_X8R8G8B8 ? 0xff : a, r, g, b);
}

bool ReadTexturePixel(PDIRECT3DTEXTURE9 texture, int x, int y, D3DCOLOR* color)
{
    if (!texture || !color) return false;
    D3DSURFACE_DESC desc = {};
    if (FAILED(texture->GetLevelDesc(0, &desc)) ||
        !IsEditable32BitTextureFormat(desc.Format) || x < 0 || y < 0 ||
        x >= (int)desc.Width || y >= (int)desc.Height) return false;
    D3DLOCKED_RECT locked = {};
    if (FAILED(texture->LockRect(0, &locked, NULL, D3DLOCK_READONLY))) return false;
    const DWORD pixel = *(const DWORD*)((const BYTE*)locked.pBits +
        (size_t)y * locked.Pitch + (size_t)x * sizeof(DWORD));
    texture->UnlockRect(0);
    *color = DecodeTexturePixel(desc.Format, pixel);
    return true;
}

bool PaintTextureLine(PDIRECT3DTEXTURE9 texture, int x0, int y0, int x1, int y1,
    D3DCOLOR color)
{
    if (!texture) return false;
    D3DSURFACE_DESC desc = {};
    if (FAILED(texture->GetLevelDesc(0, &desc)) ||
        !IsEditable32BitTextureFormat(desc.Format)) return false;
    x0 = (std::max)(0, (std::min)(x0, (int)desc.Width - 1));
    y0 = (std::max)(0, (std::min)(y0, (int)desc.Height - 1));
    x1 = (std::max)(0, (std::min)(x1, (int)desc.Width - 1));
    y1 = (std::max)(0, (std::min)(y1, (int)desc.Height - 1));

    D3DLOCKED_RECT locked = {};
    if (FAILED(texture->LockRect(0, &locked, NULL, 0))) return false;
    const DWORD encoded = EncodeTexturePixel(desc.Format, color);
    int x = x0;
    int y = y0;
    const int dx = abs(x1 - x0);
    const int sx = x0 < x1 ? 1 : -1;
    const int dy = -abs(y1 - y0);
    const int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;
    for (;;) {
        *(DWORD*)((BYTE*)locked.pBits + (size_t)y * locked.Pitch +
            (size_t)x * sizeof(DWORD)) = encoded;
        if (x == x1 && y == y1) break;
        const int twiceError = error * 2;
        if (twiceError >= dy) { error += dy; x += sx; }
        if (twiceError <= dx) { error += dx; y += sy; }
    }
    texture->UnlockRect(0);
    return true;
}

static D3DXIMAGE_FILEFORMAT ImageFormatForPath(const char* filename)
{
    const char* extension = filename ? strrchr(filename, '.') : NULL;
    std::string ext = extension ? extension : "";
    std::transform(ext.begin(), ext.end(), ext.begin(),
        [](unsigned char value) { return (char)std::tolower(value); });
    if (ext == ".bmp" || ext == ".dib") return D3DXIFF_BMP;
    if (ext == ".jpg" || ext == ".jpeg") return D3DXIFF_JPG;
    if (ext == ".tga") return D3DXIFF_TGA;
    if (ext == ".dds") return D3DXIFF_DDS;
    if (ext == ".ppm") return D3DXIFF_PPM;
    if (ext == ".hdr") return D3DXIFF_HDR;
    if (ext == ".pfm") return D3DXIFF_PFM;
    return D3DXIFF_PNG;
}

static void SetImageSaveError(char* errorText, size_t errorTextSize,
    const char* message)
{
    if (!errorText || errorTextSize == 0) return;
    snprintf(errorText, errorTextSize, "%s", message ? message : "Unknown error");
}

static bool SaveNewTextureToImageFileAtomic(const char* filename,
    PDIRECT3DTEXTURE9 texture, char* errorText, size_t errorTextSize)
{
    if (errorText && errorTextSize) errorText[0] = '\0';
    if (!filename || !*filename || !texture) {
        SetImageSaveError(errorText, errorTextSize,
            "No output image path was provided.");
        return false;
    }
    if (GetFileAttributesA(filename) != INVALID_FILE_ATTRIBUTES) {
        SetImageSaveError(errorText, errorTextSize,
            "The output image already exists. Choose another filename.");
        return false;
    }

    std::string parent = filename;
    const size_t separator = parent.find_last_of("\\/");
    if (separator != std::string::npos) {
        parent.resize(separator);
        const DWORD parentAttributes = GetFileAttributesA(parent.c_str());
        if (parentAttributes == INVALID_FILE_ATTRIBUTES ||
            !(parentAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            SetImageSaveError(errorText, errorTextSize,
                "The output folder does not exist.");
            return false;
        }
    }

    const std::string temporaryPath = std::string(filename) +
        ".skineditor-create.tmp";
    DeleteFileA(temporaryPath.c_str());
    if (FAILED(D3DXSaveTextureToFileA(temporaryPath.c_str(),
        ImageFormatForPath(filename), texture, NULL))) {
        DeleteFileA(temporaryPath.c_str());
        SetImageSaveError(errorText, errorTextSize,
            "Direct3D could not encode the new image.");
        return false;
    }
    if (!MoveFileExA(temporaryPath.c_str(), filename, MOVEFILE_WRITE_THROUGH)) {
        DeleteFileA(temporaryPath.c_str());
        SetImageSaveError(errorText, errorTextSize,
            "Could not create the output image file.");
        return false;
    }
    return true;
}

static bool TextureToArgbPixels(PDIRECT3DTEXTURE9 texture,
    std::vector<D3DCOLOR>& pixels, int& width, int& height)
{
    if (!texture) return false;
    D3DSURFACE_DESC desc = {};
    if (FAILED(texture->GetLevelDesc(0, &desc)) ||
        !IsEditable32BitTextureFormat(desc.Format) ||
        desc.Width == 0 || desc.Height == 0) return false;
    D3DLOCKED_RECT locked = {};
    if (FAILED(texture->LockRect(0, &locked, NULL, D3DLOCK_READONLY)))
        return false;
    width = (int)desc.Width;
    height = (int)desc.Height;
    pixels.resize((size_t)width * height);
    for (int y = 0; y < height; ++y) {
        const DWORD* source = (const DWORD*)((const BYTE*)locked.pBits +
            (size_t)y * locked.Pitch);
        for (int x = 0; x < width; ++x)
            pixels[(size_t)y * width + x] =
                DecodeTexturePixel(desc.Format, source[x]);
    }
    texture->UnlockRect(0);
    return true;
}

static bool CreateArgbTexture(const std::vector<D3DCOLOR>& pixels,
    int width, int height, PDIRECT3DTEXTURE9* output)
{
    if (!g_pd3dDevice || !output || width <= 0 || height <= 0 ||
        pixels.size() < (size_t)width * height) return false;
    *output = NULL;
    if (FAILED(g_pd3dDevice->CreateTexture(width, height, 1, 0,
        D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, output, NULL))) return false;
    D3DLOCKED_RECT locked = {};
    if (FAILED((*output)->LockRect(0, &locked, NULL, 0))) {
        (*output)->Release();
        *output = NULL;
        return false;
    }
    for (int y = 0; y < height; ++y)
        memcpy((BYTE*)locked.pBits + (size_t)y * locked.Pitch,
            pixels.data() + (size_t)y * width, (size_t)width * sizeof(D3DCOLOR));
    (*output)->UnlockRect(0);
    return true;
}

static bool SaveArgbPixelsToImageFileAtomic(const char* filename,
    int width, int height, const std::vector<D3DCOLOR>& pixels,
    char* errorText, size_t errorTextSize)
{
    PDIRECT3DTEXTURE9 texture = NULL;
    if (!CreateArgbTexture(pixels, width, height, &texture)) {
        SetImageSaveError(errorText, errorTextSize,
            "Direct3D could not allocate the new image.");
        return false;
    }
    const bool saved = SaveNewTextureToImageFileAtomic(filename, texture,
        errorText, errorTextSize);
    texture->Release();
    return saved;
}

static float ClampColorUnit(float value)
{
    return (std::max)(0.0f, (std::min)(1.0f, value));
}

static void RgbToHsv(float red, float green, float blue,
    float& hue, float& saturation, float& value)
{
    const float maximum = (std::max)(red, (std::max)(green, blue));
    const float minimum = (std::min)(red, (std::min)(green, blue));
    const float delta = maximum - minimum;
    value = maximum;
    saturation = maximum <= 0.0f ? 0.0f : delta / maximum;
    if (delta <= 0.0f) hue = 0.0f;
    else if (maximum == red)
        hue = fmodf((green - blue) / delta, 6.0f) / 6.0f;
    else if (maximum == green)
        hue = ((blue - red) / delta + 2.0f) / 6.0f;
    else
        hue = ((red - green) / delta + 4.0f) / 6.0f;
    if (hue < 0.0f) hue += 1.0f;
}

static void HsvToRgb(float hue, float saturation, float value,
    float& red, float& green, float& blue)
{
    hue -= floorf(hue);
    const float sector = hue * 6.0f;
    const int index = (int)floorf(sector) % 6;
    const float fraction = sector - floorf(sector);
    const float p = value * (1.0f - saturation);
    const float q = value * (1.0f - fraction * saturation);
    const float t = value * (1.0f - (1.0f - fraction) * saturation);
    switch (index) {
    case 0: red = value; green = t; blue = p; break;
    case 1: red = q; green = value; blue = p; break;
    case 2: red = p; green = value; blue = t; break;
    case 3: red = p; green = q; blue = value; break;
    case 4: red = t; green = p; blue = value; break;
    default: red = value; green = p; blue = q; break;
    }
}

bool CreateColorAdjustedImageRegionAtomic(const char* filename,
    PDIRECT3DTEXTURE9 sourceTexture, int sourceX, int sourceY,
    int sourceWidth, int sourceHeight, float hueShiftDegrees,
    float saturationScale, float brightnessScale,
    char* errorText, size_t errorTextSize)
{
    if (errorText && errorTextSize) errorText[0] = '\0';
    std::vector<D3DCOLOR> sourcePixels;
    int textureWidth = 0;
    int textureHeight = 0;
    if (!TextureToArgbPixels(sourceTexture, sourcePixels,
        textureWidth, textureHeight)) {
        SetImageSaveError(errorText, errorTextSize,
            "The source image pixels could not be read.");
        return false;
    }
    if (sourceX < 0 || sourceY < 0 || sourceWidth <= 0 || sourceHeight <= 0 ||
        sourceX > textureWidth - sourceWidth ||
        sourceY > textureHeight - sourceHeight ||
        sourceWidth > 16384 || sourceHeight > 16384) {
        SetImageSaveError(errorText, errorTextSize,
            "The selected atlas region is outside the source image.");
        return false;
    }

    saturationScale = (std::max)(0.0f, (std::min)(2.0f, saturationScale));
    brightnessScale = (std::max)(0.0f, (std::min)(2.0f, brightnessScale));
    const float hueShift = hueShiftDegrees / 360.0f;
    std::vector<D3DCOLOR> outputPixels((size_t)sourceWidth * sourceHeight);
    for (int y = 0; y < sourceHeight; ++y) {
        for (int x = 0; x < sourceWidth; ++x) {
            const D3DCOLOR pixel = sourcePixels[
                (size_t)(sourceY + y) * textureWidth + sourceX + x];
            const unsigned int alpha = (pixel >> 24) & 0xff;
            float red = ((pixel >> 16) & 0xff) / 255.0f;
            float green = ((pixel >> 8) & 0xff) / 255.0f;
            float blue = (pixel & 0xff) / 255.0f;
            float hue = 0.0f;
            float saturation = 0.0f;
            float value = 0.0f;
            RgbToHsv(red, green, blue, hue, saturation, value);
            hue += hueShift;
            saturation = ClampColorUnit(saturation * saturationScale);
            value = ClampColorUnit(value * brightnessScale);
            HsvToRgb(hue, saturation, value, red, green, blue);
            outputPixels[(size_t)y * sourceWidth + x] = D3DCOLOR_ARGB(alpha,
                (unsigned int)(ClampColorUnit(red) * 255.0f + 0.5f),
                (unsigned int)(ClampColorUnit(green) * 255.0f + 0.5f),
                (unsigned int)(ClampColorUnit(blue) * 255.0f + 0.5f));
        }
    }

    PDIRECT3DTEXTURE9 outputTexture = NULL;
    if (!CreateArgbTexture(outputPixels, sourceWidth, sourceHeight,
        &outputTexture)) {
        SetImageSaveError(errorText, errorTextSize,
            "Direct3D could not allocate the adjusted image.");
        return false;
    }
    const bool saved = SaveNewTextureToImageFileAtomic(filename, outputTexture,
        errorText, errorTextSize);
    outputTexture->Release();
    return saved;
}

bool CreateSolidImageFileAtomic(const char* filename, int width, int height,
    D3DCOLOR color, char* errorText, size_t errorTextSize)
{
    if (width <= 0 || height <= 0 || width > 16384 || height > 16384) {
        SetImageSaveError(errorText, errorTextSize,
            "Image dimensions must be between 1 and 16384 pixels.");
        return false;
    }
    std::vector<D3DCOLOR> pixels((size_t)width * height, color);
    return SaveArgbPixelsToImageFileAtomic(filename, width, height, pixels,
        errorText, errorTextSize);
}

bool CreateArgbImageFileAtomic(const char* filename, int width, int height,
    const D3DCOLOR* pixels, size_t pixelCount,
    char* errorText, size_t errorTextSize)
{
    if (width <= 0 || height <= 0 || width > 16384 || height > 16384) {
        SetImageSaveError(errorText, errorTextSize,
            "Image dimensions must be between 1 and 16384 pixels.");
        return false;
    }
    const size_t requiredPixelCount = (size_t)width * height;
    if (!pixels || pixelCount < requiredPixelCount) {
        SetImageSaveError(errorText, errorTextSize,
            "The ARGB pixel buffer is smaller than the requested image.");
        return false;
    }
    const std::vector<D3DCOLOR> pixelCopy(pixels,
        pixels + requiredPixelCount);
    return SaveArgbPixelsToImageFileAtomic(filename, width, height, pixelCopy,
        errorText, errorTextSize);
}

static D3DCOLOR AlphaComposite(D3DCOLOR destination, D3DCOLOR source)
{
    const int sourceA = (source >> 24) & 0xff;
    if (sourceA <= 0) return destination;
    if (sourceA >= 255) return source;
    const int destinationA = (destination >> 24) & 0xff;
    const int inverseSourceA = 255 - sourceA;
    const int outputA = sourceA + (destinationA * inverseSourceA + 127) / 255;
    if (outputA <= 0) return 0;
    auto compositeChannel = [&](int destinationChannel, int sourceChannel) {
        const int premultiplied = sourceChannel * sourceA +
            (destinationChannel * destinationA * inverseSourceA + 127) / 255;
        return (premultiplied + outputA / 2) / outputA;
    };
    const int outputR = compositeChannel((destination >> 16) & 0xff,
        (source >> 16) & 0xff);
    const int outputG = compositeChannel((destination >> 8) & 0xff,
        (source >> 8) & 0xff);
    const int outputB = compositeChannel(destination & 0xff, source & 0xff);
    return D3DCOLOR_ARGB(outputA, outputR, outputG, outputB);
}

bool MergeTextureRegionToImageFileAtomic(const char* filename,
    PDIRECT3DTEXTURE9 baseTexture, PDIRECT3DTEXTURE9 overlayTexture,
    int overlaySourceX, int overlaySourceY, int overlaySourceWidth,
    int overlaySourceHeight, int overlayDestinationX, int overlayDestinationY,
    bool expandCanvas, int* outputWidth, int* outputHeight,
    char* errorText, size_t errorTextSize)
{
    std::vector<D3DCOLOR> basePixels;
    std::vector<D3DCOLOR> overlayPixels;
    int baseWidth = 0;
    int baseHeight = 0;
    int overlayWidth = 0;
    int overlayHeight = 0;
    if (!TextureToArgbPixels(baseTexture, basePixels, baseWidth, baseHeight) ||
        !TextureToArgbPixels(overlayTexture, overlayPixels, overlayWidth,
            overlayHeight)) {
        SetImageSaveError(errorText, errorTextSize,
            "One of the source textures cannot be read as 32-bit pixels.");
        return false;
    }
    if (overlaySourceWidth < 0) overlaySourceWidth = overlayWidth - overlaySourceX;
    if (overlaySourceHeight < 0) overlaySourceHeight = overlayHeight - overlaySourceY;
    if (overlaySourceX < 0 || overlaySourceY < 0 || overlaySourceWidth <= 0 ||
        overlaySourceHeight <= 0 ||
        overlaySourceX + overlaySourceWidth > overlayWidth ||
        overlaySourceY + overlaySourceHeight > overlayHeight) {
        SetImageSaveError(errorText, errorTextSize,
            "The overlay asset crop is outside its texture.");
        return false;
    }

    int canvasLeft = 0;
    int canvasTop = 0;
    int canvasRight = baseWidth;
    int canvasBottom = baseHeight;
    if (expandCanvas) {
        canvasLeft = (std::min)(canvasLeft, overlayDestinationX);
        canvasTop = (std::min)(canvasTop, overlayDestinationY);
        canvasRight = (std::max)(canvasRight,
            overlayDestinationX + overlaySourceWidth);
        canvasBottom = (std::max)(canvasBottom,
            overlayDestinationY + overlaySourceHeight);
    }
    const int mergedWidth = canvasRight - canvasLeft;
    const int mergedHeight = canvasBottom - canvasTop;
    if (mergedWidth <= 0 || mergedHeight <= 0 || mergedWidth > 16384 ||
        mergedHeight > 16384) {
        SetImageSaveError(errorText, errorTextSize,
            "The merged canvas is outside the supported size.");
        return false;
    }

    std::vector<D3DCOLOR> merged((size_t)mergedWidth * mergedHeight, 0);
    const int baseOffsetX = -canvasLeft;
    const int baseOffsetY = -canvasTop;
    for (int y = 0; y < baseHeight; ++y)
        for (int x = 0; x < baseWidth; ++x)
            merged[(size_t)(y + baseOffsetY) * mergedWidth + x + baseOffsetX] =
                basePixels[(size_t)y * baseWidth + x];

    const int destinationOffsetX = overlayDestinationX - canvasLeft;
    const int destinationOffsetY = overlayDestinationY - canvasTop;
    for (int sourceY = 0; sourceY < overlaySourceHeight; ++sourceY) {
        const int destinationY = destinationOffsetY + sourceY;
        if (destinationY < 0 || destinationY >= mergedHeight) continue;
        for (int sourceX = 0; sourceX < overlaySourceWidth; ++sourceX) {
            const int destinationX = destinationOffsetX + sourceX;
            if (destinationX < 0 || destinationX >= mergedWidth) continue;
            const D3DCOLOR source = overlayPixels[
                (size_t)(overlaySourceY + sourceY) * overlayWidth +
                overlaySourceX + sourceX];
            D3DCOLOR& destination = merged[
                (size_t)destinationY * mergedWidth + destinationX];
            destination = AlphaComposite(destination, source);
        }
    }

    PDIRECT3DTEXTURE9 mergedTexture = NULL;
    if (!CreateArgbTexture(merged, mergedWidth, mergedHeight, &mergedTexture)) {
        SetImageSaveError(errorText, errorTextSize,
            "Direct3D could not allocate the merged image.");
        return false;
    }
    const bool saved = SaveNewTextureToImageFileAtomic(filename, mergedTexture,
        errorText, errorTextSize);
    mergedTexture->Release();
    if (saved) {
        if (outputWidth) *outputWidth = mergedWidth;
        if (outputHeight) *outputHeight = mergedHeight;
    }
    return saved;
}

bool MergeTextureRegionAutoToImageFileAtomic(const char* filename,
    PDIRECT3DTEXTURE9 baseTexture, PDIRECT3DTEXTURE9 overlayTexture,
    int overlaySourceX, int overlaySourceY, int overlaySourceWidth,
    int overlaySourceHeight, int* placedX, int* placedY, bool* canvasExpanded,
    int* outputWidth, int* outputHeight, char* errorText, size_t errorTextSize)
{
    std::vector<D3DCOLOR> basePixels;
    int baseWidth = 0;
    int baseHeight = 0;
    if (!TextureToArgbPixels(baseTexture, basePixels, baseWidth, baseHeight)) {
        SetImageSaveError(errorText, errorTextSize,
            "The base texture cannot be read as 32-bit pixels.");
        return false;
    }
    D3DSURFACE_DESC overlayDescription = {};
    if (!overlayTexture || FAILED(overlayTexture->GetLevelDesc(0,
        &overlayDescription))) {
        SetImageSaveError(errorText, errorTextSize,
            "The overlay texture is unavailable.");
        return false;
    }
    const int overlayTextureWidth = (int)overlayDescription.Width;
    const int overlayTextureHeight = (int)overlayDescription.Height;
    if (overlaySourceWidth < 0)
        overlaySourceWidth = overlayTextureWidth - overlaySourceX;
    if (overlaySourceHeight < 0)
        overlaySourceHeight = overlayTextureHeight - overlaySourceY;
    if (overlaySourceX < 0 || overlaySourceY < 0 ||
        overlaySourceWidth <= 0 || overlaySourceHeight <= 0 ||
        overlaySourceX + overlaySourceWidth > overlayTextureWidth ||
        overlaySourceY + overlaySourceHeight > overlayTextureHeight) {
        SetImageSaveError(errorText, errorTextSize,
            "The overlay asset crop is outside its texture.");
        return false;
    }

    int destinationX = -1;
    int destinationY = -1;
    bool expanded = false;

    // One pass over the alpha channel finds the first full-size transparent
    // rectangle without allocating a width*height integral-image table.
    if (overlaySourceWidth <= baseWidth && overlaySourceHeight <= baseHeight) {
        std::vector<int> transparentHeight(baseWidth, 0);
        for (int y = 0; y < baseHeight && destinationX < 0; ++y) {
            for (int x = 0; x < baseWidth; ++x) {
                const bool transparent =
                    ((basePixels[(size_t)y * baseWidth + x] >> 24) & 0xff) == 0;
                transparentHeight[x] = transparent
                    ? transparentHeight[x] + 1 : 0;
            }
            if (y + 1 < overlaySourceHeight) continue;
            int horizontalRun = 0;
            for (int x = 0; x < baseWidth; ++x) {
                if (transparentHeight[x] >= overlaySourceHeight)
                    ++horizontalRun;
                else
                    horizontalRun = 0;
                if (horizontalRun >= overlaySourceWidth) {
                    destinationX = x - overlaySourceWidth + 1;
                    destinationY = y - overlaySourceHeight + 1;
                    break;
                }
            }
        }
    }

    if (destinationX < 0) {
        expanded = true;
        const long long rightWidth = (long long)baseWidth + overlaySourceWidth;
        const long long rightHeight = (std::max)(baseHeight, overlaySourceHeight);
        const long long bottomWidth = (std::max)(baseWidth, overlaySourceWidth);
        const long long bottomHeight = (long long)baseHeight + overlaySourceHeight;
        const long long rightArea = rightWidth * rightHeight;
        const long long bottomArea = bottomWidth * bottomHeight;
        if (rightArea <= bottomArea) {
            destinationX = baseWidth;
            destinationY = 0;
        } else {
            destinationX = 0;
            destinationY = baseHeight;
        }
    }

    const bool saved = MergeTextureRegionToImageFileAtomic(filename,
        baseTexture, overlayTexture, overlaySourceX, overlaySourceY,
        overlaySourceWidth, overlaySourceHeight, destinationX, destinationY,
        expanded, outputWidth, outputHeight, errorText, errorTextSize);
    if (saved) {
        if (placedX) *placedX = destinationX;
        if (placedY) *placedY = destinationY;
        if (canvasExpanded) *canvasExpanded = expanded;
    }
    return saved;
}

bool SaveTextureToImageFileAtomic(const char* filename, PDIRECT3DTEXTURE9 texture,
    char* errorText, size_t errorTextSize)
{
    if (errorText && errorTextSize) errorText[0] = '\0';
    if (!filename || !*filename || !texture) {
        SetImageSaveError(errorText, errorTextSize, "No writable image is selected.");
        return false;
    }

    const DWORD attributes = GetFileAttributesA(filename);
    if (attributes == INVALID_FILE_ATTRIBUTES ||
        (attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_READONLY))) {
        SetImageSaveError(errorText, errorTextSize,
            "The source image is missing or read-only.");
        return false;
    }

    const std::string backupPath = std::string(filename) + ".skineditor-pixel.bak";
    if (!CopyFileA(filename, backupPath.c_str(), TRUE) &&
        GetLastError() != ERROR_FILE_EXISTS) {
        SetImageSaveError(errorText, errorTextSize,
            "Could not create the original-image backup.");
        return false;
    }

    const std::string temporaryPath = std::string(filename) + ".skineditor-pixel.tmp";
    DeleteFileA(temporaryPath.c_str());
    const HRESULT saveResult = D3DXSaveTextureToFileA(temporaryPath.c_str(),
        ImageFormatForPath(filename), texture, NULL);
    if (FAILED(saveResult)) {
        DeleteFileA(temporaryPath.c_str());
        SetImageSaveError(errorText, errorTextSize,
            "Direct3D could not encode the edited image.");
        return false;
    }
    if (!MoveFileExA(temporaryPath.c_str(), filename,
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        DeleteFileA(temporaryPath.c_str());
        SetImageSaveError(errorText, errorTextSize,
            "Could not replace the source image file.");
        return false;
    }
    return true;
}

int RunPixelPaintSelfTest()
{
    if (!g_pd3dDevice) return 40;
    char tempDirectory[MAX_PATH] = {};
    if (!GetTempPathA(MAX_PATH, tempDirectory)) return 41;
    char imagePath[MAX_PATH] = {};
    snprintf(imagePath, sizeof(imagePath), "%sSkinEditor_pixel_%lu.png",
        tempDirectory, GetCurrentProcessId());
    const std::string backupPath = std::string(imagePath) +
        ".skineditor-pixel.bak";
    const std::string temporaryPath = std::string(imagePath) +
        ".skineditor-pixel.tmp";
    DeleteFileA(imagePath);
    DeleteFileA(backupPath.c_str());
    DeleteFileA(temporaryPath.c_str());

    PDIRECT3DTEXTURE9 texture = NULL;
    if (FAILED(g_pd3dDevice->CreateTexture(4, 4, 1, 0, D3DFMT_A8R8G8B8,
        D3DPOOL_MANAGED, &texture, NULL))) return 42;
    for (int y = 0; y < 4; ++y)
        if (!PaintTextureLine(texture, 0, y, 3, y,
            D3DCOLOR_ARGB(0, 0, 0, 0))) {
            texture->Release();
            return 43;
        }
    if (FAILED(D3DXSaveTextureToFileA(imagePath, D3DXIFF_PNG, texture, NULL))) {
        texture->Release();
        return 44;
    }

    const D3DCOLOR expected = D3DCOLOR_ARGB(255, 241, 37, 99);
    if (!PaintTextureLine(texture, 0, 0, 3, 3, expected)) {
        texture->Release();
        return 45;
    }
    D3DCOLOR painted = 0;
    if (!ReadTexturePixel(texture, 2, 2, &painted) || painted != expected) {
        texture->Release();
        return 46;
    }
    char errorText[256] = {};
    if (!SaveTextureToImageFileAtomic(imagePath, texture,
        errorText, sizeof(errorText))) {
        texture->Release();
        return 47;
    }
    texture->Release();

    PDIRECT3DTEXTURE9 savedTexture = NULL;
    int width = 0;
    int height = 0;
    if (!LoadTextureFromFile(imagePath, &savedTexture, &width, &height) ||
        width != 4 || height != 4) return 48;
    D3DCOLOR savedPixel = 0;
    const bool savedPixelMatches =
        ReadTexturePixel(savedTexture, 2, 2, &savedPixel) && savedPixel == expected;
    savedTexture->Release();
    if (!savedPixelMatches || GetFileAttributesA(backupPath.c_str()) ==
        INVALID_FILE_ATTRIBUTES) return 49;

    char solidPath[MAX_PATH] = {};
    char mergedPath[MAX_PATH] = {};
    char packedPath[MAX_PATH] = {};
    char adjustedPath[MAX_PATH] = {};
    snprintf(solidPath, sizeof(solidPath), "%sSkinEditor_solid_%lu.png",
        tempDirectory, GetCurrentProcessId());
    snprintf(mergedPath, sizeof(mergedPath), "%sSkinEditor_merged_%lu.png",
        tempDirectory, GetCurrentProcessId());
    snprintf(packedPath, sizeof(packedPath), "%sSkinEditor_packed_%lu.png",
        tempDirectory, GetCurrentProcessId());
    snprintf(adjustedPath, sizeof(adjustedPath), "%sSkinEditor_adjusted_%lu.png",
        tempDirectory, GetCurrentProcessId());
    DeleteFileA(solidPath);
    DeleteFileA(mergedPath);
    DeleteFileA(packedPath);
    DeleteFileA(adjustedPath);
    DeleteFileA((std::string(solidPath) + ".skineditor-create.tmp").c_str());
    DeleteFileA((std::string(mergedPath) + ".skineditor-create.tmp").c_str());
    DeleteFileA((std::string(packedPath) + ".skineditor-create.tmp").c_str());
    DeleteFileA((std::string(adjustedPath) + ".skineditor-create.tmp").c_str());
    const D3DCOLOR solidBlue = D3DCOLOR_ARGB(77, 14, 32, 210);
    if (!CreateSolidImageFileAtomic(solidPath, 3, 2, solidBlue,
        errorText, sizeof(errorText))) return 50;

    PDIRECT3DTEXTURE9 baseTexture = NULL;
    if (!LoadTextureFromFile(solidPath, &baseTexture, &width, &height) ||
        width != 3 || height != 2) return 51;
    D3DCOLOR solidPixel = 0;
    if (!ReadTexturePixel(baseTexture, 1, 1, &solidPixel) ||
        solidPixel != solidBlue) {
        baseTexture->Release();
        return 52;
    }
    if (!CreateColorAdjustedImageRegionAtomic(adjustedPath, baseTexture,
        1, 0, 2, 2, 120.0f, 1.0f, 1.0f,
        errorText, sizeof(errorText))) {
        baseTexture->Release();
        return 61;
    }
    PDIRECT3DTEXTURE9 adjustedTexture = NULL;
    if (!LoadTextureFromFile(adjustedPath, &adjustedTexture, &width, &height) ||
        width != 2 || height != 2) {
        baseTexture->Release();
        return 62;
    }
    D3DCOLOR adjustedPixel = 0;
    const bool adjustedPixelMatches = ReadTexturePixel(adjustedTexture,
        0, 0, &adjustedPixel) && ((adjustedPixel >> 24) & 0xff) == 77 &&
        ((adjustedPixel >> 16) & 0xff) > (adjustedPixel & 0xff);
    adjustedTexture->Release();
    if (!adjustedPixelMatches) {
        baseTexture->Release();
        return 63;
    }

    PDIRECT3DTEXTURE9 overlayTexture = NULL;
    if (FAILED(g_pd3dDevice->CreateTexture(2, 2, 1, 0, D3DFMT_A8R8G8B8,
        D3DPOOL_MANAGED, &overlayTexture, NULL))) {
        baseTexture->Release();
        return 53;
    }
    const D3DCOLOR solidRed = D3DCOLOR_ARGB(255, 220, 18, 27);
    for (int y = 0; y < 2; ++y)
        PaintTextureLine(overlayTexture, 0, y, 1, y, solidRed);
    int mergedWidth = 0;
    int mergedHeight = 0;
    int placedX = -1;
    int placedY = -1;
    bool canvasExpanded = false;
    const bool merged = MergeTextureRegionAutoToImageFileAtomic(mergedPath,
        baseTexture, overlayTexture, 0, 0, 2, 2, &placedX, &placedY,
        &canvasExpanded, &mergedWidth, &mergedHeight,
        errorText, sizeof(errorText));
    if (!merged || mergedWidth != 5 || mergedHeight != 2 ||
        placedX != 3 || placedY != 0 || !canvasExpanded) {
        overlayTexture->Release();
        baseTexture->Release();
        return 54;
    }

    PDIRECT3DTEXTURE9 mergedTexture = NULL;
    if (!LoadTextureFromFile(mergedPath, &mergedTexture, &width, &height) ||
        width != 5 || height != 2) return 55;
    D3DCOLOR mergedBasePixel = 0;
    D3DCOLOR mergedOverlayPixel = 0;
    const bool mergePixelsMatch =
        ReadTexturePixel(mergedTexture, 0, 0, &mergedBasePixel) &&
        ReadTexturePixel(mergedTexture, 4, 1, &mergedOverlayPixel) &&
        mergedBasePixel == solidBlue && mergedOverlayPixel == solidRed;
    mergedTexture->Release();
    if (!mergePixelsMatch) return 56;

    PDIRECT3DTEXTURE9 holeBaseTexture = NULL;
    if (FAILED(g_pd3dDevice->CreateTexture(4, 4, 1, 0, D3DFMT_A8R8G8B8,
        D3DPOOL_MANAGED, &holeBaseTexture, NULL))) {
        overlayTexture->Release();
        baseTexture->Release();
        return 57;
    }
    for (int y = 0; y < 4; ++y)
        PaintTextureLine(holeBaseTexture, 0, y, 3, y, solidBlue);
    PaintTextureLine(holeBaseTexture, 1, 1, 2, 1,
        D3DCOLOR_ARGB(0, 0, 0, 0));
    PaintTextureLine(holeBaseTexture, 1, 2, 2, 2,
        D3DCOLOR_ARGB(0, 0, 0, 0));
    placedX = -1;
    placedY = -1;
    canvasExpanded = true;
    mergedWidth = 0;
    mergedHeight = 0;
    const bool packed = MergeTextureRegionAutoToImageFileAtomic(packedPath,
        holeBaseTexture, overlayTexture, 0, 0, 2, 2, &placedX, &placedY,
        &canvasExpanded, &mergedWidth, &mergedHeight,
        errorText, sizeof(errorText));
    holeBaseTexture->Release();
    overlayTexture->Release();
    baseTexture->Release();
    if (!packed || mergedWidth != 4 || mergedHeight != 4 ||
        placedX != 1 || placedY != 1 || canvasExpanded) return 58;

    PDIRECT3DTEXTURE9 packedTexture = NULL;
    if (!LoadTextureFromFile(packedPath, &packedTexture, &width, &height) ||
        width != 4 || height != 4) return 59;
    D3DCOLOR packedPixel = 0;
    const bool packedPixelMatches =
        ReadTexturePixel(packedTexture, 1, 1, &packedPixel) &&
        packedPixel == solidRed;
    packedTexture->Release();
    if (!packedPixelMatches) return 60;

    DeleteFileA(imagePath);
    DeleteFileA(backupPath.c_str());
    DeleteFileA(temporaryPath.c_str());
    DeleteFileA(solidPath);
    DeleteFileA(mergedPath);
    DeleteFileA(packedPath);
    DeleteFileA(adjustedPath);
    DeleteFileA((std::string(solidPath) + ".skineditor-create.tmp").c_str());
    DeleteFileA((std::string(mergedPath) + ".skineditor-create.tmp").c_str());
    DeleteFileA((std::string(packedPath) + ".skineditor-create.tmp").c_str());
    DeleteFileA((std::string(adjustedPath) + ".skineditor-create.tmp").c_str());
    return 0;
}
