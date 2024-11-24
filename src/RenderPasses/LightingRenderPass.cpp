#include "LightingRenderPass.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../DebugConfig.h"

void LightingRenderPass::Create(int windowWidth, int windowHeight)
{
    this->m_windowWidth = windowWidth;
    this->m_windowHeight = windowHeight;
    
    glGenFramebuffers(1, &m_lightingPassFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_lightingPassFBO);
    glGenTextures(2, m_hdrBuffers);
    for(int i = 0; i < 2;i++)
    {
        glBindTexture(GL_TEXTURE_2D, m_hdrBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_hdrBuffers[i], 0);
    }

    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_positionMapKey = std::hash<std::string>{}("_gPosition");
    m_normalMapKey = std::hash<std::string>{}("_gNormal");
    m_albedoKey = std::hash<std::string>{}("_gAlbedoSpec");
    m_matParamKey = std::hash<std::string>{}("_gMaterialParam");
    m_shadowKey = std::hash<std::string>{}("_shadowMap");
    m_ssaoKey = std::hash<std::string>{}("_SSAOTex");
    m_prefiterKey = std::hash<std::string>{}("_prefilterMap");
    m_brdfKey = std::hash<std::string>{}("_brdfLUT");
}

void LightingRenderPass::Render(float dt)
{
    //copy detph from gbuffer pass
    glBindFramebuffer(GL_FRAMEBUFFER, GeometryPassFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_lightingPassFBO);
    glBlitFramebuffer(
      0, 0, m_windowWidth, m_windowHeight, 0, 0, m_windowWidth, m_windowHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST
    );
    glBindFramebuffer(GL_FRAMEBUFFER, m_lightingPassFBO);
    glClear(GL_COLOR_BUFFER_BIT);

    PreparePuncutalLightPass();

    glDrawBuffer(GL_NONE);
    glClearStencil(1);
    //glClear(GL_STENCIL_BUFFER_BIT);
    CullState(true, GL_BACK);
    DepthState(true, GL_GEQUAL, GL_FALSE);
    StencilState(true, GL_ALWAYS, 1, 0xFF, GL_KEEP, GL_KEEP, GL_DECR, 0xFF);

    DrawPointLights();

    glDrawBuffers(2, m_attachments);
    CullState(true, GL_FRONT);
    DepthState(true, GL_GEQUAL, GL_FALSE);
    StencilState(false, GL_EQUAL, 1, 0xFF, GL_KEEP, GL_KEEP, GL_KEEP, 0x00);
    BlendState(true, GL_FUNC_ADD, GL_ONE, GL_ONE);

    DrawPointLights();

    //directional lighting
    CullState(true, GL_BACK);
    DepthState(false, GL_LESS, GL_FALSE);
    StencilState(false, GL_EQUAL, 1, 0xFF, GL_KEEP, GL_KEEP, GL_KEEP, 0x00);
    BlendState(true, GL_FUNC_ADD, GL_ONE, GL_ONE);
    
    DirectionalLightShader->use();
    DrawDirectionalLight();

    glDisable(GL_STENCIL_TEST);
    DepthState(true, GL_EQUAL, GL_TRUE);
    CullState(true, GL_BACK);
    BlendState(false, GL_FUNC_ADD, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void LightingRenderPass::RenderQuad4()
{
    glBindVertexArray(FullScreenQuad4VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void LightingRenderPass::PreparePuncutalLightPass()
{
    punctualLightRenderer->GetMainMaterial()->GetShader()->use();
    punctualLightRenderer->GetMainMaterial()->SetFloat("_exposure", Exposure);
    punctualLightRenderer->GetMainMaterial()->SetFloat("_bloomThreshold", this->BloomThreshold);
    punctualLightRenderer->GetMainMaterial()->SetVec3("_cameraPos", this->CameraPosition);
    punctualLightRenderer->GetMainMaterial()->SetVec3("_diffuseColor",glm::vec3(1));
    punctualLightRenderer->GetMainMaterial()->SetTexture("_gPosition", this->gPosition);
    punctualLightRenderer->GetMainMaterial()->SetTexture("_gNormal", this->gNormal);
    punctualLightRenderer->GetMainMaterial()->SetTexture("_gAlbedoSpec", this->gAlbedoSpec);
    punctualLightRenderer->GetMainMaterial()->SetTexture("_gMaterialParam", this->gMaterialParam);
    punctualLightRenderer->GetMainMaterial()->SetTexture("_SSAOTex", this->ssao);
    punctualLightRenderer->GetMainMaterial()->SetCubeMapTexture("_prefilterMap", this->prefilterMap);
    punctualLightRenderer->GetMainMaterial()->SetTexture("_brdfLUT", this->brdfLUT);
}

void LightingRenderPass::DrawPointLights()
{
    for (unsigned int i = 0; i < PointLightsCount; i++)
    {
        std::ostringstream uniformPointLightName;
        uniformPointLightName << "_pointLight.";
        this->punctualLightRenderer->GetMainMaterial()->SetVec4(uniformPointLightName.str() + "color", PointLights[i].Color);
        this->punctualLightRenderer->GetMainMaterial()->SetVec4(uniformPointLightName.str() + "position", PointLights[i].Position);
        glm::mat4 lightObj2world = glm::mat4(1);
        lightObj2world = glm::translate(lightObj2world, glm::vec3(PointLights[i].Position));
        lightObj2world = glm::scale(lightObj2world, glm::vec3(6.0f));
        this->punctualLightRenderer->GetMainMaterial()->SetFloat("_radius", PointLights[i].Position.w);
        this->punctualLightRenderer->GetMainMaterial()->SetMat4("_model", lightObj2world);
        this->punctualLightRenderer->GetMainMaterial()->SetMat4("_cameraViewMatrix", this->ViewMatrix);
        this->punctualLightRenderer->Draw();
    }
}

void LightingRenderPass::DrawDirectionalLight()
{
    DirectionalLightShader->SetVec1("_PreComputedEnvIntensity", PreComputedEnvIntensity);
    DirectionalLightShader->SetVec1("_PreComputedLightIntensity", PreComputedLightIntensity);
    DirectionalLightShader->SetVec1("_bloomThreshold", BloomThreshold);
    DirectionalLightShader->SetVec3("_cameraPos", CameraPosition.r, CameraPosition.g, CameraPosition.b);
    DirectionalLightShader->SetTextureUniform(m_positionMapKey, "_gPosition", gPosition);
    DirectionalLightShader->SetTextureUniform(m_normalMapKey, "_gNormal", gNormal);
    DirectionalLightShader->SetTextureUniform(m_albedoKey, "_gAlbedoSpec", gAlbedoSpec);
    DirectionalLightShader->SetTextureUniform(m_matParamKey, "_gMaterialParam", gMaterialParam );
    DirectionalLightShader->SetTexture2dArrayUniform(m_shadowKey, "_shadowMap", cascadeShadowMapArray);
    DirectionalLightShader->SetTextureUniform(m_ssaoKey, "_SSAOTex", ssao);
    DirectionalLightShader->SetCubeMapTextureUniform(m_prefiterKey, "_prefilterMap", prefilterMap);
    DirectionalLightShader->SetTextureUniform(m_brdfKey, "_brdfLUT", brdfLUT);
    DirectionalLightShader->SetVec3("_mainLightDirection", DirectionalLight.Direction.x, DirectionalLight.Direction.y, DirectionalLight.Direction.z);
    DirectionalLightShader->SetVec3("_mainLightColor", DirectionalLight.Color.r, DirectionalLight.Color.g, DirectionalLight.Color.b);
    DirectionalLightShader->SetMat4("_view", glm::mat4(1.0f));
    DirectionalLightShader->SetMat4("_projection", glm::mat4(1.0f));
    DirectionalLightShader->SetMat4("_model", glm::mat4(1.0f));
    DirectionalLightShader->SetMat4("_cameraViewMatrix", ViewMatrix);
    DirectionalLightShader->SetVec1("_farPlane", _far);
    DirectionalLightShader->SetInt1("_cascadeCount", shadowCascadeLevels.size());
    DirectionalLightShader->SetVec1("_pcssSearchRadius", pcssSearchRadius);
    DirectionalLightShader->SetVec1("_pcssFilterRadius", pcssFilterRadius);
    DirectionalLightShader->SetVec1("_normalBias", normalBias);
    DirectionalLightShader->SetVec1("_depthBias", depthBias);
    DirectionalLightShader->SetVec3("_altShadowColor", AltShadowColor.r, AltShadowColor.g, AltShadowColor.b);
    
    DirectionalLightShader->SetVec1("_nearPlane", nearPlane);
    DirectionalLightShader->SetVec1("_lightWorldSize", pcssLightWorldSize);
    
    for (size_t i = 0; i < shadowCascadeLevels.size(); ++i)
    {
        DirectionalLightShader->SetVec1("_cascadePlaneDistances[" + std::to_string(i) + "]", shadowCascadeLevels[i]);
    }

    for (size_t i = 0; i < shadowCascadeFrustumSizes.size(); ++i)
    {
        DirectionalLightShader->SetVec1("_csmFrustumSizes[" + std::to_string(i) + "]", shadowCascadeFrustumSizes[i]);
    }
    RenderQuad4();
}

void LightingRenderPass::Dispose()
{
    glDeleteBuffers(1, &m_lightingPassFBO);
}


