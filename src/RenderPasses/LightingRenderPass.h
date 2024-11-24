#pragma once

#include "../RenderPass.h"
#include "../Classes/Shader.h"
#include "../Classes/Renderer.h"
#include "../LightDefine.h"

class LightingRenderPass : RenderPass
{
public:
    void Create(int windowWidth, int windowHeight) override;
    void Render(float dt) override;
    void Dispose() override;
    unsigned int GetHDRTargets(int index) { return m_hdrBuffers[index]; }
    unsigned int GetLightingPassFBO() { return m_lightingPassFBO; }

    unsigned int GeometryPassFBO;
    std::vector<PointLight> PointLights;
    size_t PointLightsCount;
    Shader* DirectionalLightShader;
    DirectionalLight DirectionalLight;
    unsigned int FullScreenQuad4VAO = 0;
    std::shared_ptr<Renderer> punctualLightRenderer = nullptr;
    unsigned int gPosition = 0;
    unsigned int gNormal = 0;
    unsigned int gAlbedoSpec = 0;
    unsigned int gMaterialParam = 0;
    unsigned int ssao = 0;
    unsigned int diffuseIrradiance = 0;
    unsigned int prefilterMap = 0;
    unsigned int brdfLUT = 0;
    unsigned int cascadeShadowMapArray;
    float _far = 0.0f;;
    std::vector<float> shadowCascadeLevels;
    std::vector<float> shadowCascadeFrustumSizes;
    float BloomThreshold = 0;
    float Exposure;
    float PreComputedEnvIntensity = 0.0f;
    float PreComputedLightIntensity = 0.0f;
    float normalBias = 0.0f;
    float depthBias = 0.0f;
    float pcssSearchRadius;
    float pcssFilterRadius;
    float pcssLightWorldSize;
    glm::vec3 AltShadowColor = glm::vec3(0);
    float nearPlane;
    glm::vec3 CameraPosition = glm::vec3(0);
    glm::mat4 ViewMatrix = glm::mat4(1);
    glm::mat4 ProjMatrix = glm::mat4(1);
private:
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    //FBO
    unsigned int m_lightingPassFBO = 0;
    //RBO
    unsigned int m_rbo = 0;
    //target rextures
    unsigned int m_hdrBuffers[2];
    unsigned int m_attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    size_t m_positionMapKey;
    size_t m_normalMapKey;
    size_t m_albedoKey;
    size_t m_matParamKey;
    size_t m_shadowKey;
    size_t m_ssaoKey;
    size_t m_prefiterKey;
    size_t m_brdfKey;
    void PreparePuncutalLightPass();
    void DrawPointLights();
    void DrawDirectionalLight();
    void RenderQuad4();
};
