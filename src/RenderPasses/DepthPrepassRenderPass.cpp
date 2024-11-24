#include "DepthPrepassRenderPass.h"

void DepthPrepassRenderPass::Create(int windowWidth, int windowHeight)
{
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;
    glGenFramebuffers(1, &m_depthPrepassFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_depthPrepassFBO);

    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, m_windowWidth, m_windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DepthPrepassRenderPass::Render(float dt)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_depthPrepassFBO);
    glViewport(0, 0, m_windowWidth, m_windowHeight);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    DepthState(true, GL_LESS, GL_TRUE);
    BlendState(false, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    DepthPrepassShader->use();
    for (size_t i = 0; i < DepthRenderers.size(); ++i)
    {
        if(DepthRenderers[i]->renderer->IsSkinnedMesh && DepthRenderers[i]->animator)
        {
            DepthRenderers[i]->animator->UpdateUboMatrices();
        }
        DepthPrepassShader->SetInt1("_isSkinnedMesh", DepthRenderers[i]->renderer->GetMainMaterial()->GetInt("_isSkinnedMesh"));
        DepthPrepassShader->SetMat4("_model",  DepthRenderers[i]->GetModelMatrix());
        DepthRenderers[i]->renderer->DrawWithShader((*DepthPrepassShader));
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DepthPrepassRenderPass::Dispose()
{
    
}


