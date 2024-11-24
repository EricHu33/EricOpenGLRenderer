#include "PBRSphereDemoScene.h"

#include <random>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "../Classes/PhotmetryLighting.h"
#include "../RenderUtil.h"

namespace EricScene
{
    void PBRSphereDemoScene::CreateScene()
    {
        Scene::CreateScene();
        _MeshModelLoaderMap["sphere"] = std::make_shared<ModelLoader>((dirStr + "resources/models/uv_sphere.fbx").c_str());
        
        std::mt19937 rng(std::time(nullptr));
        std::uniform_real_distribution<float> distribution(0.2f, 1.0f);
        
        for(unsigned int i = 0; i < 8; i++)
        {
            PointLight p;
            p.Color = glm::vec4(0.4 * distribution(rng), 0.6 *distribution(rng), 0.5 * distribution(rng), 0);
            p.Position = glm::vec4(- 2.5 + i * 1.3f, 0.25f, -6.2f, 0);
            p.Position.w = 5.0f;
            p.Color.w = PhotometryLighting::lmToCandela(2400);
            _PointLights.push_back(p);
        }

        Material debugLightCubeMat(&unlitLightShader);
        auto debugLightRenderers = Util::RenderUtil::CreateRenderers(*_MeshModelLoaderMap["lightVolume"], debugLightCubeMat);
        
        Material sphereMat(&geometryPassDeferredShader);
        drawPBRSpheresDeferred(5,5, glm::vec3(-2.5, 2.0f, -2.0f), sphereMat, PBRType::Standard, glm::vec4(1,1,1,1));
        
        Material cleraCoatMat(&geometryPassDeferredShader);
        drawPBRSpheresDeferred(5,5, glm::vec3(-2.5, 2.0f, -3.0f), cleraCoatMat, PBRType::ClearCoat, glm::vec4(1,0,0,1));
        
        Material fwdCleraCoatMat(&litForwardShader);
        drawPBRSpheresForward(5,5, glm::vec3(-2.5, 2.0f, -4.0f), fwdCleraCoatMat, PBRType::ClearCoat, glm::vec4(1,0,0,1));

        Material fwdAnisoMat(&litAnisoForwardShader);
        drawPBRSpheresForward(5,5, glm::vec3(-2.5, 2.0f, -5.0f), fwdAnisoMat, PBRType::Aniso,  glm::vec4(1.0,0.50f,0.20f, 1));
        
        Material forwardSphereMat(&litForwardShader);
        //Transparent PBR
        drawPBRSpheresForward(5,5, glm::vec3(-2.5, 2.0f, -6.0f), forwardSphereMat, PBRType::Transparent, glm::vec4(1,1,1,1));
        SortRenderers();
    }

    void PBRSphereDemoScene::Update(float dt)
    {
        Scene::Update(dt);
        float timeValue = glfwGetTime();
        for (unsigned int i = 0; i < std::size(_PointLights); i++)
        {
            auto pos = _PointLights[i].Position;
            _PointLights[i].Position = glm::vec4((i % 10) +  sin(timeValue) - 5, 0, -4.5f, 0);
            _PointLights[i].Position.w = 3.0f;
            
        }
    }

    void PBRSphereDemoScene::DeleteScene()
    {
        Scene::DeleteScene();
    }

