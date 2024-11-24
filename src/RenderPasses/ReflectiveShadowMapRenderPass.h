#pragma once

#include <vector>
#include "../RenderPass.h"
#include "../Classes/Shader.h"
#include "../Classes/Camera.h"
#include "../Scene.h"

class ReflectiveShadowMapRenderPass : RenderPass
{
public:
    void Create(int windowWidth, int windowHeight) override;
    void Render(float dt) override;
    void Dispose() override;
    void CreateShadowMatricesUBO(int uboIndex);
    std::vector<float> GetCsmSizes(){ return m_csmFrustumSizes; }
    unsigned int GetRSMDepth() { return m_rsmDepth; }
    unsigned int GetPositionBuffer() { return m_rsmPosition; }
    unsigned int GetNormalBuffer() { return m_rsmNormal; }
    unsigned int GetFluxBuffer() { return m_rsmFlux; }
    Camera* Camera = NULL;
    glm::vec3 LightDirection = glm::vec3(1);
    float Fov = 0;
    float Near = 0;
    float Far = 0;
    std::vector<float> shadowCascadeLevels;
    size_t ShadowCasterCount = 0;
    std::vector<std::shared_ptr<SceneObject>> ShadowCastRenderers;
    Shader* RSMDepthShader = nullptr;
    Shader* RSMColorShader = nullptr;
    
private:
    std::vector<glm::mat4> GetCascadeShadowMatrices();
    glm::mat4 GetShadowMatrices(int i ,float nearPlane, float farPlane);
    void UpdateShadowMatricesUBO();

    
    const int m_numCascades = 1;
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    std::vector<float> m_csmFrustumSizes;
    const unsigned SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    unsigned int m_rsmDepthFBO;
    unsigned int m_rsmFBO;
    //target rextures
    unsigned int m_rsmPosition = 0;
    unsigned int m_rsmNormal = 0;
    unsigned int m_rsmFlux = 0;
    unsigned int m_rsmAttachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    //RBO
    unsigned int m_rbo = 0;
    
    unsigned int m_rsmDepth = 0;
    unsigned int m_rsmMatricesUBO = 0;

};