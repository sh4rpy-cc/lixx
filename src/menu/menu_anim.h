#pragma once
#include <imgui.h>
#include <unordered_map>
#include <string>
#include <cmath>
#include <algorithm>

#undef min
#undef max

namespace lixxuid::ui {

    class anim_manager {
    public:
        static float get_anim(const std::string& id, bool condition, float speed = 1.0f) {
            auto& anim = animations[id];
            float delta = ImGui::GetIO().DeltaTime * (speed * 10.0f);

            if (condition) {
                anim = std::min(anim + delta, 1.0f);
            }
            else {
                anim = std::max(anim - delta, 0.0f);
            }

            return anim;
        }

        static float ease_out_expo(float x) {
            return x == 1.0f ? 1.0f : 1.0f - powf(2.0f, -10.0f * x);
        }

    private:
        static inline std::unordered_map<std::string, float> animations;
    };
}