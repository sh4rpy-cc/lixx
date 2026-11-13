#include "hooks.hpp"
#include "interfaces.hpp"
#include "src/render/render.hpp"
#include "src/memory/schema.hpp"
#include "src/features/chams/chams.hpp"
#include "src/features/removals/removals.hpp"
#include "src/utils/log.hpp"
#include <minhook.h>
#include <thread>
#include <chrono>

namespace core {

    bool Hooks::Initialize() {
        utils::Logger::Info("Initializing LIXXUID Core...");

        if (MH_Initialize() != MH_OK) {
            utils::Logger::Error("Failed to initialize MinHook");
            return false;
        }

        // Retry interface and SwapChain resolution
        bool interfacesReady = false;
        for (int attempt = 0; attempt < 20; ++attempt) {
            if (Interfaces::Initialize()) {
                interfacesReady = true;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(400));
        }

        if (!interfacesReady) {
            utils::Logger::Error("Failed to resolve game interfaces / SwapChain after multiple attempts");
            return false;
        }

        source2::SchemaSystem::Initialize();
        features::Chams::Initialize();
        features::Removals::Initialize();

        if (!render::RenderManager::Initialize()) {
            utils::Logger::Error("Failed to initialize RenderManager on game SwapChain");
            return false;
        }

        utils::Logger::Info("Hooks initialized successfully. Menu key: INSERT or DELETE");
        return true;
    }

    void Hooks::Shutdown() {
        utils::Logger::Info("Shutting down LIXXUID Core...");
        features::Removals::Shutdown();
        features::Chams::Shutdown();
        render::RenderManager::Shutdown();
        MH_Uninitialize();
        ShouldUnload = true;
    }

} // namespace core
