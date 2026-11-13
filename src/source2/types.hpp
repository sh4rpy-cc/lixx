#pragma once
#include <cstdint>
#include <windows.h>
#include <dxgi.h>
#include "src/utils/math.hpp"

namespace source2 {

    struct CSchemaClassField {
        const char* name;
        void* type;
        uint32_t offset;
        uint32_t metadata_size;
        void* metadata;
    };

    struct CSchemaClassInfo {
        void* pointer;
        const char* name;
        const char* module_name;
    private:
        char pad0[0x8];
    public:
        uint32_t size;
        uint16_t num_fields;
    private:
        char pad1[0x2];
    public:
        uint16_t static_size;
        uint16_t metadata_size;
    private:
        char pad2[0x4];
    public:
        CSchemaClassField* fields;
    };

    struct CSchemaDeclaredClass {
        void* pointer;
        const char* name;
        const char* module_name;
        const char* unk;
        CSchemaClassInfo* class_info;
    };

    struct CSchemaDeclaredClassEntry {
        uint64_t hash[2];
        CSchemaDeclaredClass* declared_class;
    };

    struct CSchemaTypeScope {
        void* pointer;
        char name[256];

        [[nodiscard]] CSchemaClassInfo* FindDeclaredClass(const char* className) noexcept {
            auto vtable = *reinterpret_cast<void***>(this);
            if (!vtable) return nullptr;

            using Fn = CSchemaClassInfo* (__fastcall*)(void*, CSchemaClassInfo**, const char*);
            CSchemaClassInfo* classInfo = nullptr;
            auto result = reinterpret_cast<Fn>(vtable[2])(this, &classInfo, className);
            return classInfo ? classInfo : result;
        }
    };

    class ISchemaSystem {
    public:
        [[nodiscard]] CSchemaTypeScope* FindTypeScopeForModule(const char* module) noexcept {
            auto vtable = *reinterpret_cast<void***>(this);
            if (!vtable) return nullptr;
            using Fn = CSchemaTypeScope* (__thiscall*)(void*, const char*, void*);
            return reinterpret_cast<Fn>(vtable[13])(this, module, nullptr);
        }
    };

    struct CSwapChainBase {
        virtual ~CSwapChainBase() = 0;
        virtual int GetScreenWidth() = 0;
        virtual int GetScreenHeight() = 0;
    };

    struct CSwapChain : CSwapChainBase {
        char pad000[0x8];
        HWND window;
        char pad001[0x8];
        int width;
        int height;
        char pad003[0x148];
        IDXGISwapChain* dxgi_swapchain;
        char pad004[0x90];
    };

} // namespace source2
