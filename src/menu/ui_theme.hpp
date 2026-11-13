#pragma once
#include <windows.h>
#include <imgui.h>
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui_internal.h>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <cmath>
#include <concepts>

#include "material_icons.h"
#include "material_icons_regular.h"
#include "outfit_regular.h"
#include "menu_anim.h"
#include "menu_utils.h"

namespace math {
    struct Color {
        float r{1.f}, g{1.f}, b{1.f}, a{1.f};

        constexpr Color() = default;
        constexpr Color(float _r, float _g, float _b, float _a = 1.0f) : r(_r), g(_g), b(_b), a(_a) {}
        Color(const float arr[4]) : r(arr[0]), g(arr[1]), b(arr[2]), a(arr[3]) {}

        ImU32 ToU32() const {
            return IM_COL32((int)(r * 255.f), (int)(g * 255.f), (int)(b * 255.f), (int)(a * 255.f));
        }

        void ToArray(float arr[4]) const {
            arr[0] = r; arr[1] = g; arr[2] = b; arr[3] = a;
        }

        Color& FromArray(const float arr[4]) {
            r = arr[0]; g = arr[1]; b = arr[2]; a = arr[3];
            return *this;
        }
    };
}

namespace lixxuid::ui {

    struct TabItem {
        std::string label;
        std::string icon;
    };

    template <typename T>
    concept Numeric = std::is_arithmetic_v<T>;

    template <typename T>
    using SelectOptions = std::vector<std::pair<T, std::string>>;

    class ResourceManager {
    public:
        static inline ImFont* FontMain{nullptr};
        static inline ImFont* FontIcons{nullptr};

        static void SetupFonts() {
            ImGuiIO& io = ImGui::GetIO();

            ImFontConfig fontConfig;
            fontConfig.PixelSnapH = false;
            fontConfig.OversampleH = 2;
            fontConfig.OversampleV = 2;

            // 1. Primary UI Typography
            FontMain = io.Fonts->AddFontFromMemoryTTF(
                const_cast<void*>(static_cast<const void*>(outfit_regular)),
                sizeof(outfit_regular),
                14.5f,
                &fontConfig
            );

            // 2. Merge Cyrillic glyphs from system font for Russian nicknames
            ImFontConfig mergeCyrillic;
            mergeCyrillic.MergeMode = true;
            mergeCyrillic.OversampleH = 2;
            mergeCyrillic.OversampleV = 2;

            const char* sysFont = "C:\\Windows\\Fonts\\segoeui.ttf";
            if (GetFileAttributesA(sysFont) == INVALID_FILE_ATTRIBUTES) {
                sysFont = "C:\\Windows\\Fonts\\arial.ttf";
            }
            if (GetFileAttributesA(sysFont) != INVALID_FILE_ATTRIBUTES) {
                io.Fonts->AddFontFromFileTTF(sysFont, 14.5f, &mergeCyrillic, io.Fonts->GetGlyphRangesCyrillic());
            }

            // 3. Merge Vector Icons
            static const ImWchar icons_ranges[] = { static_cast<ImWchar>(ICON_MIN_MD), static_cast<ImWchar>(ICON_MAX_16_MD), 0 };
            ImFontConfig mergeIcons;
            mergeIcons.MergeMode = true;
            mergeIcons.OversampleH = 1;
            mergeIcons.OversampleV = 1;
            mergeIcons.GlyphOffset.y = 1.0f;

            FontIcons = io.Fonts->AddFontFromMemoryTTF(
                const_cast<void*>(static_cast<const void*>(material_icons_regular)),
                sizeof(material_icons_regular),
                16.0f,
                &mergeIcons,
                icons_ranges
            );
        }
    };

    class Widgets {
    public:
        // --- Logo Text ---
        static void LogoText(std::string_view mainText, std::string_view coloredText) noexcept {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            if (window->SkipItems) return;

            const ImVec2 mainSize = ImGui::CalcTextSize(mainText.data());
            const ImVec2 coloredSize = ImGui::CalcTextSize(coloredText.data());
            const ImVec2 totalSize(mainSize.x + coloredSize.x, ImMax(mainSize.y, coloredSize.y));

            const ImVec2 pos = window->DC.CursorPos;
            const ImRect bb(pos, pos + totalSize);

            ImGui::ItemSize(totalSize);
            if (!ImGui::ItemAdd(bb, window->GetID("logo_text"))) return;

            window->DrawList->AddText(pos, ImGui::GetColorU32(ImVec4(1.f, 1.f, 1.f, 1.f)), mainText.data());
            window->DrawList->AddText(
                ImVec2(pos.x + mainSize.x, pos.y),
                ImGui::GetColorU32(color::hex_to_imvec4(0x00DBCD)),
                coloredText.data()
            );
        }

