#include "DebugConfig.h"

#include <glm/detail/type_vec3.hpp>

namespace Debug
{
    std::vector<std::string>  DebugConfig::ExposeUniformTextures;
    unsigned int DebugConfig::debugTextureId;
    float DebugConfig::sunLux = 60000;
    float DebugConfig::envLux = 60000;
    float DebugConfig::normalBias = 0.025f;
    float DebugConfig::depthBias = 0.063f;
    glm::vec3 DebugConfig::_lightDir;
    float DebugConfig::shadowMapMultZ = 10.0f;
    glm::vec3 DebugConfig::altShadowColor = glm::vec3(0);
    float DebugConfig::pcssSearchRadius = 0.75f;
    float DebugConfig::pcssFilterRadius = 10.0f;
    float DebugConfig::pcssLightWorldSize = 0.5f;
    float DebugConfig::pcssLightFrustumWidth = 8.0f;
    float DebugConfig::pcssNearPlane = 20.0f;
    bool DebugConfig::demoSphereUseThreadMap = false;
    float DebugConfig::demoSkinSSSBlend = 1.0f;
    float DebugConfig::demoSkinCurvatureOffset = 0.0f;
    float DebugConfig::demoSphereThreadScale = 4.0f;
    float DebugConfig::demoSphereThreadStrength = 1.0f;
    float DebugConfig::demoSphereThreadAO = 0.0f;
    float DebugConfig::demoSphereFuzzScale = 1.0f;
    float DebugConfig::demoSphereFuzzStrength = 0.5f;
    float DebugConfig::demoSphereAnisotropy  = 0.5f;
    float DebugConfig::demoSphereRoughness = 0.5f;
    float DebugConfig::demoSphereMetallic = 0.0f;
    float DebugConfig::demoSphereClearCoat = 0.5f;
    float DebugConfig::demoSphereClearCoatRoughness = 0.5f;
    float DebugConfig::normalMapStrength = 0.8f;
    float DebugConfig::heightScale = 0.0f; // Initialize as needed
    float DebugConfig::exposure = 1.30f;
    float DebugConfig::bloomThreshold = 0.5f;
    bool DebugConfig::enableToneMapping = true;
    bool DebugConfig::enableBloom = false;
    int DebugConfig::parallaxSteps = 32;
    int DebugConfig::shadowMapLayer = 0;
    bool DebugConfig::visualizeRSMDepth = false;
    bool DebugConfig::visualizeRSMPosition = false;
    bool DebugConfig::visualizeRSMNormal = false;
    bool DebugConfig::visualizeRSMFlux = false;
    bool DebugConfig::visualizeIndirectLighting = false;
    bool DebugConfig::visualizeShadowMap = false;
    bool DebugConfig::visualizeBloomMask = false;
    bool DebugConfig::visualizeBlurPass = false;
    bool DebugConfig::visualizeNormal = false;
    bool DebugConfig::visualizePosition = false;
    bool DebugConfig::visualizeAlbedo = false;
    bool DebugConfig::visualizeMaterialParams = false;
    bool DebugConfig::visualizeSSAO= false;
    bool DebugConfig::visualizeComputeResultMap = false;
    bool DebugConfig::visualizeDepthPrepass = false;
    float DebugConfig::ssaoRadius = 50.5f;
    float DebugConfig::ssaoBias = 0.025f;
    float DebugConfig::aoStrength = 1.5f;
    float DebugConfig::aoRandomSize = 0.5f;
    float DebugConfig::aoPower = 1.5f;
    float DebugConfig::blurRadius = 3;
    float DebugConfig::blurShaprness = 0.5;
    bool DebugConfig::testLogic = false;
    bool DebugConfig::enableAO = true;
    bool DebugConfig::useSSAO = false;
    bool DebugConfig::useHalfResolutioAO = false;
    bool DebugConfig::useBilateralBlur = false;

    float DebugConfig::rsmMaxRadius = 0.16f;
    float DebugConfig::rsmFluxStrength = 0.5f;
    glm::vec3 DebugConfig::directionalLightEuler = glm::vec3(0,53,190);
    float DebugConfig::pointLightRadiusFalloff = 5.0f;
    glm::vec3 DebugConfig::testSphereColor = glm::vec3(0.18f);
    glm::vec3 DebugConfig::testSphereColor2 = glm::vec3(0.04f);
    glm::vec3 DebugConfig::testSphereColor3 = glm::vec3(0.04f);
    bool DebugConfig::useSSS = false;
}
