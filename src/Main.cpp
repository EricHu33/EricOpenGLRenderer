#include <iostream>
#define NOMINMAX 
#include <chrono>
#include <windows.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb/stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <random>
#include <ctime>
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <assimp/scene.h>

#include <GL/gl.h>
#include "imgui.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <tchar.h>
#include <Shlwapi.h>
#include <stdio.h>
#include <thread>
#include <GL/gl.h>

#include"Classes/Shader.h"
#include "Classes/Camera.h"
#include "LightDefine.h"
#include "RenderPasses/BloomRenderPass.h"
#include "RenderPasses/CascadedShadowMapRenderPass.h"
#include "RenderPasses/GeometryRenderPass.h"
#include "RenderPasses/LightingRenderPass.h"
#include "RenderPasses/SSAORenderPass.h"
#include "RenderPasses/ForwardRenderPass.h"
#include "RenderPasses/PostProcessingRenderPass.h"
#include "RenderPasses/DebugRenderPass.h"
#include "DebugConfig.h"
#include "Classes/PhotmetryLighting.h"
#include "RenderPasses/CubeMapIBLRenderPass.h"
#include "Scenes\PBRPSkinnedDemoScene.h"
#include "RenderPasses/DepthPrepassRenderPass.h"
#include "RenderPasses/ReflectiveShadowMapRenderPass.h"
#include "ImGuizmo.h"
#include "PlatformUtil.h"
#include <nlohmann/json.hpp>

#include "RenderUtil.h"
#include "Classes\ComputeShader.h"
#include "RenderPasses/IndirectLightingRenderPass.h"
#include "Scenes/PBRClothDemoScene.h"
#include "Scenes/PBRPropsDemoScene.h"
#include "Scenes/PBRPSkinnedDemoScene.h"
#include "Scenes/PBRSphereDemoScene.h"
#include "Scenes/SponzaDemoScene.h"

void on_gui(ImGuiIO& io, EricScene::Scene& scene, glm::mat4 viewMatrix, glm::mat4 projectionMatrix);
void ProcessInput(GLFWwindow* window, float delta);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);
void StylePooPoo(ImGuiStyle* dst);

GLuint loadTexture(const char *path);
// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
bool isInitMouse = true;
float lastX = 400, lastY = 300;
float lastLensX = 400, lastLensY = 300;
Camera camera(glm::vec3(0.0f, 1.6f, 3.0f));
float _fov = 60.0f;
float _near = 0.1f;
float _far = 30.0f;
int windowHeight = 960;
int windowWidth = 1440;
std::vector<float> shadowCascadeLevels{ 5.0f, 7.0f, 12.0f, 30.0f };

bool show_demo_window = true;
bool show_another_window = false;
bool mouseLock;
bool hide_ui = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
glm::vec3 _lightDir;
float shadowMapMultZ = 10.0f;
float normalMapStrength = 0.8f;
float heightScale;
int parallaxSteps = 32;
int debugLayer = 0;
bool is_deferred = true;

#define MAKE_VAR(x) int x = 3;

void UpdateAnimatorPerThread(EricScene::Scene& scene, int threadID, float dt)
{
    for(int i = 0; i < scene._Animators.size();i++)
    {
        scene._Animators[i]->UpdateAnimation(dt);
    }
}

void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei length, 
                            const char *message, 
                            const void *userParam)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}

