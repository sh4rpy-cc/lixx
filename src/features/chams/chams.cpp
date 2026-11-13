#include "chams.hpp"
#include "src/utils/log.hpp"
#include "src/memory/pattern.hpp"
#include "src/source2/entity.hpp"
#include <minhook.h>
#include <string>

namespace source2 {

    // Tier0 exports
    using fnLoadKV3 = bool(__fastcall*)(CKeyValues3*, void*, const char*, const KV3ID_t*, const char*, unsigned int);
    using fnUtlBufferInit = void(__fastcall*)(void*, int, int, int);
    using fnUtlBufferPutString = void(__fastcall*)(void*, const char*);
    using fnUtlBufferEnsureCapacity = void(__fastcall*)(void*, int);

    static fnLoadKV3 s_fnLoadKV3 = nullptr;
    static fnUtlBufferInit s_fnUtlBufferInit = nullptr;
    static fnUtlBufferPutString s_fnUtlBufferPutString = nullptr;
    static fnUtlBufferEnsureCapacity s_fnUtlBufferEnsureCapacity = nullptr;

    static bool InitTier0() {
        static bool s_done = false;
        if (s_done) return true;

        HMODULE hTier0 = GetModuleHandleW(L"tier0.dll");
        if (!hTier0) return false;

        s_fnLoadKV3 = reinterpret_cast<fnLoadKV3>(GetProcAddress(hTier0, "?LoadKV3@@YA_NPEAVKeyValues3@@PEAVCUtlString@@PEBDAEBUKV3ID_t@@2I@Z"));
        s_fnUtlBufferInit = reinterpret_cast<fnUtlBufferInit>(GetProcAddress(hTier0, "??0CUtlBuffer@@QEAA@HHW4BufferFlags_t@0@@Z"));
        s_fnUtlBufferPutString = reinterpret_cast<fnUtlBufferPutString>(GetProcAddress(hTier0, "?PutString@CUtlBuffer@@QEAAXPEBD@Z"));
        s_fnUtlBufferEnsureCapacity = reinterpret_cast<fnUtlBufferEnsureCapacity>(GetProcAddress(hTier0, "?EnsureCapacity@CUtlBuffer@@QEAAXH@Z"));

        s_done = (s_fnLoadKV3 != nullptr && s_fnUtlBufferInit != nullptr && s_fnUtlBufferPutString != nullptr);
        return s_done;
    }

    CKeyValues3* CKeyValues3::CreateMaterialResource() {
        using fnSetTypeKV3 = CKeyValues3* (__fastcall*)(CKeyValues3*, unsigned int, unsigned int);
        static auto oSetTypeKV3 = reinterpret_cast<fnSetTypeKV3>(
            memory::PatternScanner::Find("client.dll", "40 53 48 83 EC 30 80 FA 06 0F B6 C2 41 B9 16 00 00 00 48 8B D9 44 0F 45 C8").As<void*>()
        );
        if (!oSetTypeKV3) return nullptr;

        CKeyValues3* pKV = new CKeyValues3[0x10]{};
        return oSetTypeKV3(pKV, 1U, 6U);
    }

    bool CKeyValues3::LoadFromBuffer(const char* szVmat) {
        if (!szVmat || !InitTier0()) return false;

        // CUtlBuffer dummy container (0x80 bytes)
        uint8_t buffer[0x80]{};
        s_fnUtlBufferInit(buffer, 0, static_cast<int>(strlen(szVmat) + 16), 1);
        s_fnUtlBufferPutString(buffer, szVmat);

        // LoadKV3 from string
        KV3ID_t kv3ID{"generic", 0x469806E97412167C, 0xE73790B53EE6F2AF};
        return s_fnLoadKV3(this, nullptr, szVmat, &kv3ID, "", 0U);
    }

} // namespace source2

namespace features {

