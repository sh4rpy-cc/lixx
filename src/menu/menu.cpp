#include "menu.hpp"
#include "ui_theme.hpp"
#include "src/render/render.hpp"
#include "src/core/hooks.hpp"
#include "src/features/visuals/visuals.hpp"
#include "src/features/chams/chams.hpp"
#include "src/features/removals/removals.hpp"

namespace menu {

    void Menu::ApplyTheme() {
        if (m_bThemeInitialized) return;

        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // LIXXUID Dark Minimalist Theme
        colors[ImGuiCol_Text]                  = lixxuid::ui::color::hex_to_imvec4(0xF3F4F6);
        colors[ImGuiCol_TextDisabled]          = lixxuid::ui::color::hex_to_imvec4(0x6B7280);
        colors[ImGuiCol_WindowBg]              = lixxuid::ui::color::hex_to_imvec4(0x0C0D0F, 0.98f);
        colors[ImGuiCol_ChildBg]               = lixxuid::ui::color::hex_to_imvec4(0x0C0D0F, 0.00f);
        colors[ImGuiCol_PopupBg]               = lixxuid::ui::color::hex_to_imvec4(0x111317, 0.98f);
        colors[ImGuiCol_Border]                = lixxuid::ui::color::hex_to_imvec4(0x20242C, 1.00f);
        colors[ImGuiCol_BorderShadow]          = ImVec4(0, 0, 0, 0);
        colors[ImGuiCol_FrameBg]               = lixxuid::ui::color::hex_to_imvec4(0x131519, 1.00f);
        colors[ImGuiCol_FrameBgHovered]        = lixxuid::ui::color::hex_to_imvec4(0x1A1D24, 1.00f);
        colors[ImGuiCol_FrameBgActive]         = lixxuid::ui::color::hex_to_imvec4(0x222730, 1.00f);
        colors[ImGuiCol_CheckMark]             = lixxuid::ui::color::hex_to_imvec4(0x00DBCD, 1.00f);
        colors[ImGuiCol_SliderGrab]            = lixxuid::ui::color::hex_to_imvec4(0x00DBCD, 1.00f);
        colors[ImGuiCol_SliderGrabActive]      = lixxuid::ui::color::hex_to_imvec4(0x00DBCD, 1.00f);
        colors[ImGuiCol_Button]                = lixxuid::ui::color::hex_to_imvec4(0x16181E, 1.00f);
        colors[ImGuiCol_ButtonHovered]         = lixxuid::ui::color::hex_to_imvec4(0x20242B, 1.00f);
        colors[ImGuiCol_ButtonActive]          = lixxuid::ui::color::hex_to_imvec4(0x00DBCD, 0.80f);
        colors[ImGuiCol_Header]                = lixxuid::ui::color::hex_to_imvec4(0x00DBCD, 0.20f);
        colors[ImGuiCol_HeaderHovered]         = lixxuid::ui::color::hex_to_imvec4(0x00DBCD, 0.35f);
        colors[ImGuiCol_HeaderActive]          = lixxuid::ui::color::hex_to_imvec4(0x00DBCD, 0.50f);
        colors[ImGuiCol_Separator]             = lixxuid::ui::color::hex_to_imvec4(0x22262E, 1.00f);

        style.WindowRounding    = 2.0f;
        style.ChildRounding     = 2.0f;
        style.FrameRounding     = 2.0f;
        style.PopupRounding     = 2.0f;
        style.ScrollbarRounding = 2.0f;
        style.WindowPadding     = ImVec2(12.0f, 12.0f);
        style.FramePadding      = ImVec2(6.0f, 4.0f);
        style.ItemSpacing       = ImVec2(8.0f, 7.0f);
        style.WindowBorderSize  = 1.0f;
        style.ChildBorderSize   = 0.0f;
        style.FrameBorderSize   = 1.0f;

        m_bThemeInitialized = true;
    }