    void PBRSphereDemoScene::drawPBRSpheresDeferred(int x, int y, glm::vec3 origin, Material mat, PBRType materialType, glm::vec4 color)
    {
        std::vector<Renderer> pbrShperes = Util::RenderUtil::CreateRenderers( *_MeshModelLoaderMap["sphere"], mat);
        
        for(int i = 0; i < y; i++)
        {
            for(int j = 0; j < x; j++)
            {
                glm::mat4 obj2world;
                obj2world = glm::mat4(1);
                obj2world = glm::translate(obj2world, origin + glm::vec3( j * 0.75f, -i * 0.75f, 0)); // translate it down so it's at the center of the scene
                obj2world = glm::rotate(obj2world, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                obj2world = glm::scale(obj2world, glm::vec3(0.25f));
                for(int k = 0; k < pbrShperes.size(); k++)
                {
                    pbrShperes[k].GetMainMaterial()->SetFloat("_UseParallaxMapping", 0);
                    pbrShperes[k].GetMainMaterial()->SetInt("_parallaxSteps", 1);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_heightScale", 1);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_normalStrength", 0.0f);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_roughness", ((float)j) / (float)x);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_metallic",  (1.0f - (float)i / (float)y));

                    if(materialType == PBRType::ClearCoat)
                    {
                        pbrShperes[k].GetMainMaterial()->SetInt("_isClearCoat", 1);
                        pbrShperes[k].GetMainMaterial()->SetFloat("_clearCoat", 1.0f - ((float)j + 0.0f) / (float)x);
                        pbrShperes[k].GetMainMaterial()->SetFloat("_clearCoatRoughness", (1.0f - (float)i / (float)y));
                    }
                    else
                    {
                        pbrShperes[k].GetMainMaterial()->SetInt("_isClearCoat", 0);
                    }
                    
                    pbrShperes[k].GetMainMaterial()->SetFloat("_uvScale", 1.0f);
                    pbrShperes[k].GetMainMaterial()->SetInt("_useColorOnly", 1.0f);
                    pbrShperes[k].GetMainMaterial()->SetVec4("_baseColor", color);
                    pbrShperes[k].GetMainMaterial()->SetInt("_useOcclusionRoughMetalMap", 0);
                }
                auto skinObj0 = AddSceneObject(pbrShperes, obj2world);
                _AllSceneObjects.push_back(skinObj0);
            }
        }
    }

    void PBRSphereDemoScene::drawPBRSpheresForward(int x, int y, glm::vec3 origin, Material mat, PBRType materialType, glm::vec4 color)
    {
        std::vector<Renderer> pbrShperes = Util::RenderUtil::CreateRenderers( *_MeshModelLoaderMap["sphere"], mat);
        
        for(int i = 0; i < y; i++)
        {
            for(int j = 0; j < x; j++)
            {
                glm::mat4 obj2world;
                obj2world = glm::mat4(1);
                obj2world = glm::translate(obj2world, origin + glm::vec3( j * 0.75f, -i * 0.75f, 0)); // translate it down so it's at the center of the scene
                obj2world = glm::rotate(obj2world, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                obj2world = glm::scale(obj2world, glm::vec3(0.25f));
                for(int k = 0; k < pbrShperes.size(); k++)
                {
                    pbrShperes[k].GetMainMaterial()->SetFloat("_UseParallaxMapping", 0);
                    pbrShperes[k].GetMainMaterial()->SetInt("_parallaxSteps", 1);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_heightScale", 1);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_normalStrength", 0.0f);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_roughness", ((float)j) / (float)x);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_metallic",  (1.0f - (float)i / (float)y));
                    if(materialType == PBRType::ClearCoat)
                    {
                        pbrShperes[k].GetMainMaterial()->SetInt("_isClearCoat", 1);
                        pbrShperes[k].GetMainMaterial()->SetFloat("_clearCoat", 1.0f - ((float)j + 0.0f) / (float)x);
                        pbrShperes[k].GetMainMaterial()->SetFloat("_clearCoatRoughness", (1.0f - (float)i / (float)y));
                    }

                    if(materialType == PBRType::Aniso)
                    {
                        pbrShperes[k].GetMainMaterial()->SetFloat("_metallic", 0.85f);
                        pbrShperes[k].GetMainMaterial()->SetFloat("_anisotropy", 2.0f * (((float)i+0.5f) / (float)y) - 1.0f);
                    }
                    
                    pbrShperes[k].GetMainMaterial()->SetFloat("_uvScale", 1.0f);
                    pbrShperes[k].GetMainMaterial()->SetInt("_useColorOnly", 1.0f);
                    pbrShperes[k].GetMainMaterial()->SetVec4("_baseColor", color);
                    pbrShperes[k].GetMainMaterial()->SetInt("_useOcclusionRoughMetalMap", 0);
                    
                    if(materialType == PBRType::Transparent)
                    {
                        color.a = ((float)j+0.3f) / (float)x;
                        pbrShperes[k].GetMainMaterial()->SetVec4("_baseColor", color);
                    }
                }
                auto obj = AddSceneObject(pbrShperes, obj2world);
                _AllSceneObjects.push_back(obj);
            }
        }
    }
}

