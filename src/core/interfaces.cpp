#include "interfaces.hpp"
#include "src/memory/pattern.hpp"
#include "src/utils/log.hpp"
#include <d3d11.h>

namespace core {

    using CreateInterfaceFn = void* (*)(const char*, int*);

    template <typename T>
    T* Interfaces::Get(std::string_view moduleName, std::string_view interfaceName) {
        HMODULE handle = GetModuleHandleA(moduleName.data());
        if (!handle) return nullptr;

        auto createInterface = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(handle, "CreateInterface"));
        if (!createInterface) return nullptr;

        return reinterpret_cast<T*>(createInterface(interfaceName.data(), nullptr));
    }

    static bool ValidateSwapChain(source2::CSwapChain* candidate) {
        if (!candidate) return false;
        __try {
            if (!candidate->dxgi_swapchain) return false;
            ID3D11Device* testDevice = nullptr;
            HRESULT hr = candidate->dxgi_swapchain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&testDevice));
            if (SUCCEEDED(hr) && testDevice) {
                testDevice->Release();
                return true;
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            return false;
        }
        return false;
    }

    bool Interfaces::Initialize() {
        SchemaSystem = Get<source2::ISchemaSystem>("schemasystem.dll", "SchemaSystem_001");
        EngineClient = Get<IEngineClient>("engine2.dll", "Source2EngineToClient001");

        // 1. Resolve SwapChain from rendersystemdx11.dll
        auto swapChainSig = memory::PatternScanner::Find("rendersystemdx11.dll", "48 89 2D ? ? ? ? 66 0F 7F 05 ? ? ? ? FF 15");
        if (swapChainSig) {
            auto relAddr = swapChainSig.Rip(3, 7);
            if (relAddr.IsValid()) {
                __try {
                    uintptr_t ptr1 = *reinterpret_cast<uintptr_t*>(relAddr.m_ptr);
                    if (ptr1) {
                        // Check if ptr1 is CSwapChain directly
                        if (ValidateSwapChain(reinterpret_cast<source2::CSwapChain*>(ptr1))) {
                            SwapChain = reinterpret_cast<source2::CSwapChain*>(ptr1);
                            utils::Logger::Info("SwapChain verified directly at level 1: {:p}", static_cast<void*>(SwapChain));
                        } else {
                            // Check if ptr1 is pointer to CSwapChain (level 2)
                            uintptr_t ptr2 = *reinterpret_cast<uintptr_t*>(ptr1);
                            if (ptr2 && ValidateSwapChain(reinterpret_cast<source2::CSwapChain*>(ptr2))) {
                                SwapChain = reinterpret_cast<source2::CSwapChain*>(ptr2);
                                utils::Logger::Info("SwapChain verified at level 2: {:p}", static_cast<void*>(SwapChain));
                            }
                        }
                    }
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    utils::Logger::Error("Exception probing SwapChain address");
                }
            }
        }

        // 2. Resolve EntitySystem from client.dll
        auto entitySystemAddr = memory::PatternScanner::Find("client.dll", "48 8B 0D ? ? ? ? 4C 8D 05 ? ? ? ? 48 8D 54 24 ? E8");
        if (!entitySystemAddr) {
            entitySystemAddr = memory::PatternScanner::Find("client.dll", "48 8B 0D ? ? ? ? 48 89 7C 24 60 8B FB");
        }
        if (entitySystemAddr) {
            auto resolved = entitySystemAddr.Rip(3, 7);
            if (resolved.IsValid()) {
                __try {
                    GameEntitySystem = *reinterpret_cast<void**>(resolved.m_ptr);
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    GameEntitySystem = nullptr;
                }
            }
        }

        // 3. Resolve Entity Helper Functions
        auto getBaseEntityAddr = memory::PatternScanner::Find("client.dll", "4C 8D 49 ? 81 FA");
        if (getBaseEntityAddr) {
            GetBaseEntity = getBaseEntityAddr.As<GetBaseEntityFn>();
        }

        auto getLocalControllerAddr = memory::PatternScanner::Find("client.dll", "E8 ? ? ? ? 48 85 C0 74 ? 8B CF");
        if (getLocalControllerAddr) {
            GetLocalController = getLocalControllerAddr.Rip(1, 5).As<GetLocalControllerFn>();
        }

        auto getLocalPawnAddr = memory::PatternScanner::Find("client.dll", "E8 ? ? ? ? 48 85 C0 74 ? B8 ? ? ? ? 48 C7 44 24");
        if (getLocalPawnAddr) {
            GetLocalPawn = getLocalPawnAddr.Rip(1, 5).As<GetLocalPawnFn>();
        }

        // 4. Resolve ViewMatrix
        auto viewMatrixAddr = memory::PatternScanner::Find("client.dll", memory::PatternScanner::Signatures::ViewMatrix);
        if (viewMatrixAddr) {
            auto resolved = viewMatrixAddr.Rip();
            if (resolved.IsValid()) {
                ViewMatrix = resolved.As<math::Matrix4x4*>();
                utils::Logger::Info("ViewMatrix resolved at: {:p}", static_cast<void*>(ViewMatrix));
            }
        }

        return (SwapChain != nullptr && SwapChain->dxgi_swapchain != nullptr);
    }

} // namespace core
