#include "imgui/imgui.h"

#include "../LR2/structure.h"
#include "../LR2/LR2_skinmanage.h"
#include "../LR2/LR2_skinload.h"
#include "../LR2/LR2_skindraw.h"
#include "../LR2/LR2_skinobject.h"
#include "../LR2/LR2_configsave.h"
#include "../LR2/En_fileutil.h"
#include "../LR2/Scene07_Skinselect.h"
#include "../LR2/En_timer.h"
#include "../LR2/En_value.h" //for ByTime

#include "../lib/DxLib/DxLib.h"
#include "winWorkspace.h"

#include "skin.h"
#include "op.h"
#include "arr.hpp"
#include "seHelper.h"
#include "inputwrap.h"
#include "imgui/imgui_internal.h"
#include <algorithm>
#include <cstdio>
#include <map>
#include <shellapi.h>
#include <string>
#include <vector>

static void WriteSkinLoadLog(const char* stage, const char* detail = NULL) {
    FILE* fp = fopen("SkinEditor_load_crash.log", "a");
    if (!fp) return;
    fprintf(fp, "%s%s%s\n", stage ? stage : "(null)", detail ? " : " : "", detail ? detail : "");
    fflush(fp);
    fclose(fp);
}

static bool SkinPathExists(const char* path) {
    if (!path || !*path) return false;
    WIN32_FIND_DATAA findData{};
    HANDLE find = FindFirstFileA(path, &findData);
    if (find == INVALID_HANDLE_VALUE) return false;
    FindClose(find);
    return true;
}

// Some distributed skins were renamed after authoring (for example
// LITONE5_PLAYAC -> LITONE5) while their internal paths kept the old theme
// folder. Resolve the same Play-relative asset inside the folder containing
// the opened skin without modifying the source text saved by the editor.
static bool ResolveSiblingPlayPath(const char* requested, const char* mainSkinPath,
    char* resolved, size_t resolvedSize) {
    if (!requested || !mainSkinPath || !resolved || resolvedSize == 0) return false;
    if (SkinPathExists(requested)) return false;

    std::string request = requested;
    std::string mainPath = mainSkinPath;
    std::replace(request.begin(), request.end(), '/', '\\');
    std::replace(mainPath.begin(), mainPath.end(), '/', '\\');
    const size_t requestPlay = request.find("\\Play\\");
    const size_t mainPlay = mainPath.find("\\Play\\");
    if (requestPlay == std::string::npos || mainPlay == std::string::npos) return false;

    const std::string candidate = mainPath.substr(0, mainPlay) + request.substr(requestPlay);
    if (!SkinPathExists(candidate.c_str())) return false;
    strncpy_s(resolved, resolvedSize, candidate.c_str(), _TRUNCATE);
    return true;
}

static bool SaveResolutionToSkinFile(const char* path, int width, int height);

const char* SKINTYPESTR[]= {
    "7KEYS",
    "5KEYS",
    "14KEYS",
    "10KEYS",
    "9KEYS",
    "SELECT",
    "DECIDE",
    "RESULT",
    "KEYCONFIG",
    "SKINSELECT",
    "SOUNDSET",
    "THEME",
    "7KEYSBATTLE",
    "5KEYSBATTLE",
    "9KEYSBATTLE",
    "COURSERESULT",
    "OPENING",
    "MODESELECT",
    "MODEDECIDE",
    "COURSESELECT",
    "COURSEEDIT"
}; 
///////////////////
byte TexTransparent[] =    "\x42\x4D\x76\x02\x00\x00\x00\x00\x00\x00\x76\x00\x00\x00\x28\x00\x00\x00\x20\x00\x00\x00\x20\x00\x00\x00\x01\x00\x04\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x80\x00\x00\x00\x80\x80\x00\x80\x00\x00\x00\x80\x00\x80\x00\x80\x80\x00\x00\x80\x80\x80\x00\xC0\xC0\xC0\x00\x00\x00\xFF\x00\x00\xFF\x00\x00\x00\xFF\xFF\x00\xFF\x00\x00\x00\xFF\x00\xFF\x00\xFF\xFF\x00\x00\xFF\xFF\xFF\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x88\x88\x88\x88\x88\x88\x88\x88\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";

PDIRECT3DTEXTURE9 transBackground;
int makeTransBackground(){
    int x, y;
    LoadTextureFromMemory(TexTransparent, sizeof(TexTransparent), &transBackground, &x, &y);
    //LoadTextureFromRawMemory(TexTransparent, renderer, &transBackground, 64, 64, 1);
    return 0;
}


