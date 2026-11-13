#pragma once
#include <windows.h>

namespace core {

    class Hooks {
    public:
        static bool Initialize();
        static void Shutdown();

        static inline HMODULE ModuleHandle{nullptr};
        static inline bool ShouldUnload{false};
    };

} // namespace core
