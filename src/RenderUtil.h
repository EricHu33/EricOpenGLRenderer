#pragma once

#include <vector>
#include "Classes/Renderer.h"
#include "Classes/ModelLoader.h"

namespace Util
{
    class RenderUtil
    {
    public:
        static std::vector<Renderer> CreateRenderers(ModelLoader& loader, Material mat);
        
        static unsigned int CreateQuad4();
        
        static GLuint LoadTexture(const char *path, bool isSRGB = true);
        static GLuint LoadTexturePoint(const char *path, bool isSRGB = true); 
        static unsigned int LoadHDRTexture(std::string& prefix, std::string path);
        static void SaveTextureToPNG(std::string& prefix, std::string path, GLuint textureID, int width, int height);
    };
}
