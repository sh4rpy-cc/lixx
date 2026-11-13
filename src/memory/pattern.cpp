#include "pattern.hpp"
#include <charconv>
#include <unordered_map>
#include <mutex>

namespace memory {

    static std::unordered_map<std::string_view, ModuleInfo> g_moduleCache;
    static std::mutex g_cacheMutex;

    ModuleInfo PatternScanner::GetModule(std::string_view moduleName) {
        std::lock_guard lock(g_cacheMutex);
        if (auto it = g_moduleCache.find(moduleName); it != g_moduleCache.end()) {
            return it->second;
        }

        HMODULE handle = GetModuleHandleA(moduleName.data());
        if (!handle) return {};

        auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(handle);
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) return {};

        auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(
            reinterpret_cast<uintptr_t>(handle) + dosHeader->e_lfanew
        );
        if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) return {};

        ModuleInfo info{
            .base = reinterpret_cast<uintptr_t>(handle),
            .size = static_cast<size_t>(ntHeaders->OptionalHeader.SizeOfImage),
            .name = moduleName
        };

        g_moduleCache[moduleName] = info;
        return info;
    }

    std::vector<int> PatternScanner::PatternToBytes(std::string_view pattern) {
        std::vector<int> bytes;
        const char* start = pattern.data();
        const char* end = start + pattern.size();

        for (const char* current = start; current < end; ++current) {
            if (*current == ' ') continue;

            if (*current == '?') {
                bytes.push_back(-1);
                if (current + 1 < end && *(current + 1) == '?') {
                    ++current;
                }
            } else {
                char hex[3] = { current[0], (current + 1 < end && current[1] != ' ') ? current[1] : '\0', '\0' };
                int val = 0;
                auto res = std::from_chars(hex, hex + 2, val, 16);
                if (res.ec == std::errc{}) {
                    bytes.push_back(val);
                    if (hex[1] != '\0') ++current;
                }
            }
        }
        return bytes;
    }

    Address PatternScanner::Find(std::string_view moduleName, std::string_view pattern) {
        auto mod = GetModule(moduleName);
        if (!mod.base || !mod.size) return Address{0};
        return FindInRange(mod.base, mod.size, pattern);
    }

    Address PatternScanner::FindInRange(uintptr_t start, size_t size, std::string_view pattern) {
        auto patternBytes = PatternToBytes(pattern);
        if (patternBytes.empty()) return Address{0};

        const size_t patternSize = patternBytes.size();
        const auto* scanBytes = reinterpret_cast<const uint8_t*>(start);

        for (size_t i = 0; i <= size - patternSize; ++i) {
            bool found = true;
            for (size_t j = 0; j < patternSize; ++j) {
                if (patternBytes[j] != -1 && scanBytes[i + j] != static_cast<uint8_t>(patternBytes[j])) {
                    found = false;
                    break;
                }
            }
            if (found) {
                return Address{start + i};
            }
        }
        return Address{0};
    }

} // namespace memory