///////////////////
int WORKSPACE::proc() {

    return 0;
}
int WORKSPACE::init() {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    zoom = 1.0f;

    initFlag = 1;
    return 0;
}
int WORKSPACE::draw() {
    if (initFlag == 0) init();
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
    ImGui::SetNextWindowViewport(viewport->ID);
    const ImGuiWindowFlags workspaceFlags = ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::Begin(title, &alive, workspaceFlags);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", NULL, &wNewskin)) {
                //
            }
            if (ImGui::MenuItem("Open", NULL, &wSkinList)) {
                ScanSkins();
            }
            if (loaded) {
                ImGui::MenuItem("Save as", NULL, &wSaveMenu);
                ImGui::MenuItem("Export", NULL, &wSaveMenu2); //todo
            }
            ImGui::EndMenu();
        }
        if (loaded) {
            
            if (ImGui::BeginMenu("Windows")) {
                //HOW TO ADD FEATURE - STEP 3 : add menu and flag here
                if (isTextmode) {
                    ImGui::MenuItem("Text Editor", NULL, &wTextEdit);
                    ImGui::MenuItem("Preview", NULL, &wPreview);
                    ImGui::MenuItem("history", NULL, &wHistory);
                }
                else {
                    ImGui::MenuItem("Preview", NULL, &wPreview);
                    ImGui::MenuItem("Customize", NULL, &wCustomize);
                    ImGui::MenuItem("ImgManager", NULL, &wImgManager);
                    //if (ImGui::MenuItem("ImgManager", NULL, &wImgManager)) { loadSRC(); };
                    ImGui::MenuItem("fileManager", NULL, &wFileManager);
                    ImGui::MenuItem("SimplePreview", NULL, &wSimplePreview);
                    ImGui::MenuItem("dstView", NULL, &wDstView);
                    ImGui::MenuItem("Object Editor", NULL, &wObjectEditor);
                    ImGui::MenuItem("objectManager", NULL, &wObjectManager);
                    ImGui::MenuItem("objectManagerTest", NULL, &wObjectManagerTest);
                    ImGui::MenuItem("objectProperty", NULL, &wProperty);
                    ImGui::MenuItem("OpList", NULL, &wOpList);
                    ImGui::MenuItem("history", NULL, &wHistory);
                }
                ImGui::EndMenu();
            }
        }
        ImGui::EndMenuBar();
    }

    char dockName[64];
    snprintf(dockName, sizeof(dockName), "WorkspaceDock##%d", num);
    const ImGuiID dockspaceId = ImGui::GetID(dockName);
    ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    if (!dockLayoutBuilt) {
        ImGui::DockBuilderRemoveNode(dockspaceId);
        ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->WorkSize);
        ImGuiID rightDock = 0;
        ImGuiID centerDock = dockspaceId;
        ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Right, 0.14f, &rightDock, &centerDock);

        ImGuiID editorDock = 0;
        ImGuiID previewDock = centerDock;
        ImGui::DockBuilderSplitNode(centerDock, ImGuiDir_Down, 0.36f, &editorDock, &previewDock);

        ImGuiID customizeDock = 0;
        ImGuiID opListDock = rightDock;
        ImGui::DockBuilderSplitNode(rightDock, ImGuiDir_Down, 0.36f, &customizeDock, &opListDock);

        char previewTitle[64];
        char objectEditorTitle[64];
        char imageManagerTitle[64];
        char dstViewTitle[64];
        char opListTitle[64];
        char customizeTitle[64];
        snprintf(previewTitle, sizeof(previewTitle), "Preview##%d", num);
        snprintf(objectEditorTitle, sizeof(objectEditorTitle), "Object Editor##%d", num);
        snprintf(imageManagerTitle, sizeof(imageManagerTitle), "ImageManager##%d", num);
        snprintf(dstViewTitle, sizeof(dstViewTitle), "dstView##%d", num);
        snprintf(opListTitle, sizeof(opListTitle), "OpList##%d", num);
        snprintf(customizeTitle, sizeof(customizeTitle), "Customize##%d", num);
        ImGui::DockBuilderDockWindow(previewTitle, previewDock);
        ImGui::DockBuilderDockWindow(imageManagerTitle, previewDock);
        ImGui::DockBuilderDockWindow(dstViewTitle, previewDock);
        ImGui::DockBuilderDockWindow(objectEditorTitle, editorDock);
        ImGui::DockBuilderDockWindow(opListTitle, opListDock);
        ImGui::DockBuilderDockWindow(customizeTitle, customizeDock);
        ImGui::DockBuilderFinish(dockspaceId);
        dockLayoutBuilt = true;
    }

    if (loaded && false) {
        char tempTitle[260];

        snprintf(tempTitle, sizeof(tempTitle), "objList##%d", num);
        if (ImGui::BeginChild(tempTitle, { 230,400 }, ImGuiChildFlags_ResizeX | ImGuiChildFlags_FrameStyle)) {
            for (int i = 0; i < arr_seobj.count; i++) {
                ImGui::PushID(i);
                SEOBJ& seobj = ((SEOBJ*)arr_seobj.data)[i];

                char buf[260];
                sprintf(buf, "%03d(%s)", i, seobj.name.outstr());

                if (ImGui::Selectable(buf, selected_obj == i)) {
                    selected_obj = i;
                }
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
        
        //ImGui::SameLine();
        //preview

        ImGui::Separator();
        SEOBJ& seobj = ((SEOBJ*)arr_seobj.data)[selected_obj];
        snprintf(tempTitle, sizeof(tempTitle), "objProperty##%d", num);
        if (ImGui::BeginChild(tempTitle, { 400,-1 }, ImGuiChildFlags_ResizeX | ImGuiChildFlags_FrameStyle)) {
            ImGui::PushID(selected_obj);

            for (int b = 0; b < seobj.body.count; b++) {
                ImGui::Text("%s", ((CSTR*)seobj.body.data)[b]);
            }

            if (seobj.type2 == 0) {
                if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None))
                {
                    if (ImGui::BeginTabItem("SRC"))
                    {
                        ImGui::SeparatorText("basic");
                        ImGui::InputText("gr", ((CSVbuf*)seobj.bodyCSV.data)[0].str[2], IM_COUNTOF(((CSVbuf*)seobj.bodyCSV.data)[0].str[2].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        ImGui::InputText("x", ((CSVbuf*)seobj.bodyCSV.data)[0].str[3], IM_COUNTOF(((CSVbuf*)seobj.bodyCSV.data)[0].str[3].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        ImGui::InputText("y", ((CSVbuf*)seobj.bodyCSV.data)[0].str[4], IM_COUNTOF(((CSVbuf*)seobj.bodyCSV.data)[0].str[4].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        ImGui::InputText("w", ((CSVbuf*)seobj.bodyCSV.data)[0].str[5], IM_COUNTOF(((CSVbuf*)seobj.bodyCSV.data)[0].str[5].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        ImGui::InputText("h", ((CSVbuf*)seobj.bodyCSV.data)[0].str[6], IM_COUNTOF(((CSVbuf*)seobj.bodyCSV.data)[0].str[6].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        ImGui::SeparatorText("animation");
                        ImGui::InputText("div_x", ((CSVbuf*)seobj.bodyCSV.data)[0].str[7], IM_COUNTOF(((CSVbuf*)seobj.bodyCSV.data)[0].str[7].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        ImGui::InputText("div_y", ((CSVbuf*)seobj.bodyCSV.data)[0].str[8], IM_COUNTOF(((CSVbuf*)seobj.bodyCSV.data)[0].str[8].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        ImGui::InputText("cycle", ((CSVbuf*)seobj.bodyCSV.data)[0].str[9], IM_COUNTOF(((CSVbuf*)seobj.bodyCSV.data)[0].str[9].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        //ImGui::InputText("timer", ((CSVbuf*)seobj.bodyCSV.data)[0].str[10], IM_COUNTOF(((CSVbuf*)seobj.bodyCSV.data)[0].str[10].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        CstrInputText("timer", &((CSVbuf*)seobj.bodyCSV.data)[0].str[10], ImGuiInputTextFlags_None);
                        ImGui::Text("Data: %p\nSize: %d\nCapacity: %d", (void*)((CSVbuf*)seobj.bodyCSV.data)[0].str[10].body, ((CSVbuf*)seobj.bodyCSV.data)[0].str[10].length(), ((CSVbuf*)seobj.bodyCSV.data)[0].str[10].msize());


                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("DST"))
                    {
                        ImGui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            }

            ImGui::PopID();
        }
        ImGui::EndChild();
    }
    else if (!loaded) {
        ImGui::Text("%s", "no skin loaded");
    }
    

    ImGui::End();

    //subwindows
    //HOW TO ADD FEATURE - STEP 4 : call function. end
    if (wSkinList) drawSkinList();
    if (wNewskin) drawNewskin();

    if (wSaveMenu) drawSaveMenu();
    if (wSaveMenu2) drawSaveMenu2();

    if (wTextEdit) drawTextEdit();

    if (wPreview) drawPreview();
    if (wCustomize) drawCustomize();
    if (wImgManager) drawImgManager();
    if (wFileManager) drawFileManager();
    if (wSimplePreview) drawSimplePreview();
    if (wDstView) drawDstView();
    if (wObjectEditor) drawObjectEditor();
    if (wObjectManager) drawObjectManager();
    if (wObjectManagerTest) drawObjectManagerTest();
    if (wProperty) drawProperty();
    if (wOpList) drawOpList();
    if (wHistory) drawHistory();

    if (wNewObject) drawNewObject();


    return 0;
}

int WORKSPACE::ScanSkins() {
    InitSkinData(&g.skinData);
    MakeSkinList(&g.skinData, CSTR("LR2files\\Theme\\"));
    MakeSkinList(&g.skinData, CSTR("LR2files\\Sound\\"));
    return 1;
}

int WORKSPACE::drawSkinList() {
    static int item_selected_idx = 0;
    int oldSelected = item_selected_idx;
    static PDIRECT3DTEXTURE9 preview_tex;
    static int preview_size_x, preview_size_y;
    static bool isPreview = false;
    static int resolutionSaveState = 0; // 0: untouched, 1: saved, -1: failed
    static std::string resolutionSavePath;

    const ImVec2 workSize = ImGui::GetMainViewport()->WorkSize;
    const ImVec2 minimumSize(
        (std::min)(820.0f, workSize.x),
        (std::min)(540.0f, workSize.y));
    ImGui::SetNextWindowSizeConstraints(minimumSize, workSize);
    ImGui::Begin("Open from skinlist", &wSkinList);

    if (g.skinData.Count <= 0) {
        ImGui::TextUnformatted("No skins found.");
        ImGui::End();
        return 0;
    }
    if (item_selected_idx < 0 || item_selected_idx >= g.skinData.Count)
        item_selected_idx = 0;

    if (ImGui::BeginChild(ImGuiChildFlags_FrameStyle, { 300,-1 }))
    {
        for (int n = 0; n < g.skinData.Count; n++)
        {
            const bool is_selected = (item_selected_idx == n);
            char itemname[260];
            const std::string titleUtf8 = Cp932ToUtf8(g.skinData.Data[n].title.outstr());
            snprintf(itemname, sizeof(itemname), "%02d:%s -%s", n, titleUtf8.c_str(), SKINTYPESTR[g.skinData.Data[n].type]);
            if (ImGui::Selectable(itemname, is_selected)) {
                item_selected_idx = n;
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndChild();
    }
    

    int& m = item_selected_idx;
    ImGui::SameLine(0, 0);
    ImGui::BeginGroup();
    //ImTextureRef preview_texid = io.Fonts->TexRef;

    if (oldSelected != m) {
        isPreview = LoadTextureFromFile(g.skinData.Data[m].thumbnail.outstr(), &preview_tex, &preview_size_x, &preview_size_y);
        oldSelected = m;
        resolutionSaveState = 0;
        resolutionSavePath.clear();
    }
    ImGui::Image(isPreview ? preview_tex : NULL, { 320, 240 }, { 0, 0 }, { 1, 1 });
    ImGui::Text("%s", Cp932ToUtf8(g.skinData.Data[m].skinFile.outstr()).c_str());
    ImGui::Text("%s", Cp932ToUtf8(g.skinData.Data[m].title.outstr()).c_str());
    ImGui::Text("%s", Cp932ToUtf8(g.skinData.Data[m].maker.outstr()).c_str());
    ImGui::Text("%s", SKINTYPESTR[g.skinData.Data[m].type]);
    ImGui::SeparatorText("Resolution");
    ImGui::TextColored(ImVec4(1.0f, 0.78f, 0.28f, 1.0f),
        "AUTO SAVE: Changes are written directly to the original skin file and cannot be undone.");
    int resolution[2] = {
        g.skinData.Data[m].targetX,
        g.skinData.Data[m].targetY
    };
    ImGui::SetNextItemWidth(100.0f);
    if (ImGui::InputInt2("Width / Height (?)", resolution)) {
        if (resolution[0] < 64) resolution[0] = 64;
        if (resolution[0] > 7680) resolution[0] = 7680;
        if (resolution[1] < 64) resolution[1] = 64;
        if (resolution[1] > 4320) resolution[1] = 4320;
        g.skinData.Data[m].targetX = resolution[0];
        g.skinData.Data[m].targetY = resolution[1];
    }
    auto persistResolution = [&]() {
        const char* skinPath = g.skinData.Data[m].skinFile.outstr();
        resolutionSaveState = SaveResolutionToSkinFile(skinPath,
            g.skinData.Data[m].targetX, g.skinData.Data[m].targetY) ? 1 : -1;
        resolutionSavePath = skinPath ? skinPath : "";
    };
    if (ImGui::IsItemDeactivatedAfterEdit()) persistResolution();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        ImGui::SetTooltip("The #INFORMATION width/height fields will be updated.\nPress Enter or leave this field to save immediately.");

    struct ResolutionPreset { const char* label; int width; int height; };
    const ResolutionPreset presets[] = {
        { "640 x 480", 640, 480 },
        { "1280 x 720", 1280, 720 },
        { "1920 x 1080", 1920, 1080 }
    };
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.32f, 0.20f, 0.48f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.44f, 0.28f, 0.63f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.53f, 0.34f, 0.72f, 1.0f));
    for (int i = 0; i < 3; ++i) {
        if (i > 0) ImGui::SameLine();
        if (ImGui::Button(presets[i].label)) {
            g.skinData.Data[m].targetX = presets[i].width;
            g.skinData.Data[m].targetY = presets[i].height;
            persistResolution();
        }
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
            ImGui::SetTooltip("Apply this resolution and save it to the skin file immediately.");
    }
    ImGui::PopStyleColor(3);
    if (resolutionSaveState > 0)
        ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.40f, 1.0f),
            "Saved resolution to #INFORMATION: %s", resolutionSavePath.c_str());
    else if (resolutionSaveState < 0)
        ImGui::TextColored(ImVec4(1.0f, 0.30f, 0.30f, 1.0f),
            "Failed to save resolution: %s", resolutionSavePath.c_str());

    if (ImGui::Button("LOAD", { 0, 0 })) {
        meta = g.skinData.Data[m];
        snprintf(title, 260, "%s -%s", meta.title.outstr(), SKINTYPESTR[meta.type]);
        
        strncpy(mainpath, g.skinData.Data[m].skinFile.outstr(), MAX_PATH);
        loaded = (LoadSkin(mainpath) == 0);
        if (loaded) wSkinList = false;
        
    }

    ImGui::SameLine(0, 3);
    if (ImGui::Button("LOAD(TEXT)", { 0, 0 })) {
        meta = g.skinData.Data[m];
        snprintf(title, 260, "%s -%s", meta.title.outstr(), SKINTYPESTR[meta.type]);

        strncpy(mainpath, g.skinData.Data[m].skinFile.outstr(), MAX_PATH);
        loaded = (LoadSkin(mainpath) == 0);
        if (loaded) {
            wSkinList = false;
            isTextmode = true;
        }

    }
    ImGui::SameLine(0, 3);
    if (ImGui::Button("BROWSE", { 0, 0 })) {
        const char* skinPath = g.skinData.Data[m].skinFile.outstr();
        if (skinPath && *skinPath) {
            char fullPath[MAX_PATH] = {};
            if (!GetFullPathNameA(skinPath, MAX_PATH, fullPath, NULL))
                strncpy(fullPath, skinPath, MAX_PATH - 1);
            std::string explorerArgs = "/select,\"";
            explorerArgs += fullPath;
            explorerArgs += "\"";
            ShellExecuteA(NULL, "open", "explorer.exe", explorerArgs.c_str(), NULL, SW_SHOWNORMAL);
        }
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        ImGui::SetTooltip("Open this skin file's folder in Windows Explorer.");
    ImGui::SameLine(0, 0);
    ImGui::EndGroup();
    ImGui::End();

    return 0;
}

int WORKSPACE::LoadSkinScript(char* path) {
    if (!path || !*path) return -1;
    char canonicalPath[MAX_PATH] = {};
    if (!GetFullPathNameA(path, MAX_PATH, canonicalPath, NULL))
        strncpy(canonicalPath, path, MAX_PATH - 1);
    for (const std::string& activePath : loadScriptStack) {
        if (_stricmp(activePath.c_str(), canonicalPath) == 0) {
            WriteSkinLoadLog("Include cycle blocked", canonicalPath);
            return -2;
        }
    }

    FILE* pFile;

    pFile = fopen(canonicalPath, "rb");
    if (!pFile) return -1;
    loadScriptStack.push_back(canonicalPath);

    //TODO : $FILE are stacked when save/load
    SKINFILELINEREAD* readS = (SKINFILELINEREAD*)skinfileLines.Get_new();
    readS->line.resize(1024);
    sprintf(readS->line, "$FILE \'%s\' start", canonicalPath);
    readS->isComment = true;
    readS->isSEcomment = true;
    readS->numTotal = skinfileLines.count-1;
    readS->num = 0;
    readS->filename.assign(canonicalPath);

    int c = 1;
    while (1) {
        std::string readLine;
        char readbuf[1024];
        while (fgets(readbuf, sizeof(readbuf), pFile) != 0) {
            readLine += readbuf;
            const size_t len = strlen(readbuf);
            if (len > 0 && readbuf[len - 1] == '\n') break;
        }
        if (readLine.empty()) break;

        SKINFILELINEREAD* read = (SKINFILELINEREAD*)skinfileLines.Get_new();
        read->line.resize(1024);
        read->filename.resize(260);
        read->line.assign(readLine.c_str());
            
        DealWhiteSpace(&read->line);
        read->numTotal = skinfileLines.count - 1;
        read->filename.assign(canonicalPath);
        read->num = c;
        read->isComment = (*read->line.atPos(0) != '#');
        read->isSEcomment = (*read->line.atPos(0) == '$');

        if(!read->isComment) {
            SplitCSV(read->line, &read->csv, ",");
            read->csvColumnCount = CountCsvColumns(read->line);
        }
            
        c++;

        if (read->line.left(8).isSame("#INCLUDE")) {
            const char* includeText = read->csv.str[1].body ? read->csv.str[1].outstr() : "";
            char includePath[MAX_PATH] = {};
            CSTR parent(canonicalPath);
            parent.assign(parent.getDirectory());
            snprintf(includePath, sizeof(includePath), "%s%s", parent.outstr(), includeText);
            if (GetFileAttributesA(includePath) == INVALID_FILE_ATTRIBUTES)
                strncpy(includePath, includeText, sizeof(includePath) - 1);
            char siblingInclude[MAX_PATH] = {};
            if (ResolveSiblingPlayPath(includePath, mainpath,
                siblingInclude, sizeof(siblingInclude)))
                strncpy_s(includePath, siblingInclude, _TRUNCATE);
            if (*includePath) {
                CSTR tmp(includePath);
                arr_subpath.push_back(&tmp);
                const int includeResult = LoadSkinScript(includePath);
                if (includeResult < 0 && includeResult != -2)
                    WriteSkinLoadLog("Include open failed", includePath);
            }
        }
    }

    SKINFILELINEREAD* readE = (SKINFILELINEREAD*)skinfileLines.Get_new();
    readE->line.resize(1024);
    sprintf(readE->line, "$FILE \'%s\' end", canonicalPath);
    readE->isComment = true;
    readE->isSEcomment = true;
    readE->numTotal = skinfileLines.count - 1;
    readE->num = c;
    readE->filename.assign(canonicalPath);
        

    fclose(pFile);
    loadScriptStack.pop_back();

    return 0;
}

int WORKSPACE::ParseSkin() {

    int IFcur = 0;
    int IFdepth = 0;    
    int cOrder = 0;

    int grCount = 0;
    int grInIf = 0;

    int srcNow = -1;
    int srcOld = -1;
    int objNow = -1;

    int objTypeCount[100] = { 0, };
    
    //obj groups
    for (int i = 0; i < skinfileLines.count; i++) {
        SKINFILELINEREAD& read = ((SKINFILELINEREAD*)skinfileLines.data)[i];
        SEOBJ* obj = NULL;
        if (read.csv.str[0].left(4).isSame("#SRC")) {
            read.isSRC = true;
            read.objID = ++objNow;
            
            obj = (SEOBJ*)arr_seobj.Get_new();
            obj->type2 = 0;
            obj->name.assign(read.csv.str[0]);
            obj->body.Alloc(sizeof(CSTR),2);
            obj->bodyCSV.Alloc(sizeof(CSVbuf), 2);

            CSTR* bodyline = (CSTR*)obj->body.Get_new();
            CSVbuf* bodyCSV = (CSVbuf*)obj->bodyCSV.Get_new();
            bodyline->assign(read.line);
            SplitCSV(*bodyline, bodyCSV, ",");

            //get all IMG presets fromSRC here
            if (read.csv.str[0].isDiff("#SRC_TEXT")){
                //check duplicated
                if (FindIMG(read.csv.val[2], read.csv.val[3], read.csv.val[4], read.csv.val[5], read.csv.val[6]) == arr_IMG.count) {
                    IMG* img = (IMG*)arr_IMG.Get_new();
                    cstrSprintf(&img->name,"%s", read.csv.str[0].outstr());
                    img->gr = read.csv.val[2];
                    img->x = read.csv.val[3];
                    img->y = read.csv.val[4];
                    img->w = read.csv.val[5];
                    img->h = read.csv.val[6];
                }
            }
        }
        else if (read.csv.str[0].left(4).isSame("#DST")) {
            read.isDST = true;
            read.objID = objNow;

            // Some large skins contain conditionally expanded DST commands
            // before a usable SRC. Never dereference an empty object list.
            if (arr_seobj.count <= 0) {
                read.isDST = false;
                read.isOther = true;
                read.objID = -1;
                continue;
            }
            obj = (SEOBJ*)arr_seobj.Get_last();

            CSTR* bodyline = (CSTR*)obj->body.Get_new();
            CSVbuf* bodyCSV = (CSVbuf*)obj->bodyCSV.Get_new();
            bodyline->assign(read.line);
            SplitCSV(*bodyline, bodyCSV, ",");
        }
        else if(read.csv.str[0].left(1).isSame("#") ){
            read.isOther = true;
            read.objID = -1;
            
            obj = (SEOBJ*)arr_seobj.Get_new(); 
            obj->type2 = -1;
            obj->name.assign(read.csv.str[0]); 
            obj->body.Alloc(sizeof(CSTR), 1);
            obj->bodyCSV.Alloc(sizeof(CSVbuf), 1);

            CSTR* bodyline = (CSTR*)obj->body.Get_new();
            CSVbuf* bodyCSV = (CSVbuf*)obj->bodyCSV.Get_new();
            bodyline->assign(read.line);
            SplitCSV(*bodyline, bodyCSV, ",");
        }
    }

    //parse skin
    // Each open conditional owns its actual current IFUNIT index. Branches are
    // not assumed to be contiguous: a nested #IF can insert units between an
    // outer #IF and its #ELSEIF/#ELSE.
    struct ConditionFrame {
        int parentGroup;
        int currentGroup;
        int nextOrder;
        int grCountBeforeBlock;
    };
    std::vector<ConditionFrame> conditionStack;

    IFUNIT rootIf;
    arr_ifunit.push_back(&rootIf);

    for (int i = 0; i < skinfileLines.count; i++) {
        SKINFILELINEREAD& read = ((SKINFILELINEREAD*)skinfileLines.data)[i];
        bool isif = false;

        if (read.isComment) continue;

        const int currentGroup = conditionStack.empty() ? 0 : conditionStack.back().currentGroup;

        if (read.csv.str[0].isSame("#IF")) {
            isif = true;

            IFUNIT tif;
            for (int val = 0; val < 10; val++) {
                tif.data[val] = read.csv.val[val + 1];
            }
            tif.depth = (int)conditionStack.size() + 1;
            tif.order = 0;
            tif.parentID = currentGroup;

            const int newGroup = arr_ifunit.count;
            arr_ifunit.push_back(&tif);
            conditionStack.push_back({ currentGroup, newGroup, 0, grCount });
            read.ifgroup = newGroup;

            read.isIfGroupHead = true;
            read.isGroupHead = true;
        }
        else if (read.csv.str[0].isSame("#ELSEIF") ||
                 read.csv.str[0].isSame("#ELSE")) {
            isif = true;

            if (conditionStack.empty()) {
                // Keep malformed standalone branch markers attached to the
                // root group instead of indexing outside arr_ifunit.
                read.ifgroup = 0;
            }
            else {
                ConditionFrame& frame = conditionStack.back();
                IFUNIT tif;
                if (read.csv.str[0].isSame("#ELSEIF")) {
                    for (int val = 0; val < 10; val++) {
                        tif.data[val] = read.csv.val[val + 1];
                    }
                }
                tif.depth = (int)conditionStack.size();
                tif.order = ++frame.nextOrder;
                tif.parentID = frame.parentGroup;

                const int newGroup = arr_ifunit.count;
                arr_ifunit.push_back(&tif);
                frame.currentGroup = newGroup;
                read.ifgroup = newGroup;

                // Alternative branches reuse the same image-number range.
                grCount = frame.grCountBeforeBlock;
                grInIf = 0;
            }

            read.isIfGroupHead = true;
            read.isGroupHead = true;
        }
        else if (read.csv.str[0].isSame("#ENDIF")) {
            isif = true;

            if (conditionStack.empty()) {
                read.ifgroup = 0;
            }
            else {
                const ConditionFrame frame = conditionStack.back();
                read.ifgroup = frame.currentGroup;
                grCount = frame.grCountBeforeBlock;
                grInIf = 0;
                conditionStack.pop_back();
            }

            read.isIfGroupEnd = true;
            read.isGroupEnd = true;
        }
        else {
            read.ifgroup = currentGroup;
        }

        if (isif) continue;


        if (read.csv.str[0].isSame("#CUSTOMFILE")) {
            CSTR* tmpstr = (CSTR*)(arr_CustomFile.Get_new());
            tmpstr->assign(read.csv.str[2]);
        }

        else if (read.csv.str[0].isSame("#IMAGE")) {
            
            CSTR line(read.csv.str[1]);

            bool isWild = false;

            for (int wc = 0; wc < arr_CustomFile.count; wc++) {
                if (line.isSame(((CSTR*)arr_CustomFile.data)[wc].outstr())) {
                    isWild = true;
                    break;
                }
            }
            if (strrchr(line.outstr(), '*')) isWild = true;

            if (!isWild) {
                SRCGR* tmp = (SRCGR*)(arr_SRCGR.Get_new());
                tmp->path.assign(line);
                
                char* cur = strrchr(read.csv.str[1].outstr(), '/');
                if (cur == NULL) cur = strrchr(read.csv.str[1].outstr(), '\\');
                if (cur)         tmp->filename.assign(cur + 1);

                tmp->grID = grCount;
                tmp->isIf = read.ifgroup;
                tmp->wildcard = false;
            }
            
            else if (isWild) {
                WIN32_FIND_DATA FindFileData;
                LPWIN32_FIND_DATAA lpFindFileData;
                HANDLE hFindFile;

                CSTR str1(line.left(line.findStrPos("*")));
                CSTR str2(line.right(line.length() - str1.length() - 1));
                CSTR str3(str1);
                str3.add("*");

                hFindFile = FindFirstFileA(str3, (LPWIN32_FIND_DATAA)&FindFileData);
                if (hFindFile != (HANDLE)-1) {
                    do {
                        if (strcmp("..", (char*)FindFileData.cFileName) && strcmp(".", (char*)FindFileData.cFileName)) {

                            SRCGR* tmp2 = (SRCGR*)(arr_SRCGR.Get_new());
                            tmp2->path.assign(str1);
                            tmp2->path.add(FindFileData.cFileName);
                            tmp2->filename.assign(FindFileData.cFileName);

                            tmp2->fromWildcard = true;
                            tmp2->grID = grCount;
                            tmp2->isIf = read.ifgroup;
                        }
                    } while (FindNextFileA(hFindFile, (LPWIN32_FIND_DATAA)&FindFileData));
                    FindClose(hFindFile);
                }
            }

            grCount++;
            ((IFUNIT*)arr_ifunit.data)[read.ifgroup].grCount++;
        }


        ////////////////
        if (read.csv.str[0].isSame("#SRC_IMAGE")) {

            if (read.csv.val[2] == 110 || read.csv.val[2] == 111) {
                // LR2 uses graphic IDs 110/111 as special placeholders.
                // The old code advanced srcNow without adding an ARR entry,
                // shifting every subsequent DST->src index and eventually
                // reading past arr_SRC on skins such as tricoro HD.
                SRC* placeholder = (SRC*)(arr_SRC.Get_new());
                placeholder->gr = 0;
                placeholder->sizeX = 1;
                placeholder->sizeY = 1;
                placeholder->div_x = 1;
                placeholder->div_y = 1;
                placeholder->declare = read.numTotal;
                placeholder->objType = 10;
                placeholder->objID = objTypeCount[9]++;
                placeholder->name.assign("LR2 special image placeholder");
                srcNow++;
                continue;
            }

            SRC *src = (SRC*)(arr_SRC.Get_new());
                        
            src->gr = read.csv.val[2];
            src->x = read.csv.val[3];
            src->y = read.csv.val[4];

            SRCGR& img = ((SRCGR*)arr_SRCGR.data)[src->gr];

            src->sizeX = read.csv.val[5];// == -1 ? img.sizeX - src->x : read.csv.val[5];
            src->sizeY = read.csv.val[6];// == -1 ? img.sizeY - src->y : read.csv.val[6];

            src->div_x = read.csv.val[7];
            src->div_y = read.csv.val[8];
            src->cycle = read.csv.val[9];
            src->timer = read.csv.val[10];

            src->declare = read.numTotal;

            char tmp[260];
            if (src->timer) {
                sprintf(tmp, "T%d(%s) : %d*%d ##%d", src->timer, timerName(src->timer), src->sizeX, src->sizeY, read.numTotal);
            }
            else {
                sprintf(tmp, "noname : %d*%d ##%d", src->sizeX, src->sizeY, read.numTotal);
            }
            src->name.assign(tmp);

            src->objType = 10;
            src->objID = objTypeCount[9]++;
            
            srcNow++;
        }

        else if (read.csv.str[0].isSame("#DST_BGA")) {

        }
        else if (read.csv.str[0].isSame("#DST_IMAGE") || read.csv.str[0].isSame("#DST_SLIDER") || read.csv.str[0].isSame("#DST_BUTTON") 
            || read.csv.str[0].isSame("#DST_BARGRAPH") || read.csv.str[0].isSame("#DST_NUMBER") || read.csv.str[0].isSame("#DST_TEXT") || read.csv.str[0].isSame("#DST_ONMOUSE")
            || read.csv.str[0].left(5).isSame("#DST_")) {
            
            DST* dst = NULL;
            if (srcNow > srcOld) {
                dst = (DST*)(arr_DST.Get_new());

                dst->name.assign(dstName(dst->op1));
                dst->animation = 0;
                currentLeadDST = arr_DST.count - 1;
                dst->src = srcNow;
                dst->declare = i;

                dst->arr_animation.Free();
                dst->arr_animation.Alloc(sizeof(DST_ANIMATION), 1);
                srcOld = srcNow;
            }
            else {// if(srcNow == srcOld){
                if (currentLeadDST < 0 || currentLeadDST >= arr_DST.count) continue;
                dst = &(((DST*)arr_DST.data)[currentLeadDST]);
            }
            
            if (!dst) continue;
            DST_ANIMATION* dstd = (DST_ANIMATION*)(dst->arr_animation.Get_new());

            dstd->time = read.csv.val[2];
            dstd->x = read.csv.val[3];
            dstd->y = read.csv.val[4];
            dstd->w = read.csv.val[5];
            dstd->h = read.csv.val[6];

            dstd->acc = read.csv.val[7];
            dstd->a = read.csv.val[8];
            dstd->r = read.csv.val[9];
            dstd->g = read.csv.val[10];
            dstd->b = read.csv.val[11];

            dstd->blend = read.csv.val[12];
            dstd->filter = read.csv.val[13];
            dstd->angle = read.csv.val[14];
            dstd->center = read.csv.val[15];

            if (dst->animation == 0) {
                dst->loop = read.csv.val[16];
                dst->timer = read.csv.val[17];

                dst->op1 = read.csv.val[18];
                dst->op2 = read.csv.val[19];
                dst->op3 = read.csv.val[20];
                dst->op4 = read.csv.val[21];
                //op5 is on 22??
            }
            dst->animation++;
            dst->leadDST = currentLeadDST;
        }
        

        else if (read.csv.str[0].isSame("#SRC_NUMBER")) { 

            if (read.csv.val[2] == 110 || read.csv.val[2] == 111) {srcNow++; continue;}

            SRC* src = (SRC*)(arr_SRC.Get_new());

            src->gr = read.csv.val[2];
            src->x = read.csv.val[3];
            src->y = read.csv.val[4];

            SRCGR& img = ((SRCGR*)arr_SRCGR.data)[src->gr];

            src->sizeX = read.csv.val[5];// == -1 ? img.sizeX - src->x : read.csv.val[5];
            src->sizeY = read.csv.val[6];// == -1 ? img.sizeY - src->y : read.csv.val[6];

            src->div_x = read.csv.val[7]; //have to be 10, 11, 24 set divs
            src->div_y = read.csv.val[8];
            src->cycle = read.csv.val[9];
            src->timer = read.csv.val[10];

            src->num = read.csv.val[11];
            src->align = read.csv.val[12];
            src->keta = read.csv.val[13];

            src->declare = read.numTotal;

            char tmp[260];
            if (src->timer || src->type) {
                sprintf(tmp, "T%d(%s) : %s %d*%d ##%d", src->timer, timerName(src->timer), numberName(src->num), src->sizeX, src->sizeY, read.numTotal);
            }
            else {
                sprintf(tmp, "noname : %d*%d ##%d", src->sizeX, src->sizeY, read.numTotal);
            }
            src->name.assign(tmp);

            src->objType = 7;
            src->objID = objTypeCount[6]++;

            srcNow++;

        }
        
        else if (read.csv.str[0].isSame("#SRC_SLIDER")) {

            if (read.csv.val[2] == 110 || read.csv.val[2] == 111) { srcNow++; continue; }

            SRC* src = (SRC*)(arr_SRC.Get_new());

            src->gr = read.csv.val[2];
            src->x = read.csv.val[3];
            src->y = read.csv.val[4];

            SRCGR& img = ((SRCGR*)arr_SRCGR.data)[src->gr];

            src->sizeX = read.csv.val[5];// == -1 ? img.sizeX - src->x : read.csv.val[5];
            src->sizeY = read.csv.val[6];// == -1 ? img.sizeY - src->y : read.csv.val[6];

            src->div_x = read.csv.val[7]; 
            src->div_y = read.csv.val[8];
            src->cycle = read.csv.val[9];
            src->timer = read.csv.val[10];

            src->muki = read.csv.val[11];
            src->range = read.csv.val[12];
            src->type = read.csv.val[13];
            src->disable = read.csv.val[14];

            src->declare = read.numTotal;

            char tmp[260];
            if (src->timer || src->type) {
                sprintf(tmp, "T%d(%s) : %s %d*%d ##%d", src->timer, timerName(src->timer), sliderName(src->type), src->sizeX, src->sizeY, read.numTotal);
            }
            else {
                sprintf(tmp, "noname : %d*%d ##%d", src->sizeX, src->sizeY, read.numTotal);
            }
            src->name.assign(tmp);

            srcNow++;
        }
        else if (read.csv.str[0].isSame("#SRC_BUTTON")) {

            if (read.csv.val[2] == 110 || read.csv.val[2] == 111) { srcNow++; continue; }

            SRC* src = (SRC*)(arr_SRC.Get_new());

            src->gr = read.csv.val[2];
            src->x = read.csv.val[3];
            src->y = read.csv.val[4];

            SRCGR& img = ((SRCGR*)arr_SRCGR.data)[src->gr];

            src->sizeX = read.csv.val[5];// == -1 ? img.sizeX - src->x : read.csv.val[5];
            src->sizeY = read.csv.val[6];// == -1 ? img.sizeY - src->y : read.csv.val[6];

            src->div_x = read.csv.val[7];
            src->div_y = read.csv.val[8];
            src->cycle = read.csv.val[9];
            src->timer = read.csv.val[10];

            src->type = read.csv.val[11];
            src->click = read.csv.val[12];
            src->panel = read.csv.val[13];
            src->plusonly = read.csv.val[14];

            src->declare = read.numTotal;

            char tmp[260];
            if (src->timer || src->type) {
                sprintf(tmp, "T%d(%s) : %s %d*%d ##%d", src->timer, timerName(src->timer), buttonName(src->type), src->sizeX, src->sizeY, read.numTotal);
            }
            else {
                sprintf(tmp, "noname : %d*%d ##%d", src->sizeX, src->sizeY, read.numTotal);
            }
            src->name.assign(tmp);

            srcNow++;
        }
        else if (read.csv.str[0].isSame("#SRC_BARGRAPH")) {

            if (read.csv.val[2] == 110 || read.csv.val[2] == 111) { srcNow++; continue; }

            SRC* src = (SRC*)(arr_SRC.Get_new());

            src->gr = read.csv.val[2];
            src->x = read.csv.val[3];
            src->y = read.csv.val[4];

            SRCGR& img = ((SRCGR*)arr_SRCGR.data)[src->gr];

            src->sizeX = read.csv.val[5];// == -1 ? img.sizeX - src->x : read.csv.val[5];
            src->sizeY = read.csv.val[6];// == -1 ? img.sizeY - src->y : read.csv.val[6];

            src->div_x = read.csv.val[7];
            src->div_y = read.csv.val[8];
            src->cycle = read.csv.val[9];
            src->timer = read.csv.val[10];

            src->type = read.csv.val[11];
            src->muki = read.csv.val[12];

            src->declare = read.numTotal;

            char tmp[260];
            if (src->timer || src->type) {
                sprintf(tmp, "T%d(%s) : %s %d*%d ##%d", src->timer, timerName(src->timer), bargraphName(src->type), src->sizeX, src->sizeY, read.numTotal);
            }
            else {
                sprintf(tmp, "noname : %d*%d ##%d", src->sizeX, src->sizeY, read.numTotal);
            }
            src->name.assign(tmp);

            srcNow++;
         }
        else if (read.csv.str[0].isSame("#SRC_TEXT")) {

            if (read.csv.val[2] == 110 || read.csv.val[2] == 111) { srcNow++; continue; }

            SRC* src = (SRC*)(arr_SRC.Get_new());

            src->gr = read.csv.val[2];
            src->x = read.csv.val[3];
            src->y = read.csv.val[4];

            SRCGR& img = ((SRCGR*)arr_SRCGR.data)[src->gr];

            src->sizeX = read.csv.val[5];// == -1 ? img.sizeX - src->x : read.csv.val[5];
            src->sizeY = read.csv.val[6];// == -1 ? img.sizeY - src->y : read.csv.val[6];

            src->div_x = read.csv.val[7]; //have to be 10, 11, 24 set divs
            src->div_y = read.csv.val[8];
            src->cycle = read.csv.val[9];
            src->timer = read.csv.val[10];

            src->num = read.csv.val[11];
            src->align = read.csv.val[12];
            src->keta = read.csv.val[13];

            src->declare = read.numTotal;

            char tmp[260];
            if (src->timer || src->type) {
                sprintf(tmp, "T%d(%s) : %s %d*%d ##%d", src->timer, timerName(src->timer), numberName(src->num), src->sizeX, src->sizeY, read.numTotal);
            }
            else {
                sprintf(tmp, "noname : %d*%d ##%d", src->sizeX, src->sizeY, read.numTotal);
            }
            src->name.assign(tmp);
            srcNow++;
        }
        else if (read.csv.str[0].isSame("#SRC_ONMOUSE")) {

            if (read.csv.val[2] == 110 || read.csv.val[2] == 111) { srcNow++; continue; }

            SRC* src = (SRC*)(arr_SRC.Get_new());

            src->gr = read.csv.val[2];
            src->x = read.csv.val[3];
            src->y = read.csv.val[4];

            SRCGR& img = ((SRCGR*)arr_SRCGR.data)[src->gr];

            src->sizeX = read.csv.val[5];// == -1 ? img.sizeX - src->x : read.csv.val[5];
            src->sizeY = read.csv.val[6];// == -1 ? img.sizeY - src->y : read.csv.val[6];

            src->div_x = read.csv.val[7]; //have to be 10, 11, 24 set divs
            src->div_y = read.csv.val[8];
            src->cycle = read.csv.val[9];
            src->timer = read.csv.val[10];

            src->num = read.csv.val[11];
            src->align = read.csv.val[12];
            src->keta = read.csv.val[13];

            src->declare = read.numTotal;

            char tmp[260];
            if (src->timer || src->type) {
                sprintf(tmp, "T%d(%s) : %s %d*%d ##%d", src->timer, timerName(src->timer), numberName(src->num), src->sizeX, src->sizeY, read.numTotal);
            }
            else {
                sprintf(tmp, "noname : %d*%d ##%d", src->sizeX, src->sizeY, read.numTotal);
            }
            src->name.assign(tmp);
            }
        else if (read.csv.str[0].isSame("#SRC_BGA")) {

            if (read.csv.val[2] == 110 || read.csv.val[2] == 111) { srcNow++; continue; }

            SRC* src = (SRC*)(arr_SRC.Get_new());

            src->gr = read.csv.val[2];
            src->x = read.csv.val[3];
            src->y = read.csv.val[4];

            SRCGR& img = ((SRCGR*)arr_SRCGR.data)[src->gr];

            src->sizeX = read.csv.val[5];// == -1 ? img.sizeX - src->x : read.csv.val[5];
            src->sizeY = read.csv.val[6];// == -1 ? img.sizeY - src->y : read.csv.val[6];

            src->div_x = read.csv.val[7]; //have to be 10, 11, 24 set divs
            src->div_y = read.csv.val[8];
            src->cycle = read.csv.val[9];
            src->timer = read.csv.val[10];

            src->num = read.csv.val[11];
            src->align = read.csv.val[12];
            src->keta = read.csv.val[13];

            src->declare = read.numTotal;

            char tmp[260];
            if (src->timer || src->type) {
                sprintf(tmp, "T%d(%s) : %s %d*%d ##%d", src->timer, timerName(src->timer), numberName(src->num), src->sizeX, src->sizeY, read.numTotal);
            }
            else {
                sprintf(tmp, "noname : %d*%d ##%d", src->sizeX, src->sizeY, read.numTotal);
            }
            src->name.assign(tmp);
            srcNow++;
        }

    }
    
    //images load
    int gr = 0;
    for (int n = 0; n < arr_SRCGR.count; n++) {
        SRCGR& img = ((SRCGR*)arr_SRCGR.data)[n];
        CSTR& path = ((SRCGR*)arr_SRCGR.data)[n].path;

        char siblingAsset[MAX_PATH] = {};
        if (ResolveSiblingPlayPath(path.outstr(), mainpath,
            siblingAsset, sizeof(siblingAsset)))
            path.assign(siblingAsset);

        WriteSkinLoadLog("ParseSkin image", path.outstr());

        if (path.isSame("CONTINUE")) {
            gr++;
            
            img.sizeX = 0;
            img.sizeY = 0;
            continue;
        }

        //TODO: deal *

		// Complex skins list hundreds of wildcard/custom candidates. Uploading
		// every candidate here exhausts the 32-bit address space. Preserve the
		// metadata and create the editor texture only when it becomes visible.
		img.texture = NULL;
		img.loaded = false;
		img.sizeX = 0;
		img.sizeY = 0;
		GetImageSizeFromFile(path.outstr(), &img.sizeX, &img.sizeY);
    }
    return 0;
}

int WORKSPACE::LoadSkin(char* path) {
    remove("SkinEditor_load_crash.log");
    WriteSkinLoadLog("LoadSkin begin", path);
    skinSizeX = meta.targetX;
    skinSizeY = meta.targetY;

    skinfileLines.Free();
    skinfileLines.Alloc(sizeof(SKINFILELINEREAD), 1000);
    arr_subpath.Free();
    arr_subpath.Alloc(sizeof(CSTR), 1);
    arr_ifunit.Free();
    arr_ifunit.Alloc(sizeof(IFUNIT), 50);

    // Release the editor-side D3D textures before discarding their records.
    // This is especially important when switching away from an HD skin with
    // hundreds of customization images.
    for (int i = 0; i < arr_SRCGR.count; ++i) {
        SRCGR& oldImage = ((SRCGR*)arr_SRCGR.data)[i];
        if (oldImage.texture != NULL) {
            oldImage.texture->Release();
            oldImage.texture = NULL;
        }
    }
    arr_SRCGR.Free();
    arr_SRCGR.Alloc(sizeof(SRCGR), 10);
    arr_SRC.Free();
    arr_SRC.Alloc(sizeof(SRC), 100);
    arr_CustomFile.Free();
    arr_CustomFile.Alloc(sizeof(CSTR), 10);

    for (int i = 0; i < arr_DST.count; i++)
        ((DST*)arr_DST.data)[i].arr_animation.Free();
    arr_DST.Free();
    arr_DST.Alloc(sizeof(DST), 100);
    for (int i = 0; i < arr_DST.count; i++)
        ((DST*)arr_DST.data)[i].arr_animation.Alloc(sizeof(DST_ANIMATION), 1);

    arr_IMG.Free();
    arr_IMG.Alloc(sizeof(IMG),400);
    arr_seobj.Free();
    arr_seobj.Alloc(sizeof(SEOBJ),400);

    arr_history.Free();
    arr_history.Alloc(sizeof(HISTORY), 1);

    //LR2
    // Delete graph handles owned by the previously loaded preview before the
    // skstruct is reset. Otherwise memset loses the handles and leaks all
    // DxLib graph memory across skin changes.
    for (int i = 0; i < g.skstruct.count && i < 200; ++i) {
        if (g.skstruct.GrHandle[i] >= 0) {
            DeleteGraph(g.skstruct.GrHandle[i]);
            g.skstruct.GrHandle[i] = -1;
        }
    }

    // Changing DxLib's graph mode invalidates every graph handle. Do it before
    // loading the new skin; doing it after ReadSkinSE made HD skins lose all
    // of the images that had just been loaded. 640x480 skins appeared to work
    // only because they often did not trigger a real device reset.
    DeleteGraph(previewScreen);
    previewScreen = -1;
    if (texture_preview) {
        texture_preview->Release();
        texture_preview = NULL;
    }
    texture_preview_width = 0;
    texture_preview_height = 0;
    previewLastRenderAt = 0;
    previewTextureDirty = true;
    SetGraphMode(skinSizeX, skinSizeY, 32, 60);
    SetDrawScreen(DX_SCREEN_BACK);

    for (int i = 0; i < 200; i++) g.skstruct.caption[i].fillzero();
    for (int i = 0; i < 10; i++) g.skstruct.helpfilePath[i].fillzero();
    for (int i = 0; i < 20; i++) g.skstruct.customfileRANDOM[i].fillzero();
    for (int i = 0; i < 20; i++) g.skstruct.customfile[i].fillzero();
    g.skstruct.skinMD5.fillzero();
    g.skstruct.skFontname.fillzero();
    memset(&g.skstruct, 0, sizeof(skstruct));
    for (int i = 0; i < 200; i++) g.skstruct.caption[i].fillzero();
    for (int i = 0; i < 200; i++) g.skstruct.caption[i].assign("(null)");
    for (int i = 0; i < 200; i++) g.skstruct.GrHandle[i] = -1;
    for (int i = 0; i < 10; i++) g.skstruct.helpfilePath[i].fillzero();
    for (int i = 0; i < 10; i++) g.skstruct.helpfilePath[i].assign("(null)");
    //g.skstruct.skFontname.assign(&config.skin.fontname);
    //g.skstruct.disableimagefont = (config.skin.disableimagefont != 0);
    g.skstruct.skinMD5.fillzero();
    g.skstruct.skinMD5.resize2(34);
    AllocDrawingBuffer(&g.skstruct.drBuf);
    //LoadScene(&g.skstruct, mainpath, 0, 0);

    //SE
    WriteSkinLoadLog("LoadSkinScript begin", path);
    loadScriptStack.clear();
    if (LoadSkinScript(path) != 0) {
        WriteSkinLoadLog("LoadSkinScript failed", path);
        return -1;
    }
    WriteSkinLoadLog("LoadSkinScript complete");

    // Dynamic #IF options (clear/fail, rank, key mode, difficulty, etc.) are
    // used while ParseSkin decides which image branches to keep. Prepare the
    // standalone scene state first; doing this after ParseSkin left complex
    // RESULT skins with only their unconditional chart objects.
    LR2SEInit(&g);
    LR2SEPreparePreviewState(&g, meta.type);
    WriteSkinLoadLog("LR2SE preview state prepared");
    ParseSkin();
    WriteSkinLoadLog("ParseSkin complete");
    if (g_seObjectEditorModel.Groups().empty()) g_seObjectEditorModel.LoadGroups("skinObjGroup.txt");
    g_seObjectEditorModel.Rebuild(*this);
    WriteSkinLoadLog("ObjectEditor Rebuild complete");
    
    WriteSkinLoadLog("LR2SEInit complete");
    LoadSceneSE();
    WriteSkinLoadLog("LoadSceneSE complete");
    LR2SEResetRenderFault();
    previewReloadPending = false;

    //MakeObjects();
    





    SetWindowUserCloseEnableFlag(0); //DxLib

    
    //previewScreen = LoadGraph("ex.bmp");
    //previewScreen = MakeGraph(skinSizeX, skinSizeY); //MakeScreen vs MakeGraph
    //SetDrawScreen(previewScreen);
    previewScreen = MakeSoftImage(skinSizeX, skinSizeY);
    WriteSkinLoadLog("LoadSkin complete");
    //previewScreen = MakeARGB8ColorSoftImage(skinSizeX, skinSizeY); //for SDL3

    wPreview = 1;
    wObjectEditor = true;
    preview_selected_object_model_indices.clear();
    preview_selected_object_model_index = -1;
    preview_selection_anchor_model_index = -1;
    preview_selected_obj_valid = false;
    preview_selected_obj_last_valid = false;
    wImgManager = true;
    wDstView = true;
    wOpList = true;
    wCustomize = true;
    wTreeView = false;
    dockLayoutBuilt = false;
    wObjectManagerTest = false;

    return 0;
}

//copied from LR2 LoadScene
int WORKSPACE::LoadSceneSE() {
    skstruct* sk = &g.skstruct;

    InitSkin(sk, 0, false);

    //copy of ReadSkinCustomize()
    sk->adjust.dark_type = 0;
    sk->adjust.judge_x = 0;
    sk->adjust.judge_y = 0;
    sk->adjust.note_1p_x = 0;
    sk->adjust.note_1p_y = 0;
    sk->adjust.note_2p_x = 0;
    sk->adjust.note_2p_y = 0;
    sk->adjust.unk18 = 0;
    sk->adjust.unk1c = 0;
    sk->adjust.size_x = 0;
    sk->adjust.size_y = 0;
    sk->adjust.shift_x = 0;
    sk->adjust.shift_y = 0;
    sk->adjust.rate_x = 100;
    sk->adjust.rate_y = 100;
    
    // Apply the skin list's selected customization before evaluating #IF.
    // Many real skins place the whole visible layout under options 900-999;
    // leaving those options unset produces a valid but completely black preview.
    sk->op[0] = 1;
    sk->op[999] = 0;
    // DECIDE skins cannot inherit #IMAGE,CONTINUE resources when opened as a
    // standalone editor document.  Prefer an explicitly supplied SELECT LINK
    // OFF branch so its self-contained image table is used.
    if (meta.type == SKINTYPE_DECIDE) {
        for (int i = 0; i < meta.custom_count; ++i) {
            SkinCustom& custom = meta.customs[i];
            if (_stricmp(custom.title.outstr(), "SELECT LINK") != 0) continue;
            for (int option = 0; option < custom.dst_op_count; ++option) {
                if (_stricmp(custom.op_label[option].outstr(), "OFF") == 0) {
                    custom.dst_op_selected = option;
                    break;
                }
            }
        }
    }
    for (int i = 0; i < meta.custom_count; ++i) {
        const SkinCustom& custom = meta.customs[i];
        if (custom.dst_op_start <= 0) continue;
        for (int option = 0; option < custom.dst_op_count; ++option) {
            const int op = custom.dst_op_start + option;
            if (op >= 900 && op < 1000) sk->op[op] = 0;
        }
        const int selectedOp = custom.dst_op_start + custom.dst_op_selected;
        if (selectedOp >= 900 && selectedOp < 1000) sk->op[selectedOp] = 1;
    }
    // LR2 refreshes all built-in options immediately before loading a scene.
    // These include RESULT_CLEAR/FAIL, rank, key mode and difficulty. The
    // editor previously initialized only customization options (900-999).
    LR2SEPreparePreviewState(&g, meta.type);
    for (int option = 0; option < 900; ++option)
        sk->op[option] = GetOptionFlag_dst(&g, option) ? 1 : 0;

    const int result = ReadSkinSE();
    {
        int validImageHandles = 0;
        int drawableImages = 0;
        for (int image = 0; image < sk->count; ++image) {
            if (sk->GrHandle[image] >= 0) ++validImageHandles;
        }
        for (int source = 0; source < sk->image.srcSize; ++source) {
            if (sk->image.src[source].graphcount > 0 &&
                sk->image.dst[source].dstCount > 0)
                ++drawableImages;
        }
        char runtimeSummary[512];
        sprintf_s(runtimeSummary,
            "Runtime scene images=%d validHandles=%d imageSrc=%d drawable=%d "
            "customFiles=%d op90=%d op91=%d op200=%d op201=%d result=%d",
            sk->count, validImageHandles, sk->image.srcSize, drawableImages,
            sk->customfile_count, sk->op[90], sk->op[91], sk->op[200],
            sk->op[201], result);
        WriteSkinLoadLog(runtimeSummary);

        const int detailCount = (std::min)(sk->image.srcSize, 24);
        for (int source = 0; source < detailCount; ++source) {
            SRCstruct& src = sk->image.src[source];
            DSTstruct& dst = sk->image.dst[source];
            const int graphHandle = src.graphcount > 0 ? src.grHandles[0] : -1;
            char sourceSummary[256];
            sprintf_s(sourceSummary,
                "Runtime imageSrc[%d] gr=%d handle=%d graphs=%d dst=%d timer=%d op=%d/%d/%d",
                source, src.n, graphHandle, src.graphcount, dst.dstCount,
                dst.timer, dst.op1, dst.op2, dst.op3);
            WriteSkinLoadLog(sourceSummary);
        }
    }
    // ReadSkinSE returns the accumulated skin sort/BGA number, not a success
    // code.  Complex skins commonly return a positive value, so gating scene
    // preparation on result == 0 left SELECT state entirely uninitialized.
    if (meta.type == SKINTYPE_SELECT) {
            char selectState[512];
            sprintf_s(selectState,
                "SELECT state center=%d songs=%d imageCount=%d image0=%d image9=%d "
                "body0Graphs=%d body0Handle=%d off9=%d/%d off10=%d/%d "
                "on10=%d/%d title0Graphs=%d title0Dst=%d/%d",
                g.skstruct.BAR_CENTER, g.sSelect.bmsListCount, g.skstruct.count,
                g.skstruct.GrHandle[0], g.skstruct.GrHandle[9],
                g.skstruct.src_BAR_BODY[0].graphcount,
                g.skstruct.src_BAR_BODY[0].graphcount > 0
                    ? g.skstruct.src_BAR_BODY[0].grHandles[0] : -1,
                g.skstruct.dst_BAR_BODY_OFF[9].dstCount,
                g.skstruct.dst_BAR_BODY_OFF[9].dataSize,
                g.skstruct.dst_BAR_BODY_OFF[10].dstCount,
                g.skstruct.dst_BAR_BODY_OFF[10].dataSize,
                g.skstruct.dst_BAR_BODY_ON[10].dstCount,
                g.skstruct.dst_BAR_BODY_ON[10].dataSize,
                g.skstruct.src_BAR_TITLE[0].graphcount,
                g.skstruct.dst_BAR_TITLE[0].dstCount,
                g.skstruct.dst_BAR_TITLE[0].dataSize);
        WriteSkinLoadLog(selectState);
    }
    else if (meta.type == SKINTYPE_DECIDE) {
        int validImages = 0;
        int validSources = 0;
        int firstValidSource = -1;
        int firstValidHandle = -1;
        for (int i = 0; i < g.skstruct.count; ++i)
            if (g.skstruct.GrHandle[i] >= 0) ++validImages;
        for (int i = 0; i < g.skstruct.image.srcSize; ++i) {
            SRCstruct& src = g.skstruct.image.src[i];
            bool valid = false;
            for (int frame = 0; frame < src.graphcount; ++frame) {
                if (src.grHandles && src.grHandles[frame] >= 0) {
                    valid = true;
                    if (firstValidSource < 0) {
                        firstValidSource = i;
                        firstValidHandle = src.grHandles[frame];
                    }
                    break;
                }
            }
            if (valid) ++validSources;
        }
        char decideState[384];
        sprintf_s(decideState,
            "DECIDE state images=%d/%d sources=%d/%d first=%d:%d "
            "dst0=%d/%d proc=%d phase=%d timer0=%.0f",
            validImages, g.skstruct.count, validSources, g.skstruct.image.srcSize,
            firstValidSource, firstValidHandle,
            g.skstruct.image.srcSize > 0 ? g.skstruct.image.dst[0].dstCount : 0,
            g.skstruct.image.srcSize > 0 ? g.skstruct.image.dst[0].dataSize : 0,
            g.procSelecter, g.procPhase, GetTimeLapse(0, &g.timer1));
        WriteSkinLoadLog(decideState);
    }
    return result;
}

// Persist the resolution from the skin picker without requiring the skin to
// be loaded first. LR2 stores the target width and height in fields 6 and 7 of
// #INFORMATION. The final replace is atomic so a failed write leaves the
// original skin untouched.
static bool SaveResolutionToSkinFile(const char* path, int width, int height) {
    if (!path || !*path || width <= 0 || height <= 0) return false;
    FILE* input = fopen(path, "rb");
    if (!input) return false;
    if (fseek(input, 0, SEEK_END) != 0) { fclose(input); return false; }
    const long fileSize = ftell(input);
    if (fileSize < 0 || fseek(input, 0, SEEK_SET) != 0) { fclose(input); return false; }
    std::string contents((size_t)fileSize, '\0');
    if (fileSize > 0 && fread(&contents[0], 1, (size_t)fileSize, input) != (size_t)fileSize) {
        fclose(input);
        return false;
    }
    fclose(input);

    size_t informationStart = std::string::npos;
    size_t informationEnd = std::string::npos;

    for (size_t lineStart = 0; lineStart <= contents.size();) {
        size_t lineBreak = contents.find('\n', lineStart);
        const size_t fullEnd = lineBreak == std::string::npos ? contents.size() : lineBreak + 1;
        size_t contentEnd = lineBreak == std::string::npos ? contents.size() : lineBreak;
        if (contentEnd > lineStart && contents[contentEnd - 1] == '\r') --contentEnd;
        size_t commandStart = lineStart;
        if (commandStart == 0 && contents.size() >= 3 &&
            (unsigned char)contents[0] == 0xEF && (unsigned char)contents[1] == 0xBB &&
            (unsigned char)contents[2] == 0xBF) commandStart = 3;
        while (commandStart < contentEnd &&
            (contents[commandStart] == ' ' || contents[commandStart] == '\t')) ++commandStart;
        const size_t commandLength = contentEnd - commandStart;
        if (commandLength >= 12 &&
            _strnicmp(contents.c_str() + commandStart, "#INFORMATION", 12) == 0 &&
            (commandLength == 12 || contents[commandStart + 12] == ',' ||
                contents[commandStart + 12] == ' ' || contents[commandStart + 12] == '\t')) {
            informationStart = commandStart;
            informationEnd = contentEnd;
            break;
        }
        if (lineBreak == std::string::npos) break;
        lineStart = fullEnd;
    }

    if (informationStart == std::string::npos) return false;

    std::string information = contents.substr(informationStart, informationEnd - informationStart);
    std::vector<std::string> fields;
    size_t fieldStart = 0;
    for (;;) {
        const size_t comma = information.find(',', fieldStart);
        fields.push_back(information.substr(fieldStart,
            comma == std::string::npos ? std::string::npos : comma - fieldStart));
        if (comma == std::string::npos) break;
        fieldStart = comma + 1;
    }
    while (fields.size() < 8) fields.push_back("");
    fields[6] = std::to_string(width);
    fields[7] = std::to_string(height);

    std::string updatedInformation;
    for (size_t i = 0; i < fields.size(); ++i) {
        if (i) updatedInformation += ',';
        updatedInformation += fields[i];
    }
    contents.replace(informationStart, informationEnd - informationStart, updatedInformation);

    const std::string tempPath = std::string(path) + ".resolution.tmp";
    FILE* output = fopen(tempPath.c_str(), "wb");
    if (!output) return false;
    const bool writeOk = contents.empty() ||
        fwrite(contents.data(), 1, contents.size(), output) == contents.size();
    const bool closeOk = fclose(output) == 0;
    if (!writeOk || !closeOk) {
        remove(tempPath.c_str());
        return false;
    }
    if (!MoveFileExA(tempPath.c_str(), path,
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        remove(tempPath.c_str());
        return false;
    }
    return true;
}

static std::vector<unsigned char> BuildPreviewRuntimeMask(ARR& skinfileLines, skstruct* sk) {
    struct ConditionalState {
        bool outerActive = true;
        bool branchTaken = false;
        bool currentActive = true;
    };
    struct FileState {
        bool enabled = true;
        bool pendingIncludeEnabled = false;
        std::vector<ConditionalState> conditions;
    };

    std::vector<unsigned char> enabled((size_t)skinfileLines.count, 0);
    std::vector<FileState> files;

    auto currentActive = [&]() -> bool {
        if (files.empty() || !files.back().enabled) return false;
        return files.back().conditions.empty() ||
            files.back().conditions.back().currentActive;
    };
    auto matches = [&](const CSVbuf& csv) -> bool {
        for (int column = 1; column < 10; ++column) {
            const int option = csv.val[column];
            if (option < 0 || option > 999 || sk->op[option] == 0)
                return false;
        }
        return true;
    };

    for (int rowIndex = 0; rowIndex < skinfileLines.count; ++rowIndex) {
        SKINFILELINEREAD& row = ((SKINFILELINEREAD*)skinfileLines.data)[rowIndex];
        const char* text = row.line.body ? row.line.outstr() : "";

        if (row.isSEcomment && !strncmp(text, "$FILE ", 6)) {
            const bool isStart = strstr(text, " start") != NULL;
            const bool isEnd = strstr(text, " end") != NULL;
            if (isStart) {
                FileState state;
                state.enabled = files.empty() ? true : files.back().pendingIncludeEnabled;
                if (!files.empty()) files.back().pendingIncludeEnabled = false;
                files.push_back(state);
            }
            else if (isEnd && !files.empty()) {
                files.pop_back();
            }
            continue;
        }
        if (files.empty() || row.isComment) continue;

        const char* command = row.csv.str[0].body ? row.csv.str[0].outstr() : "";
        FileState& file = files.back();
        if (!_stricmp(command, "#IF")) {
            ConditionalState condition;
            condition.outerActive = currentActive();
            const bool branchMatches = matches(row.csv);
            condition.branchTaken = branchMatches;
            condition.currentActive = condition.outerActive && branchMatches;
            file.conditions.push_back(condition);
            continue;
        }
        if (!_stricmp(command, "#ELSEIF")) {
            if (!file.conditions.empty()) {
                ConditionalState& condition = file.conditions.back();
                const bool branchMatches = !condition.branchTaken && matches(row.csv);
                condition.currentActive = condition.outerActive && branchMatches;
                condition.branchTaken = condition.branchTaken || branchMatches;
            }
            continue;
        }
        if (!_stricmp(command, "#ELSE")) {
            if (!file.conditions.empty()) {
                ConditionalState& condition = file.conditions.back();
                const bool branchMatches = !condition.branchTaken;
                condition.currentActive = condition.outerActive && branchMatches;
                condition.branchTaken = true;
            }
            continue;
        }
        if (!_stricmp(command, "#ENDIF")) {
            if (!file.conditions.empty()) file.conditions.pop_back();
            continue;
        }

        const bool active = currentActive();
        if (!_stricmp(command, "#INCLUDE")) {
            file.pendingIncludeEnabled = active;
            continue;
        }
        if (!active) continue;

        enabled[(size_t)rowIndex] = 1;
        if (!_stricmp(command, "#SETOPTION")) {
            const int option = row.csv.val[1];
            if (option >= 0 && option < 1000)
                sk->op[option] = row.csv.val[2] != 0;
        }
    }
    return enabled;
}

int WORKSPACE::ReadSkinSE() {
    
    CSTR dir(mainpath);
    dir.assign(dir.getDirectory());
    bool flag_skipFont = false;

    CSVbuf csv;

    int tSkin_num = 0;
    int line = 0;
    int IFCOUNT = 0, IFSWITCH[100] = {};

    bool flipside = false;

    skstruct* sk = &g.skstruct;
    const std::vector<unsigned char> runtimeLineEnabled =
        BuildPreviewRuntimeMask(skinfileLines, sk);


    for (int i = 0; i < skinfileLines.count; i++) {

        ExpandSkinObjectMax(&g.skstruct.image, 50);
        ExpandSkinObjectMax(&g.skstruct.otherObject[0], 20);
        ExpandSkinObjectMax(&g.skstruct.otherObject[1], 20);
        ExpandSkinObjectMax(&g.skstruct.otherObject[2], 20);
        ExpandSkinObjectMax(&g.skstruct.otherObject[3], 20);
        ExpandSkinObjectMax(&g.skstruct.otherObject[4], 20);
        ExpandSkinObjectMax(&g.skstruct.otherObject[6], 20);
        ExpandSkinObjectMax(&g.skstruct.otherObject[7], 20);
        ExpandSkinObjectMax(&g.skstruct.otherObject[5], 20);


        SKINFILELINEREAD& read = ((SKINFILELINEREAD*)(skinfileLines.data))[i];
        if (!runtimeLineEnabled[(size_t)i]) continue;
        // ReadSkinSE normalizes and temporarily edits its input while parsing.
        // Never do that to the editor's authoritative line: repeated preview
        // rebuilds would otherwise remove one real character on every pass.
        CSTR fBuf(read.line);
        line++;

        char loadLine[128];
        sprintf_s(loadLine, "expanded line %d, source line %d, command %.64s", i, read.num, fBuf.outstr());
#ifdef _DEBUG
        WriteSkinLoadLog("ReadSkinSE", loadLine);
#endif

        if (read.line.length() <= 6) continue;
        if (strncmp(read.line.atPos(0), "#", 1)) continue;

        while (fBuf.length() > 0) {
            const char tail = *fBuf.atPos(fBuf.length() - 1);
            if (tail != '\r' && tail != '\n') break;
            fBuf.nullAtPos(fBuf.length() - 1);
        }
        fBuf.trimWhiteSpace();
        DealWhiteSpace(&fBuf);
        if (!fBuf.left(1).isDiff("#")) {
            if (fBuf.left(3).isSame("#IF")) {
                if (IFCOUNT != 99) {
                    IFSWITCH[IFCOUNT + 1] = 1;
                    SplitCSV(fBuf, &csv, ",");
                    for (int i = 1; i < 10; i++) {
                        if (csv.val[i] < 0 || csv.val[i]>999 || sk->op[csv.val[i]] == 0) {
                            i = 10;
                            IFSWITCH[IFCOUNT + 1] = 2;
                        }
                    }
                    IFCOUNT++;
                }
                else {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\nネスト可?な#IFの上限に達しました。\n", line, fBuf);
                    if (IFSWITCH[IFCOUNT] > 1) {
                        *fBuf.atPos(0) = '\0';
                        continue;
                    }
                }
            }
            else if (fBuf.left(7).isSame("#ELSEIF") && IFSWITCH[IFCOUNT] != 3) {
                if (IFCOUNT) {
                    if (IFSWITCH[IFCOUNT] == 1) IFSWITCH[IFCOUNT] = 3;
                    else {
                        IFSWITCH[IFCOUNT] = 1;
                        SplitCSV(fBuf, &csv, ",");
                        for (int i = 1; i < 10; i++) {
                            if (csv.val[i] < 0 || csv.val[i]>999 || sk->op[csv.val[i]] == 0) {
                                i = 10;
                                IFSWITCH[IFCOUNT] = 2;
                            }
                        }
                    }
                }
                else ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n対応する#IFが見つかりません。\n", line, fBuf);
            }
            else if (fBuf.left(5).isSame("#ELSE") && IFSWITCH[IFCOUNT] != 3) {
                if (IFCOUNT == 0) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n対応する#IFが見つかりません。\n", line, fBuf);
                }
                else {
                    IFSWITCH[IFCOUNT] = (IFSWITCH[IFCOUNT] == 1) ? 3 : 1;
                }
            }
            else if (fBuf.left(6).isSame("#ENDIF")) {
                if (IFCOUNT == 0) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n対応する#IFが見つかりません。\n", line, fBuf);
                }
                else {
                    IFSWITCH[IFCOUNT] = 0;
                    IFCOUNT--;
                }
            }
        }

        if (IFCOUNT > 0) {
            bool inactiveAncestor = false;
            for (int depth = 1; depth <= IFCOUNT; ++depth) {
                if (IFSWITCH[depth] > 1) {
                    inactiveAncestor = true;
                    break;
                }
            }
            if (inactiveAncestor) {
                // Every enclosing condition must be active. Checking only the
                // innermost level allowed a nested #IF to leak objects out of
                // an inactive parent branch and overwrite image slots.
                continue;
            }
        }


        if (!fBuf.left(1).isDiff("#")) {
            if (fBuf.left(6).isSame("#IMAGE")) {
                if (sk->count == 100) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\nこれ以上#IMAGEを登?できません。\n", line, fBuf);
                }
                else {
                    SplitCSV(fBuf, &csv, ",");
                    if (csv.str[1].isSame("CONTINUE")) {
                        sk->caption[sk->count].assign("CONTINUE");
                        sk->count++;
                    }
                    else {
                        // A failed LoadGraph still leaves the requested path in
                        // caption.  Retrying the same skin then used to skip the
                        // load forever and produced a permanently black preview.
                        if (sk->GrHandle[sk->count] == -1 ||
                            sk->caption[sk->count].isDiff(&csv.str[1]) ||
                            sk->grIsMovie[sk->count] == 1 ||
                            sk->caption[sk->count].findStrPos("*") != -1) {
                            DeleteGraph(sk->GrHandle[sk->count]);
                            sk->caption[sk->count].assign(&csv.str[1]);
                            if (csv.str[1].right(3).isSame("mpg") || csv.str[1].right(3).isSame("avi") || csv.str[1].right(3).isSame("wma") || csv.str[1].right(3).isSame("ogv")) {
                                sk->grIsMovie[sk->count] = 1;
                            }
                            else {
                                sk->grIsMovie[sk->count] = 0;
                            }
                            for (int i = 0; i < sk->customfile_count; i++) {
                                if (sk->customfileRANDOM[i].isSame(csv.str[1].left(sk->customfileRANDOM[i].length())) && sk->customfile[i].isDiff("RANDOM") != 0 && sk->customfile[i].isDiff("ERROR") && sk->customfile[i].length() > 0) {
                                    csv.str[1].replace("*", sk->customfile[i]);
                                    break;
                                }
                            }
                            char siblingImage[MAX_PATH] = {};
                            if (ResolveSiblingPlayPath(csv.str[1].outstr(), mainpath,
                                siblingImage, sizeof(siblingImage)))
                                csv.str[1].assign(siblingImage);
                            CSTR temp(GetRandomFileNoError(csv.str[1], dir), 0);
                            sk->GrHandle[sk->count] = LoadGraph(temp);
                            sk->caption[sk->count].assign(&temp);
                        }
                        sk->count++;
                    }
                }
            }
            else if (fBuf.left(5).isSame("#FONT") && !flag_skipFont) {
                if (sk->num_of_struct == 10) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\nこれ以上#FONTを登?できません。\n", line, fBuf);
                }
                else {
                    SplitCSV(fBuf, &csv, ",");
                    //sk->fontHandle[sk->num_of_struct] = CreateFontToHandle(sk->fontname, csv.val[1], csv.val[2], csv.val[3], 0, -1, 0, -1, -1); //DxLib3.02
                    sk->fontHandle[sk->num_of_struct] = CreateFontToHandle(sk->fontname, csv.val[1], csv.val[2], csv.val[3], 0, -1, 0, -1); //DxLib3.24f
                    if (sk->fontHandle[sk->num_of_struct] == -1) {
                        sk->fontHandle[sk->num_of_struct] = 0;
                    }
                    sk->num_of_struct = sk->num_of_struct + 1;
                }
            }
            else if (fBuf.left(10).isSame("#SRC_IMAGE")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->image.src[sk->image.srcSize], &csv, sk);
                if (sk->image.src[sk->image.srcSize].graphcount < 1 || sk->image.src[sk->image.srcSize].count < 1) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n画像の登?に失敗しました。\n", line, fBuf);
                }
                if (sk->image.srcSize > 0 && (sk->image.dst[sk->image.srcSize - 1].dstCount < 1 || sk->image.dst[sk->image.srcSize - 1].dataSize < 1)) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n(この行のエラ?ではありません)ひとつ前の#SRC_IMAGEに対応した#DST_IMAGEが存在しないか、登?に失敗したようです\n", line, fBuf);
                }
                sk->image.srcSize++;
            }
            else if (fBuf.left(10).isSame("#DST_IMAGE") && sk->image.srcSize > 0) {
                int oldDstCount = sk->image.dst[sk->image.srcSize - 1].dstCount;
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->image.dst[sk->image.srcSize - 1], &csv, tSkin_num);
                if (sk->image.dst[sk->image.srcSize - 1].dstCount < 1 || sk->image.dst[sk->image.srcSize - 1].dataSize < 1) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\nDSTの登?に失敗しました。DSTの一番最初がエラ?を起こしている可?性があります。\n", line, fBuf);
                }
                else if (sk->image.dst[sk->image.srcSize - 1].dstCount == oldDstCount) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\nDSTの登?に失敗しました。この行の登?のみ失敗しました。\n", line, fBuf);
                }
            }
            else if (fBuf.left(9).isSame("#SRC_TEXT")) {
                SplitCSV(fBuf, &csv, ",");
                sk->otherObject[0].src[sk->otherObject[0].srcSize].n = csv.val[1];
                sk->otherObject[0].src[sk->otherObject[0].srcSize].fontHandle = sk->fontHandle[csv.val[2]];
                sk->otherObject[0].src[sk->otherObject[0].srcSize].cycle = csv.val[2];
                sk->otherObject[0].src[sk->otherObject[0].srcSize].st = csv.val[3];
                sk->otherObject[0].src[sk->otherObject[0].srcSize].align = csv.val[4];
                sk->otherObject[0].src[sk->otherObject[0].srcSize].op1 = csv.val[5];
                sk->otherObject[0].src[sk->otherObject[0].srcSize].op2 = csv.val[6];
                sk->otherObject[0].src[sk->otherObject[0].srcSize].op3 = csv.val[7];
                sk->otherObject[0].src[sk->otherObject[0].srcSize].op4 = csv.val[8];
                sk->otherObject[0].src[sk->otherObject[0].srcSize].op5 = csv.val[9];
                sk->otherObject[0].srcSize++;
            }
            else if (fBuf.left(9).isSame("#DST_TEXT") && sk->otherObject[0].srcSize > 0) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->otherObject[0].dst[sk->otherObject[0].srcSize - 1], &csv, tSkin_num);
            }
            else if (fBuf.left(11).isSame("#SRC_SLIDER")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->otherObject[2].src[sk->otherObject[2].srcSize], &csv, sk);
                if (sk->otherObject[2].src[sk->otherObject[2].srcSize].graphcount < 1 || sk->otherObject[2].src[sk->otherObject[2].srcSize].count < 1) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n画像の登?に失敗しました。\n", line, fBuf);
                }
                if (sk->otherObject[2].srcSize > 0 && (sk->otherObject[2].dst[sk->otherObject[2].srcSize - 1].dstCount < 1 || sk->otherObject[2].dst[sk->otherObject[2].srcSize - 1].dataSize < 1)) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n(この行のエラ?ではありません)ひとつ前の#SRC_SLIDERに対応した#DST_SLIDERが存在しないか、登?に失敗したようです\n", line, fBuf);
                }
                sk->otherObject[2].srcSize++;
            }
            else if (fBuf.left(11).isSame("#DST_SLIDER") && sk->otherObject[2].srcSize > 0) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->otherObject[2].dst[sk->otherObject[2].srcSize - 1], &csv, tSkin_num);
            }
            else if (fBuf.left(11).isSame("#SRC_BUTTON")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->otherObject[1].src[sk->otherObject[1].srcSize], &csv, sk);
                if (sk->otherObject[1].src[sk->otherObject[1].srcSize].graphcount < 1 || sk->otherObject[1].src[sk->otherObject[1].srcSize].count < 1) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n画像の登?に失敗しました。\n", line, fBuf);
                }
                if (sk->otherObject[1].srcSize > 0 && (sk->otherObject[1].dst[sk->otherObject[1].srcSize - 1].dstCount < 1 || sk->otherObject[1].dst[sk->otherObject[1].srcSize - 1].dataSize < 1)) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n(この行のエラ?ではありません)ひとつ前の#SRC_BUTTONに対応した#DST_BUTTONが存在しないか、登?に失敗したようです\n", line, fBuf);
                }
                sk->otherObject[1].srcSize++;
            }
            else if (fBuf.left(11).isSame("#DST_BUTTON") && sk->otherObject[1].srcSize > 0) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->otherObject[1].dst[sk->otherObject[1].srcSize - 1], &csv, tSkin_num);

            }
            else if (fBuf.left(12).isSame("#SRC_ONMOUSE")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->otherObject[3].src[sk->otherObject[3].srcSize], &csv, sk);
                if (sk->otherObject[3].src[sk->otherObject[3].srcSize].graphcount < 1 || sk->otherObject[3].src[sk->otherObject[3].srcSize].count < 1) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n画像の登?に失敗しました。\n", line, fBuf);
                }
                if (sk->otherObject[3].srcSize > 0 && (sk->otherObject[3].dst[sk->otherObject[3].srcSize - 1].dstCount < 1 || sk->otherObject[3].dst[sk->otherObject[3].srcSize - 1].dataSize < 1)) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n(この行のエラ?ではありません)ひとつ前の#SRC_BGAに対応した#DST_BGAが存在しないか、登?に失敗したようです\n", line, fBuf);
                }
                sk->otherObject[3].srcSize++;
            }
            else if (fBuf.left(12).isSame("#DST_ONMOUSE") && sk->otherObject[3].srcSize > 0) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->otherObject[3].dst[sk->otherObject[3].srcSize - 1], &csv, tSkin_num);
            }
            else if (fBuf.left(8).isSame("#SRC_BGA")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->otherObject[4].src[sk->otherObject[4].srcSize], &csv, sk);
                if (sk->otherObject[4].src[sk->otherObject[4].srcSize].graphcount < 1 || sk->otherObject[4].src[sk->otherObject[4].srcSize].count < 1) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n画像の登?に失敗しました。\n", line, fBuf);
                }
                if (sk->otherObject[4].srcSize > 0 && (sk->otherObject[4].dst[sk->otherObject[4].srcSize - 1].dstCount < 1 || sk->otherObject[4].dst[sk->otherObject[4].srcSize - 1].dataSize < 1)) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n(この行のエラ?ではありません)ひとつ前の#SRC_BGAに対応した#DST_BGAが存在しないか、登?に失敗したようです\n", line, fBuf);
                }
                sk->otherObject[4].srcSize++;
            }
            else if (fBuf.left(8).isSame("#DST_BGA") && sk->otherObject[4].srcSize > 0) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->otherObject[4].dst[sk->otherObject[4].srcSize - 1], &csv, tSkin_num);
                tSkin_num++;
            }
            else if (fBuf.left(11).isSame("#SRC_NUMBER")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->otherObject[6].src[sk->otherObject[6].srcSize], &csv, sk);
                if (sk->otherObject[6].src[sk->otherObject[6].srcSize].graphcount < 1 || sk->otherObject[6].src[sk->otherObject[6].srcSize].count < 1) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n画像の登?に失敗しました。\n", line, fBuf);
                }
                if (sk->otherObject[6].srcSize > 0 && (sk->otherObject[6].dst[sk->otherObject[6].srcSize - 1].dstCount < 1 || sk->otherObject[6].dst[sk->otherObject[6].srcSize - 1].dataSize < 1)) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n(この行のエラ?ではあ りません)ひとつ前の#SRC_NUMBERに対応した#DST_NUMBERが 存在しないか、登?に失敗したようです\n", line, fBuf);
                }
                sk->otherObject[6].srcSize++;
            }
            else if (fBuf.left(11).isSame("#DST_NUMBER") && sk->otherObject[6].srcSize > 0) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->otherObject[6].dst[sk->otherObject[6].srcSize - 1], &csv, tSkin_num);
            }
            else if (fBuf.left(9).isSame("#SRC_MASK")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->otherObject[7].src[sk->otherObject[7].srcSize], &csv, sk);
                sk->otherObject[7].srcSize++;
            }
            else if (fBuf.left(9).isSame("#DST_MASK") && sk->otherObject[7].srcSize > 0) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->otherObject[7].dst[sk->otherObject[7].srcSize - 1], &csv, tSkin_num);
            }
            else if (fBuf.left(13).isSame("#SRC_BARGRAPH")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->otherObject[5].src[sk->otherObject[5].srcSize], &csv, sk);
                if (sk->otherObject[5].src[sk->otherObject[5].srcSize].graphcount < 1 || sk->otherObject[5].src[sk->otherObject[5].srcSize].count < 1) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n画像の登?に失敗しまし た。\n", line, fBuf);
                }
                if (sk->otherObject[5].srcSize > 0 && (sk->otherObject[5].dst[sk->otherObject[5].srcSize - 1].dstCount < 1 || sk->otherObject[5].dst[sk->otherObject[5].srcSize - 1].dataSize < 1)) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n(この行のエラ?ではあ りません)ひとつ前の#SRC_BARGRAPHに対応した#DST_BARGRAP Hが存在しないか、登?に失敗したようです\n", line, fBuf);
                }
                sk->otherObject[5].srcSize++;
            }
            else if (fBuf.left(13).isSame("#DST_BARGRAPH") && sk->otherObject[5].srcSize > 0) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->otherObject[5].dst[sk->otherObject[5].srcSize - 1], &csv, tSkin_num);
                tSkin_num++;
            }
            else if (fBuf.left(13).isSame("#SRC_BAR_BODY")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 29, line, read.line.outstr())) {
                    ReadSRC(&sk->src_BAR_BODY[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(17).isSame("#DST_BAR_BODY_OFF")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 29, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_BODY_OFF[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(16).isSame("#DST_BAR_BODY_ON")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 29, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_BODY_ON[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(11).isSame("#BAR_CENTER")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 29, line, read.line.outstr())) {
                    SplitCSV(fBuf, &csv, ",");
                    sk->BAR_CENTER = csv.val[1];
                }
            }
            else if (fBuf.left(14).isSame("#SRC_BAR_TITLE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 4, line, read.line.outstr())) {
                    ReadSRC_BAR_TITLE(&sk->src_BAR_TITLE[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(14).isSame("#DST_BAR_TITLE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 4, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_TITLE[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(11).isSame("#TRANSCOLOR")) {
                SplitCSV(fBuf, &csv, ",");
                SetTransColor(csv.val[1], csv.val[2], csv.val[3]);
            }
            else if (fBuf.left(11).isSame("#TRANSCLOLR")) {
                SplitCSV(fBuf, &csv, ",");
                SetTransColor(csv.val[1], csv.val[2], csv.val[3]);
            }
            else if (fBuf.left(12).isSame("#TRANSCLOLOR")) {
                SplitCSV(fBuf, &csv, ",");
                SetTransColor(csv.val[1], csv.val[2], csv.val[3]);
            }
            else if (fBuf.left(11).isSame("#STARTINPUT")) {
                SplitCSV(fBuf, &csv, ",");
                sk->startinput_start = csv.val[1];
                sk->startinput_rank = csv.val[2];
                sk->startinput_update = csv.val[3];
            }
            else if (fBuf.left(10).isSame("#SCENETIME")) {
                SplitCSV(fBuf, &csv, ",");
                sk->scenetime = csv.val[1];
            }
            else if (fBuf.left(8).isSame("#FADEOUT")) {
                SplitCSV(fBuf, &csv, ",");
                sk->fadeout = csv.val[1];
            }
            else if (fBuf.left(6).isSame("#CLOSE")) {
                SplitCSV(fBuf, &csv, ",");
                sk->close = csv.val[1];
            }
            else if (fBuf.left(5).isSame("#SKIP")) {
                SplitCSV(fBuf, &csv, ",");
                sk->close = csv.val[1];
            }
            else if (fBuf.left(10).isSame("#PLAYSTART")) {
                SplitCSV(fBuf, &csv, ",");
                sk->playstart = csv.val[1];
            }
            else if (fBuf.left(10).isSame("#LOADSTART")) {
                SplitCSV(fBuf, &csv, ",");
                sk->loadstart = csv.val[1];
            }
            else if (fBuf.left(8).isSame("#LOADEND")) {
                SplitCSV(fBuf, &csv, ",");
                sk->loadend = csv.val[1];
            }
            else if (fBuf.left(14).isSame("#BAR_AVAILABLE")) {
                SplitCSV(fBuf, &csv, ",");
                sk->bar_availabe_from = csv.val[1];
                sk->bar_availabe_to = csv.val[2];
            }
            else if (fBuf.left(14).isSame("#SRC_BAR_FLASH")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->src_BAR_FLASH, &csv, sk);
            }
            else if (fBuf.left(14).isSame("#DST_BAR_FLASH")) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->dst_BAR_FLASH, &csv, tSkin_num);
            }
            else if (fBuf.left(18).isSame("#DST_BAR_STAGEFILE")) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->dst_BAR_STAGEFILE, &csv, tSkin_num);
            }
            else if (fBuf.left(16).isSame("#SRC_MOUSECURSOR")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->src_MOUSECURSOR, &csv, sk);
            }
            else if (fBuf.left(16).isSame("#DST_MOUSECURSOR")) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->dst_MOUSECURSOR, &csv, tSkin_num);
            }
            else if (fBuf.left(14).isSame("#SRC_BAR_LEVEL")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 10, line, read.line.outstr())) {
                    ReadSRC(&sk->src_BAR_LEVEL[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(14).isSame("#DST_BAR_LEVEL")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 10, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_LEVEL[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(9).isSame("#SRC_NOTE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_NOTE[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(9).isSame("#SRC_MINE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_MINE[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(13).isSame("#SRC_LN_START")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_LN_START[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(11).isSame("#SRC_LN_END")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_LN_END[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(12).isSame("#SRC_LN_BODY")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_LN_BODY[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(14).isSame("#SRC_AUTO_NOTE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_AUTO_NOTE[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(14).isSame("#SRC_AUTO_MINE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_AUTO_MINE[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(18).isSame("#SRC_AUTO_LN_START")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_AUTO_LN_START[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(16).isSame("#SRC_AUTO_LN_END")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_AUTO_LN_END[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(17).isSame("#SRC_AUTO_LN_BODY")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadSRC(&sk->src_AUTO_LN_BODY[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(9).isSame("#DST_NOTE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 19, line, read.line.outstr())) {
                    ReadDST(&sk->dst_NOTE[csv.val[1]], &csv, tSkin_num);
                    tSkin_num += 2;
                }
            }
            else if (fBuf.left(16).isSame("#SRC_NOWJUDGE_1P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 5, line, read.line.outstr())) {
                    ReadSRC(&sk->src_NOWJUDGE_1P[csv.val[1]], &csv, sk);
                    sk->src_NOWJUDGE_1P[csv.val[1]].timer = 46;
                }
            }
            else if (fBuf.left(16).isSame("#DST_NOWJUDGE_1P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 5, line, read.line.outstr())) {
                    ReadDST(&sk->dst_NOWJUDGE_1P[csv.val[1]], &csv, tSkin_num);
                    sk->dst_NOWJUDGE_1P[csv.val[1]].timer = 46;
                }
            }
            else if (fBuf.left(16).isSame("#SRC_NOWCOMBO_1P")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->src_NOWCOMBO_1P[csv.val[1]], &csv, sk);
                sk->src_NOWCOMBO_1P[csv.val[1]].timer = 46;
            }
            else if (fBuf.left(16).isSame("#DST_NOWCOMBO_1P")) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->dst_NOWCOMBO_1P[csv.val[1]], &csv, tSkin_num);
                sk->dst_NOWJUDGE_1P[csv.val[1]].timer = 46; //???mistake?
            }
            else if (fBuf.left(16).isSame("#SRC_NOWJUDGE_2P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 5, line, read.line.outstr())) {
                    ReadSRC(&sk->src_NOWJUDGE_2P[csv.val[1]], &csv, sk);
                    sk->src_NOWJUDGE_2P[csv.val[1]].timer = 47;
                }
            }
            else if (fBuf.left(16).isSame("#DST_NOWJUDGE_2P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 5, line, read.line.outstr())) {
                    ReadDST(&sk->dst_NOWJUDGE_2P[csv.val[1]], &csv, tSkin_num);
                    sk->dst_NOWJUDGE_2P[csv.val[1]].timer = 47;
                }
            }
            else if (fBuf.left(16).isSame("#SRC_NOWCOMBO_2P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 5, line, read.line.outstr())) {
                    ReadSRC(&sk->src_NOWCOMBO_2P[csv.val[1]], &csv, sk);
                    sk->src_NOWCOMBO_2P[csv.val[1]].timer = 47;
                }
            }
            else if (fBuf.left(16).isSame("#DST_NOWCOMBO_2P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 5, line, read.line.outstr())) {
                    ReadDST(&sk->dst_NOWCOMBO_2P[csv.val[1]], &csv, tSkin_num);
                    sk->dst_NOWJUDGE_2P[csv.val[1]].timer = 47; //???mistake?
                }
            }
            else if (fBuf.left(16).isSame("#SRC_GROOVEGAUGE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadSRC(&sk->src_GROOVEGAUGE[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(16).isSame("#DST_GROOVEGAUGE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadDST(&sk->dst_GROOVEGAUGE[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(18).isSame("#SRC_GAUGECHART_1P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadSRC(&sk->src_GAUGECHART_1P[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(18).isSame("#DST_GAUGECHART_1P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadDST(&sk->dst_GAUGECHART_1P[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(18).isSame("#SRC_GAUGECHART_2P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadSRC(&sk->src_GAUGECHART_2P[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(18).isSame("#DST_GAUGECHART_2P")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadDST(&sk->dst_GAUGECHART_2P[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(15).isSame("#SRC_SCORECHART")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 2, line, read.line.outstr())) {
                    ReadSRC(&sk->src_SCORECHART[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(15).isSame("#DST_SCORECHART")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 2, line, read.line.outstr())) {
                    ReadDST(&sk->dst_SCORECHART[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(9).isSame("#SRC_LINE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadSRC(&sk->src_LINE[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(9).isSame("#DST_LINE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadDST(&sk->dst_LINE[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(14).isSame("#SRC_JUDGELINE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadSRC(&sk->src_JUDGELINE[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(14).isSame("#DST_JUDGELINE")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadDST(&sk->dst_JUDGELINE[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(13).isSame("#SRC_BAR_LAMP")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadSRC(&sk->src_BAR_LAMP[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(13).isSame("#DST_BAR_LAMP")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_LAMP[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(16).isSame("#SRC_BAR_MY_LAMP")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadSRC(&sk->src_BAR_MY_LAMP[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(16).isSame("#DST_BAR_MY_LAMP")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_MY_LAMP[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(19).isSame("#SRC_BAR_RIVAL_LAMP")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadSRC(&sk->src_BAR_RIVAL_LAMP[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(19).isSame("#DST_BAR_RIVAL_LAMP")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_RIVAL_LAMP[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(13).isSame("#SRC_BAR_STAR")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 5, line, read.line.outstr())) {
                    ReadSRC(&sk->src_BAR_STAR[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(13).isSame("#DST_BAR_STAR")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 5, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_STAR[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(14).isSame("#SRC_THUMBNAIL")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->src_THUMBNAIL, &csv, sk);
            }
            else if (fBuf.left(14).isSame("#DST_THUMBNAIL")) {
                SplitCSV(fBuf, &csv, ",");
                ReadDST(&sk->dst_THUMBNAIL, &csv, tSkin_num);
            }
            else if (fBuf.left(11).isSame("#SRC_README")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadSRC_BAR_TITLE(&sk->src_README[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(11).isSame("#DST_README")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 1, line, read.line.outstr())) {
                    ReadDST(&sk->dst_README[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(20).isSame("#DST_EVENT_LOADINGBG")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 4, line, read.line.outstr())) {
                    ReadDST(&sk->dst_EVENT_LOADINGBG[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(22).isSame("#SRC_EVENT_MODE_CURSOR")) {
                SplitCSV(fBuf, &csv, ",");
                ReadSRC(&sk->src_EVENT_MODE_CURSOR, &csv, sk);
            }
            else if (fBuf.left(25).isSame("#DST_EVENT_MODE_CURSOR_ON")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 10, line, read.line.outstr())) {
                    ReadDST(&sk->dst_EVENT_MODE_CURSOR_ON[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(26).isSame("#DST_EVENT_MODE_CURSOR_OFF")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 10, line, read.line.outstr())) {
                    ReadDST(&sk->dst_EVENT_MODE_CURSOR_OFF[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(17).isSame("#EVENT_STARTINPUT")) {
                SplitCSV(fBuf, &csv, ",");
                sk->event_STARTINPUT[0] = csv.val[1];
                sk->event_STARTINPUT[1] = csv.val[2];
                sk->event_STARTINPUT[2] = csv.val[3];
                sk->event_STARTINPUT[3] = csv.val[4];
                sk->event_STARTINPUT[4] = csv.val[5];
                sk->event_STARTINPUT[5] = csv.val[6];
                sk->event_STARTINPUT[6] = csv.val[7];
                sk->event_STARTINPUT[7] = csv.val[8];
                sk->event_STARTINPUT[8] = csv.val[9];
                sk->event_STARTINPUT[9] = csv.val[10];
            }
            else if (fBuf.left(14).isSame("#EVENT_FADEOUT")) {
                SplitCSV(fBuf, &csv, ",");
                sk->event_FADEOUT[0] = csv.val[1];
                sk->event_FADEOUT[1] = csv.val[2];
                sk->event_FADEOUT[2] = csv.val[3];
                sk->event_FADEOUT[3] = csv.val[4];
                sk->event_FADEOUT[4] = csv.val[5];
                sk->event_FADEOUT[5] = csv.val[6];
                sk->event_FADEOUT[6] = csv.val[7];
                sk->event_FADEOUT[7] = csv.val[8];
                sk->event_FADEOUT[8] = csv.val[9];
                sk->event_FADEOUT[9] = csv.val[10];
            }
            else if (fBuf.left(8).isSame("#LR2FONT") && !flag_skipFont) {
                SplitCSV(fBuf, &csv, ",");
                if (sk->num_of_ImageFont == 10) {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\nこれ以上の登?はできま せん。\n", line, fBuf);
                }
                else if (csv.val[2] == 1 || sk->disableimagefont == 0) {
                    if (csv.str[1].isDiff("CONTINUE")) {
                        for (int i = 0; i < sk->customfile_count; i++) {
                            if (sk->customfileRANDOM[i].isSame(csv.str[1].left(sk->customfileRANDOM[i].length())) && sk->customfile[i].isDiff("RANDOM") && sk->customfile[i].isDiff("ERROR") && sk->customfile[i].length() > 0) {
                                csv.str[1].replace("*", sk->customfile[i]);
                                //line?
                                break;
                            }
                        }
                        ReadImageFont(GetRandomFileNoError(csv.str[1], dir), &sk->ImageFonts[sk->num_of_ImageFont]);
                    }
                    sk->num_of_ImageFont++;
                }
                else {
                    InitImageFont(&sk->ImageFonts[sk->num_of_ImageFont]);
                    sk->num_of_ImageFont++;
                }
            }
            else if (fBuf.left(9).isSame("#HELPFILE")) {
                SplitCSV(fBuf, &csv, ",");
                if (sk->helpfileCount < 10) {
                    sk->helpfilePath[sk->helpfileCount].assign(&csv.str[1]);
                    sk->helpfileCount = sk->helpfileCount + 1;
                }
            }
            else if (fBuf.left(6).isSame("#NOBGA")) {
                sk->flag_BGA = 0;
            }
            else if (fBuf.left(11).isSame("#FLIPRESULT")) {
                sk->flag_flip = true;
                sk->op[350] = false;
                sk->op[351] = true;
            }
            else if (fBuf.left(9).isSame("#FLIPSIDE")) {
                flipside = true;
            }
            else if (fBuf.left(12).isSame("#DISABLEFLIP")) {
                sk->flag_flip = false;
                sk->op[350] = true;
                sk->op[351] = false;
            }
            else if (fBuf.left(11).isSame("#TEXTMERGIN")) {
                SplitCSV(fBuf, &csv, ",");
                sk->textmergin_1 = csv.val[1];
                sk->textmergin_2 = csv.val[2];
            }
            else if (fBuf.left(12).isSame("#SCRATCHSIDE")) {
                SplitCSV(fBuf, &csv, ",");
                sk->scratchside_1 = csv.val[1];
                sk->scratchside_2 = csv.val[2];
            }
            else if (fBuf.left(8).isSame("#INCLUDE")) {
                /*SplitCSV(fBuf, &csv, ",");
                for (int i = 0; i < sk->customfile_count; i++) {
                    if (sk->customfileRANDOM[i].isSame(csv.str[1].left(sk->customfileRANDOM[i].length()))
                        && sk->customfile[i].isDiff("RANDOM") && sk->customfile[i].isDiff("ERROR")
                        && (sk->customfile[i].length() > 0)) {

                        csv.str[1].replace("*", sk->customfile[i]);
                        break;
                    }
                }
                if (tSkin_num == 0) tSkin_num = 1;
                tSkin_num += ReadSkinSE(sk, GetRandomFileNoError(csv.str[1], dir), unused, tSkin_num, sku, flag_skipFont);*/
            }
            else if (fBuf.left(13).isSame("#CUSTOMOPTION")) {
                sk->customfile_count++;
            }
            else if (fBuf.left(11).isSame("#CUSTOMFILE")) {
                SplitCSV(fBuf, &csv, ",");
                char siblingCustom[MAX_PATH] = {};
                if (ResolveSiblingPlayPath(csv.str[2].outstr(), mainpath,
                    siblingCustom, sizeof(siblingCustom)))
                    csv.str[2].assign(siblingCustom);
                sk->customfileRANDOM[sk->customfile_count].assign(&csv.str[2]);
                sk->customfile[sk->customfile_count].assign("RANDOM");
                if (sk->customfile[sk->customfile_count].isSame("RANDOM")) {
                    sk->customfile[sk->customfile_count].assign(GetRandomFile(sk->customfileRANDOM[sk->customfile_count], 1));
                }
                sk->customfile_count++;
            }
            else if (fBuf.left(13).isSame("#CUSTOMFOLDER")) {
                SplitCSV(fBuf, &csv, ",");
                sk->customfileRANDOM[sk->customfile_count].assign(&csv.str[2]);
                sk->customfile[sk->customfile_count].assign("RANDOM");
                sk->customfile_count++;
            }
            else if (fBuf.left(13).isSame("#RELOADBANNER")) {
                sk->reloadbanner = 1;
            }
            else if (fBuf.left(10).isSame("#SETOPTION")) {
                SplitCSV(fBuf, &csv, ",");
                if (csv.val[1] < 1000) {
                    sk->op[csv.val[1]] = (csv.val[2] != 0);
                }
                else {
                    ErrorLogFmtAdd("スキン読み込みエラ? %d行目\n%s\n#SETOPTIONの第一引数(オプション値)は900?999の範囲内にして下さい。\n", line, fBuf);
                }
            }
            else if (fBuf.left(13).isSame("#SRC_BAR_RANK")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadSRC(&sk->src_BAR_RANK[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(13).isSame("#DST_BAR_RANK")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_RANK[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(14).isSame("#SRC_BAR_RIVAL")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadSRC(&sk->src_BAR_RIVAL[csv.val[1]], &csv, sk);
                }
            }
            else if (fBuf.left(14).isSame("#DST_BAR_RIVAL")) {
                SplitCSV(fBuf, &csv, ",");
                if (CheckIndexRange(csv.val[1], 0, 9, line, read.line.outstr())) {
                    ReadDST(&sk->dst_BAR_RIVAL[csv.val[1]], &csv, tSkin_num);
                }
            }
            else if (fBuf.left(11).isSame("#HORIZONTAL")) {
                sk->horizontal = 1;
            }
        }
        tSkin_num++;
    }

    if(flipside)ApplyFlipside(sk);

    return tSkin_num;
}

//deprecated
//int WORKSPACE::MakeObjects() {
//
//    for (int i = 0; i < arr_DST.count; i++) {
//        SEOBJ* seobj = (SEOBJ*)arr_seobj.Get_new();
//        seobj->ID = i;
//        seobj->dst = i;
//        seobj->src = ((DST*)arr_DST.data)[i].src;
//
//        seobj->name.assign("");
//        if (((DST*)arr_DST.data)[i].op1 >= 0) {
//            seobj->name.add(dstName(((DST*)arr_DST.data)[i].op1));
//        }
//        else {
//            seobj->name.add("Not_");
//            seobj->name.add(dstName(-((DST*)arr_DST.data)[i].op1));
//        }
//
//        seobj->ifGroup = ((SRC*)arr_SRC.data)[seobj->src].ifGroup;
//        if (seobj->ifGroup) {
//            seobj->name.add("_");
//        }
//
//        seobj->igType;
//        seobj->igID;
//        
//    }
//    
//    
//
//    return 0;
//}

int WORKSPACE::SeInit() {
    zoom = 1.0f;
}
int WORKSPACE::SeLoadInit() {

}


int WORKSPACE::drawCustomize() {

    char title[260];
    snprintf(title, sizeof(title), "Customize##%d", num);
    ImGui::Begin(title, &wCustomize);

    for (int i = 0; i < meta.custom_count; i++) {
        SkinCustom& cu = meta.customs[i];

        const std::string customTitleUtf8 = Cp932ToUtf8(cu.title.outstr());
        ImGui::Text("%s", customTitleUtf8.c_str());
        ImGui::SameLine();
        
        char item[256];
        std::string selectedLabelUtf8 = Cp932ToUtf8(cu.op_label[cu.dst_op_selected].outstr());

        if(cu.dst_op_start)
            snprintf(item, sizeof(item), "%03d : %s", cu.dst_op_start + cu.dst_op_selected, selectedLabelUtf8.c_str());
        else
            snprintf(item, sizeof(item), "FILE : %s", selectedLabelUtf8.c_str());

        char label[32];
        snprintf(label, sizeof(label), "##%d", i);
        if (ImGui::BeginCombo(label, item, ImGuiComboFlags_WidthFitPreview))
        {
            for (int n = 0; n < cu.dst_op_count; n++)
            {
                const std::string optionLabelUtf8 = Cp932ToUtf8(cu.op_label[n].outstr());
                if (cu.dst_op_start)
                    snprintf(item, sizeof(item), "%03d : %s", cu.dst_op_start + n, optionLabelUtf8.c_str());
                else
                    snprintf(item, sizeof(item), "FILE : %s", optionLabelUtf8.c_str());

                const bool is_selected = (cu.dst_op_selected == n);
                if (ImGui::Selectable(item, is_selected))
                    cu.dst_op_selected = n;

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }
    
    ImGui::End();
    return 0;
}

int WORKSPACE::drawPreview() {
    // Preview texture is owned by this workspace and recreated whenever
    // the loaded skin resolution changes.
    
    char title[260];
    snprintf(title, sizeof(title), "Preview##%d", num);

    static bool playing = false;

    // An inactive dock tab returns false here. Do not run LR2 rendering or
    // transfer a full preview texture while the user cannot see it.
    if (!ImGui::Begin(title, &wPreview, ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::End();
        return 0;
    }

    const unsigned long long previewNow = GetTickCount64();
    bool previewFrameUpdated = false;

    // Object Editor changes first update the in-memory CSV. Rebuild the LR2
    // preview objects shortly after the last edit so coordinates and all DST
    // properties become visible without saving and reopening the skin.
    if (previewReloadPending && previewNow - previewReloadRequestedAt >= 80) {
        LoadSceneSE();
        LR2SEResetRenderFault();
        previewReloadPending = false;
        previewLastRenderAt = 0;
    }

    // Keep editing and live scene playback at the same responsive refresh
    // rate. Image candidates are now loaded lazily, so the former 15/30 FPS
    // memory-pressure workaround is no longer needed.
    const unsigned long long previewFrameInterval = 16;
    if (previewLastRenderAt == 0 || previewNow - previewLastRenderAt >= previewFrameInterval) {
        // SELECT still needs its editor-side bar/BGA placeholders while its
        // timers are running; the original selector loop is not driven here.
        const bool staticSpecialPreview = !playing || meta.type == SKINTYPE_SELECT;
        if (LR2SEDrawLoopSafe(&g, previewScreen, skinSizeX, skinSizeY, staticSpecialPreview) == 0) {
            previewFrameUpdated = true;
            previewTextureDirty = true;
        }
        previewLastRenderAt = previewNow;
    }

    float zoomPercent = 100.0f / zoom;
    if (ImGui::SliderFloat("zoom##zoom", &zoomPercent, 25.0f, 400.0f,
        "%.0f%%", ImGuiSliderFlags_Logarithmic)) {
        if (zoomPercent < 1.0f) zoomPercent = 1.0f;
        zoom = 100.0f / zoomPercent;
    }
    ImVec2 p = ImGui::GetCursorScreenPos();
    //TODO init zoom value
    //TODO support HD skins
    
    void* previewPixels = NULL;
    if (previewTextureDirty || !texture_preview)
        previewPixels = GetImageAddressSoftImage(previewScreen);
    const size_t previewBytes = (size_t)skinSizeX * (size_t)skinSizeY * 4;
    if (texture_preview &&
        (texture_preview_width != skinSizeX || texture_preview_height != skinSizeY)) {
        texture_preview->Release();
        texture_preview = NULL;
    }
    if (!texture_preview && previewPixels) {
        if (LoadTextureFromRawMemory(previewPixels, previewBytes,
            skinSizeX, skinSizeY, &texture_preview)) {
            texture_preview_width = skinSizeX;
            texture_preview_height = skinSizeY;
            previewTextureDirty = false;
        }
    }
    else if (previewPixels && previewFrameUpdated) {
        RefreshTextureByRawMemory(previewPixels, previewBytes,
            skinSizeX, skinSizeY, &texture_preview);
        previewTextureDirty = false;
    }

    // An Image does not claim mouse input, so dragging an Object over it can
    // also start ImGui's window move behavior. Use a transparent button as the
    // preview canvas so the canvas owns the drag from its first frame.
    const ImVec2 previewCanvasSize((float)skinSizeX / zoom, (float)skinSizeY / zoom);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 0, 0, 0));
    ImGui::ImageButton("##preview_canvas", texture_preview, previewCanvasSize,
        ImVec2(0, 0), ImVec2(1, 1));
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar();

    if (ImGui::Button("MainStart")) {
        LoadSceneSE();
        playing = (LR2SESceneInitSafe(&g, meta.type) == 0);
    }
    if (playing && LR2SESceneProcSafe(&g, meta.type) == -1)
        playing = false;

    if (ImGui::BeginCombo("##Timer",timerName(timerSelected))) {
        for (int timer = 0; timer < 200; timer++) {
            ImGui::PushID(timer);
            const bool is_selected = (timerSelected == timer);
            char timerNameBuf[64];
            sprintf(timerNameBuf, "%03d:%s", timer, timerName(timer));
            if (ImGui::Selectable(timerNameBuf, is_selected))
                timerSelected = timer;

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
            

            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Start")) {
        SetTimeLapse(timerSelected, &g.timer1);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        ResetTimeLapse(timerSelected, &g.timer1);
    }

    

    //D_IDirect3DSurface9* d9 = (D_IDirect3DSurface9*)GetUseDirect3D9BackBufferSurface();
    //d9->GetDC()

    // Drag the highlighted Object. All DST animation rows receive the same
    // delta, preserving animation while EditValue records CSV and History.
    if (preview_selected_obj_valid) {
        const float previewScale = 1.0f / zoom;
        float hitX1 = preview_selected_obj.x;
        float hitY1 = preview_selected_obj.y;
        float hitX2 = preview_selected_obj.x + preview_selected_obj.w;
        float hitY2 = preview_selected_obj.y + preview_selected_obj.h;
        if (hitX1 > hitX2) std::swap(hitX1, hitX2);
        if (hitY1 > hitY2) std::swap(hitY1, hitY2);
        // Keep at least a 20x20 screen-space grab area. Very small or thin
        // LR2 objects are otherwise nearly impossible to catch with a mouse.
        const float grabPadding = 10.0f;
        const ImVec2 hitMin(p.x + hitX1 * previewScale - grabPadding,
            p.y + hitY1 * previewScale - grabPadding);
        const ImVec2 hitMax(p.x + hitX2 * previewScale + grabPadding,
            p.y + hitY2 * previewScale + grabPadding);
        const ImVec2 imageMax(p.x + skinSizeX * previewScale,
            p.y + skinSizeY * previewScale);
        const ImVec2 mousePos = ImGui::GetIO().MousePos;
        const bool overPreviewImage = mousePos.x >= p.x && mousePos.x <= imageMax.x &&
            mousePos.y >= p.y && mousePos.y <= imageMax.y;
        const bool overSelectedObject = overPreviewImage &&
            mousePos.x >= hitMin.x && mousePos.x <= hitMax.x &&
            mousePos.y >= hitMin.y && mousePos.y <= hitMax.y;

        if (overSelectedObject || preview_object_dragging)
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
        if (overSelectedObject && !preview_object_dragging)
            ImGui::SetTooltip("Drag to move Object");

        // Read MouseDown directly. The preview Image may already own the
        // frame's click, so relying on IsMouseClicked can miss the drag start.
        if (!preview_object_dragging && overSelectedObject && ImGui::GetIO().MouseDown[ImGuiMouseButton_Left]) {
            preview_object_dragging = true;
            preview_drag_mouse_start = mousePos;
            preview_drag_object_start_x = preview_selected_obj.x;
            preview_drag_object_start_y = preview_selected_obj.y;
            preview_drag_last_start_x = preview_selected_obj_last.x;
            preview_drag_last_start_y = preview_selected_obj_last.y;
        }

        if (preview_object_dragging) {
            const ImVec2 mouseNow = ImGui::GetIO().MousePos;
            const float deltaX = (mouseNow.x - preview_drag_mouse_start.x) * zoom;
            const float deltaY = (mouseNow.y - preview_drag_mouse_start.y) * zoom;
            preview_selected_obj.x = preview_drag_object_start_x + deltaX;
            preview_selected_obj.y = preview_drag_object_start_y + deltaY;
            if (preview_selected_obj_last_valid) {
                preview_selected_obj_last.x = preview_drag_last_start_x + deltaX;
                preview_selected_obj_last.y = preview_drag_last_start_y + deltaY;
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
                const int moveX = (int)std::round(deltaX);
                const int moveY = (int)std::round(deltaY);
                if ((moveX != 0 || moveY != 0) && !preview_selected_object_model_indices.empty()) {
                    for (int selectedModelIndex : preview_selected_object_model_indices) {
                    if (selectedModelIndex < 0 ||
                        selectedModelIndex >= (int)g_seObjectEditorModel.Objects().size()) continue;
                    const SEObjectInstance& selectedObject = g_seObjectEditorModel.Objects()[selectedModelIndex];
                    for (int rowIndex = 0; rowIndex < (int)selectedObject.rows.size(); ++rowIndex) {
                        const int row = selectedObject.rows[rowIndex];
                        if (row < 0 || row >= skinfileLines.count) continue;
                        SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                        if (!line.csv.str[0].body || strncmp(line.csv.str[0].outstr(), "#DST", 4) != 0) continue;
                        EditValue(row, 3, line.csv.val[3] + moveX);
                        EditValue(row, 4, line.csv.val[4] + moveY);
                    }
                    for (int dstIndex = 0; dstIndex < arr_DST.count; ++dstIndex) {
                        DST& dst = ((DST*)arr_DST.data)[dstIndex];
                        bool belongsToObject = false;
                        for (int rowIndex = 0; rowIndex < (int)selectedObject.rows.size(); ++rowIndex) {
                            if (dst.declare == selectedObject.rows[rowIndex]) { belongsToObject = true; break; }
                        }
                        if (!belongsToObject) continue;
                        for (int frameIndex = 0; frameIndex < dst.arr_animation.count; ++frameIndex) {
                            DST_ANIMATION& frame = ((DST_ANIMATION*)dst.arr_animation.data)[frameIndex];
                            frame.x += moveX;
                            frame.y += moveY;
                        }
                    }
                    }
                }
                preview_selected_obj.x = preview_drag_object_start_x + moveX;
                preview_selected_obj.y = preview_drag_object_start_y + moveY;
                if (preview_selected_obj_last_valid) {
                    preview_selected_obj_last.x = preview_drag_last_start_x + moveX;
                    preview_selected_obj_last.y = preview_drag_last_start_y + moveY;
                }
                preview_object_dragging = false;
            }
        }

        // Fine positioning: move every selected Object by exactly one skin
        // pixel per arrow-key step. Key repeat is enabled for held keys, but
        // text/property editing keeps ownership of the arrows.
        if (!preview_object_dragging && !ImGui::GetIO().WantTextInput && !ImGui::IsAnyItemActive()) {
            int keyMoveX = 0;
            int keyMoveY = 0;
            if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, true)) --keyMoveX;
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, true)) ++keyMoveX;
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, true)) --keyMoveY;
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, true)) ++keyMoveY;
            if ((keyMoveX != 0 || keyMoveY != 0) &&
                !preview_selected_object_model_indices.empty()) {
                for (int selectedModelIndex : preview_selected_object_model_indices) {
                    if (selectedModelIndex < 0 ||
                        selectedModelIndex >= (int)g_seObjectEditorModel.Objects().size()) continue;
                    const SEObjectInstance& selectedObject =
                        g_seObjectEditorModel.Objects()[selectedModelIndex];
                    for (int rowIndex = 0; rowIndex < (int)selectedObject.rows.size(); ++rowIndex) {
                        const int row = selectedObject.rows[rowIndex];
                        if (row < 0 || row >= skinfileLines.count) continue;
                        SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                        if (!line.csv.str[0].body ||
                            strncmp(line.csv.str[0].outstr(), "#DST", 4) != 0) continue;
                        EditValue(row, 3, line.csv.val[3] + keyMoveX);
                        EditValue(row, 4, line.csv.val[4] + keyMoveY);
                    }
                    for (int dstIndex = 0; dstIndex < arr_DST.count; ++dstIndex) {
                        DST& dst = ((DST*)arr_DST.data)[dstIndex];
                        bool belongsToObject = false;
                        for (int rowIndex = 0; rowIndex < (int)selectedObject.rows.size(); ++rowIndex) {
                            if (dst.declare == selectedObject.rows[rowIndex]) {
                                belongsToObject = true;
                                break;
                            }
                        }
                        if (!belongsToObject) continue;
                        for (int frameIndex = 0; frameIndex < dst.arr_animation.count; ++frameIndex) {
                            DST_ANIMATION& frame = ((DST_ANIMATION*)dst.arr_animation.data)[frameIndex];
                            frame.x += keyMoveX;
                            frame.y += keyMoveY;
                        }
                    }
                }
                preview_selected_obj.x += keyMoveX;
                preview_selected_obj.y += keyMoveY;
                if (preview_selected_obj_last_valid) {
                    preview_selected_obj_last.x += keyMoveX;
                    preview_selected_obj_last.y += keyMoveY;
                }
            }
        }
    }
    // Flash the selected Object's destination rectangle over the preview.
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if (preview_selected_obj_valid && ((int)GetTimeLapse(1, &g.timer1) % 500 < 300)) {
        const float previewScale = 1.0f / zoom;
        auto drawDstBounds = [&](const PreviewSelectionBounds& bounds, ImU32 color) {
            float x1 = bounds.x;
            float y1 = bounds.y;
            float x2 = bounds.x + bounds.w;
            float y2 = bounds.y + bounds.h;
            if (x1 > x2) std::swap(x1, x2);
            if (y1 > y2) std::swap(y1, y2);
            ImVec2 dstTopLeft(p.x + x1 * previewScale - 2.0f,
                p.y + y1 * previewScale - 2.0f);
            ImVec2 dstBottomRight(p.x + x2 * previewScale + 2.0f,
                p.y + y2 * previewScale + 2.0f);
            draw_list->AddRect(dstTopLeft, dstBottomRight, color,
                0.0f, ImDrawFlags_Closed, 3.0f);
            draw_list->AddRect(ImVec2(dstTopLeft.x + 2, dstTopLeft.y + 2),
                ImVec2(dstBottomRight.x - 2, dstBottomRight.y - 2),
                IM_COL32(255, 255, 255, 210), 0.0f, ImDrawFlags_Closed, 1.0f);
        };
        // Cyan marks the starting DST frame; red marks the final destination.
        drawDstBounds(preview_selected_obj, IM_COL32(32, 210, 255, 255));
        if (preview_selected_obj_last_valid)
            drawDstBounds(preview_selected_obj_last, IM_COL32(255, 48, 48, 255));
    }

    //test objects on cursor
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        ImGuiIO& io = ImGui::GetIO();
        clickPos = { io.MousePos.x,io.MousePos.y };
        drawRightClick = true;
    }
    else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        drawRightClick = false;
    }
    if(drawRightClick){
        ImGui::PushID(num);
        if (ImGui::BeginPopupContextWindow()) {
            for (int i = 0; i < arr_DST.count; i++) {
                DST& dst = ((DST*)arr_DST.data)[i];
                DST_ANIMATION& dstd = ((DST_ANIMATION*)dst.arr_animation.data)[dst.arr_animation.count - 1];
                ImVec2 dstposLU = { (float)p.x + dstd.x, (float)p.y + dstd.y };
                ImVec2 dstposRB = { (float)p.x + dstd.x + dstd.w, (float)p.y + dstd.y + dstd.h };
                
                if (dstposLU.x <= clickPos.x && clickPos.x <= dstposRB.x && dstposLU.y <= clickPos.y && clickPos.y <= dstposRB.y) {
                    ImGui::Text("%d", i);
                    //printSrcImg(((SRC*)arr_SRC.data)[dst.src],1);
                    ImGui::SameLine();
                    printSrcImgEx(((SRC*)arr_SRC.data)[dst.src], 90, 60);
                }
            }

            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
    

    ImGui::End();

    if (!wPreview) SetWindowVisibleFlag(0);
    return 0;
}

int WORKSPACE::drawTextEdit() {
    char title[260];
    snprintf(title, sizeof(title), "TextEdit##%d", num);

    ImGui::Begin(title, &wTextEdit, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Hide")) {

            ImGui::MenuItem("Blank", NULL, &hideBlank);
            ImGui::MenuItem("Comment", NULL, &hideComment);

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    //vertical scroll & skip hidden //todo only mouse on
    if(ImGui::IsWindowHovered()) {
        ImGuiIO& io = ImGui::GetIO();
        textCursor -= io.MouseWheel;
        if (textCursor < 0) textCursor = 0;
        SKINFILELINEREAD* read1 = &((SKINFILELINEREAD*)skinfileLines.data)[textCursor];
        while ((hideBlank && *read1->line.atPos(0) == '\0') || (hideComment && read1->isComment)) {
            if (io.MouseWheel < 0) textCursor++;
            else textCursor--;

            if (textCursor < 0) {
                textCursor = 0;
                read1 = &((SKINFILELINEREAD*)skinfileLines.data)[textCursor];
                while ((hideBlank && *read1->line.atPos(0) == '\0') || (hideComment && read1->isComment)) {
                    textCursor++;
                    read1 = &((SKINFILELINEREAD*)skinfileLines.data)[textCursor];
                }
            }
            read1 = &((SKINFILELINEREAD*)skinfileLines.data)[textCursor];
        }
        if (textCursor < 0) textCursor = 0;
        if (textCursor >= skinfileLines.count) textCursor = skinfileLines.count - 1;
    }
    
    
    //print every line
    int printed = 0;
    for (int n = textCursor; n < skinfileLines.count && printed < 30; n++) {
        SKINFILELINEREAD& read = ((SKINFILELINEREAD*)skinfileLines.data)[n];

        ImVec4 color;
        color = { 1.0f, 1.0f, 1.0f, 1.0f };

        if ((hideBlank && *read.line.atPos(0) == '\0') || (hideComment && read.isComment)) {
            continue;
        }
        /*char itemname[260];
        snprintf(itemname, sizeof(itemname), "%04d:%04d: %s", read.numTotal, read.num, read.line.outstr());*/

        //ImGui::PushID(n);
        //ImGui::Button(" ");
        ///*if (ImGui::BeginItemTooltip())
        //{
        //    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        //    ImGui::Text("%d", read.ifgroup);
        //    ImGui::EndTooltip();
        //}*/

        //ImGui::PopID();

        //ImGui::SameLine();

        IFUNIT& ifs = ((IFUNIT*)arr_ifunit.data)[read.ifgroup];
        bool head = read.isIfGroupHead || read.isGroupHead;//(read.numTotal == ifs.declare);
        bool isHide = ifs.hide;
        int parIFCursor = ifs.parentID;
        for (int i = 0; i < ifs.depth; i++) {
            isHide |= ((IFUNIT*)arr_ifunit.data)[parIFCursor].hide;
            parIFCursor = ((IFUNIT*)arr_ifunit.data)[parIFCursor].parentID;
        }
        if (isHide && !head) continue;

        ImGui::PushID(n);
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV((head + isHide) / 3.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV((head + isHide) / 3.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV((head + isHide) / 3.0f, 0.8f, 0.8f));
        if (ImGui::Button(" ")) {
            if (head){//read.numTotal == ifs.declare) {
                ifs.hide ^= 1;
            }
        }

        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right), 1) {
            if (ImGui::BeginPopupContextItem()) {
                ImGui::Text("selected : %d", n);
                ImGui::MenuItem("move");
                if (ImGui::MenuItem("insert")) {
                    InsertLine(n);
                }
                ImGui::MenuItem("group");
                if (ImGui::MenuItem("delete")) {
                    DeleteLine(n);
                }
                ImGui::EndPopup();
            }
        }

        ImGui::PopStyleColor(3);
        ImGui::PopID();
        ImGui::SameLine();


        ImGui::Text("%d ", read.ifgroup);
        
        ImGui::SameLine();
        
        if (read.isComment) {
            ImGui::TextDisabled("%04d:%04d: ", read.numTotal, read.num);

            //ImGui::SameLine();
            //ImGui::PushID(n);
            //ImGui::Button(" ");
            //
            ///*if (ImGui::BeginItemTooltip())
            //{
            //    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            //    ImGui::Text("%d", read.ifgroup);
            //    ImGui::EndTooltip();
            //}*/

            //ImGui::PopID();

            ImGui::SameLine();
            //ImGui::TextDisabled("%s", read.line.outstr());
            ImGui::PushID(n);
            CstrInputText("", &read.line, ImGuiInputTextFlags_None);

            static CSTR tmp;
            if (ImGui::IsItemActivated()) {
                tmp.assign(read.line.outstr());
            }
            if (ImGui::IsItemDeactivatedAfterEdit()) {
                HISTORY* hs = (HISTORY*)arr_history.Get_new();
                hs->op = overwriteLine;
                hs->target = n;
                CsvToLine(n);
                hs->older.line.assign(tmp);
                hs->newer.line.assign(read.line.outstr());
            }
            ImGui::PopID();
            //ImGui::TextColored(color, "%04d:%04d: %s", read.numTotal, read.num, read.line.outstr());


        }
        else {
            //ImGui::TextColored(color, "%04d:%04d: %s", read.numTotal, read.num, read.line.outstr());

            for (int depth = 0; depth < ((IFUNIT*)(arr_ifunit.data))[read.ifgroup].depth; depth++) {
                ImGui::TextColored(color, "_");
                ImGui::SameLine();
            }
            ImGui::TextColored(color, "%04d:%04d: ", read.numTotal, read.num);

            ImGui::SameLine();

            char tablename[260];
            snprintf(tablename, sizeof(tablename), "##w%d_text", num);
            if (ImGui::BeginTable(tablename, 22, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoPadInnerX , {4000,18},3999))
            {
                ImGui::PushItemWidth(FLT_MAX);
                ImGui::TableNextRow();

                //color
                ImU32 row_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
                if(read.isSRC)
                    row_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.7f, 0.3f, 0.65f)); 
                else if (read.isDST)
                    row_bg_color = ImGui::GetColorU32(ImVec4(0.7f, 0.3f, 0.3f, 0.65f));
                else if (read.isOther)
                    row_bg_color = ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 0.65f));
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, row_bg_color);

                for (int column = 0; column < 22; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    if (read.csv.str[column].atPos(0) == nullptr) {
                        ImGui::TextDisabled("%s", Cp932ToUtf8(read.csv.str[column].outstr()).c_str());
                    }
                    else {
                        //ImGui::Text("%s", read.csv.str[column]);
                        char inputname[260];
                        sprintf(inputname, "##%d_%d_cell", read.numTotal, column);
                        ImGui::SetNextItemWidth(-FLT_MIN);

                        
                        if (GetCommandHelp(read.csv.str[0].outstr(), column).left(5).isSame("$type")) {
                            if (ImGui::BeginCombo(inputname, SKINTYPESTR[read.csv.val[column]], ImGuiComboFlags_None)) {
                                for (int op = 0; op < 21; op++) {
                                    ImGui::PushID(op);
                                    const bool is_selected = (read.csv.val[column] == op);
                                    char opname[64];
                                    sprintf(opname, "%03d:%s", op, SKINTYPESTR[op]);

                                    if (ImGui::Selectable(opname, is_selected)) {
                                        //read.csv.val[column] = op;
                                        EditValue(n, column, op);
                                    }

                                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                    if (is_selected)
                                        ImGui::SetItemDefaultFocus();

                                    ImGui::PopID();
                                }
                                ImGui::EndCombo();
                            }
                        }
                        else if (GetCommandHelp(read.csv.str[0].outstr(), column).left(3).isSame("$op")) {
                            if (ImGui::BeginCombo(inputname, dstName(read.csv.val[column]), ImGuiComboFlags_None)) {
                                for (int op = 0; op < 1000; op++) {
                                    ImGui::PushID(op);
                                    const bool is_selected = (read.csv.val[column] == op);
                                    char opname[64];
                                    sprintf(opname, "%03d:%s", op, dstName(op));

                                    if (ImGui::Selectable(opname, is_selected)) {
                                        //read.csv.val[column] = op;
                                        EditValue(n, column, op);
                                    }
                                        
                                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                    if (is_selected)
                                        ImGui::SetItemDefaultFocus();

                                    ImGui::PopID();
                                }
                                ImGui::EndCombo();
                            }
                        }
                        else if (GetCommandHelp(read.csv.str[0].outstr(), column).left(3).isSame("$st")) {
                            const std::string currentTextNameUtf8 = Cp932ToUtf8(textName(read.csv.val[column]));
                            if (ImGui::BeginCombo(inputname, currentTextNameUtf8.c_str(), ImGuiComboFlags_None)) {
                                for (int op = 0; op < 200; op++) {
                                    ImGui::PushID(op);
                                    const bool is_selected = (read.csv.val[column] == op);
                                    char opname[64];
                                    const std::string textNameUtf8 = Cp932ToUtf8(textName(op));
                                    snprintf(opname, sizeof(opname), "%03d:%s", op, textNameUtf8.c_str());

                                    if (ImGui::Selectable(opname, is_selected)) {
                                        //read.csv.val[column] = op;
                                        EditValue(n, column, op);
                                    }
                                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                    if (is_selected)
                                        ImGui::SetItemDefaultFocus();

                                    ImGui::PopID();
                                }
                                ImGui::EndCombo();
                            }
                        }
                        else if (GetCommandHelp(read.csv.str[0].outstr(), column).left(4).isSame("$num")) {
                            const std::string currentNumberNameUtf8 = Cp932ToUtf8(numberName(read.csv.val[column]));
                            if (ImGui::BeginCombo(inputname, currentNumberNameUtf8.c_str(), ImGuiComboFlags_None)) {
                                for (int op = 0; op < 300; op++) {
                                    ImGui::PushID(op);
                                    const bool is_selected = (read.csv.val[column] == op);
                                    char opname[64];
                                    const std::string numberNameUtf8 = Cp932ToUtf8(numberName(op));
                                    snprintf(opname, sizeof(opname), "%03d:%s", op, numberNameUtf8.c_str());

                                    if (ImGui::Selectable(opname, is_selected)) {
                                        //read.csv.val[column] = op;
                                        EditValue(n, column, op);
                                    }
                                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                    if (is_selected)
                                        ImGui::SetItemDefaultFocus();

                                    ImGui::PopID();
                                }
                                ImGui::EndCombo();
                            }
                        }
                        else if (GetCommandHelp(read.csv.str[0].outstr(), column).left(6).isSame("$timer")) {
                            const std::string currentTimerNameUtf8 = Cp932ToUtf8(timerName(read.csv.val[column]));
                            if (ImGui::BeginCombo(inputname, currentTimerNameUtf8.c_str(), ImGuiComboFlags_None)) {
                                for (int op = 0; op < 200; op++) {
                                    ImGui::PushID(op);
                                    const bool is_selected = (read.csv.val[column] == op);
                                    char opname[64];
                                    const std::string timerNameUtf8 = Cp932ToUtf8(timerName(op));
                                    snprintf(opname, sizeof(opname), "%03d:%s", op, timerNameUtf8.c_str());

                                    if (ImGui::Selectable(opname, is_selected)) {
                                        //read.csv.val[column] = op;
                                        EditValue(n, column, op);
                                    }
                                    // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                    if (is_selected)
                                        ImGui::SetItemDefaultFocus();

                                    ImGui::PopID();
                                }
                                ImGui::EndCombo();
                            }
                        }
                        else {
                            static CSTR tmp;
                            CstrInputText(inputname, &read.csv.str[column], ImGuiInputTextFlags_AutoSelectAll);
                            //ImGui::InputText(inputname, read.csv.str[column], 260, ImGuiInputTextFlags_AutoSelectAll);
                            if (ImGui::IsItemActivated()) {
                                tmp.assign(read.line.outstr());
                            }
                            if (ImGui::IsItemDeactivatedAfterEdit()) {
                                HISTORY* hs = (HISTORY*)arr_history.Get_new();
                                hs->op = overwriteLine;
                                hs->target = n;
                                CsvToLine(n);
                                hs->older.line.assign(tmp);
                                hs->newer.line.assign(read.line.outstr());
                            }
                        }
                            
                        
                    }

                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone) && ImGui::BeginTooltip())
                    {
                        ImGui::Text("%d", read.objID);;
                        //ImGui::Text("%d",ifs.grCount);
                        ImGui::Text("%s", Cp932ToUtf8(
                            GetCommandHelp(read.csv.str[0].outstr(), column).outstr()).c_str());
                        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                        const std::string cellValueUtf8 = Cp932ToUtf8(read.csv.str[column].outstr());
                        ImGui::TextUnformatted(cellValueUtf8.c_str());
                        if (read.csv.str[0].isSame("#SRC_IMAGE")) {

                            int handle = read.csv.val[2];
                            EnsureSRCGRTexture(handle);
                            SRCGR& img = ((SRCGR*)arr_SRCGR.data)[handle];

                            int iX = read.csv.val[3];
                            int iY = read.csv.val[4];
                            int iW = read.csv.val[5] == -1 ? img.sizeX - iX : read.csv.val[5];
                            int iH = read.csv.val[6] == -1 ? img.sizeY - iY : read.csv.val[6];

                            if (img.texture != NULL) {
                                ImVec2 display_min = ImVec2(iX / (float)img.sizeX, iY / (float)img.sizeY);
                                ImVec2 display_max = ImVec2((iX + iW) / (float)img.sizeX, (iY + iH) / (float)img.sizeY);
                                ImVec2 display_size = ImVec2(iW, iH);

                                ImGui::Image(img.texture, display_size, display_min, display_max);;
                            }
                            //printSrcImg(((SRC*)arr_SRC.data)[handle]);
                        }
                        ImGui::PopTextWrapPos();
                        ImGui::EndTooltip();
                    }
                }
                ImGui::EndTable();
            }
        }
        
        printed++;
        /*if (ImGui::BeginChild(ImGuiChildFlags_FrameStyle))
        {
            for (int n = 0; n < skinfileLines.count; n++){
                SKINFILELINEREAD& read = ((SKINFILELINEREAD*)skinfileLines.data)[n];

                const bool is_selected = 0;
                char itemname[260];
                snprintf(itemname, sizeof(itemname), "%04d:%04d: %s", read.numTotal, read.num, read.line.outstr());
                ImGui::Selectable(itemname, is_selected);
            }
        }
        ImGui::EndChild();*/


    }

    ImGui::End();
    return 0;
}

