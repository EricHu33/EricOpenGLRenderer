#pragma once
#include <map>
#include "Classes/Renderer.h"
#include "Classes/Animation.h"
#include "Classes/ModelLoader.h"
#include "LightDefine.h"
#include "RenderPasses/CubeMapIBLRenderPass.h"
#include <glm/gtx/matrix_decompose.hpp>

#include "Classes/UUID.h"


struct SceneObject : std::enable_shared_from_this<SceneObject>
{
    Util::UUID uuid;
    std::vector<std::shared_ptr<SceneObject>> childs;
    std::shared_ptr<SceneObject> parent = NULL;
    std::shared_ptr<Renderer> renderer;
    Animator* animator = NULL;
    glm::vec3 localScale;
    glm::vec3 localPosition;
    glm::vec3 eulerRot;
    bool hasFetchedParentMatrix = false;
    glm::mat4 modelMatrix = glm::mat4(1);
    bool isDirty = false;

    SceneObject()
    {
    }
    
    SceneObject(Renderer renderer, glm::mat4 transform)
    {
        localScale = glm::vec3(1);
        localPosition = glm::vec3(1);
        eulerRot = glm::vec3(0);
        this->renderer = std::make_shared<Renderer>(renderer);
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(transform, scale, rotation, translation, skew,perspective);
        this->localPosition = translation;
        this->localScale = scale;
        this->eulerRot = glm::eulerAngles(rotation);
    }

    void Init(std::vector<Renderer> renderers, glm::mat4 transform)
    {
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(transform, scale, rotation, translation, skew,perspective);
        rotation = glm::conjugate(rotation);
        this->localPosition = translation;
        this->localScale = scale;
        this->eulerRot = glm::eulerAngles(rotation) * 180.0f / 3.1515926f;
        
        parent = NULL;
        renderer = std::make_shared<Renderer>(renderers[0]);
        for(int i = 1; i < renderers.size();i++)
        {
            AddChild(SceneObject(renderers[i], glm::mat4(1)));
        }
    }
    
    void AddChild(const SceneObject& obj)
    {
        auto child = std::make_shared<SceneObject>(obj);
        child->parent = shared_from_this();
        childs.push_back(child);
    }

    std::vector<std::shared_ptr<SceneObject>> GetChilds()
    {
        std::vector<std::shared_ptr<SceneObject>> nodes;
        if(!childs.empty())
        {
            nodes.insert(nodes.end(), childs.begin(), childs.end());
        }
        for(size_t i = 0 ; i < childs.size();++i)
        {
            auto childNodes = childs[static_cast<int>(i)]->GetChilds();
            nodes.insert(nodes.end(), childNodes.begin(), childNodes.end());
        }
        return nodes;
    }

