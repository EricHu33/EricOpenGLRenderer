#pragma once

#include <glad/glad.h>

class RenderPass
{
public:
    virtual void Create(int windowWidth, int windowHeight) = 0;
    virtual void Render(float dt) = 0;
    virtual void Dispose() = 0;
    virtual ~RenderPass() = default;
    void BlendState(bool enable, GLenum mode, GLenum srcFactor, GLenum dstFactor)
    {
        if(enable)
        {
            glEnable(GL_BLEND);
        }
        else
        {
            glDisable(GL_BLEND);
        }
        glBlendEquation(mode);
        glBlendFunc(srcFactor, dstFactor);
    }

    void StencilState(bool enable, GLenum compare, GLint ref, GLuint mask, GLenum  sfail, GLenum zfail, GLenum zpass, GLuint writeMask)
    {
        if(enable)
        {
            glEnable(GL_STENCIL_TEST);
        }
        else
        {
            glDisable(GL_STENCIL_TEST);
        }
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(writeMask);
    }

    void DepthState(bool enable, GLenum depthFunc, GLenum detphWrite)
    {
        if(enable)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }
        glDepthFunc(depthFunc);
        glDepthMask(detphWrite);
    }

    void CullState(bool enable, GLenum cullFace)
    {
        if(enable)
        {
            glEnable(GL_CULL_FACE);
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }
        glCullFace(cullFace);
    }
};