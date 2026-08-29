#pragma once
#include "../LR2/structure.h"
#include "arr.hpp"
#include <imgui.h>
#include "ImageLoader.h"
#include "seObjectEditor.h"
#include "skinResolution.h"
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

enum DOCUMENTCHANGE {
    DOCUMENT_CHANGE_VALUE = 1 << 0,
    DOCUMENT_CHANGE_STRUCTURE = 1 << 1,
    DOCUMENT_CHANGE_OBJECT_METADATA = 1 << 2,
};

struct SEObjectSelectionKey {
    std::string editorId;
    int group = -1;
    int anchorRow = -1;

    bool IsValid() const { return !editorId.empty() || group >= 0 || anchorRow >= 0; }
};

struct SEObjectSelectionState {
    std::vector<SEObjectSelectionKey> selected;
    SEObjectSelectionKey active;
    SEObjectSelectionKey anchor;
    bool focusBrowserRequested = false;
};

struct SkinLineSnapshot {
    std::string filename;
    std::string line;
    int num = 0;
    bool modified = false;
    bool show = true;
};

struct SkinDocumentSnapshot {
    std::vector<SkinLineSnapshot> lines;
    SEObjectSelectionState selection;
};

typedef struct SKINFILELINEREAD {
    int numTotal;

    CSTR filename;
    int num;

    CSTR line;
    bool isComment;
    bool isSEcomment;
    bool modified = false;
    int csvColumnCount = 0;
    bool show = true;

    CSVbuf csv;

    int ifgroup;

    bool isIfGroupHead = false;
    bool isIfGroupEnd = false;
    bool isGroupHead = false;
    bool isGroupEnd = false;
    bool isObjectHead = false;
    bool isObjectEnd = false;

    int objID;
    int objType;
    int objInTypeID;

    bool isSRC = false;
    bool isDST = false;
    bool isOther = false;
}SKINFILELINEREAD;

typedef struct IFUNIT {
    int data[10] = {};
    int depth = 0; //
    int order = 0;
    int parentID = 0; //
    //int declare = 0;

    bool hide = 0;

    int grCount=0;
}IFUNIT;

typedef struct IMG {
    CSTR name;
    int gr, x, y, w, h;
    int ifGroup = 0;
    // Coordinates alone cannot identify the animation when duplicate SRC
    // declarations use the same crop with different div/cycle values.
    int sourceDeclare = -1;
    int editorDeclare = -1;
}IMG;

enum class SEImageDiagnosticKind {
    MissingFile,
    UnloadableFile,
    CropOutOfBounds,
    DuplicateCrop,
    UnusedAsset,
    SourceWithoutAsset,
};

struct SEImageDiagnostic {
    SEImageDiagnosticKind kind = SEImageDiagnosticKind::MissingFile;
    std::string message;
    int graphicIndex = -1;
    int assetIndex = -1;
    int sourceRow = -1;
};

typedef struct SRCGR {
    CSTR path{};
    CSTR filename{};

    bool loaded = false;
    //SDL_Texture* texture = NULL;
    //D3Image texture;
    PDIRECT3DTEXTURE9 texture;
    int sizeX, sizeY;

    CSTR name{};

    int grID;
    int isIf;
    int declare = -1;

    int LR2ID;
    int SEID;

    bool wildcard = false;
    bool fromWildcard = false;
    ARR arr_wildcard; //CSTR
}SRCGR;

typedef struct SRC {
    int gr;
    CSTR name{"noname"};
    int x, y;
    int sizeX, sizeY;
    int div_x, div_y, cycle;
    int timer;

    int num, align, keta; //for numSRC
    int muki, range, type, disable;//for sliderSRC
    int click, panel, plusonly; //for buttonSRC

    int declare;
    int ifGroup;
    int srcID;

    int objType;//
    int objID;
}SRC;

typedef struct DST {
    int leadDST = -1;
    CSTR name{"noname"};
    int src;
    int animation = 0;

    ARR arr_animation;//DST_ANIMATION

    int loop, timer;
    int op1, op2, op3;
    int op4;//for turntable

    int declare;
    int ifGroup;
    int dstID;
}DST;

typedef struct DST_ANIMATION{
    int time;
    float x, y, w, h;
    int acc;
    int a, r, g, b;
    int blend; //
    int filter; //Anti-aliasing
    float angle;
    int center; //angle axis, numpad
}DST_ANIMATION;