    // --- KV3 VMAT Templates ---
    static constexpr char MATERIAL_SOLID_VISIBLE[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
    shader = "csgo_unlitgeneric.vfx"
    F_DISABLE_Z_BUFFERING = 0
    F_BLEND_MODE = 0
    F_RENDER_BACKFACES = 1
    F_IGNOREZ = 0

    g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
    g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
    g_tRoughness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"

    g_flBumpStrength = 1.0
    g_vGlossinessRange = [0.8, 1.0, 0.0, 0.0]
    g_vTexCoordScale = [1.0, 1.0, 0.0, 0.0]
    g_vTexCoordOffset = [0.0, 0.0, 0.0, 0.0]
    g_flFresnelExponent = 2.0
    g_flFresnelFalloff = 1.0
    g_flFresnelMax = 1.0
    g_flFresnelMin = 0.2
})";

    static constexpr char MATERIAL_SOLID_INVISIBLE[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
    shader = "csgo_unlitgeneric.vfx"
    F_DISABLE_Z_BUFFERING = 1
    F_BLEND_MODE = 1
    F_TRANSLUCENT = 1
    F_IGNOREZ = 1
    F_DISABLE_Z_WRITE = 1
    F_RENDER_BACKFACES = 1

    g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
    g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
    g_tRoughness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"

    g_flBumpStrength = 0.5
    g_vGlossinessRange = [0.5, 0.8, 0.0, 0.0]
    g_vTexCoordScale = [1.0, 1.0, 0.0, 0.0]
    g_vTexCoordOffset = [0.0, 0.0, 0.0, 0.0]
    g_flFresnelExponent = 1.5
    g_flFresnelFalloff = 0.8
    g_flFresnelMax = 0.8
    g_flFresnelMin = 0.1
    g_flOpacityScale = 0.5
})";

    static constexpr char MATERIAL_SPECIAL_VISIBLE[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
    shader = "csgo_effects.vfx"
    g_tColor = resource:"materials/dev/water_waves.vtex"
    g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
    g_tMask1 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask2 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask3 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tRoughness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
    g_tMetalness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
    g_tAmbientOcclusion = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"

    g_flOpacityScale = 7.55
    g_flFresnelExponent = 7.75
    g_flFresnelFalloff = 5.0
    g_flFresnelMax = 0.0
    g_flFresnelMin = 9.0
    F_ADDITIVE_BLEND = 1
    F_BLEND_MODE = 1
    F_TRANSLUCENT = 1
    F_IGNOREZ = 0
    F_DISABLE_Z_WRITE = 0
    F_DISABLE_Z_BUFFERING = 0
    F_RENDER_BACKFACES = 0
    g_flColorBoost = 4.0
    g_vColorTint = [7.0, 7.0, 7.0, 0.37522]
})";

    static constexpr char MATERIAL_SPECIAL_INVISIBLE[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
    shader = "csgo_effects.vfx"
    g_tColor = resource:"materials/dev/water_waves.vtex"
    g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
    g_tMask1 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask2 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask3 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tRoughness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
    g_tMetalness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
    g_tAmbientOcclusion = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"

    g_flOpacityScale = 7.55
    g_flFresnelExponent = 7.75
    g_flFresnelFalloff = 5.0
    g_flFresnelMax = 0.0
    g_flFresnelMin = 9.0
    F_ADDITIVE_BLEND = 1
    F_BLEND_MODE = 1
    F_TRANSLUCENT = 1
    F_IGNOREZ = 1
    F_DISABLE_Z_WRITE = 1
    F_DISABLE_Z_BUFFERING = 1
    F_RENDER_BACKFACES = 0
    g_flColorBoost = 4.0
    g_vColorTint = [7.0, 7.0, 7.0, 0.37522]
})";

    static constexpr char MATERIAL_SELFILLUM_VISIBLE[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
    shader = "csgo_effects.vfx"
    g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
    g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
    g_tMask1 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask2 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask3 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tRoughness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
    g_tMetalness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"

    g_flOpacityScale = 0.85
    g_flFresnelExponent = 4.5
    g_flFresnelFalloff = 3.0
    g_flFresnelMax = 1.0
    g_flFresnelMin = 0.15

    F_ADDITIVE_BLEND = 1
    F_BLEND_MODE = 1
    F_TRANSLUCENT = 1
    F_RENDER_BACKFACES = 1
})";

    static constexpr char MATERIAL_SELFILLUM_INVISIBLE[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
    shader = "csgo_effects.vfx"
    g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
    g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
    g_tMask1 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask2 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask3 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tRoughness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
    g_tMetalness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"

    g_flOpacityScale = 0.85
    g_flFresnelExponent = 4.5
    g_flFresnelFalloff = 3.0
    g_flFresnelMax = 1.0
    g_flFresnelMin = 0.15

    F_ADDITIVE_BLEND = 1
    F_BLEND_MODE = 1
    F_TRANSLUCENT = 1
    F_IGNOREZ = 1
    F_DISABLE_Z_WRITE = 1
    F_DISABLE_Z_BUFFERING = 1
    F_RENDER_BACKFACES = 1
})";

    static constexpr char MATERIAL_HOLOGRAM_VISIBLE[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
    shader = "csgo_effects.vfx"
    g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
    g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
    g_tMask1 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask2 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask3 = resource:"materials/default/default_mask_tga_344101f8.vtex"

    g_flOpacityScale = 0.85
    g_flFresnelExponent = 5.0
    g_flFresnelFalloff = 2.0
    g_flFresnelMax = 1.0
    g_flFresnelMin = 0.2

    F_ADDITIVE_BLEND = 1
    F_BLEND_MODE = 1
    F_TRANSLUCENT = 1
    F_IGNOREZ = 0
    F_DISABLE_Z_WRITE = 0
    F_RENDER_BACKFACES = 1
})";

    static constexpr char MATERIAL_HOLOGRAM_INVISIBLE[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
    shader = "csgo_effects.vfx"
    g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
    g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
    g_tMask1 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask2 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask3 = resource:"materials/default/default_mask_tga_344101f8.vtex"

    g_flOpacityScale = 0.85
    g_flFresnelExponent = 5.0
    g_flFresnelFalloff = 2.0
    g_flFresnelMax = 1.0
    g_flFresnelMin = 0.2

    F_ADDITIVE_BLEND = 1
    F_BLEND_MODE = 1
    F_TRANSLUCENT = 1
    F_IGNOREZ = 1
    F_DISABLE_Z_WRITE = 1
    F_DISABLE_Z_BUFFERING = 1
    F_RENDER_BACKFACES = 1
})";

    static constexpr char MATERIAL_GLITCH_VISIBLE[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
    shader = "csgo_effects.vfx"
    g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
    g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
    g_tMask1 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask2 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask3 = resource:"materials/default/default_mask_tga_344101f8.vtex"

    g_flOpacityScale = 1.0
    g_flFresnelExponent = 2.0
    g_flFresnelFalloff = 1.0
    g_flFresnelMax = 1.0
    g_flFresnelMin = 0.0

    F_ADDITIVE_BLEND = 1
    F_BLEND_MODE = 1
    F_TRANSLUCENT = 1
    F_IGNOREZ = 0
    F_DISABLE_Z_WRITE = 0
    F_RENDER_BACKFACES = 1
})";

    static constexpr char MATERIAL_GLITCH_INVISIBLE[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
    shader = "csgo_effects.vfx"
    g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
    g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
    g_tMask1 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask2 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask3 = resource:"materials/default/default_mask_tga_344101f8.vtex"

    g_flOpacityScale = 1.0
    g_flFresnelExponent = 2.0
    g_flFresnelFalloff = 1.0
    g_flFresnelMax = 1.0
    g_flFresnelMin = 0.0

    F_ADDITIVE_BLEND = 1
    F_BLEND_MODE = 1
    F_TRANSLUCENT = 1
    F_IGNOREZ = 1
    F_DISABLE_Z_WRITE = 1
    F_DISABLE_Z_BUFFERING = 1
    F_RENDER_BACKFACES = 1
})";

    static constexpr char MATERIAL_DIAMOND_VISIBLE[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
    shader = "csgo_effects.vfx"
    g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
    g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
    g_tMask1 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask2 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask3 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tRoughness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
    g_tMetalness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
    g_tAmbientOcclusion = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"

    g_flOpacityScale = 0.95
    g_flFresnelExponent = 3.0
    g_flFresnelFalloff = 4.0
    g_flFresnelMax = 1.0
    g_flFresnelMin = 0.2

    F_ADDITIVE_BLEND = 1
    F_BLEND_MODE = 1
    F_TRANSLUCENT = 1
    F_SELF_ILLUM = 1

    g_vColorTint = [1.2, 1.2, 1.4, 0.9]
    g_flSelfIllumScale = 0.4
})";

    static constexpr char MATERIAL_DIAMOND_INVISIBLE[] =
