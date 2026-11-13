#pragma once
#include "material.hpp"
#include <d3d11.h>
#include <cstdint>
#include <vector>

namespace features {

    enum class ChamsMaterial : int {
        Solid = 0,
        Special = 1,
        Illum = 2,
        Hologram = 3,
        Glitch = 4,
        Diamond = 5,
        End = 6
    };

    struct ChamsConfig {
        bool m_bEnabled{false};
        bool m_bIgnoreZ{true};
        ChamsMaterial m_Material{ChamsMaterial::Solid};

        float m_flVisibleColor[4]{0.2f, 0.7f, 1.0f, 1.0f};
        float m_flOccludedColor[4]{1.0f, 0.2f, 0.4f, 1.0f};

        bool m_bTeamCheck{true};
        bool m_bLocalPlayer{false};
    };

    struct CustomMaterialPair {
        source2::CStrongHandle<source2::CMaterial2> visible;
        source2::CStrongHandle<source2::CMaterial2> occluded;
    };

    class Chams {
    public:
        static void Initialize();
        static void Shutdown();

        static bool SetupMaterials();
        static void HookDrawArray();
        static void UnhookDrawArray();

        // The hooked DrawArray callback
        static void __fastcall HookedDrawArray(
            void* pAnimatableSceneObjectDesc,
            void* pDx11,
            source2::CBaseSceneData* data,
            int count,
            void* pSceneView,
            void* pSceneLayer,
            void* pUnk
        );

        static inline ChamsConfig Config{};

    private:
        static inline bool m_bInitialized{false};
        static inline bool m_bMaterialsCreated{false};
        static inline CustomMaterialPair m_Materials[static_cast<int>(ChamsMaterial::End)]{};

        using DrawArray_t = void(__fastcall*)(void*, void*, source2::CBaseSceneData*, int, void*, void*, void*);
        static inline DrawArray_t OriginalDrawArray{nullptr};
        static inline void* m_pDrawArrayTarget{nullptr};
    };

} // namespace features
