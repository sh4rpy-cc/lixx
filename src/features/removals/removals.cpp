#include "removals.hpp"
#include "src/utils/log.hpp"
#include "src/memory/pattern.hpp"
#include "src/source2/entity.hpp"
#include <minhook.h>

namespace features {

    void Removals::Initialize() {
        // 1. Hook Smoke DrawArray in client.dll
        // Sig: 48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 56 41 57 48 83 EC 40 48 8B 9C 24 88 00 00 00 4D 8B F8
        auto smokeAddr = memory::PatternScanner::Find(
            "client.dll",
            "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 56 41 57 48 83 EC 40 48 8B 9C 24 88 00 00 00 4D 8B F8"
        );

        if (smokeAddr.IsValid()) {
            m_pSmokeDrawArrayTarget = smokeAddr.As<void*>();
            if (MH_CreateHook(m_pSmokeDrawArrayTarget, reinterpret_cast<void*>(&HookedSmokeDrawArray), reinterpret_cast<void**>(&OriginalSmokeDrawArray)) == MH_OK) {
                MH_EnableHook(m_pSmokeDrawArrayTarget);
                utils::Logger::Info("Removals: Hooked Smoke DrawArray successfully.");
            } else {
                utils::Logger::Error("Removals: Failed to hook Smoke DrawArray with MinHook.");
            }
        } else {
            utils::Logger::Error("Removals: Failed to find Smoke DrawArray pattern.");
        }

        // 2. Hook Draw Flashbang in client.dll
        // Sig: 85 D2 0F 88 ? ? ? ? 48 89 4C 24 08 55 56 41 55 41 56 41 57 48 8D AC 24 10 FF FF FF
        auto flashAddr = memory::PatternScanner::Find(
            "client.dll",
            "85 D2 0F 88 ? ? ? ? 48 89 4C 24 08 55 56 41 55 41 56 41 57 48 8D AC 24 10 FF FF FF"
        );

        if (flashAddr.IsValid()) {
            m_pDrawFlashbangTarget = flashAddr.As<void*>();
            if (MH_CreateHook(m_pDrawFlashbangTarget, reinterpret_cast<void*>(&HookedDrawFlashbang), reinterpret_cast<void**>(&OriginalDrawFlashbang)) == MH_OK) {
                MH_EnableHook(m_pDrawFlashbangTarget);
                utils::Logger::Info("Removals: Hooked Draw Flashbang successfully.");
            } else {
                utils::Logger::Error("Removals: Failed to hook Draw Flashbang with MinHook.");
            }
        } else {
            utils::Logger::Error("Removals: Failed to find Draw Flashbang pattern.");
        }
    }

    void Removals::Shutdown() {
        if (m_pSmokeDrawArrayTarget) {
            MH_DisableHook(m_pSmokeDrawArrayTarget);
            m_pSmokeDrawArrayTarget = nullptr;
            OriginalSmokeDrawArray = nullptr;
        }

        if (m_pDrawFlashbangTarget) {
            MH_DisableHook(m_pDrawFlashbangTarget);
            m_pDrawFlashbangTarget = nullptr;
            OriginalDrawFlashbang = nullptr;
        }
    }

    void Removals::OnRender() {
        // Continuous memory suppression for flash duration and alpha on local player pawn
        if (!Config.m_bNoFlash) return;

        auto localPawn = source2::EntitySystem::GetLocalPlayerPawn();
        if (!localPawn) return;

        __try {
            if (localPawn->GetFlashDuration() > 0.0f) {
                localPawn->SetFlashDuration(0.0f);
            }
            if (localPawn->GetFlashMaxAlpha() > Config.m_flFlashAlpha) {
                localPawn->SetFlashMaxAlpha(Config.m_flFlashAlpha);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }

    void __fastcall Removals::HookedSmokeDrawArray(
        void* thisptr,
        int64_t a2,
        int64_t a3,
        uint32_t a4,
        int64_t a5,
        uint32_t* a6
    ) {
        if (!OriginalSmokeDrawArray) return;

        // When No Smoke is active, completely skip particle array submission
        if (Config.m_bNoSmoke) {
            return;
        }

        OriginalSmokeDrawArray(thisptr, a2, a3, a4, a5, a6);
    }

    void __fastcall Removals::HookedDrawFlashbang(
        void* render_context,
        int slot,
        void* view,
        void* layer,
        const void* viewport,
        bool auxiliary_pass
    ) {
        if (!OriginalDrawFlashbang) return;

        // When No Flash is enabled, suppress the screen whiteout render pass
        if (Config.m_bNoFlash) {
            return;
        }

        OriginalDrawFlashbang(render_context, slot, view, layer, viewport, auxiliary_pass);
    }

} // namespace features