R"(<!-- kv3 encoding:text:version{e21c7f3c-8a33-41c5-9977-a76d3a32aa0d} format:generic:version{7412167c-06e9-4698-aff2-e63eb59037e7} -->
{
    shader = "csgo_effects.vfx"
    g_tColor = resource:"materials/dev/primary_white_color_tga_21186c76.vtex"
    g_tNormal = resource:"materials/default/default_normal_tga_7652cb.vtex"
    g_tMask1 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask2 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tMask3 = resource:"materials/default/default_mask_tga_344101f8.vtex"
    g_tRoughness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
    g_tMetalness = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"
    g_tAmbientOcclusion = resource:"materials/default/default_normal_tga_b3f4ec4c.vtex"

    g_flOpacityScale = 0.95
    g_flFresnelExponent = 3.0
    g_flFresnelFalloff = 4.0
    g_flFresnelMax = 1.0
    g_flFresnelMin = 0.2

    F_ADDITIVE_BLEND = 1
    F_BLEND_MODE = 1
    F_TRANSLUCENT = 1
    F_IGNOREZ = 1
    F_DISABLE_Z_WRITE = 1
    F_DISABLE_Z_BUFFERING = 1
    F_SELF_ILLUM = 1

    g_vColorTint = [1.2, 1.2, 1.4, 0.9]
    g_flSelfIllumScale = 0.4
})";

    static source2::CStrongHandle<source2::CMaterial2> CreateMaterialFromVmat(const char* szName, const char* szVmat) {
        source2::CStrongHandle<source2::CMaterial2> result{};

        using createmat_def = int64_t(__fastcall*)(void*, void*, const char*, void*, unsigned int, unsigned int);
        static auto createmat_fn = reinterpret_cast<createmat_def>(
            memory::PatternScanner::Find("materialsystem2.dll", "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 48 89 7C 24 20 41 56 48 81 EC 10 01 00 00 48 8B 05 ? ? ? ? 48 8B F2").As<void*>()
        );

        if (!createmat_fn) {
            utils::Logger::Error("Chams: CreateMaterial signature not found");
            return result;
        }

        source2::CKeyValues3* pKV3 = source2::CKeyValues3::CreateMaterialResource();
        if (!pKV3 || !pKV3->LoadFromBuffer(szVmat)) {
            utils::Logger::Error("Chams: KV3 parse failed for {}", szName);
            return result;
        }

        createmat_fn(nullptr, &result, szName, pKV3, 0, 1);
        return result;
    }

    bool Chams::SetupMaterials() {
        if (m_bMaterialsCreated) return true;

        m_Materials[static_cast<int>(ChamsMaterial::Solid)] = {
            CreateMaterialFromVmat("materials/lixxuid/solid_vis.vmat", MATERIAL_SOLID_VISIBLE),
            CreateMaterialFromVmat("materials/lixxuid/solid_occ.vmat", MATERIAL_SOLID_INVISIBLE)
        };

        m_Materials[static_cast<int>(ChamsMaterial::Special)] = {
            CreateMaterialFromVmat("materials/lixxuid/special_vis.vmat", MATERIAL_SPECIAL_VISIBLE),
            CreateMaterialFromVmat("materials/lixxuid/special_occ.vmat", MATERIAL_SPECIAL_INVISIBLE)
        };

        m_Materials[static_cast<int>(ChamsMaterial::Illum)] = {
            CreateMaterialFromVmat("materials/lixxuid/illum_vis.vmat", MATERIAL_SELFILLUM_VISIBLE),
            CreateMaterialFromVmat("materials/lixxuid/illum_occ.vmat", MATERIAL_SELFILLUM_INVISIBLE)
        };

        m_Materials[static_cast<int>(ChamsMaterial::Hologram)] = {
            CreateMaterialFromVmat("materials/lixxuid/hologram_vis.vmat", MATERIAL_HOLOGRAM_VISIBLE),
            CreateMaterialFromVmat("materials/lixxuid/hologram_occ.vmat", MATERIAL_HOLOGRAM_INVISIBLE)
        };

        m_Materials[static_cast<int>(ChamsMaterial::Glitch)] = {
            CreateMaterialFromVmat("materials/lixxuid/glitch_vis.vmat", MATERIAL_GLITCH_VISIBLE),
            CreateMaterialFromVmat("materials/lixxuid/glitch_occ.vmat", MATERIAL_GLITCH_INVISIBLE)
        };

        m_Materials[static_cast<int>(ChamsMaterial::Diamond)] = {
            CreateMaterialFromVmat("materials/lixxuid/diamond_vis.vmat", MATERIAL_DIAMOND_VISIBLE),
            CreateMaterialFromVmat("materials/lixxuid/diamond_occ.vmat", MATERIAL_DIAMOND_INVISIBLE)
        };

        int ready = 0;
        for (const auto& pair : m_Materials) {
            if (static_cast<source2::CMaterial2*>(pair.visible) && static_cast<source2::CMaterial2*>(pair.occluded)) {
                ++ready;
            }
        }

        m_bMaterialsCreated = (ready > 0);
        utils::Logger::Info("Chams: Created materials {}/{}", ready, static_cast<int>(ChamsMaterial::End));
        return m_bMaterialsCreated;
    }

    void Chams::HookDrawArray() {
        if (m_pDrawArrayTarget) return;

        auto addr = memory::PatternScanner::Find(
            "scenesystem.dll",
            "48 8B C4 48 89 50 10 48 89 48 08 53 56 41 57 48 81 EC C0 00 00 00 49 63 F1"
        );

        if (!addr.IsValid()) {
            utils::Logger::Error("Failed to find scenesystem.dll DrawArray signature");
            return;
        }

        m_pDrawArrayTarget = addr.As<void*>();
        if (MH_CreateHook(m_pDrawArrayTarget, reinterpret_cast<void*>(&HookedDrawArray), reinterpret_cast<void**>(&OriginalDrawArray)) == MH_OK) {
            MH_EnableHook(m_pDrawArrayTarget);
            utils::Logger::Info("Chams: DrawArray hooked successfully.");
        } else {
            utils::Logger::Error("Chams: Failed to hook DrawArray with MinHook.");
        }
    }

    void Chams::UnhookDrawArray() {
        if (m_pDrawArrayTarget) {
            MH_DisableHook(m_pDrawArrayTarget);
            m_pDrawArrayTarget = nullptr;
            OriginalDrawArray = nullptr;
        }
    }

    void Chams::Initialize() {
        if (m_bInitialized) return;
        m_bInitialized = true;

        SetupMaterials();
        HookDrawArray();
    }

    void Chams::Shutdown() {
        UnhookDrawArray();
        m_bInitialized = false;
        m_bMaterialsCreated = false;
    }

    void __fastcall Chams::HookedDrawArray(
        void* pAnimatableSceneObjectDesc,
        void* pDx11,
        source2::CBaseSceneData* data,
        int count,
        void* pSceneView,
        void* pSceneLayer,
        void* pUnk
    ) {
        if (!OriginalDrawArray) return;

        if (!Config.m_bEnabled || !m_bMaterialsCreated || !data || count <= 0) {
            return OriginalDrawArray(pAnimatableSceneObjectDesc, pDx11, data, count, pSceneView, pSceneLayer, pUnk);
        }

        auto localPawn = source2::EntitySystem::GetLocalPlayerPawn();
        uint8_t localTeam = localPawn ? localPawn->GetTeamNum() : 0;

        std::vector<source2::CBaseSceneData> visible(data, data + count);
        std::vector<source2::CBaseSceneData> occluded;
        occluded.reserve(count);
        bool changed = false;

        const int matIdx = static_cast<int>(Config.m_Material);
        if (matIdx < 0 || matIdx >= static_cast<int>(ChamsMaterial::End)) {
            return OriginalDrawArray(pAnimatableSceneObjectDesc, pDx11, data, count, pSceneView, pSceneLayer, pUnk);
        }

        const auto& matPair = m_Materials[matIdx];
        source2::CMaterial2* normalMat = matPair.visible;
        source2::CMaterial2* hiddenMat = matPair.occluded;

        if (!normalMat) {
            return OriginalDrawArray(pAnimatableSceneObjectDesc, pDx11, data, count, pSceneView, pSceneLayer, pUnk);
        }

        source2::CMaterial2* normalForView = normalMat->GetForView(pSceneView);
        source2::CMaterial2* hiddenForView = (Config.m_bIgnoreZ && hiddenMat) ? hiddenMat->GetForView(pSceneView) : nullptr;

        for (int i = 0; i < count; ++i) {
            auto& mesh = visible[i];
            if (!mesh.pSceneAnimatableObject || !mesh.pMaterial) continue;

            const char* matName = mesh.pMaterial->GetName();
            if (!matName || strstr(matName, "glowproperty") != nullptr) continue;

            uint32_t ownerHandle = mesh.pSceneAnimatableObject->hOwner;
            auto entity = source2::EntitySystem::GetEntityFromHandle(ownerHandle);
            if (!entity) continue;

            auto pawn = reinterpret_cast<source2::C_CSPlayerPawn*>(entity);
            if (!pawn->IsAlive()) continue;

            bool isLocal = (pawn == localPawn);
            if (isLocal && !Config.m_bLocalPlayer) continue;

            bool isEnemy = (localTeam == 0) ? true : (pawn->GetTeamNum() != localTeam);
            if (!isLocal && Config.m_bTeamCheck && !isEnemy) continue;

            auto setColor = [](source2::CBaseSceneData& target, const float* col) {
                target.r = static_cast<uint8_t>(col[0] * 255.f);
                target.g = static_cast<uint8_t>(col[1] * 255.f);
                target.b = static_cast<uint8_t>(col[2] * 255.f);
                target.a = static_cast<uint8_t>(col[3] * 255.f);
            };

            // Occluded (through walls) pass
            if (hiddenForView) {
                auto hidden = mesh;
                hidden.pMaterial = hiddenMat;
                hidden.pMaterialForView = hiddenForView;
                setColor(hidden, Config.m_flOccludedColor);
                occluded.push_back(hidden);
            }

            // Visible pass
            if (normalForView) {
                mesh.pMaterial = normalMat;
                mesh.pMaterialForView = normalForView;
                setColor(mesh, Config.m_flVisibleColor);
                changed = true;
            }
        }

        if (!changed) {
            return OriginalDrawArray(pAnimatableSceneObjectDesc, pDx11, data, count, pSceneView, pSceneLayer, pUnk);
        }

        // Draw occluded first, then visible
        if (!occluded.empty()) {
            OriginalDrawArray(pAnimatableSceneObjectDesc, pDx11, occluded.data(), static_cast<int>(occluded.size()), pSceneView, pSceneLayer, pUnk);
        }
        OriginalDrawArray(pAnimatableSceneObjectDesc, pDx11, visible.data(), count, pSceneView, pSceneLayer, pUnk);
    }

} // namespace features