    void Menu::Render() {
        if (!render::RenderManager::IsMenuOpen) return;

        ApplyTheme();

        const ImVec2 window_size = ImVec2(780.f, 560.f);
        ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoTitleBar |
                                 ImGuiWindowFlags_NoCollapse |
                                 ImGuiWindowFlags_NoScrollbar |
                                 ImGuiWindowFlags_NoScrollWithMouse;

        if (ImGui::Begin("##LIXXUID_MAIN_MENU", nullptr, flags)) {
            // 1. Top Logo Text
            ImGui::SetCursorPos(ImVec2(16.f, 12.f));
            lixxuid::ui::Widgets::LogoText("lixxuid", ".cs2");

            float text_height = ImGui::GetTextLineHeight() + 4.f;
            ImVec2 sep_pos = ImGui::GetCursorScreenPos();
            sep_pos.x += ImGui::CalcTextSize("lixxuid.cs2").x + 14.f;
            sep_pos.y -= (text_height * 0.15f);

            lixxuid::ui::Widgets::TextSeparator(sep_pos, text_height);

            ImGui::SetCursorPos(ImVec2(16.f + ImGui::CalcTextSize("lixxuid.cs2").x + 22.f, 12.f));
            lixxuid::ui::Widgets::LogoText("internal ", "[v2.0]");

            // 2. Top-Right Tab Bar with Vector Icons
            static std::vector<lixxuid::ui::TabItem> tabs = {
                { "visuals",  ICON_MD_VISIBILITY },
                { "chams",    ICON_MD_PALETTE },
                { "removals", ICON_MD_LAYERS_CLEAR },
                { "settings", ICON_MD_SETTINGS }
            };

            ImGui::SetCursorPos(ImVec2(12.f, 10.f));
            lixxuid::ui::Widgets::TabBar("##main_tabs", tabs, m_iSelectedTab);

            // Divider line under header
            ImGui::SetCursorPos(ImVec2(0.f, 44.f));
            ImGui::Separator();

            // 3. Tab Contents Layout
            const float content_y = 52.f;
            const float avail_w = window_size.x - 28.f;
            const float col_w = (avail_w - 14.f) * 0.5f;
            const float content_h = window_size.y - content_y - 14.f;

            // ==================== TAB 0: VISUALS (ESP) ====================
            if (m_iSelectedTab == 0) {
                auto& cfg = features::Visuals::Config;

                // Left Column
                ImGui::SetCursorPos(ImVec2(14.f, content_y));
                ImGui::BeginChild("##tab0_col_left", ImVec2(col_w, content_h), false, ImGuiWindowFlags_NoScrollbar);
                {
                    lixxuid::ui::Widgets::GroupBox("general & filters", 100.0f, 150.0f, [&]() {
                        lixxuid::ui::Widgets::Checkbox("master enable (esp)", &cfg.m_bEnabled);
                        lixxuid::ui::Widgets::Checkbox("team check", &cfg.m_bTeamCheck);
                        lixxuid::ui::Widgets::Checkbox("local player esp", &cfg.m_bLocalPlayer);
                    });

                    ImGui::Spacing();

                    lixxuid::ui::Widgets::GroupBox("status bars", 100.0f, 150.0f, [&]() {
                        lixxuid::ui::Widgets::Checkbox("health bar", &cfg.m_bHealthBar);
                        if (cfg.m_bHealthBar) {
                            lixxuid::ui::Widgets::Checkbox("hp number text", &cfg.m_bHealthText);
                        }
                        lixxuid::ui::Widgets::Checkbox("armor bar", &cfg.m_bArmorBar);
                    });
                }
                ImGui::EndChild();

                // Right Column
                ImGui::SetCursorPos(ImVec2(14.f + col_w + 14.f, content_y));
                ImGui::BeginChild("##tab0_col_right", ImVec2(col_w, content_h), false, ImGuiWindowFlags_NoScrollbar);
                {
                    const float overlays_h = cfg.m_bBox && cfg.m_bSkeleton ? 250.0f :
                                            (cfg.m_bBox || cfg.m_bSkeleton) ? 220.0f : 160.0f;
                    lixxuid::ui::Widgets::GroupBox("overlays & bones", 100.0f, overlays_h, [&]() {
                        lixxuid::ui::Widgets::Checkbox("bounding box", &cfg.m_bBox);
                        if (cfg.m_bBox) {
                            std::vector<std::pair<int, std::string>> box_options = {
                                { 0, "full outline" },
                                { 1, "corner brackets" }
                            };
                            lixxuid::ui::Widgets::SelectBox("box style", cfg.m_iBoxType, box_options);
                            lixxuid::ui::Widgets::ColorPicker(cfg.m_flBoxColor, "##box_col");
                            ImGui::SameLine();
                            ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x6B7280), "box color");
                        }

                        lixxuid::ui::Widgets::Checkbox("skeleton (bones)", &cfg.m_bSkeleton);
                        if (cfg.m_bSkeleton) {
                            lixxuid::ui::Widgets::Checkbox("draw joints", &cfg.m_bSkeletonJoints);
                            lixxuid::ui::Widgets::Slider("bone thickness", &cfg.m_flSkeletonThickness, 1.0f, 3.5f, "%.1f px");
                            lixxuid::ui::Widgets::ColorPicker(cfg.m_flSkeletonColor, "##skel_col");
                            ImGui::SameLine();
                            ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x6B7280), "skeleton color");
                        }
                    });