        // --- Text Separator ---
        static void TextSeparator(const ImVec2& pos, float height) noexcept {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            if (window->SkipItems) return;

            window->DrawList->AddLine(
                pos,
                ImVec2(pos.x, pos.y + height),
                ImGui::GetColorU32(color::hex_to_imvec4(0x374151, 1.0f)),
                1.0f
            );
        }

        // --- Tab Bar ---
        static bool TabBar(const char* str_id, const std::vector<TabItem>& tabs, int& selectedTab, const ImVec2& size = ImVec2(0, 25)) noexcept {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            if (window->SkipItems) return false;

            bool tab_changed = false;
            const ImVec2 pos = window->DC.CursorPos;
            float total_width = 0.0f;
            std::vector<float> tab_widths(tabs.size());

            for (size_t i = 0; i < tabs.size(); ++i) {
                float text_w = ImGui::CalcTextSize(tabs[i].label.c_str()).x;
                float icon_w = tabs[i].icon.empty() ? 0.f : ImGui::CalcTextSize(tabs[i].icon.c_str()).x + 6.0f;
                tab_widths[i] = text_w + icon_w + 20.0f;
                total_width += tab_widths[i];
            }

            // Align to the right
            const float avail_w = ImGui::GetContentRegionAvail().x;
            const float h = 28.0f;
            float current_x = pos.x + ImMax(0.0f, avail_w - total_width - 8.0f);

            for (size_t i = 0; i < tabs.size(); ++i) {
                const bool is_selected = (selectedTab == static_cast<int>(i));
                const float w = tab_widths[i];

                ImGui::SetCursorScreenPos(ImVec2(current_x, pos.y));
                ImRect bb(ImVec2(current_x, pos.y), ImVec2(current_x + w, pos.y + h));
                const ImGuiID id = window->GetID((std::string(str_id) + "_tab_" + std::to_string(i)).c_str());

                ImGui::Dummy(ImVec2(w, h));
                if (!ImGui::ItemAdd(bb, id)) continue;

                bool hovered, held;
                bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
                if (pressed) {
                    selectedTab = static_cast<int>(i);
                    tab_changed = true;
                }

                float anim = anim_manager::get_anim("tab_" + std::string(str_id) + "_" + std::to_string(i), is_selected, 1.5f);

                if (anim > 0.01f) {
                    window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::GetColorU32(ImVec4(0.0f, 0.86f, 0.80f, anim * 0.15f)), 3.0f);
                    window->DrawList->AddLine(
                        ImVec2(bb.Min.x + 4.f, bb.Max.y - 1.f),
                        ImVec2(bb.Max.x - 4.f, bb.Max.y - 1.f),
                        ImGui::GetColorU32(color::hex_to_imvec4(0x00DBCD, anim)),
                        2.0f
                    );
                }

                ImVec4 text_col = ImLerp(
                    color::hex_to_imvec4(0x6B7280, 1.0f),
                    color::hex_to_imvec4(0x00DBCD, 1.0f),
                    anim
                );
                if (hovered && !is_selected) {
                    text_col = color::hex_to_imvec4(0xD1D5DB, 1.0f);
                }

                std::string full_label;
                if (!tabs[i].icon.empty()) {
                    full_label = tabs[i].icon + " " + tabs[i].label;
                } else {
                    full_label = tabs[i].label;
                }

                ImVec2 label_size = ImGui::CalcTextSize(full_label.c_str());
                ImVec2 text_pos(
                    bb.Min.x + (w - label_size.x) * 0.5f,
                    bb.Min.y + (h - label_size.y) * 0.5f
                );

                window->DrawList->AddText(text_pos, ImGui::GetColorU32(text_col), full_label.c_str());
                current_x += w + 4.0f;
            }

            ImGui::SetCursorPosY(pos.y - window->Pos.y + 32.0f);
            return tab_changed;
        }

