#pragma once
#include <string>
#include <vector>
#include <glm/detail/type_vec.hpp>

namespace Debug {
    struct DebugConfig {
        static float sunLux;
        static float envLux;
        static float normalBias;
        static float depthBias;
        static glm::vec3 _lightDir;
        static float shadowMapMultZ;
        static glm::vec3 altShadowColor;
        static float pcssSearchRadius;
        static float pcssFilterRadius;
        static float pcssLightWorldSize;
        static float pcssLightFrustumWidth;
        static float pcssNearPlane;
        static float demoSphereAnisotropy;
        static float demoSkinSSSBlend;
        static float demoSkinCurvatureOffset;
        static bool demoSphereUseThreadMap;
        static float demoSphereThreadScale;
        static float demoSphereThreadStrength;
        static float demoSphereThreadAO;
        static float demoSphereFuzzScale;
        static float demoSphereFuzzStrength;
        static float demoSphereRoughness;
        static float demoSphereMetallic;
        static float demoSphereClearCoat;
        static float demoSphereClearCoatRoughness;
        static float normalMapStrength;
        static float heightScale;
        static float exposure;
        static float bloomThreshold;
        static bool enableToneMapping;
        static bool enableBloom;
        static int parallaxSteps;
        static int shadowMapLayer;
        static bool visualizeRSMDepth;
        static bool visualizeRSMPosition;
        static bool visualizeRSMNormal;
        static bool visualizeRSMFlux;
        static bool visualizeIndirectLighting;
        static bool visualizeShadowMap;
        static bool visualizeBloomMask;
        static bool visualizeBlurPass;
        static bool visualizeNormal;
        static bool visualizePosition;
        static bool visualizeAlbedo;
        static bool visualizeMaterialParams;
        static bool visualizeSSAO;
        static bool visualizeComputeResultMap;
        static bool visualizeDepthPrepass;
        static float ssaoRadius;
        static bool testLogic;
        static bool enableAO;
        static bool useSSAO;
        static float blurRadius;
        static float blurShaprness;
        static bool useHalfResolutioAO;
        static bool useBilateralBlur;
        static float ssaoBias;
        static float aoStrength;
        static float aoPower;
        static float aoRandomSize;
        static float rsmMaxRadius;
        static float rsmFluxStrength;
        static glm::vec3 directionalLightEuler;
        static float pointLightRadiusFalloff;
        static glm::vec3 testSphereColor;
        static glm::vec3 testSphereColor2;
        static glm::vec3 testSphereColor3;
        static unsigned int debugTextureId;
        static bool useSSS;
        static std::vector<std::string> ExposeUniformTextures;
    };
}

