#include "Scene.h"

#include <iostream>
#include <random>
#include <Shlwapi.h>
#include <tchar.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <stb/stb_image.h>

#include "DebugConfig.h"
#include "imgui.h"
#include "RenderUtil.h"
#include "RenderPasses/CubeMapIBLRenderPass.h"

void EricScene::Scene::CreateScene()
{
    TCHAR buffer[2048];
    GetModuleFileName ( NULL, buffer, sizeof(buffer));
    std::wcout << "path = " << buffer << std::endl;
    TCHAR directory[2048];
    _tcscpy_s(directory, 2048, buffer);
    PathRemoveFileSpec(directory);
    std::wstring wstr(directory);
    dirStr =  std::string(wstr.begin(), wstr.end());
    dirStr = dirStr + "\\..\\..\\";

    litForwardShader.create(dirStr.c_str(), "resources/shaders/LitForward.vert", "resources/shaders/LitForward.frag");
    _AllShaders.push_back(&litForwardShader);
    unlitLightShader.create(dirStr.c_str(), "resources/shaders/Unlit.vert", "resources/shaders/Unlit.frag");
    _AllShaders.push_back(&unlitLightShader);
    antiAliasingShader.create(dirStr.c_str(), "resources/shaders/ScreenQuad.vert", "resources/shaders/Fxaa.frag");
    _AllShaders.push_back(&antiAliasingShader);
    toneMappingShader.create(dirStr.c_str(), "resources/shaders/ScreenQuad.vert", "resources/shaders/ToneMapping.frag");
    _AllShaders.push_back(&toneMappingShader);
    blitShader.create(dirStr.c_str(), "resources/shaders/ScreenQuad.vert", "resources/shaders/Blit.frag");
    _AllShaders.push_back(&blitShader);
    debugTextureShader.create(dirStr.c_str(), "resources/shaders/ScreenQuad.vert", "resources/shaders/DebugTexture.frag");
    _AllShaders.push_back(&debugTextureShader);
    debugTextureArrayShader.create(dirStr.c_str(), "resources/shaders/ScreenQuad.vert", "resources/shaders/DebugTextureArray.frag");
    _AllShaders.push_back(&debugTextureArrayShader);
    ssaoShader.create(dirStr.c_str(), "resources/shaders/ScreenQuad.vert", "resources/shaders/SSAO.frag");
    _AllShaders.push_back(&ssaoShader);
    ssaoBlurShader.create(dirStr.c_str(), "resources/shaders/ScreenQuad.vert", "resources/shaders/SSAOBlur.frag");
    _AllShaders.push_back(&ssaoBlurShader);
    hbaoShader.create(dirStr.c_str(), "resources/shaders/ScreenQuad.vert", "resources/shaders/HBAO.frag");
    _AllShaders.push_back(&hbaoShader);
    hbaoBlurShader.create(dirStr.c_str(), "resources/shaders/ScreenQuad.vert", "resources/shaders/Bilateralblur.frag");
    _AllShaders.push_back(&hbaoBlurShader);
    skyboxShader.create(dirStr.c_str(), "resources/shaders/Skybox.vert", "resources/shaders/Skybox.frag");
    _AllShaders.push_back(&skyboxShader);
    shadowDepthShader.create(dirStr.c_str(), "resources/shaders/SimpleShadowDepth.vert", "resources/shaders/SimpleDepth.frag", false);
    _AllShaders.push_back(&shadowDepthShader);
    rsmDepthShader.create(dirStr.c_str(), "resources/shaders/SimpleRSMDepth.vert", "resources/shaders/SimpleDepth.frag", false);
    _AllShaders.push_back(&rsmDepthShader);
    rsmColorShader.create(dirStr.c_str(), "resources/shaders/SimpleRSMColor.vert", "resources/shaders/SimpleRSMColor.frag", false);
    _AllShaders.push_back(&rsmColorShader);
    depthPrepassShader.create(dirStr.c_str(), "resources/shaders/SimpleCameraDepth.vert", "resources/shaders/SimpleDepth.frag", false);
    _AllShaders.push_back(&depthPrepassShader);
    indirectLightingShader.create(dirStr.c_str(), "resources/shaders/ScreenQuad.vert", "resources/shaders/indirectLighting.frag");
    _AllShaders.push_back(&indirectLightingShader);
    geometryPassDeferredShader.create(dirStr.c_str(), "resources/shaders/GeometryPassDeferred.vert", "resources/shaders/GeometryPassDeferred.frag");
    _AllShaders.push_back(&geometryPassDeferredShader);
    punctualLightDeferredShader.create(dirStr.c_str(), "resources/shaders/LightPassDeferred.vert", "resources/shaders/PunctualLightDeferred.frag");
    _AllShaders.push_back(&punctualLightDeferredShader);
    directionalLightDeferredShader.create(dirStr.c_str(), "resources/shaders/DirectionalLightPassDeferred.vert", "resources/shaders/DirectionalDeferred.frag");
    _AllShaders.push_back(&directionalLightDeferredShader);
    envCubeShader.create(dirStr.c_str(), "resources/shaders/EnvCube.vert", "resources/shaders/EnvCube.frag");
    _AllShaders.push_back(&envCubeShader);
    diffuseIrradianceShader.create(dirStr.c_str(), "resources/shaders/DiffuseIrradiance.vert", "resources/shaders/DiffuseIrradiance.frag");
    _AllShaders.push_back(&diffuseIrradianceShader);
    prefilterMapShader.create(dirStr.c_str(), "resources/shaders/DiffuseIrradiance.vert", "resources/shaders/PrefilterMap.frag");
    _AllShaders.push_back(&prefilterMapShader);
    brdfLutShader.create(dirStr.c_str(), "resources/shaders/ScreenQuad.vert", "resources/shaders/BrdfLUT.frag");
    _AllShaders.push_back(&brdfLutShader);
    litAnisoForwardShader.create(dirStr.c_str(), "resources/shaders/LitForward.vert", "resources/shaders/LitAnisoForward.frag");
    _AllShaders.push_back(&litAnisoForwardShader);
    litClothForwardShader.create(dirStr.c_str(), "resources/shaders/LitForward.vert", "resources/shaders/LitClothForward.frag");
    _AllShaders.push_back(&litClothForwardShader);
    preIntegratedSSSShader.create(dirStr.c_str(), "resources/shaders/ScreenQuad.vert", "resources/shaders/preIntegratedSSS.frag");
    _AllShaders.push_back(&preIntegratedSSSShader);
    litSkinForwardShader.create(dirStr.c_str(), "resources/shaders/LitForward.vert", "resources/shaders/LitSkinForward.frag");
    _AllShaders.push_back(&litSkinForwardShader);
    forwardLightDebugShader.create(dirStr.c_str(), "resources/shaders/ScreenQuad.vert", "resources/shaders/light_debug.frag");
    _AllShaders.push_back(&forwardLightDebugShader);
    
    _FullScreenVAO = Util::RenderUtil::CreateQuad4();
    _MeshModelLoaderMap["lightVolume"] = std::make_shared<ModelLoader>((dirStr + "resources/models/LightSphere.fbx").c_str());
    _MeshModelLoaderMap["cube"] = std::make_shared<ModelLoader>((dirStr + "resources/models/cube.fbx").c_str());
    //MeshModelLoaderMap["sphere"] = std::make_shared<ModelLoader>((dirStr + "resources/models/uv_sphere.fbx").c_str());
    //_MeshModelLoaderMap["helmet"] = std::make_shared<ModelLoader>((dirStr + "resources/models/FlightHelmet.gltf").c_str());
    //_MeshModelLoaderMap["d_helmet"] = std::make_shared<ModelLoader>((dirStr + "resources/models/DamagedHelmet.gltf").c_str());
    //_MeshModelLoaderMap["sponza"] = std::make_shared<ModelLoader>((dirStr + "resources/models/Sponza.gltf").c_str());
    //_MeshModelLoaderMap["plane"] = std::make_shared<ModelLoader>((dirStr + "resources/models/plane.fbx").c_str());

    Material lightSphereMat(&punctualLightDeferredShader);
    auto mesh = _MeshModelLoaderMap["lightVolume"]->GetMeshes();
    this->lightSphereRenderer = std::make_shared<Renderer>( &((*mesh)[0]), lightSphereMat);

    Material unlitDebugSphereMat(&unlitLightShader);
    auto debugMesh = _MeshModelLoaderMap["cube"]->GetMeshes();
    this->DebugLightSourceRenderer = std::make_shared<Renderer>( &((*debugMesh)[0]), unlitDebugSphereMat);
    
    _DirectionalLight.Color = glm::vec3(0.2f);
    _DirectionalLight.Ambient = glm::vec3(0.0f,0.0f,0.0f);
    _DirectionalLight.Direction =  glm::normalize(glm::vec3(2,2,3));

    hdrTexture = Util::RenderUtil::LoadHDRTexture(dirStr, "resources/textures/newport_loft.hdr");
    envCubeMap = RenderUtil::CubeMapIBLRenderPass::CreateCubeMapFromHDRTexture(envCubeShader, hdrTexture);
   // irradianceMap = RenderUtil::CubeMapIBLRenderPass::ComputeIrradianceMap(diffuseIrradianceShader, envCubeMap);
    SceneSH9Color = RenderUtil::CubeMapIBLRenderPass::ProjectCubemapToSH(envCubeMap, 512);
    //CubeMapIBLRenderPass::AppliedSH9ResultToCubeMap(myScene.envCubeMap, sh9, 512);
    preFilterMap = RenderUtil::CubeMapIBLRenderPass::ComputePrefilterMap(prefilterMapShader, envCubeMap);
    brdfLutTexture = RenderUtil::CubeMapIBLRenderPass::PrecomputeDFG(brdfLutShader, _FullScreenVAO);
    preIntegratedSSSTexture = RenderUtil::CubeMapIBLRenderPass::PrecomputeDFG(preIntegratedSSSShader, _FullScreenVAO);
   // Debug::DebugConfig::debugTextureId = preIntegratedSSSTexture;

    //Util::RenderUtil::SaveTextureToPNG(dirStr, "sss_lut.png", preIntegratedSSSTexture, 512,512);
    
}

