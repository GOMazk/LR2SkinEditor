#pragma once

#include <cstddef>
#include <cstdio>

// The workspace UI catalog is the machine-readable contract for editor windows.
// Keep one entry per Windows-menu tool. The UI-map generator reads this table,
// while the runtime uses the same titles and stable IDs for ImGui windows.
enum class SEUIWindowId {
    Preview,
    TimerControl,
    Customize,
    ImageManager,
    AssetBrowser,
    TextEditor,
    FileManager,
    SimplePreview,
    DstView,
    ObjectBrowser,
    ObjectInspector,
    ObjectManager,
    ObjectManagerTest,
    ObjectProperty,
    OptionList,
    History,
    Count
};

struct SEUIWindowSpec {
    SEUIWindowId id;
    const char* key;
    const char* title;
    const char* purpose;
    const char* ownerFunction;
    const char* group;
    const char* defaultDock;
    bool defaultVisible;
};

inline constexpr SEUIWindowSpec kSEUIWindowSpecs[] = {
    { SEUIWindowId::Preview, "preview", "Preview", "Render and directly manipulate the loaded skin canvas.", "WORKSPACE::drawPreview", "Workspace", "center-tabs", true },
    { SEUIWindowId::TimerControl, "timer-control", "Timer Control", "Restart the scene runtime and inspect or override LR2 timers.", "WORKSPACE::drawTimerControl", "Workspace", "right-lower", true },
    { SEUIWindowId::Customize, "customize", "Customize", "Choose skin-defined customization options.", "WORKSPACE::drawCustomize", "Data", "right-lower", true },
    { SEUIWindowId::ImageManager, "image-manager", "Image Manager", "Inspect source atlases and edit or generate image files.", "WORKSPACE::drawImgManager", "Assets", "center-tabs", true },
    { SEUIWindowId::AssetBrowser, "asset-browser", "Asset Browser", "Search, preview and drag tagged image crops.", "WORKSPACE::drawAssetBrowser", "Assets", "center-bottom", true },
    { SEUIWindowId::TextEditor, "text-editor", "Text Editor", "Edit the authoritative LR2 skin text while preserving encoding.", "WORKSPACE::drawTextEdit", "Data", "center-tabs", false },
    { SEUIWindowId::FileManager, "file-manager", "File Manager", "Inspect scripts and images referenced by the workspace.", "WORKSPACE::drawFileManager", "Data", "center-bottom", false },
    { SEUIWindowId::SimplePreview, "simple-preview", "Simple Preview", "Show the legacy lightweight preview surface.", "WORKSPACE::drawSimplePreview", "Advanced", "center-tabs", false },
    { SEUIWindowId::DstView, "dst-view", "DST View", "Inspect destination rows and preview their animation frames.", "WORKSPACE::drawDstView", "Assets", "center-tabs", true },
    { SEUIWindowId::ObjectBrowser, "object-browser", "Object Browser", "Filter, group, select and reorder parsed skin objects.", "WORKSPACE::drawObjectEditor", "Workspace", "left-browser", true },
    { SEUIWindowId::ObjectInspector, "object-inspector", "Object Inspector", "Edit properties of the shared Object Browser selection.", "WORKSPACE::drawObjectEditor", "Workspace", "left-inspector", true },
    { SEUIWindowId::ObjectManager, "object-manager", "Object Manager", "Inspect the legacy object list and property editor.", "WORKSPACE::drawObjectManager", "Advanced", "left-browser", false },
    { SEUIWindowId::ObjectManagerTest, "object-manager-test", "Object Manager Test", "Exercise the legacy runtime object model for development.", "WORKSPACE::drawObjectManagerTest", "Advanced", "left-browser", false },
    { SEUIWindowId::ObjectProperty, "object-property", "Object Property", "Show the legacy standalone object property surface.", "WORKSPACE::drawProperty", "Advanced", "left-inspector", false },
    { SEUIWindowId::OptionList, "option-list", "Option List", "Inspect and toggle LR2 option flags used by previews.", "WORKSPACE::drawOpList", "Data", "right-upper", true },
    { SEUIWindowId::History, "history", "History", "Inspect legacy row-level edit history.", "WORKSPACE::drawHistory", "Data", "center-bottom", false }
};