bool WORKSPACE::EnsureSRCGRTexture(int iSRCGR) {
    if (iSRCGR < 0 || iSRCGR >= arr_SRCGR.count) return false;
    SRCGR& img = ((SRCGR*)arr_SRCGR.data)[iSRCGR];
    if (img.texture != NULL) return true;
    if (img.loaded || img.path.isSame("CONTINUE")) return false;

    // Mark the attempt so missing optional files are not retried every frame.
    img.loaded = true;
    if (LoadTextureFromFile(img.path.outstr(), &img.texture, &img.sizeX, &img.sizeY))
        return true;

    int dxf = DxLib::FileRead_open(img.path);
    if (dxf < 0) return false;
    const int fileSize = FileRead_size(img.path.outstr());
    void* buffer = fileSize > 0 ? malloc((size_t)fileSize) : NULL;
    bool loaded = false;
    if (buffer && DxLib::FileRead_read(buffer, fileSize, dxf) >= 0)
        loaded = LoadTextureFromMemory(buffer, (size_t)fileSize,
            &img.texture, &img.sizeX, &img.sizeY);
    DxLib::FileRead_close(dxf);
    if (buffer) free(buffer);
    return loaded;
}

//deprecated
int WORKSPACE::loadSRC() {

    for (int n = 0; n < arr_SRCGR.count; n++) {
        CSTR& path = ((SRCGR*)arr_SRCGR.data)[n].path;
        SRCGR& img = ((SRCGR*)arr_SRCGR.data)[n];

        bool isLoaded = EnsureSRCGRTexture(n);
    }
    return 0;
}


