#pragma once

#include "../RenderPass.h"
#include "../Classes/Shader.h"
#include "../Classes/Renderer.h"

class DebugRenderPass : RenderPass
{
public:
    void Create(int windowWidth, int windowHeight) override;
    void Render(float dt) override;
    void Dispose() override;

    Shader* ForwardLightDebugShader;
    Shader* DebugTextureShader;
    Shader* DebugTextureArrayShader;
    Shader* DebugTestShader;

    unsigned int debugVAO;
    bool isInit = false;

    int shadowMapLayer;
    unsigned int shadowMap;
    unsigned int rsmDepthMap;
    unsigned int rsmPositionMap;
    unsigned int rsmNormalMap;
    unsigned int rsmFluxMap;
    unsigned int indirectLightingMap;
    unsigned int bloomMask;
    unsigned int bloomResultMap;
    unsigned int gBufferNormal;
    unsigned int gBufferPosition;
    unsigned int gBufferAlbedo;
    unsigned int gBufferMaterial;
    unsigned int brdfLut;
    unsigned int ssaoMap;
    unsigned int depthPrepassMap;
    unsigned int computeResultMap;

    bool visualizeRSMDepth;
    bool visualizeRSMPosition;
    bool visualizeRSMNormal;
    bool visualizeRSMFlux;
    bool visualizeIndirectLighting;
    bool visualizeShadowMap;
    bool visuallizeBloomMask;
    bool visualizeBlurPass;
    
    bool visualizeNormal;
    bool visualizePosition;
    bool visualizeAlbedo;
    bool visualizeMaterialParams;
    bool visualizeSSAO;
    bool visualizeDepthPrepass;
    bool visualizeComputeShaderTex;
    int DebugTextureArrayLayer;
    glm::mat4 ProjectionMatrix;
    void DrawTextureOnQuadWithShader()
    {
        glDisable(GL_DEPTH_TEST);
        DebugTestShader->use();
        glBindVertexArray(debugVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
private:
    unsigned int m_fbo;
    unsigned int m_debugTexture;

    void DrawForwardLightDebugTextureOnQuad(unsigned int texture)
    {
        glDisable(GL_DEPTH_TEST);
        ForwardLightDebugShader->use();
        glBindVertexArray(debugVAO);
        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void DrawTextureOnQuad(unsigned int texture, bool debugCompute = false)
    {
        glDisable(GL_DEPTH_TEST);
        DebugTextureShader->use();
        DebugTextureShader->SetMat4("_projectionMatrix", ProjectionMatrix);
        DebugTextureShader->SetInt1("_debugCompute", debugCompute);
        glBindVertexArray(debugVAO);
        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D, texture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void DrawTextureArrayOnQuad(unsigned int textureArray, unsigned int layer)
    {
        glDisable(GL_DEPTH_TEST);
        DebugTextureArrayShader->use();
        DebugTextureArrayShader->SetInt1("_layer", layer);
        glBindVertexArray(debugVAO);
        glActiveTexture(GL_TEXTURE0); 
        glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindTexture(GL_TEXTURE_2D_ARRAY, layer);
    }
};