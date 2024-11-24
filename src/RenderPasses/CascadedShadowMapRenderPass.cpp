#include "CascadedShadowMapRenderPass.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

void CascadedShadowMapRenderPass::CreateShadowMatricesUBO(int uboIndex)
{
    glGenBuffers(1, &m_shadowMatricesUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_shadowMatricesUBO);
    glBufferData(GL_UNIFORM_BUFFER, 5 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, uboIndex, m_shadowMatricesUBO);
    for (size_t i = 0; i < 5; ++i)
    {
        glm::mat4 shadowMatrix(1);
        glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4x4), sizeof(glm::mat4x4), &shadowMatrix);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void CascadedShadowMapRenderPass::UpdateShadowMatricesUBO()
{
    auto shadowMatrices = GetCascadeShadowMatrices();
    glBindBuffer(GL_UNIFORM_BUFFER, m_shadowMatricesUBO);
    for (size_t i = 0; i < shadowMatrices.size(); ++i)
    {
        glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::mat4), sizeof(glm::mat4), &shadowMatrices[i]);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void CascadedShadowMapRenderPass::Create(int windowWidth, int windowHeight)
{
    this->m_windowWidth = windowWidth;
    this->m_windowHeight = windowHeight;
    this->m_csmFrustumSizes = std::vector<float>();
    for(int i = 0; i < this->shadowCascadeLevels.size();i++)
    {
        m_csmFrustumSizes.push_back(0);   
    }
    glGenFramebuffers(m_numCascades, m_cascadedShadowFBO);
    
    glGenTextures(1, &m_cascadedDepthTexturesArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_cascadedDepthTexturesArray);
    glTexImage3D(
        GL_TEXTURE_2D_ARRAY,
        0,
        GL_DEPTH_COMPONENT,
        SHADOW_WIDTH,
        SHADOW_HEIGHT,
        m_numCascades,
        0,
        GL_DEPTH_COMPONENT,
        GL_FLOAT,
        NULL
    );
 
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[] = { 1.0,  1.0,  1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

    for (int i = 0; i < m_numCascades; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_cascadedShadowFBO[i]);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_cascadedDepthTexturesArray, 0, i);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "frame buffer gen error in cascadeDepthTexturesArray[" << i << "]" << std::endl;
        }
        else
        {
            // std::cout << "frame buffer bind  in cascadeDepthTexturesArray[" << i << "]" << std::endl;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CascadedShadowMapRenderPass::Render(float dt)
{
    DepthState(true, GL_LESS, GL_TRUE);
    BlendState(false, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    UpdateShadowMatricesUBO();
    for (int cascadeIndex = 0; cascadeIndex < m_numCascades; ++cascadeIndex)
    {
        // Bind the FBO for the current cascade
        glBindFramebuffer(GL_FRAMEBUFFER, m_cascadedShadowFBO[cascadeIndex]);
        // Set the viewport to the cascade dimensions
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_cascadedDepthTexturesArray, 0, cascadeIndex);
        glClear(GL_DEPTH_BUFFER_BIT);

        ShadowCasterShader->use();
        ShadowCasterShader->SetInt1("_cascadeIndex", cascadeIndex);
        if(drawShadows)
        {
            for (size_t i = 0; i < ShadowCastRenderers.size(); ++i)
            {
                if(ShadowCastRenderers[i]->renderer->IsSkinnedMesh && ShadowCastRenderers[i]->animator)
                {
                    ShadowCastRenderers[i]->animator->UpdateUboMatrices();
                }
                ShadowCasterShader->SetInt1("_isSkinnedMesh", ShadowCastRenderers[i]->renderer->GetMainMaterial()->GetInt("_isSkinnedMesh"));
                ShadowCasterShader->SetMat4("_model",  ShadowCastRenderers[i]->GetModelMatrix());
                ShadowCastRenderers[i]->renderer->DrawWithShader((*ShadowCasterShader));
            }
            for (size_t i = 0; i < ShadowCastFowardRenderers.size(); ++i)
            {
                if(ShadowCastFowardRenderers[i]->renderer->IsSkinnedMesh && ShadowCastFowardRenderers[i]->animator)
                {
                    ShadowCastFowardRenderers[i]->animator->UpdateUboMatrices();
                }
                ShadowCasterShader->SetInt1("_isSkinnedMesh", ShadowCastFowardRenderers[i]->renderer->GetMainMaterial()->GetInt("_isSkinnedMesh"));
                ShadowCasterShader->SetMat4("_model",  ShadowCastFowardRenderers[i]->GetModelMatrix());
                ShadowCastFowardRenderers[i]->renderer->DrawWithShader((*ShadowCasterShader));
            }
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CascadedShadowMapRenderPass::Dispose()
{
    glDeleteBuffers(1, &m_shadowMatricesUBO);
    glDeleteBuffers(5, m_cascadedShadowFBO);
}

std::vector<glm::mat4> CascadedShadowMapRenderPass::GetCascadeShadowMatrices()
{
    std::vector<glm::mat4> shadowMatrices;
    for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
    {
        if (i == 0)
        {
            shadowMatrices.push_back(GetShadowMatrices(i, Near, shadowCascadeLevels[i]));
        }
        else if (i < shadowCascadeLevels.size())
        {
            shadowMatrices.push_back(GetShadowMatrices(i, Near, shadowCascadeLevels[i]));
        }
        else
        {
            shadowMatrices.push_back(GetShadowMatrices(i, Near, Far));
        }
    }
    return shadowMatrices;
}

glm::mat4 CascadedShadowMapRenderPass::GetShadowMatrices(int i , float nearPlane, float farPlane)
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




