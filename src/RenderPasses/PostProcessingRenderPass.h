#pragma once

#include "../RenderPass.h"
#include "../Classes/Shader.h"
#include "../Classes/Renderer.h"

class PostProcessingRenderPass : RenderPass
{
public:
    void Create(int windowWidth, int windowHeight) override;
    void Render(float dt) override;
    void Dispose() override;

    unsigned int GetCameraColorImage(){ return m_fullScreenColorBuffer; }
    
    Shader* ToneMappingCompositeShader;
    Shader* FxaaShader;
    Shader* BlitShader;
    bool TestLogic;
    unsigned int FullScreenQuad4VAO;
    unsigned int BloomTexture;
    unsigned int BaseMap;
    unsigned int SsaoMap;

    int enableBloom;
    float exposure;
    int enableToneMapping;
private:
    unsigned int m_toneMappingFBO;
    unsigned int m_aaFBO;
    unsigned int m_fullScreenColorFBO;
    unsigned int m_toneMappingColorBuffer;
    unsigned int m_aaColorBuffer;
    unsigned int m_fullScreenColorBuffer;
    size_t m_baseMapHash;
    size_t m_bloomBlurMapHash;
    size_t m_ssaoMapHash;
    void RenderQuad4();
};