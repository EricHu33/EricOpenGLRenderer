#pragma once
#include "imgui.h"
#include "../Scene.h"
#include "../DebugConfig.h"
namespace EricScene
{
    struct PBRLightingUnitTestScene : Scene
    {
        glm::quat rotate_angle_axis(float angle, glm::vec3 axis);
        void CreateScene() override;
        void Update(float dt) override;
        void DeleteScene() override;
        std::vector<int> directionlLightCubeIndex;
        glm::vec3 directionalLightPosition;
        glm::vec3 directionalLightEuler;
        float pointLightFalloffRadius;
        glm::vec4 testSphereColor = glm::vec4(0.18f, 0.18f, 0.18f, 1);
        std::vector<std::shared_ptr<Renderer>> testSpheres;
        virtual void OnGUI() override
        {
            ImGui::SliderFloat3("Sun Rotation", &Debug::DebugConfig::directionalLightEuler.x, 0, 360);
            ImGui::SliderFloat("Point Light Radius", &Debug::DebugConfig::pointLightRadiusFalloff, 0.1f, 8.0f);
            ImGui::ColorPicker3("Test Sphere Color", &Debug::DebugConfig::testSphereColor.x);
        }
    };
}