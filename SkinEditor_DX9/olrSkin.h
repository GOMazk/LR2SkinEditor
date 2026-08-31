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

    struct SourceBinding {
        // One-based compiler address inside lr2/main.lr2skin.
        int sourceRow = -1;
        std::string sourceCommand;
    };

    struct Destination {
        std::string id;
        std::string destinationCommand;
        Transform layout;
        std::vector<AnimationFrame> animationFrames;
        bool hasTimer = false;
        int timer = 0;
        bool hasLoop = false;
        int loop = 0;
        bool hasOptions[3] = {};
        int options[3] = {};
    };

    struct Part {
        // Part ids are stable only inside one projected package. Source row
        // plus exact command remains the compiler binding.
        std::string id;
        std::vector<SourceBinding> sources;
        std::vector<Destination> destinations;
    };

    std::string id;
    std::string category;
    std::string name;
    std::string group;
    std::vector<Part> parts;
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
    // OLR authoring defaults to LR2's HD family when the source has no
    // explicit or inferable canvas. Explicit SD/FHD sources remain unchanged.
    int canvasWidth = 1280;
    int canvasHeight = 720;
    bool resolutionInferred = false;
    std::vector<SEOLRSemanticObject> objects;
    std::vector<SEOLRSimpleSlot> simpleSlots;
    std::vector<SEOLRSourceMapEntry> sourceMap;
    std::string lr2Script;
    std::vector<SEOLRAssetInput> assets;
    std::vector<SEOLRVirtualRootInput> virtualRoots;
    std::string lr2ExportMainPath;
    // V0.9 may materialize the original include-based LR2 main instead of the
    // flattened compatibility script, but only while that script is still
    // byte-identical to this baseline. Semantic or editor changes disable the
    // shortcut so no edit can be silently discarded.
    bool preserveOriginalMainWhenUnchanged = false;
    std::string lr2CompatibilityBaseline;
    int unresolvedImageCount = 0;
    int unresolvedResourceCount = 0;
};

struct SEOLRPackageInfo {
    std::vector<std::string> entries;
    int formatVersion = 0;
    int objectCount = 0;
    int semanticPartCount = 0;
    int destinationCount = 0;
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
    bool preservesOriginalMainWhenUnchanged = false;
};

// True when a projected source atlas can be safely edited by the V0.4 Simple
// Mode compiler. Legacy LR2 rows outside this contract remain raw compatibility
// rows and must not prevent the package from importing.
bool SEIsOLRSimpleSlotCompilable(const SEOLRSimpleSlot& slot);

struct SEOLRLr2ExportInfo {
    int copiedFileCount = 0;
    int rewrittenVirtualPathCount = 0;
    std::string mainSkinPath;
    bool preservedOriginalMain = false;
    std::string originalMainFallbackReason;
};

// LR2 enumerates installable skins only below LR2files/Theme and
// LR2files/Sound, using the .lr2skin and .lr2ss extensions.
bool SEIsLr2DiscoverableExportMainPath(const char* path);

// Writes a deterministic, stored-method ZIP package through a temporary file.
// The destination may be replaced, but source assets are only read.
bool SEWriteOLRSkinPackage(const char* packagePath,
    const SEOLRSkinDocument& document, SEOLRPackageInfo& packageInfo,
    std::string& errorMessage);

// Strictly parses the manifest JSON contract without opening an archive.
// This is useful to validate tooling input before resolving package entries.
bool SEParseOLRManifestJson(const std::string& manifestJson,
    SEOLRPackageInfo& packageInfo, std::string& errorMessage);

// Validates the complete central directory and CRC of every entry without
// extracting it. V0.1 through the current stored-method format are accepted.
bool SEInspectOLRSkinPackage(const char* packagePath,
    SEOLRPackageInfo& packageInfo, std::string& errorMessage);

// Compiles V0.4 skin.json Simple Mode slots into an LR2 compatibility script.
// Only gr/crop/grid/cycle fields of validated #SRC_* rows are changed. Output
// is assigned only after every slot succeeds, so callers can replace files
// atomically without exposing a partially compiled document.
bool SECompileOLRSimpleMode(const std::string& skinJson,
    const std::string& lr2Script, std::string& compiledScript,
    int& compiledSlotCount, std::string& errorMessage);

// Compiles the V0.7 flat and V0.8/V0.9 part-based semantic authorities
// with V0.4 Simple Mode into an LR2 compatibility script atomically.
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
