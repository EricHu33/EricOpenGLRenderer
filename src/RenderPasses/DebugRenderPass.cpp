#include "DebugRenderPass.h"

#include <iostream>

#include "../DebugConfig.h"
void DebugRenderPass::Create(int windowWidth, int windowHeight)
{
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    glGenTextures(1, &m_debugTexture);
    glBindTexture(GL_TEXTURE_2D, m_debugTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_debugTexture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void DebugRenderPass::Render(float dt)
{
    DepthState(false, GL_LESS, GL_FALSE);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glClear(GL_COLOR_BUFFER_BIT);
    unsigned int previewShadowMap = -1;
    previewShadowMap = visualizeShadowMap ? shadowMap: -1;
    if(previewShadowMap != -1)
    {
        DrawTextureArrayOnQuad(previewShadowMap, shadowMapLayer);
    }

    unsigned int previewRSMMap = -1;
    previewRSMMap = visualizeRSMDepth ? rsmDepthMap: -1;
    previewRSMMap = visualizeRSMPosition ? rsmPositionMap: previewRSMMap;
    previewRSMMap = visualizeRSMNormal ? rsmNormalMap: previewRSMMap;
    previewRSMMap = visualizeRSMFlux ? rsmFluxMap: previewRSMMap;
    previewRSMMap = visualizeIndirectLighting ? indirectLightingMap: previewRSMMap;
    if(previewRSMMap != -1)
    {
        DrawTextureOnQuad(previewRSMMap);
    }

    unsigned int previewBloomBuffer = -1;
    previewBloomBuffer = visuallizeBloomMask ? bloomMask : -1;
    previewBloomBuffer = visualizeBlurPass ?  bloomResultMap : previewBloomBuffer;
    if(previewBloomBuffer != -1)
    {
       DrawTextureOnQuad(previewBloomBuffer);
    }
    unsigned int previewGbuffer = -1;
    previewGbuffer = visualizeNormal ? gBufferNormal : previewGbuffer;
    previewGbuffer = visualizePosition ? gBufferPosition : previewGbuffer;
    previewGbuffer = visualizeAlbedo ? gBufferAlbedo : previewGbuffer;
    previewGbuffer = visualizeMaterialParams ? gBufferMaterial : previewGbuffer;
    if(previewGbuffer != -1)
    {
        DrawTextureOnQuad(previewGbuffer);
    }

    unsigned int previewSSAO = -1;
    previewSSAO = visualizeSSAO ? ssaoMap : -1;
    if(previewSSAO != -1)
    {
        DrawTextureOnQuad(previewSSAO);
    }
    
    unsigned int previewComputeTex = -1;
    previewComputeTex = visualizeDepthPrepass ? depthPrepassMap : -1;
    previewComputeTex = visualizeComputeShaderTex ? computeResultMap : -1;
    if(previewComputeTex != -1)
    {
        DrawForwardLightDebugTextureOnQuad(previewComputeTex);
    }
    
    Debug::DebugConfig::debugTextureId = m_debugTexture;
    /*DrawTextureOnQuad(brdfLut);
        Debug::DebugConfig::debugTextureId = m_debugTexture;*/
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
}

void DebugRenderPass::Dispose()
{
    
}

