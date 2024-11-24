#include "Shader.h"

#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include <vector>

Shader::Shader(const char* prefix, const char* vertexPath, const char* fragmentPath, bool includeCommon)
{
    lastBindTextureSlot = -1;
    
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream commonShaderFile;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream brdfShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    std::string dirPath = prefix;
    std::string vPath = vertexPath;
    std::string fPath = fragmentPath;
    std::string commonPath = "resources/shaders/common.glsl";
    std::string brdfPath = "resources/shaders/BRDF.glsl";
    
    try
    {
        // open files
        commonShaderFile.open(dirPath + commonPath);
        brdfShaderFile.open(dirPath + brdfPath);
        vShaderFile.open(dirPath + vPath);
        fShaderFile.open(dirPath + fPath);
        std::stringstream commonShaderStream;
        std::stringstream vShaderStream, fShaderStream;
        commonShaderStream << commonShaderFile.rdbuf();
        
        vShaderStream << commonShaderStream.str();
        vShaderStream << vShaderFile.rdbuf();

        fShaderStream << commonShaderStream.str();
        fShaderStream << brdfShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        commonShaderFile.close();
        vShaderFile.close();
        brdfShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::" << vertexPath << std::endl;
        std::cout << "ERROR::SHADER::" << fragmentPath << std::endl;
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;

    //vertex 
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    checkCompileErrors(vertex, "VERTEX", vertexPath);
    //fragment
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT", fragmentPath);

    // shader program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM", "");
   
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::create(const char* prefix, const char* vertexPath, const char* fragmentPath, bool includeCommon)
{
    lastBindTextureSlot = -1;
    
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream commonShaderFile;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    std::string dirPath = prefix;
    std::string vPath = vertexPath;
    std::string fPath = fragmentPath;
    std::string commonPath = "resources/shaders/common.glsl";
    std::vector<std::string> fsIncludePaths;
    fsIncludePaths.push_back("resources/shaders/common/BRDF.glsl");
    fsIncludePaths.push_back("resources/shaders/common/sh.glsl");
    fsIncludePaths.push_back("resources/shaders/common/lighting.glsl");
    fsIncludePaths.push_back("resources/shaders/common/pom.glsl");
    fsIncludePaths.push_back("resources/shaders/common/shadow.glsl");
    
    try
    {
        // open files
        commonShaderFile.open(dirPath + commonPath);
        vShaderFile.open(dirPath + vPath);
        fShaderFile.open(dirPath + fPath);
        std::stringstream commonShaderStream;
        std::stringstream vShaderStream, fShaderStream;
        commonShaderStream << commonShaderFile.rdbuf();
        
        vShaderStream << commonShaderStream.str();
        vShaderStream << vShaderFile.rdbuf();

        fShaderStream << commonShaderStream.str();
        for(int i = 0; i < fsIncludePaths.size();i++)
        {
            std::ifstream includeFile;
            includeFile.open(dirPath + fsIncludePaths[i]);
            fShaderStream << includeFile.rdbuf();
            includeFile.close();
            fShaderStream << "\n";
        }
        fShaderStream << "#line 0\n";
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        commonShaderFile.close();
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::" << vertexPath << std::endl;
        std::cout << "ERROR::SHADER::" << fragmentPath << std::endl;
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;

    //vertex 
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);

    checkCompileErrors(vertex, "VERTEX", vertexPath);
    //fragment
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT", fragmentPath);

    // shader program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM", "");
   
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}


void Shader::use()
{
    glUseProgram(ID);
}

void Shader::Dispose()
{
    glDeleteProgram(ID);
}

void Shader::SetInt1(const std::string& name, int r) const
{
    int vertexColorLocation = glGetUniformLocation(ID, name.c_str());
    glUniform1i(vertexColorLocation, r);
}

void Shader::SetVec1(const std::string& name, float r) const
{
    int vertexColorLocation = glGetUniformLocation(ID, name.c_str());
    glUniform1f(vertexColorLocation, r);
}

void Shader::SetVec2(const std::string& name, float r, float g) const
{
    int vertexColorLocation = glGetUniformLocation(ID, name.c_str());
    glUniform2f(vertexColorLocation, r,g);
}

void Shader::SetVec2(const std::string& name, glm::vec2 v) const
{
    int vertexColorLocation = glGetUniformLocation(ID, name.c_str());
    glUniform2f(vertexColorLocation, v.r, v.g);
}

void Shader::SetVec3(const std::string& name, float r, float g, float b) const
{
    int vertexColorLocation = glGetUniformLocation(ID, name.c_str());
    glUniform3f(vertexColorLocation, r,g,b);
}

void Shader::SetVec3(const std::string& name, glm::vec3 v) const
{
    int vertexColorLocation = glGetUniformLocation(ID, name.c_str());
    glUniform3f(vertexColorLocation, v.r, v.g, v.b);
}

void Shader::SetVec4(const std::string& name, float r, float g, float b, float a) const
{
    int vertexColorLocation = glGetUniformLocation(ID, name.c_str());
    glUniform4f(vertexColorLocation, r,g,b,a);
}

void Shader::SetVec4(const std::string& name, glm::vec4 v) const
{
    int vertexColorLocation = glGetUniformLocation(ID, name.c_str());
    glUniform4f(vertexColorLocation, v.r, v.g, v.b, v.a);
}

void Shader::SetMat4(const std::string& name, glm::mat4 mat) const
{
    int uLoc = glGetUniformLocation(ID, name.c_str());
    glUniformMatrix4fv(uLoc, 1, GL_FALSE, glm::value_ptr(mat));
}

GLuint Shader::FindTextureUnitSlot(size_t key)
{
    if(BindedTextures.find(key) != BindedTextures.end())
    {
        return BindedTextures[key];
    }
    lastBindTextureSlot += 1;
    GLuint textureUnit = lastBindTextureSlot;
    BindedTextures[key] = textureUnit;
    return textureUnit;
}

void Shader::SetCubeMapTextureUniform(size_t key, std::string name, unsigned int texture)
{
    auto textureUnit = FindTextureUnitSlot(key);
    glUniform1i(glGetUniformLocation(ID, (name).c_str()), textureUnit);
    glBindTextureUnit(textureUnit, texture);
}

void Shader::SetTextureUniform(size_t key, std::string name, unsigned int texture)
{
    auto textureUnit = FindTextureUnitSlot(key);
    glUniform1i(glGetUniformLocation(ID, (name).c_str()), textureUnit);
    glBindTextureUnit(textureUnit, texture);
}

void Shader::SetTexture2dArrayUniform(size_t key, std::string name, unsigned int texture)
{
    auto textureUnit = FindTextureUnitSlot(key);
    glUniform1i(glGetUniformLocation(ID, (name).c_str()), textureUnit);
    glBindTextureUnit(textureUnit, texture);
}

void Shader::checkCompileErrors(unsigned shader, std::string type, std::string filePath)
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
