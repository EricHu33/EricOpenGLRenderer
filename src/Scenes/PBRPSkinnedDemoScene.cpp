#include "PBRPSkinnedDemoScene.h"

#include <random>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "../Classes/PhotmetryLighting.h"
#include "../RenderUtil.h"
#include <glm/gtc/quaternion.hpp>

namespace EricScene
{
    glm::quat PBRPSkinnedDemoScene::rotate_angle_axis(float angle, glm::vec3 axis)
    {
        float sn = sin(angle * 0.5);
        float cs = cos(angle * 0.5);
        return glm::quat(axis.x * sn, axis.y * sn, axis.z * sn, cs);
    }
    
    void PBRPSkinnedDemoScene::CreateScene()
    {
        Scene::CreateScene();
       // _MeshModelLoaderMap["helmet"] = std::make_shared<ModelLoader>((dirStr + "resources/models/FlightHelmet.gltf").c_str());
      //  _MeshModelLoaderMap["d_helmet"] = std::make_shared<ModelLoader>((dirStr + "resources/models/DamagedHelmet.gltf").c_str());
        _MeshModelLoaderMap["plane"] = std::make_shared<ModelLoader>((dirStr + "resources/models/plane.fbx").c_str());
        _MeshModelLoaderMap["orc"] = std::make_shared<ModelLoader>((dirStr + "resources/models/X_Bot.fbx").c_str());
    
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
        
      /*  glm::mat4 helmet_object2World = glm::mat4(1);
        helmet_object2World = glm::translate(helmet_object2World, glm::vec3(-1.5f, 1.0f, -3.0)); // translate it down so it's at the center of the scene
        helmet_object2World = glm::rotate(helmet_object2World, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        helmet_object2World = glm::scale(helmet_object2World, glm::vec3(2));*/
       
        Material pbrMat(&geometryPassDeferredShader);
        pbrMat.SetFloat("_UseParallaxMapping", 0);
        pbrMat.SetInt("_parallaxSteps", 1);
        pbrMat.SetFloat("_heightScale", 1);
        pbrMat.SetFloat("_normalStrength", 0.5f);
        pbrMat.SetInt("_useOcclusionRoughMetalMap", 1.0f);
        pbrMat.SetFloat("_roughness", 0);
        pbrMat.SetFloat("_metallic", 0.2);
        
        pbrMat.SetFloat("_uvScale", 1.0f);
        pbrMat.SetInt("_useColorOnly", 1.0f);
        pbrMat.SetInt("_isSkinnedMesh", 1);
        pbrMat.SetVec4("_baseColor", glm::vec4(1,1,1, 1));
        /*auto helemtRenderers = Util::RenderUtil::CreateRenderers( *_MeshModelLoaderMap["helmet"], pbrMat);
        auto helmetObj = std::make_shared<SceneObject>(SceneObject());
        helmetObj->Init(helemtRenderers, helmet_object2World);
        _AllSceneObjects.push_back(helmetObj);*/

        for(int i = 0; i < 5; i++)
        {
            glm::mat4 orcObject2World = glm::mat4(1);
            orcObject2World = glm::translate(orcObject2World, glm::vec3(-3.5f + i * 2, 1.0f, -3.0)); // translate it down so it's at the center of the scene
            orcObject2World = glm::rotate(orcObject2World, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            orcObject2World = glm::rotate(orcObject2World, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            orcObject2World = glm::scale(orcObject2World, glm::vec3(0.01f));

            pbrMat.SetInt("_useOcclusionRoughMetalMap", 0.0f);
            pbrMat.SetTexture("_baseMap",-1);
            pbrMat.SetTexture("_normalMap",-1);
            auto orcRenderers = Util::RenderUtil::CreateRenderers( *_MeshModelLoaderMap["orc"], pbrMat);
            auto orcObj = AddSceneObject(orcRenderers, orcObject2World);
            _AllSceneObjects.push_back(orcObj);
       
            _Animations.push_back(std::make_shared<Animation>((dirStr + "resources/models/X_Bot_Dance.fbx").c_str(), _MeshModelLoaderMap["orc"].get()));
            _Animators.push_back(std::make_shared<Animator>(_Animations[0].get(), orcObj.get(), i * 6.0f));   
        }

        for(int i = 0; i < 5; i++)
        {
            glm::mat4 orcObject2World = glm::mat4(1);
            orcObject2World = glm::translate(orcObject2World, glm::vec3(-3.5f + i * 2, 1.0f, -6.0)); // translate it down so it's at the center of the scene
            orcObject2World = glm::rotate(orcObject2World, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            orcObject2World = glm::rotate(orcObject2World, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            orcObject2World = glm::scale(orcObject2World, glm::vec3(0.01f));

            pbrMat.SetInt("_useOcclusionRoughMetalMap", 0.0f);
            pbrMat.SetTexture("_baseMap",-1);
            pbrMat.SetTexture("_normalMap",-1);
            auto orcRenderers = Util::RenderUtil::CreateRenderers( *_MeshModelLoaderMap["orc"], pbrMat);
            auto orcObj = AddSceneObject(orcRenderers, orcObject2World);
            _AllSceneObjects.push_back(orcObj);
       
            _Animations.push_back(std::make_shared<Animation>((dirStr + "resources/models/X_Bot_Dance.fbx").c_str(), _MeshModelLoaderMap["orc"].get()));
            _Animators.push_back(std::make_shared<Animator>(_Animations[0].get(), orcObj.get(), i * 9.0f));   
        }
        
      /*  glm::mat4 d_helmet_object2World = glm::mat4(1);
        d_helmet_object2World = glm::translate(d_helmet_object2World, glm::vec3(3.0f, 1.5f, -3.0)); // translate it down so it's at the center of the scene
        d_helmet_object2World = glm::rotate(d_helmet_object2World, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        d_helmet_object2World = glm::scale(d_helmet_object2World, glm::vec3(0.5f));

        auto damagedHelmetRenderer = Util::RenderUtil::CreateRenderers( *_MeshModelLoaderMap["d_helmet"], pbrMat);
        auto d_helmetObj = std::make_shared<SceneObject>(SceneObject());
        d_helmetObj->Init(damagedHelmetRenderer, d_helmet_object2World);
        _AllSceneObjects.push_back(d_helmetObj);*/

        glm::mat4 plane_object2World = glm::mat4(1);
        plane_object2World = glm::translate(plane_object2World, glm::vec3(0.0f, 1.0f, 0.0f)); // translate it down so it's at the center of the scene
        plane_object2World = glm::rotate(plane_object2World, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        plane_object2World = glm::scale(plane_object2World, glm::vec3(15));

        Material planeMat(&geometryPassDeferredShader);
        
        Texture planeBaseMap, planeNormalMap;
        auto planeBaseImgPath = dirStr + "resources/models/plane_baseMap.jpg";
        planeBaseMap.id = Util::RenderUtil::LoadTexture((planeBaseImgPath).c_str());
        planeBaseMap.Path = "base.png";
        planeBaseMap.samplerName = "_baseMap";

        auto planeNormalImgPath = dirStr + "resources/models/plane_normalMap.jpg";
        planeNormalMap.id = Util::RenderUtil::LoadTexture((planeNormalImgPath).c_str());
        planeNormalMap.Path = "normalMap.jpg";
        planeNormalMap.samplerName = "_normalMap";

        Texture planeHeightMap;
        auto planeHeightImgPath = dirStr + "resources/models/parallax_mapping_height_map3.jpg";
        planeHeightMap.id = Util::RenderUtil::LoadTexture((planeHeightImgPath).c_str());
        planeHeightMap.Path = "plane_heightMap.jpg";
        planeHeightMap.samplerName = "_heightMap";

        planeMat.SetTexture(planeBaseMap.samplerName, planeBaseMap.id, planeBaseImgPath.c_str());
        planeMat.SetTexture(planeNormalMap.samplerName, planeNormalMap.id, planeNormalImgPath.c_str());
        planeMat.SetTexture(planeHeightMap.samplerName, planeHeightMap.id, planeHeightImgPath.c_str());
        planeMat.SetInt("_useOcclusionRoughMetalMap", 0.0f);planeMat.SetFloat("_UseParallaxMapping", 0.0f);
        planeMat.SetInt("_parallaxSteps", 32);
        planeMat.SetFloat("_heightScale", 0.0f);
        planeMat.SetFloat("_normalStrength", 0.0f);
        planeMat.SetFloat("_uvScale", 4.0f);
        planeMat.SetFloat("_roughness", 0.4f);
        planeMat.SetFloat("_metallic", 0.0f);
        planeMat.SetInt("_useColorOnly", 0.0f);
        planeMat.SetInt("_isSkinnedMesh", 0);
        planeMat.SetVec4("_baseColor", glm::vec4(0.3,0.3,0.3, 1));
        auto planeRenderers = Util::RenderUtil::CreateRenderers(*_MeshModelLoaderMap["plane"], planeMat);
        auto planeObj = AddSceneObject(planeRenderers, plane_object2World);
        _AllSceneObjects.push_back(planeObj);
        
        Material cubeMat(&litForwardShader);

        glm::mat4  cube_object2World = glm::mat4(1);
        cube_object2World = glm::translate(cube_object2World, glm::vec3(0.0f, 5.0f, 0.0f)); // translate it down so it's at the center of the scene
        cube_object2World = glm::rotate(cube_object2World, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        cube_object2World = glm::scale(cube_object2World, glm::vec3(0.2f, 1.0f, 0.2f));
        
        //cubeMat.SetInt("_useOcclusionRoughMetalMap", 0.0f);
        cubeMat.SetFloat("_UseParallaxMapping", 0.0f);
        cubeMat.SetInt("_parallaxSteps", 32);
        cubeMat.SetFloat("_heightScale", 0.0f);
        cubeMat.SetFloat("_normalStrength", 0.0f);
        cubeMat.SetFloat("_uvScale", 4.0f);
        cubeMat.SetFloat("_roughness", 0.4f);
        cubeMat.SetFloat("_metallic", 0.0f);
        cubeMat.SetInt("_useColorOnly", 1.0f);
        cubeMat.SetVec4("_baseColor", glm::vec4(0.3,0.3,0.3, 1));
        cubeMat.SetInt("_isSkinnedMesh", 0);
        cubeMat.SetInt("_isInstancing", 0);

        auto cubeRenderers = Util::RenderUtil::CreateRenderers(*_MeshModelLoaderMap["cube"], cubeMat);
        auto cubeObj = AddSceneObject(cubeRenderers, cube_object2World);
        _AllSceneObjects.push_back(cubeObj);

        cube_object2World = glm::mat4(1);
        cube_object2World = glm::translate(cube_object2World, glm::vec3(0.0f, 1.0f, 0.0f)); // translate it down so it's at the center of the scene
        cube_object2World = glm::rotate(cube_object2World, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        cube_object2World = glm::scale(cube_object2World, glm::vec3(0.2f, 0.2f, 0.2f));
        
        //auto instancingCubeRenderers = Util::RenderUtil::CreateRenderers(*_MeshModelLoaderMap["cube"], cubeMat);
       //auto instancingCube = AddSceneObject(instancingCubeRenderers, cube_object2World);
       // _ForwardInstancingOpaqueSceneObjects.push_back(instancingCube);
        
        Debug::DebugConfig::envLux = 6000;
        SortRenderers();
    }

    void PBRPSkinnedDemoScene::Update(float dt)
    {
        Scene::Update(dt);
        float timeValue = glfwGetTime();

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
        
        for (unsigned int i = 0; i < std::size(_PointLights); i++)
        {
            auto pos = _PointLights[i].Position;
            _PointLights[i].Position = glm::vec4((i % 10) +  sin(timeValue) - 5, pos.y, -2.2f, 0);
            _PointLights[i].Position.w = Debug::DebugConfig::pointLightRadiusFalloff;
        }
    }

    void PBRPSkinnedDemoScene::DeleteScene()
    {
        Scene::DeleteScene();
    }
}

