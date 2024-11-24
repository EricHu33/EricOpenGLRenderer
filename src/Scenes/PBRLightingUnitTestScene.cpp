#include "PBRLightingUnitTestScene.h"
#include <random>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../Classes/PhotmetryLighting.h"
#include "../RenderUtil.h"


namespace EricScene
{
    glm::quat PBRLightingUnitTestScene::rotate_angle_axis(float angle, glm::vec3 axis)
    {
        float sn = sin(angle * 0.5);
        float cs = cos(angle * 0.5);
        return glm::quat(axis.x * sn, axis.y * sn, axis.z * sn, cs);
    }
    
    void PBRLightingUnitTestScene::CreateScene()
    {
        Scene::CreateScene();
        _MeshModelLoaderMap["sphere"] = std::make_shared<ModelLoader>((dirStr + "resources/models/uv_sphere.fbx").c_str());
        _MeshModelLoaderMap["plane"] = std::make_shared<ModelLoader>((dirStr + "resources/models/plane.fbx").c_str());
        std::mt19937 rng(std::time(nullptr));
        std::uniform_real_distribution<float> distribution(0.2f, 1.0f);
        
        Material sphereMat(&geometryPassDeferredShader);
        auto pbrShperes = Util::RenderUtil::CreateRenderers( *_MeshModelLoaderMap["sphere"], sphereMat);
        
        for(int i = 0; i < 6; i++)
        {
            for(int j = 0; j < 6; j++)
            {
                glm::mat4 obj2world;
                obj2world = glm::mat4(1);
                obj2world = glm::translate(obj2world, glm::vec3( -2.5 + j*0.75f, 0, -3 - i*0.7)); // translate it down so it's at the center of the scene
                obj2world = glm::rotate(obj2world, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                obj2world = glm::scale(obj2world, glm::vec3(0.25f));
                auto index = i * 6 + j;
                for(int k = 0; k < pbrShperes.size(); k++)
                {
                    pbrShperes[k].GetMainMaterial()->SetFloat("_UseParallaxMapping", 0);
                    pbrShperes[k].GetMainMaterial()->SetInt("_parallaxSteps", 1);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_heightScale", 1);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_normalStrength", 0.0f);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_roughness", ((float)j) / 6.0f);
                    pbrShperes[k].GetMainMaterial()->SetFloat("_metallic",  (1.0f - (float)i / 6.0f));
            
                    pbrShperes[k].GetMainMaterial()->SetFloat("_uvScale", 1.0f);
                    pbrShperes[k].GetMainMaterial()->SetInt("_useColorOnly", 1.0f);
                    pbrShperes[k].GetMainMaterial()->SetVec4("_baseColor", glm::vec4(0.18,0.18,0.18, 1));
                    pbrShperes[k].GetMainMaterial()->SetInt("_useOcclusionRoughMetalMap", 0);
                }
                auto obj0 = AddSceneObject(pbrShperes, obj2world);
                _AllSceneObjects.push_back(obj0);
            }
        }
        
        glm::mat4 plane_object2World = glm::mat4(1);
        plane_object2World = glm::translate(plane_object2World, glm::vec3(0.0f, -0.25f, 0.0f)); // translate it down so it's at the center of the scene
        plane_object2World = glm::rotate(plane_object2World, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        plane_object2World = glm::scale(plane_object2World, glm::vec3(15));

        Material planeMat(&geometryPassDeferredShader);
        auto planeRenderers = Util::RenderUtil::CreateRenderers(*_MeshModelLoaderMap["plane"], planeMat);
        Texture planeBaseMap, planeNormalMap;
        planeBaseMap.id = Util::RenderUtil::LoadTexture((dirStr + "resources/models/plane_baseMap.jpg").c_str());
        planeBaseMap.Path = "base.png";
        planeBaseMap.samplerName = "_baseMap";

        planeNormalMap.id = Util::RenderUtil::LoadTexture((dirStr + "resources/models/plane_normalMap.jpg").c_str());
        planeNormalMap.Path = "normalMap.jpg";
        planeNormalMap.samplerName = "_normalMap";

        Texture planeHeightMap;
        planeHeightMap.id = Util::RenderUtil::LoadTexture((dirStr + "resources/models/parallax_mapping_height_map3.jpg").c_str());
        planeHeightMap.Path = "plane_heightMap.jpg";
        planeHeightMap.samplerName = "_heightMap";
        
        for(int i =0;i < planeRenderers.size();i++){

           planeRenderers[i].GetMainMaterial()->SetTexture(planeBaseMap.samplerName, planeBaseMap.id);
           planeRenderers[i].GetMainMaterial()->SetTexture(planeNormalMap.samplerName, planeNormalMap.id);
           planeRenderers[i].GetMainMaterial()->SetTexture(planeHeightMap.samplerName, planeHeightMap.id);
           planeRenderers[i].GetMainMaterial()->SetFloat("_UseParallaxMapping", 0.0f);
           planeRenderers[i].GetMainMaterial()->SetInt("_parallaxSteps", 32);
           planeRenderers[i].GetMainMaterial()->SetFloat("_heightScale", 0.0f);
           planeRenderers[i].GetMainMaterial()->SetFloat("_normalStrength", 0.0f);
           planeRenderers[i].GetMainMaterial()->SetFloat("_uvScale", 4.0f);
           planeRenderers[i].GetMainMaterial()->SetFloat("_roughness", 0.4f);
           planeRenderers[i].GetMainMaterial()->SetFloat("_metallic", 0.0f);
           planeRenderers[i].GetMainMaterial()->SetInt("_useColorOnly", 1.0f);
           planeRenderers[i].GetMainMaterial()->SetVec4("_baseColor", glm::vec4(0.3,0.3,0.3, 1));
       }
        auto obj0 = AddSceneObject(pbrShperes, plane_object2World);
        _AllSceneObjects.push_back(obj0);

        for(unsigned int i = 0; i < 3; i++)
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

        Material unlitCubeMat(&unlitLightShader);
        auto cubeRenderer = Util::RenderUtil::CreateRenderers(*_MeshModelLoaderMap["cube"], unlitCubeMat);
        glm::mat4 cubeObject2World = glm::mat4(1);
        cubeObject2World = glm::translate(cubeObject2World, directionalLightPosition);
        cubeObject2World = glm::scale(cubeObject2World, glm::vec3(0.2f));
        auto cubeObj = AddSceneObject(cubeRenderer, cubeObject2World);
        _AllSceneObjects.push_back(cubeObj);
    }

    void PBRLightingUnitTestScene::Update(float dt)
    {
        Scene::Update(dt);
        float timeValue = glfwGetTime();
        
        testSphereColor = glm::vec4(Debug::DebugConfig::testSphereColor, 1);
        pointLightFalloffRadius = Debug::DebugConfig::pointLightRadiusFalloff;
        directionalLightPosition = glm::vec3(0,3,-2.5);
        directionalLightEuler = Debug::DebugConfig::directionalLightEuler;

        glm::mat4 cubeObject2World = glm::mat4(1);
        cubeObject2World = glm::translate(cubeObject2World, directionalLightPosition);
        cubeObject2World = glm::rotate(cubeObject2World, glm::radians(directionalLightEuler.x), glm::vec3(1.0f, 0.0f, 0.0f));
        cubeObject2World = glm::rotate(cubeObject2World, glm::radians(directionalLightEuler.y), glm::vec3(0.0f, 1.0f, 0.0f));
        cubeObject2World = glm::rotate(cubeObject2World, glm::radians(directionalLightEuler.z), glm::vec3(0.0f, 0.0f, 1.0f));
        cubeObject2World = glm::scale(cubeObject2World, glm::vec3(0.2f));
        auto lightQuat = rotate_angle_axis(directionalLightEuler.x * (3.141592653589793f / 180.0f), glm::vec3(1,0,0));
        lightQuat *= rotate_angle_axis(directionalLightEuler.y * (3.141592653589793f / 180.0f), glm::vec3(0,1,0));
        lightQuat *= rotate_angle_axis(directionalLightEuler.z * (3.141592653589793f / 180.0f), glm::vec3(0,0,1));
        _DirectionalLight.Direction = lightQuat * glm::vec3(0,0,-1);
        for(int i = 0; i < directionlLightCubeIndex.size();i++)
        {
           // _ForwardOpaqueSceneObjects[directionlLightCubeIndex[i]].transform = cubeObject2World;
        }
        for(int p = 0; p < std::size(_PointLights);p++)
        {
            _PointLights[p].Position.w = pointLightFalloffRadius;
        }

        for (int i = 0; i < testSpheres.size(); i++)
        {
            testSpheres[i]->GetMainMaterial()->SetVec4("_baseColor", testSphereColor);
        }
    }

    void PBRLightingUnitTestScene::DeleteScene()
    {
        Scene::DeleteScene();
    }
}

