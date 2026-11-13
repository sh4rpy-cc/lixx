#pragma once

namespace menu {

    class Menu {
    public:
        static void Render();
        static void ApplyTheme();

    private:
        static inline bool m_bThemeInitialized{false};
        static inline int m_iSelectedTab{0};
    };

} // namespace menu
