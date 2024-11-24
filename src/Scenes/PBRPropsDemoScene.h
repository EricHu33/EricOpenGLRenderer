#pragma once
#include "imgui.h"
#include "../Scene.h"
#include "../DebugConfig.h"
namespace EricScene
{
    struct PBRPropsDemoScene : Scene
    {
        glm::quat rotate_angle_axis(float angle, glm::vec3 axis);
        void CreateScene() override;
        void Update(float dt) override;
        void DeleteScene() override;
        glm::vec3 directionalLightPosition;
        glm::vec3 directionalLightEuler;
        virtual void OnGUI() override
        {
            ImGui::SliderFloat3("Sun Rotation", &Debug::DebugConfig::directionalLightEuler.x, 0, 360, "%.4f");
            ImGui::SliderFloat("Point Light Radius", &Debug::DebugConfig::pointLightRadiusFalloff, 0.1f, 8.0f);
        }
    };
}
