#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "../RenderPass.h"
#include "../Classes/Shader.h"
#include "../Classes/Renderer.h"
#include "SH.h"

namespace RenderUtil
{
    class CubeMapIBLRenderPass
    {
    public :
        static glm::vec3 MapXYSToDirection(int x, int y, int s, int width, int height)
        {
            float u = ((x + 0.5f) / float(width)) * 2.0f - 1.0f;
            float v = ((y + 0.5f) / float(height)) * 2.0f - 1.0f;
            v *= -1.0f;

            glm::vec3 dir = glm::vec3(0.0f);

            // +x, -x, +y, -y, +z, -z
            switch(s) {
            case 0:
                dir = glm::normalize(glm::vec3(1.0f, v, -u));
                break;
            case 1:
                dir = glm::normalize(glm::vec3(-1.0f, v, u));
                break;
            case 2:
                dir = glm::normalize(glm::vec3(u, 1.0f, -v));
                break;
            case 3:
                dir = glm::normalize(glm::vec3(u, -1.0f, v));
                break;
            case 4:
                dir = glm::normalize(glm::vec3(u, v, 1.0f));
                break;
            case 5:
                dir = glm::normalize(glm::vec3(-u, v, -1.0f));
                break;
            }

            return dir;
        }

        static SH9 ProjectOntoSH9(const glm::vec3& dir)
        {
            SH9 sh;

            // Band 0
            sh.Coefficients[0] = 0.282095f;

            // Band 1
            sh.Coefficients[1] = -0.488603f * dir.y;
            sh.Coefficients[2] = 0.488603f * dir.z;
            sh.Coefficients[3] = -0.488603f * dir.x;

            // Band 2
            sh.Coefficients[4] = 1.092548f * dir.x * dir.y;
            sh.Coefficients[5] = -1.092548f * dir.y * dir.z;
            sh.Coefficients[6] = 0.315392f * (3.0f * dir.z * dir.z - 1.0f);
            sh.Coefficients[7] = -1.092548f * dir.x * dir.z;
            sh.Coefficients[8] = 0.546274f * (dir.x * dir.x - dir.y * dir.y);

            return sh;
        }

        static SH9Color ProjectOntoSH9Color(const glm::vec3& dir, const glm::vec3& color)
        {
            SH9 sh = ProjectOntoSH9(dir);
            SH9Color shColor;
            for(int i = 0; i < 9; ++i)
                shColor.Coefficients[i] = color * sh.Coefficients[i];
            return shColor;
        }

        static glm::vec3 EvalSH9Cosine(const glm::vec3& dir, const SH9Color& sh)
        {
            SH9 dirSH = ProjectOntoSH9(dir);
            dirSH.Coefficients[0] *= CosineA0;
            dirSH.Coefficients[1] *= CosineA1;
            dirSH.Coefficients[2] *= CosineA1;
            dirSH.Coefficients[3] *= CosineA1;
            dirSH.Coefficients[4] *= CosineA2;
            dirSH.Coefficients[5] *= CosineA2;
            dirSH.Coefficients[6] *= CosineA2;
            dirSH.Coefficients[7] *= CosineA2;
            dirSH.Coefficients[8] *= CosineA2;

            glm::vec3 result;
            for(int i = 0; i < 9; ++i)
                result += dirSH.Coefficients[i] * sh.Coefficients[i];

            return result;
        }
    
