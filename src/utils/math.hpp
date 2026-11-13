#pragma once
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace math {

    struct Vector2 {
        float x{0.f}, y{0.f};

        constexpr Vector2() = default;
        constexpr Vector2(float _x, float _y) : x(_x), y(_y) {}

        Vector2 operator+(const Vector2& o) const noexcept { return {x + o.x, y + o.y}; }
        Vector2 operator-(const Vector2& o) const noexcept { return {x - o.x, y - o.y}; }
        Vector2 operator*(float f) const noexcept { return {x * f, y * f}; }
        Vector2 operator/(float f) const noexcept { return {x / f, y / f}; }

        [[nodiscard]] float Length() const noexcept { return std::sqrt(x * x + y * y); }
        [[nodiscard]] float DistTo(const Vector2& o) const noexcept { return (*this - o).Length(); }
    };

    struct Vector3 {
        float x{0.f}, y{0.f}, z{0.f};

        constexpr Vector3() = default;
        constexpr Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

        Vector3 operator+(const Vector3& o) const noexcept { return {x + o.x, y + o.y, z + o.z}; }
        Vector3 operator-(const Vector3& o) const noexcept { return {x - o.x, y - o.y, z - o.z}; }
        Vector3 operator*(float f) const noexcept { return {x * f, y * f, z * f}; }
        Vector3 operator/(float f) const noexcept { return {x / f, y / f, z / f}; }

        [[nodiscard]] float Length() const noexcept {
            return std::sqrt(x * x + y * y + z * z);
        }

        [[nodiscard]] float LengthSqr() const noexcept {
            return x * x + y * y + z * z;
        }

        [[nodiscard]] float Length2D() const noexcept {
            return std::sqrt(x * x + y * y);
        }

        [[nodiscard]] float DistTo(const Vector3& o) const noexcept {
            return (*this - o).Length();
        }
    };

    struct Matrix4x4 {
        float m[4][4]{};

        const float* operator[](int index) const noexcept { return m[index]; }
        float* operator[](int index) noexcept { return m[index]; }
    };

    struct Matrix3x4_t {
        float m[3][4]{};
    };

    struct Matrix2x4_t {
        float array[2][4]{};

        [[nodiscard]] Vector3 GetOrigin(int index = 0) const noexcept {
            return Vector3(
                array[0][0],
                array[0][1],
                array[0][2]
            );
        }

        void SetOrigin(const Vector3& vec) noexcept {
            array[0][0] = vec.x;
            array[0][1] = vec.y;
            array[0][2] = vec.z;
        }
    };

    inline bool WorldToScreen(const Vector3& world, Vector2& screen, const Matrix4x4& vMatrix, float screenWidth, float screenHeight) {
        float w = vMatrix[3][0] * world.x + vMatrix[3][1] * world.y + vMatrix[3][2] * world.z + vMatrix[3][3];
        if (w < 0.001f) return false;

        float invW = 1.0f / w;
        float x = (vMatrix[0][0] * world.x + vMatrix[0][1] * world.y + vMatrix[0][2] * world.z + vMatrix[0][3]) * invW;
        float y = (vMatrix[1][0] * world.x + vMatrix[1][1] * world.y + vMatrix[1][2] * world.z + vMatrix[1][3]) * invW;

        screen.x = (screenWidth * 0.5f) + (0.5f * x * screenWidth + 0.5f);
        screen.y = (screenHeight * 0.5f) - (0.5f * y * screenHeight + 0.5f);
        return true;
    }

} // namespace math