    glm::mat4 GetLocalModelMatrix()
    {
        const glm::mat4 transformX = glm::rotate(glm::mat4(1.0f),
                             glm::radians(eulerRot.x),
                             glm::vec3(1.0f, 0.0f, 0.0f));
        const glm::mat4 transformY = glm::rotate(glm::mat4(1.0f),
                             glm::radians(eulerRot.y),
                             glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::mat4 transformZ = glm::rotate(glm::mat4(1.0f),
                             glm::radians(eulerRot.z),
                             glm::vec3(0.0f, 0.0f, 1.0f));

        // Y * X * Z
         glm::mat4 roationMatrix = transformY * transformX * transformZ;

        // translation * rotation * scale (also know as TRS matrix)
        return glm::translate(glm::mat4(1.0f), localPosition) *
                    roationMatrix *
                    glm::scale(glm::mat4(1.0f), localScale);
    }

    glm::mat4 computeModelMatrix()
    {
        isDirty = false;
        return GetLocalModelMatrix();
    }

    glm::mat4 computeModelMatrix(const glm::mat4& parentGlobalModelMatrix)
    {
        isDirty = false;
        return parentGlobalModelMatrix * GetLocalModelMatrix();
    }

    void forceUpdateSelfAndChild()
    {
        if (parent != NULL)
            modelMatrix = computeModelMatrix(parent->GetModelMatrix());
        else
            modelMatrix = computeModelMatrix();

        for (auto&& child : childs)
        {
            child->forceUpdateSelfAndChild();
        }
    }

    void forceUpdateSelfAndChildNoParent()
    {
        modelMatrix = computeModelMatrix();

        for (auto&& child : childs)
        {
            child->forceUpdateSelfAndChild();
        }
    }

    glm::mat4 GetModelMatrix()
    {
        return modelMatrix;
    }

    void SetLocalMatrix(glm::mat4 transform)
    {
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(transform, scale, rotation, translation, skew,perspective);
        bool isNan = glm::all(glm::isnan(translation));
        if(this->localPosition != translation && !isNan)
        {
            this->localPosition = translation;
            this->localScale = scale;
            this->eulerRot = glm::eulerAngles(rotation);
            forceUpdateSelfAndChildNoParent();
        }
    }
};

namespace EricScene
{
    //static std::unordered_map<Util::UUID, SceneObject> s_ObjectMap;
    enum PBRType {
        Standard,
        ClearCoat,
        Aniso,
        Transparent,
        Cloth
    };
    struct Scene
    {
        std::unordered_map<Util::UUID, std::shared_ptr<SceneObject>> _ObjectsMap;
        unsigned int _FullScreenVAO = 0;
        unsigned int _SkyboxCubeVAO = 0;
        std::map<std::string, std::shared_ptr<ModelLoader>> _MeshModelLoaderMap;
        std::vector<std::shared_ptr<SceneObject>> _AllSceneObjects;
        std::vector<std::shared_ptr<SceneObject>> _DeferredSceneObjects;
        std::vector<std::shared_ptr<SceneObject>> _ForwardOpaqueSceneObjects;
        std::vector<std::shared_ptr<SceneObject>> _ForwardInstancingOpaqueSceneObjects;
        std::vector<std::shared_ptr<SceneObject>> _ForwardTransparentSceneObjects;
        std::vector<std::shared_ptr<SceneObject>> _DepthCasters;
        std::vector<std::shared_ptr<Animation>> _Animations;
        std::vector<std::shared_ptr<Animator>> _Animators;
        std::vector<PointLight> _PointLights;
        DirectionalLight _DirectionalLight;
        float lightRotation = 0;
        std::vector<Shader*> _AllShaders;
        std::string dirStr;
        std::shared_ptr<Renderer> lightSphereRenderer;
        std::shared_ptr<Renderer> DebugLightSourceRenderer;
        unsigned int hdrTexture = 0;
        unsigned int envCubeMap = 0;
        unsigned int irradianceMap = 0;
        unsigned int preFilterMap = 0;
        unsigned int brdfLutTexture = 0;
        unsigned int preIntegratedSSSTexture = 0;
        SH9Color SceneSH9Color;

        Shader litSkinForwardShader;
        Shader litClothForwardShader;
        Shader litAnisoForwardShader;
        Shader litForwardShader;
        Shader unlitLightShader;
        Shader antiAliasingShader;
        Shader toneMappingShader;
        Shader blitShader;
        Shader debugTextureShader;
        Shader debugTextureArrayShader;
        Shader ssaoShader;
        Shader ssaoBlurShader;
        Shader hbaoShader;
        Shader hbaoBlurShader;
        Shader skyboxShader;
        Shader shadowDepthShader;
        Shader rsmDepthShader;
        Shader rsmColorShader;
        Shader indirectLightingShader;
        Shader depthPrepassShader;
        Shader geometryPassDeferredShader;
        Shader punctualLightDeferredShader;
        Shader directionalLightDeferredShader;
        Shader envCubeShader;
        Shader diffuseIrradianceShader;
        Shader prefilterMapShader;
        Shader brdfLutShader;
        Shader preIntegratedSSSShader;
        Shader forwardLightDebugShader;
        
        virtual void CreateScene();
        virtual void Update(float dt);
        virtual void DeleteScene();
        virtual void OnGUI();
        std::shared_ptr<SceneObject> AddSceneObject();
        std::shared_ptr<SceneObject> AddSceneObject(std::vector<Renderer> renderers, glm::mat4 transform);
        void SortRenderers();
        void SortRenderer(std::shared_ptr<SceneObject> renderer);
    };
}