        static SH9Color ProjectCubemapToSH(unsigned int cubeMapTexture, int resolution)
        {
            SH9Color result;
            for (size_t i = 0; i < 9; ++i)
            {
                result.Coefficients[i] = glm::vec3(0.0f);
            }
            auto width = resolution;
            auto height = resolution;
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
            float* imgData = new float[3 * width * height];
            float weightSum = 0.0f;
            for(int face = 0; face < 6; ++face)
            {
                glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGB, GL_FLOAT, imgData);
                for(int y = 0; y < height; ++y)
                {
                    for(int x = 0; x < width; ++x)
                    {
                        auto row = 3 * resolution * y;
                        auto col = 3 * x;
                        glm::vec3 sample = glm::vec3(imgData[row + col], imgData[row + col + 1], imgData[row + col + 2]);

                        float u = (x + 0.5f) / width;
                        float v = (y + 0.5f) / height;

                        // Account for cubemap texel distribution
                        u = u * 2.0f - 1.0f;
                        v = v * 2.0f - 1.0f;
                        const float temp = 1.0f + u * u + v * v;
                        const float weight = 4.0f / (sqrt(temp) * temp);

                        glm::vec3 dir = MapXYSToDirection(x, y, face, width, height);
                        result += ProjectOntoSH9Color(dir, sample) * weight;
                        weightSum += weight;
                    }
                }
            }
            delete imgData;
        
            result *= (4.0f * 3.14159f) / weightSum;
            return result;
        }

