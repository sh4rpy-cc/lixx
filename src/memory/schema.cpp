#include "schema.hpp"
#include "src/core/interfaces.hpp"
#include "src/utils/log.hpp"
#include <unordered_map>
#include <string>
#include <cstring>

namespace source2 {

    static std::unordered_map<std::string, int32_t> g_schemaCache;

    void SchemaSystem::Initialize() {
        g_schemaCache.clear();
    }

    int32_t SchemaSystem::QuerySchema(std::string_view moduleName, std::string_view className, std::string_view memberName) {
        if (!core::Interfaces::SchemaSystem) return 0;

        __try {
            CSchemaTypeScope* typeScope = core::Interfaces::SchemaSystem->FindTypeScopeForModule(moduleName.data());
            if (!typeScope) return 0;

            CSchemaClassInfo* schemaClass = typeScope->FindDeclaredClass(className.data());
            if (!schemaClass || !schemaClass->fields) return 0;

            for (uint16_t i = 0; i < schemaClass->num_fields; ++i) {
                const auto& field = schemaClass->fields[i];
                if (field.name && memberName == field.name) {
                    return static_cast<int32_t>(field.offset);
                }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            utils::Logger::Error("Exception inside QuerySchema: {}::{}", className, memberName);
            return 0;
        }

        return 0;
    }

    int32_t SchemaSystem::GetOffset(std::string_view moduleName, std::string_view className, std::string_view memberName) {
        std::string key = std::string(moduleName) + "->" + std::string(className) + "->" + std::string(memberName);

        if (auto it = g_schemaCache.find(key); it != g_schemaCache.end()) {
            return it->second;
        }

        int32_t offset = QuerySchema(moduleName, className, memberName);
        if (offset != 0) {
            g_schemaCache[key] = offset;
            utils::Logger::Info("Schema: {} = 0x{:X}", key, offset);
        } else {
            utils::Logger::Error("Failed to resolve schema: {}", key);
        }

        return offset;
    }

} // namespace source2
