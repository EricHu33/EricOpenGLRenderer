#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // include glad to get all the required OpenGL headers
#include <robin_hood/robin_hood.h>
#include <string>
#include <sstream>
#include <unordered_map>

#include <glm/detail/func_packing_simd.inl>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_vec.hpp>


class Shader
{
public:
    // the program ID
    unsigned int ID;
    robin_hood::unordered_map<size_t, GLuint> BindedTextures;
    int lastBindTextureSlot = -1;
    Shader(){}
    
    Shader(const char* prefix, const char* vertexPath, const char* fragmentPath, bool includeCommon = true);
    void create(const char* prefix, const char* vertexPath, const char* fragmentPath, bool includeCommon = true);
    void use();
    void Dispose();
    void SetInt1(const std::string& name, int r) const;
    void SetVec1(const std::string& name, float r) const;
    void SetVec2(const std::string& name, float r, float g) const;
    void SetVec2(const std::string& name, glm::vec2 v) const;
    void SetVec3(const std::string& name, float r, float g, float b) const;
    void SetVec3(const std::string& name, glm::vec3 v) const;
    void SetVec4(const std::string& name, float r, float g, float b, float a) const;
    void SetVec4(const std::string& name, glm::vec4 v) const;
    void SetMat4(const std::string& name, glm::mat4 mat) const;
    void SetCubeMapTextureUniform(size_t key, std::string name, unsigned int texture);
    void SetTextureUniform(size_t key, std::string name, unsigned int texture);
    void SetTexture2dArrayUniform(size_t key, std::string name, unsigned int texture);
    GLuint FindTextureUnitSlot(size_t key);

private:
    void checkCompileErrors(unsigned int shader, std::string type, std::string filePath);
};

#endif