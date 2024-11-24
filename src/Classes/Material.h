#ifndef MATERIAL_H
#define MATERIAL_H
#include <unordered_map>
#include <glm/glm.hpp>
#include "Shader.h"
#include <nlohmann/json.hpp>
#include <robin_hood/robin_hood.h>
struct TextureVar
{
    int ID = -1;
    bool IsSharedTexture = false;
    std::string Path = "";
    size_t HashedTextureName;
    TextureVar() {  }
};

template <typename T>
struct MaterialVar
{
    T value;
    bool isDirty;
    MaterialVar() : value() { isDirty = true; }
    MaterialVar(T val) : value(val) { isDirty = true; }
    MaterialVar<T>& operator= (const MaterialVar<T>& otherMatVar)
    {
        if(value != otherMatVar.value)
        {
            value = otherMatVar.value;
            isDirty = true;
        }
        return *this;
    }
};

class Material
{
private :
    robin_hood::unordered_map<std::string, MaterialVar<float>> _floatVars;
    robin_hood::unordered_map<std::string, MaterialVar<int>> _intVars;
    robin_hood::unordered_map<std::string, MaterialVar<glm::vec2>> _vec2Vars;
    robin_hood::unordered_map<std::string, MaterialVar<glm::vec3>> _vec3Vars;
    robin_hood::unordered_map<std::string, MaterialVar<glm::vec4>> _vec4Vars;
    robin_hood::unordered_map<std::string, MaterialVar<glm::mat4>> _mat4Vars;
    robin_hood::unordered_map<std::string, TextureVar> _textures;
    robin_hood::unordered_map<std::string, TextureVar> _textureArrays;
    robin_hood::unordered_map<std::string, TextureVar> _textureCubeMaps;
    Shader* _shader;
    bool _isForceAllUpdate = false;
    bool _isTextureDirty = false;
public:
    nlohmann::json ToJson();
    void FromJson(nlohmann::json json);
    int GetInt(std::string name);
    float GetFloat(std::string name);
    void SetInt(std::string name, int value);
    void SetFloat(std::string name, float value);
    void SetVec2(std::string name, glm::vec2 value);
    void SetVec3(std::string name, glm::vec3 value);
    void SetVec4(std::string name, glm::vec4 value);
    void SetMat4(std::string name, glm::mat4 value);
    TextureVar TryGetTexture(std::string name)
    {
        if(_textures.find(name) != _textures.end())
        {
            return _textures[name];
        }
        TextureVar empty;
        return empty;
    }
    void SetTexture(std::string name, int id, const char* path = NULL, bool isSharedTexture = false);
    void SetTextureArray(std::string name, int id, bool isSharedTexture = false);
    void SetCubeMapTexture(std::string name, int id, bool isSharedTexture = false);
    void MarkTextureDirty();
    void ForceUpdate();
    void UpdateVariablesToShader();
    void DrawMaterial();
    Material()
    {
    }
    Material(Shader* shader)
    {
        _shader = shader;
    }
    
    Shader* GetShader()
    {
        return _shader;
    }

    void SetShader(Shader* shader)
    {
        _shader = shader;
    }
    
};
#endif