int WORKSPACE::printSrcImg(SRC src, bool button) {
    EnsureSRCGRTexture(src.gr);
    SRCGR &img = ((SRCGR*)arr_SRCGR.data)[src.gr];
    if (img.texture != NULL) {
        int sizeX = src.sizeX == -1 ? img.sizeX - src.x : src.sizeX;
        int sizeY = src.sizeY == -1 ? img.sizeY - src.y : src.sizeY;
        ImVec2 display_min = ImVec2(src.x / (float)img.sizeX, src.y / (float)img.sizeY);
        ImVec2 display_max = ImVec2((src.x + sizeX) / (float)img.sizeX, (src.y + sizeY) / (float)img.sizeY);
        ImVec2 display_size = ImVec2(sizeX, sizeY);

        if (src.cycle && (src.div_x >= 1 || src.div_y >= 1)) {
            if (src.div_x == 0) src.div_x = 1;
            if (src.div_y == 0) src.div_y = 1;
            ImVec2 chopsize = { src.sizeX / (float)src.div_x , src.sizeY / (float)src.div_y };

            int units = src.div_x * src.div_y;
            int tick = src.cycle / units;
            int ani = ((int)GetTimeLapse(0, &(g.timer1)) % src.cycle) / tick;

            int ax = ani % src.div_x;
            int ay = ani / src.div_x;

            ImVec2 chopstart = { (src.x + chopsize.x * ax) / (float)img.sizeX ,
                                (src.y + chopsize.y * ay) / (float)img.sizeY };

            ImVec2 chopend = { (src.x - 1 + chopsize.x * (ax + 1)) / (float)img.sizeX ,
                                (src.y - 1 + chopsize.y * (ay + 1)) / (float)img.sizeY };

            if (button) ImGui::ImageButton("aaaa", img.texture, { (float)150,(float)100 }, chopstart, chopend);
            else ImGui::Image(img.texture, chopsize, chopstart, chopend);
            
        }
        else {
            if (button) ImGui::ImageButton("aaaa", img.texture, { (float)150,(float)100 }, display_min, display_max);
            else ImGui::Image(img.texture, display_size, display_min, display_max);;
        }
    }
    return 0;
}

