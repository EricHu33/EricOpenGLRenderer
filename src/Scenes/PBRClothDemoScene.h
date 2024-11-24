#pragma once
#include "imgui.h"
#include "../Scene.h"
#include "../DebugConfig.h"
namespace EricScene
{
    struct PBRClothDemoScene : Scene
    {
        void CreateScene() override;
        void Update(float dt) override;
        void DeleteScene() override;
        std::vector<std::shared_ptr<Renderer>> testCloths;
        std::vector<int> rotateCloths;
        float rotateTheta = 0;
        virtual void OnGUI() override
        {
           // ImGui::SliderFloat("Point Light Radius", &Debug::DebugConfig::pointLightRadiusFalloff, 0.1f, 8.0f);
            ImGui::SliderFloat("Roughness", &Debug::DebugConfig::demoSphereRoughness, 0.0f, 1.0f);
            ImGui::Checkbox("Use Thread Map", &Debug::DebugConfig::demoSphereUseThreadMap);
            ImGui::InputFloat("Thread map scale", &Debug::DebugConfig::demoSphereThreadScale ,1.0f, 1.0f);
            ImGui::SliderFloat("Thread Strength", &Debug::DebugConfig::demoSphereThreadStrength, 0.0f, 2.0f);
            ImGui::SliderFloat("Thread AO", &Debug::DebugConfig::demoSphereThreadAO, 0.0f, 1.0f);
            ImGui::InputFloat("Demo Fuzz Scale", &Debug::DebugConfig::demoSphereFuzzScale, 1.0f, 1.0f);
            ImGui::SliderFloat("Demo Fuzz Strength", &Debug::DebugConfig::demoSphereFuzzStrength, 0.0f, 1.0f);
            ImGui::ColorPicker4("Base Color", &Debug::DebugConfig::testSphereColor.x);
            ImGui::ColorPicker4("Sheen Color", &Debug::DebugConfig::testSphereColor2.x);
            ImGui::Checkbox("Use SSS", &Debug::DebugConfig::useSSS);
            ImGui::ColorPicker4("SSS Color", &Debug::DebugConfig::testSphereColor3.x);
           //  ImGui::SliderFloat("Demo Metallic", &Debug::DebugConfig::demoSphereMetallic,  0.0f, 1.0f);
            //ImGui::SliderFloat("Clear Coat", &Debug::DebugConfig::demoSphereClearCoat, 0.0f, 1.0f);
            //ImGui::SliderFloat("Clear Coat Roughness", &Debug::DebugConfig::demoSphereClearCoatRoughness,  0.0f, 1.0f);
            //ImGui::SliderFloat("Anisotropy", &Debug::DebugConfig::demoSphereAnisotropy, -1.0f, 1.0f);
        }
        void drawPBRSpheresForward(int x, int y, glm::vec3 origin, Material mat, PBRType materialType, glm::vec4 color);
    };
}