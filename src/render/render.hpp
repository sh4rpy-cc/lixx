#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include <windows.h>

namespace render {

    class RenderManager {
    public:
        static bool Initialize();
        static void Shutdown();

        static inline bool IsMenuOpen{false};
        static inline HWND GameWindow{nullptr};
        static inline ID3D11Device* D3DDevice{nullptr};
        static inline ID3D11DeviceContext* D3DContext{nullptr};
        static inline ID3D11RenderTargetView* RenderTargetView{nullptr};

    private:
        static void CreateRenderTarget(IDXGISwapChain* swapChain);
        static void CleanupRenderTarget();

        static HRESULT WINAPI HookPresent(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);
        static HRESULT WINAPI HookResizeBuffers(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags);
        static LRESULT CALLBACK HookWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        static inline void* OriginalPresent{nullptr};
        static inline void* OriginalResizeBuffers{nullptr};
        static inline WNDPROC OriginalWndProc{nullptr};
        static inline bool InitializedImGui{false};
    };

} // namespace render