int WORKSPACE::printSrcImgButton(SRC src, int num, int w, int h) {
    EnsureSRCGRTexture(src.gr);
    SRCGR& img = ((SRCGR*)arr_SRCGR.data)[src.gr];
    ImGui::PushID(num);
    if (img.texture != NULL) {
        int sizeX = src.sizeX == -1 ? img.sizeX - src.x : src.sizeX;
        int sizeY = src.sizeY == -1 ? img.sizeY - src.y : src.sizeY;
        ImVec2 display_min = ImVec2(src.x / (float)img.sizeX, src.y / (float)img.sizeY);
        ImVec2 display_max = ImVec2((src.x + sizeX) / (float)img.sizeX, (src.y + sizeY) / (float)img.sizeY);
        ImVec2 display_size = ImVec2(sizeX, sizeY);

        if (src.cycle && (src.div_x >= 1 || src.div_y >= 1)) {
            if (src.div_x == 0) src.div_x = 1;
            if (src.div_y == 0) src.div_y = 1;
            ImVec2 chopsize = { src.sizeX / (float)src.div_x , src.sizeY / (float)src.div_y };

            int units = src.div_x * src.div_y;
            int tick = src.cycle / units;
            int ani = ((int)GetTimeLapse(0, &(g.timer1)) % src.cycle) / tick;

            int ax = ani % src.div_x;
            int ay = ani / src.div_x;

            ImVec2 chopstart = { (src.x + chopsize.x * ax) / (float)img.sizeX ,
                                (src.y + chopsize.y * ay) / (float)img.sizeY };

            ImVec2 chopend = { (src.x - 1 + chopsize.x * (ax + 1)) / (float)img.sizeX ,
                                (src.y - 1 + chopsize.y * (ay + 1)) / (float)img.sizeY };

            ImGui::ImageButton("imgbutton", img.texture, { (float)w,(float)h }, chopstart, chopend);
            //ImGui::Image(img.texture, chopsize, chopstart, chopend);

        }
        else {
            ImGui::ImageButton("imgbutton", img.texture, { (float)w,(float)h }, display_min, display_max);
            //ImGui::Image(img.texture, display_size, display_min, display_max);;
        }
    }
    ImGui::PopID();
    return 0;
}

int WORKSPACE::printSrcImgEx(SRC src, int w, int h) {
    EnsureSRCGRTexture(src.gr);
    SRCGR& img = ((SRCGR*)arr_SRCGR.data)[src.gr];
    ImGui::PushID(num);
    if (img.texture != NULL) {
        int sizeX = src.sizeX == -1 ? img.sizeX - src.x : src.sizeX;
        int sizeY = src.sizeY == -1 ? img.sizeY - src.y : src.sizeY;
        ImVec2 display_min = ImVec2(src.x / (float)img.sizeX, src.y / (float)img.sizeY);
        ImVec2 display_max = ImVec2((src.x + sizeX) / (float)img.sizeX, (src.y + sizeY) / (float)img.sizeY);
        ImVec2 display_size = ImVec2(sizeX, sizeY);

        if (src.cycle && (src.div_x >= 1 || src.div_y >= 1)) {
            if (src.div_x == 0) src.div_x = 1;
            if (src.div_y == 0) src.div_y = 1;
            ImVec2 chopsize = { src.sizeX / (float)src.div_x , src.sizeY / (float)src.div_y };

            int units = src.div_x * src.div_y;
            int tick = src.cycle / units;
            int ani = ((int)GetTimeLapse(0, &(g.timer1)) % src.cycle) / tick;

            int ax = ani % src.div_x;
            int ay = ani / src.div_x;

            ImVec2 chopstart = { (src.x + chopsize.x * ax) / (float)img.sizeX ,
                                (src.y + chopsize.y * ay) / (float)img.sizeY };

            ImVec2 chopend = { (src.x - 1 + chopsize.x * (ax + 1)) / (float)img.sizeX ,
                                (src.y - 1 + chopsize.y * (ay + 1)) / (float)img.sizeY };

            ImGui::Image(img.texture, { (float)w,(float)h }, chopstart, chopend);

        }
        else {
            ImGui::Image(img.texture, { (float)w,(float)h }, display_min, display_max);;
        }
    }
    ImGui::PopID();
    return 0;
}

int WORKSPACE::drawImgManager() {
    char title[260];
    snprintf(title, sizeof(title), "ImageManager##%d", num);
    ImGui::Begin(title, &wImgManager, ImGuiWindowFlags_HorizontalScrollbar);
    
    static bool newSquare = 0;
    bool clicked = 0;

    //testing new img list
    ImGui::BeginGroup();
    ImGui::SameLine();
    snprintf(title, sizeof(title), "ImgList##%d", num);
    if (ImGui::BeginChild(title, { 250,-150 }, ImGuiChildFlags_ResizeX | ImGuiChildFlags_FrameStyle)) {

        for (int i = 0; i < arr_IMG.count; i++) {
            IMG& img = ((IMG*)arr_IMG.data)[i];
            char buf[260];
            sprintf(buf, "%03d %02d:%s", i, img.gr, img.name.outstr());
            ImGui::PushID(i);
            if (ImGui::Selectable(buf, i == src_selected)) {
                src_selected = i;
                gr_selected = img.gr;
                grID_selected = img.gr;
            }
            
            ImGui::PopID();
        }
        IMG& img = ((IMG*)arr_IMG.data)[src_selected];
        if (ImGui::BeginPopupContextWindow()) {
            ImGui::Text("selected : %03d", src_selected);
            
            ImGui::Separator();
            if (ImGui::MenuItem("Delete")) {
                DeleteIMG(src_selected);
            }
            ImGui::Separator();
            if(ImGui::MenuItem("New")) {
                //mosue drag
                newSquare = 1;
            }
            ImGui::EndPopup();
        }
        
    }
    ImGui::EndChild();
    //left bottom
    ImGui::Separator();
    snprintf(title, sizeof(title), "ImgEdit##%d", num);
    if (ImGui::BeginChild(title, { 250,150 }, ImGuiChildFlags_ResizeX | ImGuiChildFlags_FrameStyle)) {
        IMG& img = ((IMG*)arr_IMG.data)[src_selected];
        
        char buf[64];
        strncpy(buf, img.name.outstr(), sizeof(buf));
        ImGui::InputText("##name", buf, sizeof(buf));
        img.name.assign(buf);

        ImGui::InputInt("gr", &img.gr);
        ImGui::InputInt("x", &img.x);
        ImGui::InputInt("y", &img.y);
        ImGui::InputInt("w", &img.w);
        ImGui::InputInt("h", &img.h);

        //ModifyIMG(src_selected,, , , , );
    }
    ImGui::EndChild();
    ImGui::EndGroup();
    // test new image view
    ImGui::SameLine(0, 0);

    ImGui::BeginGroup();

    static int gr_type;
    char buf[260] = "temp";
    if (((SRCGR*)arr_SRCGR.data)[gr_selected].grID != grID_selected) {
        for (int i = 0; i < arr_SRCGR.count; i++) {
            SRCGR& gr = ((SRCGR*)arr_SRCGR.data)[i];
            if (gr.grID == grID_selected) {
                gr_selected = i;
                sprintf(buf, "%03d %02d : %s", i, gr.grID, gr.filename.outstr());
            }
        }
    }
    
    if (ImGui::BeginCombo("##grSelect", buf)) {
        for (int i = 0; i < arr_SRCGR.count; i++) {
            SRCGR& gr = ((SRCGR*)arr_SRCGR.data)[i];
            char buf[260];
            sprintf(buf, "%03d %02d : %s", i, gr.grID, gr.filename.outstr());
            if (gr.grID != grID_selected) continue;
            ImGui::PushID(i);
            if (ImGui::Selectable(buf, i == gr_selected)) {
                gr_selected = i;
            }

            ImGui::PopID();
        }
        ImGui::EndCombo();
    }

    EnsureSRCGRTexture(gr_selected);
    SRCGR& img = ((SRCGR*)arr_SRCGR.data)[gr_selected];
    ImGui::Text("%03d_%d ", gr_selected, img.grID);
    
    ImGui::SameLine(0, 0);
    ImGui::Text("%s %d %d", img.path.outstr(), img.sizeX, img.sizeY);
    ImGui::SameLine(0, 0);
    snprintf(title, sizeof(title), "grReload##%d", num);
    ImGui::Button(title);
    ImGui::SameLine(0, 0);
    ImGui::ColorEdit4("MyColor##3", (float*)&bgColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None);

    snprintf(title, sizeof(title), "ImgWorking##%d", num);
    if (ImGui::BeginChild(title, { -1, -1 }, ImGuiChildFlags_Borders | ImGuiChildFlags_FrameStyle, ImGuiWindowFlags_HorizontalScrollbar)) {
        
        const ImVec2 pb = ImGui::GetCursorScreenPos();
        ImGui::Image(transBackground, { (float)img.sizeX, (float)img.sizeY }, { 0,0 }, { img.sizeX / (float)32, img.sizeY / (float)32 });
        ImGui::SetCursorScreenPos(pb);
        if (img.texture)
            ImGui::ImageWithBg(img.texture, { (float)img.sizeX, (float)img.sizeY }, { 0,0 }, { 1, 1 }, bgColor);


        if (img.texture != NULL) {
            const ImVec2 p = ImGui::GetCursorScreenPos();
            ImVec2 grpos = { p.x, p.y - img.sizeY - 4 };

            IMG& src = ((IMG*)arr_IMG.data)[src_selected];
            int sizeX = src.w == -1 ? img.sizeX - src.x : src.w;
            int sizeY = src.h == -1 ? img.sizeY - src.y : src.h;

            ImVec2 srcposLU = { grpos.x + src.x - 1, grpos.y + src.y - 1 };
            ImVec2 srcposRB = { grpos.x + src.x + sizeX + 1, grpos.y + src.y + sizeY + 1 };

            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            bool flicking = ((int)GetTimeLapse(1, &g.timer1) % 200 > 100);
            ImColor color = flicking ? 0xff0080ff : 0x000000;
            draw_list->AddRect(srcposLU, srcposRB, color, 0.0f, ImDrawFlags_Closed, 1.0f);

            if (clicked) {
                ImGui::SetScrollX(src.x);
                ImGui::SetScrollY(src.y);
            }

            for (int i = 0; i < arr_IMG.count; i++) {
                IMG& hoversrc = ((IMG*)arr_IMG.data)[i];
                if (hoversrc.gr != src.gr) continue;
                srcposLU = { (float)grpos.x + hoversrc.x, (float)grpos.y + hoversrc.y };
                srcposRB = { (float)grpos.x + hoversrc.x + hoversrc.w, (float)grpos.y + hoversrc.y + hoversrc.h };
                if (ImGui::IsMouseHoveringRect(srcposLU, srcposRB)) {
                    flicking = ((int)GetTimeLapse(1, &g.timer1) % 200 < 100);
                    ImColor color = flicking ? 0xffff0000 : 0x000000;
                    draw_list->AddRect(srcposLU, srcposRB, color, 0.0f, ImDrawFlags_Closed, 1.0f);

                    if (ImGui::IsItemClicked()) {
                        src_selected = i;
                    }
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone) && ImGui::BeginTooltip())
                    {
                        ImGui::Text("%03d : %d %d ~ %d %d", i, hoversrc.x, hoversrc.y, hoversrc.x + hoversrc.w, hoversrc.y + hoversrc.h);
                        ImGui::Text("Size %d %d", hoversrc.w, hoversrc.h);
                        ImGui::EndTooltip();
                    }
                    
                }
            }

            if (newSquare) {
                ImGui::BeginTooltip();
                ImGui::Text("click any image");
                ImGui::EndTooltip();
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    ImGuiIO& io = ImGui::GetIO();
                    clickPos = { io.MousePos.x - grpos.x,io.MousePos.y - grpos.y };
                    
                    int x = clickPos.x, y = clickPos.y, w, h;
                    AutoSRCObjectPos( &((SRCGR*)arr_SRCGR.data)[gr_selected], &x,&y,&w,&h);
                    NewIMG(gr_selected, x,y,w,h);

                    newSquare = 0;
                }
            }
        }
        

        
    }
    ImGui::EndChild();
    ImGui::EndGroup();

    ImGui::End();
    return 0;
}

//stretch find
int AutoSRCObjectPos(SRCGR* gr, int* x, int* y, int* w, int* h) {

    D3DLOCKED_RECT lockedRect;

    HRESULT hr = gr->texture->LockRect(0, &lockedRect, NULL, D3DLOCK_READONLY);

    if (SUCCEEDED(hr)) {
        DWORD* pPixelData = (DWORD*)lockedRect.pBits;

        int xCur = *x, yCur = *y;
        int wCur = 1, hCur = 1;

        while (1) {
            bool xDone = true, yDone = true;

            //expand x
            for (int cur = yCur; cur < yCur + hCur; cur++) {
                while (pPixelData[cur * (lockedRect.Pitch / 4) + xCur - 1] & 0xFF000000) {
                    xCur--;
                    xDone = false;
                }
                while (pPixelData[cur * (lockedRect.Pitch / 4) + xCur + wCur] & 0xFF000000) {
                    wCur++;
                    xDone = false;
                }
            }
            
            //expand y
            for (int cur = xCur; cur < xCur + wCur; cur++) {
                while (pPixelData[(yCur - 1) * (lockedRect.Pitch / 4) + cur] & 0xFF000000) {
                    yCur--;
                    yDone = false;
                }
                while (pPixelData[(yCur + hCur) * (lockedRect.Pitch / 4) + cur] & 0xFF000000) {
                    hCur++;
                    yDone = false;
                }
            }
            //check
            if (xDone && yDone) break;
        }

        *x = xCur;
        *y = yCur;
        *w = wCur;
        *h = hCur;
        
        gr->texture->UnlockRect(0);
    }
    return 0;
}

int WORKSPACE::drawSrc(int iSRCGR, int iSRCID, int posX, int posY, int w, int h, bool stretch) {
    EnsureSRCGRTexture(iSRCGR);
    SRCGR& img = ((SRCGR*)arr_SRCGR.data)[iSRCGR];

    if (img.texture != NULL) {
        SRC& src = ((SRC*)arr_SRC.data)[iSRCID];

        int sizeX = src.sizeX == -1 ? img.sizeX - src.x : src.sizeX;
        int sizeY = src.sizeY == -1 ? img.sizeY - src.y : src.sizeY;
        ImVec2 display_min, display_max, display_size;
        if (stretch) {
            display_min = ImVec2(src.x / (float)img.sizeX, src.y / (float)img.sizeY);
            display_max = ImVec2((src.x + sizeX) / (float)img.sizeX, (src.y + sizeY) / (float)img.sizeY);
            display_size = ImVec2((float)w, (float)h);
        }
        else {
            display_min = ImVec2(src.x / (float)img.sizeX, src.y / (float)img.sizeY);
            display_max = ImVec2((src.x + sizeX) / (float)img.sizeX, (src.y + sizeY) / (float)img.sizeY);
            display_size = ImVec2(sizeX, sizeY);
        }

        if (src.cycle && (src.div_x >= 1 || src.div_y >= 1)) {
            if (src.div_x == 0) src.div_x = 1;
            if (src.div_y == 0) src.div_y = 1;
            ImVec2 chopsize = { src.sizeX / (float)src.div_x , src.sizeY / (float)src.div_y };

            int units = src.div_x * src.div_y;
            int tick = src.cycle / units;
            int ani = ((int)GetTimeLapse(0, &(g.timer1)) % src.cycle) / tick;

            int ax = ani % src.div_x;
            int ay = ani / src.div_x;

            ImVec2 chopstart = { (src.x + chopsize.x * ax) / (float)img.sizeX ,
                                (src.y + chopsize.y * ay) / (float)img.sizeY };

            ImVec2 chopend = { (src.x - 1 + chopsize.x * (ax + 1)) / (float)img.sizeX ,
                                (src.y - 1 + chopsize.y * (ay + 1)) / (float)img.sizeY };

            
            //ImVec2 pb = ImGui::GetCursorScreenPos();
            //ImVec2 pos = { (float)posX, (float)posY};
            //ImGui::SetCursorPos(pb);
            ImGui::Image(img.texture, chopsize, chopstart, chopend);
            /*ImGui::SetCursorPos(pb);*/
            /*ImGui::Dummy(pb);*/
        }
        else {
            /*ImVec2 pb = ImGui::GetCursorScreenPos();
            ImVec2 pos = { (float)posX, (float)posY};
            ImGui::SetCursorPos(pb);*/
            ImGui::Image(img.texture, display_size, display_min, display_max);;
            /*ImGui::SetCursorPos(pb);*/
            /*ImGui::Dummy(pb);*/
        }
    }
    return 0;
}

int WORKSPACE::drawSrc(int iSRCGR, int iSRCID) {
    return drawSrc(iSRCGR, iSRCID, 0, 0);
}

//int WORKSPACE::drawSrc(IMG img) {
//
//    return 0;
//}