                    ImGui::Spacing();

                    const float labels_h = content_h - overlays_h - 16.0f;
                    lixxuid::ui::Widgets::GroupBox("labels & tracelines", 100.0f, ImMax(200.0f, labels_h), [&]() {
                        lixxuid::ui::Widgets::Checkbox("player nickname", &cfg.m_bName);
                        if (cfg.m_bName) {
                            lixxuid::ui::Widgets::ColorPicker(cfg.m_flNameColor, "##name_col");
                            ImGui::SameLine();
                            ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x6B7280), "name color");
                        }

                        lixxuid::ui::Widgets::Checkbox("active weapon name", &cfg.m_bWeapon);
                        if (cfg.m_bWeapon) {
                            lixxuid::ui::Widgets::ColorPicker(cfg.m_flWeaponColor, "##weap_col");
                            ImGui::SameLine();
                            ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x6B7280), "weapon color");
                        }

                        lixxuid::ui::Widgets::Checkbox("distance indicator", &cfg.m_bDistance);
                        lixxuid::ui::Widgets::Checkbox("state flags (scoped/defusing)", &cfg.m_bFlags);

                        lixxuid::ui::Widgets::Checkbox("head dot marker", &cfg.m_bHeadDot);
                        lixxuid::ui::Widgets::Checkbox("snaplines from screen", &cfg.m_bSnaplines);
                    });
                }
                ImGui::EndChild();
            }

            // ==================== TAB 1: CHAMS (MODELS) ====================
            else if (m_iSelectedTab == 1) {
                auto& chamsCfg = features::Chams::Config;

                // Left Column: Chams Configuration
                ImGui::SetCursorPos(ImVec2(14.f, content_y));
                ImGui::BeginChild("##tab1_col_left", ImVec2(col_w, content_h), false, ImGuiWindowFlags_NoScrollbar);
                {
                    lixxuid::ui::Widgets::GroupBox("model chams", 100.0f, content_h - 10.0f, [&]() {
                        lixxuid::ui::Widgets::Checkbox("enable player chams", &chamsCfg.m_bEnabled);
                        lixxuid::ui::Widgets::Checkbox("ignore z (x-ray wallhack)", &chamsCfg.m_bIgnoreZ);

                        std::vector<std::pair<int, std::string>> mat_options = {
                            { 0, "flat (solid unlit)" },
                            { 1, "special (water ripple)" },
                            { 2, "self illuminated (glow)" },
                            { 3, "hologram" },
                            { 4, "glitch" },
                            { 5, "diamond" }
                        };
                        int currentMat = static_cast<int>(chamsCfg.m_Material);
                        if (lixxuid::ui::Widgets::SelectBox("material style", currentMat, mat_options)) {
                            chamsCfg.m_Material = static_cast<features::ChamsMaterial>(currentMat);
                        }

                        lixxuid::ui::Widgets::Checkbox("team check", &chamsCfg.m_bTeamCheck);
                        lixxuid::ui::Widgets::Checkbox("local player chams", &chamsCfg.m_bLocalPlayer);
                    });
                }
                ImGui::EndChild();

                // Right Column: Color Modulation
                ImGui::SetCursorPos(ImVec2(14.f + col_w + 14.f, content_y));
                ImGui::BeginChild("##tab1_col_right", ImVec2(col_w, content_h), false, ImGuiWindowFlags_NoScrollbar);
                {
                    lixxuid::ui::Widgets::GroupBox("color modulation", 100.0f, content_h - 10.0f, [&]() {
                        ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x9CA3AF), "visible mesh color");
                        lixxuid::ui::Widgets::ColorPicker(chamsCfg.m_flVisibleColor, "##chams_vis");

                        ImGui::Spacing();
                        ImGui::Spacing();
                        ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x9CA3AF), "occluded (wallhack) color");
                        lixxuid::ui::Widgets::ColorPicker(chamsCfg.m_flOccludedColor, "##chams_occ");
                    });
                }
                ImGui::EndChild();
            }

            // ==================== TAB 2: REMOVALS ====================
            else if (m_iSelectedTab == 2) {
                auto& remCfg = features::Removals::Config;

                // Left Column: Flashbang Removals
                ImGui::SetCursorPos(ImVec2(14.f, content_y));
                ImGui::BeginChild("##tab2_col_left", ImVec2(col_w, content_h), false, ImGuiWindowFlags_NoScrollbar);
                {
                    lixxuid::ui::Widgets::GroupBox("flashbang suppression", 100.0f, content_h - 10.0f, [&]() {
                        lixxuid::ui::Widgets::Checkbox("no flash (anti-flash)", &remCfg.m_bNoFlash);
                        if (remCfg.m_bNoFlash) {
                            lixxuid::ui::Widgets::Slider("max flash opacity", &remCfg.m_flFlashAlpha, 0.0f, 255.0f, "%.0f");
                            ImGui::Spacing();
                            ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x6B7280), "0 = full blind immunity");
                        }
                    });
                }
                ImGui::EndChild();

                // Right Column: Smoke Removals
                ImGui::SetCursorPos(ImVec2(14.f + col_w + 14.f, content_y));
                ImGui::BeginChild("##tab2_col_right", ImVec2(col_w, content_h), false, ImGuiWindowFlags_NoScrollbar);
                {
                    lixxuid::ui::Widgets::GroupBox("smoke particle suppression", 100.0f, content_h - 10.0f, [&]() {
                        lixxuid::ui::Widgets::Checkbox("no smoke (remove volumetric smoke)", &remCfg.m_bNoSmoke);
                        ImGui::Spacing();
                        ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x6B7280), "Removes volumetric particle cloud rendering across client pipeline.");
                    });
                }
                ImGui::EndChild();
            }

            // ==================== TAB 3: SETTINGS ====================
            else if (m_iSelectedTab == 3) {
                // Left Column: System Architecture
                ImGui::SetCursorPos(ImVec2(14.f, content_y));
                ImGui::BeginChild("##tab3_col_left", ImVec2(col_w, content_h), false, ImGuiWindowFlags_NoScrollbar);
                {
                    lixxuid::ui::Widgets::GroupBox("system info", 100.0f, content_h - 10.0f, [&]() {
                        ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x00DBCD), "LIXXUID CS2 Internal Base");
                        ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x6B7280), "Build: Release x64 C++20");
                        ImGui::Spacing();
                        ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x9CA3AF), "Hook: DX11 Present + ResizeBuffers");
                        ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x9CA3AF), "Scene Hook: scenesystem.dll DrawArray");
                        ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x9CA3AF), "Particles Hook: client.dll Smoke DrawArray");
                        ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x9CA3AF), "Render Hook: client.dll DrawFlashbang");
                        ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x9CA3AF), "Schema System: ISchemaSystem VMT 2");
                        ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x9CA3AF), "UI Backend: Native ImGui Engine");
                        ImGui::Spacing();
                        ImGui::Separator();
                        ImGui::Spacing();
                        ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0xF3F4F6), "Hotkeys:");
                        ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x6B7280), "• [INSERT] / [DELETE] Toggle Menu");
                        ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x6B7280), "• [END] Clean Unhook & Eject DLL");
                    });
                }
                ImGui::EndChild();

                // Right Column: Lifecycle & Management
                ImGui::SetCursorPos(ImVec2(14.f + col_w + 14.f, content_y));
                ImGui::BeginChild("##tab3_col_right", ImVec2(col_w, content_h), false, ImGuiWindowFlags_NoScrollbar);
                {
                    lixxuid::ui::Widgets::GroupBox("lifecycle management", 100.0f, content_h - 10.0f, [&]() {
                        ImGui::TextColored(lixxuid::ui::color::hex_to_imvec4(0x9CA3AF), "Hot-Unload:");
                        ImGui::TextWrapped("Restores original game WndProc, disables MinHook hooks on DirectX 11, cleans up fonts and exits cleanly without game restart.");
                        ImGui::Spacing();
                        ImGui::Spacing();

                        if (lixxuid::ui::Widgets::Button("Eject DLL (END)", ImVec2(-1, 38))) {
                            core::Hooks::Shutdown();
                        }
                    });
                }
                ImGui::EndChild();
            }

            // Ensure window boundaries are cleanly resolved
            ImGui::SetCursorPos(ImVec2(0.f, window_size.y - 1.f));
            ImGui::Dummy(ImVec2(window_size.x, 1.0f));

            ImGui::End();
        }
    }

} // namespace menu
