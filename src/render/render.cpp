#include "render.hpp"
#include "src/core/interfaces.hpp"
#include "src/menu/menu.hpp"
#include "src/menu/ui_theme.hpp"
#include "src/features/visuals/visuals.hpp"
#include "src/features/removals/removals.hpp"
#include "src/utils/log.hpp"
#include <minhook.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace render {

    static BOOL CALLBACK FindGameWindowCallback(HWND handle, LPARAM param) {
        if (GetWindow(handle, GW_OWNER) == nullptr && IsWindowVisible(handle) && handle != GetConsoleWindow()) {
            DWORD pid = 0;
            GetWindowThreadProcessId(handle, &pid);
            if (pid == GetCurrentProcessId()) {
                *reinterpret_cast<HWND*>(param) = handle;
                return FALSE;
            }
        }
        return TRUE;
    }

    static HWND ResolveGameWindow() {
        HWND window = FindWindowW(L"SDL_app", L"Counter-Strike 2");
        if (window) return window;

        EnumWindows(FindGameWindowCallback, reinterpret_cast<LPARAM>(&window));
        return window;
    }

    void RenderManager::CreateRenderTarget(IDXGISwapChain* swapChain) {
        if (!swapChain || !D3DDevice) return;

        ID3D11Texture2D* backBuffer{nullptr};
        if (SUCCEEDED(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) {
            DXGI_SWAP_CHAIN_DESC sd{};
            if (SUCCEEDED(swapChain->GetDesc(&sd))) {
                D3D11_RENDER_TARGET_VIEW_DESC desc{};
                desc.Format = (sd.BufferDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) ? DXGI_FORMAT_R8G8B8A8_UNORM : sd.BufferDesc.Format;
                desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                D3DDevice->CreateRenderTargetView(backBuffer, &desc, &RenderTargetView);
            } else {
                D3DDevice->CreateRenderTargetView(backBuffer, nullptr, &RenderTargetView);
            }
            backBuffer->Release();
        }
    }

    void RenderManager::CleanupRenderTarget() {
        if (RenderTargetView) {
            RenderTargetView->Release();
            RenderTargetView = nullptr;
        }
    }

    LRESULT CALLBACK RenderManager::HookWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        if (uMsg == WM_KEYDOWN && (wParam == VK_INSERT || wParam == VK_DELETE)) {
            IsMenuOpen = !IsMenuOpen;
            return 0;
        }

        if (InitializedImGui && IsMenuOpen) {
            ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
            ImGuiIO& io = ImGui::GetIO();
            if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
                return 1;
            }
        }

        if (OriginalWndProc) {
            return CallWindowProcW(OriginalWndProc, hWnd, uMsg, wParam, lParam);
        }
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }

    HRESULT WINAPI RenderManager::HookPresent(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags) {
        static thread_local bool inPresent = false;
        using PresentFn = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT);

        if (inPresent) {
            return reinterpret_cast<PresentFn>(OriginalPresent)(swapChain, syncInterval, flags);
        }
        inPresent = true;

        if (!InitializedImGui) {
            if (SUCCEEDED(swapChain->GetDevice(IID_PPV_ARGS(&D3DDevice)))) {
                D3DDevice->GetImmediateContext(&D3DContext);

                if (!GameWindow) {
                    GameWindow = ResolveGameWindow();
                }

                CreateRenderTarget(swapChain);

                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO();
                io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

                // Load LIXXUID UI Fonts & Vector Icons
                lixxuid::ui::ResourceManager::SetupFonts();

                if (GameWindow) {
                    ImGui_ImplWin32_Init(GameWindow);
                }
                ImGui_ImplDX11_Init(D3DDevice, D3DContext);

                InitializedImGui = true;
                utils::Logger::Info("ImGui DX11 initialized on game SwapChain successfully");
            }
        }

        if (RenderTargetView && D3DContext) {
            // Backup game render target and depth-stencil view
            ID3D11RenderTargetView* prevRTV = nullptr;
            ID3D11DepthStencilView* prevDSV = nullptr;
            D3DContext->OMGetRenderTargets(1, &prevRTV, &prevDSV);

            D3DContext->OMSetRenderTargets(1, &RenderTargetView, nullptr);

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            __try {
                // Execute continuous removal suppressions
                features::Removals::OnRender();

                // Only render entity ESP while in-game, preventing null derefs in menu
                if (core::Interfaces::EngineClient && core::Interfaces::EngineClient->IsInGame()) {
                    features::Visuals::Render();
                }
                menu::Menu::Render();
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                // Shield the game against any potential rendering exceptions
            }

            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            // Restore original render targets
            D3DContext->OMSetRenderTargets(1, &prevRTV, prevDSV);
            if (prevRTV) prevRTV->Release();
            if (prevDSV) prevDSV->Release();
        }

        HRESULT hr = reinterpret_cast<PresentFn>(OriginalPresent)(swapChain, syncInterval, flags);
        inPresent = false;
        return hr;
    }

    HRESULT WINAPI RenderManager::HookResizeBuffers(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags) {
        if (D3DContext) {
            D3DContext->OMSetRenderTargets(0, nullptr, nullptr);
        }
        CleanupRenderTarget();

        using ResizeBuffersFn = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
        HRESULT hr = reinterpret_cast<ResizeBuffersFn>(OriginalResizeBuffers)(swapChain, bufferCount, width, height, newFormat, swapChainFlags);

        CreateRenderTarget(swapChain);
        return hr;
    }

    bool RenderManager::Initialize() {
        if (!core::Interfaces::SwapChain || !core::Interfaces::SwapChain->dxgi_swapchain) {
            utils::Logger::Error("Failed to hook Present: Native SwapChain pointer not available");
            return false;
        }

        auto* swapChain = core::Interfaces::SwapChain->dxgi_swapchain;
        GameWindow = ResolveGameWindow();

        if (GameWindow) {
            OriginalWndProc = reinterpret_cast<WNDPROC>(
                SetWindowLongPtrW(GameWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HookWndProc))
            );
            utils::Logger::Info("WndProc hooked on HWND {:p}", static_cast<void*>(GameWindow));
        }

        void** vtable = *reinterpret_cast<void***>(swapChain);
        void* presentTarget = vtable[8];
        void* resizeBuffersTarget = vtable[13];

        if (MH_CreateHook(presentTarget, reinterpret_cast<void*>(&HookPresent), &OriginalPresent) != MH_OK) {
            utils::Logger::Error("MinHook failed on Present (slot 8)");
            return false;
        }

        if (MH_CreateHook(resizeBuffersTarget, reinterpret_cast<void*>(&HookResizeBuffers), &OriginalResizeBuffers) != MH_OK) {
            utils::Logger::Error("MinHook failed on ResizeBuffers (slot 13)");
            return false;
        }

        bool enabled = (MH_EnableHook(MH_ALL_HOOKS) == MH_OK);
        utils::Logger::Info("Present and ResizeBuffers hooks enabled: {}", enabled);
        return enabled;
    }

    void RenderManager::Shutdown() {
        if (OriginalWndProc && GameWindow) {
            SetWindowLongPtrW(GameWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(OriginalWndProc));
            OriginalWndProc = nullptr;
        }

        MH_DisableHook(MH_ALL_HOOKS);

        if (InitializedImGui) {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
            InitializedImGui = false;
        }

        CleanupRenderTarget();

        if (D3DContext) {
            D3DContext->Release();
            D3DContext = nullptr;
        }
        if (D3DDevice) {
            D3DDevice->Release();
            D3DDevice = nullptr;
        }
    }

} // namespace render
