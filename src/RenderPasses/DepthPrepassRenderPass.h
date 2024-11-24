#pragma once

#include <vector>
#include "../RenderPass.h"
#include "../Classes/Shader.h"
#include "../Classes/Camera.h"
#include "../Scene.h"

class DepthPrepassRenderPass : RenderPass
{
public:
    void Create(int windowWidth, int windowHeight) override;
    void Render(float dt) override;
    void Dispose() override;
    unsigned int FullScreenVAO;
    std::vector<std::shared_ptr<SceneObject>> DepthRenderers;
    unsigned int GetDepthPrepassBuffer() { return m_depthTexture; }
    unsigned int GetDepthPrepassFBO() { return m_depthPrepassFBO; }
    Shader* DepthPrepassShader = nullptr;
    
private:
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    unsigned int m_depthPrepassFBO = 0;
    unsigned int m_depthTexture;
    const unsigned SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
};