#include "Material.h"
#define NOMINMAX 
#include <fstream>
#include <iostream>
#include <glm/glm.hpp>

#include "imgui.h"
#include <tchar.h>
#include <Shlwapi.h>
#include <stdio.h>

#include "../DebugConfig.h"
#include "../PlatformUtil.h"
#include "../RenderUtil.h"

namespace glm {
    void to_json(nlohmann::json& j, const glm::vec4& P) {
        j = { { "x", P.x }, { "y", P.y } , { "z", P.z } , { "w", P.w }};
    };
    
    void from_json(const nlohmann::json& j, glm::vec4& P) {
        P.x = j.at("x").get<double>();
        P.y = j.at("y").get<double>();
        P.z = j.at("z").get<double>();
        P.w = j.at("w").get<double>();
    }
    
    void to_json(nlohmann::json& j, const glm::vec3& P) {
        j = { { "x", P.x }, { "y", P.y } , { "z", P.z }};
    };
    
    void from_json(const nlohmann::json& j, glm::vec3& P) {
        P.x = j.at("x").get<double>();
        P.y = j.at("y").get<double>();
        P.z = j.at("z").get<double>();
    }
    
    void to_json(nlohmann::json& j, const glm::vec2& P) {
        j = { { "x", P.x }, { "y", P.y } };
    };
    
    void from_json(const nlohmann::json& j, glm::vec2& P) {
        P.x = j.at("x").get<double>();
        P.y = j.at("y").get<double>();
    }
}

nlohmann::json Material::ToJson()
{
    nlohmann::json json;
    for (auto it = _intVars.begin(); it != _intVars.end(); ++it) {
        json["int"][it->first] = it->second.value;
    }
    
    for (auto it = _floatVars.begin(); it != _floatVars.end(); ++it) {
        json["float"][it->first] = it->second.value;
    }
    
    for (auto it = _vec2Vars.begin(); it != _vec2Vars.end(); ++it) {
        json["vec2"][it->first] = it->second.value;
    }
    
    for (auto it = _vec3Vars.begin(); it != _vec3Vars.end(); ++it) {
        json["vec3"][it->first] = it->second.value;
    }
    
    for (auto it = _vec4Vars.begin(); it != _vec4Vars.end(); ++it) {
        json["vec4"][it->first] = it->second.value;
    }
    
    for (auto it = _textures.begin(); it != _textures.end(); ++it) {
        if(!it->second.Path.empty() && it->second.ID != -1)
        {
            json["texture"][it->first] = it->second.Path;
        }
    }
    return json;
}


void Material::FromJson(nlohmann::json json)
{
    for (auto it = _intVars.begin(); it != _intVars.end(); ++it) {
        if(json["int"].contains(it->first))
        {
            it->second = MaterialVar<int>(json["int"].at(it->first).get<int>());
        }
    }
    
    for (auto it = _floatVars.begin(); it != _floatVars.end(); ++it) {
        if(json["float"].contains(it->first))
        {
            it->second = MaterialVar<float>(json["float"].at(it->first).get<float>());
        }
    }
    
    for (auto it = _vec2Vars.begin(); it != _vec2Vars.end(); ++it) {
        if(json["vec2"].contains(it->first))
        {
            it->second = MaterialVar<glm::vec2>(json["vec2"].at(it->first).get<glm::vec2>());
        }
    }
    
    for (auto it = _vec3Vars.begin(); it != _vec3Vars.end(); ++it) {
        if(json["vec3"].contains(it->first))
        {
            it->second = MaterialVar<glm::vec3>(json["vec3"].at(it->first).get<glm::vec3>());
        }
    }
    
    for (auto it = _vec4Vars.begin(); it != _vec4Vars.end(); ++it) {
        if(json["vec4"].contains(it->first))
        {
            it->second = MaterialVar<glm::vec4>(json["vec4"].at(it->first).get<glm::vec4>());
        }
    }
    
    for (auto it = _textures.begin(); it != _textures.end(); ++it) {
        if(json["texture"].contains(it->first))
        {
            std::string path = json["texture"][it->first];
            auto textureId = Util::RenderUtil::LoadTexture(path.c_str());
            if(textureId != -1)
            {
                it->second.ID = textureId;
                it->second.Path = path;
                it->second.HashedTextureName = std::hash<std::string>{}(it->first);
            }
        }
    }
}

