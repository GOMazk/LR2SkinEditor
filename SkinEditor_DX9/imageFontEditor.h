#pragma once
#include "imageFontDocument.h"
#include <d3d9.h>
#include <memory>

// Resolve #LR2FONT once for both native Preview and the editor's font list.
// Uses DxLib's archive-aware search; an empty result means no matching file.
std::string SEFindPreviewImageFontPath(const char* requestedPath,
    const char* ownerFilePath, const char* mainSkinPath);
int CheckPreviewImageFontsCommand();

struct SEImageFontTexture {
    std::string path;
    std::shared_ptr<IDirect3DTexture9> texture;
    int width = 0, height = 0;
};
// Owned by WORKSPACE. All controls edit a draft; only Save font touches disk.
struct SEImageFontEditor {
    SEImageFontDocument document;
    std::map<int, SEImageFontTexture> textures;
    int selectedCode = -1, selectedPage = -1;
    SEImageFontGlyph glyph;
    int height = 0, margin = 0, pageIndex = 0;
    char pagePath[1024] = {}, character[32] = {}, search[128] = {};
    char sample[512] = "ABC abc 0123456789";
    float zoom = 1.0f, sampleZoom = 1.0f;
    bool fieldsDirty = false, focused = false, showBoxes = true;
    bool metricsDirty = false, pageDirty = false, glyphDirty = false;
    bool characterInvalid = false;
    std::string status, pendingPath;
    std::string archiveChoicePath;
    std::vector<std::string> archiveChoices;
    bool chooseArchiveMember = false;
    bool pendingOpen = false, pendingDelete = false;
    bool Dirty() const { return fieldsDirty || document.Dirty(); }
    void SelectGlyph(int code);
    void SelectPage(int page);
    void RefreshFields();
    bool ApplyFields();
};
