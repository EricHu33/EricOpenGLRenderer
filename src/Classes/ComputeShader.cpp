#include "ComputeShader.h"

#include <fstream>
#include <iostream>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>


ComputeShader::ComputeShader(const char* prefix, const char* filePath)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string computeShaderCodeStr;
    //std::ifstream commonShaderFile;
    std::ifstream computeShaderFile;
    // ensure ifstream objects can throw exceptions:
    computeShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    std::string dirPath = prefix;
    std::string vPath = filePath;
    std::string commonPath = "resources/shaders/common.glsl";
    //std::string brdfPath = "resources/shaders/BRDF.glsl";
    
    try
    {
        // open files
        computeShaderFile.open(dirPath + vPath);
        std::stringstream computeShaderStream;
        computeShaderStream << computeShaderFile.rdbuf();
        // close file handlers
        computeShaderFile.close();
        // convert stream into string
        computeShaderCodeStr = computeShaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::COMPUTE::SHADER::" << filePath << std::endl;
        std::cout << "ERROR::COMPUTE::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    const char* computeShaderCode = computeShaderCodeStr.c_str();

    unsigned int computeShader;

    //compute shader text 
    computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &computeShaderCode, NULL);
    glCompileShader(computeShader);

    checkCompileErrors(computeShader, "COMPUTE", filePath);

    // compute shader program
    ID = glCreateProgram();
    glAttachShader(ID, computeShader);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM", "");
   
    glDeleteShader(computeShader);
}

void ComputeShader::SetFloat(const std::string& name, float r) const
{
    int uLoc = glGetUniformLocation(ID, name.c_str());
    glUniform1f(uLoc, r);
}

void ComputeShader::SetVec2(const std::string& name, float r, float g) const
{
    int uLoc = glGetUniformLocation(ID, name.c_str());
    glUniform2f(uLoc, r,g);
}

void ComputeShader::SetVec3(const std::string& name, float r, float g, float b) const
{
    int uLoc = glGetUniformLocation(ID, name.c_str());
    glUniform3f(uLoc, r,g,b);
}

void ComputeShader::SetUInt1(const std::string& name, GLuint r) const
{
    int uLoc = glGetUniformLocation(ID, name.c_str());
    glUniform1ui(uLoc, r);
}

void ComputeShader::SetMat4(const std::string& name, glm::mat4 mat) const
{
    int uLoc = glGetUniformLocation(ID, name.c_str());
    glUniformMatrix4fv(uLoc, 1, GL_FALSE, glm::value_ptr(mat));
}

GLuint ComputeShader::FindTextureUnitSlot(GLuint texture)
{
    if(BindedTextures.find(texture) != BindedTextures.end())
    {
        return BindedTextures[texture];
    }
    lastBindTextureSlot += 1;
    GLuint textureUnit = lastBindTextureSlot;
    BindedTextures[texture] = textureUnit;
    return textureUnit;
}

void ComputeShader::SetTextureUniform(std::string name, unsigned int texture)
{
    auto textureUnit = FindTextureUnitSlot(texture);
    glUniform1i(glGetUniformLocation(ID, (name).c_str()), textureUnit);
    glBindTextureUnit(textureUnit, texture);
}

void ComputeShader::use()
{
    glUseProgram(ID);
}

void ComputeShader::Dispose()
{
    glDeleteProgram(ID);
}

void ComputeShader::checkCompileErrors(unsigned shader, std::string type, std::string filePath)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            std::cout << "error at "<<filePath << std::endl;
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