int Material::GetInt(std::string name)
{
    if (_intVars.find(name) != _intVars.end())
    {
        return _intVars[name].value;
    }
    return 0;
}


float Material::GetFloat(std::string name)
{
    return _floatVars[name].value;
}

void Material::SetFloat(std::string name, float value)
{
    _floatVars[name] = MaterialVar<float>(value);
}

void Material::SetInt(std::string name, int value)
{
    _intVars[name] = MaterialVar<int>(value);
}


void Material::SetVec2(std::string name, glm::vec2 value)
{
    _vec2Vars[name] = MaterialVar<glm::vec2>(value);
}

void Material::SetVec3(std::string name, glm::vec3 value)
{
    _vec3Vars[name] = MaterialVar<glm::vec3>(value);
}

void Material::SetVec4(std::string name, glm::vec4 value)
{
    _vec4Vars[name] = MaterialVar<glm::vec4>(value);
}

void Material::SetMat4(std::string name, glm::mat4 value)
{
    _mat4Vars[name] = MaterialVar<glm::mat4>(value);
}

void Material::SetTexture(std::string name, int id, const char* path, bool isSharedTexture)
{
    if(_textures[name].ID != id)
    {
        _textures[name].HashedTextureName = std::hash<std::string>{}(name);
    }
    _textures[name].ID = id;
    _textures[name].Path = path != NULL ? path : "";
    _textures[name].IsSharedTexture = isSharedTexture;
}

void Material::SetTextureArray(std::string name, int id, bool isSharedTexture)
{
    if(_textureArrays[name].ID != id)
    {
        _textureArrays[name].HashedTextureName = std::hash<std::string>{}(name);
    }
    _textureArrays[name].ID = id;
    _textureArrays[name].IsSharedTexture = isSharedTexture;
}

void Material::SetCubeMapTexture(std::string name, int id, bool isSharedTexture)
{
    if(_textureCubeMaps[name].ID != id)
    {
        _textureCubeMaps[name].HashedTextureName = std::hash<std::string>{}(name);
    }
    _textureCubeMaps[name].ID = id;
    _textureCubeMaps[name].IsSharedTexture = isSharedTexture;
}

void Material::ForceUpdate()
{
    _isForceAllUpdate = true;
}

void Material::MarkTextureDirty()
{
    _isTextureDirty = true;
}

void Material::DrawMaterial()
{
    for (auto it = _intVars.begin(); it != _intVars.end(); ++it) {
        ImGui::InputInt(it->first.c_str(), &(it->second.value), 1);
    }
    
    for (auto it = _floatVars.begin(); it != _floatVars.end(); ++it) {
        ImGui::SliderFloat(it->first.c_str(), &(it->second.value), 0.0f,4.0f);
    }
    
    for (auto it = _vec2Vars.begin(); it != _vec2Vars.end(); ++it) {
        ImGui::InputFloat2(it->first.c_str(), &(it->second.value).x);
    }
    
    for (auto it = _vec3Vars.begin(); it != _vec3Vars.end(); ++it) {
        if(it->first.find("Color") != std::string::npos)
        {
            ImGui::ColorEdit3(it->first.c_str(), &it->second.value.x);
        }
        else
        {
            ImGui::InputFloat3(it->first.c_str(), &(it->second.value).x);
        }
    }
    
    for (auto it = _vec4Vars.begin(); it != _vec4Vars.end(); ++it) {
        if(it->first.find("Color") != std::string::npos)
        {
            ImGui::ColorEdit4(it->first.c_str(), &it->second.value.x);
        }
        else
        {
            ImGui::InputFloat4(it->first.c_str(), &(it->second.value).x);
        }
    }
    
    for (auto it = _textures.begin(); it != _textures.end(); ++it) {
        if(std::find(Debug::DebugConfig::ExposeUniformTextures.begin(), Debug::DebugConfig::ExposeUniformTextures.end(), it->first) != Debug::DebugConfig::ExposeUniformTextures.end()) {
            if(it->second.ID != -1)
            {
                auto imGuiTexId = (void*)(intptr_t)it->second.ID;
                ImVec4 tint_col =  ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                ImVec4 border_col = ImGui::GetStyleColorVec4(ImGuiCol_Border);
                ImGui::Image(imGuiTexId, ImVec2((float)40, (float)40), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), tint_col, border_col);
                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    printf("clicked");
                }
                ImGui::SameLine();   
            }
            ImGui::Text(it->first.c_str());
            ImGui::SameLine();
            //ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 30 - ImGui::GetStyle().WindowPadding.x);
            if(ImGui::Button(("select###" + it->first).c_str()))
            {
                printf("select\n");
                TCHAR buffer[2048];
                GetModuleFileName ( NULL, buffer, sizeof(buffer));
                std::wcout << "path = " << buffer << std::endl;
                TCHAR directory[2048];
                _tcscpy_s(directory, 2048, buffer);
                PathRemoveFileSpec(directory);
                std::wstring wstr(directory);
                auto dirStr =  std::string(wstr.begin(), wstr.end());
                dirStr = dirStr + "\\..\\..\\";
                
                auto filePath  = Util::FileDialogs::OpenFile(dirStr.c_str());
                if(!filePath.empty())
                {
                    auto textureId = Util::RenderUtil::LoadTexture(filePath.c_str());
                    it->second.ID = textureId;
                    it->second.HashedTextureName = std::hash<std::string>{}(it->first);
                    it->second.Path = filePath;
                }
            }
        } else {

        }
        /*if(it->second.ID != -1)
        {
        }*/
    }
}


