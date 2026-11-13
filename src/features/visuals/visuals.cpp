#include "visuals.hpp"
#include "src/source2/entity.hpp"
#include "src/core/interfaces.hpp"
#include "src/memory/pattern.hpp"
#include "src/utils/math.hpp"

#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

namespace features {

    // --- Bone connection pairs for CS2 player model ---
    struct BoneConnection {
        int bone1;
        int bone2;
    };

    static const BoneConnection g_boneConnections[] = {
        // Spine & Head
        { source2::BONE_HEAD, source2::BONE_NECK },
        { source2::BONE_NECK, source2::BONE_SPINE_3 },
        { source2::BONE_SPINE_3, source2::BONE_SPINE_2 },
        { source2::BONE_SPINE_2, source2::BONE_SPINE_1 },
        { source2::BONE_SPINE_1, source2::BONE_SPINE_0 },

        // Left Arm (Shoulder -> Elbow -> Hand)
        { source2::BONE_NECK, source2::BONE_ARM_UPPER_L },
        { source2::BONE_ARM_UPPER_L, source2::BONE_ARM_LOWER_L },
        { source2::BONE_ARM_LOWER_L, source2::BONE_HAND_L },

        // Right Arm (Shoulder -> Elbow -> Hand)
        { source2::BONE_NECK, source2::BONE_ARM_UPPER_R },
        { source2::BONE_ARM_UPPER_R, source2::BONE_ARM_LOWER_R },
        { source2::BONE_ARM_LOWER_R, source2::BONE_HAND_R },

        // Left Leg (Pelvis/Waist -> Knee -> Foot)
        { source2::BONE_SPINE_0, source2::BONE_LEG_UPPER_L },
        { source2::BONE_LEG_UPPER_L, source2::BONE_LEG_LOWER_L },
        { source2::BONE_LEG_LOWER_L, source2::BONE_ANKLE_L },

        // Right Leg (Pelvis/Waist -> Knee -> Foot)
        { source2::BONE_SPINE_0, source2::BONE_LEG_UPPER_R },
        { source2::BONE_LEG_UPPER_R, source2::BONE_LEG_LOWER_R },
        { source2::BONE_LEG_LOWER_R, source2::BONE_ANKLE_R }
    };

    // --- ViewMatrix resolution ---
    static math::Matrix4x4 GetViewMatrix() {
        if (core::Interfaces::ViewMatrix) {
            return *core::Interfaces::ViewMatrix;
        }

        static bool attempted = false;
        static uintptr_t viewMatrixAddr = 0;
        if (!attempted) {
            attempted = true;
            auto addr = memory::PatternScanner::Find("client.dll", memory::PatternScanner::Signatures::ViewMatrix).Rip();
            if (addr) viewMatrixAddr = static_cast<uintptr_t>(addr);
        }
        if (viewMatrixAddr) {
            return *reinterpret_cast<math::Matrix4x4*>(viewMatrixAddr);
        }
        return {};
    }

    // --- Drawing primitives ---

    static void DrawOutlinedText(ImDrawList* drawList, const ImVec2& pos, ImU32 color, const char* text) {
        drawList->AddText(ImVec2(pos.x - 1.f, pos.y), IM_COL32(0, 0, 0, 230), text);
        drawList->AddText(ImVec2(pos.x + 1.f, pos.y), IM_COL32(0, 0, 0, 230), text);
        drawList->AddText(ImVec2(pos.x, pos.y - 1.f), IM_COL32(0, 0, 0, 230), text);
        drawList->AddText(ImVec2(pos.x, pos.y + 1.f), IM_COL32(0, 0, 0, 230), text);
        drawList->AddText(pos, color, text);
    }

    static void DrawBoundingBox(ImDrawList* drawList, float left, float top, float right, float bottom, ImU32 color, int boxType) {
        if (boxType == 0) {
            drawList->AddRect(ImVec2(left - 1.f, top - 1.f), ImVec2(right + 1.f, bottom + 1.f), IM_COL32(0, 0, 0, 200));
            drawList->AddRect(ImVec2(left + 1.f, top + 1.f), ImVec2(right - 1.f, bottom - 1.f), IM_COL32(0, 0, 0, 200));
            drawList->AddRect(ImVec2(left, top), ImVec2(right, bottom), color);
        } else {
            float w = right - left;
            float h = bottom - top;
            float lineW = w * 0.25f;
            float lineH = h * 0.25f;

            auto drawCorner = [&](float x1, float y1, float x2, float y2, ImU32 col, float thickness = 1.0f) {
                drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), col, thickness);
            };

