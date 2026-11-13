#pragma once
#include <cstdint>
#include <cstddef>
#include <type_traits>

namespace memory {

    class Address {
    public:
        uintptr_t m_ptr{0};

        constexpr Address() = default;
        constexpr Address(uintptr_t ptr) : m_ptr(ptr) {}
        constexpr Address(int ptr) : m_ptr(static_cast<uintptr_t>(ptr)) {}
        constexpr Address(std::nullptr_t) : m_ptr(0) {}
        Address(const void* ptr) : m_ptr(reinterpret_cast<uintptr_t>(ptr)) {}

        [[nodiscard]] bool IsValid() const noexcept {
            return m_ptr >= 0x10000 && m_ptr < 0x7FFFFFFFFFFF;
        }

        template <typename T>
        [[nodiscard]] T As() const noexcept {
            return reinterpret_cast<T>(m_ptr);
        }

        template <typename T>
        [[nodiscard]] T Get(size_t offset = 0) const noexcept {
            if (!IsValid()) return T{};
            return *reinterpret_cast<T*>(m_ptr + offset);
        }

        [[nodiscard]] Address Offset(ptrdiff_t offset) const noexcept {
            if (!m_ptr) return Address{0};
            return Address{m_ptr + offset};
        }

        [[nodiscard]] Address Deref(size_t count = 1) const noexcept {
            uintptr_t cur = m_ptr;
            while (count-- && cur) {
                if (cur < 0x10000 || cur >= 0x7FFFFFFFFFFF) return Address{0};
                cur = *reinterpret_cast<uintptr_t*>(cur);
            }
            return Address{cur};
        }

        /// Resolves x64 RIP-relative addressing: [Instruction + opCodeSize + disp32]
        [[nodiscard]] Address Rip(ptrdiff_t rvaOffset = 3, ptrdiff_t instructionSize = 7) const noexcept {
            if (!IsValid()) return Address{0};
            int32_t displacement = Get<int32_t>(rvaOffset);
            return Address{m_ptr + instructionSize + displacement};
        }

        explicit operator bool() const noexcept { return IsValid(); }
        explicit operator uintptr_t() const noexcept { return m_ptr; }
        explicit operator void*() const noexcept { return reinterpret_cast<void*>(m_ptr); }
    };

} // namespace memory
