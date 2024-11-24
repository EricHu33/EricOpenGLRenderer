#pragma once

#include "../RenderPass.h"
#include "../Classes/Shader.h"
#include "../Classes/Renderer.h"

class SSAORenderPass : RenderPass
{
public:
    void Create(int windowWidth, int windowHeight) override;
    void Render(float dt) override;
    void Dispose() override;
    unsigned int GetSSAOPreBlurResult() { return m_ssaoTexture; }
    unsigned int GetSSAOBlurredResult() { return m_ssaoBlurredTexture; }
    unsigned int GetHBAOBlurredResult() { return m_ssaoBlurredTexture1; }
    Shader* SSAOShader;
    Shader* SSAOBlurShader;

    Shader* HBAOShader;
    Shader* HBAOBlurShader;
    float blurShaprness;
    float blurRadius;
    unsigned int testBlurSource;
    unsigned int gPosition;
    unsigned int depthMap;
    unsigned int gNormal;
    float projectionFOV;
    float aoStrength;
    float aoRandomSize;
    float aoPower;
    float ssaoRadius;
    float ssaoBias;
    bool EnableTestLogic;
    glm::mat4 ProjectionMatrix;
    glm::mat4 ViewMatrix;
    unsigned int FullScreenQuad4VAO;
    bool useHalfResolution;
    bool useSSAO;
    bool useBilateralBlur = false;
    unsigned int blueNoiseTexture;
private:
    int m_windowHeight;
    int m_windowWidth;
    bool m_isHalfResolution = false;
    unsigned int m_fbo;
    unsigned int m_blurFBO;
    unsigned int m_verticalBlurFBO;
    unsigned int m_bilateralBlurFBO;
    unsigned int m_ssaoTexture;
    unsigned int m_ssaoBlurredTexture;
    unsigned int m_ssaoBlurredTexture1;
    size_t m_depthMapHash;
    size_t m_positionMapHash;
    size_t m_normalMapKey;
    size_t m_noiseKey;
    size_t m_blueNoiseKey;
    size_t m_ssaoKey;
    size_t m_blurSrcKey;
    std::vector<glm::vec3> m_ssaoNoise;
    unsigned int m_noiseTexture;
    void RenderQuad4();

    float Lerp(float a, float b, float f);
};