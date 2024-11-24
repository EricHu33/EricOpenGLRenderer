#include "PBRFurnanceTestScene.h"

#include <random>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "../Classes/PhotmetryLighting.h"
#include "../RenderUtil.h"

namespace EricScene
{
    void PBRFurnanceTestScene::CreateScene()
    {
        Scene::CreateScene();
        _MeshModelLoaderMap["sphere"] = std::make_shared<ModelLoader>((dirStr + "resources/models/uv_sphere.fbx").c_str());
       
        std::mt19937 rng(std::time(nullptr));
        std::uniform_real_distribution<float> distribution(0.2f, 1.0f);
        
        for(unsigned int i = 0; i < 1; i++)
        {
            PointLight p;
            p.Color = glm::vec4(0.4 * distribution(rng), 0.6 *distribution(rng), 0.5 * distribution(rng), 0);
            p.Position = glm::vec4(- 2.5 + i * 1.3f, 0.25f, -6.2f, 0);
            p.Position.w = 5.0f;
            p.Color.w = PhotometryLighting::lmToCandela(2400);
           // _PointLights.push_back(p);
        }

        Material debugLightCubeMat(&unlitLightShader);
        auto debugLightRenderers = Util::RenderUtil::CreateRenderers(*_MeshModelLoaderMap["lightVolume"], debugLightCubeMat);
        
        Material standardPBRMat(&geometryPassDeferredShader);
        std::vector<Renderer> pbrShperes = Util::RenderUtil::CreateRenderers( *_MeshModelLoaderMap["sphere"], standardPBRMat);
        
        for(int i = 0; i < 1; i++)
        {
            for(int j = 0; j < 11; j++)
            {
                glm::mat4 obj2world;
                obj2world = glm::mat4(1);
                obj2world = glm::translate(obj2world,  glm::vec3( j * 0.75f, -i * 0.75f, 0)); // translate it down so it's at the center of the scene
                obj2world = glm::rotate(obj2world, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                obj2world = glm::scale(obj2world, glm::vec3(0.25f));
                for(int k = 0; k < pbrShperes.size(); k++)
                {
                    pbrShperes[k].GetMainMaterial()->SetFloat("_UseParallaxMapping", 0);
                    pbrShperes[k].GetMainMaterial()->SetInt("_parallaxSteps", 1);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_heightScale", 1);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_normalStrength", 0.0f);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_roughness", ((float)j) / (float)11);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_metallic",  (1.0f));
                    pbrShperes[k].GetMainMaterial()->SetFloat("_uvScale", 1.0f);
                    pbrShperes[k].GetMainMaterial()->SetInt("_useColorOnly", 1.0f);
                    pbrShperes[k].GetMainMaterial()->SetVec4("_baseColor", glm::vec4(1,1,1,1));
                    pbrShperes[k].GetMainMaterial()->SetInt("_useOcclusionRoughMetalMap", 0);
                }
                auto obj0 = AddSceneObject(pbrShperes, obj2world);
                _AllSceneObjects.push_back(obj0);
            }
        }

        for(int i = 0; i < 1; i++)
        {
            for(int j = 0; j < 11; j++)
            {
                glm::mat4 obj2world;
                obj2world = glm::mat4(1);
                obj2world = glm::translate(obj2world,  glm::vec3( j * 0.75f, 1 -i * 0.75f, 0)); // translate it down so it's at the center of the scene
                obj2world = glm::rotate(obj2world, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                obj2world = glm::scale(obj2world, glm::vec3(0.25f));
                for(int k = 0; k < pbrShperes.size(); k++)
                {
                    pbrShperes[k].GetMainMaterial()->SetFloat("_UseParallaxMapping", 0);
                    pbrShperes[k].GetMainMaterial()->SetInt("_parallaxSteps", 1);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_heightScale", 1);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_normalStrength", 0.0f);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_roughness", ((float)j) / (float)11);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_metallic",  (0.0f));
                    pbrShperes[k].GetMainMaterial()->SetFloat("_uvScale", 1.0f);
                    pbrShperes[k].GetMainMaterial()->SetInt("_useColorOnly", 1.0f);
                    pbrShperes[k].GetMainMaterial()->SetVec4("_baseColor", glm::vec4(1,1,1,1));
                    pbrShperes[k].GetMainMaterial()->SetInt("_useOcclusionRoughMetalMap", 0);
                }
                auto obj0 = AddSceneObject(pbrShperes, obj2world);
                _AllSceneObjects.push_back(obj0);
            }
        }
        
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
        //0.57
        // 0 12 52
        //0 114 255
        //Material sphereCloth(&litClothForwardShader);
        //drawPBRSpheresForward(10,1, glm::vec3(-2.5, 2.0f, -5.0f), sphereCloth, PBRType::Cloth,  glm::vec4(0.5,0.0f,0.0f, 1));
        
    }

    void PBRFurnanceTestScene::Update(float dt)
    {
        Scene::Update(dt);
        float timeValue = glfwGetTime();
      
    }

    void PBRFurnanceTestScene::DeleteScene()
    {
        Scene::DeleteScene();
    }
}

