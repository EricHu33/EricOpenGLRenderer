#pragma once
#include "imgui.h"
#include "../Scene.h"
#include "../DebugConfig.h"
namespace EricScene
{
    struct PBRSphereDemoScene : Scene
    {
        void CreateScene() override;
        void Update(float dt) override;
        void DeleteScene() override;
        virtual void OnGUI() override
        {
            ImGui::SliderFloat("Point Light Radius", &Debug::DebugConfig::pointLightRadiusFalloff, 0.1f, 8.0f);
            ImGui::SliderFloat("DemoSphere Roughness", &Debug::DebugConfig::demoSphereRoughness, 0.0f, 1.0f);
            ImGui::SliderFloat("DemoSphere Metallic", &Debug::DebugConfig::demoSphereMetallic,  0.0f, 1.0f);
            ImGui::SliderFloat("Clear Coat", &Debug::DebugConfig::demoSphereClearCoat, 0.0f, 1.0f);
            ImGui::SliderFloat("Clear Coat Roughness", &Debug::DebugConfig::demoSphereClearCoatRoughness,  0.0f, 1.0f);
            ImGui::SliderFloat("Anisotropy", &Debug::DebugConfig::demoSphereAnisotropy, -1.0f, 1.0f);
        }

    private:
        std::vector<std::shared_ptr<Renderer>> m_anisoRenderer;
        std::vector<std::shared_ptr<Renderer>> m_standardRenderers;
        void drawPBRSpheresForward(int x, int y, glm::vec3 origin, Material mat, PBRType materialType, glm::vec4 color);
        void drawPBRSpheresDeferred(int x, int y, glm::vec3 origin, Material mat, PBRType materialType, glm::vec4 color);
    };
}