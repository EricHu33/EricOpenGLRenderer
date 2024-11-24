#include "ForwardRenderPass.h"

#include <iostream>
#include <stdbool.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../DebugConfig.h"

void ForwardRenderPass::Create(int windowWidth, int windowHeight)
{
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;
    if(!IsDeferred)
    {
        glGenFramebuffers(1, &m_forwardFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, m_forwardFBO);
        
        glGenTextures(1, &m_forwardBuffer);
        glBindTexture(GL_TEXTURE_2D, m_forwardBuffer);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_forwardBuffer, 0);
    }

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
    
    std::vector<float> skyboxVertices = GetSkyboxVertices();
    glGenVertexArrays(1, &m_skyboxVAO);
    glGenBuffers(1, &m_skyboxVBO);
    glBindVertexArray(m_skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size() * sizeof(float), &skyboxVertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void ForwardRenderPass::Render(float dt)
{
    if(zPrepass <= 0)
    {
        DepthState(true, GL_EQUAL, GL_TRUE);
        BlendState(false, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        DepthState(true, GL_EQUAL, GL_FALSE);
        BlendState(false, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    
    if(IsDeferred)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, GeometryPassFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, CurrentFullScreenFBO);
        glBlitFramebuffer(
          0, 0, m_windowWidth, m_windowHeight, 0, 0, m_windowWidth, m_windowHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST
        );
        glBindFramebuffer(GL_FRAMEBUFFER, CurrentFullScreenFBO);   
    }
    else
    {
        glViewport(0, 0, m_windowWidth, m_windowHeight);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, zPrepass);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_forwardFBO);
        glBlitFramebuffer(
          0, 0, m_windowWidth, m_windowHeight, 0, 0, m_windowWidth, m_windowHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST
        );
        glBindFramebuffer(GL_FRAMEBUFFER, m_forwardFBO);
    }
    //render meshes
    if(DrawDebugLightSourceCubes)
    {
        for (unsigned int i = 0; i < ForwardPassLightDatas.size(); i++)
        {
            DebugLightSourceRenderer->GetMainMaterial()->GetShader()->use();
            glm::mat4 obj2World = glm::mat4(1);
            obj2World = glm::translate(obj2World, glm::vec3(ForwardPassLightDatas[i].Position));
            obj2World = glm::scale(obj2World, glm::vec3(0.05f));
            DebugLightSourceRenderer->GetMainMaterial()->SetMat4("_model", obj2World);
            DebugLightSourceRenderer->GetMainMaterial()->SetVec3("_lightColor",ForwardPassLightDatas[i].Color);
            DebugLightSourceRenderer->Draw();
        }   
    }
    DrawRenderers(ForwardPassMeshOpqaueRenderer);
    //DrawInstancingRenderers(ForwardPassInstancingRenderer);
    //render skybox
    glDepthFunc(GL_LEQUAL);
    SkyBoxShader->use();
    SkyBoxShader->SetMat4("_view", glm::mat4(glm::mat3(ViewMatrix)));
    SkyBoxShader->SetMat4("_projection", ProjectionMatrix);
    glBindVertexArray(m_skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, SkyboxTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    glDepthFunc(GL_LESS);
    BlendState(true, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    DepthState(true, GL_LESS, GL_FALSE);
    DrawRenderers(ForwardPassMeshTransparentRenderer);
    
    DepthState(true, GL_LESS, GL_TRUE);
    BlendState(false, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ForwardRenderPass::Dispose()
{
    glDeleteVertexArrays(1, &m_skyboxVAO);
}

void ForwardRenderPass::DrawRenderers(std::vector<std::shared_ptr<SceneObject>> sceneObjects)
{
    Shader* lastUsedShader = nullptr;
    for (unsigned int i = 0; i < sceneObjects.size(); i++)
    {
        if(!sceneObjects[i]->renderer->IsBackFaceCull)
        {
            CullState(false, GL_BACK);
        }
        
        if(sceneObjects[i]->renderer->IsSkinnedMesh && sceneObjects[i]->animator)
        {
            sceneObjects[i]->animator->UpdateUboMatrices();
        }
        sceneObjects[i]->renderer->GetMainMaterial()->GetShader()->use();
        if(lastUsedShader == nullptr || lastUsedShader != sceneObjects[i]->renderer->GetMainMaterial()->GetShader())
        {
            lastUsedShader = sceneObjects[i]->renderer->GetMainMaterial()->GetShader();
            sceneObjects[i]->renderer->GetMainMaterial()->MarkTextureDirty();
            lastUsedShader->SetInt1("_cascadeCount", shadowCascadeLevels.size());
            lastUsedShader->SetVec1("_exposure", Exposure);
            lastUsedShader->SetVec1("_radius", Debug::DebugConfig::pointLightRadiusFalloff);
            lastUsedShader->SetVec1("_PreComputedEnvIntensity", PreComputedEnvIntensity);
            lastUsedShader->SetVec1("_PreComputedLightIntensity", PreComputedLightIntensity);
            lastUsedShader->SetVec3("_cameraPos", CameraPosition);
            lastUsedShader->SetVec3("_mainLightDirection", DirectionalLight.Direction);
            lastUsedShader->SetVec3("_mainLightColor", DirectionalLight.Color);
            lastUsedShader->SetVec1("_pcssSearchRadius", pcssSearchRadius);
            lastUsedShader->SetVec1("_pcssFilterRadius", pcssFilterRadius);
            lastUsedShader->SetVec1("_normalBias", normalBias);
            lastUsedShader->SetVec1("_depthBias", depthBias);
            lastUsedShader->SetInt1("_isInstancing",0);
            lastUsedShader->SetVec1("_rsmMax",RsmMaxRadius);
            lastUsedShader->SetVec1("_rsmFluxStrength",RsmFluxStrength);
        }
        sceneObjects[i]->renderer->GetMainMaterial()->ForceUpdate();
        sceneObjects[i]->renderer->GetMainMaterial()->SetTextureArray("_shadowMap", cascadeShadowMapArray, true);
        sceneObjects[i]->renderer->GetMainMaterial()->SetTexture("_SSAOTex", ssao, NULL, true);
        //Indirect Ligthing
        sceneObjects[i]->renderer->GetMainMaterial()->SetTexture("_indirectLightingMap", indirectLightingMap, NULL, true);
     //   sceneObjects[i]->renderer->GetMainMaterial()->SetCubeMapTexture("_irradianceMap", diffuseIrradiance, true);
        sceneObjects[i]->renderer->GetMainMaterial()->SetCubeMapTexture("_prefilterMap", prefilterMap, true);
        sceneObjects[i]->renderer->GetMainMaterial()->SetTexture("_brdfLUT", brdfLUT, NULL, true);
        sceneObjects[i]->renderer->GetMainMaterial()->SetMat4("_model",  sceneObjects[i]->GetModelMatrix());

        sceneObjects[i]->renderer->Draw();
        if(!sceneObjects[i]->renderer->IsBackFaceCull)
        {
            CullState(true, GL_BACK);
        }
    }
}

void ForwardRenderPass::DrawInstancingRenderers(std::vector<std::shared_ptr<SceneObject>> sceneObjects)
{
    Shader* lastUsedShader = nullptr;
    for (unsigned int i = 0; i < sceneObjects.size(); i++)
    {
        if(!sceneObjects[i]->renderer->IsBackFaceCull)
        {
            CullState(false, GL_BACK);
        }
        
        if(sceneObjects[i]->renderer->IsSkinnedMesh && sceneObjects[i]->animator)
        {
            sceneObjects[i]->animator->UpdateUboMatrices();
        }
        sceneObjects[i]->renderer->GetMainMaterial()->GetShader()->use();
        if(lastUsedShader == nullptr || lastUsedShader != sceneObjects[i]->renderer->GetMainMaterial()->GetShader())
        {
            lastUsedShader = sceneObjects[i]->renderer->GetMainMaterial()->GetShader();
        }
            sceneObjects[i]->renderer->GetMainMaterial()->MarkTextureDirty();
        sceneObjects[i]->renderer->GetMainMaterial()->SetTextureArray("_shadowMap", cascadeShadowMapArray);
        sceneObjects[i]->renderer->GetMainMaterial()->SetTexture("_SSAOTex", ssao, NULL, true);
        sceneObjects[i]->renderer->GetMainMaterial()->SetCubeMapTexture("_irradianceMap", diffuseIrradiance);
        sceneObjects[i]->renderer->GetMainMaterial()->SetCubeMapTexture("_prefilterMap", prefilterMap);
        sceneObjects[i]->renderer->GetMainMaterial()->SetTexture("_brdfLUT", brdfLUT, NULL, true);
        sceneObjects[i]->renderer->GetMainMaterial()->SetMat4("_model",  sceneObjects[i]->GetModelMatrix());
        sceneObjects[i]->renderer->GetMainMaterial()->SetInt("_cascadeCount", shadowCascadeLevels.size());
        sceneObjects[i]->renderer->GetMainMaterial()->SetFloat("_exposure", Exposure);
        sceneObjects[i]->renderer->GetMainMaterial()->SetFloat("_radius", Debug::DebugConfig::pointLightRadiusFalloff);
        sceneObjects[i]->renderer->GetMainMaterial()->SetFloat("_PreComputedEnvIntensity", PreComputedEnvIntensity);
        sceneObjects[i]->renderer->GetMainMaterial()->SetFloat("_PreComputedLightIntensity", PreComputedLightIntensity);
        sceneObjects[i]->renderer->GetMainMaterial()->SetVec3("_cameraPos", CameraPosition);
        sceneObjects[i]->renderer->GetMainMaterial()->SetVec3("_mainLightDirection", DirectionalLight.Direction);
        sceneObjects[i]->renderer->GetMainMaterial()->SetVec3("_mainLightColor", DirectionalLight.Color);
        sceneObjects[i]->renderer->GetMainMaterial()->SetInt("_isInstancing",1);
        sceneObjects[i]->renderer->GetMainMaterial()->SetFloat("_pcssSearchRadius", pcssSearchRadius);
        sceneObjects[i]->renderer->GetMainMaterial()->SetFloat("_pcssFilterRadius", pcssFilterRadius);
        sceneObjects[i]->renderer->GetMainMaterial()->SetFloat("_normalBias", normalBias);
        sceneObjects[i]->renderer->GetMainMaterial()->SetFloat("_depthBias", depthBias);
        sceneObjects[i]->renderer->DrawInstanced(128*128);
        if(!sceneObjects[i]->renderer->IsBackFaceCull)
        {
            CullState(true, GL_BACK);
        }
    }
}

std::vector<float> ForwardRenderPass::GetSkyboxVertices()
{
    std::vector<float>skyboxVertices = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    return skyboxVertices;
}

