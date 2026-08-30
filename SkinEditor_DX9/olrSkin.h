#pragma once

#include <string>
#include <vector>

struct SEOLRSemanticObject {
    struct Transform {
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        int rotation = 0;
        int blend = 0;
    };
    struct AnimationFrame {
        // One-based compiler address inside lr2/main.lr2skin.
        int destinationRow = -1;
        int timeMs = 0;
        int alpha = 255;
        Transform transform;
    };

    std::string id;
    std::string category;
    std::string name;
    std::string group;
    std::string sourceCommand;
    std::string destinationCommand;
    std::vector<int> sourceRows;
    bool hasDestination = false;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    int timer = 0;
    int loop = 0;
    int op1 = 0;
    int op2 = 0;
    int op3 = 0;
    Transform layout;
    std::vector<AnimationFrame> animationFrames;
};

struct SEOLRSourceMapEntry {
    int expandedRow = -1;
    int packagedRow = -1;
    std::string owner;
};

struct SEOLRAssetInput {
    int declarationRow = -1;
    std::string sourcePath;
    std::string packagePath;
};

// Compilable Simple Mode projection. V0.4 promotes only these source-asset
// fields; every unsupported LR2 row remains owned by the compatibility script.
struct SEOLRSimpleSlot {
    std::string id;
    std::string category;
    std::string label;
    std::string objectId;
    std::string sourceCommand;
    int sourceRow = -1;
    int graphicId = 0;
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    int divX = 1;
    int divY = 1;
    int cycle = 0;
};

struct SEOLRVirtualRootInput {
    // Slash-normalized path such as LR2files/Theme/IIDX. It never contains
    // a drive or parent traversal and becomes lr2/vfs/<logicalRoot>.
    std::string logicalRoot;
    std::string sourceDirectory;
};

struct SEOLRSkinDocument {
    std::string title;
    std::string maker;
    std::string scene;
    std::string resolutionSource;
    int canvasWidth = 640;
    int canvasHeight = 480;
    bool resolutionInferred = false;
    std::vector<SEOLRSemanticObject> objects;
    std::vector<SEOLRSimpleSlot> simpleSlots;
    std::vector<SEOLRSourceMapEntry> sourceMap;
    std::string lr2Script;
    std::vector<SEOLRAssetInput> assets;
    std::vector<SEOLRVirtualRootInput> virtualRoots;
    std::string lr2ExportMainPath;
    int unresolvedImageCount = 0;
    int unresolvedResourceCount = 0;
};

struct SEOLRPackageInfo {
    std::vector<std::string> entries;
    int formatVersion = 0;
    int objectCount = 0;
    int simpleSlotCount = 0;
    int compiledSimpleSlotCount = 0;
    int compiledSemanticObjectCount = 0;
    int compiledAnimationFrameCount = 0;
    int assetCount = 0;
    int virtualRootCount = 0;
    int virtualFileCount = 0;
    int skippedVirtualFileCount = 0;
    int unresolvedImageCount = 0;
    int unresolvedResourceCount = 0;
};

// True when a projected source atlas can be safely edited by the V0.4 Simple
// Mode compiler. Legacy LR2 rows outside this contract remain raw compatibility
// rows and must not prevent the package from importing.
bool SEIsOLRSimpleSlotCompilable(const SEOLRSimpleSlot& slot);

struct SEOLRLr2ExportInfo {
    int copiedFileCount = 0;
    std::string mainSkinPath;
};

// Writes a deterministic, stored-method ZIP package through a temporary file.
// The destination may be replaced, but source assets are only read.
bool SEWriteOLRSkinPackage(const char* packagePath,
    const SEOLRSkinDocument& document, SEOLRPackageInfo& packageInfo,
    std::string& errorMessage);

// Validates the complete central directory and CRC of every entry without
// extracting it. Only the OLR V0.1 stored method is accepted.
bool SEInspectOLRSkinPackage(const char* packagePath,
    SEOLRPackageInfo& packageInfo, std::string& errorMessage);

// Compiles V0.4 skin.json Simple Mode slots into an LR2 compatibility script.
// Only gr/crop/grid/cycle fields of validated #SRC_* rows are changed. Output
// is assigned only after every slot succeeds, so callers can replace files
// atomically without exposing a partially compiled document.
bool SECompileOLRSimpleMode(const std::string& skinJson,
    const std::string& lr2Script, std::string& compiledScript,
    int& compiledSlotCount, std::string& errorMessage);

// Compiles the V0.7 semantic destination authority and the V0.4 Simple Mode
// authority into an LR2 compatibility script in one atomic in-memory pass.
// V0.4 documents without semantic Objects remain supported.
bool SECompileOLRSemantics(const std::string& skinJson,
    const std::string& lr2Script, std::string& compiledScript,
    int& compiledSlotCount, int& compiledObjectCount,
    int& compiledAnimationFrameCount, std::string& errorMessage);

// Extracts the lr2/ subtree only. outputDirectory must not already exist.
// The returned mainSkinPath is outputDirectory/main.lr2skin.
bool SEExtractOLRSkinPackage(const char* packagePath,
    const char* outputDirectory, std::string& mainSkinPath,
    SEOLRPackageInfo& packageInfo, std::string& errorMessage);

// Returns true only for an extracted V0.2+ workspace with export metadata.
bool SEIsOLRVirtualWorkspace(const char* mainSkinPath);

// Materializes an extracted V0.2+ workspace into a new install-ready LR2 root.
// outputDirectory must not exist; no existing LR2 installation is overwritten.
bool SEExportOLRWorkspaceToLR2(const char* mainSkinPath,
    const char* outputDirectory, SEOLRLr2ExportInfo& exportInfo,
    std::string& errorMessage);
