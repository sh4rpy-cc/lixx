#include <windows.h>
#include <thread>
#include <chrono>
#include "hooks.hpp"
#include "src/utils/log.hpp"

DWORD WINAPI OnAttach(LPVOID lpParameter) {
    auto hModule = static_cast<HMODULE>(lpParameter);
    core::Hooks::ModuleHandle = hModule;

    utils::Logger::Attach("LIXXUID // CS2 Internal Debug");

    // Wait until CS2 main modules are loaded
    while (!GetModuleHandleA("client.dll") ||
           !GetModuleHandleA("engine2.dll") ||
           !GetModuleHandleA("schemasystem.dll") ||
           !GetModuleHandleA("rendersystemdx11.dll")) {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    // Small delay to ensure engine structures are populated
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (!core::Hooks::Initialize()) {
        utils::Logger::Error("Failed to initialize hooks. Exiting cleanly...");
        core::Hooks::Shutdown();
    }

    // Main thread lifecycle loop
    while (!core::Hooks::ShouldUnload) {
        if (GetAsyncKeyState(VK_END) & 0x8000) {
            core::Hooks::Shutdown();
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Clean exit
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    utils::Logger::Detach();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        HANDLE hThread = CreateThread(nullptr, 0, OnAttach, hModule, 0, nullptr);
        if (hThread) {
            CloseHandle(hThread);
        }
    }
    return TRUE;
}