        static void AppliedSH9ResultToCubeMap(unsigned int& cubeMapTexture, SH9Color sh, int resolution)
        {
            SH9Color result;
            for (size_t i = 0; i < 9; ++i)
            {
                result.Coefficients[i] = glm::vec3(0.0f);
            }
            auto width = resolution;
            auto height = resolution;
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
            float* imgData = new float[3 * width * height];
            for(int face = 0; face < 6; ++face)
            {
                for(int y = 0; y < height; ++y)
                {
                    for(int x = 0; x < width; ++x)
                    {
                        auto row = 3 * resolution * y;
                        auto col = 3 * x;
                        glm::vec3 dir = MapXYSToDirection(x, y, face, width, height);
                        glm::vec3 color = EvalSH9Cosine(dir, sh);
                        imgData[row + col] = color.r;
                        imgData[row + col + 1] = color.g;
                        imgData[row + col + 2] = color.b;
                    }
                }
                glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, imgData);
            }
            delete imgData;
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        } 
        static unsigned int CreateCubeMapFromHDRTexture(Shader& cubeMapGenShader, unsigned int hdrTexture)
        {
            // create cubemap mesh
            std::vector<float> skyboxVertices = GetSkyboxVertices();
            unsigned int skyboxVAO, skyboxVBO;
            glGenVertexArrays(1, &skyboxVAO);
            glGenBuffers(1, &skyboxVBO);
            glBindVertexArray(skyboxVAO);
            glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
            glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size() * sizeof(float), &skyboxVertices[0], GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            //
        
            unsigned int captureFBO, captureRBO;
            glGenFramebuffers(1, &captureFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            
            glGenRenderbuffers(1, &captureRBO);
            glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
            unsigned int cubeMapTexture;
            glGenTextures(1, &cubeMapTexture);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
            for (unsigned int i = 0; i < 6; ++i)
            {
                // note that we store each face with 16 bit floating point values
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 
                             512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

            glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
            glm::mat4 captureViews[] = 
            {
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
             };

            glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            glDepthFunc(GL_LEQUAL); 
            cubeMapGenShader.use();
            cubeMapGenShader.SetMat4("_projection", captureProjection);
            //shader.SetTextureUniform("_equirectangularMap",0, hdrTexture);glActiveTexture(GL_TEXTURE0);
            cubeMapGenShader.SetInt1("_equirectangularMap", 0);
            glBindTexture(GL_TEXTURE_2D, hdrTexture);
            glActiveTexture(GL_TEXTURE0);
            for(int i = 0; i < 6; i++)
            {
                cubeMapGenShader.SetMat4("_view", captureViews[i]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeMapTexture, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glBindVertexArray(skyboxVAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexture);
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
            glBindFramebuffer(GL_FRAMEBUFFER, 0); 
            glDepthFunc(GL_LESS);

            glDeleteVertexArrays(1, &skyboxVAO);
            return cubeMapTexture;
        }

        static unsigned int ComputeIrradianceMap(Shader& shader, unsigned int envCubeMap)
        {
            // create cubemap mesh
            std::vector<float> skyboxVertices = GetSkyboxVertices();
            unsigned int skyboxVAO, skyboxVBO;
            glGenVertexArrays(1, &skyboxVAO);
            glGenBuffers(1, &skyboxVBO);
            glBindVertexArray(skyboxVAO);
            glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
            glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size() * sizeof(float), &skyboxVertices[0], GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            //
        
            unsigned int captureFBO, captureRBO;
            glGenFramebuffers(1, &captureFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            glGenRenderbuffers(1, &captureRBO);
            glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
            

            glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
            glm::mat4 captureViews[] = 
            {
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
             };
            unsigned int irradianceMap;
            glGenTextures(1, &irradianceMap);
            glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
            for (unsigned int i = 0; i < 6; ++i)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, 
                             GL_RGB, GL_FLOAT, nullptr);
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
            glDepthFunc(GL_LEQUAL);
            shader.use();
            shader.SetMat4("_projection", captureProjection);
            shader.SetInt1("_envCubeMap", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);
            
            glViewport(0, 0, 32, 32);
            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        
            for(int i = 0; i < 6; i++)
            {
                shader.SetMat4("_view", captureViews[i]);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glBindVertexArray(skyboxVAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDepthFunc(GL_LESS);
            glDeleteVertexArrays(1, &skyboxVAO);
            return irradianceMap;
        }

        static unsigned int ComputePrefilterMap(Shader& shader, unsigned int envCubeMap)
        {
            // create cubemap mesh
            std::vector<float> skyboxVertices = GetSkyboxVertices();
            unsigned int skyboxVAO, skyboxVBO;
            glGenVertexArrays(1, &skyboxVAO);
            glGenBuffers(1, &skyboxVBO);
            glBindVertexArray(skyboxVAO);
            glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
            glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size() * sizeof(float), &skyboxVertices[0], GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            //
            
            unsigned int captureFBO, captureRBO;
            glGenFramebuffers(1, &captureFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            glGenRenderbuffers(1, &captureRBO);
            glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
            glDepthFunc(GL_LEQUAL);

            glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
            glm::mat4 captureViews[] = 
            {
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
                glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
             };

            unsigned int preFilterMap;
            glGenTextures(1, &preFilterMap);
            glBindTexture(GL_TEXTURE_CUBE_MAP, preFilterMap);
            for(int i = 0; i < 6; i++)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        
            glDepthFunc(GL_LEQUAL); 
            shader.use();
            shader.SetMat4("_projection", captureProjection);
            shader.SetInt1("_envCubeMap", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, envCubeMap);
            
            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            unsigned int maxMipLevels = 5;
            for(unsigned int mip = 0; mip < maxMipLevels; mip++)
            {
                unsigned int mipWidth  = 128 * std::pow(0.5, mip);
                unsigned int mipHeight = 128 * std::pow(0.5, mip);
                glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
                glViewport(0,0,mipWidth, mipHeight);

                float roughness = (float)mip / float(maxMipLevels);
                shader.SetVec1("_roughness", roughness);
                for(int i = 0; i < 6; i++)
                {
                    shader.SetMat4("_view", captureViews[i]);
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, preFilterMap, mip);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    glBindVertexArray(skyboxVAO);
                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDepthFunc(GL_LESS);
            glDeleteVertexArrays(1, &skyboxVAO);
            return preFilterMap;
        }

        static unsigned int PrecomputeDFG(Shader& brdfLutShader, unsigned int quadVAO)
        {
            unsigned int brdfLutTexture;
            glGenTextures(1, &brdfLutTexture);
            glBindTexture(GL_TEXTURE_2D, brdfLutTexture);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGB16F,512,512,0,GL_RGB, GL_FLOAT, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            unsigned int captureFBO, captureRBO;
            glGenFramebuffers(1, &captureFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            glGenRenderbuffers(1, &captureRBO);
            glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
        
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLutTexture, 0);
            glViewport(0, 0, 512, 512);
            brdfLutShader.use();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
            return brdfLutTexture;
        }

        static std::vector<float> GetSkyboxVertices()
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
    };
};