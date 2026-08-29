// Dear ImGui: standalone example application for Windows API + DirectX 9

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include <WinSock2.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>

// added includes
#pragma comment(lib, "d3d9.lib")
#include <WinSock2.h>
#include <Windows.h>
#include <DxLib/DxLib.h>

#include "winWorkspace.h"
#include "seHelper.h"
#include "seUI.h"
#include "selfTests.h"
#include "uiCatalog.h"


// Data
static LPDIRECT3D9              g_pD3D = nullptr;
//static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr; //moved to imageLoader.cpp
static bool                     g_DeviceLost = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void DrawHelpWindow(bool* open)
{
    char title[64];
    FormatSEUISurfaceTitle(title, sizeof(title), SEUISurfaceId::Help);
    if (!ImGui::Begin(title, open, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    SEUI::SectionHeader("Getting started");
    ImGui::TextUnformatted("1. Create or open a workspace from the Workspace menu.");
    ImGui::TextUnformatted("2. Choose an LR2 skin script, then inspect it in Preview.");
    ImGui::TextUnformatted("3. Select objects in Object Browser and edit them in Object Inspector.");
    ImGui::TextUnformatted("4. Use Asset Browser and Image Manager for source images.");

    SEUI::SectionHeader("Workspace map");
    ImGui::TextUnformatted("Left: object navigation and selection");
    ImGui::TextUnformatted("Center: preview, images, destinations and assets");
    ImGui::TextUnformatted("Right: options, properties and customization");
    ImGui::TextDisabled("Window visibility can be changed from Workspace > Windows.");

    ImGui::Separator();
    if (ImGui::Button("Close"))
        *open = false;
    ImGui::End();
}

// Main code
int WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    if (cmdline && strstr(cmdline, "--self-test-schema-contract"))
        return RunSchemaContractSelfTest();
    if (cmdline && strstr(cmdline, "--self-test-ui-contract"))
        return RunUiCatalogSelfTest();
    if (cmdline && strstr(cmdline, "--self-test-skin-browser"))
        return RunSkinBrowserSelfTest();
    if (cmdline && strstr(cmdline, "--self-test-preview-simulator"))
        return RunPreviewSimulatorSelfTest();
    if (cmdline && strstr(cmdline, "--self-test-resolution-estimator"))
        return RunResolutionEstimatorSelfTest();
    if (cmdline && strstr(cmdline, "--self-test-olr-package"))
        return RunOlrPackageSelfTest();
    if (cmdline && strstr(cmdline, "--self-test-simple-mode"))
        return RunSimpleModeProjectionSelfTest();
    if (cmdline && strstr(cmdline, "--self-test-reload-lifecycle"))
        return RunWorkspaceReloadLifecycleSelfTest();
    if (cmdline && strstr(cmdline, "--self-test-dst-color"))
        return RunDstColorSelfTest();

    if (cmdline && strstr(cmdline, "--self-test-asset-metadata"))
        return RunAssetMetadataSelfTest();

    // Make process DPI aware and obtain main monitor scale
    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    // Create application window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"LR2SkinEditor_v0.8", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, (int)(1280 * main_scale), (int)(800 * main_scale),
        nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    if (cmdline && strstr(cmdline, "--self-test-pixel-paint")) {
        const int result = RunPixelPaintSelfTest();
        CleanupDeviceD3D();
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return result;
    }
    if (cmdline && strstr(cmdline, "--self-test-initial-preset")) {
        const int result = RunInitialPresetSelfTest();
        CleanupDeviceD3D();
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return result;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_MAXIMIZE);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = NULL;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    // Keep all editor tools inside the main workspace.
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup the shared SkinEditor design system before individual windows are drawn.
    ImGuiStyle& style = ImGui::GetStyle();
    SEUI::ApplyModernTheme(main_scale);
    style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)
    io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
    io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    // Load Fonts
    // - If fonts are not explicitly loaded, Dear ImGui will select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
    //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a small threshold.
    // - You can load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefaultVector();
    //io.Fonts->AddFontDefaultBitmap();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);
    // Segoe UI gives the editor a native, web-like UI face. Japanese and
    // Korean glyphs are merged into the same font so skin metadata remains
    // readable without changing widget code.
    ImFont* uiFont = io.Fonts->AddFontFromFileTTF(
        "C:\\Windows\\Fonts\\segoeui.ttf", 17.0f, NULL, io.Fonts->GetGlyphRangesDefault());
    if (uiFont) {
        ImFontConfig mergeConfig;
        mergeConfig.MergeMode = true;
        mergeConfig.PixelSnapH = true;
        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\meiryo.ttc", 17.0f,
            &mergeConfig, io.Fonts->GetGlyphRangesJapanese());
        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", 17.0f,
            &mergeConfig, io.Fonts->GetGlyphRangesKorean());
        io.FontDefault = uiFont;
    }

    // Our state
    bool show_help_window = false;
    ImVec4 clear_color = ImVec4(0.035f, 0.043f, 0.060f, 1.00f);

    //DxLib init
    ChangeWindowMode(1);
    SetMultiThreadFlag(1);
    SetAlwaysRunFlag(1);
    SetUseDirect3DVersion(DX_DIRECT3D_9);
    SetWindowVisibleFlag(0);
    SetMainWindowText("skinPreview2");
    DxLib_Init();

    // Optional real-skin regression probe. It is intentionally not part of CI
    // because the two paths refer to user-supplied LR2 trees.
    if (cmdline && (strstr(cmdline, "--skin-reload-smoke") ||
        strstr(cmdline, "--skin-multi-workspace-smoke"))) {
        LoadCommandHelp("..\\skinHelper.txt");
        const bool useSeparateWorkspaces =
            strstr(cmdline, "--skin-multi-workspace-smoke") != NULL;
        const int result = useSeparateWorkspaces
            ? RunWorkspaceRuntimeMultiWorkspaceSmokeTest(
                getenv("SKINEDITOR_RELOAD_FIRST"),
                getenv("SKINEDITOR_RELOAD_SECOND"))
            : RunWorkspaceRuntimeReloadSmokeTest(
                getenv("SKINEDITOR_RELOAD_FIRST"),
                getenv("SKINEDITOR_RELOAD_SECOND"));
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        CleanupDeviceD3D();
        ::DestroyWindow(hwnd);
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        if (DxLib_IsInit()) DxLib_End();
        return result;
    }

    //SE init
    // Development builds read the editable source file. Packaged builds fall
    // back to the RCDATA copy embedded in the executable.
    LoadCommandHelp("..\\skinHelper.txt");
    makeTransBackground();
    workspaceList.clear();

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle lost D3D9 device
        if (g_DeviceLost)
        {
            HRESULT hr = g_pd3dDevice->TestCooperativeLevel();
            if (hr == D3DERR_DEVICELOST)
            {
                ::Sleep(10);
                continue;
            }
            if (hr == D3DERR_DEVICENOTRESET)
                ResetDevice();
            g_DeviceLost = false;
        }

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            g_d3dpp.BackBufferWidth = g_ResizeWidth;
            g_d3dpp.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (show_help_window)
            DrawHelpWindow(&show_help_window);

        //MainFrame and menu
        if (ImGui::BeginMainMenuBar()) {
            ImGui::TextColored(SEUI::Colors::Accent(), "LR2 Skin Editor");
            ImGui::Separator();
            if (ImGui::BeginMenu("Workspace")) {
                if (ImGui::MenuItem("New Workspace", NULL, false, true)) {
                    workspaceList.push_back(std::unique_ptr<WORKSPACE>(new WORKSPACE()));
                    WORKSPACE* work = workspaceList.back().get();
                    work->alive = true;
                    work->num = (int)workspaceList.size() - 1;
                    snprintf(work->title, 260, "Workspace %d", (int)workspaceList.size() - 1);
                    //work->init();
                }
                ImGui::Separator();
                for (int i = 0; i < (int)workspaceList.size(); i++) {
                    WORKSPACE& workspace = *workspaceList[i];
                    ImGui::MenuItem(workspace.title, NULL, &workspace.alive);
                }
                ImGui::EndMenu();
            }

            const SEUISurfaceSpec& helpSpec = SEUISurfaceSpecFor(SEUISurfaceId::Help);
            ImGui::MenuItem(helpSpec.title, NULL, &show_help_window);
            //ImGui::MenuItem("Info", NULL, );

            ImGui::EndMainMenuBar();
        }

        //draw workspaces
        for (int i = 0; i < (int)workspaceList.size(); i++) {

            //char dock[64];
            //snprintf(dock, sizeof(dock), "dock%d", i);
            //ImGuiID dockspace_id = ImGui::GetID(dock);
            //ImGui::DockSpace(dockspace_id, ImVec2(1.0f, 1.0f), ImGuiDockNodeFlags_NoResize);
            //float minWinSizeX = style.WindowMinSize.x;
            //style.WindowMinSize.x = 1280;
            //style.WindowMinSize.x = minWinSizeX;


            WORKSPACE& workspace = *workspaceList[i];
            //if (arr[i].initFlag == 0) {
            //    arr[i].init();
            //}
            if (workspace.alive) {
                workspace.draw();
            }
        }

        // Rendering
        ImGui::EndFrame();
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
        if (result == D3DERR_DEVICELOST)
            g_DeviceLost = true;
    }

    // Cleanup
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    //
    if (DxLib_IsInit()) DxLib_End();
    workspaceList.clear();

    return 0;
}

// Helper functions
bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (SUCCEEDED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
        hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp,
        &g_pd3dDevice)))
        return true;

    // Hosted CI and remote Windows sessions may expose Direct3D 9 without
    // hardware vertex processing. Keep the normal fast path above, then use a
    // software vertex device so graphics self-tests remain meaningful there.
    if (SUCCEEDED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
        hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &g_d3dpp,
        &g_pd3dDevice)))
        return true;

    if (SUCCEEDED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF,
        hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &g_d3dpp,
        &g_pd3dDevice)))
        return true;

    return false;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
