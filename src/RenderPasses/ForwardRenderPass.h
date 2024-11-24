#pragma once

#include "../RenderPass.h"
#include "../Classes/Shader.h"
#include "../Classes/Renderer.h"
#include "../LightDefine.h"
#include "../Scene.h"
class ForwardRenderPass : RenderPass
{
public:
    void Create(int windowWidth, int windowHeight) override;
    void Render(float dt) override;
    void Dispose() override;
    unsigned int GetFowardPathBuffer() { return m_forwardBuffer; }
    unsigned int GetSkyBoxVAO() { return m_skyboxVAO; }

    unsigned int zPrepass;
    unsigned int GeometryPassFBO = 0;
    unsigned int CurrentFullScreenFBO = 0;
    unsigned int SkyboxTexture;

    bool DrawDebugLightSourceCubes;
    bool IsDeferred = false;
    std::shared_ptr<Renderer> DebugLightSourceRenderer;
    std::vector<PointLight> ForwardPassLightDatas;
    std::vector<std::shared_ptr<SceneObject>> ForwardPassInstancingRenderer;
    std::vector<std::shared_ptr<SceneObject>> ForwardPassMeshOpqaueRenderer;
    std::vector<std::shared_ptr<SceneObject>> ForwardPassMeshTransparentRenderer;
    Shader* SkyBoxShader;

    //RSM
    unsigned int rsmDepth = 0;
    unsigned int rsmPosition = 0;
    unsigned int rsmNormal = 0;
    unsigned int rsmFlux = 0;
    float RsmMaxRadius = 0.16f;
    float RsmFluxStrength = 0.5f;

    unsigned int indirectLightingMap = 0;
    
    //SSAO
    unsigned int ssao = 0;
    unsigned int diffuseIrradiance = 0;
    unsigned int prefilterMap = 0;
    unsigned int brdfLUT = -1;
    unsigned int cascadeShadowMapArray = 0;
    std::vector<float> shadowCascadeLevels;
    std::vector<float> shadowCascadeFrustumSizes;
    DirectionalLight DirectionalLight;
    glm::vec3 CameraPosition;
    float Exposure;
    float PreComputedEnvIntensity = 0.0f;
    float PreComputedLightIntensity = 0.0f;
    float normalBias = 0.0f;
    float depthBias = 0.0f;
    float pcssSearchRadius;
    float pcssFilterRadius;
    
    float bloomThreshold = 0;
    glm::mat4 ViewMatrix;
    glm::mat4 ProjectionMatrix;
private:
    unsigned int m_skyboxVAO = 0, m_skyboxVBO = 0;
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    unsigned int m_forwardFBO;
    unsigned int m_rbo;
    //target rextures
    unsigned int m_forwardBuffer = 0;
    void DrawRenderers(std::vector<std::shared_ptr<SceneObject>> sceneObjects);
    void DrawInstancingRenderers(std::vector<std::shared_ptr<SceneObject>> sceneObjects);
    std::vector<float> GetSkyboxVertices();
};