typedef struct SKINUNIT {
    int ID;
    int type; //0:text 1:buttoon 2:slider 3:onmouse 4:BGA 5:bargraph 6:number 7:mask //10:img
    CSTR name{ "noname" };

    int src;
    int dst;

    ARR body; //CSTR
    ARR bodyCSV; //CSV
    SRC srcc;
    DST dstt;
    int type2; //-1:non-group 0:group

    int ifGroup;

    int igType;
    int igID;
}SKINUNIT, SEOBJ;

typedef enum HISTORYOP {
    overwriteLine,
    insertLine,
    removeLine,
    moveLine,
    restoreDocument,
    group,
    ungroup,
    openFile = 255,
}HISTORYOP;

typedef struct HISTORY {
    HISTORYOP op;

    int target;
    SKINFILELINEREAD older;
    SKINFILELINEREAD newer;
}HISTORY;

struct SESimpleModeCategoryCounts {
    int numberFonts = 0;
    int judgementFonts = 0;
    int gear = 0;
    int notes = 0;
    int gauge = 0;
};

enum class SESimpleModeCategory {
    NumberFonts = 0,
    JudgementFonts,
    Gear,
    Notes,
    Gauge,
    Unsupported
};

// A derived, semantic view of one authoritative LR2 source row. The row index
// is valid only while the owning WORKSPACE projection remains clean.
struct SESimpleModeSlot {
    SESimpleModeCategory category = SESimpleModeCategory::Unsupported;
    std::string id;
    std::string objectId;
    std::string label;
    std::string command;
    int row = -1;
    int sourceIndex = -1;
    int imageIndex = -1;
    int graphicId = 0;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    int divX = 1;
    int divY = 1;
    int cycle = 0;
};

