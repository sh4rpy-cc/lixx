#pragma once
#include <imgui.h>
#include <cstdint>

namespace features {

    struct VisualsConfig {
        bool m_bEnabled{true};
        bool m_bTeamCheck{true};
        bool m_bLocalPlayer{false};

        // Bounding Box
        bool m_bBox{true};
        int  m_iBoxType{0}; // 0 = Full outline, 1 = Corner
        float m_flBoxColor[4]{1.0f, 1.0f, 1.0f, 1.0f};

        // Skeleton
        bool m_bSkeleton{true};
        bool m_bSkeletonJoints{true};
        float m_flSkeletonThickness{1.5f};
        float m_flSkeletonColor[4]{0.9f, 0.9f, 0.9f, 0.9f};

        // Bars
        bool m_bHealthBar{true};
        bool m_bHealthText{true};
        bool m_bArmorBar{true};

        // Text labels
        bool m_bName{true};
        float m_flNameColor[4]{1.0f, 1.0f, 1.0f, 1.0f};

        bool m_bWeapon{true};
        float m_flWeaponColor[4]{0.8f, 0.8f, 0.8f, 1.0f};

        bool m_bDistance{true};
        bool m_bFlags{true};

        // Extras
        bool m_bHeadDot{false};
        float m_flHeadDotColor[4]{1.0f, 0.2f, 0.2f, 1.0f};

        bool m_bSnaplines{false};
        float m_flSnaplinesColor[4]{1.0f, 1.0f, 1.0f, 0.6f};
    };

    class Visuals {
    public:
        static void Render();
        static inline VisualsConfig Config{};
    };

} // namespace features
