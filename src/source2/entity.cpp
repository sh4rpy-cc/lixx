#include "entity.hpp"
#include "src/core/interfaces.hpp"

namespace source2 {

    C_BaseEntity* EntitySystem::GetBaseEntity(int index) {
        if (!core::Interfaces::GameEntitySystem || !core::Interfaces::GetBaseEntity || index <= 0) {
            return nullptr;
        }

        __try {
            return reinterpret_cast<C_BaseEntity*>(core::Interfaces::GetBaseEntity(core::Interfaces::GameEntitySystem, index));
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return nullptr;
        }
    }

    C_BaseEntity* EntitySystem::GetEntityFromHandle(uint32_t handle) {
        if (!handle || handle == 0xFFFFFFFF) return nullptr;
        int index = static_cast<int>(handle & 0x7FFF);
        return GetBaseEntity(index);
    }

    C_CSPlayerController* EntitySystem::GetLocalPlayerController() {
        if (!core::Interfaces::GetLocalController) return nullptr;
        __try {
            return reinterpret_cast<C_CSPlayerController*>(core::Interfaces::GetLocalController(0));
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return nullptr;
        }
    }

    C_CSPlayerPawn* EntitySystem::GetLocalPlayerPawn() {
        if (!core::Interfaces::GetLocalPawn) return nullptr;
        __try {
            return reinterpret_cast<C_CSPlayerPawn*>(core::Interfaces::GetLocalPawn(0));
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return nullptr;
        }
    }

    C_CSPlayerPawn* C_CSPlayerController::GetPlayerPawn() {
        __try {
            uint32_t handle = GetPawnHandle();
            return reinterpret_cast<C_CSPlayerPawn*>(EntitySystem::GetEntityFromHandle(handle));
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return nullptr;
        }
    }

    C_CSWeaponBase* C_CSPlayerPawn::GetActiveWeapon() {
        __try {
            auto ws = GetWeaponServices();
            if (!ws) return nullptr;
            uint32_t handle = ws->GetActiveWeaponHandle();
            return reinterpret_cast<C_CSWeaponBase*>(EntitySystem::GetEntityFromHandle(handle));
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return nullptr;
        }
    }

    std::string_view C_CSWeaponBase::GetWeaponName() {
        __try {
            auto attr = GetAttributeManager();
            if (!attr) return "Unknown";
            auto item = attr->GetItem();
            if (!item) return "Unknown";

            uint16_t defIndex = item->GetItemDefinitionIndex();
            switch (defIndex) {
                case 1:  return "Desert Eagle";
                case 2:  return "Dual Berettas";
                case 3:  return "Five-SeveN";
                case 4:  return "Glock-18";
                case 7:  return "AK-47";
                case 8:  return "AUG";
                case 9:  return "AWP";
                case 10: return "FAMAS";
                case 11: return "G3SG1";
                case 13: return "Galil AR";
                case 14: return "M249";
                case 16: return "M4A4";
                case 17: return "MAC-10";
                case 19: return "P90";
                case 23: return "MP5-SD";
                case 24: return "UMP-45";
                case 25: return "XM1014";
                case 26: return "PP-Bizon";
                case 27: return "MAG-7";
                case 28: return "Negev";
                case 29: return "Sawed-Off";
                case 30: return "Tec-9";
                case 31: return "Zeus x27";
                case 32: return "P2000";
                case 33: return "MP7";
                case 34: return "MP9";
                case 35: return "Nova";
                case 36: return "P250";
                case 38: return "SCAR-20";
                case 39: return "SG 553";
                case 40: return "SSG 08";
                case 42: return "Knife";
                case 43: return "Flashbang";
                case 44: return "HE Grenade";
                case 45: return "Smoke Grenade";
                case 46: return "Molotov";
                case 47: return "Decoy";
                case 48: return "Incendiary";
                case 49: return "C4 Explosive";
                case 60: return "M4A1-S";
                case 61: return "USP-S";
                case 63: return "CZ75-Auto";
                case 64: return "R8 Revolver";
                default:
                    if (defIndex >= 500) return "Knife";
                    return "Weapon";
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return "Weapon";
        }
    }

} // namespace source2
