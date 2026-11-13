#pragma once
#include <cstdint>
#include <string_view>
#include <concepts>
#include <cstdlib>
#include <imgui.h>

namespace lixxuid::ui {

    namespace color {
        constexpr ImVec4 hex_to_imvec4(std::uint32_t hex, float alpha = 1.0f) noexcept {
            return ImVec4{
                static_cast<float>((hex >> 16) & 0xFF) / 255.0f,
                static_cast<float>((hex >> 8) & 0xFF) / 255.0f,
                static_cast<float>(hex & 0xFF) / 255.0f,
                alpha
            };
        }

        inline ImVec4 hex_to_imvec4(std::string_view hex_str, float alpha = 1.0f) noexcept {
            if (hex_str.empty()) return ImVec4{};

            const char* str = hex_str.data();
            str += (str[0] == '#') ? 1 : 0;

            std::uint32_t hex = static_cast<std::uint32_t>(std::strtoull(str, nullptr, 16));
            return hex_to_imvec4(hex, alpha);
        }

        template<std::floating_point T>
        constexpr ImVec4 rgba(T r, T g, T b, T a = static_cast<T>(1)) noexcept {
            return ImVec4{
                static_cast<float>(r),
                static_cast<float>(g),
                static_cast<float>(b),
                static_cast<float>(a)
            };
        }
    }

} // namespace lixxuid::ui