int WORKSPACE::drawSaveMenu() {
    char title[260], input[32], result[32];
    snprintf(title, sizeof(title), "SaveMenu##%d", num);
    snprintf(input, sizeof(input), "##savePathInput%d", num);


    if(ImGui::Begin(title, &wSaveMenu, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_Modal)) {

        ImGui::Text("old path is %s ", mainpath);
        if (newPath[0] == '\0') { //very init
            strncpy(newPath, mainpath, sizeof(newPath));
        }

        if (ImGui::Button("BROWSE", { 0, 0 })) {
            //TODO
        }
        ImGui::SameLine(0, 0);
        ImGui::InputText(input, newPath, IM_ARRAYSIZE(newPath));
        exist = IsFileExist(newPath) || !strcmp(mainpath, newPath); //TODO reduce cpu usage

        if (exist) ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "CAUTION: OVERWRITE");
        ImGui::SeparatorText("Script files");
        ImGui::RadioButton("merge scripts", &split, 0);
        ImGui::RadioButton("split scripts", &split, 1);
        if (split)
            ImGui::TextColored(ImVec4(1.0f, 0.78f, 0.28f, 1.0f),
                "Included scripts stay linked and are saved to their existing paths.");
        else
            ImGui::TextDisabled("Included scripts are embedded in the new main script.");
        ImGui::TextDisabled("Image, font, and other resource files are not copied.");
        if (_stricmp(mainpath, newPath) != 0)
            ImGui::TextColored(ImVec4(0.35f, 0.75f, 1.0f, 1.0f),
                "After Save As, editing will continue on the new main script.");
        ImGui::SeparatorText("Comments");
        ImGui::RadioButton("maintain memo", &nocomment, 0);
        ImGui::RadioButton("delete memo", &nocomment, 1);
        if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone) && ImGui::BeginTooltip()){
            ImGui::Text("this will remove all group, only for Scene start speed on LR2.\nAre your sure this?");
            ImGui::EndTooltip();
        }
        
        if (ImGui::Button("SAVE", { 0, 0 })) {
            char previousMainPath[MAX_PATH] = {};
            strncpy(previousMainPath, mainpath, MAX_PATH - 1);
            success = (SaveSkinScript(newPath, split, nocomment) == 0);
            if (success && _stricmp(previousMainPath, newPath) != 0) {
                // Save As continues editing the newly written main script.
                // In merged mode every expanded line now belongs to that file;
                // in split mode only the former main-file lines change owner.
                for (int row = 0; row < skinfileLines.count; ++row) {
                    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    const char* owner = line.filename.body ? line.filename.outstr() : previousMainPath;
                    if (!split || _stricmp(owner, previousMainPath) == 0)
                        line.filename.assign(newPath);
                }
                strncpy(mainpath, newPath, MAX_PATH - 1);
                mainpath[MAX_PATH - 1] = '\0';
            }
            if(success) snprintf(result, sizeof(result), "SaveResult##Save%d", num);
            ImGui::OpenPopup(result);
        }
        
        snprintf(result, sizeof(result), "SaveResult##Save%d", num);
        if (ImGui::BeginPopupModal(result, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            if (success) {
                ImGui::TextUnformatted("Saved successfully.");
                ImGui::TextWrapped("The workspace is now using: %s", mainpath);
            } else {
                ImGui::TextUnformatted("Save failed - original files were preserved.");
            }
            if (ImGui::Button("OK")) {
                wSaveMenu = 0;
                success = 0;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::End();
    }
    return 0;
}
//{
//    CSTR savepath(mainpath);
//    savepath.add("_master");
//    SaveMaster(savepath);
//}

int WORKSPACE::drawSaveMenu2() {
    char title[260], input[32], result[32];
    snprintf(title, sizeof(title), "SaveMenu##%d", num);
    snprintf(input, sizeof(input), "##savePathInput%d", num);


    if (ImGui::Begin(title, &wSaveMenu2, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_Modal)) {

        ImGui::Text("old path is %s ", mainpath);
        if (newPath[0] == '\0') { //very init
            strncpy(newPath, mainpath, sizeof(newPath));
        }

        if (ImGui::Button("BROWSE", { 0, 0 })) {
            //TODO
        }
        ImGui::SameLine(0, 0);
        ImGui::InputText(input, newPath, IM_ARRAYSIZE(newPath));
        exist = IsFileExist(newPath) || !strcmp(mainpath, newPath); //TODO reduce cpu usage

        if (exist) ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "CAUTION: OVERWRITE");
        ImGui::SeparatorText("Script files");
        ImGui::RadioButton("merge scripts", &split, 0);
        ImGui::RadioButton("split scripts", &split, 1);
        ImGui::SeparatorText("Comments");
        ImGui::RadioButton("maintain memo", &nocomment, 0);
        ImGui::RadioButton("delete memo", &nocomment, 1);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone) && ImGui::BeginTooltip()) {
            ImGui::Text("this will remove all group, only for Scene start speed on LR2.\nAre your sure this?");
            ImGui::EndTooltip();
        }

        if (ImGui::Button("SAVE", { 0, 0 })) {
            success = (SaveSkinScript2(newPath, split, nocomment) == 0);
            if (success) snprintf(result, sizeof(result), "SaveResult##Save%d", num);
            ImGui::OpenPopup(result);
        }

        snprintf(result, sizeof(result), "SaveResult##Save%d", num);
        if (ImGui::BeginPopupModal(result, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted(success ? "save success" : "save failed - original files were preserved");
            if (ImGui::Button("OK")) {
                wSaveMenu2 = 0;
                success = 0;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::End();
    }
    return 0;
}

int WORKSPACE::drawFileManager() {
    char title[260];
    snprintf(title, sizeof(title), "FileManager##%d", num);

    ImGui::Begin(title, &wFileManager);
    //ImGui::Text("they are related files. %d scripts, %d images", arr_subpath.count, arr_imgpath.count);
    ImGui::SeparatorText("Scripts");
    for (int i = 0; i < arr_subpath.count; i++) {
        CSTR& path = ((CSTR*)arr_subpath.data)[i];
        ImGui::Text("%s", path.outstr());
    }
    ImGui::SeparatorText("Images");
    for (int i = 0; i < arr_SRCGR.count; i++) {
        SRCGR& img = ((SRCGR*)arr_SRCGR.data)[i];
        ImGui::Text("(%03d)%02d - %s %s", img.isIf, img.grID, img.path.outstr(), img.fromWildcard? "from *":"");
    }

    ImGui::End();
    return 0;
}

int WORKSPACE::drawTreeView() {
    char title[260];
    snprintf(title, sizeof(title), "TreeView##%d", num);
    ImGui::Begin(title, &wTreeView);

    snprintf(title, sizeof(title), "Tree##%d", num);
    

    if (ImGui::BeginTable(title, 24, ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody)) //ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody))
    {

    //    // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
    //    ImGui::TableSetupColumn("Command", ImGuiTableColumnFlags_NoHide);
    //    for (int p = 0; p < 24 - 1; p++) {
    //        ImGui::TableSetupColumn("Params", ImGuiTableColumnFlags_WidthFixed, 12.0f);
    //    }
    //    ImGui::TableHeadersRow();

    //    /*static int hideGroups[MAX_IFDEPTH];
    //    for (int i = 0; i < MAX_IFDEPTH; i++) {
    //        hideGroups[i] = -1;
    //    }*/
    //    
    //    for (int l = 0; l < skinfileLines.count; l++) {
    //        int hide = 0;
    //        SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[l];
    //        ImGui::TableNextRow();
    //        ImGui::TableNextColumn();

    //        if (line.isComment) continue;
    //        CSTR& command = line.csv.str[0];
    //        CSTR *params = line.csv.str;
    //        bool is_head = //!(strcmp(command, "#INCLUDE")) ||
    //                        !(strcmp(command, "#IF")) ||
    //                        !(strcmp(command, "#ELSEIF")) ||
    //                        !(strcmp(command, "#ELSE"));
    //        bool is_end = !(strcmp(command, "#ELSEIF")) ||
    //                        !(strcmp(command, "#ELSE")) ||
    //                        !(strcmp(command, "#ENDIF"));

    //        static ImGuiTreeNodeFlags tree_node_flags_base = ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_DrawLinesFull;
    //        ImGuiTreeNodeFlags node_flags = tree_node_flags_base;


    //        int lineID = ((SKINFILELINEREAD*)skinfileLines.data)[l].numTotal;
    //        sprintf(title, "%s##%d", command.outstr(), lineID);

    //        if (is_end) {
    //            ImGui::TreePop();
    //        }
    //        if (is_head)
    //        {   
    //            bool open = ImGui::TreeNodeEx(title, node_flags);
    //            
    //            for (int p = 1; p < 24 - 1; p++) {
    //                ImGui::TableNextColumn();
    //                ImGui::Text("%s", params[p].outstr());
    //            };
    //        }
    //        
    //        /*else if (!hide){
    //            ImGui::TreeNodeEx(title, node_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);

    //            for (int p = 1; p < 24 - 1; p++) {
    //                ImGui::TableNextColumn();
    //                ImGui::Text("%s", params[p]);
    //            };
    //        }*/

    //    }        
        ImGui::EndTable();
    }

    ImGui::End();
    return 0;
}

int WORKSPACE::wildcardTOAll(char* path) {
    WIN32_FIND_DATA FindFileData;
    LPWIN32_FIND_DATAA lpFindFileData;
    HANDLE hFindFile = FindFirstFileA(path, (LPWIN32_FIND_DATAA)&FindFileData);
    if (hFindFile == (HANDLE)-1) {
        return -1;
    }




    return 0;
}

int WORKSPACE::drawNewskin() {
    char title[260];
    snprintf(title, sizeof(title), "NewSkin##%d", num);
    if(ImGui::Begin(title, &wNewskin)) {


        static char buf1[32] = ""; ImGui::InputText("title", buf1, IM_ARRAYSIZE(buf1));
        static char buf5[32] = ""; ImGui::InputText("maker", buf5, IM_ARRAYSIZE(buf5));
        
        static int item_selected_idx = 0;
        const char* combo_preview_value = SKINTYPESTR[item_selected_idx];
        snprintf(title, sizeof(title), "##NewSkin_Combo##%d", num);
        if (ImGui::BeginCombo(title, combo_preview_value))
        {
            for (int n = 0; n < sizeof(SKINTYPESTR)/sizeof(char*); n++)
            {
                const bool is_selected = (item_selected_idx == n);
                if (ImGui::Selectable(SKINTYPESTR[n], is_selected))
                    item_selected_idx = n;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        static char buf2[32] = ""; ImGui::InputText("Resolution X", buf2, IM_ARRAYSIZE(buf2), ImGuiInputTextFlags_CharsDecimal);
        static char buf3[32] = ""; ImGui::InputText("Resolution Y", buf3, IM_ARRAYSIZE(buf3), ImGuiInputTextFlags_CharsDecimal);
        ImGui::Text("LR2files/Theme/"); ImGui::SameLine(); static char buf4[MAX_PATH] = "NewName/New.lr2skin"; ImGui::InputText("path", buf4, IM_ARRAYSIZE(buf4));

        if(ImGui::Button("make")) {

        }
        ImGui::End();
    }
    return 0;
}

int WORKSPACE::drawSimplePreview() {
    char title[260];
    snprintf(title, sizeof(title), "SimplePreview##%d", num);
    if (ImGui::Begin(title, &wSimplePreview, ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
        //ImDrawList* draw_list = ImGui::GetWindowDrawList();
        //
        //ImVec2 pb = ImGui::GetCursorScreenPos();
        //pb.x += 400;
        //pb.y += 400;
        ////ImVec2 bgSize = { skinSizeX,skinSizeY };
        //ImGui::Dummy(pb);
        //draw_list->AddRectFilled(pb, { skinSizeX+pb.x,skinSizeY+pb.y }, 0xFF000000, 0, ImDrawFlags_None);
        //
        //ImGui::SetCursorScreenPos(pb);

        //DST *dst = ((DST*)arr_DST.data);

        ////for (int i = 0; i < arr_DST.count; i++) {
        ////    ImGui::SetCursorScreenPos(pb);
        ////    ImVec2 PointTopLeft = { (float)dst[i].x, (float)dst[i].y };
        ////    ImVec2 PointBottomRight = { (float)dst[i].x + dst[i].w, (float)dst[i].y + dst[i].h };
        ////    //ImVec4 xywh = {PointTopLeft , PointBottomRight };
        ////    draw_list->AddRectFilled(PointTopLeft, PointBottomRight, (0xFF000000 | (0xFF0000 >> i)), 0, ImDrawFlags_None);

        ////    //drawSrc(((SRC*)arr_SRC.data)[dst[i].src].gr, dst[i].src, dst[i].x, dst[i].y);
        ////    ImGui::Dummy({ (float)dst[i].w, (float)dst[i].h });
        ////}

        //skinSizeX;
        //skinSizeY;
        ////zoom in zoom out
        //
        //


        //ImGui::End();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        ImGui::Text("dst animation");
        ImGui::SameLine(0, 0);
        ImGui::ColorEdit4("MyColor##3", (float*)&bgColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None);
        const ImVec2 pbs = ImGui::GetCursorScreenPos();

        const ImVec2 pb = ImGui::GetCursorPos();
        ImGui::SetCursorPos(pb);
        ImGui::Image(transBackground, { (float)skinSizeX, (float)skinSizeY }, { 0,0 }, { skinSizeX / (float)32.0, skinSizeY / (float)32.0 });
        const ImVec2 belowImage = ImGui::GetCursorPos();

        draw_list->AddRectFilled(pbs, { skinSizeX + pbs.x,skinSizeY + pbs.y }, ImGui::GetColorU32(bgColor), 0, ImDrawFlags_None);

        DST* dst = ((DST*)arr_DST.data);

        for (int i = 0; i < arr_DST.count; i++) {
            //ImVec2 pos = { pb.x + dst[i].x, pb.y + dst[i].y };

            if (dst[i].src != -1) {

                float dx, dy, dw, dh;
                float da, dr, dg, db;

                DST_ANIMATION& dstdFirst = ((DST_ANIMATION*)dst[i].arr_animation.data)[0];
                DST_ANIMATION& dstdLast = ((DST_ANIMATION*)dst[i].arr_animation.data)[dst->arr_animation.count - 1];

                int tStart = dstdFirst.time;
                int tEnd = dstdLast.time;
                int viewTime = (int)DstViewTime;
                dst[i].loop;
                int t = tEnd;
                int ani;

                if (tStart <= tEnd && tStart <= viewTime && (0 <= dst[i].loop || viewTime <= tEnd)) {
                    if (tStart == tEnd || dst[i].loop == tEnd) {
                        if (viewTime < t) {
                            t = viewTime;
                        }
                    }
                    else if (dst[i].loop < tEnd) {
                        t = viewTime;
                        if (tEnd < viewTime) {
                            //if (dst[i].loop == -1) continue; // only for SE
                            t = (int)(viewTime - dst[i].loop) % (tEnd - dst[i].loop) + dst[i].loop;
                        }
                    }
                    else {
                        t = 0;
                    }

                    ani = 0;
                    for (int j = 0; j < dst->arr_animation.count; j++) {
                        if (((DST_ANIMATION*)dst[i].arr_animation.data)[j].time <= t) {
                            ani = j;
                        }
                    }
                    DST_ANIMATION dstd1 = ((DST_ANIMATION*)dst[i].arr_animation.data)[ani];
                    if (t != dstd1.time && ani != dst->arr_animation.count - 1) {
                        DST_ANIMATION dstd2 = ((DST_ANIMATION*)dst[i].arr_animation.data)[ani + 1];
                        dx = ChangeValueByTime(dstd1.x, dstd2.x, dstd1.time, dstd2.time, t, dstdFirst.acc);
                        dy = ChangeValueByTime(dstd1.y, dstd2.y, dstd1.time, dstd2.time, t, dstdFirst.acc);
                        dw = ChangeValueByTime(dstd1.w, dstd2.w, dstd1.time, dstd2.time, t, dstdFirst.acc);
                        dh = ChangeValueByTime(dstd1.h, dstd2.h, dstd1.time, dstd2.time, t, dstdFirst.acc);

                        da = ChangeValueByTime(dstd1.a, dstd2.a, dstd1.time, dstd2.time, t, dstdFirst.acc);
                        dr = ChangeValueByTime(dstd1.r, dstd2.r, dstd1.time, dstd2.time, t, dstdFirst.acc);
                        dg = ChangeValueByTime(dstd1.g, dstd2.g, dstd1.time, dstd2.time, t, dstdFirst.acc);
                        db = ChangeValueByTime(dstd1.b, dstd2.b, dstd1.time, dstd2.time, t, dstdFirst.acc);
                    }
                    else {
                        dx = dstd1.x;
                        dy = dstd1.y;
                        dw = dstd1.w;
                        dh = dstd1.h;

                        da = dstd1.a;
                        dr = dstd1.r;
                        dg = dstd1.g;
                        db = dstd1.b;
                    }
                    ImVec2 pos = { pb.x + dx, pb.y + dy };
                    ImGui::SetCursorPos(pos);

                    drawSrc(((SRC*)arr_SRC.data)[dst[i].src].gr, dst[i].src, 0, 0, dw, dh, 1);

                }
            }

        }
        ImGui::End();
    }


    return 0;
}

int WORKSPACE::drawDstView() {

    char title[260];
    snprintf(title, sizeof(title), "dstView##%d", num);
    if (ImGui::Begin(title, &wDstView)) {

        snprintf(title, sizeof(title), "dstList##%d", num);
        if (ImGui::BeginChild(title, { 250,-1 }, ImGuiChildFlags_ResizeX | ImGuiChildFlags_FrameStyle)) {
            for (int i = 0; i < arr_DST.count; i++) {
                ImGui::PushID(i);
                DST& dst = ((DST*)arr_DST.data)[i];

                char buf[260];
                sprintf(buf, "%03d:%03d(%s)", dst.src, dst.timer, timerName(dst.timer));
                if (ImGui::Selectable(buf, selected_dst == i)) {
                    selected_dst = i;
                    SetTimeLapse(1, &g.timer1);
                }
                if(ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone) && ImGui::BeginTooltip()) {
                    if (dst.op1) ImGui::Text("%03d(%s)\n", dst.op1, dstName(dst.op1));
                    if (dst.op2) ImGui::Text("%03d(%s)\n", dst.op2, dstName(dst.op2));
                    if (dst.op3) ImGui::Text("%03d(%s)", dst.op3, dstName(dst.op3));
                    ImGui::EndTooltip();
                }
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
        ImGui::SameLine();

        snprintf(title, sizeof(title), "dstAnimationView##%d", num);
        if (ImGui::BeginChild(title, { -1,0 }, ImGuiChildFlags_ResizeX | ImGuiChildFlags_FrameStyle)) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            ImGui::Text("dst animation");
            ImGui::SameLine(0, 0);
            ImGui::ColorEdit4("MyColor##3", (float*)&bgColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None);
            const ImVec2 pbs = ImGui::GetCursorScreenPos();

            const ImVec2 pb = ImGui::GetCursorPos();
            ImGui::SetCursorPos(pb);
            ImGui::Image(transBackground, { (float)skinSizeX, (float)skinSizeY }, { 0,0 }, { skinSizeX / (float)32.0, skinSizeY / (float)32.0 });
            const ImVec2 belowImage = ImGui::GetCursorPos();

            draw_list->AddRectFilled(pbs, { skinSizeX + pbs.x,skinSizeY + pbs.y }, ImGui::GetColorU32(bgColor), 0, ImDrawFlags_None);

            DST* dst = ((DST*)arr_DST.data);

            for (int i = 0; i < arr_DST.count; i++) {
                //ImVec2 pos = { pb.x + dst[i].x, pb.y + dst[i].y };
                
                if (i == selected_dst) {
                    
                    if (dst[i].src != -1) {

                        float dx, dy, dw, dh;
                        float da, dr, dg, db;

                        DST_ANIMATION &dstdFirst = ((DST_ANIMATION*)dst[i].arr_animation.data)[0];
                        DST_ANIMATION &dstdLast = ((DST_ANIMATION*)dst[i].arr_animation.data)[dst->arr_animation.count-1];

                        int tStart = dstdFirst.time;
                        int tEnd = dstdLast.time;
                        int viewTime = (int)DstViewTime;
                        dst[i].loop;
                        int t = tEnd;
                        int ani;

                        if (tStart <= tEnd && tStart <= viewTime && (0 <= dst[i].loop || viewTime <= tEnd)) {
                            if (tStart == tEnd || dst[i].loop == tEnd) {
                                if (viewTime < t) {
                                    t = viewTime;
                                }
                            }
                            else if (dst[i].loop < tEnd) {
                                t = viewTime;
                                if (tEnd < viewTime) {
                                    //if (dst[i].loop == -1) continue; // only for SE
                                    t = (int)(viewTime - dst[i].loop) % (tEnd - dst[i].loop) + dst[i].loop;
                                }
                            }
                            else {
                                t = 0;
                            }

                            ani = 0;
                            for (int j = 0; j < dst->arr_animation.count; j++) {
                                if (((DST_ANIMATION*)dst[i].arr_animation.data)[j].time <= t) {
                                    ani = j;
                                }
                            }
                            DST_ANIMATION dstd1 = ((DST_ANIMATION*)dst[i].arr_animation.data)[ani];
                            if (t != dstd1.time && ani != dst->arr_animation.count-1) {
                                DST_ANIMATION dstd2 = ((DST_ANIMATION*)dst[i].arr_animation.data)[ani+1];
                                dx = ChangeValueByTime(dstd1.x, dstd2.x, dstd1.time, dstd2.time, t, dstdFirst.acc);
                                dy = ChangeValueByTime(dstd1.y, dstd2.y, dstd1.time, dstd2.time, t, dstdFirst.acc);
                                dw = ChangeValueByTime(dstd1.w, dstd2.w, dstd1.time, dstd2.time, t, dstdFirst.acc);
                                dh = ChangeValueByTime(dstd1.h, dstd2.h, dstd1.time, dstd2.time, t, dstdFirst.acc);

                                da = ChangeValueByTime(dstd1.a, dstd2.a, dstd1.time, dstd2.time, t, dstdFirst.acc);
                                dr = ChangeValueByTime(dstd1.r, dstd2.r, dstd1.time, dstd2.time, t, dstdFirst.acc);
                                dg = ChangeValueByTime(dstd1.g, dstd2.g, dstd1.time, dstd2.time, t, dstdFirst.acc);
                                db = ChangeValueByTime(dstd1.b, dstd2.b, dstd1.time, dstd2.time, t, dstdFirst.acc);
                            }
                            else {
                                dx = dstd1.x;
                                dy = dstd1.y;
                                dw = dstd1.w;
                                dh = dstd1.h;

                                da = dstd1.a;
                                dr = dstd1.r;
                                dg = dstd1.g;
                                db = dstd1.b;
                            }
                            ImVec2 pos = { pb.x + dx, pb.y + dy };
                            ImGui::SetCursorPos(pos);

                            drawSrc(((SRC*)arr_SRC.data)[dst[i].src].gr, dst[i].src, 0, 0, dw, dh, 1);
                            
                        }
                    }
                     
                }
            }

            //skinSizeX;
            //skinSizeY;
            //zoom in zoom out

            
            ImGui::SetCursorPos(belowImage);
            snprintf(title, sizeof(title), "dstAnimationViewTimer##%d", num);
            if (!isDstViewTimeStop) {
                DstViewTime = GetTimeLapse(1, &g.timer1);
            }
            ImGui::SliderFloat(title, &DstViewTime, 0, 240000, "%.0f", 0);// ImGuiSliderFlags_)
            if(ImGui::Button("PLAY")) {
                SetTimeLapse(1,&g.timer1);
                isDstViewTimeStop = 0;
            }
            ImGui::SameLine(0, 0);
            if (ImGui::Button("STOP")) {
                ResetTimeLapse(1,&g.timer1);
                isDstViewTimeStop = 1;
            }
            
            dst = &((DST*)arr_DST.data)[selected_dst];
            if (dst->op1) ImGui::Text("%03d(%s)\n", dst->op1, dstName(dst->op1));
            if (dst->op2) ImGui::Text("%03d(%s)\n", dst->op2, dstName(dst->op2));
            if (dst->op3) ImGui::Text("%03d(%s)", dst->op3, dstName(dst->op3));
            ImGui::Text("%s %dlines", dst->name.outstr(), dst->animation);
            for (int i = 0; i < dst->arr_animation.count; i++)
            {
                DST_ANIMATION dstd = ((DST_ANIMATION*)dst->arr_animation.data)[i];                
                ImGui::Text("%d %.0f %.0f %.0f %.0f", dstd.time, dstd.x, dstd.y, dstd.w, dstd.h);
                if (i == 0) ImGui::Text("%d", dst->loop);;
            }
            
        }
        ImGui::EndChild();
    }
    ImGui::End();
    return 0;
}

//HOW TO ADD FEATURE - STEP 2 : write function

int WORKSPACE::drawObjectManagerTest() {

    char title[260];
    snprintf(title, sizeof(title), "objectManagerTest##%d", num);

    int objcount = 0;
    if (ImGui::Begin(title, &wObjectManagerTest)) {
        char buf[260];

        for (int i = 0; i < g.skstruct.image.srcSize; i++) {
            SRCstruct& src = g.skstruct.image.src[i];
            DSTstruct& dst = g.skstruct.image.dst[i];
            sprintf(buf, "$OBJ_IMAGE%03d: %s %s %s %s %s %s %s %s", i, timerName(src.timer,1), dstName(src.op1,1), dstName(src.op2,1), dstName(src.op3,1), timerName(dst.timer,1), dstName(dst.op1,1), dstName(dst.op2,1), dstName(dst.op3,1));
            if (ImGui::Selectable(buf)) {
                preview_selected_obj = { dst.draw[dst.dstCount-1].x, dst.draw[dst.dstCount - 1].y, dst.draw[dst.dstCount - 1].w, dst.draw[dst.dstCount - 1].h};
                selectedObjectTest = objcount;
            }
            objcount++;
            //ImGui::Text("#SRC_IMAGE%03d: %s %s %s %s", i, timerName(src.timer), src.op1 ? dstName(src.op1) : "", src.op2 ? dstName(src.op2) : "", src.op3 ? dstName(src.op3) : "");
            //ImGui::Text("#DST_IMAGE%03d: %s %s %s %s", i, timerName(dst.timer), dst.op1 ? dstName(dst.op1) : "", dst.op2 ? dstName(dst.op2) : "", dst.op3 ? dstName(dst.op3) : "");
        }
        for (int i = 0; i < g.skstruct.otherObject[0].srcSize; i++) {
            SRCstruct& src = g.skstruct.otherObject[0].src[i];
            DSTstruct& dst = g.skstruct.otherObject[0].dst[i];
            sprintf(buf, "$OBJ_TEXT%03d: %s %s %s %s %s", i, textName(src.st), timerName(dst.timer, 1), dstName(dst.op1, 1), dstName(dst.op2, 1), dstName(dst.op3, 1));
            if (ImGui::Selectable(buf)) {
                preview_selected_obj = { dst.draw[dst.dstCount - 1].x, dst.draw[dst.dstCount - 1].y, dst.draw[dst.dstCount - 1].w, dst.draw[dst.dstCount - 1].h };
                selectedObjectTest = objcount;
            }
            objcount++;
        }
        for (int i = 0; i < g.skstruct.otherObject[1].srcSize; i++) {
            SRCstruct& src = g.skstruct.otherObject[1].src[i];
            DSTstruct& dst = g.skstruct.otherObject[1].dst[i];
            sprintf(buf, "$OBJ_BUTTON%03d: %s %s %s panel:%d %s %s %s %s", i, timerName(src.timer, 1), buttonName(src.op1),src.op2? "":"disabled", src.op3, timerName(dst.timer, 1), dstName(dst.op1, 1), dstName(dst.op2, 1), dstName(dst.op3, 1));
            if (ImGui::Selectable(buf)) {
                preview_selected_obj = { dst.draw[dst.dstCount - 1].x, dst.draw[dst.dstCount - 1].y, dst.draw[dst.dstCount - 1].w, dst.draw[dst.dstCount - 1].h };
                selectedObjectTest = objcount;
            }
        }
        for (int i = 0; i < g.skstruct.otherObject[2].srcSize; i++) {
            SRCstruct& src = g.skstruct.otherObject[2].src[i];
            DSTstruct& dst = g.skstruct.otherObject[2].dst[i];
            ImGui::Text("#SRC_SLIDER%03d: %s %s %s %s", i, timerName(src.timer), src.op1 ? dstName(src.op1) : "", src.op2 ? dstName(src.op2) : "", src.op3 ? dstName(src.op3) : "");
            ImGui::Text("#DST_SLIDER%03d: %s %s %s %s", i, timerName(dst.timer), dst.op1 ? dstName(dst.op1) : "", dst.op2 ? dstName(dst.op2) : "", dst.op3 ? dstName(dst.op3) : "");
        }
        for (int i = 0; i < g.skstruct.otherObject[3].srcSize; i++) {
            SRCstruct& src = g.skstruct.otherObject[3].src[i];
            DSTstruct& dst = g.skstruct.otherObject[3].dst[i];
            ImGui::Text("#SRC_ONMOUSET%03d: %s %s %s %s", i, timerName(src.timer), src.op1 ? dstName(src.op1) : "", src.op2 ? dstName(src.op2) : "", src.op3 ? dstName(src.op3) : "");
            ImGui::Text("#DST_ONMOUSE%03d: %s %s %s %s", i, timerName(dst.timer), dst.op1 ? dstName(dst.op1) : "", dst.op2 ? dstName(dst.op2) : "", dst.op3 ? dstName(dst.op3) : "");
        }
        for (int i = 0; i < g.skstruct.otherObject[4].srcSize; i++) {
            SRCstruct& src = g.skstruct.otherObject[4].src[i];
            DSTstruct& dst = g.skstruct.otherObject[4].dst[i];
            ImGui::Text("#SRC_BGA%03d: %s %s %s %s", i, timerName(src.timer), src.op1 ? dstName(src.op1) : "", src.op2 ? dstName(src.op2) : "", src.op3 ? dstName(src.op3) : "");
            ImGui::Text("#DST_BGA%03d: %s %s %s %s", i, timerName(dst.timer), dst.op1 ? dstName(dst.op1) : "", dst.op2 ? dstName(dst.op2) : "", dst.op3 ? dstName(dst.op3) : "");
        }
        for (int i = 0; i < g.skstruct.otherObject[5].srcSize; i++) {
            SRCstruct& src = g.skstruct.otherObject[5].src[i];
            DSTstruct& dst = g.skstruct.otherObject[5].dst[i];
            ImGui::Text("#SRC_BARGRAPH%03d: %s %s %s %s", i, timerName(src.timer), src.op1 ? dstName(src.op1) : "", src.op2 ? dstName(src.op2) : "", src.op3 ? dstName(src.op3) : "");
            ImGui::Text("#DST_BARGRAPH%03d: %s %s %s %s", i, timerName(dst.timer), dst.op1 ? dstName(dst.op1) : "", dst.op2 ? dstName(dst.op2) : "", dst.op3 ? dstName(dst.op3) : "");
        }
        for (int i = 0; i < g.skstruct.otherObject[6].srcSize; i++) {
            SRCstruct& src = g.skstruct.otherObject[6].src[i];
            DSTstruct& dst = g.skstruct.otherObject[6].dst[i];
            sprintf(buf, "$OBJ_NUMBER%03d: %s %s %s %s %s %s", i, timerName(src.timer,1), numberName(src.op1), timerName(dst.timer,1), dstName(dst.op1,1), dstName(dst.op2,1), dstName(dst.op3,1));
            if (ImGui::Selectable(buf)) {
                preview_selected_obj = { dst.draw[dst.dstCount - 1].x, dst.draw[dst.dstCount - 1].y, dst.draw[dst.dstCount - 1].w, dst.draw[dst.dstCount - 1].h };
            }
        }
        for (int i = 0; i < g.skstruct.otherObject[7].srcSize; i++) {
            SRCstruct& src = g.skstruct.otherObject[7].src[i];
            DSTstruct& dst = g.skstruct.otherObject[7].dst[i];
            ImGui::Text("#SRC_MASK%03d: %s %s %s %s", i, timerName(src.timer), src.op1 ? dstName(src.op1) : "", src.op2 ? dstName(src.op2) : "", src.op3 ? dstName(src.op3) : "");
            ImGui::Text("#DST_MASK%03d: %s %s %s %s", i, timerName(dst.timer), dst.op1 ? dstName(dst.op1) : "", dst.op2 ? dstName(dst.op2) : "", dst.op3 ? dstName(dst.op3) : "");
        }

        //select

        //
        if(true){
            SRCstruct& src = g.skstruct.src_MOUSECURSOR;
            DSTstruct& dst = g.skstruct.dst_MOUSECURSOR;
            ImGui::Text("#SRC_MOUSECURSOR: %s %s %s %s", timerName(src.timer), src.op1 ? dstName(src.op1) : "", src.op2 ? dstName(src.op2) : "", src.op3 ? dstName(src.op3) : "");
            ImGui::Text("#DST_MOUSECURSOR: %s %s %s %s", timerName(dst.timer), dst.op1 ? dstName(dst.op1) : "", dst.op2 ? dstName(dst.op2) : "", dst.op3 ? dstName(dst.op3) : "");
        }

        for(int i = 0 ; i < 20 ; i++){
            SRCstruct& src = g.skstruct.src_NOTE[i];
            ImGui::Text("#SRC_NOTE%02d: %s %s %s %s", i, timerName(src.timer), src.op1 ? dstName(src.op1) : "", src.op2 ? dstName(src.op2) : "", src.op3 ? dstName(src.op3) : "");
        }

    }
    if (ImGui::BeginPopupContextWindow()){
        ImGui::Text("selected : %d", selectedObjectTest);
        ImGui::Separator();
        ImGui::MenuItem("move");
        ImGui::MenuItem("delete");
        ImGui::EndPopup();
    }
    ImGui::End();
    return 0;
}

//TODO - iftree, *wildcardtree, insert, delete, group, dst thing
//group should have both if / endif
// new file to "skin wizard", which makes mockup and create texture template
int WORKSPACE::drawNewObject() {

    char title[260];
    snprintf(title, sizeof(title), "%s##%d",
        newCommandIncludeAll ? "New Command / Setting" : "New Object", num);
    if (ImGui::Begin(title, &wNewObject)) {

        CSVbuf* csv = ((CSVbuf*)arr_CommandHelp.data);
        static bool isCsvInit = false;
        static CSVbuf nCsv;
        static int initializedCommand = -1;
        auto isCreatableObjectCommand = [&](const char* command) {
            if (!command || !*command) return false;
            const std::vector<SEObjectGroupDef>& groups = g_seObjectEditorModel.Groups();
            for (const SEObjectGroupDef& group : groups)
                for (const std::string& candidate : group.commands)
                    if (candidate == command) return true;
            return false;
        };
        auto isSelectableCommand = [&](const char* command) {
            if (!command || command[0] != '#') return false;
            return newCommandIncludeAll || isCreatableObjectCommand(command);
        };
        if (arr_CommandHelp.count <= 0) {
            ImGui::TextDisabled("Object command definitions are not loaded.");
            ImGui::End();
            return 0;
        }
        if (selected_command < 0 || selected_command >= arr_CommandHelp.count ||
            !isSelectableCommand(csv[selected_command].str[0].outstr())) {
            selected_command = 0;
            while (selected_command < arr_CommandHelp.count &&
                !isSelectableCommand(csv[selected_command].str[0].outstr()))
                ++selected_command;
            if (selected_command >= arr_CommandHelp.count) selected_command = 0;
        }
        if (ImGui::BeginCombo("command", csv[selected_command].str[0], ImGuiComboFlags_None)) {
            for (int op = 0; op < arr_CommandHelp.count; op++) {
                if (!csv[op].str[0].body ||
                    !isSelectableCommand(csv[op].str[0].outstr())) continue;
                ImGui::PushID(op);
                const bool is_selected = (selected_command == op);
                char opname[64];
                const char* cmd = csv[op].str[0].outstr();
                const char* category = isCreatableObjectCommand(cmd) ? "Object" :
                    ((!strcmp(cmd, "#INFORMATION") || !strcmp(cmd, "#RESOLUTION") ||
                        !strcmp(cmd, "#CUSTOMOPTION") || !strcmp(cmd, "#CUSTOMFILE") ||
                        !strcmp(cmd, "#ENDOFHEADER")) ? "Header" :
                    ((!strcmp(cmd, "#IF") || !strcmp(cmd, "#ELSEIF") ||
                        !strcmp(cmd, "#ELSE") || !strcmp(cmd, "#ENDIF")) ? "Condition" : "Setting"));
                sprintf(opname, "[%s] %s", category, csv[op].str[0].body);

                if (ImGui::Selectable(opname, is_selected)) {
                    selected_command = op;
                    isCsvInit = false;
                }
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();

                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        if (!isCsvInit || initializedCommand != selected_command) {
            SplitCSV("", &nCsv, ",");
            nCsv.str[0].assign(csv[selected_command].str[0].outstr());
            initializedCommand = selected_command;
            isCsvInit = true;
        }
        for (int column = 1; column < 30; column++) {
            ImGui::PushID(column);
            if (csv[selected_command].str[column].isDiff(""))
                CstrInputText(csv[selected_command].str[column], &nCsv.str[column], ImGuiInputTextFlags_None);
                //ImGui::InputText(csv[selected_command].str[column], nCsv.str[column], 0);
            ImGui::PopID();
        }


        if (ImGui::Button("OK")) {
            nCsv.str[0].assign(csv[selected_command].str[0].outstr());
            CSTR srcLine;
            CsvToCSTR(nCsv, srcLine);
            int insertAt = newObjectInsertPosition;
            if (insertAt < 0 || insertAt > skinfileLines.count) insertAt = skinfileLines.count;
            const char* selectedCommand = csv[selected_command].str[0].outstr();
            const bool isHeaderCommand = !strcmp(selectedCommand, "#INFORMATION") ||
                !strcmp(selectedCommand, "#RESOLUTION") || !strcmp(selectedCommand, "#CUSTOMOPTION") ||
                !strcmp(selectedCommand, "#CUSTOMFILE") || !strcmp(selectedCommand, "#ENDOFHEADER");
            if (newCommandIncludeAll && isHeaderCommand && strcmp(selectedCommand, "#ENDOFHEADER") != 0) {
                for (int row = 0; row < skinfileLines.count; ++row) {
                    SKINFILELINEREAD& candidate = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    if (candidate.csv.str[0].body && candidate.csv.str[0].isSame("#ENDOFHEADER")) {
                        insertAt = row;
                        break;
                    }
                }
            }

            int wrapFirst = skinfileLines.count;
            int wrapLast = -1;
            bool wrapSameCondition = true;
            if (newCommandIncludeAll && !strcmp(selectedCommand, "#IF")) {
                const std::vector<SEObjectInstance>& objects = g_seObjectEditorModel.Objects();
                for (int modelIndex : preview_selected_object_model_indices) {
                    if (modelIndex < 0 || modelIndex >= (int)objects.size()) continue;
                    const SEObjectInstance& object = objects[modelIndex];
                    if (object.ifgroup != newObjectIfgroup) wrapSameCondition = false;
                    for (int row : object.rows) {
                        if (row < wrapFirst) wrapFirst = row;
                        if (row > wrapLast) wrapLast = row;
                    }
                    if (!object.rows.empty() && object.rows.front() > 0) {
                        const int metaRow = object.rows.front() - 1;
                        SKINFILELINEREAD& meta = ((SKINFILELINEREAD*)skinfileLines.data)[metaRow];
                        const char* metaText = meta.line.body ? meta.line.outstr() : "";
                        if (strncmp(metaText, "$SE_OBJECT_ID,", 14) == 0 && metaRow < wrapFirst)
                            wrapFirst = metaRow;
                    }
                }
                if (!wrapSameCondition) { wrapFirst = skinfileLines.count; wrapLast = -1; }
            }

            auto insertObjectLine = [&](const char* text) {
                InsertLine(insertAt);
                SKINFILELINEREAD& inserted = ((SKINFILELINEREAD*)skinfileLines.data)[insertAt];
                CSTR oldLine(inserted.line);
                EditLine(insertAt, oldLine, CSTR(text));
                inserted.filename.assign(mainpath);
                inserted.ifgroup = newObjectIfgroup;
                ++insertAt;
            };
            const bool selectedIsObject = isCreatableObjectCommand(selectedCommand);
            const bool wrapSelection = newCommandIncludeAll && !strcmp(selectedCommand, "#IF") &&
                wrapLast >= wrapFirst;
            if (wrapSelection) insertAt = wrapFirst;
            if (selectedIsObject) {
                char generatedId[64];
                snprintf(generatedId, sizeof(generatedId), "obj_%08llX_new", GetTickCount64());
                std::string idLine = std::string("$SE_OBJECT_ID,") + generatedId;
                insertObjectLine(idLine.c_str());
            }
            insertObjectLine(srcLine.outstr());

            if (wrapSelection) {
                const int ifRow = wrapFirst;
                IFUNIT condition = {};
                for (int value = 0; value < 10; ++value)
                    condition.data[value] = nCsv.str[value + 1].body ? atol(nCsv.str[value + 1]) : 0;
                condition.parentID = newObjectIfgroup;
                condition.depth = newObjectIfgroup >= 0 && newObjectIfgroup < arr_ifunit.count
                    ? ((IFUNIT*)arr_ifunit.data)[newObjectIfgroup].depth + 1 : 1;
                condition.order = 0;
                const int newIfgroup = arr_ifunit.count;
                arr_ifunit.push_back(&condition);

                ((SKINFILELINEREAD*)skinfileLines.data)[ifRow].ifgroup = newIfgroup;
                ((SKINFILELINEREAD*)skinfileLines.data)[ifRow].isIfGroupHead = true;
                ((SKINFILELINEREAD*)skinfileLines.data)[ifRow].isGroupHead = true;
                // The opening line shifted every wrapped row down by one.
                for (int row = wrapFirst + 1; row <= wrapLast + 1; ++row)
                    ((SKINFILELINEREAD*)skinfileLines.data)[row].ifgroup = newIfgroup;
                insertAt = wrapLast + 2;
                insertObjectLine("#ENDIF");
                const int endifRow = insertAt - 1;
                ((SKINFILELINEREAD*)skinfileLines.data)[endifRow].ifgroup = newIfgroup;
                ((SKINFILELINEREAD*)skinfileLines.data)[endifRow].isIfGroupEnd = true;
                ((SKINFILELINEREAD*)skinfileLines.data)[endifRow].isGroupEnd = true;
            }
            else if (newCommandIncludeAll && !strcmp(selectedCommand, "#IF")) {
                insertObjectLine("#ENDIF");
            }

            // Most editable objects are an SRC declaration followed by one
            // matching DST frame. Create that frame automatically when the
            // command exists in skinHelper.txt.
            std::string dstCommand = selectedCommand;
            if (dstCommand.compare(0, 4, "#SRC") == 0) dstCommand.replace(0, 4, "#DST");
            for (int commandIndex = 0; selectedIsObject &&
                strncmp(selectedCommand, "#SRC", 4) == 0 && commandIndex < arr_CommandHelp.count; ++commandIndex) {
                if (!csv[commandIndex].str[0].body ||
                    dstCommand != csv[commandIndex].str[0].outstr()) continue;
                CSVbuf dstCsv;
                SplitCSV("", &dstCsv, ",");
                dstCsv.str[0].assign(dstCommand.c_str());
                // Preserve an explicit object index when both commands use it.
                CSTR srcIndexHelp = GetCommandHelp(csv[selected_command].str[0].outstr(), 1);
                CSTR dstIndexHelp = GetCommandHelp(dstCommand.c_str(), 1);
                if (srcIndexHelp.isSame("index") && dstIndexHelp.isSame("index"))
                    dstCsv.str[1].assign(nCsv.str[1]);
                CSTR dstLine;
                CsvToCSTR(dstCsv, dstLine);
                insertObjectLine(dstLine.outstr());
                break;
            }

            newObjectInsertPosition = -1;
            g_seObjectEditorModel.Rebuild(*this);
            previewReloadPending = true;
            previewReloadRequestedAt = GetTickCount64();
            isCsvInit = false;
            wNewObject = 0;
        }
    }
    ImGui::End();
    return 0;
}

int WORKSPACE::drawObjectManager() {

    char title[260];
    snprintf(title, sizeof(title), "objectManager##%d", num);
    if (ImGui::Begin(title, &wObjectManager)) {

        snprintf(title, sizeof(title), "objList##%d", num);
        if (ImGui::BeginChild(title, { 250,-1 }, ImGuiChildFlags_ResizeX | ImGuiChildFlags_FrameStyle)) {
            for (int i = 0; i < arr_seobj.count; i++) {
                ImGui::PushID(i);
                SEOBJ& seobj = ((SEOBJ*)arr_seobj.data)[i];

                SRC& src = ((SRC*)arr_SRC.data)[((SEOBJ*)arr_seobj.data)[i].src];
                DST& dst = ((DST*)arr_DST.data)[((SEOBJ*)arr_seobj.data)[i].dst];
                

                SKINFILELINEREAD& readS = ((SKINFILELINEREAD*)skinfileLines.data)[src.declare];
                SKINFILELINEREAD& readD = ((SKINFILELINEREAD*)skinfileLines.data)[dst.declare];
                

                char buf[260];
                //if (readS.ifgroup == readD.ifgroup) {
                //    sprintf(buf, "%02d_", readS.ifgroup);
                //    for (int k = 0; k < ((IFUNIT*)arr_ifunit.data)[readS.ifgroup].depth; k++)
                //        sprintf(buf, "%s_", buf);
                //    sprintf(buf, "%s%03d(%s)", buf, seobj.ID, seobj.name.outstr());
                //}
                //else {
                //    sprintf(buf, "%03d(%s)", seobj.ID, seobj.name.outstr());
                //}

                //sprintf(buf, "%s %02d:%02d", buf, src.objType, src.objID);
                sprintf(buf, "%03d(%s)", i, seobj.name.outstr());

                if (ImGui::Selectable(buf, selected_obj == i)) {
                    selected_obj = i;
                }
                ImGui::PopID();
            }

            if (ImGui::BeginPopupContextWindow()) {
                ImGui::Text("selected : %03d", selected_obj);
                ImGui::Separator();
                if (ImGui::MenuItem("Delete")) {
                    arr_seobj.DeleteAt(selected_obj);
                }
                ImGui::Separator();
                if (ImGui::MenuItem("New")) {
                    wNewObject = 1;
                    
                    //SEOBJ* nObj = (SEOBJ*)arr_seobj.Get_newAt(selected_obj);

                    //nObj->name = "newObject";
                    //nObj->body.Alloc(sizeof(CSTR), 2);
                    //nObj->bodyCSV.Alloc(sizeof(CSVbuf), 2);


                    //SplitCSV(&nObj->body[0], &nObj->bodyCSV[0], ",");
                }
                ImGui::EndPopup();
            }
        }
        ImGui::EndChild();
        ImGui::SameLine();

        SEOBJ& seobj = ((SEOBJ*)arr_seobj.data)[selected_obj];
        SRC& src = ((SRC*)arr_SRC.data)[((SEOBJ*)arr_seobj.data)[selected_obj].src];
        DST& dst = ((DST*)arr_DST.data)[((SEOBJ*)arr_seobj.data)[selected_obj].dst];

        SKINFILELINEREAD* srcline = &((SKINFILELINEREAD*)skinfileLines.data)[src.declare];
        SKINFILELINEREAD* dstline = &((SKINFILELINEREAD*)skinfileLines.data)[dst.declare];
            
        snprintf(title, sizeof(title), "objProperty##%d", num);
        if (ImGui::BeginChild(title, { 400,-1 }, ImGuiChildFlags_ResizeX | ImGuiChildFlags_FrameStyle)) {
            ImGui::PushID(selected_obj);
            ImGui::Text(dst.name.outstr());
            
            for (int b = 0; b < seobj.body.count; b++) {
                ImGui::Text("%s", ((CSTR*)seobj.body.data)[b]);
            }
            /*ImGui::Text("%s", srcline->line);
            for (int d = 0; d < dst.arr_animation.count; d++) {
                ImGui::Text("%s", (dstline+d)->line);
                
            }*/
            if (seobj.type2 == 0) {
                if (ImGui::BeginTabBar("MyTabBar", ImGuiTabBarFlags_None))
                {
                    if (ImGui::BeginTabItem("SRC"))
                    {
                        ImGui::SeparatorText("basic");
                        int k = FindIMG(((CSVbuf*)seobj.bodyCSV.data)[0].val[2]
                            , ((CSVbuf*)seobj.bodyCSV.data)[0].val[3]
                            , ((CSVbuf*)seobj.bodyCSV.data)[0].val[4]
                            , ((CSVbuf*)seobj.bodyCSV.data)[0].val[5]
                            , ((CSVbuf*)seobj.bodyCSV.data)[0].val[6]);

                        char buf[64];
                        sprintf(buf, "%d:%s", k, ((IMG*)arr_IMG.data)[k].name);
                        if (ImGui::BeginCombo("##IMGs", buf,ImGuiComboFlags_None)) {
                            for (int n = 0; n < arr_IMG.count; n++) {
                                ImGui::PushID(n);
                                sprintf(buf, "%d:%s", n, ((IMG*)arr_IMG.data)[n].name);
                                if (ImGui::Selectable(buf, n == k, ImGuiSelectableFlags_None, { 0,0 })) {
                                    ((IMG*)arr_IMG.data)[n].gr = ((CSVbuf*)seobj.bodyCSV.data)[0].val[2];
                                    ((IMG*)arr_IMG.data)[n].x = ((CSVbuf*)seobj.bodyCSV.data)[0].val[3];
                                    ((IMG*)arr_IMG.data)[n].y = ((CSVbuf*)seobj.bodyCSV.data)[0].val[4];
                                    ((IMG*)arr_IMG.data)[n].w = ((CSVbuf*)seobj.bodyCSV.data)[0].val[5];
                                    ((IMG*)arr_IMG.data)[n].h = ((CSVbuf*)seobj.bodyCSV.data)[0].val[6];

                                }
                                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNone) && ImGui::BeginTooltip())
                                {
                                    IMG imgg = ((IMG*)arr_IMG.data)[n];
                                    int handle = imgg.gr;

                                    EnsureSRCGRTexture(handle);
                                    SRCGR& img = ((SRCGR*)arr_SRCGR.data)[handle];

                                    int iX = imgg.x;
                                    int iY = imgg.y;
                                    int iW = imgg.w == -1 ? img.sizeX - iX : imgg.w;
                                    int iH = imgg.h == -1 ? img.sizeY - iY : imgg.h;

                                    if (img.texture != NULL) {
                                        ImVec2 display_min = ImVec2(iX / (float)img.sizeX, iY / (float)img.sizeY);
                                        ImVec2 display_max = ImVec2((iX + iW) / (float)img.sizeX, (iY + iH) / (float)img.sizeY);
                                        ImVec2 display_size = ImVec2(iW, iH);

                                        ImGui::Image(img.texture, display_size, display_min, display_max);;
                                    }
                                    //printSrcImg(((SRC*)arr_SRC.data)[handle]);
                                    
                                    
                                    ImGui::EndTooltip();
                                }
                                ImGui::PopID();
                            }
                            ImGui::EndCombo();
                        }
                        CSVbuf* csv = (CSVbuf*)seobj.bodyCSV.data;
                        
                        ImGui::PushItemWidth(45);
                        /*ImGui::InputInt("gr", &csv[0].val[2], 0, 0, ImGuiInputTextFlags_None);
                        ImGui::InputInt("x", &csv[0].val[3], 0, 0, ImGuiInputTextFlags_None);
                        ImGui::InputInt("y", &csv[0].val[4], 0, 0, ImGuiInputTextFlags_None);
                        ImGui::InputInt("w", &csv[0].val[5], 0, 0, ImGuiInputTextFlags_None);
                        ImGui::InputInt("h", &csv[0].val[6], 0, 0, ImGuiInputTextFlags_None);*/
                        CstrInputText("gr", &csv[0].str[2], ImGuiInputTextFlags_CharsDecimal);
                        CstrInputText("x", &csv[0].str[3], ImGuiInputTextFlags_CharsDecimal);
                        CstrInputText("y", &csv[0].str[4], ImGuiInputTextFlags_CharsDecimal);
                        CstrInputText("w", &csv[0].str[5], ImGuiInputTextFlags_CharsDecimal);
                        CstrInputText("h", &csv[0].str[6], ImGuiInputTextFlags_CharsDecimal);
                        

                        ImGui::SeparatorText("animation");
                        /*ImGui::InputInt("div_x", &csv[0].val[7], 0, 0, ImGuiInputTextFlags_None);
                        ImGui::InputInt("div_y", &csv[0].val[8], 0, 0, ImGuiInputTextFlags_None);
                        ImGui::InputInt("cycle", &csv[0].val[9], 0, 0, ImGuiInputTextFlags_None);*/
                        CstrInputText("div_x", &csv[0].str[7], ImGuiInputTextFlags_CharsDecimal);
                        CstrInputText("div_y", &csv[0].str[8], ImGuiInputTextFlags_CharsDecimal);
                        CstrInputText("cycle", &csv[0].str[9], ImGuiInputTextFlags_CharsDecimal);
                        //ImGui::InputText("timer", csv[0].str[10], IM_COUNTOF(csv[0].str[10].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        if (ImGui::BeginCombo("timer",csv[0].str[10], ImGuiComboFlags_None)) {
                            for (int op = 0; op < 200; op++) {
                                ImGui::PushID(op);
                                const bool is_selected = (atol(csv[0].str[10]) == op);
                                char opname[64];
                                sprintf(opname, "%03d:%s", op, timerName(op));

                                if (ImGui::Selectable(opname, is_selected)) {
                                    csv[0].val[10] = op;
                                    ltoa(op, csv[0].str[10],10);
                                    //EditValue(n, column, op);
                                }
                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();

                                ImGui::PopID();
                            }
                            ImGui::EndCombo();
                        }

                        ImGui::PopItemWidth();


                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("DST"))
                    {
                        CSVbuf* csv = (CSVbuf*)seobj.bodyCSV.data;
                        ImGui::PushItemWidth(45);
                        ImGui::SeparatorText("basic");
                        //ImGui::InputText("op1", csv[1].str[18], IM_COUNTOF(csv[1].str[18].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        if (ImGui::BeginCombo("op1", csv[1].str[18], ImGuiComboFlags_None)) {
                            for (int op = 0; op < 200; op++) {
                                ImGui::PushID(op);
                                const bool is_selected = (atol(csv[1].str[18]) == op);
                                char opname[64];
                                sprintf(opname, "%03d:%s", op, dstName(op));

                                if (ImGui::Selectable(opname, is_selected)) {
                                    csv[1].val[18] = op;
                                    ltoa(op, csv[1].str[18], 10);
                                }
                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();

                                ImGui::PopID();
                            }
                            ImGui::EndCombo();
                        }
                        //ImGui::InputText("op2", csv[1].str[19], IM_COUNTOF(csv[1].str[19].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        if (ImGui::BeginCombo("op2", csv[1].str[19], ImGuiComboFlags_None)) {
                            for (int op = 0; op < 200; op++) {
                                ImGui::PushID(op);
                                const bool is_selected = (atol(csv[1].str[19]) == op);
                                char opname[64];
                                sprintf(opname, "%03d:%s", op, dstName(op));

                                if (ImGui::Selectable(opname, is_selected)) {
                                    csv[1].val[19] = op;
                                    ltoa(op, csv[1].str[19], 10);
                                }
                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();

                                ImGui::PopID();
                            }
                            ImGui::EndCombo();
                        }
                        //ImGui::InputText("op3", csv[1].str[20], IM_COUNTOF(csv[1].str[20].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        if (ImGui::BeginCombo("op3", csv[1].str[20], ImGuiComboFlags_None)) {
                            for (int op = 0; op < 200; op++) {
                                ImGui::PushID(op);
                                const bool is_selected = (atol(csv[1].str[20]) == op);
                                char opname[64];
                                sprintf(opname, "%03d:%s", op, dstName(op));

                                if (ImGui::Selectable(opname, is_selected)) {
                                    csv[1].val[20] = op;
                                    ltoa(op, csv[1].str[20], 10);
                                }
                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();

                                ImGui::PopID();
                            }
                            ImGui::EndCombo();
                        }
                        //ImGui::InputText("timer", csv[1].str[17], IM_COUNTOF(csv[1].str[17].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        if (ImGui::BeginCombo("timer", csv[1].str[17], ImGuiComboFlags_None)) {
                            for (int op = 0; op < 200; op++) {
                                ImGui::PushID(op);
                                const bool is_selected = (atol(csv[1].str[17]) == op);
                                char opname[64];
                                sprintf(opname, "%03d:%s", op, timerName(op));

                                if (ImGui::Selectable(opname, is_selected)) {
                                    csv[1].val[17] = op;
                                    ltoa(op, csv[1].str[17], 10);
                                }
                                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                                if (is_selected)
                                    ImGui::SetItemDefaultFocus();

                                ImGui::PopID();
                            }
                            ImGui::EndCombo();
                        }
                        //ImGui::InputText("loop", csv[1].str[16], IM_COUNTOF(csv[1].str[16].outstr()), ImGuiInputTextFlags_CharsDecimal);
                        //ImGui::InputInt("loop", &csv[1].val[16],0,0);
                        CstrInputText("loop", &csv[1].str[16], ImGuiInputTextFlags_CharsDecimal);

                        ImGui::SeparatorText("animation");
                        if(ImGui::Button("+")) {
                            CSVbuf* csvN = (CSVbuf*)seobj.bodyCSV.Get_new();
                            CSTR* bodyN = (CSTR*)seobj.body.Get_new();
                            bodyN->assign(((CSTR*)seobj.body.data)[seobj.body.count - 2]);
                            memset(csvN, 0, sizeof(CSVbuf));
                            for (int i = 0; i < 30; i++) {
                                csvN->str[i].assign("");
                            }
                            SplitCSV(bodyN->body, csvN, ",");
                            CSVatol(csvN);
                            CSVltoa (csvN,10);
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("-")) {
                            if (seobj.bodyCSV.count > 2) {
                                seobj.bodyCSV.DeleteAt(seobj.bodyCSV.count-1);
                                seobj.body.DeleteAt(seobj.bodyCSV.count - 1);
                            }
                        }
                        ImGui::BeginGroup();
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("time");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("x");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("y");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("w");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("h");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("acc");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("a");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("r");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("g");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("b");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("blend");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("filter");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("angle");
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("center");
                        ImGui::EndGroup();
                        for (int i = 1; i < seobj.bodyCSV.count; i++) {
                            ImGui::SameLine();

                            ImGui::PushID(i);
                            ImGui::BeginGroup();
                            ImGui::InputInt("##time", &csv[i].val[2],0,0,ImGuiInputTextFlags_None);
                            ImGui::InputInt("##x", &csv[i].val[3], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##y", &csv[i].val[4], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##w", &csv[i].val[5], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##h", &csv[i].val[6], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##acc", &csv[i].val[7], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##a", &csv[i].val[8], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##r", &csv[i].val[9], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##g", &csv[i].val[10], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##b", &csv[i].val[11], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##blend", &csv[i].val[12], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##filter", &csv[i].val[13], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##angle", &csv[i].val[14], 0, 0, ImGuiInputTextFlags_None);
                            ImGui::InputInt("##center", &csv[i].val[15], 0, 0, ImGuiInputTextFlags_None);
                            //CSVltoa(&csv[i],10);
                            ImGui::EndGroup();
                            ImGui::PopID();
                        }
                        ImGui::PopItemWidth();
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            }
            else if (seobj.type2 == -1) {


            }

            ImGui::PopID();
        }
        ImGui::EndChild();



    }
    ImGui::End();

    return 0;
}

int WORKSPACE::drawProperty() {

    char title[260];
    snprintf(title, sizeof(title), "objectProperty##%d", num);
    if (ImGui::Begin(title, &wProperty)) {
        selectedObjectTest;

    }
    ImGui::End();
    return 0;
}


int WORKSPACE::drawOpList() {

    char title[260];
    snprintf(title, sizeof(title), "OpList##%d", num);
    if (ImGui::Begin(title, &wOpList)) {
        //init ops
        for (int i = 0; i < 1000; i++) {
            op[i] = GetOptionFlag_dst(&g, i);
        }
        for (int i = 0; i < 1000; i++) {
            ImGui::PushID(i);
            ImGui::Text("%03d", i);
            ImGui::SameLine();
            ImGui::Checkbox(dstName(i), &op[i]);
            ImGui::PopID();
        }
    }
    ImGui::End();
    return 0;
}

int WORKSPACE::drawHistory() {


    ImGui::PushID(num);
    if (ImGui::Begin("History", &wHistory)) {

        int item_selected_idx = 0;
    
        for (int n = 0; n < arr_history.count; n++){
            ImGui::PushID(n);
            const bool is_selected = (item_selected_idx == n);
            char itemname[260];
            
            HISTORY& hs = ((HISTORY*)arr_history.data)[n];
            sprintf(itemname, "%d %d\n%s \n%s", hs.op, hs.target, hs.older.line.outstr(), hs.newer.line.outstr());
            if (ImGui::Selectable(itemname, is_selected)) {
                item_selected_idx = n;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();

            ImGui::PopID();
        }

    }
    ImGui::End();
    ImGui::PopID();

    return 0;
}



// 
// [file ---> (line, csv) ---> LR2]
// file ---> line, csv ---> current SE
// csv -> line -> file

// file -> csv -> file : this is textedit
// we need objedit
// csv -> obj(multi line) -> csv


//utf-8 shift-jis problem




ARR workspaceList;
// Data-driven Object Editor.  The schema comes from skinObjGroup.txt and
// argument names from skinHelper.txt.  The editor works directly on
// skinfileLines so the existing CSV/table/save path remains authoritative.
int WORKSPACE::drawObjectEditor() {
    char title[128];
    snprintf(title, sizeof(title), "Object Editor##%d", num);
    if (!ImGui::Begin(title, &wObjectEditor)) { ImGui::End(); return 0; }

    if (g_seObjectEditorModel.Groups().empty()) {
        ImGui::Text("skinObjGroup.txt not loaded.");
        ImGui::End();
        return 0;
    }
    if (!loaded || skinfileLines.count <= 0) {
        ImGui::Text("No skin loaded.");
        ImGui::End();
        return 0;
    }

    static WORKSPACE* lastWs = NULL;
    static int lastLineCount = -1;
    if (lastWs != this || lastLineCount != skinfileLines.count) {
        g_seObjectEditorModel.Rebuild(*this);
        lastWs = this;
        lastLineCount = skinfileLines.count;
        selected_object_editor = 0;
    }

    const std::vector<SEObjectGroupDef>& groups = g_seObjectEditorModel.Groups();
    if (selected_object_group >= (int)groups.size()) selected_object_group = -1;

    auto formatConditionHeader = [&](SKINFILELINEREAD& row) -> std::string {
        std::string result = row.csv.str[0].body ? row.csv.str[0].outstr() : "#IF";
        const char* command = row.csv.str[0].body ? row.csv.str[0].outstr() : "";
        if (strcmp(command, "#IF") != 0 && strcmp(command, "#ELSEIF") != 0) return result;
        for (int col = 1; col < 30; ++col) {
            if (!row.csv.str[col].body || row.csv.str[col].length() == 0) continue;
            const int op = row.csv.val[col];
            const int optionId = op < 0 ? -op : op;
            const char* optionName = dstName(optionId);
            char option[160];
            if (op < 0)
                snprintf(option, sizeof(option), " !%03d:%s", optionId, optionName ? optionName : "");
            else
                snprintf(option, sizeof(option), " %03d:%s", optionId, optionName ? optionName : "");
            result += option;
        }
        return result;
    };

    static char objectSearch[128] = "";
    static bool requestCreateGroupPopup = false;
    static float objectListWidth = 260.0f;
    const float objectEditorPaneHeight = ImGui::GetContentRegionAvail().y;

    // Left navigator: filters on top, condition/object list below.
    ImGui::BeginChild("ObjectNavigator", ImVec2(objectListWidth, 0), false);
    if (ImGui::BeginChild("ObjectFilters", ImVec2(0, 88), true)) {
        ImGui::TextDisabled("Type");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        const char* typePreview = selected_user_object_group < 0 && selected_object_group >= 0
            ? groups[selected_object_group].name.c_str() : "All Objects";
        if (ImGui::BeginCombo("##ObjectTypeFilter", typePreview)) {
            if (ImGui::Selectable("All Objects", selected_user_object_group < 0 && selected_object_group < 0)) {
                selected_object_group = -1;
                selected_user_object_group = -1;
                selected_object_editor = 0;
            }
            for (int gidx = 0; gidx < (int)groups.size(); ++gidx) {
                std::vector<int> objs = g_seObjectEditorModel.ObjectsForGroup(gidx);
                if (objs.empty()) continue;
                char label[128];
                snprintf(label, sizeof(label), "%s (%d)", groups[gidx].name.c_str(), (int)objs.size());
                if (ImGui::Selectable(label, selected_user_object_group < 0 && selected_object_group == gidx)) {
                    selected_object_group = gidx;
                    selected_user_object_group = -1;
                    selected_object_editor = 0;
                }
            }
            ImGui::EndCombo();
        }

        const std::vector<SEUserObjectGroup>& userGroups = g_seObjectEditorModel.UserGroups();
        ImGui::TextDisabled("Group");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        const std::string groupPreviewUtf8 = selected_user_object_group >= 0 &&
            selected_user_object_group < (int)userGroups.size()
            ? Cp932ToUtf8(userGroups[selected_user_object_group].name.c_str()) : "No My Group";
        if (ImGui::BeginCombo("##ObjectUserGroupFilter", groupPreviewUtf8.c_str())) {
            if (ImGui::Selectable("No My Group", selected_user_object_group < 0))
                selected_user_object_group = -1;
            for (int gidx = 0; gidx < (int)userGroups.size(); ++gidx) {
                std::vector<int> members = g_seObjectEditorModel.ObjectsForUserGroup(gidx);
                char label[160];
                const std::string nameUtf8 = Cp932ToUtf8(userGroups[gidx].name.c_str());
                snprintf(label, sizeof(label), "%s (%d)##user%d", nameUtf8.c_str(),
                    (int)members.size(), gidx);
                if (ImGui::Selectable(label, selected_user_object_group == gidx)) {
                    selected_user_object_group = gidx;
                    selected_object_editor = 0;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputTextWithHint("##ObjectSearch", "Search objects...", objectSearch, sizeof(objectSearch));

        if (requestCreateGroupPopup) {
            ImGui::OpenPopup("Create Object Group");
            requestCreateGroupPopup = false;
        }
        if (ImGui::BeginPopupModal("Create Object Group", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            static char newGroupName[128] = "New Group";
            ImGui::InputText("Group name", newGroupName, sizeof(newGroupName));
            if (ImGui::Button("Create")) {
                struct PendingId { int row; int sourceNum; std::string owner; std::string id; };
                std::vector<PendingId> pendingIds;
                std::vector<std::string> memberIds;
                const std::vector<SEObjectInstance>& allObjects = g_seObjectEditorModel.Objects();
                const unsigned long long stamp = GetTickCount64();
                for (int modelIndex : preview_selected_object_model_indices) {
                    if (modelIndex < 0 || modelIndex >= (int)allObjects.size()) continue;
                    const SEObjectInstance& object = allObjects[modelIndex];
                    std::string id = object.editorId;
                    if (id.empty() && !object.rows.empty()) {
                        char generated[64];
                        snprintf(generated, sizeof(generated), "obj_%08llX_%04d", stamp, modelIndex);
                        id = generated;
                        const int firstRow = object.rows.front();
                        SKINFILELINEREAD& source = ((SKINFILELINEREAD*)skinfileLines.data)[firstRow];
                        PendingId pending;
                        pending.row = firstRow;
                        pending.sourceNum = source.num;
                        pending.owner = source.filename.body ? source.filename.outstr() : mainpath;
                        pending.id = id;
                        pendingIds.push_back(pending);
                    }
                    if (!id.empty()) memberIds.push_back(id);
                }

                std::sort(pendingIds.begin(), pendingIds.end(), [](const PendingId& a, const PendingId& b) {
                    return a.row > b.row;
                });
                auto insertMetaLine = [&](int position, const char* text, const char* owner, int sourceNum) {
                    InsertLine(position);
                    SKINFILELINEREAD& inserted = ((SKINFILELINEREAD*)skinfileLines.data)[position];
                    CSTR oldLine(inserted.line);
                    EditLine(position, oldLine, CSTR(text));
                    inserted.filename.assign(owner && *owner ? owner : mainpath);
                    inserted.num = sourceNum;
                    inserted.numTotal = position;
                };
                for (const PendingId& pending : pendingIds) {
                    std::string line = "$SE_OBJECT_ID," + pending.id;
                    insertMetaLine(pending.row, line.c_str(), pending.owner.c_str(), pending.sourceNum);
                }

                const int groupNumber = (int)userGroups.size();
                std::string begin = std::string("$SE_GROUP_BEGIN,") +
                    (newGroupName[0] ? newGroupName : "New Group");
                insertMetaLine(skinfileLines.count, begin.c_str(), mainpath, skinfileLines.count);
                for (const std::string& id : memberIds) {
                    std::string member = "$SE_GROUP_MEMBER," + id;
                    insertMetaLine(skinfileLines.count, member.c_str(), mainpath, skinfileLines.count);
                }
                insertMetaLine(skinfileLines.count, "$SE_GROUP_END", mainpath, skinfileLines.count);

                g_seObjectEditorModel.Rebuild(*this);
                selected_user_object_group = groupNumber;
                selected_object_editor = 0;
                preview_selected_object_model_indices.clear();
                const std::vector<SEObjectInstance>& rebuiltObjects = g_seObjectEditorModel.Objects();
                for (int objectIndex = 0; objectIndex < (int)rebuiltObjects.size(); ++objectIndex) {
                    if (std::find(memberIds.begin(), memberIds.end(), rebuiltObjects[objectIndex].editorId) != memberIds.end())
                        preview_selected_object_model_indices.push_back(objectIndex);
                }
                preview_selected_object_model_index = preview_selected_object_model_indices.empty()
                    ? -1 : preview_selected_object_model_indices.front();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        ImGui::EndChild();
    }

    std::vector<int> groupObjects;
    if (selected_user_object_group >= 0)
        groupObjects = g_seObjectEditorModel.ObjectsForUserGroup(selected_user_object_group);
    else if (selected_object_group >= 0)
        groupObjects = g_seObjectEditorModel.ObjectsForGroup(selected_object_group);
    else {
        const std::vector<SEObjectInstance>& allObjects = g_seObjectEditorModel.Objects();
        for (int i = 0; i < (int)allObjects.size(); ++i) groupObjects.push_back(i);
    }

    if (objectSearch[0]) {
        auto containsText = [](const char* text, const char* query) {
            if (!text || !query || !*query) return true;
            const size_t queryLength = strlen(query);
            for (const char* cursor = text; *cursor; ++cursor)
                if (_strnicmp(cursor, query, queryLength) == 0) return true;
            return false;
        };
        const std::vector<SEObjectInstance>& allObjects = g_seObjectEditorModel.Objects();
        groupObjects.erase(std::remove_if(groupObjects.begin(), groupObjects.end(), [&](int objectIndex) {
            if (objectIndex < 0 || objectIndex >= (int)allObjects.size()) return true;
            const SEObjectInstance& object = allObjects[objectIndex];
            if (containsText(object.name.c_str(), objectSearch) ||
                containsText(object.editorId.c_str(), objectSearch)) return false;
            const SEObjectGroupDef* def = g_seObjectEditorModel.Group(object.group);
            if (def && containsText(def->name.c_str(), objectSearch)) return false;
            for (int row : object.rows) {
                if (row < 0 || row >= skinfileLines.count) continue;
                SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                if (line.line.body && containsText(line.line.outstr(), objectSearch)) return false;
            }
            return true;
        }), groupObjects.end());
    }
    int contextCreateModel = -1;
    int contextDeleteModel = -1;

    // Middle: control-flow is metadata, never an Object command.
    // Sibling #IF/#ELSEIF/#ELSE branches are displayed together.
    if (ImGui::BeginChild("ObjectList", ImVec2(0, 0), true)) {
        struct BranchBlock {
            int ifgroup;
            int order;
            std::string label;
            std::vector<int> localObjectIndices;
        };
        struct ConditionBlock {
            int rootIfgroup;
            int depth;
            std::string label;
            std::vector<BranchBlock> branches;
        };
        std::vector<ConditionBlock> conditions;

        auto getHeader = [&](int ifgroup, std::string& label, int& order) -> bool {
            label = "";
            order = 0;
            if (ifgroup <= 0 || ifgroup >= arr_ifunit.count) return false;
            IFUNIT& unit = ((IFUNIT*)arr_ifunit.data)[ifgroup];
            order = unit.order;
            for (int rno = 0; rno < skinfileLines.count; ++rno) {
                SKINFILELINEREAD& r = ((SKINFILELINEREAD*)skinfileLines.data)[rno];
                if (r.ifgroup != ifgroup || !r.isIfGroupHead || !r.csv.str[0].body) continue;
                label = formatConditionHeader(r);
                return true;
            }
            return false;
        };

        auto getRootIfgroup = [&](int ifgroup) -> int {
            if (ifgroup <= 0 || ifgroup >= arr_ifunit.count) return 0;
            IFUNIT& u = ((IFUNIT*)arr_ifunit.data)[ifgroup];
            if (u.order == 0) return ifgroup;
            int root = ifgroup;
            for (int j = ifgroup - 1; j > 0; --j) {
                IFUNIT& c = ((IFUNIT*)arr_ifunit.data)[j];
                if (c.depth == u.depth && c.parentID == u.parentID && c.order == 0) {
                    root = j;
                    break;
                }
            }
            return root;
        };

        // Cache the exact active branch state. A branch is active only when
        // its parent is active, its own IF/ELSEIF matches, and no earlier
        // sibling branch has already matched.
        static WORKSPACE* statusCacheWorkspace = nullptr;
        static int statusCacheLineCount = -1;
        static int statusCacheIfCount = -1;
        static unsigned long long statusCacheAt = 0;
        static std::vector<bool> branchActive;
        const unsigned long long statusNow = GetTickCount64();
        const bool refreshBranchStatus = statusCacheWorkspace != this ||
            statusCacheLineCount != skinfileLines.count || statusCacheIfCount != arr_ifunit.count ||
            statusNow - statusCacheAt >= 200;
        if (refreshBranchStatus) {
            std::vector<bool> branchMatches(arr_ifunit.count, true);
            branchActive.assign(arr_ifunit.count, false);
            if (!branchActive.empty()) branchActive[0] = true;
            for (int row = 0; row < skinfileLines.count; ++row) {
                SKINFILELINEREAD& header = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                if (!header.isIfGroupHead || header.ifgroup <= 0 ||
                    header.ifgroup >= arr_ifunit.count || !header.csv.str[0].body) continue;
                const char* command = header.csv.str[0].outstr();
                if (!strcmp(command, "#ELSE")) {
                    branchMatches[header.ifgroup] = true;
                } else {
                    bool matches = !strcmp(command, "#IF") || !strcmp(command, "#ELSEIF");
                    for (int col = 1; matches && col <= 10; ++col) {
                        const int option = header.csv.val[col];
                        const int optionId = option < 0 ? -option : option;
                        if (optionId > 999) {
                            matches = false;
                        } else {
                            const bool optionEnabled = optionId == 0 || g.skstruct.op[optionId] != 0;
                            if ((option >= 0) != optionEnabled) matches = false;
                        }
                    }
                    branchMatches[header.ifgroup] = matches;
                }
            }
            for (int group = 1; group < arr_ifunit.count; ++group) {
                IFUNIT& unit = ((IFUNIT*)arr_ifunit.data)[group];
                const int chainRoot = getRootIfgroup(group);
                bool earlierTaken = false;
                for (int sibling = 1; sibling < group; ++sibling) {
                    IFUNIT& previous = ((IFUNIT*)arr_ifunit.data)[sibling];
                    if (previous.parentID == unit.parentID && previous.depth == unit.depth &&
                        previous.order < unit.order && getRootIfgroup(sibling) == chainRoot &&
                        branchMatches[sibling]) {
                        earlierTaken = true;
                        break;
                    }
                }
                const bool parentActive = unit.parentID >= 0 && unit.parentID < (int)branchActive.size()
                    ? branchActive[unit.parentID] : false;
                branchActive[group] = parentActive && !earlierTaken && branchMatches[group];
            }
            statusCacheWorkspace = this;
            statusCacheLineCount = skinfileLines.count;
            statusCacheIfCount = arr_ifunit.count;
            statusCacheAt = statusNow;
        }

        // Command metadata never changes while the helper definition is
        // loaded. Cache the $op column positions instead of linearly searching
        // the whole help table for every DST, every UI frame.
        static std::map<std::string, std::vector<int> > dstOptionColumns;
        auto optionColumnsFor = [&](const char* command) -> const std::vector<int>& {
            const std::string key = command ? command : "";
            std::map<std::string, std::vector<int> >::iterator found = dstOptionColumns.find(key);
            if (found != dstOptionColumns.end()) return found->second;
            std::vector<int> columns;
            for (int col = 1; col < 30; ++col) {
                CSTR optionHelp = GetCommandHelp(key.c_str(), col);
                optionHelp.trimWhiteSpace();
                if (optionHelp.left(3).isSame("$op")) columns.push_back(col);
            }
            return dstOptionColumns.insert(std::make_pair(key, columns)).first->second;
        };

        for (int oi = 0; oi < (int)groupObjects.size(); ++oi) {
            const SEObjectInstance& o = g_seObjectEditorModel.Objects()[groupObjects[oi]];
            int ifgroup = o.ifgroup;
            if (ifgroup == 0 && !o.rows.empty())
                ifgroup = ((SKINFILELINEREAD*)skinfileLines.data)[o.rows[0]].ifgroup;

            if (ifgroup == 0) {
                int ci = -1;
                for (int i = 0; i < (int)conditions.size(); ++i)
                    if (conditions[i].rootIfgroup == 0) { ci = i; break; }
                if (ci < 0) {
                    ConditionBlock c;
                    c.rootIfgroup = 0; c.depth = 0; c.label = "ALWAYS";
                    conditions.push_back(c); ci = (int)conditions.size() - 1;
                }
                if (conditions[ci].branches.empty()) {
                    BranchBlock b; b.ifgroup = 0; b.order = 0; b.label = "ALWAYS";
                    conditions[ci].branches.push_back(b);
                }
                conditions[ci].branches[0].localObjectIndices.push_back(oi);
                continue;
            }

            int root = getRootIfgroup(ifgroup);
            int ci = -1;
            for (int i = 0; i < (int)conditions.size(); ++i)
                if (conditions[i].rootIfgroup == root) { ci = i; break; }
            if (ci < 0) {
                ConditionBlock c;
                c.rootIfgroup = root;
                c.depth = ((IFUNIT*)arr_ifunit.data)[root].depth;
                int dummyOrder = 0;
                getHeader(root, c.label, dummyOrder);
                if (c.label.empty()) c.label = "#IF";
                conditions.push_back(c); ci = (int)conditions.size() - 1;
            }

            int bi = -1;
            for (int i = 0; i < (int)conditions[ci].branches.size(); ++i)
                if (conditions[ci].branches[i].ifgroup == ifgroup) { bi = i; break; }
            if (bi < 0) {
                BranchBlock b;
                b.ifgroup = ifgroup;
                getHeader(ifgroup, b.label, b.order);
                if (b.label.empty()) b.label = (b.order == 0) ? "#IF" : "#ELSE";
                conditions[ci].branches.push_back(b); bi = (int)conditions[ci].branches.size() - 1;
            }
            conditions[ci].branches[bi].localObjectIndices.push_back(oi);
        }

        // A condition containing only nested conditions has no direct object,
        // so the object-driven pass above cannot discover it. Add every
        // missing ancestor chain to keep the navigator rooted at the actual
        // top-level #IF instead of starting midway through the source tree.
        std::vector<int> visibleConditionRoots;
        for (const ConditionBlock& condition : conditions)
            if (condition.rootIfgroup > 0) visibleConditionRoots.push_back(condition.rootIfgroup);
        for (int visibleRoot : visibleConditionRoots) {
            int ancestorBranch = visibleRoot > 0 && visibleRoot < arr_ifunit.count
                ? ((IFUNIT*)arr_ifunit.data)[visibleRoot].parentID : 0;
            while (ancestorBranch > 0 && ancestorBranch < arr_ifunit.count) {
                const int ancestorRoot = getRootIfgroup(ancestorBranch);
                bool alreadyPresent = false;
                for (const ConditionBlock& condition : conditions) {
                    if (condition.rootIfgroup == ancestorRoot) {
                        alreadyPresent = true;
                        break;
                    }
                }
                if (!alreadyPresent) {
                    ConditionBlock ancestor;
                    ancestor.rootIfgroup = ancestorRoot;
                    ancestor.depth = ((IFUNIT*)arr_ifunit.data)[ancestorRoot].depth;
                    int dummyOrder = 0;
                    getHeader(ancestorRoot, ancestor.label, dummyOrder);
                    if (ancestor.label.empty()) ancestor.label = "#IF";
                    for (int group = ancestorRoot; group < arr_ifunit.count; ++group) {
                        if (getRootIfgroup(group) != ancestorRoot) continue;
                        BranchBlock branch;
                        branch.ifgroup = group;
                        getHeader(group, branch.label, branch.order);
                        if (branch.label.empty()) branch.label = branch.order == 0 ? "#IF" : "#ELSE";
                        ancestor.branches.push_back(branch);
                    }
                    conditions.push_back(ancestor);
                }
                ancestorBranch = ((IFUNIT*)arr_ifunit.data)[ancestorRoot].parentID;
            }
        }
        std::stable_sort(conditions.begin(), conditions.end(), [](const ConditionBlock& a, const ConditionBlock& b) {
            if (a.rootIfgroup == b.rootIfgroup) return false;
            if (a.rootIfgroup == 0) return true;
            if (b.rootIfgroup == 0) return false;
            return a.rootIfgroup < b.rootIfgroup;
        });

        static WORKSPACE* branchTreeWorkspace = nullptr;
        static int branchTreeIfCount = -1;
        static std::map<int, bool> branchTreeOpen;
        if (branchTreeWorkspace != this || branchTreeIfCount != arr_ifunit.count) {
            branchTreeOpen.clear();
            branchTreeWorkspace = this;
            branchTreeIfCount = arr_ifunit.count;
        }

        for (int ci = 0; ci < (int)conditions.size(); ++ci) {
            ConditionBlock& cond = conditions[ci];
            // Conditions are stored in source order, but visually form a
            // tree. Do not draw a nested condition when its owning branch is
            // collapsed; this makes folding a parent hide the whole subtree.
            if (cond.rootIfgroup > 0 && cond.rootIfgroup < arr_ifunit.count) {
                bool ancestorCollapsed = false;
                int parentBranch = ((IFUNIT*)arr_ifunit.data)[cond.rootIfgroup].parentID;
                while (parentBranch > 0 && parentBranch < arr_ifunit.count) {
                    std::map<int, bool>::const_iterator parentOpen = branchTreeOpen.find(parentBranch);
                    if (parentOpen != branchTreeOpen.end() && !parentOpen->second) {
                        ancestorCollapsed = true;
                        break;
                    }
                    const int parentRoot = getRootIfgroup(parentBranch);
                    parentBranch = parentRoot > 0 && parentRoot < arr_ifunit.count
                        ? ((IFUNIT*)arr_ifunit.data)[parentRoot].parentID : 0;
                }
                if (ancestorCollapsed) continue;
            }
            const float conditionIndent = cond.depth > 1
                ? (float)(cond.depth - 1) * ImGui::GetStyle().IndentSpacing : 0.0f;
            if (conditionIndent > 0.0f) ImGui::Indent(conditionIndent);
            ImGui::PushID(cond.rootIfgroup);

            auto drawBranchObjects = [&](BranchBlock& branch) {
                for (int k = 0; k < (int)branch.localObjectIndices.size(); ++k) {
                    int oi = branch.localObjectIndices[k];
                    const SEObjectInstance& o = g_seObjectEditorModel.Objects()[groupObjects[oi]];
                    char label[256];
                    int firstRow = o.rows.empty() ? -1 : o.rows[0];
                    int key = -1;
                    if (firstRow >= 0) {
                        SKINFILELINEREAD& r = ((SKINFILELINEREAD*)skinfileLines.data)[firstRow];
                        if (r.csv.str[1].body) key = r.csv.val[1];
                    }
                    std::vector<int> labelOps;
                    for (int rowIndex = 0; rowIndex < (int)o.rows.size() && labelOps.size() < 2; ++rowIndex) {
                        const int row = o.rows[rowIndex];
                        if (row < 0 || row >= skinfileLines.count) continue;
                        SKINFILELINEREAD& opRow = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                        const char* opCommand = opRow.csv.str[0].body ? opRow.csv.str[0].outstr() : "";
                        const std::vector<int>& labelOptionColumns = optionColumnsFor(opCommand);
                        for (int col : labelOptionColumns) {
                            if (labelOps.size() >= 2) break;
                            const int op = opRow.csv.val[col];
                            if (op == 0) continue;
                            bool duplicate = false;
                            for (std::size_t existing = 0; existing < labelOps.size(); ++existing)
                                if (labelOps[existing] == op) { duplicate = true; break; }
                            if (!duplicate) labelOps.push_back(op);
                        }
                    }

                    std::string approximateName;
                    if (!o.name.empty()) {
                        approximateName = Cp932ToUtf8(o.name.c_str());
                    } else {
                        for (std::size_t opIndex = 0; opIndex < labelOps.size(); ++opIndex) {
                            const int op = labelOps[opIndex];
                            const int optionId = op < 0 ? -op : op;
                            const char* optionName = dstName(optionId);
                            if (!optionName || !*optionName) continue;
                            if (!approximateName.empty()) approximateName += " / ";
                            if (op < 0) approximateName += "!";
                            approximateName += optionName;
                        }
                    }

                    const SEObjectGroupDef* objectTypeDef = g_seObjectEditorModel.Group(o.group);
                    const char* objectTypeName = objectTypeDef ? objectTypeDef->name.c_str() : "OBJECT";
                    if (key >= 0 && !approximateName.empty())
                        snprintf(label, sizeof(label), "%03d  [%s]  #%d  %s", oi, objectTypeName, key, approximateName.c_str());
                    else if (key >= 0)
                        snprintf(label, sizeof(label), "%03d  [%s]  #%d", oi, objectTypeName, key);
                    else if (!approximateName.empty())
                        snprintf(label, sizeof(label), "%03d  [%s]  %s", oi, objectTypeName, approximateName.c_str());
                    else
                        snprintf(label, sizeof(label), "%03d  [%s]", oi, objectTypeName);
                    bool hasDst = false;
                    bool allDstEnabled = true;
                    for (int rowIndex = 0; rowIndex < (int)o.rows.size(); ++rowIndex) {
                        int row = o.rows[rowIndex];
                        if (row < 0 || row >= skinfileLines.count) continue;
                        SKINFILELINEREAD& dstRow = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                        if (!dstRow.csv.str[0].body) continue;
                        const char* command = dstRow.csv.str[0].outstr();
                        if (strncmp(command, "#DST", 4) != 0) continue;
                        hasDst = true;
                        bool dstEnabled = true;
                        const std::vector<int>& optionColumns = optionColumnsFor(command);
                        for (int col : optionColumns) {
                            if (!GetOptionFlag_dst(&g, dstRow.csv.val[col])) {
                                dstEnabled = false;
                                break;
                            }
                        }
                        if (!dstEnabled) allDstEnabled = false;
                    }

                    const bool conditionActive = o.ifgroup <= 0 ||
                        (o.ifgroup < (int)branchActive.size() && branchActive[o.ifgroup]);
                    const bool objectEnabled = hasDst && allDstEnabled && conditionActive;

                    ImGui::PushID(oi);
                    if (hasDst) {
                        const ImVec4 rowColor = objectEnabled
                            ? ImVec4(0.18f, 0.48f, 0.23f, 0.62f)
                            : ImVec4(0.55f, 0.18f, 0.18f, 0.62f);
                        const ImVec4 hoverColor = objectEnabled
                            ? ImVec4(0.24f, 0.62f, 0.30f, 0.78f)
                            : ImVec4(0.68f, 0.22f, 0.22f, 0.78f);
                        const ImVec2 rowMin = ImGui::GetCursorScreenPos();
                        const float rowWidth = ImGui::GetContentRegionAvail().x;
                        ImGui::GetWindowDrawList()->AddRectFilled(rowMin,
                            ImVec2(rowMin.x + rowWidth, rowMin.y + ImGui::GetFrameHeight()),
                            ImGui::GetColorU32(rowColor));
                        ImGui::PushStyleColor(ImGuiCol_Header, rowColor);
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, hoverColor);
                        ImGui::PushStyleColor(ImGuiCol_HeaderActive, hoverColor);
                    }
                    const int modelIndex = groupObjects[oi];
                    const bool isMultiSelected = std::find(preview_selected_object_model_indices.begin(),
                        preview_selected_object_model_indices.end(), modelIndex) !=
                        preview_selected_object_model_indices.end();
                    const bool clicked = ImGui::Selectable(label, isMultiSelected);
                    if (hasDst) ImGui::PopStyleColor(3);
                    if (clicked) {
                        selected_object_editor = oi;
                        preview_selected_object_model_index = modelIndex;
                        preview_object_dragging = false;
                        preview_selected_obj_valid = false;
                        preview_selected_obj_last_valid = false;

                        if (ImGui::GetIO().KeyShift && preview_selection_anchor_model_index >= 0) {
                            std::vector<int>::const_iterator anchorIt = std::find(groupObjects.begin(), groupObjects.end(),
                                preview_selection_anchor_model_index);
                            std::vector<int>::const_iterator clickedIt = std::find(groupObjects.begin(), groupObjects.end(), modelIndex);
                            if (anchorIt != groupObjects.end() && clickedIt != groupObjects.end()) {
                                int anchorPosition = (int)(anchorIt - groupObjects.begin());
                                int clickedPosition = (int)(clickedIt - groupObjects.begin());
                                if (anchorPosition > clickedPosition) std::swap(anchorPosition, clickedPosition);
                                if (!ImGui::GetIO().KeyCtrl) preview_selected_object_model_indices.clear();
                                for (int rangePosition = anchorPosition; rangePosition <= clickedPosition; ++rangePosition) {
                                    const int rangeModelIndex = groupObjects[rangePosition];
                                    if (std::find(preview_selected_object_model_indices.begin(),
                                        preview_selected_object_model_indices.end(), rangeModelIndex) ==
                                        preview_selected_object_model_indices.end())
                                        preview_selected_object_model_indices.push_back(rangeModelIndex);
                                }
                            } else {
                                preview_selected_object_model_indices.clear();
                                preview_selected_object_model_indices.push_back(modelIndex);
                                preview_selection_anchor_model_index = modelIndex;
                            }
                        } else if (ImGui::GetIO().KeyCtrl) {
                            std::vector<int>::iterator selectedIt = std::find(
                                preview_selected_object_model_indices.begin(),
                                preview_selected_object_model_indices.end(), modelIndex);
                            if (selectedIt == preview_selected_object_model_indices.end())
                                preview_selected_object_model_indices.push_back(modelIndex);
                            else
                                preview_selected_object_model_indices.erase(selectedIt);
                            preview_selection_anchor_model_index = modelIndex;
                        } else {
                            preview_selected_object_model_indices.clear();
                            preview_selected_object_model_indices.push_back(modelIndex);
                            preview_selection_anchor_model_index = modelIndex;
                        }

                        bool firstBounds = true;
                        bool lastBounds = true;
                        float minX = 0.0f, minY = 0.0f, maxX = 0.0f, maxY = 0.0f;
                        float lastMinX = 0.0f, lastMinY = 0.0f, lastMaxX = 0.0f, lastMaxY = 0.0f;
                        for (int selectedModelIndex : preview_selected_object_model_indices) {
                            if (selectedModelIndex < 0 || selectedModelIndex >=
                                (int)g_seObjectEditorModel.Objects().size()) continue;
                            const SEObjectInstance& selectedObject =
                                g_seObjectEditorModel.Objects()[selectedModelIndex];
                            int textAlign = 0;
                            bool isTextObject = false;
                            int numberAlign = 0;
                            bool isNumberObject = false;
                            for (int rowIndex = 0; rowIndex < (int)selectedObject.rows.size(); ++rowIndex) {
                                const int sourceRow = selectedObject.rows[rowIndex];
                                if (sourceRow < 0 || sourceRow >= skinfileLines.count) continue;
                                SKINFILELINEREAD& sourceLine =
                                    ((SKINFILELINEREAD*)skinfileLines.data)[sourceRow];
                                if (sourceLine.csv.str[0].isSame("#SRC_TEXT")) {
                                    isTextObject = true;
                                    textAlign = sourceLine.csv.val[4];
                                    break;
                                }
                                if (sourceLine.csv.str[0].isSame("#SRC_NUMBER")) {
                                    isNumberObject = true;
                                    numberAlign = sourceLine.csv.val[12];
                                    break;
                                }
                            }
                            auto alignedObjectX = [&](float anchorX, float width) -> float {
                                if (isTextObject) {
                                    if (textAlign == 1) return anchorX - width * 0.5f;
                                    if (textAlign == 2) return anchorX - width;
                                    return anchorX;
                                }
                                if (isNumberObject) {
                                    if (numberAlign == 0) return anchorX - width;
                                    if (numberAlign == 2) return anchorX - width * 0.5f;
                                }
                                return anchorX;
                            };
                            DST* fallbackDst = NULL;
                            DST* enabledDst = NULL;
                            for (int dstIndex = 0; dstIndex < arr_DST.count; ++dstIndex) {
                                DST& candidate = ((DST*)arr_DST.data)[dstIndex];
                                bool belongsToObject = false;
                                for (int rowIndex = 0; rowIndex < (int)selectedObject.rows.size(); ++rowIndex) {
                                    if (candidate.declare == selectedObject.rows[rowIndex]) {
                                        belongsToObject = true;
                                        break;
                                    }
                                }
                                if (!belongsToObject || candidate.arr_animation.count <= 0) continue;
                                if (!fallbackDst) fallbackDst = &candidate;
                                if (GetOptionFlag_dst(&g, candidate.op1) &&
                                    GetOptionFlag_dst(&g, candidate.op2) &&
                                    GetOptionFlag_dst(&g, candidate.op3)) {
                                    enabledDst = &candidate;
                                    break;
                                }
                            }
                            DST* selectedDst = enabledDst ? enabledDst : fallbackDst;
                            if (!selectedDst) continue;
                            DST_ANIMATION& frame = ((DST_ANIMATION*)selectedDst->arr_animation.data)[0];
                            const float frameX = alignedObjectX(frame.x, frame.w);
                            const float x1 = frameX < frameX + frame.w ? frameX : frameX + frame.w;
                            const float y1 = frame.y < frame.y + frame.h ? frame.y : frame.y + frame.h;
                            const float x2 = frameX > frameX + frame.w ? frameX : frameX + frame.w;
                            const float y2 = frame.y > frame.y + frame.h ? frame.y : frame.y + frame.h;
                            if (firstBounds) {
                                minX = x1; minY = y1; maxX = x2; maxY = y2;
                                firstBounds = false;
                            } else {
                                if (x1 < minX) minX = x1;
                                if (y1 < minY) minY = y1;
                                if (x2 > maxX) maxX = x2;
                                if (y2 > maxY) maxY = y2;
                            }
                            DST_ANIMATION& lastFrame = ((DST_ANIMATION*)selectedDst->arr_animation.data)
                                [selectedDst->arr_animation.count - 1];
                            const float lastFrameX = alignedObjectX(lastFrame.x, lastFrame.w);
                            const float lx1 = lastFrameX < lastFrameX + lastFrame.w
                                ? lastFrameX : lastFrameX + lastFrame.w;
                            const float ly1 = lastFrame.y < lastFrame.y + lastFrame.h
                                ? lastFrame.y : lastFrame.y + lastFrame.h;
                            const float lx2 = lastFrameX > lastFrameX + lastFrame.w
                                ? lastFrameX : lastFrameX + lastFrame.w;
                            const float ly2 = lastFrame.y > lastFrame.y + lastFrame.h
                                ? lastFrame.y : lastFrame.y + lastFrame.h;
                            if (lastBounds) {
                                lastMinX = lx1; lastMinY = ly1; lastMaxX = lx2; lastMaxY = ly2;
                                lastBounds = false;
                            } else {
                                if (lx1 < lastMinX) lastMinX = lx1;
                                if (ly1 < lastMinY) lastMinY = ly1;
                                if (lx2 > lastMaxX) lastMaxX = lx2;
                                if (ly2 > lastMaxY) lastMaxY = ly2;
                            }
                        }
                        if (!firstBounds) {
                            preview_selected_obj = { minX, minY, maxX - minX, maxY - minY };
                            preview_selected_obj_valid = true;
                        }
                        preview_selected_obj_last_valid = !lastBounds;
                        if (!lastBounds)
                            preview_selected_obj_last = { lastMinX, lastMinY,
                                lastMaxX - lastMinX, lastMaxY - lastMinY };
                    }
                    if (ImGui::BeginPopupContextItem("ObjectContext")) {
                        ImGui::TextDisabled("Object %03d", oi);
                        if (ImGui::MenuItem("Create Object (new)")) {
                            newObjectInsertPosition = o.rows.empty() ? skinfileLines.count : o.rows.back() + 1;
                            newObjectIfgroup = o.ifgroup;
                            newCommandIncludeAll = false;
                            wNewObject = true;
                        }
                        if (ImGui::MenuItem("Create Command / Setting")) {
                            newObjectInsertPosition = o.rows.empty() ? skinfileLines.count : o.rows.back() + 1;
                            newObjectIfgroup = o.ifgroup;
                            newCommandIncludeAll = true;
                            wNewObject = true;
                        }
                        if (ImGui::MenuItem("Create Object (duplicate)")) contextCreateModel = modelIndex;
                        if (ImGui::MenuItem("Remove Object")) contextDeleteModel = modelIndex;
                        ImGui::Separator();
                        if (ImGui::MenuItem("Create Group from Selection", NULL, false,
                            !preview_selected_object_model_indices.empty()))
                            requestCreateGroupPopup = true;
                        ImGui::EndPopup();
                    }
                    ImGui::PopID();
                }
            };

            if (cond.rootIfgroup == 0) {
                // Unconditional objects are not part of a condition block.
                bool open = ImGui::TreeNodeEx("always", ImGuiTreeNodeFlags_DefaultOpen, "ALWAYS");
                if (open) {
                    for (int bi = 0; bi < (int)cond.branches.size(); ++bi)
                        drawBranchObjects(cond.branches[bi]);
                    ImGui::TreePop();
                }
            }
            else {
                // The real #IF is the condition node; no synthetic CONDITION
                // wrapper is needed. ELSEIF/ELSE remain sibling branches.
                std::stable_sort(cond.branches.begin(), cond.branches.end(),
                    [](const BranchBlock& a, const BranchBlock& b) { return a.order < b.order; });
                for (int bi = 0; bi < (int)cond.branches.size(); ++bi) {
                    BranchBlock& branch = cond.branches[bi];
                    ImGui::PushID(branch.ifgroup);
                    const bool isBranchActive = branch.ifgroup > 0 &&
                        branch.ifgroup < (int)branchActive.size() && branchActive[branch.ifgroup];
                    bool isParentActive = false;
                    if (branch.ifgroup > 0 && branch.ifgroup < arr_ifunit.count) {
                        const int parent = ((IFUNIT*)arr_ifunit.data)[branch.ifgroup].parentID;
                        isParentActive = parent >= 0 && parent < (int)branchActive.size() &&
                            branchActive[parent];
                    }
                    const ImVec4 branchColor = !isParentActive
                        ? ImVec4(0.28f, 0.28f, 0.30f, 0.62f)
                        : (isBranchActive
                            ? ImVec4(0.18f, 0.48f, 0.23f, 0.62f)
                            : ImVec4(0.55f, 0.18f, 0.18f, 0.62f));
                    const ImVec4 branchHoverColor = !isParentActive
                        ? ImVec4(0.38f, 0.38f, 0.41f, 0.78f)
                        : (isBranchActive
                            ? ImVec4(0.24f, 0.62f, 0.30f, 0.78f)
                            : ImVec4(0.68f, 0.22f, 0.22f, 0.78f));
                    const ImVec2 branchRowMin = ImGui::GetCursorScreenPos();
                    ImGui::GetWindowDrawList()->AddRectFilled(branchRowMin,
                        ImVec2(branchRowMin.x + ImGui::GetContentRegionAvail().x,
                            branchRowMin.y + ImGui::GetFrameHeight()),
                        ImGui::GetColorU32(branchColor));
                    ImGui::PushStyleColor(ImGuiCol_Header, branchColor);
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, branchHoverColor);
                    ImGui::PushStyleColor(ImGuiCol_HeaderActive, branchHoverColor);
                    bool branchOpen = ImGui::TreeNodeEx("branch", ImGuiTreeNodeFlags_DefaultOpen,
                        "%s  (%d)", branch.label.c_str(), (int)branch.localObjectIndices.size());
                    branchTreeOpen[branch.ifgroup] = branchOpen;
                    ImGui::PopStyleColor(3);
                    if (ImGui::BeginPopupContextItem("BranchContext")) {
                        int insertPosition = skinfileLines.count;
                        for (int row = 0; row < skinfileLines.count; ++row) {
                            SKINFILELINEREAD& candidate = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                            if (candidate.ifgroup != branch.ifgroup) continue;
                            if (candidate.isIfGroupEnd) {
                                insertPosition = row;
                                break;
                            }
                            insertPosition = row + 1;
                        }
                        if (ImGui::MenuItem("Create Object in this branch")) {
                            newObjectInsertPosition = insertPosition;
                            newObjectIfgroup = branch.ifgroup;
                            newCommandIncludeAll = false;
                            wNewObject = true;
                        }
                        if (ImGui::MenuItem("Create Command / Setting in this branch")) {
                            newObjectInsertPosition = insertPosition;
                            newObjectIfgroup = branch.ifgroup;
                            newCommandIncludeAll = true;
                            wNewObject = true;
                        }
                        ImGui::EndPopup();
                    }
                    if (branchOpen) {
                        drawBranchObjects(branch);
                        ImGui::TreePop();
                    }
                    ImGui::PopID();
                }
            }
            ImGui::PopID();
            if (conditionIndent > 0.0f) ImGui::Unindent(conditionIndent);
        }
        ImGui::EndChild();
    }

    if (contextCreateModel >= 0 || contextDeleteModel >= 0) {
        const std::vector<SEObjectInstance>& allObjects = g_seObjectEditorModel.Objects();
        if (contextCreateModel >= 0 && contextCreateModel < (int)allObjects.size()) {
            const SEObjectInstance sourceObject = allObjects[contextCreateModel];
            struct ClonedLine { std::string text, owner; int sourceNum, ifgroup; };
            std::vector<ClonedLine> cloneLines;
            for (int row : sourceObject.rows) {
                if (row < 0 || row >= skinfileLines.count) continue;
                SKINFILELINEREAD& source = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                ClonedLine clone;
                clone.text = source.line.body ? source.line.outstr() : "";
                clone.owner = source.filename.body ? source.filename.outstr() : mainpath;
                clone.sourceNum = source.num;
                clone.ifgroup = source.ifgroup;
                cloneLines.push_back(clone);
            }
            if (!cloneLines.empty()) {
                char generatedId[64];
                snprintf(generatedId, sizeof(generatedId), "obj_%08llX_%04d", GetTickCount64(), contextCreateModel);
                int insertPosition = sourceObject.rows.back() + 1;
                auto insertCloneLine = [&](const char* text, const char* owner, int sourceNum, int ifgroup) {
                    InsertLine(insertPosition);
                    SKINFILELINEREAD& inserted = ((SKINFILELINEREAD*)skinfileLines.data)[insertPosition];
                    CSTR oldLine(inserted.line);
                    EditLine(insertPosition, oldLine, CSTR(text));
                    inserted.filename.assign(owner && *owner ? owner : mainpath);
                    inserted.num = sourceNum;
                    inserted.ifgroup = ifgroup;
                    ++insertPosition;
                };
                std::string idLine = std::string("$SE_OBJECT_ID,") + generatedId;
                insertCloneLine(idLine.c_str(), cloneLines.front().owner.c_str(), cloneLines.front().sourceNum,
                    cloneLines.front().ifgroup);
                for (const ClonedLine& clone : cloneLines)
                    insertCloneLine(clone.text.c_str(), clone.owner.c_str(), clone.sourceNum, clone.ifgroup);

                if (selected_user_object_group >= 0) {
                    int currentGroup = -1;
                    for (int row = 0; row < skinfileLines.count; ++row) {
                        SKINFILELINEREAD& meta = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                        const char* text = meta.line.body ? meta.line.outstr() : "";
                        if (strncmp(text, "$SE_GROUP_BEGIN,", 16) == 0) ++currentGroup;
                        else if (strcmp(text, "$SE_GROUP_END") == 0 && currentGroup == selected_user_object_group) {
                            InsertLine(row);
                            SKINFILELINEREAD& member = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                            CSTR oldLine(member.line);
                            std::string memberLine = std::string("$SE_GROUP_MEMBER,") + generatedId;
                            EditLine(row, oldLine, CSTR(memberLine.c_str()));
                            member.filename.assign(mainpath);
                            break;
                        }
                    }
                }
                g_seObjectEditorModel.Rebuild(*this);
                preview_selected_object_model_indices.clear();
                const std::vector<SEObjectInstance>& rebuilt = g_seObjectEditorModel.Objects();
                for (int i = 0; i < (int)rebuilt.size(); ++i) {
                    if (rebuilt[i].editorId == generatedId) {
                        preview_selected_object_model_indices.push_back(i);
                        preview_selected_object_model_index = i;
                        break;
                    }
                }
            }
        } else if (contextDeleteModel >= 0 && contextDeleteModel < (int)allObjects.size()) {
            const SEObjectInstance deleting = allObjects[contextDeleteModel];
            std::vector<int> deleteRows = deleting.rows;
            if (!deleting.rows.empty()) {
                for (int row = deleting.rows.front() - 1; row >= 0; --row) {
                    SKINFILELINEREAD& meta = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    const char* text = meta.line.body ? meta.line.outstr() : "";
                    if (strncmp(text, "$SE_OBJECT_ID,", 14) == 0) { deleteRows.push_back(row); break; }
                    if (*text == '#') break;
                }
            }
            if (!deleting.editorId.empty()) {
                const std::string memberLine = "$SE_GROUP_MEMBER," + deleting.editorId;
                for (int row = 0; row < skinfileLines.count; ++row) {
                    SKINFILELINEREAD& meta = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    if (meta.line.body && memberLine == meta.line.outstr()) deleteRows.push_back(row);
                }
            }
            std::sort(deleteRows.begin(), deleteRows.end());
            deleteRows.erase(std::unique(deleteRows.begin(), deleteRows.end()), deleteRows.end());
            for (std::vector<int>::reverse_iterator it = deleteRows.rbegin(); it != deleteRows.rend(); ++it)
                DeleteLine(*it);
            g_seObjectEditorModel.Rebuild(*this);
            selected_object_editor = 0;
        }
        ImGui::EndChild(); // ObjectNavigator
        ImGui::End();
        return 0;
    }
    ImGui::EndChild(); // ObjectNavigator
    ImGui::SameLine(0.0f, 2.0f);
    ImGui::InvisibleButton("##ObjectListWidthSplitter", ImVec2(6.0f, objectEditorPaneHeight));
    if (ImGui::IsItemHovered() || ImGui::IsItemActive())
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
    if (ImGui::IsItemActive()) {
        objectListWidth += ImGui::GetIO().MouseDelta.x;
        if (objectListWidth < 160.0f) objectListWidth = 160.0f;
        if (objectListWidth > 800.0f) objectListWidth = 800.0f;
    }
    ImGui::SameLine(0.0f, 2.0f);

    // Right: actual Object data. #IF/#ELSEIF/#ELSE are context only.
    if (ImGui::BeginChild("ObjectProperties", ImVec2(0, 0), true)) {
        if (selected_object_editor >= 0 && selected_object_editor < (int)groupObjects.size()) {
            SEObjectInstance& obj = g_seObjectEditorModel.ObjectsMutable()[groupObjects[selected_object_editor]];
            const SEObjectGroupDef* def = g_seObjectEditorModel.Group(obj.group);
            if (def) {
                ImGui::Text("%s", def->name.c_str());

                int ifgroup = obj.ifgroup;
                if (ifgroup == 0 && !obj.rows.empty())
                    ifgroup = ((SKINFILELINEREAD*)skinfileLines.data)[obj.rows[0]].ifgroup;
                ImGui::SameLine();
                if (ifgroup == 0) {
                    ImGui::TextDisabled("[ALWAYS]");
                } else {
                    std::string flowName = "IF";
                    if (ifgroup < arr_ifunit.count) {
                        IFUNIT& unit = ((IFUNIT*)arr_ifunit.data)[ifgroup];
                        if (unit.order > 0) flowName = "ELSE";
                        for (int rno = 0; rno < skinfileLines.count; ++rno) {
                            SKINFILELINEREAD& r = ((SKINFILELINEREAD*)skinfileLines.data)[rno];
                            if (r.ifgroup != ifgroup || !r.isIfGroupHead || !r.csv.str[0].body) continue;
                            flowName = formatConditionHeader(r);
                            break;
                        }
                    }
                    ImGui::TextDisabled("[%s]", flowName.c_str());
                }
                ImGui::Separator();

                char objectName[256];
                const std::string objectNameUtf8 = Cp932ToUtf8(obj.name.c_str());
                strncpy(objectName, objectNameUtf8.c_str(), sizeof(objectName) - 1);
                objectName[sizeof(objectName) - 1] = '\0';
                ImGui::SetNextItemWidth(260.0f);
                if (ImGui::InputText("Name", objectName, sizeof(objectName)))
                    obj.name = Utf8ToCp932(objectName);

                auto drawTaggedImageSelector = [&](int row, const char* command) {
                    if (!command || strncmp(command, "#SRC", 4) != 0 || arr_IMG.count <= 0) return;
                    SKINFILELINEREAD& srcLine = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    const char* fieldNames[5] = { "gr", "x", "y", "w", "h" };
                    int columns[5] = { -1, -1, -1, -1, -1 };
                    for (int col = 1; col < 30; ++col) {
                        CSTR help = GetCommandHelp(command, col);
                        help.trimWhiteSpace();
                        const char* label = help.body ? help.outstr() : "";
                        if (*label == '$') ++label;
                        for (int field = 0; field < 5; ++field)
                            if (_stricmp(label, fieldNames[field]) == 0) columns[field] = col;
                    }
                    for (int field = 0; field < 5; ++field) if (columns[field] < 0) return;

                    const int currentTag = FindIMG(srcLine.csv.val[columns[0]], srcLine.csv.val[columns[1]],
                        srcLine.csv.val[columns[2]], srcLine.csv.val[columns[3]], srcLine.csv.val[columns[4]]);
                    char preview[260];
                    if (currentTag >= 0 && currentTag < arr_IMG.count) {
                        IMG& tag = ((IMG*)arr_IMG.data)[currentTag];
                        const std::string tagNameUtf8 = Cp932ToUtf8(
                            tag.name.body ? tag.name.outstr() : "noname");
                        snprintf(preview, sizeof(preview), "%03d  %s", currentTag,
                            tagNameUtf8.c_str());
                    } else snprintf(preview, sizeof(preview), "Custom coordinates");

                    ImGui::SetNextItemWidth(300.0f);
                    if (ImGui::BeginCombo("Tagged image", preview)) {
                        for (int tagIndex = 0; tagIndex < arr_IMG.count; ++tagIndex) {
                            IMG& tag = ((IMG*)arr_IMG.data)[tagIndex];
                            char tagLabel[260];
                            const std::string tagNameUtf8 = Cp932ToUtf8(
                                tag.name.body ? tag.name.outstr() : "noname");
                            snprintf(tagLabel, sizeof(tagLabel), "%03d  G%02d  %s", tagIndex, tag.gr,
                                tagNameUtf8.c_str());
                            if (ImGui::Selectable(tagLabel, tagIndex == currentTag)) {
                                for (int field = 0; field < 5; ++field) {
                                    const int values[5] = { tag.gr, tag.x, tag.y, tag.w, tag.h };
                                    EditValue(row, columns[field], values[field]);
                                }
                                src_selected = tagIndex;
                                gr_selected = tag.gr;
                            }
                            if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) &&
                                tag.gr >= 0 && tag.gr < arr_SRCGR.count) {
                                EnsureSRCGRTexture(tag.gr);
                                SRCGR& sourceImage = ((SRCGR*)arr_SRCGR.data)[tag.gr];
                                const int tagW = tag.w == -1 ? sourceImage.sizeX - tag.x : tag.w;
                                const int tagH = tag.h == -1 ? sourceImage.sizeY - tag.y : tag.h;
                                if (sourceImage.texture && sourceImage.sizeX > 0 && sourceImage.sizeY > 0 &&
                                    tagW > 0 && tagH > 0 && ImGui::BeginTooltip()) {
                                    float scale = 1.0f;
                                    if (tagW > 240 || tagH > 180) {
                                        const float sx = 240.0f / tagW, sy = 180.0f / tagH;
                                        scale = sx < sy ? sx : sy;
                                    }
                                    ImGui::Image(sourceImage.texture, ImVec2(tagW * scale, tagH * scale),
                                        ImVec2(tag.x / (float)sourceImage.sizeX, tag.y / (float)sourceImage.sizeY),
                                        ImVec2((tag.x + tagW) / (float)sourceImage.sizeX,
                                            (tag.y + tagH) / (float)sourceImage.sizeY));
                                    ImGui::Text("gr %d  x %d  y %d  w %d  h %d", tag.gr, tag.x, tag.y, tag.w, tag.h);
                                    ImGui::EndTooltip();
                                }
                            }
                        }
                        ImGui::EndCombo();
                    }
                };

                auto drawObjectPropertyRow = [&](int row, int compactDstIndex) {
                    if (row < 0 || row >= skinfileLines.count) return;
                    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    const char* command = line.csv.str[0].body ? line.csv.str[0].outstr() : "";
                    const bool compactDst = compactDstIndex >= 0;
                    ImGui::PushID(row);
                    if (compactDst) {
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 2.0f));
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 2.0f));
                    }
                    if (compactDst || ImGui::CollapsingHeader(command, ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (!compactDst) drawTaggedImageSelector(row, command);
                        int maxcol = 30;
                        for (int col = 1; col < maxcol; ++col) {
                            CSTR help = GetCommandHelp(command, col);
                            help.trimWhiteSpace();
                            if (!line.csv.str[col].body && !help.body) continue;
                            const char* label = help.body ? help.outstr() : "value";
                            if (!label || !*label) continue;
                            // In animation frames after the first DST, these values are inherited
                            // from the first row and are not used independently. Match by field
                            // name because their column positions differ between DST commands.
                            const bool isSharedDstField = _strnicmp(label, "loop", 4) == 0 ||
                                _strnicmp(label, "timer", 5) == 0 || _strnicmp(label, "$timer", 6) == 0 ||
                                _strnicmp(label, "op1", 3) == 0 || _strnicmp(label, "$op1", 4) == 0 ||
                                _strnicmp(label, "op2", 3) == 0 || _strnicmp(label, "$op2", 4) == 0 ||
                                _strnicmp(label, "op3", 3) == 0 || _strnicmp(label, "$op3", 4) == 0;
                            if (compactDstIndex > 0 && isSharedDstField) continue;
                            ImGui::PushID(col);
                            const char* widgetLabel = label;
                            if (compactDst) {
                                if (compactDstIndex == 0) {
                                    ImGui::AlignTextToFramePadding();
                                    ImGui::TextDisabled("%s", label);
                                    ImGui::SameLine(62.0f);
                                }
                                widgetLabel = "##value";
                                ImGui::SetNextItemWidth(-FLT_MIN);
                            }
                            const bool isType = help.left(5).isSame("$type");
                            const bool isOp = help.left(3).isSame("$op");
                            const bool isString = help.left(3).isSame("$st");
                            const bool isNumber = help.left(4).isSame("$num");
                            const bool isTimer = help.left(6).isSame("$timer");
                            if (isType || isOp || isString || isNumber || isTimer) {
                                const int current = line.csv.val[col];
                                int itemCount = 0;
                                const char* currentName = "";
                                if (isType) { itemCount = 21; currentName = SKINTYPESTR[current]; }
                                else if (isOp) { itemCount = 1999; currentName = dstName(current < 0 ? -current : current); }
                                else if (isString) { itemCount = 200; currentName = textName(current); }
                                else if (isNumber) { itemCount = 300; currentName = numberName(current); }
                                else { itemCount = 300; currentName = timerName(current); }

                                char preview[128];
                                if (isOp && current < 0)
                                    snprintf(preview, sizeof(preview), "NOT %03d:%s", -current, currentName ? currentName : "");
                                else
                                    snprintf(preview, sizeof(preview), "%03d:%s", current, currentName ? currentName : "");
                                if (ImGui::BeginCombo(widgetLabel, preview)) {
                                    ImGuiListClipper clipper;
                                    clipper.Begin(itemCount);
                                    while (clipper.Step()) {
                                        for (int item = clipper.DisplayStart; item < clipper.DisplayEnd; ++item) {
                                            const int value = isOp && item >= 1000 ? -(item - 999) : item;
                                            const char* valueName = "";
                                            if (isType) valueName = SKINTYPESTR[value];
                                            else if (isOp) valueName = dstName(value < 0 ? -value : value);
                                            else if (isString) valueName = textName(value);
                                            else if (isNumber) valueName = numberName(value);
                                            else valueName = timerName(value);
                                            char option[128];
                                            if (isOp && value < 0)
                                                snprintf(option, sizeof(option), "NOT %03d:%s", -value, valueName ? valueName : "");
                                            else
                                                snprintf(option, sizeof(option), "%03d:%s", value, valueName ? valueName : "");
                                            if (ImGui::Selectable(option, current == value))
                                                EditValue(row, col, value);
                                        }
                                    }
                                    ImGui::EndCombo();
                                }
                            } else {
                                CSTR before(line.csv.str[col]);
                                CstrInputText(widgetLabel, &line.csv.str[col], ImGuiInputTextFlags_EnterReturnsTrue);
                                if (before.isDiff(line.csv.str[col])) {
                                    CSTR newValue(line.csv.str[col]);
                                    line.csv.str[col] = before;
                                    EditValue(row, col, newValue.outstr());
                                }
                            }
                            ImGui::PopID();
                        }
                    }
                    if (compactDst) ImGui::PopStyleVar(2);
                    ImGui::PopID();
                };

                std::vector<int> srcRows;
                std::vector<int> dstRows;
                for (std::size_t ri = 0; ri < obj.rows.size(); ++ri) {
                    const int row = obj.rows[ri];
                    if (row < 0 || row >= skinfileLines.count) continue;
                    SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                    const char* command = line.csv.str[0].body ? line.csv.str[0].outstr() : "";
                    if (strncmp(command, "#DST", 4) == 0)
                        dstRows.push_back(row);
                    else
                        srcRows.push_back(row);
                }

                if (ImGui::BeginTabBar("ObjectPropertyTabs")) {
                    if (ImGui::BeginTabItem("SRC")) {
                        ImGui::SeparatorText("basic");
                        if (srcRows.empty()) ImGui::TextDisabled("No SRC properties.");
                        for (std::size_t i = 0; i < srcRows.size(); ++i)
                            drawObjectPropertyRow(srcRows[i], -1);
                        ImGui::EndTabItem();
                    }

                    char dstTabLabel[64];
                    snprintf(dstTabLabel, sizeof(dstTabLabel), "DST (%d)", (int)dstRows.size());
                    if (ImGui::BeginTabItem(dstTabLabel)) {
                        ImGui::SeparatorText(dstRows.size() > 1 ? "animation frames" : "basic");
                        if (dstRows.empty()) {
                            ImGui::TextDisabled("No DST properties.");
                        } else if (dstRows.size() == 1) {
                            drawObjectPropertyRow(dstRows[0], -1);
                        } else {
                            // Most DST values are short integers. Keep animation
                            // columns dense so several frames fit on screen.
                            const float labelWidth = 50.0f;
                            const float valueWidth = 82.0f;
                            const float cellPaddingWidth = ImGui::GetStyle().CellPadding.x * 2.0f;
                            const float innerWidth = labelWidth + valueWidth * (float)dstRows.size() +
                                cellPaddingWidth * ((float)dstRows.size() + 1.0f) + 16.0f;
                            const ImGuiTableFlags flags = ImGuiTableFlags_Borders |
                                ImGuiTableFlags_ScrollX | ImGuiTableFlags_SizingFixedFit |
                                ImGuiTableFlags_RowBg;
                            if (ImGui::BeginTable("DSTHorizontalRows", (int)dstRows.size() + 1, flags,
                                ImVec2(0.0f, 0.0f), innerWidth)) {
                                // Keep property names and the authoritative first DST visible;
                                // only animation frames after DST 1 scroll horizontally.
                                ImGui::TableSetupScrollFreeze(2, 1);
                                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, labelWidth);
                                for (int i = 0; i < (int)dstRows.size(); ++i) {
                                    char columnLabel[32];
                                    snprintf(columnLabel, sizeof(columnLabel), "DST %d", i + 1);
                                    ImGui::TableSetupColumn(columnLabel, ImGuiTableColumnFlags_WidthFixed, valueWidth);
                                }
                                ImGui::TableHeadersRow();

                                SKINFILELINEREAD& firstDst = ((SKINFILELINEREAD*)skinfileLines.data)[dstRows[0]];
                                const char* firstCommand = firstDst.csv.str[0].body ? firstDst.csv.str[0].outstr() : "";
                                for (int col = 1; col < 30; ++col) {
                                    CSTR firstHelp = GetCommandHelp(firstCommand, col);
                                    firstHelp.trimWhiteSpace();
                                    // Multi-DST rows follow the documented command fields only.
                                    // Ignore trailing CSV values and CR/LF-only helper entries.
                                    if (!firstHelp.body) continue;
                                    const char* rowLabel = firstHelp.outstr();
                                    if (!rowLabel || !*rowLabel) continue;

                                    const bool sharedField = _strnicmp(rowLabel, "loop", 4) == 0 ||
                                        _strnicmp(rowLabel, "timer", 5) == 0 || _strnicmp(rowLabel, "$timer", 6) == 0 ||
                                        _strnicmp(rowLabel, "op1", 3) == 0 || _strnicmp(rowLabel, "$op1", 4) == 0 ||
                                        _strnicmp(rowLabel, "op2", 3) == 0 || _strnicmp(rowLabel, "$op2", 4) == 0 ||
                                        _strnicmp(rowLabel, "op3", 3) == 0 || _strnicmp(rowLabel, "$op3", 4) == 0;

                                    ImGui::TableNextRow();
                                    ImGui::TableSetColumnIndex(0);
                                    ImGui::AlignTextToFramePadding();
                                    ImGui::TextDisabled("%s", rowLabel);

                                    for (int i = 0; i < (int)dstRows.size(); ++i) {
                                        ImGui::TableSetColumnIndex(i + 1);
                                        if (i > 0 && sharedField) continue;

                                        const int row = dstRows[i];
                                        SKINFILELINEREAD& line = ((SKINFILELINEREAD*)skinfileLines.data)[row];
                                        CSTR help = GetCommandHelp(line.csv.str[0].outstr(), col);
                                        help.trimWhiteSpace();
                                        ImGui::PushID(row);
                                        ImGui::PushID(col);
                                        ImGui::SetNextItemWidth(-FLT_MIN);
                                        const bool isType = help.left(5).isSame("$type");
                                        const bool isOp = help.left(3).isSame("$op");
                                        const bool isString = help.left(3).isSame("$st");
                                        const bool isNumber = help.left(4).isSame("$num");
                                        const bool isTimer = help.left(6).isSame("$timer");
                                        if (isType || isOp || isString || isNumber || isTimer) {
                                            const int current = line.csv.val[col];
                                            int itemCount = 0;
                                            const char* currentName = "";
                                            if (isType) { itemCount = 21; currentName = SKINTYPESTR[current]; }
                                            else if (isOp) { itemCount = 1999; currentName = dstName(current < 0 ? -current : current); }
                                            else if (isString) { itemCount = 200; currentName = textName(current); }
                                            else if (isNumber) { itemCount = 300; currentName = numberName(current); }
                                            else { itemCount = 300; currentName = timerName(current); }
                                            char preview[128];
                                            if (isOp && current < 0)
                                                snprintf(preview, sizeof(preview), "NOT %03d:%s", -current, currentName ? currentName : "");
                                            else
                                                snprintf(preview, sizeof(preview), "%03d:%s", current, currentName ? currentName : "");
                                            if (ImGui::BeginCombo("##value", preview)) {
                                                ImGuiListClipper clipper;
                                                clipper.Begin(itemCount);
                                                while (clipper.Step()) {
                                                    for (int item = clipper.DisplayStart; item < clipper.DisplayEnd; ++item) {
                                                        const int value = isOp && item >= 1000 ? -(item - 999) : item;
                                                        const char* valueName = "";
                                                        if (isType) valueName = SKINTYPESTR[value];
                                                        else if (isOp) valueName = dstName(value < 0 ? -value : value);
                                                        else if (isString) valueName = textName(value);
                                                        else if (isNumber) valueName = numberName(value);
                                                        else valueName = timerName(value);
                                                        char option[128];
                                                        if (isOp && value < 0)
                                                            snprintf(option, sizeof(option), "NOT %03d:%s", -value, valueName ? valueName : "");
                                                        else
                                                            snprintf(option, sizeof(option), "%03d:%s", value, valueName ? valueName : "");
                                                        if (ImGui::Selectable(option, current == value)) EditValue(row, col, value);
                                                    }
                                                }
                                                ImGui::EndCombo();
                                            }
                                            if ((isOp || isTimer) && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                                                ImGui::SetTooltip("%s", preview);
                                        } else {
                                            CSTR before(line.csv.str[col]);
                                            CstrInputText("##value", &line.csv.str[col], ImGuiInputTextFlags_EnterReturnsTrue);
                                            if (before.isDiff(line.csv.str[col])) {
                                                CSTR newValue(line.csv.str[col]);
                                                line.csv.str[col] = before;
                                                EditValue(row, col, newValue.outstr());
                                            }
                                        }
                                        ImGui::PopID();
                                        ImGui::PopID();
                                    }
                                }
                                ImGui::EndTable();
                            }
                        }
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            }
        } else {
            ImGui::Text("No object.");
        }
        ImGui::EndChild();
    }

    ImGui::End();
    return 0;
}
