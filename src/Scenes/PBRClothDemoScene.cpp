#include "PBRClothDemoScene.h"

#include <random>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "../Classes/PhotmetryLighting.h"
#include "../RenderUtil.h"

namespace EricScene
{
    void PBRClothDemoScene::CreateScene()
    {
        Scene::CreateScene();
        _MeshModelLoaderMap["sphere"] = std::make_shared<ModelLoader>((dirStr + "resources/models/uv_sphere.fbx").c_str());
        _MeshModelLoaderMap["cloth"] = std::make_shared<ModelLoader>((dirStr + "resources/models/UnityCloth.obj").c_str());
        _MeshModelLoaderMap["cloth2"] = std::make_shared<ModelLoader>((dirStr + "resources/models/cloth.glb").c_str());
        
        std::mt19937 rng(std::time(nullptr));
        std::uniform_real_distribution<float> distribution(0.2f, 1.0f);
        
        for(unsigned int i = 0; i < 1; i++)
        {
            PointLight p;
            p.Position = glm::vec4(- 2.5 + i * 1.3f, 0.25f, -6.2f, 0);
            p.Color = glm::vec4(0.4 * distribution(rng), 0.6 *distribution(rng), 0.5 * distribution(rng), 0);
            p.Position.w = 2.0f;
            p.Color.w = PhotometryLighting::lmToCandela(2400);
        }

        Material debugLightCubeMat(&unlitLightShader);
        auto debugLightRenderers = Util::RenderUtil::CreateRenderers(*_MeshModelLoaderMap["lightVolume"], debugLightCubeMat);
        
        Material clothMat(&litClothForwardShader);
        glm::mat4 object2World = glm::mat4(1);

        Texture threadMap;
        threadMap.id = Util::RenderUtil::LoadTexture((dirStr + "resources/models/Knit_Ribbed__TM.png").c_str(), false);
        threadMap.Path = "Knit_Ribbed__TM.png";
        threadMap.samplerName = "_threadMap";
        
        Texture fuzzMap;
        fuzzMap.id = Util::RenderUtil::LoadTexture((dirStr + "resources/models/WeavePattern01_F.png").c_str(), false);
        fuzzMap.Path = "WeavePattern01_F.png";
        fuzzMap.samplerName = "_fuzzMap";

        object2World = glm::mat4(1);
        object2World = glm::translate(object2World, glm::vec3(-2.0f, 1.0f, -3.0)); // translate it down so it's at the center of the scene
        object2World = glm::rotate(object2World, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        object2World = glm::scale(object2World, glm::vec3(3.0f));
        
        clothMat.SetFloat("_UseParallaxMapping", 0);
        clothMat.SetInt("_parallaxSteps", 1);
        clothMat.SetFloat("_heightScale", 1);
        clothMat.SetFloat("_normalStrength", 0.0f);
        clothMat.SetFloat("_roughness", 0.3f);
        clothMat.SetFloat("_metallic", 0);
        
        clothMat.SetFloat("_uvScale", 1.0f);
        clothMat.SetInt("_useColorOnly", 1);
        clothMat.SetVec4("_baseColor", glm::vec4(0,0,0, 1));
        clothMat.SetInt("_useOcclusionRoughMetalMap", 0);
        clothMat.SetFloat("_useThreadMap", 0);
        clothMat.SetInt("_useFuzzMap", 0);
        clothMat.SetFloat("_fuzzStrength", 0);
        clothMat.SetVec4("_sheenColor", glm::vec4(1.0,0,0, 1));
        auto clothRenderers0 = Util::RenderUtil::CreateRenderers(*_MeshModelLoaderMap["cloth"], clothMat);
        for(int i = 0; i < clothRenderers0.size();i++)
        {
            clothRenderers0[i].IsBackFaceCull = true;
        }
        auto obj0 = AddSceneObject(clothRenderers0, object2World);
        _AllSceneObjects.push_back(obj0);
        

        object2World = glm::mat4(1);
        object2World = glm::translate(object2World, glm::vec3(-1.0f, 1.0f, -3.0)); // translate it down so it's at the center of the scene
        object2World = glm::rotate(object2World, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        object2World = glm::scale(object2World, glm::vec3(3.0f));

        clothMat.SetFloat("_UseParallaxMapping", 0);
        clothMat.SetInt("_parallaxSteps", 1);
        clothMat.SetFloat("_heightScale", 1);
        clothMat.SetFloat("_normalStrength", 0.0f);
        clothMat.SetFloat("_roughness", 0.8f);
        clothMat.SetFloat("_metallic", 0);
        clothMat.SetFloat("_useSSS", 0);
        clothMat.SetVec4("_sssColor", glm::vec4(Debug::DebugConfig::testSphereColor3, 1));
        clothMat.SetFloat("_uvScale", 1.0f);
        clothMat.SetInt("_useColorOnly", 1);
        clothMat.SetVec4("_baseColor", glm::vec4(Debug::DebugConfig::testSphereColor, 1));
        clothMat.SetInt("_useOcclusionRoughMetalMap", 0);
        clothMat.SetInt("_useFuzzMap", 1);
        clothMat.SetFloat("_fuzzStrength", 1);
        clothMat.SetFloat("_useThreadMap", 0);
        clothMat.SetFloat("_threadScale", Debug::DebugConfig::demoSphereThreadScale);
        clothMat.SetFloat("_threadStrength", Debug::DebugConfig::demoSphereThreadStrength);
        clothMat.SetFloat("_threadAO", Debug::DebugConfig::demoSphereThreadAO);
        clothMat.SetFloat("_fuzzScale", Debug::DebugConfig::demoSphereFuzzScale);
        clothMat.SetVec4("_sheenColor", glm::vec4(Debug::DebugConfig::testSphereColor2, 1));
        

        
        clothMat.SetTexture(threadMap.samplerName, threadMap.id);
        clothMat.SetTexture(fuzzMap.samplerName, fuzzMap.id);
        auto clothRenderers1 = Util::RenderUtil::CreateRenderers(*_MeshModelLoaderMap["cloth"], clothMat);
        for(int i = 0; i < clothRenderers1.size();i++)
        {
            clothRenderers1[i].IsBackFaceCull = true;
        }
        auto obj1 = AddSceneObject(clothRenderers1, object2World);
        _AllSceneObjects.push_back(obj1);

        object2World = glm::mat4(1);
        object2World = glm::translate(object2World, glm::vec3(0.0f, 1.0f, -3.0)); // translate it down so it's at the center of the scene
        object2World = glm::rotate(object2World, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        object2World = glm::scale(object2World, glm::vec3(3.0f));

        clothMat.SetFloat("_UseParallaxMapping", 0);
        clothMat.SetInt("_parallaxSteps", 1);
        clothMat.SetFloat("_heightScale", 1);
        clothMat.SetFloat("_normalStrength", 0.0f);
        clothMat.SetFloat("_roughness", 0.57f);
        clothMat.SetFloat("_metallic", 0);
        
        clothMat.SetFloat("_uvScale", 1.0f);
        clothMat.SetInt("_useColorOnly", 1);
        clothMat.SetVec4("_baseColor", glm::vec4(0,12.0f/255.0f,52.0f/255.0f, 1));
        clothMat.SetInt("_useOcclusionRoughMetalMap", 0);
        clothMat.SetInt("_useFuzzMap", 0);
        clothMat.SetFloat("_fuzzStrength", 0);
        clothMat.SetVec4("_sheenColor", glm::vec4(0,114.0f/255.0f,1, 1));

        auto clothRenderers2 = Util::RenderUtil::CreateRenderers(*_MeshModelLoaderMap["cloth"], clothMat);
        for(int i = 0; i < clothRenderers2.size();i++)
        {
            clothRenderers2[i].IsBackFaceCull = true;
        }
        auto obj2 = AddSceneObject(clothRenderers2, object2World);
        _AllSceneObjects.push_back(obj2);
        
        object2World = glm::mat4(1);
        object2World = glm::translate(object2World, glm::vec3(4.25f, 1.0f, -3.0)); // translate it down so it's at the center of the scene
        object2World = glm::rotate(object2World, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        object2World = glm::scale(object2World, glm::vec3(0.20f));

        clothMat.SetFloat("_UseParallaxMapping", 0);
        clothMat.SetInt("_parallaxSteps", 1);
        clothMat.SetFloat("_heightScale", 1);
        clothMat.SetFloat("_normalStrength", 0.0f);
        clothMat.SetFloat("_roughness", 0.7f);
        clothMat.SetFloat("_metallic", 0);
        
        clothMat.SetFloat("_uvScale", 1.0f);
        clothMat.SetInt("_useColorOnly", 1);
        clothMat.SetVec4("_baseColor", glm::vec4(0,12.0f/255.0f,52.0f/255.0f, 1));
        clothMat.SetInt("_useOcclusionRoughMetalMap", 0);
        clothMat.SetInt("_useFuzzMap", 0);
        clothMat.SetFloat("_fuzzStrength", 0);
        clothMat.SetVec4("_sheenColor", glm::vec4(0.8,0.8,0.8, 1));
        
        auto clothRenderers3 = Util::RenderUtil::CreateRenderers(*_MeshModelLoaderMap["cloth2"], clothMat);
        for(int i = 0; i < clothRenderers3.size();i++)
        {
            clothRenderers3[i].IsBackFaceCull = true;
        }
        auto obj3 = AddSceneObject(clothRenderers3, object2World);
       // testCloths.push_back(clothRenderers2);
        _AllSceneObjects.push_back(obj3);
        
        Material standardMat(&geometryPassDeferredShader);
        
        glm::mat4 object2World2 = glm::mat4(1);
        object2World2 = glm::translate(object2World2, glm::vec3(5.0f, 1.0f, -3.0)); // translate it down so it's at the center of the scene
        object2World2 = glm::rotate(object2World2, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        object2World2 = glm::scale(object2World2, glm::vec3(0.2f));
        
        standardMat.SetFloat("_UseParallaxMapping", 0);
        standardMat.SetInt("_parallaxSteps", 1);
        standardMat.SetFloat("_heightScale", 1);
        standardMat.SetFloat("_normalStrength", 0.0f);
        standardMat.SetFloat("_roughness", 1);
        standardMat.SetFloat("_metallic", 0);
        
        standardMat.SetFloat("_uvScale", 1.0f);
        standardMat.SetInt("_useColorOnly", 1);
        standardMat.SetVec4("_baseColor", glm::vec4(0,12.0f/255.0f,52.0f/255.0f, 1));
        standardMat.SetInt("_useOcclusionRoughMetalMap", 0);
        
        auto standardClothRenderers = Util::RenderUtil::CreateRenderers(*_MeshModelLoaderMap["cloth2"], standardMat);
        for(int i = 0; i < standardClothRenderers.size();i++)
        {
            standardClothRenderers[i].IsBackFaceCull = false;
        }
        auto obj4 = AddSceneObject(standardClothRenderers, object2World2);
        _AllSceneObjects.push_back(obj4);
        
        Debug::DebugConfig::demoSphereFuzzScale = 7.0f;
        Debug::DebugConfig::demoSphereRoughness = 0.95f;
        Debug::DebugConfig::sunLux = 12000.0f;
        Debug::DebugConfig::envLux = 6000.0f;
        Debug::DebugConfig::testSphereColor = glm::vec3(106.0f / 255.0f,109.0f / 255.0f,111.0f / 255.0f);
        Debug::DebugConfig::testSphereColor2 =  glm::vec3(0.12,0.12,0.12);
        Debug::DebugConfig::demoSphereThreadScale = 7.0f;
        Debug::DebugConfig::demoSphereFuzzScale = 7.0f;
        Debug::DebugConfig::demoSphereThreadStrength = 2.0f;
        Debug::DebugConfig::demoSphereThreadAO = 0.5f;
        SortRenderers();
    }

    void PBRClothDemoScene::Update(float dt)
    {
        Scene::Update(dt);
        float timeValue = glfwGetTime();
    }

    void PBRClothDemoScene::DeleteScene()
    {
        Scene::DeleteScene();
    }

    void PBRClothDemoScene::drawPBRSpheresForward(int x, int y, glm::vec3 origin, Material mat, PBRType materialType, glm::vec4 color)
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
                    pbrShperes[k].GetMainMaterial()->SetVec4("_sheenColor", glm::vec4(Debug::DebugConfig::testSphereColor2, 1));
                    pbrShperes[k].GetMainMaterial()->SetInt("_useSSS", Debug::DebugConfig::useSSS);
                    pbrShperes[k].GetMainMaterial()->SetVec4("_sssColor", glm::vec4(Debug::DebugConfig::testSphereColor3, 1));
                    pbrShperes[k].GetMainMaterial()->SetFloat("_UseParallaxMapping", 0);
                    pbrShperes[k].GetMainMaterial()->SetInt("_parallaxSteps", 1);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_heightScale", 1);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_normalStrength", 0.0f);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_roughness", ((float)j) / (float)x);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_metallic",  0.0f);
                    if(materialType == PBRType::ClearCoat)
                    {
                        pbrShperes[k].GetMainMaterial()->SetInt("_isClearCoat", 1);
                        pbrShperes[k].GetMainMaterial()->SetFloat("_clearCoat", 1.0f - ((float)j + 0.0f) / (float)x);
                        pbrShperes[k].GetMainMaterial()->SetFloat("_clearCoatRoughness", (1.0f - (float)i / (float)y));
                    }

                    if(materialType == PBRType::Aniso)
                    {
                        //pbrShperes[k].GetMainMaterial()->SetFloat("_roughness", 1.0f - ((float)j) / (float)x);
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

