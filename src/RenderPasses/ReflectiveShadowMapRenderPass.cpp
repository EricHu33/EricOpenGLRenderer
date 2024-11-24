#include "ReflectiveShadowMapRenderPass.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

void ReflectiveShadowMapRenderPass::CreateShadowMatricesUBO(int uboIndex)
{
    glGenBuffers(1, &m_rsmMatricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_rsmMatricesUBO);
    glBufferData(GL_UNIFORM_BUFFER, 5 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, uboIndex, m_rsmMatricesUBO);
    for (size_t i = 0; i < 5; ++i)
    {
        glm::mat4 shadowMatrix(1);
        glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &shadowMatrix);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void ReflectiveShadowMapRenderPass::UpdateShadowMatricesUBO()
{
    auto shadowMatrices = GetCascadeShadowMatrices();
    glBindBuffer(GL_UNIFORM_BUFFER, m_rsmMatricesUBO);
    for (size_t i = 0; i < shadowMatrices.size(); ++i)
    {
        glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4), sizeof(glm::mat4), &shadowMatrices[i]);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void ReflectiveShadowMapRenderPass::Create(int windowWidth, int windowHeight)
{
    this->m_windowWidth = windowWidth;
    this->m_windowHeight = windowHeight;
    this->m_csmFrustumSizes = std::vector<float>();
    for(int i = 0; i < this->shadowCascadeLevels.size();i++)
    {
        m_csmFrustumSizes.push_back(0);   
    }
    glGenFramebuffers(1, &m_rsmDepthFBO);
    glGenTextures(1, &m_rsmDepth);
    glBindTexture(GL_TEXTURE_2D, m_rsmDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = { 1.0,  1.0,  1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, m_rsmDepthFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_rsmDepth, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "frame buffer gen error in rsm depth map" << std::endl;
    }
    else
    {
        // std::cout << "frame buffer bind  in cascadeDepthTexturesArray[" << i << "]" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &m_rsmFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_rsmFBO);   
    // position color buffer
    glGenTextures(1, &m_rsmPosition);
    glBindTexture(GL_TEXTURE_2D, m_rsmPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SHADOW_WIDTH, SHADOW_WIDTH, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_rsmPosition, 0); 
    // normal color buffer
    glGenTextures(1, &m_rsmNormal);
    glBindTexture(GL_TEXTURE_2D, m_rsmNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SHADOW_WIDTH, SHADOW_WIDTH, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_rsmNormal, 0);
    // flux color buffer
    glGenTextures(1, &m_rsmFlux);
    glBindTexture(GL_TEXTURE_2D, m_rsmFlux);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SHADOW_WIDTH, SHADOW_WIDTH, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_rsmFlux, 0);
    
    glDrawBuffers(3, m_rsmAttachments);
    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SHADOW_WIDTH, SHADOW_WIDTH); 
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
}

void ReflectiveShadowMapRenderPass::Render(float dt)
{
    DepthState(true, GL_LESS, GL_TRUE);
    BlendState(false, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    UpdateShadowMatricesUBO();
    // Bind the FBO for the current cascade
    glBindFramebuffer(GL_FRAMEBUFFER, m_rsmDepthFBO);
    // Set the viewport to the cascade dimensions
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_rsmDepth, 0);
    glClear(GL_DEPTH_BUFFER_BIT);

    RSMDepthShader->use();
    //ShadowCasterShader->SetInt1("_cascadeIndex", 0);
    for (size_t i = 0; i < ShadowCastRenderers.size(); ++i)
    {
        if(ShadowCastRenderers[i]->renderer->IsSkinnedMesh && ShadowCastRenderers[i]->animator)
        {
            ShadowCastRenderers[i]->animator->UpdateUboMatrices();
        }
        RSMDepthShader->SetInt1("_isSkinnedMesh", ShadowCastRenderers[i]->renderer->GetMainMaterial()->GetInt("_isSkinnedMesh"));
        RSMDepthShader->SetMat4("_model",  ShadowCastRenderers[i]->GetModelMatrix());
        ShadowCastRenderers[i]->renderer->DrawWithShader((*RSMDepthShader));
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //RSM Color
    DepthState(true, GL_EQUAL, GL_FALSE);
    BlendState(false, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_rsmDepthFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_rsmFBO);
    glBlitFramebuffer(
      0, 0, SHADOW_WIDTH, SHADOW_WIDTH, 0, 0, SHADOW_WIDTH, SHADOW_WIDTH, GL_DEPTH_BUFFER_BIT, GL_NEAREST
    );
    glBindFramebuffer(GL_FRAMEBUFFER, m_rsmFBO);
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glClearColor(0,0,1,1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    
    glDrawBuffer(GL_COLOR_ATTACHMENT1);
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT );
    
    glDrawBuffer(GL_COLOR_ATTACHMENT2);
    glClearColor(0,1,0,1);
    glClear(GL_COLOR_BUFFER_BIT );
    
    glDisable(GL_BLEND);
    glDrawBuffers(3, m_rsmAttachments);
    RSMColorShader->use();
    //ShadowCasterShader->SetInt1("_cascadeIndex", 0);
    for (size_t i = 0; i < ShadowCastRenderers.size(); ++i)
    {
        if(ShadowCastRenderers[i]->renderer->IsSkinnedMesh && ShadowCastRenderers[i]->animator)
        {
            ShadowCastRenderers[i]->animator->UpdateUboMatrices();
        }
        TextureVar baseMapVar = ShadowCastRenderers[i]->renderer->GetMainMaterial()->TryGetTexture("_baseMap");
        if( baseMapVar.ID != -1 )
        {
            RSMColorShader->SetTextureUniform(baseMapVar.HashedTextureName, "_baseMap", baseMapVar.ID);
        }
        RSMColorShader->SetInt1("_isSkinnedMesh", ShadowCastRenderers[i]->renderer->GetMainMaterial()->GetInt("_isSkinnedMesh"));
        RSMColorShader->SetMat4("_model",  ShadowCastRenderers[i]->GetModelMatrix());
        ShadowCastRenderers[i]->renderer->DrawWithShader((*RSMColorShader));
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    DepthState(true, GL_LESS, GL_TRUE);
}

void ReflectiveShadowMapRenderPass::Dispose()
{
    glDeleteBuffers(1, &m_rsmMatricesUBO);
    glDeleteBuffers(1, &m_rsmFBO);
    glDeleteBuffers(1, &m_rsmDepthFBO);
}

std::vector<glm::mat4> ReflectiveShadowMapRenderPass::GetCascadeShadowMatrices()
{
    std::vector<glm::mat4> shadowMatrices;
    shadowMatrices.push_back(GetShadowMatrices(0, Near, Far));
    return shadowMatrices;
}

glm::mat4 ReflectiveShadowMapRenderPass::GetShadowMatrices(int i , float nearPlane, float farPlane)
{
    float aspect = (float)m_windowWidth / (float)m_windowHeight;
    glm::mat4 projection  = glm::perspective(glm::radians(Fov), aspect, nearPlane, farPlane);
    std::vector<glm::vec4> frustumBoundsWS =  Camera->getFrustumCornersWorldSpace(projection, Camera->GetViewMatrix());
    
    glm::vec3 centerWS = glm::vec3(0, 0, 0);

    for (const auto& v : frustumBoundsWS)
    {
        centerWS += glm::vec3(v);
    }
    centerWS /= frustumBoundsWS.size();
    
    float radius = 0.0f;
    float dMin = 10000, dMax = -10000;
    for (const auto& v : frustumBoundsWS)
    {
        float distance = glm::length(glm::vec3(v) - centerWS);
        dMin = std::min(dMin, distance);
        dMax = std::max(dMax, distance);
    }
    radius = dMax - dMin;
    radius *= 1.0f;
    radius = ceil(radius * 16.0f) / 16.0f;
    glm::vec3 radius3(radius, radius, radius);
    glm::vec3 max = radius3;
    glm::vec3 min = -radius3;
    if(i < 4)
    {
        m_csmFrustumSizes[i] = 2 * radius;
    }
    
    float m_near_offset = 20.0f;
    glm::vec3 cascade_extents = max - min;
    // Push the light position back along the light direction by the near offset.
    glm::vec3 shadow_camera_pos = centerWS + normalize(LightDirection) * m_near_offset ;
    glm::mat4 orthProjectMatrix = glm::ortho(min.x, max.x, min.y, max.y, -m_near_offset, m_near_offset + cascade_extents.z);
    glm::mat4 viewMatrix = glm::lookAt(shadow_camera_pos, centerWS, glm::vec3(0,1,0));
    glm::mat4 preOffsetShadowMatrix = orthProjectMatrix * viewMatrix;

    glm::vec4 shadow_origin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    shadow_origin = preOffsetShadowMatrix * shadow_origin;
    shadow_origin = shadow_origin * ((float)SHADOW_WIDTH / 2.0f);

    glm::vec4 rounded_origin = glm::round(shadow_origin);
    glm::vec4 round_offset = rounded_origin - shadow_origin;
    round_offset = round_offset * (2.0f / (float)SHADOW_WIDTH);
    round_offset.z = 0.0f;
    round_offset.w = 0.0f;

    glm::mat4 shadow_proj = orthProjectMatrix;
    shadow_proj[3][0] += round_offset.x;
    shadow_proj[3][1] += round_offset.y;
    shadow_proj[3][2] += round_offset.z;
    shadow_proj[3][3] += round_offset.w;
    
    //glm::mat4 lightView = glm::lookAt(  centerWS + glm::normalize(LightDirection), centerWS, glm::vec3(0.0f, 1.0f, 0.0f));
    
    glm::mat4 lightSpaceMatrix = shadow_proj * viewMatrix;
    
    return lightSpaceMatrix;
}