            drawCorner(left - 1, top - 1, left + lineW + 1, top - 1, IM_COL32(0, 0, 0, 200), 3.0f);
            drawCorner(left - 1, top - 1, left - 1, top + lineH + 1, IM_COL32(0, 0, 0, 200), 3.0f);
            drawCorner(right + 1, top - 1, right - lineW - 1, top - 1, IM_COL32(0, 0, 0, 200), 3.0f);
            drawCorner(right + 1, top - 1, right + 1, top + lineH + 1, IM_COL32(0, 0, 0, 200), 3.0f);
            drawCorner(left - 1, bottom + 1, left + lineW + 1, bottom + 1, IM_COL32(0, 0, 0, 200), 3.0f);
            drawCorner(left - 1, bottom + 1, left - 1, bottom - lineH - 1, IM_COL32(0, 0, 0, 200), 3.0f);
            drawCorner(right + 1, bottom + 1, right - lineW - 1, bottom + 1, IM_COL32(0, 0, 0, 200), 3.0f);
            drawCorner(right + 1, bottom + 1, right + 1, bottom - lineH - 1, IM_COL32(0, 0, 0, 200), 3.0f);

            drawCorner(left, top, left + lineW, top, color);
            drawCorner(left, top, left, top + lineH, color);
            drawCorner(right, top, right - lineW, top, color);
            drawCorner(right, top, right, top + lineH, color);
            drawCorner(left, bottom, left + lineW, bottom, color);
            drawCorner(left, bottom, left, bottom - lineH, color);
            drawCorner(right, bottom, right - lineW, bottom, color);
            drawCorner(right, bottom, right, bottom - lineH, color);
        }
    }

    static void DrawHealthBar(ImDrawList* drawList, float left, float top, float bottom, int health, bool drawText) {
        float clampedHp = std::clamp(static_cast<float>(health), 0.0f, 100.0f);
        float height = bottom - top;
        float barHeight = height * (clampedHp / 100.0f);

        float barLeft = left - 6.0f;
        float barRight = left - 2.0f;

        drawList->AddRectFilled(ImVec2(barLeft - 1.f, top - 1.f), ImVec2(barRight + 1.f, bottom + 1.f), IM_COL32(10, 10, 10, 220));

        int r = static_cast<int>((100.0f - clampedHp) * 2.55f);
        int g = static_cast<int>(clampedHp * 2.55f);
        ImU32 hpColor = IM_COL32(std::min(255, r), std::min(255, g), 40, 255);

        drawList->AddRectFilled(ImVec2(barLeft, bottom - barHeight), ImVec2(barRight, bottom), hpColor);

        if (drawText && health < 100 && health > 0) {
            std::string hpStr = std::to_string(health);
            DrawOutlinedText(drawList, ImVec2(barLeft - 18.0f, bottom - barHeight - 4.0f), IM_COL32(255, 255, 255, 230), hpStr.c_str());
        }
    }

    static void DrawArmorBar(ImDrawList* drawList, float left, float top, float bottom, int armor) {
        if (armor <= 0) return;

        float clampedArmor = std::clamp(static_cast<float>(armor), 0.0f, 100.0f);
        float height = bottom - top;
        float barHeight = height * (clampedArmor / 100.0f);

        float barLeft = left - 11.0f;
        float barRight = left - 8.0f;

        drawList->AddRectFilled(ImVec2(barLeft - 1.f, top - 1.f), ImVec2(barRight + 1.f, bottom + 1.f), IM_COL32(10, 10, 10, 220));
        drawList->AddRectFilled(ImVec2(barLeft, bottom - barHeight), ImVec2(barRight, bottom), IM_COL32(40, 160, 255, 255));
    }

    static void DrawSkeleton(ImDrawList* drawList, source2::C_CSPlayerPawn* pawn, const math::Matrix4x4& viewMatrix, float screenW, float screenH, ImU32 color, float thickness, bool drawJoints) {
        auto sceneNode = pawn->GetGameSceneNode();
        if (!sceneNode) return;

        math::Vector3 origin = sceneNode->GetOrigin();

        for (const auto& conn : g_boneConnections) {
            math::Vector3 pos1, pos2;
            if (!sceneNode->GetBonePosition(conn.bone1, pos1) || !sceneNode->GetBonePosition(conn.bone2, pos2)) {
                continue;
            }

            // Reject invalid/zero positions and extreme deviations
            if (pos1.LengthSqr() < 1.0f || pos2.LengthSqr() < 1.0f) continue;
            if (pos1.DistTo(origin) > 160.0f || pos2.DistTo(origin) > 160.0f) continue;

            math::Vector2 s1, s2;
            if (!math::WorldToScreen(pos1, s1, viewMatrix, screenW, screenH) ||
                !math::WorldToScreen(pos2, s2, viewMatrix, screenW, screenH)) {
                continue;
            }

            drawList->AddLine(ImVec2(s1.x, s1.y), ImVec2(s2.x, s2.y), IM_COL32(0, 0, 0, 200), thickness + 1.5f);
            drawList->AddLine(ImVec2(s1.x, s1.y), ImVec2(s2.x, s2.y), color, thickness);

            if (drawJoints) {
                drawList->AddCircleFilled(ImVec2(s1.x, s1.y), 2.2f, IM_COL32(255, 255, 255, 220));
                drawList->AddCircleFilled(ImVec2(s2.x, s2.y), 2.2f, IM_COL32(255, 255, 255, 220));
            }
        }
    }

    static void DrawPlayerFlags(ImDrawList* drawList, source2::C_CSPlayerPawn* pawn, float right, float top) {
        float currentY = top;
        float startX = right + 4.0f;

        auto renderFlag = [&](const char* text, ImU32 color) {
            DrawOutlinedText(drawList, ImVec2(startX, currentY), color, text);
            currentY += 10.0f;
        };

        if (pawn->IsDefusing()) {
            renderFlag("DEFUSING", IM_COL32(255, 60, 60, 255));
        }

        if (pawn->IsScoped()) {
            renderFlag("SCOPED", IM_COL32(80, 200, 255, 255));
        }

        if (pawn->GetFlashDuration() > 0.1f) {
            renderFlag("FLASHED", IM_COL32(255, 200, 40, 255));
        }

        if (pawn->GetArmorValue() > 0) {
            renderFlag("HK", IM_COL32(220, 220, 220, 230));
        }
    }

    // --- Main Visuals Loop ---
    void Visuals::Render() {
        if (!Config.m_bEnabled) return;

        auto drawList = ImGui::GetBackgroundDrawList();
        if (!drawList) return;

        ImGuiIO& io = ImGui::GetIO();
        float screenW = io.DisplaySize.x;
        float screenH = io.DisplaySize.y;

        math::Matrix4x4 viewMatrix = GetViewMatrix();

        auto localController = source2::EntitySystem::GetLocalPlayerController();
        auto localPawn = source2::EntitySystem::GetLocalPlayerPawn();
        uint8_t localTeam = localController ? localController->GetTeamNum() : 0;
        math::Vector3 localOrigin = (localPawn && localPawn->GetGameSceneNode()) ? localPawn->GetGameSceneNode()->GetOrigin() : math::Vector3{};

        for (int i = 1; i <= 64; ++i) {
            auto entity = source2::EntitySystem::GetBaseEntity(i);
            if (!entity) continue;

            auto controller = reinterpret_cast<source2::C_CSPlayerController*>(entity);
            auto pawn = controller->GetPlayerPawn();
            if (!pawn || !pawn->IsAlive()) continue;

            bool isLocal = (pawn == localPawn);
            if (isLocal && !Config.m_bLocalPlayer) continue;

            if (Config.m_bTeamCheck && !isLocal && controller->GetTeamNum() == localTeam) {
                continue;
            }

            auto sceneNode = pawn->GetGameSceneNode();
            if (!sceneNode) continue;

            math::Vector3 origin = sceneNode->GetOrigin();
            math::Vector3 topPos = origin + math::Vector3(0.f, 0.f, 72.f);

            math::Vector2 screenOrigin, screenTop;
            if (!math::WorldToScreen(origin, screenOrigin, viewMatrix, screenW, screenH) ||
                !math::WorldToScreen(topPos, screenTop, viewMatrix, screenW, screenH)) {
                continue;
            }

            float height = screenOrigin.y - screenTop.y;
            float width = height * 0.45f;
            float left = screenTop.x - (width * 0.5f);
            float right = screenTop.x + (width * 0.5f);
            float top = screenTop.y;
            float bottom = screenOrigin.y;

            // 1. Bounding Box
            if (Config.m_bBox) {
                ImU32 boxCol = ImColor(Config.m_flBoxColor[0], Config.m_flBoxColor[1], Config.m_flBoxColor[2], Config.m_flBoxColor[3]);
                DrawBoundingBox(drawList, left, top, right, bottom, boxCol, Config.m_iBoxType);
            }

            // 2. Health & Armor Bars
            if (Config.m_bHealthBar) {
                DrawHealthBar(drawList, left, top, bottom, pawn->GetHealth(), Config.m_bHealthText);
            }
            if (Config.m_bArmorBar) {
                DrawArmorBar(drawList, left, top, bottom, pawn->GetArmorValue());
            }

            // 3. Skeleton (Bones)
            if (Config.m_bSkeleton) {
                ImU32 skelCol = ImColor(Config.m_flSkeletonColor[0], Config.m_flSkeletonColor[1], Config.m_flSkeletonColor[2], Config.m_flSkeletonColor[3]);
                DrawSkeleton(drawList, pawn, viewMatrix, screenW, screenH, skelCol, Config.m_flSkeletonThickness, Config.m_bSkeletonJoints);
            }

            // 4. Player Name
            if (Config.m_bName) {
                const char* playerName = controller->GetPlayerName();
                if (playerName && *playerName) {
                    ImVec2 textSize = ImGui::CalcTextSize(playerName);
                    float nameX = screenTop.x - (textSize.x * 0.5f);
                    float nameY = top - textSize.y - 2.0f;
                    ImU32 nameCol = ImColor(Config.m_flNameColor[0], Config.m_flNameColor[1], Config.m_flNameColor[2], Config.m_flNameColor[3]);
                    DrawOutlinedText(drawList, ImVec2(nameX, nameY), nameCol, playerName);
                }
            }

            // 5. Active Weapon
            float bottomTextY = bottom + 2.0f;
            if (Config.m_bWeapon) {
                auto activeWeapon = pawn->GetActiveWeapon();
                if (activeWeapon) {
                    std::string_view weaponName = activeWeapon->GetWeaponName();
                    ImVec2 weaponSize = ImGui::CalcTextSize(weaponName.data());
                    float weaponX = screenTop.x - (weaponSize.x * 0.5f);
                    ImU32 wepCol = ImColor(Config.m_flWeaponColor[0], Config.m_flWeaponColor[1], Config.m_flWeaponColor[2], Config.m_flWeaponColor[3]);
                    DrawOutlinedText(drawList, ImVec2(weaponX, bottomTextY), wepCol, weaponName.data());
                    bottomTextY += weaponSize.y + 1.0f;
                }
            }

            // 6. Distance
            if (Config.m_bDistance && localPawn) {
                float distMeters = (origin - localOrigin).Length() * 0.0254f;
                std::string distStr = std::to_string(static_cast<int>(distMeters)) + "m";
                ImVec2 distSize = ImGui::CalcTextSize(distStr.c_str());
                float distX = screenTop.x - (distSize.x * 0.5f);
                DrawOutlinedText(drawList, ImVec2(distX, bottomTextY), IM_COL32(200, 200, 200, 220), distStr.c_str());
            }

            // 7. Status Flags
            if (Config.m_bFlags) {
                DrawPlayerFlags(drawList, pawn, right, top);
            }

            // 8. Head Dot
            if (Config.m_bHeadDot) {
                math::Vector3 headPos;
                if (sceneNode->GetBonePosition(source2::BONE_HEAD, headPos)) {
                    math::Vector2 headScreen;
                    if (math::WorldToScreen(headPos, headScreen, viewMatrix, screenW, screenH)) {
                        ImU32 dotCol = ImColor(Config.m_flHeadDotColor[0], Config.m_flHeadDotColor[1], Config.m_flHeadDotColor[2], Config.m_flHeadDotColor[3]);
                        drawList->AddCircleFilled(ImVec2(headScreen.x, headScreen.y), 3.5f, dotCol);
                        drawList->AddCircle(ImVec2(headScreen.x, headScreen.y), 3.5f, IM_COL32(0, 0, 0, 200), 12, 1.0f);
                    }
                }
            }

            // 9. Snaplines
            if (Config.m_bSnaplines) {
                ImU32 snapCol = ImColor(Config.m_flSnaplinesColor[0], Config.m_flSnaplinesColor[1], Config.m_flSnaplinesColor[2], Config.m_flSnaplinesColor[3]);
                drawList->AddLine(ImVec2(screenW * 0.5f, screenH), ImVec2(screenOrigin.x, screenOrigin.y), snapCol);
            }
        }
    }

} // namespace features
