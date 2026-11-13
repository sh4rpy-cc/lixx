#pragma once
#include <cstdint>
#include <string_view>
#include <windows.h>

namespace source2 {

    struct ResourceBinding_t {
        void* pData;
    };

    template <typename T>
    class CStrongHandle {
    public:
        operator T* () const {
            if (!pBinding) return nullptr;
            return static_cast<T*>(pBinding->pData);
        }

        T* operator->() const {
            if (!pBinding) return nullptr;
            return static_cast<T*>(pBinding->pData);
        }

        const ResourceBinding_t* pBinding{nullptr};
    };

    class CMaterial2 {
    public:
        virtual const char* GetName() = 0;
        virtual const char* GetShareName() = 0;

        CMaterial2* GetForView(void* scene_view) {
            if (!scene_view) return this;
            // Slot 6: GetForView
            auto vtable = *reinterpret_cast<void***>(this);
            using fn_t = CMaterial2* (__fastcall*)(CMaterial2*, void*);
            return reinterpret_cast<fn_t>(vtable[6])(this, reinterpret_cast<uint8_t*>(scene_view) + 0x30);
        }
    };

    class CSceneAnimatableObject {
    public:
        uint8_t pad0[0xC0];
        uint32_t hOwner; // 0xC0: CBaseHandle
    };

    class CBaseSceneData {
    public:
        uint8_t pad0[0x18];                       // 0x00
        CSceneAnimatableObject* pSceneAnimatableObject; // 0x18
        CMaterial2* pMaterial;                    // 0x20
        CMaterial2* pMaterialForView;             // 0x28
        uint8_t pad1[0x20];                       // 0x30
        uint8_t r;                                // 0x50
        uint8_t g;                                // 0x51
        uint8_t b;                                // 0x52
        uint8_t a;                                // 0x53
        uint8_t pad2[0x1C];                       // 0x54
    };

    static_assert(sizeof(CBaseSceneData) == 0x70);
    static_assert(offsetof(CBaseSceneData, pSceneAnimatableObject) == 0x18);
    static_assert(offsetof(CBaseSceneData, pMaterial) == 0x20);
    static_assert(offsetof(CBaseSceneData, pMaterialForView) == 0x28);
    static_assert(offsetof(CBaseSceneData, r) == 0x50);

    struct KV3ID_t {
        const char* szName;
        uint64_t unk0;
        uint64_t unk1;
    };

    class CKeyValues3 {
    public:
        uint8_t pad0[0x100];
        uint64_t uKey;
        void* pValue;
        uint8_t pad1[0x8];

        static CKeyValues3* CreateMaterialResource();
        bool LoadFromBuffer(const char* szVmat);
    };

} // namespace source2
