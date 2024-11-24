#pragma once

#include <vector>
#include "../RenderPass.h"
#include "../Classes/Shader.h"
#include "../Classes/Renderer.h"
#include "../Classes/Camera.h"
#include "../Scene.h"

class CascadedShadowMapRenderPass : RenderPass
{
public:
    void Create(int windowWidth, int windowHeight) override;
    void Render(float dt) override;
    void Dispose() override;
    void CreateShadowMatricesUBO(int uboIndex);
    unsigned int GetShadowMapTexuture() { return m_cascadedDepthTexturesArray; }
    std::vector<float> GetCsmSizes(){ return m_csmFrustumSizes; }
    Camera* Camera = NULL;
    glm::vec3 LightDirection = glm::vec3(1);
    float Fov = 0;
    float Near = 0;
    float Far = 0;
    bool drawShadows = true;
    std::vector<float> shadowCascadeLevels;
    size_t ShadowCasterCount = 0;
    std::vector<std::shared_ptr<SceneObject>> ShadowCastRenderers;
    std::vector<std::shared_ptr<SceneObject>> ShadowCastFowardRenderers;
    Shader* ShadowCasterShader = nullptr;
    
private:
    std::vector<glm::mat4> GetCascadeShadowMatrices();
    glm::mat4 GetShadowMatrices(int i ,float nearPlane, float farPlane);
    void UpdateShadowMatricesUBO();

    
    const int m_numCascades = 5;
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    std::vector<float> m_csmFrustumSizes;
    const unsigned SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
    unsigned int m_cascadedShadowFBO[5];
    unsigned int m_cascadedDepthTexturesArray = 0;
    unsigned int m_shadowMatricesUBO = 0;

};