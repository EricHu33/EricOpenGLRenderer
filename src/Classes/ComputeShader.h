#pragma once

#include <glad/glad.h> // include glad to get all the required OpenGL headers

#include <string>
#include <sstream>
#include <robin_hood/robin_hood.h>
#include <glm/detail/func_packing_simd.inl>
#include <glm/detail/type_mat.hpp>


class ComputeShader
{
public:
    robin_hood::unordered_map<GLuint, GLuint> BindedTextures;
    int lastBindTextureSlot = -1;
    // the program ID
    unsigned int ID;
    ComputeShader(const char* prefix, const char* filePath);
    void use();
    void Dispose();
    void SetFloat(const std::string& name, float r) const;
    void SetVec2(const std::string& name, float r, float g) const;
    void SetVec3(const std::string& name, float r, float g, float b) const;
    void SetUInt1(const std::string& name, GLuint r) const;
    void SetMat4(const std::string& name, glm::mat4 mat) const;
    void SetTextureUniform(std::string name, unsigned int texture);

private:
    GLuint FindTextureUnitSlot(GLuint texture);
    void checkCompileErrors(unsigned int shader, std::string type, std::string filePath);
};

