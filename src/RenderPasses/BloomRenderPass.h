#pragma once

#include "../RenderPass.h"
#include "../Classes/Shader.h"

class BloomRenderPass : RenderPass
{
public:
    //members
    unsigned int CurrentColorBuffer = 0;
    unsigned int FullScreenQuad4VAO = 0;
    
    void Create(int windowWidth, int windowHeight) override;
    void Render(float dt) override;
    void Dispose() override;
    unsigned int GetBlurResult(){ return m_pingpongColorbuffers[!m_horizontal];}
private:
    bool m_horizontal = false;
    unsigned int m_pingpongFBO[2];
    unsigned int m_pingpongColorbuffers[2];
    void RenderQuad4();
    std::shared_ptr<Shader> m_shader = nullptr;
};