typedef struct WORKSPACE {

    char initFlag = 0;
    //data
    game g;
    // LR2SE core buffers live inside this workspace's game instance. They are
    // initialized once per workspace, not once per editor process.
    bool lr2CoreInitialized = false;

    SkinHeader meta;
    SESkinResolutionSource skinResolutionSource =
        SESkinResolutionSource::Default640x480;
    int skinResolutionEvidenceCount = 0;

    bool loaded = false;
    bool dockLayoutBuilt = false;
    bool previewReloadPending = false;
    unsigned long long previewReloadRequestedAt = 0;
    bool editorDerivedRebuildPending = false;
    unsigned long long editorDerivedRebuildRequestedAt = 0;
    bool objectModelRebuildPending = false;
    unsigned long long documentRevision = 0;
    unsigned long long savedDocumentRevision = 0;
    int lastSaveState = 0; // 0: no result, 1: saved, -1: failed
    std::string lastSaveMessage;
    unsigned long long lastSaveMessageAt = 0;
    std::string olrPackageMessage;
    int olrPackageState = 0;
    int olrResultKind = 0; // 0: OLR import, 1: LR2 materialization
    bool olrImportResultPopupRequested = false;
    int RefreshPreviewSelectionBounds();
    char mainpath[MAX_PATH];

    byte* filedata = NULL;
    unsigned int filedatasize = 0;

    int skinSizeX = 640 , skinSizeY = 480;

    ARR arr_subpath; //CSTR
    std::vector<std::string> loadScriptStack;
    ARR skinfileLines; //SKINFILELINEREAD
    ARR arr_ifunit; //IFUNIT

    ARR arr_CustomFile; //CSTR
    ARR arr_SRCGR; //SRCGR
    ARR arr_SRC; //SRC
    ARR arr_DST; //DST

    ARR arr_IMG; //IMG
    ARR arr_seobj; //SKINUNIT, SEOBJ

    ARR arr_history; //HISTORY
    // Snapshot history entries point into this vector. Object reorders and
    // Simple Mode batch replacements can touch non-contiguous document rows.
    std::vector<SkinDocumentSnapshot> historyDocumentSnapshots;
    int pendingHistorySnapshotRestore = -1;
    bool applyingHistory = false;
    // A ColorEdit gesture may update several ARGB columns over many frames.
    // Keep one live history entry for the gesture instead of recording every
    // component and mouse movement as a separate undo step.
    int objectColorEditRow = -1;
    int objectColorEditHistoryIndex = -1;
    int ApplyDstArgbEdit(int row, const int argb[4]);
    void EndDstArgbEdit();
    int UndoLastEdit();
    void NotifyDocumentChanged(unsigned int changes);
    bool IsDocumentDirty() const;
    void MarkDocumentSaved();
    int SaveCurrentSkin();
    int ExportOlrSkin(const char* packagePath, std::string& resultMessage);
    int ImportOlrSkinInteractive();
    int ExportLr2SkinInteractive();
    int previewScreen = -1; //DxLib handle

    // Every workspace owns its semantic Object model.  The previous global
    // model let two open workspaces overwrite each other's Browser/Inspector.
    SEObjectEditorModel objectEditorModel;
    SEObjectSelectionState objectSelection;
    SEObjectSelectionKey MakeObjectSelectionKey(int modelIndex) const;
    int ResolveObjectSelectionKey(const SEObjectSelectionKey& key) const;
    void ClearObjectSelection();
    void SetObjectSelection(const std::vector<int>& modelIndices,
        int activeModelIndex = -1, int anchorModelIndex = -1,
        bool requestBrowserFocus = false);
    void RestoreObjectSelection();
    bool ResolveImageCropColumns(const char* command, int columns[5]) const;
    void CollectImageAssignableSourceRows(int modelIndex,
        std::vector<int>& rows) const;
    bool ApplyImageAssetToObjectSource(int imageIndex, int modelIndex,
        int sourceRow, bool copyAnimationFields);
    int FindImageAssetForRow(int row);
    int FindImageAssetForObject(int modelIndex);
    void BuildImageAssetUsage(std::vector<std::vector<int>>& usage);
    const std::vector<std::vector<int>>& ImageAssetUsage();
    bool SynchronizeImageManagerToObject(int modelIndex);
    bool SetImageManagerHoveredObject(int modelIndex, int frameCount);
    void RebuildObjectModel();
    int SetObjectName(int modelIndex, const char* name);
    int DeleteObject(int modelIndex);
    SkinDocumentSnapshot CaptureDocumentSnapshot() const;
    int RestoreDocumentSnapshot(const SkinDocumentSnapshot& snapshot);
    bool CanReorderObject(int sourceModelIndex, int targetModelIndex) const;
    bool QueueObjectReorder(int sourceModelIndex, int targetModelIndex, bool placeAfter);
    int ApplyPendingObjectReorder();

    //mainwindow
    int num;
    char title[260];
    int proc();
    int init();
    bool needInit = true;
    bool alive;
    int draw();

    //subwindows
    // 
    //
    int SeInit();
    int SeLoadInit();
    //HOW TO ADD FEATURE - STEP 1 : declare flag and function
    int wildcardTOAll(char* path);

    int ScanSkins(const char* folder = nullptr, const char* folderLabelUtf8 = nullptr);
    int OpenSkinFolderDialog();
    bool skinBrowserDataInitialized = false;
    std::string skinBrowserFolder;
    std::string skinBrowserFolderLabelUtf8;
    std::string skinBrowserMessage;
    unsigned long skinBrowserScanRevision = 0;
    int skinBrowserState = 0; // 0: idle, 1: scanned, -1: failed
    int skinBrowserSelectedIndex = 0;
    PDIRECT3DTEXTURE9 skinBrowserPreviewTexture = NULL;
    int skinBrowserPreviewWidth = 0;
    int skinBrowserPreviewHeight = 0;
    bool skinBrowserPreviewAvailable = false;
    int skinBrowserResolutionSaveState = 0;
    std::string skinBrowserResolutionSavePath;
    unsigned long skinBrowserObservedScanRevision = 0;
    bool wSkinList;
    int drawSkinList();

    int LoadSkin(char* path);
    int LoadSkinScript(char* path);
    int ResetEditorDocumentForLoad();
    int ParseSkin();
    int ParseSkinConditions();
    int ParseSkinLegacyObjectsAndAssets();
    int ParseSkinGraphics();
    int ParseSkinSourcesAndDestinations();
    int LoadSkinGraphicMetadata();
    int RebuildEditorDerivedState();
    int currentLeadDST = -1;


    int LoadSceneSE();
    int ReadSkinSE();

    int ParseSkin2();

    int SaveSkinScript(char* path, bool split, bool nocomment);
    int SaveSkinScript2(char* path, bool split, bool nocomment);

    bool wSaveMenu;
    int drawSaveMenu();
    bool wSaveMenu2;
    int drawSaveMenu2();

    //TextEdit
    bool wTextEdit;
    int drawTextEdit();
    int textCursor = 0;
    bool hideComment = false;
    bool hideBlank = false;



    bool wPreview;
    int drawPreview();
    PDIRECT3DTEXTURE9 texture_preview = NULL;
    int texture_preview_width = 0;
    int texture_preview_height = 0;
    unsigned long long previewLastRenderAt = 0;
    bool previewTextureDirty = true;
    bool previewSimulationPlaying = false;
    bool previewChartFull = false;
    bool UpdatePreviewRuntime(unsigned long long previewNow);
    ImVec2 clickPos;
    bool drawRightClick;
    float zoom = 1.0f;

    bool wCustomize;
    int drawCustomize();

    bool wTimerControl = false;
    int drawTimerControl();
    // 0: runtime state, 1: user started, -1: user reset.
    signed char timerManualOverride[200] = {};

    //ImgManager
    bool wImgManager;
    int drawImgManager();
    float ImageManagerZoom = 0.0f;
    bool imagePixelPaintMode = false;
    ImVec4 imagePixelPaintColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    int imagePixelPaintLastX = -1;
    int imagePixelPaintLastY = -1;
    int imagePixelPaintLastButton = -1;
    std::map<std::string, bool> imagePixelPaintDirtyPaths;
    std::string imagePixelPaintStatus;
    std::string imageManagerReloadPathRequest;
    bool imageNewDialogRequested = false;
    bool imageMergeDialogRequested = false;
    bool imageReplaceDialogRequested = false;
    bool imageGridDialogRequested = false;
    char imageToolOutputPathUtf8[1024] = {};
    int imageNewWidth = 1;
    int imageNewHeight = 1;
    ImVec4 imageNewColor = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    int imageMergeAssetIndex = 0;
    int imageReplaceDeclarationRow = -1;
    int imageReplaceOldWidth = 0;
    int imageReplaceOldHeight = 0;
    int imageReplaceNewWidth = 0;
    int imageReplaceNewHeight = 0;
    int imageReplaceAffectedCropCount = 0;
    int imageReplaceOutOfBoundsCropCount = 0;
    std::string imageReplaceDiskPath;
    int imageGridAssetIndex = -1;
    int imageGridGr = 0;
    int imageGridX = 0;
    int imageGridY = 0;
    int imageGridW = 0;
    int imageGridH = 0;
    int imageGridIfGroup = 0;
    int imageGridColumns = 1;
    int imageGridRows = 1;
    char imageGridNamePrefix[128] = {};
    std::vector<unsigned char> imageGridSelectedCells;
    bool imageToolRegisterInCsv = true;
    std::string imageToolStatus;
    int imageManagerGeneratedGrFocusRequest = -1;
    int imageManagerGraphicDeclarationFocusRequest = -1;
    int imageManagerAssetDeclarationFocusRequest = -1;
    int grID_selected = 0;
    int gr_selected = 0;
    int src_selected = 0;
    int imageManagerFocusRequest = -1;
    int imageManagerHoveredAssetIndex = -1;
    int imageManagerHoveredAssetFrame = -1;
    int loadSRC();
    ImVec4 bgColor;
    int oldIf = -1;

    // Asset Browser shares Image Manager's IMG/SRCGR selection. It owns only
    // presentation state so future drag/drop consumers can reuse the same
    // asset index without creating another image model.
    bool wAssetBrowser;
    int drawAssetBrowser();
    float assetThumbnailSize = 96.0f;
    bool assetAnimateSrc = true;
    bool assetShowUnusedOnly = false;
    bool assetApplyCopyAnimation = false;
    bool assetApplyDialogRequested = false;
    int assetApplyAssetIndex = -1;
    SEObjectSelectionKey assetApplyObject;
    bool assetDeleteDialogRequested = false;
    int assetDeleteAssetIndex = -1;
    std::string assetDeleteStatus;
    char assetSearch[128] = {};
    int assetBrowserFocusRequest = -1;
    unsigned long long imageAssetUsageGeneration = 0;
    unsigned long long imageAssetUsageCacheGeneration = ~0ULL;
    int imageAssetUsageCacheAssetCount = -1;
    std::vector<std::vector<int>> imageAssetUsageCache;
    int ResolveIMGTextureIndex(int imageIndex);
    void CollectIMGTextureCandidates(int imageIndex,
        std::vector<std::pair<int, int>>& candidates);
    int ResolveIMGSourceIndex(int imageIndex) const;
    void ResolveIMGDivision(int imageIndex, int& divX, int& divY,
        int& cycle, int& timer) const;
    bool InitializeAssetBackedObjectSource(CSVbuf& values,
        const char* command, int imageIndex);
    void SynchronizeNewObjectAutoName(const char* command,
        bool assetDropModal);
    bool SelectIMGAsset(int imageIndex, bool requestImageManagerScroll = false);
    bool OpenNewObjectFromAsset(int imageIndex, int dropX, int dropY);
    int RegisterGeneratedImage(const char* diskPath, int width, int height,
        std::string& errorText);
    bool ReplaceImageDeclarationPath(int graphicIndex, const char* diskPath,
        std::string& errorText);
    bool RegisterImageAssetGrid(int imageIndex, int columns, int rows,
        const std::vector<unsigned char>& selectedCells, const char* namePrefix,
        std::vector<int>& insertedRows, std::string& errorText);
    void BuildImageDiagnostics(std::vector<SEImageDiagnostic>& diagnostics);

    int printSrcImg(SRC src, bool button = 0);
    int printSrcImgButton(SRC src, int num, int w, int h);
    int printSrcImgEx(SRC src, int w, int h, bool ignoreIfGroup = false);

    bool wFileManager;
    int drawFileManager();

    bool wTreeView;
    int drawTreeView();
    int split, nocomment, exist, success;
    bool wSaveMenuResult;
    char newPath[260] = "";
    bool workspaceResolutionDialogRequested = false;
    int workspaceResolutionWidth = 640;
    int workspaceResolutionHeight = 480;
    int workspaceResolutionState = 0; // 0: idle, 1: saved/reloaded, -1: failed
    std::string workspaceResolutionMessage;

    //newfile
    bool wNewskin;
    int drawNewskin();
    
    // Simple Mode presents semantic component slots while WORKSPACE remains
    // the only owner of CSV values, assets, History and preview invalidation.
    bool wSimpleMode;
    int simpleModeCategory = 0;
    std::string simpleModeSelectedSlotId;
    int simpleModeCandidateAsset = -1;
    int simpleModeApplyScope = 0;
    bool simpleModeOnlyCompatibleGrid = true;
    bool simpleModeUseTargetAtlasGrid = true;
    float simpleModeHueShift = 0.0f;
    float simpleModeSaturationPercent = 0.0f;
    float simpleModeBrightnessPercent = 0.0f;
    std::string simpleModeStatus;
    int simpleModeStatusState = 0;
    // Building this projection scans the complete CSV/Object/asset model, so
    // retain it per workspace and invalidate it at document/model boundaries.
    std::vector<SESimpleModeSlot> simpleModeProjection;
    bool simpleModeProjectionDirty = true;
    unsigned long long simpleModeProjectionGeneration = 0;
    const std::vector<SESimpleModeSlot>& GetSimpleModeSlots();
    void InvalidateSimpleModeProjection();
    int drawSimpleMode();
    int ApplySimpleModeAsset(int targetRow, int imageIndex,
        int applyScope, std::string& resultMessage);
    int ImportSimpleModeImage(int targetRow, const char* sourcePath,
        int applyScope, bool useTargetAtlasGrid, std::string& resultMessage);
    int GenerateSimpleModeColorVariant(int targetRow, int applyScope,
        float hueShiftDegrees, float saturationPercent,
        float brightnessPercent, std::string& resultMessage);
    SESimpleModeCategoryCounts GetSimpleModeCategoryCounts();

    int drawSrc(int iSRCGR, int iSRCID);
    bool EnsureSRCGRTexture(int iSRCGR);
    //int drawSrc(int iSRCGR, int iSRCID, int posX, int posY);
    int drawSrc(int iSRCGR, int iSRCID, int posX, int posY, int w = -1, int h = -1, bool stretch = 0);
    //dstview
    bool wDstView;
    int drawDstView();
    int selected_dst;
    int dst_view_scroll_request = -1;
    float DstViewZoom = 0.0f;
    float DstViewTime;
    bool isDstViewTimeStop;

    bool wNewObject;
    int drawNewObject();
    int selected_command;
    int newObjectInsertPosition = -1;
    int newObjectIfgroup = 0;
    bool newCommandIncludeAll = false;
    bool newObjectCsvInitialized = false;
    int newObjectInitializedCommand = -1;
    int newObjectAssetIndex = -1;
    int newObjectDropX = 0;
    int newObjectDropY = 0;
    int newObjectDropW = 1;
    int newObjectDropH = 1;
    bool newObjectFocusRequest = false;
    CSTR newObjectOwner;
    CSTR newObjectName;
    bool newObjectNameManuallyEdited = false;
    std::string newObjectAutoName;
    CSVbuf nCsv;

    int MakeObjects();
    bool wObjectManager;
    int drawObjectManager();

    bool wObjectEditor = false;
    // wObjectEditor remains a compatibility request that opens both panes.
    // The two panes own only visibility; selection stays in shared WORKSPACE state.
    bool wObjectBrowser = false;
    bool wObjectInspector = false;
    bool objectBrowserActiveOnly = false;
    int selected_object_editor = 0;
    int selected_object_group = -1;
    int selected_user_object_group = -1;
    int object_editor_select_request = -1;
    int objectEditorLastLineCount = -1;
    char objectSearch[128] = {};
    bool requestCreateGroupPopup = false;
    char newObjectGroupName[128] = "New Group";
    int objectStatusCacheLineCount = -1;
    int objectStatusCacheIfCount = -1;
    unsigned long long objectStatusCacheAt = 0;
    std::vector<bool> objectBranchActive;
    int objectBranchTreeIfCount = -1;
    std::map<int, bool> objectBranchTreeOpen;
    bool pendingObjectReorder = false;
    bool pendingObjectReorderAfter = false;
    SEObjectSelectionKey pendingObjectReorderSource;
    SEObjectSelectionKey pendingObjectReorderTarget;
    bool objectDeleteDialogRequested = false;
    SEObjectSelectionKey pendingObjectDelete;
    int drawObjectEditor();
    int selected_obj;


    bool wObjectManagerTest;
    int drawObjectManagerTest();
    struct PreviewSelectionBounds {
        float x;
        float y;
        float w;
        float h;
    }preview_selected_obj;
    PreviewSelectionBounds preview_selected_obj_last = {};
    PreviewSelectionBounds preview_hover_obj = {};
    bool preview_selected_obj_valid = false;
    bool preview_selected_obj_last_valid = false;
    bool preview_hover_obj_valid = false;
    int preview_selected_object_model_index = -1;
    std::vector<int> preview_selected_object_model_indices;
    int preview_selection_anchor_model_index = -1;
    bool preview_object_dragging = false;
    bool preview_object_resizing = false;
    ImVec2 preview_drag_mouse_start = {};
    float preview_drag_object_start_x = 0.0f;
    float preview_drag_object_start_y = 0.0f;
    float preview_drag_last_start_x = 0.0f;
    float preview_drag_last_start_y = 0.0f;
    float preview_resize_object_start_w = 0.0f;
    float preview_resize_object_start_h = 0.0f;
    int selectedObjectTest;

    bool wHistory;
    int drawHistory();

    bool wProperty;
    int drawProperty();

    bool wOpList;
    int drawOpList();
    bool op[1000];
    
    int NewIMG(int gr, int x, int y, int w, int h, int ifGroup = 0);
    bool CanDeleteIMG(int pos, std::string* reason = nullptr);
    int DeleteIMG(int pos);
    int ModifyIMG(int pos, int gr, int x, int y, int w, int h);
    int FindIMG(int gr, int x, int y, int w, int h, int ifGroup = -1);

    int InsertLine(int pos);
    int DeleteLine(int pos);
    int EditLine(int pos, CSTR oldlinebody, CSTR newlinebody);
    int EditValue(int pos, int column, const char* newVal);
    int EditValue(int pos, int column, int newVal);
    int CsvToLine(int pos);


    int LoadSceneSE(skstruct* sk, CSTR skinfile, int p5, char font);

}WORKSPACE;

//every windows have procs, flags
//they have 


extern std::vector<std::unique_ptr<WORKSPACE> > workspaceList;

int makeTransBackground();
int AutoSRCObjectPos(SRCGR* gr, int* x, int* y, int* w, int* h);
int CsvToCSTR(CSVbuf& csv, CSTR& line);
int CountCsvColumns(CSTR& line);
int RunAssetMetadataSelfTest();
int RunSimpleModeProjectionSelfTest();
int RunSimpleModeScopeRuleSelfTest();
int RunWorkspaceReloadLifecycleSelfTest();
int RunDstColorSelfTest();
int RunWorkspaceRuntimeMultiWorkspaceSmokeTest(const char* firstPath,
    const char* secondPath);
int RunInitialPresetSelfTest();
int RunWorkspaceRuntimeReloadSmokeTest(const char* firstPath,
    const char* secondPath);
int RunWorkspaceRuntimeMultiWorkspaceSmokeTest(const char* firstPath,
    const char* secondPath);
