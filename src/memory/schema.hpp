#pragma once
#include <string_view>
#include <cstdint>
#include <unordered_map>
#include "src/memory/address.hpp"

namespace source2 {

    class SchemaSystem {
    public:
        static void Initialize();
        static int32_t GetOffset(std::string_view moduleName, std::string_view className, std::string_view memberName);

    private:
        static int32_t QuerySchema(std::string_view moduleName, std::string_view className, std::string_view memberName);
    };

    #define SCHEMA_FIELD(type, name, moduleName, className, memberName) \
        type& name() { \
            static const int32_t offset = source2::SchemaSystem::GetOffset(moduleName, className, memberName); \
            return *reinterpret_cast<type*>(reinterpret_cast<uintptr_t>(this) + offset); \
        }

} // namespace source2
