#include "IndirectLightingRenderPass.h"

#include <iostream>

void IndirectLightingRenderPass::Create(int windowWidth, int windowHeight)
{
    glGenFramebuffers(1, &IndirectLightingFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, IndirectLightingFBO);
    
    glGenTextures(1, &m_indirectLightingColorBuffer);
    glBindTexture(GL_TEXTURE_2D, m_indirectLightingColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_indirectLightingColorBuffer, 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    m_baseMapHash = std::hash<std::string>{}("_baseMap");
    m_depthMapHash = std::hash<std::string>{}("_depthMap");
    m_rsmDepthMapHash = std::hash<std::string>{}("_rsmDepth");
    m_rsmPositionMapHash = std::hash<std::string>{}("_rsmPosition");
    m_rsmNormalMapHash = std::hash<std::string>{}("_rsmNormal");
    m_rsmFluxMapHash = std::hash<std::string>{}("_rsmFlux");
}

void IndirectLightingRenderPass::Render(float dt)
{
    glBindFramebuffer(GL_FRAMEBUFFER, IndirectLightingFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    DepthState(false, GL_LESS, GL_TRUE);
        
    IndirectLightingShader->use();
    IndirectLightingShader->SetMat4("_projection", ProjectionMatrix);
    IndirectLightingShader->SetMat4("_cameraViewMatrix", ViewMatrix);
    IndirectLightingShader->SetVec1("_rsmMax",RsmMaxRadius);
    IndirectLightingShader->SetVec1("_rsmFluxStrength",RsmFluxStrength);
    
    IndirectLightingShader->SetTextureUniform(m_depthMapHash, "_depthMap", DepthMap);
    IndirectLightingShader->SetTextureUniform(m_rsmDepthMapHash, "_rsmDepth", RsmDepth);
    IndirectLightingShader->SetTextureUniform(m_rsmPositionMapHash,"_rsmPosition", RsmPosition);
    IndirectLightingShader->SetTextureUniform(m_rsmNormalMapHash,"_rsmNormal", RsmNormal);
    IndirectLightingShader->SetTextureUniform(m_rsmFluxMapHash,"_rsmFlux", RsmFlux);
    RenderQuad4();

    /*glBindFramebuffer(GL_FRAMEBUFFER, CurrentFullScreenColorFBO);
    BlitShader->use();
    BlitShader->SetInt1("_performGammaCorrection", 0 );
    BlitShader->SetTextureUniform(m_baseMapHash, "_baseMap",  m_indirectLightingColorBuffer);
    RenderQuad4();*/
    
    DepthState(true, GL_LESS, GL_TRUE);
}

void IndirectLightingRenderPass::Dispose()
{
    glDeleteBuffers(1, &IndirectLightingFBO);
}

void IndirectLightingRenderPass::RenderQuad4()
{
    glBindVertexArray(FullScreenQuad4VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}



