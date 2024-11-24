#pragma once
#include "imgui.h"
#include "../DebugConfig.h"
#include "../Scene.h"

namespace EricScene
{
    struct SponzaDemoScene : Scene
    {
        glm::quat rotate_angle_axis(float angle, glm::vec3 axis)
        {
            float sn = sin(angle * 0.5);
            float cs = cos(angle * 0.5);
            return glm::quat(axis.x * sn, axis.y * sn, axis.z * sn, cs);
        }
        void CreateScene() override;
        void Update(float dt) override;
        void DeleteScene() override;
        virtual void OnGUI() override
        {
            ImGui::SliderFloat3("Sun Rotation", &Debug::DebugConfig::directionalLightEuler.x, 0, 360, "%.4f");
            ImGui::SliderFloat("Point Light Radius", &Debug::DebugConfig::pointLightRadiusFalloff, 0.1f, 8.0f);
        }
    };
}