// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "gui_setup_dx11.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <d3d11.h>
#include <tchar.h>
#include <xinput.h>

#include "util/scope.h"

#include "gui_state.h"

using PGetDpiForWindow = UINT(WINAPI*)(HWND hwnd);

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool g_SwapChainOccluded = false;
static bool g_WindowMoved = false;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Using XInput for gamepad (will load DLL dynamically)
typedef DWORD(WINAPI* PFN_XInputGetCapabilities)(DWORD, DWORD, XINPUT_CAPABILITIES*);
typedef DWORD(WINAPI* PFN_XInputGetState)(DWORD, XINPUT_STATE*);
static bool s_HasGamepad = false;
static bool s_WantUpdateHasGamepad = false;
static HMODULE s_XInputDLL = nullptr;
static PFN_XInputGetCapabilities s_XInputGetCapabilities = nullptr;
static PFN_XInputGetState s_XInputGetState = nullptr;

static void ImplXInput_NewFrame(HWND hwnd, ImGuiIO& io) {
    // Calling XInputGetState() every frame on disconnected gamepads is unfortunately too slow.
    // Instead we refresh gamepad availability by calling XInputGetCapabilities() _only_ after
    // receiving WM_DEVICECHANGE.
    if (s_WantUpdateHasGamepad) {
        XINPUT_CAPABILITIES caps = {};
        s_HasGamepad =
            s_XInputGetCapabilities
                ? (s_XInputGetCapabilities(0, XINPUT_FLAG_GAMEPAD, &caps) == ERROR_SUCCESS)
                : false;
        s_WantUpdateHasGamepad = false;
    }

    io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;

    HWND focused_window = ::GetForegroundWindow();
    const bool is_app_focused = (focused_window == hwnd);
    if (!is_app_focused) {
        return;
    }

    XINPUT_STATE xinput_state{};
    if (!s_HasGamepad || s_XInputGetState == nullptr
        || s_XInputGetState(0, &xinput_state) != ERROR_SUCCESS) {
        return;
    }
    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;

    XINPUT_GAMEPAD& gamepad = xinput_state.Gamepad;

#define IM_SATURATE(V) (V < 0.0f ? 0.0f : V > 1.0f ? 1.0f : V)
#define MAP_BUTTON(KEY_NO, BUTTON_ENUM)                                \
    {                                                                  \
        io.AddKeyEvent(KEY_NO, (gamepad.wButtons & BUTTON_ENUM) != 0); \
    }
#define MAP_ANALOG(KEY_NO, VALUE, V0, V1)                          \
    {                                                              \
        float vn = (float)(VALUE - V0) / (float)(V1 - V0);         \
        io.AddKeyAnalogEvent(KEY_NO, vn > 0.10f, IM_SATURATE(vn)); \
    }
    MAP_BUTTON(ImGuiKey_GamepadStart, XINPUT_GAMEPAD_START);
    MAP_BUTTON(ImGuiKey_GamepadBack, XINPUT_GAMEPAD_BACK);
    MAP_BUTTON(ImGuiKey_GamepadFaceLeft, XINPUT_GAMEPAD_X);
    MAP_BUTTON(ImGuiKey_GamepadFaceRight, XINPUT_GAMEPAD_B);
    MAP_BUTTON(ImGuiKey_GamepadFaceUp, XINPUT_GAMEPAD_Y);
    MAP_BUTTON(ImGuiKey_GamepadFaceDown, XINPUT_GAMEPAD_A);
    MAP_BUTTON(ImGuiKey_GamepadDpadLeft, XINPUT_GAMEPAD_DPAD_LEFT);
    MAP_BUTTON(ImGuiKey_GamepadDpadRight, XINPUT_GAMEPAD_DPAD_RIGHT);
    MAP_BUTTON(ImGuiKey_GamepadDpadUp, XINPUT_GAMEPAD_DPAD_UP);
    MAP_BUTTON(ImGuiKey_GamepadDpadDown, XINPUT_GAMEPAD_DPAD_DOWN);
    MAP_BUTTON(ImGuiKey_GamepadL1, XINPUT_GAMEPAD_LEFT_SHOULDER);
    MAP_BUTTON(ImGuiKey_GamepadR1, XINPUT_GAMEPAD_RIGHT_SHOULDER);
    MAP_ANALOG(ImGuiKey_GamepadL2, gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 255);
    MAP_ANALOG(ImGuiKey_GamepadR2, gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, 255);
    MAP_BUTTON(ImGuiKey_GamepadL3, XINPUT_GAMEPAD_LEFT_THUMB);
    MAP_BUTTON(ImGuiKey_GamepadR3, XINPUT_GAMEPAD_RIGHT_THUMB);
    MAP_ANALOG(
        ImGuiKey_GamepadLStickLeft, gamepad.sThumbLX, -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32768);
    MAP_ANALOG(
        ImGuiKey_GamepadLStickRight, gamepad.sThumbLX, +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
    MAP_ANALOG(
        ImGuiKey_GamepadLStickUp, gamepad.sThumbLY, +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
    MAP_ANALOG(
        ImGuiKey_GamepadLStickDown, gamepad.sThumbLY, -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32768);
    MAP_ANALOG(
        ImGuiKey_GamepadRStickLeft, gamepad.sThumbRX, -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32768);
    MAP_ANALOG(
        ImGuiKey_GamepadRStickRight, gamepad.sThumbRX, +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
    MAP_ANALOG(
        ImGuiKey_GamepadRStickUp, gamepad.sThumbRY, +XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, +32767);
    MAP_ANALOG(
        ImGuiKey_GamepadRStickDown, gamepad.sThumbRY, -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, -32768);
#undef MAP_BUTTON
#undef MAP_ANALOG
}

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int SenTools::RunGuiDX11(
    GuiState& state,
    const wchar_t* windowTitle,
    const ImVec4& backgroundColor,
    const std::function<void(ImGuiIO& io, GuiState& state)>& loadFontsCallback,
    const std::function<bool(ImGuiIO& io, GuiState& state)>& renderFrameCallback,
    const std::function<void(ImGuiIO& io, GuiState& state)>& loadIniCallback,
    const std::function<void(ImGuiIO& io, GuiState& state)>& saveIniCallback) {
    // Create application window
    ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = {sizeof(wc),
                      CS_CLASSDC,
                      WndProc,
                      0L,
                      0L,
                      GetModuleHandle(nullptr),
                      nullptr,
                      nullptr,
                      nullptr,
                      nullptr,
                      L"SenPatcher",
                      nullptr};
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName,
                                windowTitle,
                                WS_OVERLAPPEDWINDOW,
                                100,
                                100,
                                1280,
                                800,
                                nullptr,
                                nullptr,
                                wc.hInstance,
                                nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.IniFilename = nullptr;                             // we manually handle the ini

    loadIniCallback(io, state);

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);

    s_WantUpdateHasGamepad = true;
    const char* xinput_dll_names[] = {
        "xinput1_4.dll",   // Windows 8+
        "xinput1_3.dll",   // DirectX SDK
        "xinput9_1_0.dll", // Windows Vista, Windows 7
        "xinput1_2.dll",   // DirectX SDK
        "xinput1_1.dll"    // DirectX SDK
    };
    for (int n = 0; n < IM_ARRAYSIZE(xinput_dll_names); n++) {
        if (HMODULE dll = ::LoadLibraryA(xinput_dll_names[n])) {
            s_XInputDLL = dll;
            s_XInputGetCapabilities =
                (PFN_XInputGetCapabilities)::GetProcAddress(dll, "XInputGetCapabilities");
            s_XInputGetState = (PFN_XInputGetState)::GetProcAddress(dll, "XInputGetState");
            break;
        }
    }


    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    HMODULE user32_dll = LoadLibraryW(L"USER32.dll");
    auto user32_dll_scope = HyoutaUtils::MakeScopeGuard([&] {
        if (user32_dll != NULL) {
            FreeLibrary(user32_dll);
            user32_dll = NULL;
        }
    });
    PGetDpiForWindow ptrGetDpiForWindow = nullptr;
    if (user32_dll != NULL) {
        FARPROC ptr = GetProcAddress(user32_dll, "GetDpiForWindow");
        if (ptr != NULL) {
            ptrGetDpiForWindow = reinterpret_cast<PGetDpiForWindow>(ptr);
        }
    }


    g_WindowMoved = true; // init the DPI and style stuff

    // Main loop
    state.CurrentDpi = -1.0f;
    bool done = false;
    while (!done) {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window being minimized or screen locked
        if (g_SwapChainOccluded
            && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
            g_WindowMoved = true;
        }

        // Moved window may imply DPI change, handle that
        if (g_WindowMoved) {
            float oldDpi = state.CurrentDpi;
            float newDpi;
            UINT dpi = 0;
            if (ptrGetDpiForWindow) {
                dpi = (*ptrGetDpiForWindow)(hwnd);
            }
            if (dpi == 0) {
                // error or function not available, assume unchanged
                newDpi = oldDpi;
            } else {
                newDpi = static_cast<float>(dpi) / USER_DEFAULT_SCREEN_DPI;
            }
            if (newDpi <= 0.0f) {
                // something broken, set to a sane default
                newDpi = 1.0f;
            }
            if (oldDpi != newDpi) {
                ImGui_ImplDX11_InvalidateDeviceObjects();

                state.CurrentDpi = newDpi;
                auto& style = ImGui::GetStyle();
                style = ImGuiStyle();
                ImGui::StyleColorsDark(&style);
                if (newDpi != 1.0f) {
                    style.ScaleAllSizes(newDpi);
                }
                loadFontsCallback(io, state);
            }
        }

        io.ConfigNavSwapGamepadButtons = state.GuiSettings.GamepadSwapConfirmCancel;

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImplXInput_NewFrame(hwnd, io);
        ImGui::NewFrame();

        if (!renderFrameCallback(io, state)) {
            break;
        }

        if (io.WantSaveIniSettings) {
            saveIniCallback(io, state);
            io.WantSaveIniSettings = false;
        }

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = {backgroundColor.x * backgroundColor.w,
                                                 backgroundColor.y * backgroundColor.w,
                                                 backgroundColor.z * backgroundColor.w,
                                                 backgroundColor.w};
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Present
        HRESULT hr = g_pSwapChain->Present(1, 0); // Present with vsync
        // HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    saveIniCallback(io, state);

    // Cleanup
    ImGui_ImplDX11_Shutdown();

    if (s_XInputDLL) {
        ::FreeLibrary(s_XInputDLL);
        s_XInputDLL = nullptr;
        s_XInputGetCapabilities = nullptr;
        s_XInputGetState = nullptr;
    }

    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd) {
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    // createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr,
                                                D3D_DRIVER_TYPE_HARDWARE,
                                                nullptr,
                                                createDeviceFlags,
                                                featureLevelArray,
                                                2,
                                                D3D11_SDK_VERSION,
                                                &sd,
                                                &g_pSwapChain,
                                                &g_pd3dDevice,
                                                &featureLevel,
                                                &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is
                                       // not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr,
                                            D3D_DRIVER_TYPE_WARP,
                                            nullptr,
                                            createDeviceFlags,
                                            featureLevelArray,
                                            2,
                                            D3D11_SDK_VERSION,
                                            &sd,
                                            &g_pSwapChain,
                                            &g_pd3dDevice,
                                            &featureLevel,
                                            &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) {
        g_pSwapChain->Release();
        g_pSwapChain = nullptr;
    }
    if (g_pd3dDeviceContext) {
        g_pd3dDeviceContext->Release();
        g_pd3dDeviceContext = nullptr;
    }
    if (g_pd3dDevice) {
        g_pd3dDevice->Release();
        g_pd3dDevice = nullptr;
    }
}

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to
// use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or
// clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main
// application, or clear/overwrite your copy of the keyboard data. Generally you may always pass all
// inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_DEVICECHANGE && static_cast<UINT>(wParam) == 0x0007) {
        s_WantUpdateHasGamepad = true;
    }

    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
        case WM_MOVE: g_WindowMoved = true; break;
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
        case WM_DESTROY: ::PostQuitMessage(0); return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
