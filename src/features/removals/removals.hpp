#pragma once
#include <cstdint>

namespace features {

    struct RemovalsConfig {
        bool m_bNoFlash{true};
        float m_flFlashAlpha{0.0f}; // 0 = complete no-flash, up to 255
        bool m_bNoSmoke{true};
    };

    class Removals {
    public:
        static void Initialize();
        static void Shutdown();

        // Called on frame / present render loop
        static void OnRender();

        static inline RemovalsConfig Config{};

    private:
        // Smoke drawarray hook
        using SmokeDrawArray_t = void(__fastcall*)(void*, int64_t, int64_t, uint32_t, int64_t, uint32_t*);
        static inline SmokeDrawArray_t OriginalSmokeDrawArray{nullptr};
        static inline void* m_pSmokeDrawArrayTarget{nullptr};

        static void __fastcall HookedSmokeDrawArray(
            void* thisptr,
            int64_t a2,
            int64_t a3,
            uint32_t a4,
            int64_t a5,
            uint32_t* a6
        );

        // Flashbang render pass hook
        using DrawFlashbang_t = void(__fastcall*)(void*, int, void*, void*, const void*, bool);
        static inline DrawFlashbang_t OriginalDrawFlashbang{nullptr};
        static inline void* m_pDrawFlashbangTarget{nullptr};

        static void __fastcall HookedDrawFlashbang(
            void* render_context,
            int slot,
            void* view,
            void* layer,
            const void* viewport,
            bool auxiliary_pass
        );
    };

} // namespace features
