#include "SSAORenderPass.h"

#include <iostream>
#include <random>

#include "imgui_impl_opengl3_loader.h"

float SSAORenderPass::Lerp(float a, float b, float f)
{
    return a + f * (b - a);
}

void SSAORenderPass::RenderQuad4()
{
    glBindVertexArray(FullScreenQuad4VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void SSAORenderPass::Create(int windowWidth, int windowHeight)
{
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_ssaoTexture);
    glBindTexture(GL_TEXTURE_2D, m_ssaoTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssaoTexture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    glGenFramebuffers(1, &m_blurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_blurFBO);

    glGenTextures(1, &m_ssaoBlurredTexture);
    glBindTexture(GL_TEXTURE_2D, m_ssaoBlurredTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssaoBlurredTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &m_verticalBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_verticalBlurFBO);

    glGenTextures(1, &m_ssaoBlurredTexture1);
    glBindTexture(GL_TEXTURE_2D, m_ssaoBlurredTexture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssaoBlurredTexture1, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Vertical Blur Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;
    for(int i = 0; i < 64; i++)
    {
        glm::vec3 sample(randomFloats(generator) * 2 - 1, randomFloats(generator) * 2 - 1,  randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);

        float scale = (float)i / 64.0; 
        scale  = Lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);  
    }
    
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(
            randomFloats(generator) * 2.0 - 1.0, 
            randomFloats(generator) * 2.0 - 1.0, 
            0.0f); 
        m_ssaoNoise.push_back(noise);
    }
    
    SSAOShader->use();
    for (unsigned int i = 0; i < 64; ++i)
        SSAOShader->SetVec3("_samples[" + std::to_string(i) + "]", ssaoKernel[i].r, ssaoKernel[i].g, ssaoKernel[i].b);
    
    glGenTextures(1, &m_noiseTexture);
    glBindTexture(GL_TEXTURE_2D, m_noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &m_ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    HBAOShader->use();
    for (unsigned int i = 0; i < 64; ++i)
        HBAOShader->SetVec3("_samples[" + std::to_string(i) + "]", ssaoKernel[i].r, ssaoKernel[i].g, ssaoKernel[i].b);

    m_depthMapHash = std::hash<std::string>{}("_depthMap");
    m_positionMapHash = std::hash<std::string>{}("_gPosition");
    m_normalMapKey = std::hash<std::string>{}("_gNormal");
    m_noiseKey = std::hash<std::string>{}("_noiseTex");
    m_blueNoiseKey = std::hash<std::string>{}("_blueNoiseTex");
    m_ssaoKey = std::hash<std::string>{}("_SSAOTex");
    m_blurSrcKey = std::hash<std::string>{}("_src");
}

void SSAORenderPass::Render(float dt)
{
    if(m_isHalfResolution != useHalfResolution)
    {
        m_isHalfResolution = useHalfResolution;
        auto resWidth = m_isHalfResolution ? (int)(m_windowWidth * 0.5f) : m_windowWidth;
        auto resHeight = m_isHalfResolution ? (int)(m_windowHeight * 0.5f) : m_windowHeight;
        
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
        glGenTextures(1, &m_ssaoTexture);
        glBindTexture(GL_TEXTURE_2D, m_ssaoTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resWidth, resHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssaoTexture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, m_blurFBO);

        glGenTextures(1, &m_ssaoBlurredTexture);
        glBindTexture(GL_TEXTURE_2D, m_ssaoBlurredTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resWidth, resHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssaoBlurredTexture, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        glBindFramebuffer(GL_FRAMEBUFFER, m_verticalBlurFBO);

        glGenTextures(1, &m_ssaoBlurredTexture1);
        glBindTexture(GL_TEXTURE_2D, m_ssaoBlurredTexture1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resWidth, resHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssaoBlurredTexture1, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Vertical Blur Framebuffer not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    auto resWidth = m_isHalfResolution ? (int)(m_windowWidth * 0.5f) : m_windowWidth;
    auto resHeight = m_isHalfResolution ? (int)(m_windowHeight * 0.5f) : m_windowHeight;
    glm::vec2 res = glm::vec2(resWidth, resHeight);
    
    glViewport(0, 0, resWidth, resHeight);
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    float tanHalfFovY = tan(0.5f * glm::radians(projectionFOV));
    float projectionScale = float(m_windowHeight) / (tanHalfFovY  * 2.0f);
    if(useSSAO)
    {
        SSAOShader->use();
        SSAOShader->SetTextureUniform(m_depthMapHash, "_depthMap", depthMap);
        SSAOShader->SetTextureUniform(m_positionMapHash, "_gPosition", gPosition);
        SSAOShader->SetTextureUniform(m_normalMapKey,"_gNormal", gNormal);
        SSAOShader->SetTextureUniform(m_noiseKey,"_noiseTex", this->m_noiseTexture);
        SSAOShader->SetVec1("_radius", ssaoRadius);
        SSAOShader->SetVec1("_bias", ssaoBias);
        SSAOShader->SetVec1("_aoStrength", aoStrength);
        SSAOShader->SetInt1("_testLogic", EnableTestLogic);
        SSAOShader->SetMat4("_projection", ProjectionMatrix);
        SSAOShader->SetMat4("_cameraViewMatrix", ViewMatrix);
        SSAOShader->SetVec2("_aoRes", res);
        SSAOShader->SetVec2("_invAORes", glm::vec2(1.0f / res.x, 1.0f / res.y));
    }
    else
    {
        HBAOShader->use();
        HBAOShader->SetTextureUniform(m_depthMapHash, "_depthMap", depthMap);
        HBAOShader->SetTextureUniform(m_positionMapHash, "_gPosition", gPosition);
        HBAOShader->SetTextureUniform(m_normalMapKey,"_gNormal", gNormal);
        HBAOShader->SetTextureUniform(m_noiseKey,"_noiseTex", this->m_noiseTexture);
        HBAOShader->SetTextureUniform(m_blueNoiseKey, "_blueNoiseTex", blueNoiseTexture);
        HBAOShader->SetVec1("_RadiusPixels", projectionScale * ssaoRadius * 0.5f);
        HBAOShader->SetVec1("_radius", ssaoRadius);
        HBAOShader->SetVec1("_bias", ssaoBias);
        HBAOShader->SetVec1("_aoStrength", aoStrength);
        HBAOShader->SetVec1("_aoRandomSize", aoRandomSize);
        HBAOShader->SetVec1("_aoPower", aoPower);
        HBAOShader->SetInt1("_testLogic", EnableTestLogic);
        HBAOShader->SetMat4("_projection", ProjectionMatrix);
        HBAOShader->SetMat4("_cameraViewMatrix", ViewMatrix);
        HBAOShader->SetVec2("_aoRes", res);
        HBAOShader->SetVec2("_invAORes", glm::vec2(1.0f / res.x, 1.0f / res.y));
    }
    RenderQuad4();  
    
    if(useBilateralBlur)
    {
        //horizontal blur
        glBindFramebuffer(GL_FRAMEBUFFER, m_blurFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        HBAOBlurShader->use();
        HBAOBlurShader->SetVec1("g_Sharpness", blurShaprness);
        HBAOBlurShader->SetVec1("_blurRadius", blurRadius);
        HBAOBlurShader->SetTextureUniform(m_blurSrcKey, "_src", m_ssaoTexture);
        HBAOBlurShader->SetTextureUniform(m_depthMapHash, "_depthMap", depthMap);
        HBAOBlurShader->SetVec2("_invResolutionDirection", glm::vec2(1.0f / resWidth, 0));
        HBAOBlurShader->SetVec2("_res", res);
        HBAOBlurShader->SetMat4("_projection", ProjectionMatrix);
        RenderQuad4();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        //vertical blur
        glBindFramebuffer(GL_FRAMEBUFFER, m_verticalBlurFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        HBAOBlurShader->use();
        HBAOBlurShader->SetTextureUniform(m_blurSrcKey, "_src", m_ssaoBlurredTexture);
        HBAOBlurShader->SetTextureUniform(m_depthMapHash, "_depthMap", depthMap);
        HBAOBlurShader->SetVec2("_invResolutionDirection", glm::vec2(0, 1.0f / resHeight));
        HBAOBlurShader->SetVec2("_res", res);
        HBAOBlurShader->SetMat4("_projection", ProjectionMatrix);
        RenderQuad4();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_blurFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        SSAOBlurShader->use();
        SSAOBlurShader->SetTextureUniform(m_ssaoKey, "_SSAOTex", m_ssaoTexture);
        SSAOBlurShader->SetVec2("_res",m_isHalfResolution ? 0.5f * res : res);
        RenderQuad4();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    glViewport(0, 0, m_windowWidth, m_windowHeight);
}

void SSAORenderPass::Dispose()
{
    
}


