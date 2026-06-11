#include <D3dx9tex.h>
#pragma comment(lib, "D3dx9")

#include "ImageLoader.h"


LPDIRECT3DDEVICE9 g_pd3dDevice;
// Simple helper function to load an image into a DX9 texture with common settings
bool LoadTextureFromFile(const char* filename, PDIRECT3DTEXTURE9* out_texture, int* out_width, int* out_height)
{
    PDIRECT3DTEXTURE9 texture;
    D3DXIMAGE_INFO info;
    //HRESULT hr = D3DXCreateTextureFromFileA(g_pd3dDevice, filename, &texture);
    HRESULT hr = D3DXCreateTextureFromFileEx(
            g_pd3dDevice, filename,
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

bool LoadTextureFromFile(const char* filename, D3Image* d3)
{
    // Load texture from disk
    PDIRECT3DTEXTURE9 texture;
    HRESULT hr = D3DXCreateTextureFromFileA(g_pd3dDevice, filename, &texture);
    if (hr != S_OK)
        return false;

    // Retrieve description of the texture surface so we can access its size
    D3DSURFACE_DESC my_image_desc;
    texture->GetLevelDesc(0, &my_image_desc);
    d3->texture = texture;
    d3->width = (int)my_image_desc.Width;
    d3->height = (int)my_image_desc.Height;
    return true;
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

bool RefreshTextureByRawMemory(void* data, size_t size, PDIRECT3DTEXTURE9* out_texture)
{
    IDirect3DTexture9* pTexture = *out_texture;
    D3DLOCKED_RECT lockedRect;

    if (SUCCEEDED(pTexture->LockRect(0, &lockedRect, NULL, 0)))
    {        
        BYTE* pDestBits = (BYTE*)lockedRect.pBits;
        
        memcpy(pDestBits, data, size);

        pTexture->UnlockRect(0);
        return true;
    }
    return false;
}