void Material::UpdateVariablesToShader()
{
    _shader->use();
    /*if(_textures.find("_baseMap") == _textures.end() || _textures["_baseMap"].ID == -1)
    {
       SetInt("_useColorOnly", 1);
    }*/

    for (auto it = _intVars.begin(); it != _intVars.end(); ++it) {
        if( _isForceAllUpdate || it->second.isDirty)
        {
            _shader->SetInt1(it->first, it->second.value);
            it->second.isDirty = false;
        }
    }
    
    for (auto it = _floatVars.begin(); it != _floatVars.end(); ++it) {
        if( _isForceAllUpdate || it->second.isDirty)
        {
            _shader->SetVec1(it->first, it->second.value);
            it->second.isDirty = false;
        }
    }

    // Update glm::vec2 variables
    for (auto it = _vec2Vars.begin(); it != _vec2Vars.end(); ++it) {
        if( _isForceAllUpdate || it->second.isDirty)
        {
            _shader->SetVec2(it->first, it->second.value.r, it->second.value.g);
            it->second.isDirty = false;
        }
    }

    // Update glm::vec3 variables
    for (auto it = _vec3Vars.begin(); it != _vec3Vars.end(); ++it) {
        if( _isForceAllUpdate || it->second.isDirty)
        {
            _shader->SetVec3(it->first, it->second.value.r, it->second.value.g, it->second.value.b);
            it->second.isDirty = false;
        }
    }

    // Update glm::vec4 variables
    for (auto it = _vec4Vars.begin(); it != _vec4Vars.end(); ++it) {
        if(_isForceAllUpdate ||  it->second.isDirty)
        {
            _shader->SetVec4(it->first, it->second.value.r, it->second.value.g, it->second.value.b, it->second.value.a);
            it->second.isDirty = false;
        }
    }

    // Update glm::mat4 variables
    for (auto it = _mat4Vars.begin(); it != _mat4Vars.end(); ++it) {
        if(_isForceAllUpdate ||  it->second.isDirty)
        {
            _shader->SetMat4(it->first, it->second.value);
            it->second.isDirty = false;
        }
    }

    for (auto it = _textures.begin(); it != _textures.end(); ++it) {
        if(it->second.ID != -1)
        {
            if(!it->second.IsSharedTexture || _isTextureDirty)
            _shader->SetTextureUniform( it->second.HashedTextureName, it->first, it->second.ID);
        }
    }

    for (auto it = _textureArrays.begin(); it != _textureArrays.end(); ++it) {
        if(it->second.ID != -1)
        {
            if(!it->second.IsSharedTexture || _isTextureDirty)
            _shader->SetTexture2dArrayUniform(it->second.HashedTextureName, it->first, it->second.ID);
        }
    }

    for (auto it = _textureCubeMaps.begin(); it != _textureCubeMaps.end(); ++it) {
        if(it->second.ID != -1)
        {
            if(!it->second.IsSharedTexture || _isTextureDirty)
            _shader->SetCubeMapTextureUniform(it->second.HashedTextureName, it->first, it->second.ID);
        }
    }
    _isForceAllUpdate = false;
    _isTextureDirty = false;
}