int main()
{
    MAKE_VAR(hmm)
    std::cout<<hmm<<std::endl;
    //initialize glfw and window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "EricOpenGLRenderer", NULL, NULL);
    Util::FileDialogs::window = window;
    if (window == NULL)
    {
        std::cout << "failed to create GLFW window";
        glfwTerminate();
        return -1;
    }

    TCHAR buffer[2048];
    GetModuleFileName ( NULL, buffer, sizeof(buffer));
    std::wcout << "path = " << buffer << std::endl;
    TCHAR directory[2048];
    _tcscpy_s(directory, 2048, buffer);
    PathRemoveFileSpec(directory);
    std::wstring wstr(directory);
    std::string dirStr(wstr.begin(), wstr.end());
    
    dirStr = dirStr + "\\..\\..\\";
    //initialize camera class
    glfwMakeContextCurrent(window);
    gladLoadGL();
    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    //GL_CONTEXT_FLAG_NO_ERROR_BIT
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT )
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
        printf( "Debug" );
    }
    else
    {
        printf( "Debug for OpenGL not supported by your system!" );
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(window, MouseCallback);  
    GLint no_of_extensions = 0;
    //glGetIntegerv(GL_NUM_EXTENSIONS, &no_of_extensions);
    //printf("ext : %d\n", no_of_extensions);
    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    ImGuiStyle& style = ImGui::GetStyle();
    StylePooPoo(&style);
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
    stbi_set_flip_vertically_on_load(true);

    Debug::DebugConfig::ExposeUniformTextures.push_back("_baseMap");
    Debug::DebugConfig::ExposeUniformTextures.push_back("_normalMap");
    Debug::DebugConfig::ExposeUniformTextures.push_back("_occlusionRoughMetalMap");
    Debug::DebugConfig::ExposeUniformTextures.push_back("_threadMap");
    Debug::DebugConfig::ExposeUniformTextures.push_back("_fuzzMap");
    Debug::DebugConfig::ExposeUniformTextures.push_back("_curvatureMap");
    Debug::DebugConfig::ExposeUniformTextures.push_back("_sssLut");
    Debug::DebugConfig::ExposeUniformTextures.push_back("_shLUT");
    
    EricScene::PBRSphereDemoScene myScene;
    myScene.CreateScene();
    glm::mat4 projection;
    //fov, ratio, near, far
    float _aspect = (float)windowWidth / (float)windowHeight;
    projection = glm::perspective(glm::radians(_fov), _aspect, _near, _far);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    CascadedShadowMapRenderPass shadowMapRenderPass;
    shadowMapRenderPass.ShadowCasterShader = &myScene.shadowDepthShader;
    shadowMapRenderPass.shadowCascadeLevels = shadowCascadeLevels;
    shadowMapRenderPass.Create(windowWidth, windowHeight);

    ReflectiveShadowMapRenderPass rsmRenderPass;
    rsmRenderPass.RSMDepthShader = &myScene.rsmDepthShader;
    rsmRenderPass.RSMColorShader = &myScene.rsmColorShader;
    rsmRenderPass.shadowCascadeLevels = shadowCascadeLevels;
    rsmRenderPass.Create(windowWidth, windowHeight);

    DepthPrepassRenderPass depthPrepassRenderPass;
    depthPrepassRenderPass.DepthPrepassShader = &myScene.depthPrepassShader;
    depthPrepassRenderPass.DepthRenderers = myScene._DepthCasters;
    depthPrepassRenderPass.Create(windowWidth, windowHeight);

    GeometryRenderPass geometryRenderPass;
    geometryRenderPass.Create(windowWidth, windowHeight);
    
    LightingRenderPass lightingRenderPass;
    lightingRenderPass.Create(windowWidth, windowHeight);

    SSAORenderPass ssaoRenderPass;
    ssaoRenderPass.SSAOShader = &myScene.ssaoShader;
    ssaoRenderPass.SSAOBlurShader = &myScene.ssaoBlurShader;
    ssaoRenderPass.HBAOShader = &myScene.hbaoShader;
    ssaoRenderPass.HBAOBlurShader = &myScene.hbaoBlurShader;
    ssaoRenderPass.Create(windowWidth, windowHeight);
    
    BloomRenderPass bloomRenderPass;
    bloomRenderPass.Create(windowWidth, windowHeight);

    ForwardRenderPass forwardRenderPass;
    forwardRenderPass.DebugLightSourceRenderer = myScene.DebugLightSourceRenderer;
    forwardRenderPass.SkyBoxShader = &myScene.skyboxShader;
    forwardRenderPass.IsDeferred = is_deferred;
    forwardRenderPass.Create(windowWidth, windowHeight);

    IndirectLightingRenderPass indirectLightingPass;
    indirectLightingPass.IndirectLightingShader = &myScene.indirectLightingShader;
    indirectLightingPass.BlitShader = &myScene.blitShader;
    indirectLightingPass.Create(windowWidth, windowHeight);

    PostProcessingRenderPass postProcessingRenderPass;
    postProcessingRenderPass.ToneMappingCompositeShader = &myScene.toneMappingShader;
    postProcessingRenderPass.FxaaShader = &myScene.antiAliasingShader;
    postProcessingRenderPass.BlitShader = &myScene.blitShader;
    postProcessingRenderPass.Create(windowWidth, windowHeight);

    DebugRenderPass debugRenderPass;
    debugRenderPass.ForwardLightDebugShader = &myScene.forwardLightDebugShader;
    debugRenderPass.DebugTextureShader = &myScene.debugTextureShader;
    debugRenderPass.DebugTextureArrayShader = &myScene.debugTextureArrayShader;
    debugRenderPass.DebugTestShader = &myScene.brdfLutShader;
    debugRenderPass.Create(windowWidth, windowHeight);
    
    //test global uniform variables
    glUniformBlockBinding(myScene.litForwardShader.ID, glGetUniformBlockIndex(myScene.litForwardShader.ID, "Matrices"), 0);
    glUniformBlockBinding(myScene.litAnisoForwardShader.ID, glGetUniformBlockIndex(myScene.litAnisoForwardShader.ID, "Matrices"), 0);
    glUniformBlockBinding(myScene.litClothForwardShader.ID, glGetUniformBlockIndex(myScene.litClothForwardShader.ID, "Matrices"), 0);
    glUniformBlockBinding(myScene.litSkinForwardShader.ID, glGetUniformBlockIndex(myScene.litSkinForwardShader.ID, "Matrices"), 0);
    glUniformBlockBinding(myScene.unlitLightShader.ID, glGetUniformBlockIndex(myScene.unlitLightShader.ID, "Matrices"), 0);
    glUniformBlockBinding(myScene.shadowDepthShader.ID, glGetUniformBlockIndex(myScene.shadowDepthShader.ID, "Matrices"), 0);
    glUniformBlockBinding(myScene.depthPrepassShader.ID, glGetUniformBlockIndex(myScene.depthPrepassShader.ID, "Matrices"), 0);
    glUniformBlockBinding(myScene.punctualLightDeferredShader.ID, glGetUniformBlockIndex(myScene.punctualLightDeferredShader.ID, "Matrices"), 0);

    unsigned int uboMatrices;
    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboMatrices);

    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(camera.GetViewMatrix()));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    glUniformBlockBinding(myScene.shadowDepthShader.ID, glGetUniformBlockIndex(myScene.shadowDepthShader.ID, "ShadowMatrices"), 1);
    glUniformBlockBinding(myScene.litForwardShader.ID, glGetUniformBlockIndex(myScene.litForwardShader.ID, "ShadowMatrices"), 1);
    glUniformBlockBinding(myScene.litAnisoForwardShader.ID, glGetUniformBlockIndex(myScene.litAnisoForwardShader.ID, "ShadowMatrices"), 1);
    glUniformBlockBinding(myScene.litClothForwardShader.ID, glGetUniformBlockIndex(myScene.litClothForwardShader.ID, "ShadowMatrices"), 1);
    glUniformBlockBinding(myScene.litSkinForwardShader.ID, glGetUniformBlockIndex(myScene.litSkinForwardShader.ID, "ShadowMatrices"), 1);
    glUniformBlockBinding(myScene.directionalLightDeferredShader.ID, glGetUniformBlockIndex(myScene.directionalLightDeferredShader.ID, "ShadowMatrices"), 1);
    shadowMapRenderPass.CreateShadowMatricesUBO(1);

    //SH9 ubo
    glUniformBlockBinding(myScene.directionalLightDeferredShader.ID, glGetUniformBlockIndex(myScene.directionalLightDeferredShader.ID, "SH9"), 2);
    glUniformBlockBinding(myScene.litForwardShader.ID, glGetUniformBlockIndex(myScene.litForwardShader.ID, "SH9"), 2);
    glUniformBlockBinding(myScene.litAnisoForwardShader.ID, glGetUniformBlockIndex(myScene.litAnisoForwardShader.ID, "SH9"), 2);
    glUniformBlockBinding(myScene.litClothForwardShader.ID, glGetUniformBlockIndex(myScene.litClothForwardShader.ID, "SH9"), 2);
    glUniformBlockBinding(myScene.litSkinForwardShader.ID, glGetUniformBlockIndex(myScene.litSkinForwardShader.ID, "SH9"), 2);
    
    unsigned int uboSH9;
    glGenBuffers(1, &uboSH9);
    glBindBuffer(GL_UNIFORM_BUFFER, uboSH9);
    glBufferData(GL_UNIFORM_BUFFER, 9 * sizeof(glm::vec4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboSH9);

    glBindBuffer(GL_UNIFORM_BUFFER, uboSH9);
    for (int i = 0; i < 9; ++i)
    {
        glBufferSubData(GL_UNIFORM_BUFFER, i * sizeof(glm::vec4), sizeof(glm::vec4), &myScene.SceneSH9Color.Coefficients[i]);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    //skeleton bones ubo
    glUniformBlockBinding(myScene.geometryPassDeferredShader.ID, glGetUniformBlockIndex(myScene.geometryPassDeferredShader.ID, "BonesMatrices"), 3);
    glUniformBlockBinding(myScene.shadowDepthShader.ID, glGetUniformBlockIndex(myScene.shadowDepthShader.ID, "BonesMatrices"), 3);
    glUniformBlockBinding(myScene.rsmDepthShader.ID, glGetUniformBlockIndex(myScene.rsmDepthShader.ID, "BonesMatrices"), 3);
    glUniformBlockBinding(myScene.rsmColorShader.ID, glGetUniformBlockIndex(myScene.rsmColorShader.ID, "BonesMatrices"), 3);
    glUniformBlockBinding(myScene.depthPrepassShader.ID, glGetUniformBlockIndex(myScene.depthPrepassShader.ID, "BonesMatrices"), 3);
    unsigned int uboBones;
    glGenBuffers(1, &uboBones);
    glBindBuffer(GL_UNIFORM_BUFFER, uboBones);
    glBufferData(GL_UNIFORM_BUFFER, 100 * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, uboBones);

    for (int i = 0; i < myScene._Animators.size(); ++i) {
        myScene._Animators[i]->UpdateMatricesCache();
    }

    glUniformBlockBinding(myScene.directionalLightDeferredShader.ID, glGetUniformBlockIndex(myScene.directionalLightDeferredShader.ID, "ShadowCSMParam"), 4);
    glUniformBlockBinding(myScene.litForwardShader.ID, glGetUniformBlockIndex(myScene.litForwardShader.ID, "ShadowCSMParam"), 4);
    glUniformBlockBinding(myScene.litAnisoForwardShader.ID, glGetUniformBlockIndex(myScene.litAnisoForwardShader.ID, "ShadowCSMParam"), 4);
    glUniformBlockBinding(myScene.litClothForwardShader.ID, glGetUniformBlockIndex(myScene.litClothForwardShader.ID, "ShadowCSMParam"), 4);
    glUniformBlockBinding(myScene.litSkinForwardShader.ID, glGetUniformBlockIndex(myScene.litSkinForwardShader.ID, "ShadowCSMParam"), 4);
    unsigned int uboCSMParam;
    glGenBuffers(1, &uboCSMParam);
    glBindBuffer(GL_UNIFORM_BUFFER, uboCSMParam);
    glBufferData(GL_UNIFORM_BUFFER, 4 * sizeof(float) * shadowCascadeLevels.size() * 2, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 4, uboCSMParam);

    glUniformBlockBinding(myScene.rsmDepthShader.ID, glGetUniformBlockIndex(myScene.rsmDepthShader.ID, "RSMMatrices"), 5);
    glUniformBlockBinding(myScene.rsmColorShader.ID, glGetUniformBlockIndex(myScene.rsmColorShader.ID, "RSMMatrices"), 5);
    glUniformBlockBinding(myScene.directionalLightDeferredShader.ID, glGetUniformBlockIndex(myScene.directionalLightDeferredShader.ID, "RSMMatrices"), 5);
    glUniformBlockBinding(myScene.litForwardShader.ID, glGetUniformBlockIndex(myScene.litForwardShader.ID, "RSMMatrices"), 5);
    glUniformBlockBinding(myScene.indirectLightingShader.ID, glGetUniformBlockIndex(myScene.indirectLightingShader.ID, "RSMMatrices"), 5);
    glUniformBlockBinding(myScene.litAnisoForwardShader.ID, glGetUniformBlockIndex(myScene.litAnisoForwardShader.ID, "RSMMatrices"), 5);
    glUniformBlockBinding(myScene.litClothForwardShader.ID, glGetUniformBlockIndex(myScene.litClothForwardShader.ID, "RSMMatrices"), 5);
    glUniformBlockBinding(myScene.litSkinForwardShader.ID, glGetUniformBlockIndex(myScene.litSkinForwardShader.ID, "RSMMatrices"), 5);
    rsmRenderPass.CreateShadowMatricesUBO(5);
    //test compute shader
    ComputeShader myCompute(dirStr.c_str(), "resources/shaders/ForwardPlusLightCull.glsl");
    myCompute.use();

    // X : 1440 / 16 = 90
    // Y : 960/ 16 = 60
    int workGroupsX = (int)std::ceil((float)windowWidth / 16.0 );
    int workGroupsY = (int)std::ceil((float)windowHeight / 16.0 );

    unsigned int csTexture;
    glGenTextures(1, &csTexture);
    glBindTexture(GL_TEXTURE_2D, csTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, 
                 GL_FLOAT, NULL);
    const int maxLightPerTile = 300;
    // 1 tile = 16x16 pixel
    const int totalTileNumbers = workGroupsX * workGroupsY;
    
    GLuint lightsListSSBO;
    glGenBuffers(1, &lightsListSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightsListSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(glm::vec4) * myScene._PointLights.size(), myScene._PointLights.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, lightsListSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    std::vector<int> lightIndexList(maxLightPerTile * totalTileNumbers);
    GLuint lightIndexesSSBO;
    glGenBuffers(1, &lightIndexesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightIndexesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * lightIndexList.size(), lightIndexList.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, lightIndexesSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    std::vector<int> tileLightCounts(totalTileNumbers);
    GLuint tileLightCountsSSBO;
    glGenBuffers(1, &tileLightCountsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, tileLightCountsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * tileLightCounts.size(), tileLightCounts.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, tileLightCountsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    auto nearV = projection[3][2]/(projection[2][2] - 1.0f);
    auto farV  = projection[3][2]/(projection[2][2] + 1.0f);
    std::cout << "near :" << nearV<<std::endl;
    std::cout << "far :" << farV<<std::endl;
    GLuint blueNoiseTexture = Util::RenderUtil::LoadTexture((dirStr + "resources/textures/BlueNoise470.png").c_str(), false);

    unsigned int lensFBO;
    unsigned int lensColorBuffer;
    glGenFramebuffers(1, &lensFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, lensFBO);
    glGenTextures(1, &lensColorBuffer);
    glBindTexture(GL_TEXTURE_2D, lensColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 96, 96, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lensColorBuffer, 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    
    
    while (!glfwWindowShouldClose(window))
    {
        float timeValue = glfwGetTime();
        deltaTime = timeValue - lastFrame;
        lastFrame = timeValue;
        ProcessInput(window, deltaTime);
        glfwPollEvents();

        on_gui(io, myScene, camera.GetViewMatrix(), projection);
        myScene.Update(deltaTime);
        
        auto viewMatrix = camera.GetViewMatrix();
        std::thread animationThread;
        for(int i = 0; i < myScene._Animators.size();i++)
        {
           myScene._Animators[i]->SetUbo(uboBones);
        }
        auto start1 = std::chrono::high_resolution_clock::now();
        animationThread = std::thread(UpdateAnimatorPerThread, std::ref(myScene), 0, deltaTime);
        
        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(camera.GetViewMatrix()));
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(projection));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBuffer(GL_UNIFORM_BUFFER, uboCSMParam);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * shadowCascadeLevels.size(),shadowCascadeLevels.data());
        auto csmSizeData =  shadowMapRenderPass.GetCsmSizes();
        glBufferSubData(GL_UNIFORM_BUFFER,  4 * sizeof(float) * shadowCascadeLevels.size(), sizeof(float) * shadowCascadeLevels.size(),
            csmSizeData.data());
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        
        glDisable(GL_STENCIL_TEST);
        
        shadowMapRenderPass.Camera = &camera;
        shadowMapRenderPass.LightDirection = myScene._DirectionalLight.Direction;
        shadowMapRenderPass.Fov = _fov;
        shadowMapRenderPass.Near = _near;
        shadowMapRenderPass.Far = _far;
        shadowMapRenderPass.shadowCascadeLevels = shadowCascadeLevels;
        shadowMapRenderPass.ShadowCastRenderers = myScene._DeferredSceneObjects;
        shadowMapRenderPass.ShadowCastFowardRenderers = myScene._ForwardOpaqueSceneObjects;
        shadowMapRenderPass.drawShadows = true;
        shadowMapRenderPass.Render(deltaTime);

        rsmRenderPass.Camera = &camera;
        rsmRenderPass.LightDirection = myScene._DirectionalLight.Direction;
        rsmRenderPass.Fov = _fov;
        rsmRenderPass.Near = _near;
        rsmRenderPass.Far = 10;
        rsmRenderPass.shadowCascadeLevels = shadowCascadeLevels;
        rsmRenderPass.ShadowCastRenderers = myScene._ForwardOpaqueSceneObjects;
        rsmRenderPass.Render(deltaTime);
        // -- DepthPrepass Start -- //
        depthPrepassRenderPass.FullScreenVAO = myScene._FullScreenVAO;
        depthPrepassRenderPass.Render(deltaTime);
        //-- DepthPrepass End --//

        if(is_deferred)
        {
            // -- Geometry Pass (G-Buffer) Start --
            geometryRenderPass.zPrepass = depthPrepassRenderPass.GetDepthPrepassFBO();
            geometryRenderPass.CameraPosition = camera.Position;
            geometryRenderPass.ViewMatrix = viewMatrix;
            geometryRenderPass.ProjMatrix = projection;
            geometryRenderPass.SceneObjects = myScene._DeferredSceneObjects;
            geometryRenderPass.Render(deltaTime);
            //-- Geometry Pass End --//   
        }

        //-- forward+ light culling compute Start--//
        myCompute.use();
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightsListSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * sizeof(glm::vec4) * myScene._PointLights.size(), myScene._PointLights.data(), GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightsListSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glBindImageTexture(0, csTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        
        myCompute.SetVec3("_cameraPos", camera.Position.x, camera.Position.y, camera.Position.z);
        myCompute.SetVec2("_ScreenDimensions", windowWidth, windowHeight);
        myCompute.SetUInt1("_maxLightPerTile", (unsigned int)maxLightPerTile);
        myCompute.SetUInt1("_numOfLights", myScene._PointLights.size());
        auto cameraModel = glm::mat4(1);
        cameraModel = glm::translate(cameraModel, camera.Position);
        myCompute.SetMat4("_model", cameraModel);
        myCompute.SetMat4("_view", camera.GetViewMatrix());
        myCompute.SetMat4("_projection", projection);
        myCompute.SetMat4("_InverseProjection", glm::inverse(projection));
        myCompute.SetTextureUniform("_depthMap", depthPrepassRenderPass.GetDepthPrepassBuffer());
        glDispatchCompute((unsigned int)workGroupsX, (unsigned int)workGroupsY, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        //-- forward+ light culling compute End--//
        
        float aperture = 16.0f;
        float shutterSpeed = 1.0f / 125.0f;
        float sensitivity = 100;
        float sunIlluminance  = Debug::DebugConfig::sunLux;
        float ev100 = PhotometryLighting::exposureSettings(aperture, shutterSpeed, sensitivity);
        float exposure = PhotometryLighting::exposure(ev100);
        float precomputedLightIntensity = sunIlluminance * exposure;
        float evnIlluminance = Debug::DebugConfig::envLux;
        float preComputedEnvIntensity = evnIlluminance * exposure;

        //-- SSAO PASS Start --//
        ssaoRenderPass.blueNoiseTexture = blueNoiseTexture;
        ssaoRenderPass.projectionFOV = _fov;
        ssaoRenderPass.aoRandomSize = Debug::DebugConfig::aoRandomSize;
        ssaoRenderPass.aoStrength = Debug::DebugConfig::enableAO ? Debug::DebugConfig::aoStrength : 0;
        ssaoRenderPass.aoPower = Debug::DebugConfig::aoPower;
        ssaoRenderPass.blurRadius = Debug::DebugConfig::blurRadius;
        ssaoRenderPass.blurShaprness = Debug::DebugConfig::blurShaprness;
      //  ssaoRenderPass.testBlurSource = lightingRenderPass.GetHDRTargets(0);
        ssaoRenderPass.depthMap = depthPrepassRenderPass.GetDepthPrepassBuffer();
        ssaoRenderPass.gPosition = geometryRenderPass.GetPositionBuffer();
        ssaoRenderPass.gNormal = geometryRenderPass.GetNormalBuffer();
        ssaoRenderPass.ssaoRadius = Debug::DebugConfig::ssaoRadius;
        ssaoRenderPass.ssaoBias = Debug::DebugConfig::ssaoBias;
        ssaoRenderPass.ViewMatrix = viewMatrix;
        ssaoRenderPass.ProjectionMatrix = projection;
        ssaoRenderPass.EnableTestLogic = Debug::DebugConfig::testLogic;
        ssaoRenderPass.useSSAO = Debug::DebugConfig::useSSAO;
        ssaoRenderPass.useHalfResolution = Debug::DebugConfig::useHalfResolutioAO;
        ssaoRenderPass.useBilateralBlur = Debug::DebugConfig::useBilateralBlur;
        ssaoRenderPass.FullScreenQuad4VAO = myScene._FullScreenVAO;
        ssaoRenderPass.Render(deltaTime);
        //-- SSAO PASS End --//

        if(is_deferred)
        {
            //-- Lighting Pass Start --
            lightingRenderPass.GeometryPassFBO = geometryRenderPass.GetFBO();
            lightingRenderPass.PointLights = myScene._PointLights;
            lightingRenderPass.PointLightsCount = std::size(myScene._PointLights);
            lightingRenderPass.DirectionalLightShader = &myScene.directionalLightDeferredShader;
            lightingRenderPass.DirectionalLight = myScene._DirectionalLight;
            lightingRenderPass.Exposure = exposure;
            lightingRenderPass.PreComputedEnvIntensity = preComputedEnvIntensity;
            lightingRenderPass.PreComputedLightIntensity = precomputedLightIntensity;
            lightingRenderPass.AltShadowColor = Debug::DebugConfig::altShadowColor;
            lightingRenderPass.pcssSearchRadius = Debug::DebugConfig::pcssSearchRadius;
            lightingRenderPass.pcssFilterRadius = Debug::DebugConfig::pcssFilterRadius;
            lightingRenderPass.normalBias = Debug::DebugConfig::normalBias;
            lightingRenderPass.depthBias = Debug::DebugConfig::depthBias;
            lightingRenderPass.nearPlane = Debug::DebugConfig::pcssNearPlane;
            lightingRenderPass.FullScreenQuad4VAO = myScene._FullScreenVAO;
            
            lightingRenderPass.punctualLightRenderer =  myScene.lightSphereRenderer;
            lightingRenderPass.gPosition = geometryRenderPass.GetPositionBuffer();
            lightingRenderPass.gNormal = geometryRenderPass.GetNormalBuffer();
            lightingRenderPass.gAlbedoSpec = geometryRenderPass.GetAlbedoBuffer();
            lightingRenderPass.gMaterialParam = geometryRenderPass.GetMaterialParamBuffer();
            lightingRenderPass.ssao = ssaoRenderPass.GetSSAOPreBlurResult();
            lightingRenderPass.diffuseIrradiance = -1;
            lightingRenderPass.prefilterMap = myScene.preFilterMap;
            lightingRenderPass.brdfLUT = myScene.brdfLutTexture;
            lightingRenderPass.cascadeShadowMapArray = shadowMapRenderPass.GetShadowMapTexuture();
            lightingRenderPass._far = _far;
            lightingRenderPass.shadowCascadeLevels = shadowCascadeLevels;
            lightingRenderPass.shadowCascadeFrustumSizes = shadowMapRenderPass.GetCsmSizes();
            lightingRenderPass.BloomThreshold = Debug::DebugConfig::bloomThreshold;
            lightingRenderPass.CameraPosition = camera.Position;
            lightingRenderPass.ViewMatrix = viewMatrix;
            lightingRenderPass.ProjMatrix = projection;
            lightingRenderPass.Render(deltaTime);
            //-- Lighting Pass End --
        }
        
        // -- Indirect Lighting Start --
        indirectLightingPass.FullScreenQuad4VAO = myScene._FullScreenVAO;
        indirectLightingPass.DepthMap = depthPrepassRenderPass.GetDepthPrepassBuffer();
        indirectLightingPass.RsmDepth = rsmRenderPass.GetRSMDepth();
        indirectLightingPass.RsmPosition = rsmRenderPass.GetPositionBuffer();
        indirectLightingPass.RsmNormal = rsmRenderPass.GetNormalBuffer();
        indirectLightingPass.RsmFlux = rsmRenderPass.GetFluxBuffer();
        indirectLightingPass.ViewMatrix = viewMatrix;
        indirectLightingPass.ProjectionMatrix = projection;
        indirectLightingPass.RsmMaxRadius = Debug::DebugConfig::rsmMaxRadius;
        indirectLightingPass.RsmFluxStrength = Debug::DebugConfig::rsmFluxStrength;
        indirectLightingPass.Render(deltaTime);
        // -- Indirect Lighting End --
        
        // -- Forward Pass Start --
        forwardRenderPass.DrawDebugLightSourceCubes = false;
        forwardRenderPass.IsDeferred = is_deferred;
        forwardRenderPass.zPrepass = depthPrepassRenderPass.GetDepthPrepassFBO();
        forwardRenderPass.GeometryPassFBO = geometryRenderPass.GetFBO();
        forwardRenderPass.CurrentFullScreenFBO = lightingRenderPass.GetLightingPassFBO();
        forwardRenderPass.SkyboxTexture = myScene.envCubeMap;
        forwardRenderPass.ForwardPassLightDatas = myScene._PointLights;
        forwardRenderPass.ForwardPassInstancingRenderer = myScene._ForwardInstancingOpaqueSceneObjects;
        forwardRenderPass.ForwardPassMeshOpqaueRenderer = myScene._ForwardOpaqueSceneObjects;
        forwardRenderPass.ForwardPassMeshTransparentRenderer = myScene._ForwardTransparentSceneObjects;
        forwardRenderPass.bloomThreshold = Debug::DebugConfig::bloomThreshold;
        forwardRenderPass.ViewMatrix = viewMatrix;
        forwardRenderPass.ProjectionMatrix = projection;
        forwardRenderPass.DirectionalLight = myScene._DirectionalLight;
        forwardRenderPass.CameraPosition = camera.Position;
        forwardRenderPass.rsmDepth = rsmRenderPass.GetRSMDepth();
        forwardRenderPass.rsmPosition = rsmRenderPass.GetPositionBuffer();
        forwardRenderPass.rsmNormal = rsmRenderPass.GetNormalBuffer();
        forwardRenderPass.rsmFlux = rsmRenderPass.GetFluxBuffer();
        forwardRenderPass.indirectLightingMap = indirectLightingPass.GetIndirectLightingMap();
        forwardRenderPass.RsmMaxRadius = Debug::DebugConfig::rsmMaxRadius;
        forwardRenderPass.RsmFluxStrength = Debug::DebugConfig::rsmFluxStrength;
        if(Debug::DebugConfig::useBilateralBlur)
        {
            forwardRenderPass.ssao = ssaoRenderPass.GetSSAOPreBlurResult();
        }
        else
        {
            forwardRenderPass.ssao = ssaoRenderPass.GetSSAOPreBlurResult();
        }
        forwardRenderPass.diffuseIrradiance = -1;
        forwardRenderPass.prefilterMap = myScene.preFilterMap;
        forwardRenderPass.brdfLUT = myScene.brdfLutTexture;
        forwardRenderPass.cascadeShadowMapArray = shadowMapRenderPass.GetShadowMapTexuture();
        forwardRenderPass.pcssSearchRadius = Debug::DebugConfig::pcssSearchRadius;
        forwardRenderPass.pcssFilterRadius = Debug::DebugConfig::pcssFilterRadius;
        forwardRenderPass.normalBias = Debug::DebugConfig::normalBias;
        forwardRenderPass.depthBias = Debug::DebugConfig::depthBias;
        forwardRenderPass.shadowCascadeFrustumSizes = shadowMapRenderPass.GetCsmSizes();
        forwardRenderPass.shadowCascadeLevels = shadowCascadeLevels;
        forwardRenderPass.Exposure = exposure;
        forwardRenderPass.PreComputedEnvIntensity = preComputedEnvIntensity;
        forwardRenderPass.PreComputedLightIntensity = precomputedLightIntensity;
        forwardRenderPass.Render(deltaTime);
        // -- Forward Pass End --
        
        // -- MRT Bloom Pass Start --
        bloomRenderPass.CurrentColorBuffer = lightingRenderPass.GetHDRTargets(1);
        bloomRenderPass.FullScreenQuad4VAO = myScene._FullScreenVAO;
        bloomRenderPass.Render(deltaTime);
        // -- Bloom Pass End --

        // -- ToneMapping Start --
        postProcessingRenderPass.SsaoMap = ssaoRenderPass.GetSSAOBlurredResult();
        postProcessingRenderPass.FullScreenQuad4VAO = myScene._FullScreenVAO;
        postProcessingRenderPass.BaseMap = is_deferred ? lightingRenderPass.GetHDRTargets(0) : forwardRenderPass.GetFowardPathBuffer();
        postProcessingRenderPass.BloomTexture = bloomRenderPass.GetBlurResult();
        postProcessingRenderPass.TestLogic = Debug::DebugConfig::testLogic;
        postProcessingRenderPass.enableBloom = Debug::DebugConfig::enableBloom;
        postProcessingRenderPass.exposure = Debug::DebugConfig::exposure;
        postProcessingRenderPass.enableToneMapping = Debug::DebugConfig::enableToneMapping;
        postProcessingRenderPass.Render(deltaTime);
        // -- ToneMapping End --

        glm::vec4 ndcMouse = glm::vec4(2.0f * (lastLensX/windowWidth - 0.5f), 2.0f * (1.0 - lastLensY/windowHeight -0.5f), -0.9, 1.0f);
        glm::vec4 viewMoise = glm::inverse(projection) * ndcMouse;
        viewMoise = glm::vec4(viewMoise.x / viewMoise.w, viewMoise.y / viewMoise.w, viewMoise.z / viewMoise.w, viewMoise.w / viewMoise.w);
        glm::vec4 worldMouse = glm::inverse( camera.GetViewMatrix()) * viewMoise;
        glm::mat4 view = glm::lookAt(glm::vec3(worldMouse), camera.Position -  glm::vec3(worldMouse), camera.Up);
        
        glBindFramebuffer(GL_FRAMEBUFFER, lensFBO);
        glViewport((int)(windowWidth*0.0f) , (int)(windowHeight*0.0), 96,96);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glClear(GL_COLOR_BUFFER_BIT);
        myScene.unlitLightShader.use();
        myScene.unlitLightShader.SetTextureUniform(111, "_cameraColorMap", postProcessingRenderPass.GetCameraColorImage());
        myScene.unlitLightShader.SetVec3("_lightColor", 1,0,0);
        myScene.unlitLightShader.SetVec2("_mousePos", lastLensX, 960-lastLensY);
        glBindVertexArray(myScene._FullScreenVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, windowWidth, windowHeight);

        // -- DebugView Start --
        myScene.forwardLightDebugShader.use();
        myScene.forwardLightDebugShader.SetInt1("totalLightCount", myScene._PointLights.size());
        myScene.forwardLightDebugShader.SetInt1("numberOfTilesX", workGroupsX);
        debugRenderPass.debugVAO = myScene._FullScreenVAO;

        debugRenderPass.shadowMapLayer = Debug::DebugConfig::shadowMapLayer;
        debugRenderPass.shadowMap = shadowMapRenderPass.GetShadowMapTexuture();
        debugRenderPass.rsmDepthMap = rsmRenderPass.GetRSMDepth();
        debugRenderPass.rsmPositionMap = rsmRenderPass.GetPositionBuffer();
        debugRenderPass.rsmNormalMap = rsmRenderPass.GetNormalBuffer();
        debugRenderPass.rsmFluxMap = rsmRenderPass.GetFluxBuffer();
        debugRenderPass.indirectLightingMap = indirectLightingPass.GetIndirectLightingMap();
        debugRenderPass.bloomMask = lightingRenderPass.GetHDRTargets(1);
        debugRenderPass.bloomResultMap = bloomRenderPass.GetBlurResult();
        debugRenderPass.gBufferNormal = geometryRenderPass.GetNormalBuffer();
        debugRenderPass.gBufferPosition = geometryRenderPass.GetPositionBuffer();
        debugRenderPass.gBufferAlbedo = geometryRenderPass.GetAlbedoBuffer();
        debugRenderPass.gBufferMaterial = geometryRenderPass.GetMaterialParamBuffer();
        debugRenderPass.computeResultMap = csTexture;
        debugRenderPass.depthPrepassMap = depthPrepassRenderPass.GetDepthPrepassBuffer();
        debugRenderPass.brdfLut = myScene.preIntegratedSSSTexture;
        if(Debug::DebugConfig::useBilateralBlur)
        {
            debugRenderPass.ssaoMap = ssaoRenderPass.GetSSAOPreBlurResult();
        }
        else
        {
            debugRenderPass.ssaoMap = ssaoRenderPass.GetSSAOPreBlurResult();
        }
        debugRenderPass.visualizeShadowMap = Debug::DebugConfig::visualizeShadowMap;
        debugRenderPass.visualizeRSMDepth = Debug::DebugConfig::visualizeRSMDepth;
        debugRenderPass.visualizeRSMPosition = Debug::DebugConfig::visualizeRSMPosition;
        debugRenderPass.visualizeRSMNormal = Debug::DebugConfig::visualizeRSMNormal;
        debugRenderPass.visualizeRSMFlux = Debug::DebugConfig::visualizeRSMFlux;
        debugRenderPass.visualizeIndirectLighting = Debug::DebugConfig::visualizeIndirectLighting;
        debugRenderPass.visuallizeBloomMask =  Debug::DebugConfig::visualizeBloomMask;
        debugRenderPass.visualizeBlurPass  =  Debug::DebugConfig::visualizeBlurPass;
        debugRenderPass.visualizeNormal =  Debug::DebugConfig::visualizeNormal;
        debugRenderPass.visualizePosition =  Debug::DebugConfig::visualizePosition;
        debugRenderPass.visualizeAlbedo =  Debug::DebugConfig::visualizeAlbedo;
        debugRenderPass.visualizeMaterialParams =  Debug::DebugConfig::visualizeMaterialParams;
        debugRenderPass.visualizeSSAO = Debug::DebugConfig::visualizeSSAO;
        debugRenderPass.visualizeComputeShaderTex = Debug::DebugConfig::visualizeComputeResultMap;
        debugRenderPass.visualizeDepthPrepass = Debug::DebugConfig::visualizeDepthPrepass;
        debugRenderPass.DebugTextureArrayLayer = debugLayer;
        debugRenderPass.ProjectionMatrix = projection;
        debugRenderPass.Render(deltaTime);
        // -- DebugView End --
        
       /* myScene.unlitLightShader.use();
        myScene.unlitLightShader.SetTextureUniform(122, "_lensMap", lensColorBuffer);
        myScene.unlitLightShader.SetVec3("_lightColor", 1,0,0);
        myScene.unlitLightShader.SetVec1("_isFinalBlit", true);
        myScene.unlitLightShader.SetVec3("CameraRight_worldspace", camera.Right);
        myScene.unlitLightShader.SetVec3("CameraUp_worldspace", camera.Up);
        myScene.unlitLightShader.SetMat4("_VP", projection * camera.GetViewMatrix());
        myScene.unlitLightShader.SetVec3("_billboardPos", worldMouse);
        myScene.unlitLightShader.SetVec2("BillboardSize", 0.01f, 0.01f);
        
        glBindVertexArray(myScene._FullScreenVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);*/
        
        //DebugLightSourceRenderer->GetMainMaterial()->SetMat4("_model", obj2World);
        
        // std::cout << "Execution time: " << duration.count() << " milliseconds" << std::endl;
       // printf("mouse pos wS : %.2f, %.2f, %.2f \n",worldMouse.x, worldMouse.y, worldMouse.z);
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        auto start2 = std::chrono::high_resolution_clock::now();
        animationThread.join();
        for (int j = 0; j < myScene._Animators.size(); j++)
        {
            myScene._Animators[j]->UpdateMatricesCache();
        }
        
        auto end2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end2 - start1;

        
        // -- Default FBO Blit Pass End --
        //swap back buffer with the front buffer
        glfwSwapBuffers(window);
    }
    glDeleteBuffers(1, &uboMatrices);
    shadowMapRenderPass.Dispose();
    depthPrepassRenderPass.Dispose();
    rsmRenderPass.Dispose();
    geometryRenderPass.Dispose();
    lightingRenderPass.Dispose();
    forwardRenderPass.Dispose();
    ssaoRenderPass.Dispose();
    bloomRenderPass.Dispose();
    postProcessingRenderPass.Dispose();
    debugRenderPass.Dispose();
    myScene.DeleteScene();
    myCompute.Dispose();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
	return 0;
}
uint64_t selectedObjectUUID = -1;

void DisplayChildrenRecursive(const SceneObject& obj, int baseIndex, int subIndex) {
    if (ImGui::TreeNode((void*)(intptr_t)(baseIndex + subIndex), "Child %d", (baseIndex)))
    {
        if(ImGui::IsItemClicked())
        {
            selectedObjectUUID = obj.uuid;
        }
        const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
        const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
        static ImGuiTableFlags flags =  ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
        for (size_t i = 0; i < obj.childs.size(); ++i) {
            DisplayChildrenRecursive(*obj.childs[i],baseIndex, i+1);
        }
        ImGui::TreePop();
    }
}

void DrawSaveMaterialButton(std::shared_ptr<Renderer> renderer)
{
    if(ImGui::Button("Save Material"))
    {
        TCHAR buffer[2048];
        GetModuleFileName ( NULL, buffer, sizeof(buffer));
        std::wcout << "path = " << buffer << std::endl;
        TCHAR directory[2048];
        _tcscpy_s(directory, 2048, buffer);
        PathRemoveFileSpec(directory);
        std::wstring wstr(directory);
        auto dirStr =  std::string(wstr.begin(), wstr.end());
        dirStr = dirStr + "\\..\\..\\";
        auto filePath  = Util::FileDialogs::SaveFile(dirStr.c_str());
        if(!filePath.empty())
        {
            auto jsonData = renderer->GetMainMaterial()->ToJson();
            std::ofstream file(filePath);
            if (file.is_open()) {
                file << std::setw(4) << jsonData << std::endl;
                file.close();
                std::cout << "save json successfully." << std::endl;
            } else {
                std::cerr << "Unable to open file for writing." << std::endl;
            }            
        }
    }
}

void DrawLoadMaterialButton(std::shared_ptr<Renderer> renderer)
{
    if(ImGui::Button("Load Material"))
    {
        TCHAR buffer[2048];
        GetModuleFileName ( NULL, buffer, sizeof(buffer));
        std::wcout << "path = " << buffer << std::endl;
        TCHAR directory[2048];
        _tcscpy_s(directory, 2048, buffer);
        PathRemoveFileSpec(directory);
        std::wstring wstr(directory);
        auto dirStr =  std::string(wstr.begin(), wstr.end());
        dirStr = dirStr + "\\..\\..\\";
        auto filePath  = Util::FileDialogs::OpenFile(dirStr.c_str());
        Material mat = renderer->GetMainMaterialCopy();
        if(!filePath.empty())
        {
            std::ifstream readFile(filePath);
            if (readFile.is_open()) {
                nlohmann::json loadedJson;
                readFile >> loadedJson;
                mat.FromJson(loadedJson);
                renderer->SetMaterial(mat);
                printf("load file success\n");
            }
            else
            {
              printf("read file failed\n");
            }            
        }
    }
}

void StylePooPoo(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.34f, 0.35f, 0.94f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.60f, 0.60f, 0.60f, 0.56f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.51f, 0.51f, 0.51f, 0.56f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.70f, 0.76f, 0.31f, 0.80f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.70f, 0.76f, 0.31f, 0.80f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(1.00f, 1.00f, 1.00f, 0.45f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.60f, 0.69f, 0.01f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.70f, 0.76f, 0.31f, 0.92f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.70f, 0.76f, 0.31f, 0.80f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.52f, 0.75f, 0.62f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.22f, 0.59f, 0.71f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.70f, 0.76f, 0.31f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.31f);
    colors[ImGuiCol_Separator] = ImVec4(0.60f, 0.60f, 0.60f, 0.56f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.19f, 0.20f, 0.56f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.70f, 0.76f, 0.31f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.00f, 0.52f, 0.55f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);


    TCHAR buffer[2048];
    GetModuleFileName(NULL, buffer, sizeof(buffer));
    std::wcout << "path = " << buffer << std::endl;
    TCHAR directory[2048];
    _tcscpy_s(directory, 2048, buffer);
    PathRemoveFileSpec(directory);
    std::wstring wstr(directory);
    std::string dirStr(wstr.begin(), wstr.end());
    
    dirStr = dirStr + "\\..\\..\\resources/FOT-Seurat_Pro_DB.otf";
    ImGuiIO& io = ImGui::GetIO();
    ImFont* font2 = io.Fonts->AddFontFromFileTTF(dirStr.c_str(), 14);
}

std::string vec3ToString(const glm::vec3& vec) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return ss.str();
}

bool show_app_style_editor;
void on_gui(ImGuiIO& io, EricScene::Scene& scene, glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
    //IMGUI
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    //ImGui::ShowDemoWindow();
    ImGui::NewFrame();
    if(hide_ui)
        return;
    {
        ImGuizmo::BeginFrame();
        ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
        ImGuizmo::Enable(true);
        ImGui::Begin("Another window");
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
        {
            if (ImGui::BeginTabItem("Object"))
            {
                ImGui::Spacing();
                if (selectedObjectUUID != -1)
                {
                    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
                    glm::vec3 pos = scene._ObjectsMap[selectedObjectUUID]->localPosition;
                    ImGui::LabelText(vec3ToString(pos).c_str(), "Position : ");
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Scene"))
            {
                ImGui::Spacing();
                scene.OnGUI();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Material"))
            {
                if (selectedObjectUUID != -1)
                {
                    ImGui::Spacing();
                    ImGui::PushItemWidth(-200);
                    //ImGui::SetItemTooltip("I am a tooltip");
                    auto mat = scene._ObjectsMap[selectedObjectUUID]->renderer->GetMainMaterial();
                    DrawSaveMaterialButton(scene._ObjectsMap[selectedObjectUUID]->renderer);
                    ImGui::SameLine();
                    DrawLoadMaterialButton(scene._ObjectsMap[selectedObjectUUID]->renderer);
                    mat->DrawMaterial();
                    ImGuizmo::SetOrthographic(false);
                    ImGuizmo::AllowAxisFlip(false);
                    ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
                    ImGuizmo::SetRect(0, 0, 1440, 960);
                    auto modelMatrix = scene._ObjectsMap[selectedObjectUUID]->GetModelMatrix();
                    if (ImGuizmo::Manipulate(glm::value_ptr(viewMatrix), glm::value_ptr(projectionMatrix),
                                             ImGuizmo::TRANSLATE, ImGuizmo::LOCAL,
                                             glm::value_ptr(modelMatrix)))
                    {
                        std::cout << "some thing is moving" << std::endl;
                        scene._ObjectsMap[selectedObjectUUID]->SetLocalMatrix(modelMatrix);
                    }
                }

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
        

        static float f = 0.0f;
        static int counter = 0;
        ImVec2 vMin = ImGui::GetWindowContentRegionMin();
        ImVec2 vMax = ImGui::GetWindowContentRegionMax();

        vMin.x += ImGui::GetWindowPos().x;
        vMin.y += ImGui::GetWindowPos().y;
        vMax.x += ImGui::GetWindowPos().x;
        vMax.y += ImGui::GetWindowPos().y;
        auto open = true;
        bool body = true;
        auto constrainWindowSize = true;
        ImGui::SetNextWindowPos(ImVec2(0,0));
        if (constrainWindowSize) {
            ImGui::SetNextWindowSizeConstraints(ImVec2(200, 900), ImVec2(FLT_MAX, FLT_MAX));
        }
        ImGui::Begin("Inspector", &body,  ImGuiWindowFlags_HorizontalScrollbar);
        if(ImGui::CollapsingHeader("Debug Texture", &open))
        {
            ImGui::SeparatorText("");
            ImGui::BeginChild("Debug Texture", ImVec2(ImGui::GetWindowWidth() - 5, 25*7));
            ImGui::Checkbox("Visualize Bloom Mask", &Debug::DebugConfig::visualizeBloomMask);
            ImGui::Checkbox("Visualize Blur Pass", &Debug::DebugConfig::visualizeBlurPass);
            ImGui::Checkbox("Visualize Geometry Normal", &Debug::DebugConfig::visualizeNormal);
            ImGui::Checkbox("Visualize Geometry Position", &Debug::DebugConfig::visualizePosition);
            ImGui::Checkbox("Visualize Geometry Albedo", &Debug::DebugConfig::visualizeAlbedo);
            ImGui::Checkbox("Visualize Geometry Material Param", &Debug::DebugConfig::visualizeMaterialParams);
            ImGui::Checkbox("Visualize SSAO", &Debug::DebugConfig::visualizeSSAO);
            ImGui::Checkbox("Enable Test Logic", &Debug::DebugConfig::testLogic);
            ImGui::Checkbox("Visualize depth prepass ", &Debug::DebugConfig::visualizeDepthPrepass); 
            ImGui::Checkbox("Visualize Compute Shader Texture", &Debug::DebugConfig::visualizeComputeResultMap); 
            ImGui::Checkbox("Visualize RSM depth", &Debug::DebugConfig::visualizeRSMDepth);
            ImGui::Checkbox("Visualize RSM Position", &Debug::DebugConfig::visualizeRSMPosition);
            ImGui::Checkbox("Visualize RSM Normal", &Debug::DebugConfig::visualizeRSMNormal);
            ImGui::Checkbox("Visualize RSM Flux", &Debug::DebugConfig::visualizeRSMFlux);
            ImGui::Checkbox("Visualize Indirect Lighting", &Debug::DebugConfig::visualizeIndirectLighting);
            ImGui::EndChild();            
        }

        if(ImGui::CollapsingHeader("Shadow", &open))
        {
            ImGui::SeparatorText("");
            ImGui::BeginChild("Shadow Map Parameters",ImVec2( ImGui::GetWindowWidth() - 5, 25*8), true);
            ImGui::Checkbox("Visualize ShadowMap", &Debug::DebugConfig::visualizeShadowMap);
            ImGui::PushItemWidth(-150);
            ImGui::InputInt("debug cascade layer", &Debug::DebugConfig::shadowMapLayer);
            ImGui::SliderFloat("Normal Bias", &Debug::DebugConfig::normalBias, 0.0005f, 0.2f, "%.04f");
            ImGui::SliderFloat("Depth Bias", &Debug::DebugConfig::depthBias, 0.0005f, 0.5f);
            ImGui::InputFloat("Shadow Map MultZ", &Debug::DebugConfig::shadowMapMultZ, 1.0f, 1.0f);
            ImGui::SliderFloat("PCSS search radius", &Debug::DebugConfig::pcssSearchRadius,  0, 3.50f, "%.4f");
            ImGui::SliderFloat("PCSS filter radius", &Debug::DebugConfig::pcssFilterRadius,  0, 100.0f, "%.4f");
            ImGui::InputFloat("PCSS LightWorldSize", &Debug::DebugConfig::pcssLightWorldSize, 0.05, 0.05f);
            ImGuiColorEditFlags misc_flags = (ImGuiColorEditFlags_HDR| ( ImGuiColorEditFlags_NoDragDrop) | ImGuiColorEditFlags_NoOptions);
            ImGui::ColorEdit4("Penumbra Color", &Debug::DebugConfig::altShadowColor.x, misc_flags);
            ImGui::EndChild();
        }
        
        if(ImGui::CollapsingHeader("Light", &open))
        {
            ImGui::SeparatorText("");
            ImGui::BeginChild("Lighting Parameters", ImVec2( ImGui::GetWindowWidth() - 5, 25*5), true);
            ImGui::PushItemWidth(-150);
            ImGui::SliderFloat("Sun Intensity(Lux)", &Debug::DebugConfig::sunLux, 100, 130000);
            ImGui::SliderFloat("Environment Intensity(Lux)", &Debug::DebugConfig::envLux, 100, 130000);
            ImGui::Checkbox("Enable Tone Mapping", &Debug::DebugConfig::enableToneMapping);
            ImGui::SliderFloat("Tone mapping exposure", &Debug::DebugConfig::exposure, 0, 10.0);
            ImGui::EndChild();
        }

        if(ImGui::CollapsingHeader("Scene Objects", &open))
        {
            ImGui::SeparatorText("");
            ImGui::BeginChild("Scene Objects", ImVec2( 200, 5*25), true);
            for(auto i = 0; i < scene._AllSceneObjects.size();i++)
            {
                DisplayChildrenRecursive(*scene._AllSceneObjects[i], i, i*100);
            }
            ImGui::EndChild();
            ImGui::SameLine();
            // Right
            {
                ImGui::BeginGroup();
                ImGui::BeginChild("item view", ImVec2(0, 5*25), true); // Leave room for 1 line below us
                /*if(selectedObjectUUID != -1)
                {
                    ImVec2 buttonSize(ImGui::GetContentRegionAvail().x, 0);
                    ImGui::SliderFloat3("Position", (float*)&scene._ObjectsMap[selectedObjectUUID]->localPosition, -50,50, "%.3f");
                    ImGui::SliderFloat3("Rotation", (float*)&scene._ObjectsMap[selectedObjectUUID]->eulerRot, 0,360, "%.3f");
                }*/
                ImGui::Separator();
               /* if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
                {
                    if (ImGui::BeginTabItem("Description"))
                    {
                        ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Details"))
                    {
                        ImGui::Text("ID: 0123456789");
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }*/
                ImGui::EndChild();
                ImGui::EndGroup();
            }
        }
        if(ImGui::CollapsingHeader("Whatever", &open))
        {
            ImGui::BeginChild("Post Process");
            {
                if (ImGui::Button("Style"))
                {
                    show_app_style_editor = true;
                }
        
                if (show_app_style_editor)
                {
                    ImGui::Begin("Dear ImGui Style Editor", &show_app_style_editor);
                    ImGui::ShowStyleEditor();
                    ImGui::End();
                }

                ImGui::Checkbox("Enable Bloom", &Debug::DebugConfig::enableBloom);
                ImGui::SliderFloat("Bloom Threshold", &Debug::DebugConfig::bloomThreshold, 0,  1.0);
                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                ImGui::Checkbox("Enable AO", &Debug::DebugConfig::enableAO);
                ImGui::Checkbox("Use SSAO(default == HBAO)", &Debug::DebugConfig::useSSAO);
                ImGui::Checkbox("Half ResolutioAO", &Debug::DebugConfig::useHalfResolutioAO);
                ImGui::Checkbox("Use Bilateral Blur", &Debug::DebugConfig::useBilateralBlur);
                ImGui::InputFloat("Blur Radius", &Debug::DebugConfig::blurRadius, 1.0f, 1.0f);
                ImGui::InputFloat("Blur Sharpness", &Debug::DebugConfig::blurShaprness, 0.1f, 1.0f);
                ImGui::InputFloat("SSAO Random Size", &Debug::DebugConfig::aoRandomSize, 1.0f, 4.0f);
                ImGui::SliderFloat("SSAO Pow", &Debug::DebugConfig::aoPower, 1.0f, 4.0f);
                ImGui::SliderFloat("SSAO Strength", &Debug::DebugConfig::aoStrength, 0.1f, 3.0f);
                ImGui::SliderFloat("SSAO Radius", &Debug::DebugConfig::ssaoRadius, 1.5f, 80.0f);
                ImGui::SliderFloat("SSAO Bias", &Debug::DebugConfig::ssaoBias, -0.08f, 0.08f);

                ImGui::SliderFloat("RSM Max Radius", &Debug::DebugConfig::rsmMaxRadius, 0,  2.0);
                ImGui::SliderFloat("RSM Flux Stength", &Debug::DebugConfig::rsmFluxStrength, 0,  1.0);
                ImGui::EndChild();
            }
        }
        
        ImGui::End();
        
        ImGui::SetNextWindowPos(ImVec2(1440 - 340, 960 - 340));
        ImGui::Begin("Debug Textures");
      /*  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
           1000.0/(double)(ImGui::GetIO().Framerate), (double)(ImGui::GetIO().Framerate));*/
        // Get the size of the child (i.e. the whole draw size of the windows).
        ImVec2 wsize = ImVec2(340,340);
        // Because I use the texture from OpenGL, I need to invert the V from the UV.
        ImGui::Image((void*)Debug::DebugConfig::debugTextureId, wsize, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::End();
        
        ImGui::SetNextWindowPos(ImVec2(1440 - 150, 10));
        ImGui::Begin("Mouse Parameters");
        ImGui::Checkbox("Locked mouse", &mouseLock);
        ImGui::End();

    }

    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }
}

void MouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (isInitMouse)
    {
        lastX = xpos;
        lastY = ypos;
        isInitMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;
    
    if(!mouseLock)
    {
        camera.ProcessMouseEvent(xoffset, yoffset);
    }
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void ProcessInput(GLFWwindow *window, float delta)
{
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        mouseLock = true;
        lastLensX = lastX;
        lastLensY = lastY;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        mouseLock = false;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        hide_ui = !hide_ui;
    }
    
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::FORWARD, delta);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::BACKWARD, delta);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::LEFT, delta);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::RIGHT, delta);
}

GLuint loadTexture(const char *path)
{
    GLuint textureId = -1;
    int width, height, nrChannels;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format;
        GLenum internalFormat;
        bool useAlphaTexture = false;
        if (nrChannels == 1)
        {
            format = GL_RED;
            internalFormat =  GL_RED;
        }
        else if (nrChannels == 3)
        {
            format = GL_RGB;
            internalFormat = GL_SRGB;
        }
        else if (nrChannels == 4)
        {
            format = GL_RGBA;
            useAlphaTexture = true;
            internalFormat = GL_SRGB_ALPHA;
        }
        
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height,0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        if(!useAlphaTexture)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    return textureId;
}