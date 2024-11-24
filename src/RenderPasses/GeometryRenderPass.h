#pragma once

#include "../RenderPass.h"
#include "../Classes/Shader.h"
#include "../Classes/Renderer.h"
#include "../Scene.h"

class GeometryRenderPass : RenderPass
{
public:
    void Create(int windowWidth, int windowHeight) override;
    void Render(float dt) override;
    void Dispose() override;


    glm::vec3 CameraPosition = glm::vec3(0);
    glm::mat4 ViewMatrix = glm::mat4(1);
    glm::mat4 ProjMatrix = glm::mat4(1);
    std::vector<std::shared_ptr<SceneObject>> SceneObjects;

    unsigned int GetFBO() { return m_gbufferFBO; }
    unsigned int zPrepass;
    unsigned int GetPositionBuffer() { return m_gPosition; }
    unsigned int GetNormalBuffer() { return m_gNormal; }
    unsigned int GetAlbedoBuffer() { return m_gAlbedo; }
    unsigned int GetMaterialParamBuffer() { return m_gMaterialParam; }
private:
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    //FBO
    unsigned int m_gbufferFBO = 0;
    //RBO
    unsigned int m_rbo = 0;
    //target rextures
    unsigned int m_gPosition = 0;
    unsigned int m_gNormal = 0;
    unsigned int m_gAlbedo = 0;
    unsigned int m_gMaterialParam = 0;
    unsigned int m_gBufferAttachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    void DrawRenderers();
};
