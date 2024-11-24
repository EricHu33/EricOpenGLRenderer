#include "RenderUtil.h"


#include "stb/stb_image_write.h"

#include <iostream>
#include <GL/gl.h>
#include <stb/stb_image.h>

namespace Util
{
    std::vector<Renderer> RenderUtil::CreateRenderers(ModelLoader& loader, Material mat)
    {
        std::vector<Renderer> renderersForModelLoader;
        auto meshes = loader.GetMeshes();
        auto mats = loader.GetMaterials(mat);
            
        for(int i = 0; i < loader.GetMeshes()->size();i++)
        {
            Renderer renderer(&((*meshes)[i]), mats[i]);
            if((*meshes)[i].isTransparent)
            {
                renderer.IsTransparent = true;
            }
            renderersForModelLoader.push_back(renderer);
        }
        return renderersForModelLoader;
    }

    unsigned int RenderUtil::CreateQuad4()
    {
        unsigned int vao = 0, vbo = 0;
        if (vao == 0)
        {
            float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            };
            // setup plane VAO
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            return vao;
        }
    }

    GLuint RenderUtil::LoadTexture(const char *path, bool isSRGB)
        {
            GLuint textureId = -1;
            int width, height, nrChannels;
            glGenTextures(1, &textureId);
            glBindTexture(GL_TEXTURE_2D, textureId);
            
            unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
            if (data)
            {
                GLenum format;
                GLenum internalFormat;
                bool useAlphaTexture = false;
                if (nrChannels == 1)
                {
                    format = GL_RED;
                    internalFormat =  GL_RED;
                }
                else if (nrChannels == 3)
                {
                    format = GL_RGB;
                    internalFormat = isSRGB ? GL_SRGB : GL_RGB;
                }
                else if (nrChannels == 4)
                {
                    format = GL_RGBA;
                    internalFormat = isSRGB ? GL_SRGB_ALPHA : GL_RGBA;
                }
                
                glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height,0, format, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
                if(!useAlphaTexture)
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                }
                else
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                }
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            }
            else
            {
                std::cout << "Failed to load texture at " << path << std::endl;
            }
            stbi_image_free(data);
            return textureId;
        }

    GLuint RenderUtil::LoadTexturePoint(const char *path, bool isSRGB)
        {
            GLuint textureId = -1;
            int width, height, nrChannels;
            glGenTextures(1, &textureId);
            glBindTexture(GL_TEXTURE_2D, textureId);
            
            unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
            if (data)
            {
                GLenum format;
                GLenum internalFormat;
                bool useAlphaTexture = false;
                if (nrChannels == 1)
                {
                    format = GL_RED;
                    internalFormat =  GL_RED;
                }
                else if (nrChannels == 3)
                {
                    format = GL_RGB;
                    internalFormat = isSRGB ? GL_SRGB : GL_RGB;
                }
                else if (nrChannels == 4)
                {
                    format = GL_RGBA;
                    internalFormat = isSRGB ? GL_SRGB_ALPHA : GL_RGBA;
                }
                
                glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height,0, format, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
                if(!useAlphaTexture)
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                }
                else
                {
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                }
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            }
            else
            {
                std::cout << "Failed to load texture at " << path << std::endl;
            }
            stbi_image_free(data);
            return textureId;
        }

    void RenderUtil::SaveTextureToPNG(std::string& prefix, std::string path, GLuint textureID, int width, int height) {
        glBindTexture(GL_TEXTURE_2D, textureID);
        GLubyte* image = new GLubyte[width * height * 4]; // Assuming RGBA format
        stbi_flip_vertically_on_write(true);  
        // Get the texture data
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

        // Save the image as PNG
        stbi_write_png((prefix + path).c_str(), width, height, 4, image, width * 4);
printf("done writing image...........");
        delete[] image;
        glBindTexture(GL_TEXTURE_2D, 0);
        stbi_flip_vertically_on_write(false);  
    }

     unsigned int RenderUtil::LoadHDRTexture(std::string& prefix, std::string path)
    {
        stbi_set_flip_vertically_on_load(true);  
        int width, height, nrChannels;
        unsigned int textureID;
        float *data;
        data = stbi_loadf((prefix + path).c_str(), &width, &height, &nrChannels, 0);
        if(data)
        {
            glGenTextures(1, &textureID);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D,0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, data
            );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            stbi_image_free(data);
        }
        else
        {
            std::cout<<"somthing wrong when loading HDR texture"<< std::endl;
        }
        // stbi_set_flip_vertically_on_load(true);  

        return textureID;
    }
}
