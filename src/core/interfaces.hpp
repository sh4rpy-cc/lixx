#pragma once
#include <windows.h>
#include <string_view>
#include "src/source2/types.hpp"

namespace core {

    class IEngineClient {
    public:
        template <typename T, typename... Args>
        static inline T CallVFunc(void* thisPtr, size_t index, Args... args) {
            auto vtable = *reinterpret_cast<void***>(thisPtr);
            using Fn = T(__fastcall*)(void*, Args...);
            return reinterpret_cast<Fn>(vtable[index])(thisPtr, args...);
        }

        bool IsInGame() {
            // Index 35 in Source2EngineToClient001
            __try {
                return CallVFunc<bool>(this, 35);
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                return false;
            }
        }

        bool IsConnected() {
            // Index 36 in Source2EngineToClient001
            __try {
                return CallVFunc<bool>(this, 36);
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                return false;
            }
        }
    };

    class Interfaces {
    public:
        static bool Initialize();

        template <typename T>
        static T* Get(std::string_view moduleName, std::string_view interfaceName);

        static inline source2::ISchemaSystem* SchemaSystem{nullptr};
        static inline IEngineClient* EngineClient{nullptr};
        static inline void* GameEntitySystem{nullptr};
        static inline source2::CSwapChain* SwapChain{nullptr};
        static inline math::Matrix4x4* ViewMatrix{nullptr};

        // Engine entity function pointers
        using GetBaseEntityFn = void* (__fastcall*)(void*, int);
        using GetLocalControllerFn = void* (__fastcall*)(int);
        using GetLocalPawnFn = void* (__fastcall*)(int);

        static inline GetBaseEntityFn GetBaseEntity{nullptr};
        static inline GetLocalControllerFn GetLocalController{nullptr};
        static inline GetLocalPawnFn GetLocalPawn{nullptr};
    };

} // namespace core
