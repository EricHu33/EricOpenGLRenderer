    #include "BloomRenderPass.h"
#include <iostream>
#include <Shlwapi.h>
#include <tchar.h>
#include <Windows.h>

void BloomRenderPass::Create(int windowWidth, int windowHeight)
{
    glGenFramebuffers(2, m_pingpongFBO);
    glGenTextures(2, m_pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, m_pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Framebuffer not complete!" << std::endl;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    TCHAR buffer[2048];
    GetModuleFileName ( NULL, buffer, sizeof(buffer));
    std::wcout << "path = " << buffer << std::endl;
    TCHAR directory[2048];
    _tcscpy_s(directory, 2048, buffer);
    PathRemoveFileSpec(directory);
    std::wstring wstr(directory);
    std::string dirStr(wstr.begin(), wstr.end());
    
    dirStr = dirStr + "\\..\\..\\";
    
    m_shader = std::make_shared<Shader>(dirStr.c_str(), "resources/shaders/ScreenQuad.vert", "resources/shaders/Blur.frag");
}

void BloomRenderPass::RenderQuad4()
{
    glBindVertexArray(FullScreenQuad4VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void BloomRenderPass::Render(float dt)
{
    m_horizontal = true;
    bool first_iteration = true;
    int amount = 10;
    m_shader->use();
    for (unsigned int i = 0; i < amount; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_pingpongFBO[m_horizontal]); 
        m_shader->SetInt1("horizontal", m_horizontal);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(
            GL_TEXTURE_2D, first_iteration ? CurrentColorBuffer : m_pingpongColorbuffers[!m_horizontal]
        ); 
        RenderQuad4();
        m_horizontal = !m_horizontal;
        if (first_iteration)
            first_iteration = false;
    }
}


void BloomRenderPass::Dispose()
{
    m_shader->Dispose();
}



