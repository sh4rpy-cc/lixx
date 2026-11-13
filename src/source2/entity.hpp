#pragma once
#include <windows.h>
#include <cstdint>
#include <string_view>
#include "src/memory/schema.hpp"
#include "src/utils/math.hpp"

namespace source2 {

    enum EBoneIndex : int {
        BONE_PELVIS = 0,
        BONE_SPINE_0 = 1,
        BONE_SPINE_1 = 2,
        BONE_SPINE_2 = 3,
        BONE_SPINE_3 = 4,
        BONE_NECK = 5,
        BONE_HEAD = 6,

        // Left arm
        BONE_ARM_CLAVICLE_L = 7,
        BONE_ARM_UPPER_L = 8,
        BONE_ARM_LOWER_L = 9,
        BONE_HAND_L = 10,

        // Right arm
        BONE_ARM_CLAVICLE_R = 12,
        BONE_ARM_UPPER_R = 13,
        BONE_ARM_LOWER_R = 14,
        BONE_HAND_R = 15,

        // Left leg
        BONE_LEG_UPPER_L = 22,
        BONE_LEG_LOWER_L = 23,
        BONE_ANKLE_L = 24,

        // Right leg
        BONE_LEG_UPPER_R = 25,
        BONE_LEG_LOWER_R = 26,
        BONE_ANKLE_R = 27,
        BONE_MAX = 32
    };

    struct CBoneData {
        math::Vector3 pos;
        float scale;
        float rot[4];
    };

    class CGameSceneNode {
    public:
        SCHEMA_FIELD(math::Vector3, GetOrigin, "client.dll", "CGameSceneNode", "m_vecAbsOrigin");
        SCHEMA_FIELD(math::Vector3, GetRenderOrigin, "client.dll", "CGameSceneNode", "m_vRenderOrigin");

        bool GetBonePosition(int boneIndex, math::Vector3& outPos) {
            static int32_t modelStateOffset = 0;
            static int32_t boneArrayOffset = 0;
            static bool offsetsResolved = false;

            if (!offsetsResolved) {
                modelStateOffset = SchemaSystem::GetOffset("client.dll", "CSkeletonInstance", "m_modelState");
                if (modelStateOffset == 0) modelStateOffset = 0x160;

                boneArrayOffset = SchemaSystem::GetOffset("client.dll", "CModelState", "m_boneArray");
                if (boneArrayOffset == 0) boneArrayOffset = 0x80;

                offsetsResolved = true;
            }

            if (!this) return false;

            __try {
                uintptr_t modelStateAddr = reinterpret_cast<uintptr_t>(this) + modelStateOffset;
                CBoneData* boneArray = *reinterpret_cast<CBoneData**>(modelStateAddr + boneArrayOffset);
                if (!boneArray) return false;

                outPos = boneArray[boneIndex].pos;
                return true;
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                return false;
            }
        }
    };

    class CCollisionProperty {
    public:
        SCHEMA_FIELD(math::Vector3, GetMins, "client.dll", "CCollisionProperty", "m_vecMins");
        SCHEMA_FIELD(math::Vector3, GetMaxs, "client.dll", "CCollisionProperty", "m_vecMaxs");
    };

    class C_EconItemView {
    public:
        SCHEMA_FIELD(uint16_t, GetItemDefinitionIndex, "client.dll", "C_EconItemView", "m_iItemDefinitionIndex");
    };

    class C_AttributeContainer {
    public:
        SCHEMA_FIELD(C_EconItemView*, GetItem, "client.dll", "C_AttributeContainer", "m_Item");
    };

    class C_CSWeaponBase {
    public:
        SCHEMA_FIELD(int32_t, GetClip1, "client.dll", "C_BasePlayerWeapon", "m_iClip1");
        SCHEMA_FIELD(int32_t, GetReserveAmmo, "client.dll", "C_BasePlayerWeapon", "m_pReserveAmmo");
        SCHEMA_FIELD(C_AttributeContainer*, GetAttributeManager, "client.dll", "C_EconEntity", "m_AttributeManager");

        [[nodiscard]] std::string_view GetWeaponName();
    };

    class CPlayer_WeaponServices {
    public:
        SCHEMA_FIELD(uint32_t, GetActiveWeaponHandle, "client.dll", "CPlayer_WeaponServices", "m_hActiveWeapon");
    };

    class C_BaseEntity {
    public:
        SCHEMA_FIELD(int32_t, GetHealth, "client.dll", "C_BaseEntity", "m_iHealth");
        SCHEMA_FIELD(uint8_t, GetLifeState, "client.dll", "C_BaseEntity", "m_lifeState");
        SCHEMA_FIELD(uint8_t, GetTeamNum, "client.dll", "C_BaseEntity", "m_iTeamNum");
        SCHEMA_FIELD(CGameSceneNode*, GetGameSceneNode, "client.dll", "C_BaseEntity", "m_pGameSceneNode");
        SCHEMA_FIELD(CCollisionProperty*, GetCollision, "client.dll", "C_BaseEntity", "m_pCollision");

        [[nodiscard]] bool IsAlive() {
            return GetLifeState() == 0 && GetHealth() > 0;
        }
    };

    class C_CSPlayerPawn : public C_BaseEntity {
    public:
        SCHEMA_FIELD(int32_t, GetArmorValue, "client.dll", "C_CSPlayerPawn", "m_ArmorValue");
        SCHEMA_FIELD(bool, IsDefusing, "client.dll", "C_CSPlayerPawn", "m_bIsDefusing");
        SCHEMA_FIELD(bool, IsScoped, "client.dll", "C_CSPlayerPawn", "m_bIsScoped");
        SCHEMA_FIELD(float, GetFlashDuration, "client.dll", "C_CSPlayerPawnBase", "m_flFlashDuration");
        SCHEMA_FIELD(float, GetFlashMaxAlpha, "client.dll", "C_CSPlayerPawnBase", "m_flFlashMaxAlpha");

        void SetFlashDuration(float val) {
            static const int32_t offset = source2::SchemaSystem::GetOffset("client.dll", "C_CSPlayerPawnBase", "m_flFlashDuration");
            if (offset > 0) {
                *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(this) + offset) = val;
            }
        }

        void SetFlashMaxAlpha(float val) {
            static const int32_t offset = source2::SchemaSystem::GetOffset("client.dll", "C_CSPlayerPawnBase", "m_flFlashMaxAlpha");
            if (offset > 0) {
                *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(this) + offset) = val;
            }
        }
        SCHEMA_FIELD(CPlayer_WeaponServices*, GetWeaponServices, "client.dll", "C_CSPlayerPawn", "m_pWeaponServices");

        [[nodiscard]] C_CSWeaponBase* GetActiveWeapon();
    };

    class C_CSPlayerController : public C_BaseEntity {
    public:
        SCHEMA_FIELD(const char*, GetPlayerName, "client.dll", "CCSPlayerController", "m_sSanitizedPlayerName");
        SCHEMA_FIELD(uint32_t, GetPawnHandle, "client.dll", "CCSPlayerController", "m_hPlayerPawn");
        SCHEMA_FIELD(bool, IsPawnAlive, "client.dll", "CCSPlayerController", "m_bPawnIsAlive");

        [[nodiscard]] C_CSPlayerPawn* GetPlayerPawn();
    };

    class EntitySystem {
    public:
        static C_BaseEntity* GetBaseEntity(int index);
        static C_BaseEntity* GetEntityFromHandle(uint32_t handle);
        static C_CSPlayerController* GetLocalPlayerController();
        static C_CSPlayerPawn* GetLocalPlayerPawn();
    };

} // namespace source2
