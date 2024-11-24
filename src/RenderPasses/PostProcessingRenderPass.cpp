#include "PostProcessingRenderPass.h"

#include <iostream>

void PostProcessingRenderPass::Create(int windowWidth, int windowHeight)
{
    glGenFramebuffers(1, &m_toneMappingFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_toneMappingFBO);
    
    glGenTextures(1, &m_toneMappingColorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_toneMappingColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_toneMappingColorBuffer, 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &m_aaFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_aaFBO);
    
    glGenTextures(1, &m_aaColorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_aaColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_aaColorBuffer, 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &m_fullScreenColorFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fullScreenColorFBO);
    
    glGenTextures(1, &m_fullScreenColorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_fullScreenColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fullScreenColorBuffer, 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    m_baseMapHash = std::hash<std::string>{}("_baseMap");
    m_bloomBlurMapHash =  std::hash<std::string>{}("_bloomBlurMap");
    m_ssaoMapHash =  std::hash<std::string>{}("_ssaoMap");
}

void PostProcessingRenderPass::Render(float dt)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_toneMappingFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DepthState(false, GL_LESS, GL_TRUE);
        
    ToneMappingCompositeShader->use();
    ToneMappingCompositeShader->SetTextureUniform(m_baseMapHash, "_baseMap",  BaseMap);
    ToneMappingCompositeShader->SetTextureUniform(m_ssaoMapHash, "_ssaoMap", SsaoMap);
    ToneMappingCompositeShader->SetTextureUniform(m_bloomBlurMapHash, "_bloomBlurMap", BloomTexture);
    ToneMappingCompositeShader->SetInt1("_useBloom", enableBloom);
    ToneMappingCompositeShader->SetVec1("_exposure", exposure);
    ToneMappingCompositeShader->SetVec1("_enableToneMapping", enableToneMapping ? 1.0f : 0.0f);
    RenderQuad4();

    glBindFramebuffer(GL_FRAMEBUFFER, m_aaFBO);
    FxaaShader->use();
    FxaaShader->SetTextureUniform(m_baseMapHash, "_baseMap",  m_toneMappingColorBuffer);
    FxaaShader->SetVec1("_testLogic", TestLogic);
    RenderQuad4();
    
    glBindFramebuffer(GL_FRAMEBUFFER, m_fullScreenColorFBO);
    BlitShader->use();
    BlitShader->SetTextureUniform(m_baseMapHash, "_baseMap",  m_aaColorBuffer);
    RenderQuad4();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    BlitShader->use();
    BlitShader->SetTextureUniform(m_baseMapHash, "_baseMap",  m_aaColorBuffer);
    RenderQuad4();

    
    DepthState(true, GL_LESS, GL_TRUE);
}

void PostProcessingRenderPass::Dispose()
{
    
}

void PostProcessingRenderPass::RenderQuad4()
{
    glBindVertexArray(FullScreenQuad4VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}