        // --- Group Box ---
        static void GroupBox(const char* label, float widthPercent, float boxHeight, const std::function<void()>& contentFn, float padding = 10.0f, float rounding = 2.0f) noexcept {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            if (window->SkipItems) return;

            const ImVec2 avail = ImGui::GetContentRegionAvail();
            const float box_w = (widthPercent >= 99.0f) ? avail.x : (widthPercent / 100.0f) * avail.x;
            const float box_h = boxHeight;

            const ImVec2 orig_cursor = ImGui::GetCursorPos();
            const ImVec2 abs_pos = ImVec2(window->Pos.x + orig_cursor.x, window->Pos.y + orig_cursor.y);

            const ImVec2 textSize = ImGui::CalcTextSize(label);
            const float text_h = textSize.y;
            const float text_pad = 6.0f;

            ImRect frame_bb(abs_pos, ImVec2(abs_pos.x + box_w, abs_pos.y + box_h));

            window->DrawList->AddRect(
                frame_bb.Min,
                frame_bb.Max,
                ImGui::GetColorU32(color::hex_to_imvec4(0x24272E, 1.0f)),
                rounding,
                0,
                1.0f
            );

            const float label_x = frame_bb.Min.x + padding * 1.5f;
            const ImVec2 label_pos(label_x, frame_bb.Min.y - text_h * 0.5f);
            const ImRect text_cutout(
                ImVec2(label_pos.x - text_pad, label_pos.y),
                ImVec2(label_pos.x + textSize.x + text_pad, label_pos.y + text_h)
            );

            window->DrawList->AddRectFilled(text_cutout.Min, text_cutout.Max, ImGui::GetColorU32(ImGuiCol_WindowBg));
            window->DrawList->AddText(label_pos, ImGui::GetColorU32(color::hex_to_imvec4(0xD1D5DB, 1.0f)), label);

            const float content_y_offset = text_h * 0.5f;
            ImGui::SetCursorPos(ImVec2(orig_cursor.x + padding, orig_cursor.y + content_y_offset + padding));

            const ImVec2 child_size(box_w - padding * 2.0f, box_h - content_y_offset - padding * 2.0f);

            char child_id[64];
            ImFormatString(child_id, sizeof(child_id), "##gb_%s", label);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding, padding));
            ImGui::BeginChild(child_id, child_size, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            contentFn();

            ImGui::EndChild();
            ImGui::PopStyleVar();

            // Set cursor to bottom of groupbox and declare item bounds cleanly
            ImGui::SetCursorPos(ImVec2(orig_cursor.x, orig_cursor.y + box_h));
            ImGui::Dummy(ImVec2(box_w, 4.0f));
        }

        // --- Checkbox ---
        static bool Checkbox(std::string_view label, bool* value, float rounding = 2.0f, float padding = 4.0f) noexcept {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            if (window->SkipItems) return false;

            const ImGuiStyle& style = ImGui::GetStyle();
            const ImGuiID id = window->GetID(label.data());
            const ImVec2 labelSize = ImGui::CalcTextSize(label.data(), nullptr, true);

            const float square_sz = 14.0f;
            const float avail_w = ImGui::GetContentRegionAvail().x;
            const ImVec2 pos = window->DC.CursorPos;

            const float checkbox_x = pos.x + avail_w - square_sz - padding;

            const ImRect total_bb(
                pos,
                ImVec2(pos.x + avail_w, pos.y + ImMax(square_sz, labelSize.y) + 4.0f)
            );

            ImGui::ItemSize(total_bb, style.FramePadding.y);
            if (!ImGui::ItemAdd(total_bb, id)) return false;

            const ImRect check_bb(
                ImVec2(checkbox_x, pos.y + (total_bb.GetHeight() - square_sz) * 0.5f),
                ImVec2(checkbox_x + square_sz, pos.y + (total_bb.GetHeight() + square_sz) * 0.5f)
            );

            bool pressed = false;
            ImGui::SetCursorScreenPos(check_bb.Min);
            std::string btn_id = "##cb_" + std::string(label);
            if (ImGui::InvisibleButton(btn_id.c_str(), ImVec2(square_sz, square_sz))) {
                *value = !*value;
                pressed = true;
                ImGui::MarkItemEdited(id);
            }

            bool hovered = ImGui::IsItemHovered();
            float checked_anim = anim_manager::get_anim("cb_chk_" + std::string(label), *value, 2.0f);

            window->DrawList->AddRect(
                check_bb.Min, check_bb.Max,
                ImGui::GetColorU32(color::hex_to_imvec4(0x374151, 1.0f)),
                rounding, 0, 1.0f
            );

            const ImRect inner_bb(check_bb.Min + ImVec2(2, 2), check_bb.Max - ImVec2(2, 2));
            window->DrawList->AddRectFilled(
                inner_bb.Min, inner_bb.Max,
                ImGui::GetColorU32(color::hex_to_imvec4(0x111317, 1.0f)),
                rounding
            );

            if (checked_anim > 0.01f) {
                window->DrawList->AddRectFilled(
                    inner_bb.Min, inner_bb.Max,
                    ImGui::GetColorU32(color::hex_to_imvec4(0x00DBCD, checked_anim)),
                    rounding
                );
            }

            ImVec4 text_col = *value ? color::hex_to_imvec4(0xF3F4F6) : color::hex_to_imvec4(0x9CA3AF);
            if (hovered && !*value) text_col = color::hex_to_imvec4(0xE5E7EB);

            window->DrawList->AddText(
                ImVec2(pos.x + padding, pos.y + (total_bb.GetHeight() - labelSize.y) * 0.5f),
                ImGui::GetColorU32(text_col),
                label.data()
            );

            return pressed;
        }

        // --- Slider ---
        template <Numeric T>
        static bool Slider(std::string_view label, T* value, T min, T max, std::string_view format = "", float height = 8.0f) noexcept {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            if (window->SkipItems) return false;

            ImGuiContext& g = *GImGui;
            const ImGuiStyle& style = g.Style;
            const ImGuiID id = window->GetID(label.data());
            const float w = ImGui::GetContentRegionAvail().x;

            const ImVec2 labelSize = ImGui::CalcTextSize(label.data(), nullptr, true);

            char value_buf[64];
            if (format.empty()) {
                if constexpr (std::is_floating_point_v<T>) {
                    ImFormatString(value_buf, sizeof(value_buf), "%.1f", static_cast<double>(*value));
                } else {
                    ImFormatString(value_buf, sizeof(value_buf), "%d", static_cast<int>(*value));
                }
            } else {
                if constexpr (std::is_floating_point_v<T>) {
                    ImFormatString(value_buf, sizeof(value_buf), format.data(), static_cast<double>(*value));
                } else {
                    ImFormatString(value_buf, sizeof(value_buf), format.data(), static_cast<int>(*value));
                }
            }
            const ImVec2 valueSize = ImGui::CalcTextSize(value_buf);

            const float text_h = ImMax(labelSize.y, valueSize.y);
            const ImRect frame_bb(
                window->DC.CursorPos + ImVec2(0, text_h + 3.0f),
                window->DC.CursorPos + ImVec2(w, text_h + 3.0f + height)
            );

            const ImRect total_bb(window->DC.CursorPos, frame_bb.Max + ImVec2(0, 4.0f));

            ImGui::ItemSize(total_bb, style.FramePadding.y);
            if (!ImGui::ItemAdd(total_bb, id)) return false;

            const bool hovered = ImGui::ItemHoverable(frame_bb, id, 0);
            const bool clicked = hovered && ImGui::IsMouseClicked(0);

            if (clicked || g.NavActivateId == id) {
                ImGui::SetActiveID(id, window);
                ImGui::SetFocusID(id, window);
                ImGui::FocusWindow(window);
            }

            bool value_changed = false;
            if (g.ActiveId == id) {
                if (g.IO.MouseDown[0]) {
                    const float normalized_pos = ImClamp((g.IO.MousePos.x - frame_bb.Min.x) / frame_bb.GetWidth(), 0.0f, 1.0f);
                    *value = static_cast<T>(min + normalized_pos * (max - min));
                    value_changed = true;
                    ImGui::MarkItemEdited(id);
                } else {
                    ImGui::ClearActiveID();
                }
            }

            window->DrawList->AddText(total_bb.Min, ImGui::GetColorU32(color::hex_to_imvec4(0x9CA3AF)), label.data());
            window->DrawList->AddText(ImVec2(total_bb.Max.x - valueSize.x, total_bb.Min.y), ImGui::GetColorU32(color::hex_to_imvec4(0xD1D5DB)), value_buf);

            window->DrawList->AddRectFilled(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(color::hex_to_imvec4(0x1F242C)), 2.0f);

            const float progress = static_cast<float>(*value - min) / static_cast<float>(max - min);
            const float fill_w = ImClamp(progress * frame_bb.GetWidth(), 0.0f, frame_bb.GetWidth());

            if (fill_w > 0.0f) {
                window->DrawList->AddRectFilled(
                    frame_bb.Min,
                    ImVec2(frame_bb.Min.x + fill_w, frame_bb.Max.y),
                    ImGui::GetColorU32(color::hex_to_imvec4(0x00DBCD)),
                    2.0f
                );
            }

            const float grab_x = frame_bb.Min.x + fill_w;
            const float grab_y = (frame_bb.Min.y + frame_bb.Max.y) * 0.5f;
            window->DrawList->AddCircleFilled(ImVec2(grab_x, grab_y), 4.5f, ImGui::GetColorU32(ImVec4(1.f, 1.f, 1.f, 1.f)));

            return value_changed;
        }

        // --- Select Box (Combo) ---
        template <typename T>
        static bool SelectBox(std::string_view label, T& value, const SelectOptions<T>& options) noexcept {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            if (window->SkipItems) return false;

            const ImGuiStyle& style = ImGui::GetStyle();
            const ImGuiID id = window->GetID(label.data());
            const float w = ImGui::GetContentRegionAvail().x;
            const float h = 24.0f;

            const ImVec2 labelSize = ImGui::CalcTextSize(label.data());
            const ImRect frame_bb(
                window->DC.CursorPos + ImVec2(0, labelSize.y + 3.0f),
                window->DC.CursorPos + ImVec2(w, labelSize.y + 3.0f + h)
            );
            const ImRect total_bb(window->DC.CursorPos, frame_bb.Max + ImVec2(0, 4.0f));

            ImGui::ItemSize(total_bb, style.FramePadding.y);
            if (!ImGui::ItemAdd(total_bb, id)) return false;

            std::string preview_text = "Unknown";
            for (const auto& opt : options) {
                if (opt.first == value) {
                    preview_text = opt.second;
                    break;
                }
            }

            window->DrawList->AddText(total_bb.Min, ImGui::GetColorU32(color::hex_to_imvec4(0x9CA3AF)), label.data());

            ImGui::SetCursorScreenPos(frame_bb.Min);
            std::string combo_id = "##combo_" + std::string(label);
            bool value_changed = false;

            ImGui::PushStyleColor(ImGuiCol_FrameBg, color::hex_to_imvec4(0x131519));
            ImGui::PushStyleColor(ImGuiCol_Border, color::hex_to_imvec4(0x2D3748));
            ImGui::PushStyleColor(ImGuiCol_Text, color::hex_to_imvec4(0xE5E7EB));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

            ImGui::SetNextItemWidth(w);
            if (ImGui::BeginCombo(combo_id.c_str(), preview_text.c_str(), ImGuiComboFlags_NoArrowButton)) {
                for (const auto& opt : options) {
                    const bool is_selected = (opt.first == value);
                    if (ImGui::Selectable(opt.second.c_str(), is_selected)) {
                        value = opt.first;
                        value_changed = true;
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(3);

            return value_changed;
        }

        // --- Button ---
        static bool Button(std::string_view label, const ImVec2& size = ImVec2(-1, 32)) noexcept {
            ImGui::PushStyleColor(ImGuiCol_Button, color::hex_to_imvec4(0x16181E));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color::hex_to_imvec4(0x20242B));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, color::hex_to_imvec4(0x00DBCD, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_Text, color::hex_to_imvec4(0xE5E7EB));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, color::hex_to_imvec4(0x2D3748));

            bool pressed = ImGui::Button(label.data(), size);

            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(5);
            return pressed;
        }

        // --- Color Picker Square ---
        static bool ColorPicker(float col[4], const char* str_id) noexcept {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
            bool changed = ImGui::ColorEdit4(str_id, col, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
            ImGui::PopStyleVar();
            return changed;
        }
    };

} // namespace lixxuid::ui
