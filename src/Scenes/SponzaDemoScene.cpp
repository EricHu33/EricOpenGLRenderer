#include "SponzaDemoScene.h"

#include <random>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

#include "../RenderUtil.h"
#include "../Classes/PhotmetryLighting.h"

namespace EricScene
{
    void SponzaDemoScene::CreateScene()
    {
        Scene::CreateScene();
        _MeshModelLoaderMap["sponza"] = std::make_shared<ModelLoader>((dirStr + "resources/models/Sponza.gltf").c_str());
        
        std::mt19937 rng(std::time(nullptr));
        std::uniform_real_distribution<float> distribution(0.2f, 1.0f);
        std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
        std::default_random_engine generator;
        for(unsigned int i = 0; i < 300; i++)
        {
            PointLight p;
            p.Color = glm::vec4(0.4 * distribution(rng), 0.6 *distribution(rng), 0.5 * distribution(rng), 0);
            auto randX = 8 *  (2 * randomFloats(generator) - 1) - 8;
            auto randY = 1 + (randomFloats(generator))* 0.4f;
            auto randZ = 6 *  (2 * randomFloats(generator) - 1) - 6;
            p.Position = glm::vec4(randX, randY, randZ, 0);
            p.Position.w = 3.0f;
            p.Color.w = PhotometryLighting::lmToCandela(8000);
            _PointLights.push_back(p);
        }

        Material debugLightCubeMat(&unlitLightShader);
        auto debugLightRenderers = Util::RenderUtil::CreateRenderers(*_MeshModelLoaderMap["lightVolume"], debugLightCubeMat);

        Material sponzaMat(&litForwardShader);
        sponzaMat.SetFloat("_UseParallaxMapping", 0);
        sponzaMat.SetInt("_parallaxSteps", 0);
        sponzaMat.SetFloat("_heightScale", 0);
        sponzaMat.SetFloat("_normalStrength", 0);
        sponzaMat.SetInt("_useOcclusionRoughMetalMap", 1);
        sponzaMat.SetFloat("_roughness", 1);
        sponzaMat.SetFloat("_metallic", 0);
        sponzaMat.SetInt("_isClearCoat", 0);
    
        sponzaMat.SetFloat("_uvScale", 1.0f);
        sponzaMat.SetInt("_useColorOnly", 0.0f);
        sponzaMat.SetVec4("_baseColor", glm::vec4(1,1,1,1));

        glm::mat4 sponzaTransform = glm::mat4(1);
        sponzaTransform = glm::translate(sponzaTransform, glm::vec3(-10.0f, 1.0f, -4.5)); // translate it down so it's at the center of the scene
        sponzaTransform = glm::rotate(sponzaTransform, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        sponzaTransform = glm::scale(sponzaTransform, glm::vec3(0.01f));

        auto sponzaRenederers = Util::RenderUtil::CreateRenderers( *_MeshModelLoaderMap["sponza"], sponzaMat);
        auto skinObj0 = AddSceneObject(sponzaRenederers, sponzaTransform);
        _AllSceneObjects.push_back(skinObj0);
        SortRenderers();
        Debug::DebugConfig::directionalLightEuler = glm::vec3(100,97,190);

        Debug::DebugConfig::envLux = 0.0f;
    }

    void SponzaDemoScene::Update(float dt)
    {
        Scene::Update(dt);
        _DirectionalLight.Color = glm::vec3(0.5,0.5,0.5);
        auto lightQuat = rotate_angle_axis(Debug::DebugConfig::directionalLightEuler.x * (3.141592653589793f / 180.0f), glm::vec3(1,0,0));
        lightQuat *= rotate_angle_axis(Debug::DebugConfig::directionalLightEuler.y * (3.141592653589793f / 180.0f), glm::vec3(0,1,0));
        lightQuat *= rotate_angle_axis(Debug::DebugConfig::directionalLightEuler.z * (3.141592653589793f / 180.0f), glm::vec3(0,0,1));
        _DirectionalLight.Direction = lightQuat * glm::vec3(0,0,-1);
        float timeValue = glfwGetTime();
        for (unsigned int i = 0; i < std::size(_PointLights); i++)
        {
            auto pos = _PointLights[i].Position;
            _PointLights[i].Position = glm::vec4(_PointLights[i].Position.x + sin(i + timeValue)*0.01f, _PointLights[i].Position.y + cos(i + timeValue)*0.01f, _PointLights[i].Position.z, 0);
            _PointLights[i].Position.w = 1.0f;
        }
    }

    void SponzaDemoScene::DeleteScene()
    {
        Scene::DeleteScene();
    }
}