std::shared_ptr<SceneObject> EricScene::Scene::AddSceneObject()
{
    auto obj = std::make_shared<SceneObject>(SceneObject());
    EricScene::Scene::_ObjectsMap[obj->uuid] = obj;
    obj->forceUpdateSelfAndChild();
    return obj;
}

std::shared_ptr<SceneObject> EricScene::Scene::AddSceneObject(std::vector<Renderer> renderers, glm::mat4 transform)
{
    auto obj = std::make_shared<SceneObject>(SceneObject());
    EricScene::Scene::_ObjectsMap[obj->uuid] = obj;
    obj->Init(renderers, transform);
    auto childs = obj->GetChilds();
    for(int i = 0 ;i < childs.size();i++)
    {
        EricScene::Scene::_ObjectsMap[childs[i]->uuid] = childs[i];
    }
    obj->forceUpdateSelfAndChild();
    return obj;
}

void EricScene::Scene::Update(float dt)
{
    float timeValue = glfwGetTime();
}

void EricScene::Scene::OnGUI()
{
    
}

void EricScene::Scene::SortRenderer(std::shared_ptr<SceneObject> obj)
{
    auto shader = obj->renderer->GetMainMaterial()->GetShader();
    if(obj->renderer->IsTransparent)
    {
        obj->renderer->GetMainMaterial()->SetShader(&litForwardShader);
        _ForwardTransparentSceneObjects.push_back(obj);
    }
    else if(shader == &litForwardShader
        || shader == &litAnisoForwardShader
        || shader == &litClothForwardShader
        || shader == &litSkinForwardShader
        || shader == &unlitLightShader)
    {
        _ForwardOpaqueSceneObjects.push_back(obj);
        _DepthCasters.push_back(obj);
    }
    else
    {
        _DeferredSceneObjects.push_back(obj);
        _DepthCasters.push_back(obj);
    }
}


void EricScene::Scene::SortRenderers()
{
    for(int i = 0; i < _AllSceneObjects.size();i++)
    {
        auto obj = _AllSceneObjects[i];
        auto allChilds = obj->GetChilds();
        for (auto child : allChilds)
        {
            SortRenderer(child);
        }
        SortRenderer(obj);
    }
}



void EricScene::Scene::DeleteScene()
{
    for (Shader* shader : _AllShaders) {
        shader->Dispose();
    }

    for (auto it = _MeshModelLoaderMap.begin(); it != _MeshModelLoaderMap.end(); ++it) {
        it->second->Dispose();
    }
    glDeleteVertexArrays(1, &_FullScreenVAO);
}