inline constexpr std::size_t kSEUIWindowSpecCount =
    sizeof(kSEUIWindowSpecs) / sizeof(kSEUIWindowSpecs[0]);

static_assert(kSEUIWindowSpecCount == static_cast<std::size_t>(SEUIWindowId::Count),
    "Every SEUIWindowId must have one catalog entry.");

inline const SEUIWindowSpec& SEUIWindowSpecFor(SEUIWindowId id) {
    for (const SEUIWindowSpec& spec : kSEUIWindowSpecs)
        if (spec.id == id) return spec;
    return kSEUIWindowSpecs[0];
}

inline void FormatSEUIWindowTitle(char* buffer, std::size_t bufferSize,
    SEUIWindowId id, int workspaceNumber) {
    const SEUIWindowSpec& spec = SEUIWindowSpecFor(id);
    std::snprintf(buffer, bufferSize, "%s##%s-%d", spec.title, spec.key,
        workspaceNumber);
}

// Non-dockable shells and modal workflows are catalogued separately. They do
// not share the window-title helper because their popup IDs have different
// lifetime rules, but the UI-map generator assigns their controls by owner.
enum class SEUISurfaceId {
    ApplicationShell,
    WorkspaceShell,
    SkinBrowser,
    SaveAs,
    Export,
    NewSkin,
    NewObject,
    Help,
    Count
};

struct SEUISurfaceSpec {
    SEUISurfaceId id;
    const char* key;
    const char* title;
    const char* purpose;
    const char* ownerFunction;
    const char* kind;
};

inline constexpr SEUISurfaceSpec kSEUISurfaceSpecs[] = {
    { SEUISurfaceId::ApplicationShell, "application-shell", "Application Shell", "Own the application menu and workspace lifecycle.", "WinMain", "shell" },
    { SEUISurfaceId::WorkspaceShell, "workspace-shell", "Workspace Shell", "Own file commands, the toolbar, tool visibility and default docking.", "WORKSPACE::draw", "shell" },
    { SEUISurfaceId::SkinBrowser, "skin-browser", "Skin Browser", "Choose a registered LR2 skin or recursively scan another folder.", "WORKSPACE::drawSkinList", "dialog" },
    { SEUISurfaceId::SaveAs, "save-as", "Save As", "Choose the output script and switch the active working path.", "WORKSPACE::drawSaveMenu", "dialog" },
    { SEUISurfaceId::Export, "export", "Export", "Configure and write an exported skin script.", "WORKSPACE::drawSaveMenu2", "dialog" },
    { SEUISurfaceId::NewSkin, "new-skin", "New Skin", "Create a protected initial skin preset for a scene and resolution.", "WORKSPACE::drawNewskin", "dialog" },
    { SEUISurfaceId::NewObject, "new-object", "New Object", "Create an Object or raw command in the selected file and branch.", "WORKSPACE::drawNewObject", "dialog" },
    { SEUISurfaceId::Help, "help", "Help", "Explain the editor workflow and the role of each primary workspace panel.", "DrawHelpWindow", "dialog" }
};

inline constexpr std::size_t kSEUISurfaceSpecCount =
    sizeof(kSEUISurfaceSpecs) / sizeof(kSEUISurfaceSpecs[0]);

static_assert(kSEUISurfaceSpecCount == static_cast<std::size_t>(SEUISurfaceId::Count),
    "Every SEUISurfaceId must have one catalog entry.");

inline const SEUISurfaceSpec& SEUISurfaceSpecFor(SEUISurfaceId id) {
    for (const SEUISurfaceSpec& spec : kSEUISurfaceSpecs)
        if (spec.id == id) return spec;
    return kSEUISurfaceSpecs[0];
}

inline void FormatSEUISurfaceTitle(char* buffer, std::size_t bufferSize,
    SEUISurfaceId id) {
    const SEUISurfaceSpec& spec = SEUISurfaceSpecFor(id);
    std::snprintf(buffer, bufferSize, "%s##%s", spec.title, spec.key);
}
