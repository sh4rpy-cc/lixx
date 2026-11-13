#pragma once
#include "address.hpp"
#include <string_view>
#include <vector>
#include <windows.h>

namespace memory {

    struct ModuleInfo {
        uintptr_t base{0};
        size_t size{0};
        std::string_view name;
    };

    class PatternScanner {
    public:
        static ModuleInfo GetModule(std::string_view moduleName);
        static Address Find(std::string_view moduleName, std::string_view pattern);
        static Address FindInRange(uintptr_t start, size_t size, std::string_view pattern);

        // Predefined verified CS2 patterns
        struct Signatures {
            static inline constexpr std::string_view ViewMatrix = "48 8D 0D ? ? ? ? 48 C1 E0 06";
            static inline constexpr std::string_view EntitySystem = "48 8B 0D ? ? ? ? 48 8B 01 FF 50 ? 48 8B 0D";
            static inline constexpr std::string_view LocalPlayerController = "48 8B 05 ? ? ? ? 48 85 C0 74 ? 8B 88";
            static inline constexpr std::string_view GlobalVars = "48 89 0D ? ? ? ? 48 89 42";
            static inline constexpr std::string_view CSGOInput = "48 8B 0D ? ? ? ? 48 8B 01 FF 50 28 8B 83";
            static inline constexpr std::string_view SetDepthState = "48 8B 81 58 0A 00 00 48 85 C0 74 0A 44 8B 81 60";
            static inline constexpr std::string_view SetDepthOverride = "48 89 91 58 0A 00 00 44 89 81 60 0A 00 00 C3 CC";
        };

    private:
        static std::vector<int> PatternToBytes(std::string_view pattern);
    };

} // namespace memory
