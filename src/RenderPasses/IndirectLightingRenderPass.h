#pragma once

#include "../RenderPass.h"
#include "../Classes/Shader.h"
#include "../Classes/Renderer.h"

class IndirectLightingRenderPass : RenderPass
{
public:
    void Create(int windowWidth, int windowHeight) override;
    void Render(float dt) override;
    void Dispose() override;
    unsigned int GetIndirectLightingMap(){ return m_indirectLightingColorBuffer; }
    
    Shader* IndirectLightingShader;
    Shader* BlitShader;
    
    unsigned int CurrentFullScreenColorFBO = 0;
    unsigned int IndirectLightingFBO = 0;
    unsigned int FullScreenQuad4VAO = 0;
    unsigned int CurrentColorBuffer = 0;

    //camera depth
    unsigned int DepthMap;
    //RSM
    unsigned int RsmDepth = 0;
    unsigned int RsmPosition = 0;
    unsigned int RsmNormal = 0;
    unsigned int RsmFlux = 0;

    //uniforms
    glm::mat4 ProjectionMatrix;
    glm::mat4 ViewMatrix;
    float RsmMaxRadius = 0.16f;
    float RsmFluxStrength = 0.5f;
    
private:
    unsigned int m_indirectLightingColorBuffer = 0;
    size_t m_baseMapHash = 0;
    size_t m_depthMapHash = 0;
    size_t m_rsmDepthMapHash = 0;
    size_t m_rsmPositionMapHash = 0;
    size_t m_rsmNormalMapHash = 0;
    size_t m_rsmFluxMapHash = 0;
    void RenderQuad4();
};