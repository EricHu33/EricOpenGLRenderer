#include "GeometryRenderPass.h"

#include <iostream>

void GeometryRenderPass::Create(int windowWidth, int windowHeight)
{
    this->m_windowWidth = windowWidth;
    this->m_windowHeight = windowHeight;
    
    glGenFramebuffers(1, &m_gbufferFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_gbufferFBO);
    // position color buffer
    glGenTextures(1, &m_gPosition);
    glBindTexture(GL_TEXTURE_2D, m_gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gPosition, 0);
    // normal color buffer
    glGenTextures(1, &m_gNormal);
    glBindTexture(GL_TEXTURE_2D, m_gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_gNormal, 0);
    // color + specular color buffer
    glGenTextures(1, &m_gAlbedo);
    glBindTexture(GL_TEXTURE_2D, m_gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_gAlbedo, 0);
    //PBR material params
    glGenTextures(1, &m_gMaterialParam);
    glBindTexture(GL_TEXTURE_2D, m_gMaterialParam);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_gMaterialParam, 0);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    
    glDrawBuffers(4, m_gBufferAttachments);
    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GeometryRenderPass::Render(float dt)
{
    DepthState(true, GL_EQUAL, GL_FALSE);
    BlendState(false, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
  //  glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, m_windowWidth, m_windowHeight);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, zPrepass);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_gbufferFBO);
    glBlitFramebuffer(
      0, 0, m_windowWidth, m_windowHeight, 0, 0, m_windowWidth, m_windowHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST
    );
    glBindFramebuffer(GL_FRAMEBUFFER, m_gbufferFBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT );
    glDrawBuffer(GL_COLOR_ATTACHMENT1);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT );
    glDrawBuffer(GL_COLOR_ATTACHMENT2);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT );
    glDrawBuffer(GL_COLOR_ATTACHMENT3);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT );
    glDisable(GL_BLEND);
    glDrawBuffers(4, m_gBufferAttachments);

    DrawRenderers();
}

void GeometryRenderPass::DrawRenderers()
{
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    for (size_t i = 0; i < SceneObjects.size(); ++i)
    {
        auto renderer = SceneObjects[i]->renderer;
        if(!renderer->IsBackFaceCull)
        {
            CullState(false, GL_BACK);
        }
        auto mats = renderer->GetMainMaterial();
        mats->GetShader()->use();
        mats->MarkTextureDirty();
        mats->ForceUpdate();
        mats->SetVec3("_cameraPos", CameraPosition);
        mats->SetMat4("_model", SceneObjects[i]->GetModelMatrix());
        mats->SetMat4("_view", ViewMatrix);
        mats->SetMat4("_projection", ProjMatrix);
        if(SceneObjects[i]->renderer->IsSkinnedMesh && SceneObjects[i]->animator)
        {
            SceneObjects[i]->animator->UpdateUboMatrices();
        }
        renderer->Draw();
        if(!renderer->IsBackFaceCull)
        {
            CullState(true, GL_BACK);
        }
    }
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


void GeometryRenderPass::Dispose()
{
    glDeleteBuffers(1, &m_gbufferFBO);
}