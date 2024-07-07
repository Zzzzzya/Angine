#include "Header.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Scene.hpp"
#include <glm/gtc/random.hpp>
#include <GLFW/glfw3.h>
#include <Windows.h>
#include "Buffer.hpp"
#include <random>

using Camera::Movement::BACKWARD;
using Camera::Movement::FORWARD;
using Camera::Movement::LEFT;
using Camera::Movement::RIGHT;

enum RenderMode {
    FORWARD_RENDER = 0,
    DEFERRED_RENDER
};

RenderMode renderMode = DEFERRED_RENDER;
unsigned int renderTexture = 0;

static void glfw_error_callback(int error, const char *description) {
    std::cerr << "Glfw Error " << error << ": " << description << std::endl;
}

static void replaceBackslashWithForwardslash(std::string &str) {
    size_t pos = 0;
    while ((pos = str.find("\\", pos)) != std::string::npos) {
        str.replace(pos, 1, "/");
        pos += 1; // Move past the replaced "/"
    }
}

const std::string ProjectModelsPath = "D:\\Files\\Code\\Graphics\\projects\\MyEngine\\res\\models\\";
float ssrPos = 0.0f;

/* 窗口大小 */
int imageWidth = 1600;
int imageHeight = 900;
int shadowWidth = 1024;
int shadowHeight = 1024;

/* 场景 */
shared_ptr<Scene> scene = std::make_shared<Scene>();

bool skyboxOn = true;
shared_ptr<CubeMap> skybox;
shared_ptr<ArrayMesh> skyboxArrayMesh;

vector<Vertex> skyboxVertex = {
    {{-1.0f, 1.0f, -1.0f}, {}, {}},  {{-1.0f, -1.0f, -1.0f}, {}, {}}, {{1.0f, -1.0f, -1.0f}, {}, {}},
    {{-1.0f, 1.0f, -1.0f}, {}, {}},  {{1.0f, -1.0f, -1.0f}, {}, {}},  {{1.0f, 1.0f, -1.0f}, {}, {}},
    {{-1.0f, -1.0f, 1.0f}, {}, {}},  {{-1.0f, -1.0f, -1.0f}, {}, {}}, {{-1.0f, 1.0f, -1.0f}, {}, {}},
    {{-1.0f, 1.0f, -1.0f}, {}, {}},  {{-1.0f, 1.0f, 1.0f}, {}, {}},   {{-1.0f, -1.0f, 1.0f}, {}, {}},
    {{1.0f, -1.0f, -1.0f}, {}, {}},  {{1.0f, -1.0f, 1.0f}, {}, {}},   {{1.0f, 1.0f, 1.0f}, {}, {}},
    {{1.0f, 1.0f, 1.0f}, {}, {}},    {{1.0f, 1.0f, -1.0f}, {}, {}},   {{1.0f, -1.0f, -1.0f}, {}, {}},
    {{-1.0f, -1.0f, 1.0f}, {}, {}},  {{-1.0f, 1.0f, 1.0f}, {}, {}},   {{1.0f, 1.0f, 1.0f}, {}, {}},
    {{1.0f, 1.0f, 1.0f}, {}, {}},    {{1.0f, -1.0f, 1.0f}, {}, {}},   {{-1.0f, -1.0f, 1.0f}, {}, {}},
    {{-1.0f, 1.0f, -1.0f}, {}, {}},  {{1.0f, 1.0f, -1.0f}, {}, {}},   {{1.0f, 1.0f, 1.0f}, {}, {}},
    {{1.0f, 1.0f, 1.0f}, {}, {}},    {{-1.0f, 1.0f, 1.0f}, {}, {}},   {{-1.0f, 1.0f, -1.0f}, {}, {}},
    {{-1.0f, -1.0f, -1.0f}, {}, {}}, {{-1.0f, -1.0f, 1.0f}, {}, {}},  {{1.0f, -1.0f, -1.0f}, {}, {}},
    {{1.0f, -1.0f, -1.0f}, {}, {}},  {{-1.0f, -1.0f, 1.0f}, {}, {}},  {{1.0f, -1.0f, 1.0f}, {}, {}}};

vector<Vertex2D> ScreenVertex2D = {{vec2(-1.0f, 1.0f), vec2(0.0f, 1.0f)}, {vec2(-1.0f, -1.0f), vec2(0.0f, 0.0f)},
                                   {vec2(1.0f, -1.0f), vec2(1.0f, 0.0f)}, {vec2(-1.0f, 1.0f), vec2(0.0f, 1.0f)},
                                   {vec2(1.0f, -1.0f), vec2(1.0f, 0.0f)}, {vec2(1.0f, 1.0f), vec2(1.0f, 1.0f)}};
vector<Vertex2D> BackMirrorVertex2D = {{vec2(-0.5f, 1.0f), vec2(0.0f, 1.0f)}, {vec2(-0.5f, 0.5f), vec2(0.0f, 0.0f)},
                                       {vec2(0.5f, 0.5f), vec2(1.0f, 0.0f)},  {vec2(-0.5f, 1.0f), vec2(0.0f, 1.0f)},
                                       {vec2(0.5f, 0.5f), vec2(1.0f, 0.0f)},  {vec2(0.5f, 1.0f), vec2(1.0f, 1.0f)}};
shared_ptr<QuadMesh2D> quadMesh;
shared_ptr<QuadMesh2D> backMirror;

/* 帧渲染时间 */
float deltaTime = 0;
float lastTime = 0;
float curTime = 0;

/* 光标位置 */
float LastCursorX = 0;
float LastCursorY = 0;

/* 处理&回调函数*/
bool CursorIsIn = false;
bool firstIn = false;

/* 当前窗口 */
GLFWwindow *window = NULL;

/* 当前选中的模型 */
int current_model_index = -1;

/* 当前选中的光照 */
int current_light_index = -1;

/* 帧数 */
float fps = 0.0f;

/* Shaders */
/* Light */
shared_ptr<Shader> LightShader;
/* Special */
shared_ptr<Shader> BorderShader;
shared_ptr<Shader> NormalShader;
shared_ptr<Shader> ZdepthShader;
shared_ptr<Shader> SkyboxShader;
shared_ptr<Shader> ReflectShader;
shared_ptr<Shader> RefractShader;
shared_ptr<Shader> simpleDepthShader;
/* Reality */
shared_ptr<Shader> EmptyPhoneShader;
shared_ptr<Shader> PhoneShader;
shared_ptr<Shader> BlinnPhongShader;
shared_ptr<Shader> Phong_ShadowMapShader;
shared_ptr<Shader> PbrShader;
/* Screen */
shared_ptr<Shader> ScreenNothing;
shared_ptr<Shader> ScreenBlur;
shared_ptr<Shader> ScreenGrayScale;
shared_ptr<Shader> ScreenSharpen;

shared_ptr<Shader> depthShader;

/* Render */
shared_ptr<Shader> GBufferShader;
shared_ptr<Shader> D_PhongShader;
shared_ptr<Shader> D_Phong_SSR_Shader;
shared_ptr<Shader> ssaoShader;
shared_ptr<Shader> ssaoBlurShader;
shared_ptr<Shader> ssrShader;

shared_ptr<Shader> HDR2cubeShader;
shared_ptr<Shader> irradianceShader;

shared_ptr<Shader> prefilterShader;

shared_ptr<Shader> D_Shader;
vector<shared_ptr<Shader>> MyShaders;
/* actual screenShader we r using */
shared_ptr<Shader> ScreenShader;

const static mat4 lightProjection = glm::perspective(radians(90.0f), 1.0f, 0.1f, 1000.0f);
const static mat4 lightCubeProjection = glm::perspective(radians(90.0f), (1600.0f / 900.0f), 0.1f, 1000.0f);

/* 天空盒路径 */
vector<string> faces = {"skybox/right.jpg",  "skybox/left.jpg",  "skybox/top.jpg",
                        "skybox/bottom.jpg", "skybox/front.jpg", "skybox/back.jpg"};

auto ProcessKeyInput = [](GLFWwindow *window) -> void {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        if (CursorIsIn) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            CursorIsIn = false;
            firstIn = false;
        }
    }
    if (!CursorIsIn)
        return;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        scene->camera->ProcessKeyBoard(FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        scene->camera->ProcessKeyBoard(BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        scene->camera->ProcessKeyBoard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        scene->camera->ProcessKeyBoard(RIGHT, deltaTime);
    }
};
auto ProcessCursorPos = [](GLFWwindow *window, double xpos, double ypos) -> void {
    if (!CursorIsIn) {
        return;
    }

    if (!firstIn) {
        LastCursorX = static_cast<float>(xpos);
        LastCursorY = static_cast<float>(ypos);
        firstIn = true;
    }
    float offsetx = static_cast<float>(xpos - LastCursorX);
    float offsety = static_cast<float>(LastCursorY - ypos);
    LastCursorX = static_cast<float>(xpos);
    LastCursorY = static_cast<float>(ypos);

    scene->camera->ProcessCursorPos(offsetx, offsety);
};
auto ProcessScroll = [](GLFWwindow *window, double xoffset, double yoffset) -> void {
    if (!CursorIsIn)
        return;
    scene->camera->ProcessScroll(yoffset);
};

/* IMGUI页面 */
void AppMainFunction();
void AppModelList();
void AppModelEdit();
void AppLightList();
void AppLightEdit();

/* 渲染过程！！ 🤩🤩🤩🤩🤩 */
void MainRender(const mat4 &, const mat4 &);
void renderCube();
void renderQuad();
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
unsigned int quadVAO = 0;
unsigned int quadVBO;

/* ASSAO */
std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // 随机浮点数，范围0.0 - 1.0
std::default_random_engine generator;
std::vector<glm::vec3> ssaoKernel;
std::vector<glm::vec3> ssaoNoise;
bool ssaoOn = true;

int main(int argc, char **argv) {
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // // Decide GL+GLSL versions
    const char *glsl_version = "#version 330";

    // Create window with GLFW
    window = glfwCreateWindow(imageWidth, imageHeight, "Angine", NULL, NULL);
    if (window == NULL)
        return -1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // 回调函数绑定！ -- 鼠标移动 && 滚轮
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetScrollCallback(window, ProcessScroll);
    glfwSetCursorPosCallback(window, ProcessCursorPos);
    // Initialize OpenGL loader (GLAD in this case)
    glewInit();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Mine
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // 帧缓冲
    FrameBuffer framebuffer(imageWidth, imageHeight);

    // shadowMap
    FrameBufferDepthMap shadowMap(shadowWidth, shadowHeight);
    DepthCube shadowMapCube;

    /*😆 帧缓冲使用过程：
        绑定帧缓冲 --> 渲染到这个帧缓冲上
        绑定默认的帧缓冲 --> 绘制一个整个屏幕的四边形 将帧缓冲的颜色缓冲作为他的纹理
    */
    quadMesh = std::make_shared<QuadMesh2D>(ScreenVertex2D);
    backMirror = std::make_shared<QuadMesh2D>(BackMirrorVertex2D);
    /*
     😍 因此我们要建立一个四边形的Mesh 👆
    */

    /* SShaders 🤣 初始化*/
    LightShader = std::make_shared<Shader>("MVP_3.vs", "Light/Light.fs");

    BorderShader = std::make_shared<Shader>("Border.vs", "Special/Border.fs");
    NormalShader = std::make_shared<Shader>("MVP_3.vs", "Special/Normal.fs");
    ZdepthShader = std::make_shared<Shader>("MVP_depth.vs", "Special/Z-Depth.fs");
    SkyboxShader = std::make_shared<Shader>("SkyBox.vs", "Special/SkyBox.fs");
    ReflectShader = std::make_shared<Shader>("reflect.vs", "Special/reflect.fs");
    RefractShader = std::make_shared<Shader>("reflect.vs", "Special/refract.fs");
    simpleDepthShader = std::make_shared<Shader>("simpleDepth.vs", "Special/empty.fs");

    BlinnPhongShader = std::make_shared<Shader>("MVP_3.vs", "Reality/Traditional/BlinnPhong.fs");
    PhoneShader = std::make_shared<Shader>("MVP_3.vs", "Reality/Traditional/Phone.fs");
    Phong_ShadowMapShader = std::make_shared<Shader>("MVP_4_shadowMap.vs", "HighLevel/Shadow/shadowMap.fs");
    PbrShader = std::make_shared<Shader>("MVP_4_shadowMap.vs", "Reality/PBR/PBR0.fs");

    ScreenNothing = std::make_shared<Shader>("Nothing_vec2.vs", "Screen/Nothing_vec2.fs");
    ScreenBlur = std::make_shared<Shader>("Nothing_vec2.vs", "Screen/Blur.fs");
    ScreenGrayScale = std::make_shared<Shader>("Nothing_vec2.vs", "Screen/GrayScale.fs");
    ScreenSharpen = std::make_shared<Shader>("Nothing_vec2.vs", "Screen/Sharpen.fs");

    HDR2cubeShader = std::make_shared<Shader>("HDR2cube.vs", "Other/HDR2cube.fs");
    irradianceShader = std::make_shared<Shader>("HDR2cube.vs", "pbr/Convolution.fs");
    prefilterShader = std::make_shared<Shader>("HDR2cube.vs", "pbr/preFilter.fs");

    depthShader = std::make_shared<Shader>("depth.vs", "depth.fs", true, "depth.gs");

    GBufferShader = std::make_shared<Shader>("D_MVP_SHADOW.vs", "render/Gbuffer.fs");

    // Defer
    D_PhongShader = std::make_shared<Shader>("Nothing_vec2.vs", "Defer/Phong.fs");
    D_Phong_SSR_Shader = std::make_shared<Shader>("Nothing_vec2.vs", "Defer/Phong_SSR.fs");
    ssaoShader = std::make_shared<Shader>("Nothing_vec2.vs", "Defer/SSAO.fs");
    ssrShader = std::make_shared<Shader>("Nothing_vec2.vs", "Defer/SSR.fs");
    ssaoBlurShader = std::make_shared<Shader>("Nothing_vec2.vs", "Defer/ssaoBlur.fs");

    ScreenShader = ScreenNothing;
    D_Shader = D_PhongShader;
    MyShaders.push_back(BlinnPhongShader);
    MyShaders.push_back(Phong_ShadowMapShader);
    MyShaders.push_back(ZdepthShader);
    MyShaders.push_back(NormalShader);
    MyShaders.push_back(LightShader);
    MyShaders.push_back(ReflectShader);
    MyShaders.push_back(RefractShader);

    /* 甘🐟和地板 🥵🥵🥵 */
    // scene->models.push_back(
    //     std::make_shared<Model>("genshin_impact_obj/Ganyu model/Ganyu model.pmx", Phong_ShadowMapShader));
    // scene->models[0]->scale = vec3(0.2);

    // scene->models.push_back(std::make_shared<Model>("Sponza/Sponza.fbx", D_PhongShader));
    // scene->models[0]->scale = vec3(0.015f);

    scene->models.push_back(std::make_shared<Model>("Texture Shool/anime school.obj", D_PhongShader));
    // scene->models.push_back(std::make_shared<Model>("floor/bigfloor.obj", Phong_ShadowMapShader));
    //  scene->models.push_back(std::make_shared<Model>("floor/floor.obj", PbrShader));

    // Texture albedo("rust/albedo.png");
    // Texture metallic("rust/metallic.png");
    // Texture roughness("rust/roughness.png");

    // albedo.type = "texture_albedo";
    // metallic.type = "texture_metallic";
    // roughness.type = "texture_roughness";

    // int nrRows = 3;
    // int nrColumns = 3;
    // float spacing = 2.5;
    // for (int row = 0; row < nrRows; ++row) {
    //     for (int col = 0; col < nrColumns; ++col) {

    //         scene->models.push_back(std::make_shared<Model>("nanosuit/nanosuit.obj", PhoneShader));
    //         auto &curModel = scene->models[scene->models.size() - 1];
    //         curModel->translate = vec3(-3 + row * 8, 0, 3 - col * 8);
    //     }
    // }

    /* SScene */
    scene->pointLights.push_back(std::make_shared<PointLightModel>(LightShader));
    scene->pointLights[scene->pointLights.size() - 1]->name = "Light" + std::to_string(scene->pointLights.size() - 1);
    scene->pointLights[scene->pointLights.size() - 1]->light.color = vec3(300.0f, 300.0f, 300.0f);

    //  // 相机创建！

    scene->camera = std::make_shared<Camera>(vec3(-11.243f, 1.925f, 4.037f));

    PointLight theLight;
    theLight.color = vec3(300.0f, 300.0f, 300.0f);
    theLight.ones = 0.00;
    theLight.secs = 0.00;
    theLight.position = vec3(11.f, 10.f, 0.f);
    // theLight.position = vec3(-16.f, 6.f, 0.f);
    // theLight.position = vec3(6.0f, 21.0f, 14.0f);
    scene->pointLights[0]->light = theLight;

    /* shadowMap2 */

    /* 🫣 天空盒 */
    skybox = std::make_shared<CubeMap>(faces);
    skyboxArrayMesh = std::make_shared<ArrayMesh>(skyboxVertex);

    unsigned int captureFBO;
    unsigned int captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    Texture_HDR hdr("hdr3.hdr");
    // // pbr: load the HDR environment map
    // // ---------------------------------
    // auto hdrTexture = Texture_HDR::hdr();

    // // pbr: setup cubemap to render to and attach to framebuffer
    // // ---------------------------------------------------------
    unsigned int envCubemap;
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // // pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
    // // ----------------------------------------------------------------------------------------------
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

    // pbr: convert HDR equirectangular environment map to cubemap equivalent
    // ----------------------------------------------------------------------
    HDR2cubeShader->use();
    HDR2cubeShader->setInt("equirectangularMap", 0);
    HDR2cubeShader->setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdr.id);

    glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i) {
        HDR2cubeShader->setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 以上做了两件事情 读取TEXTURE hdr --> 2D  -->渲染到一个cubemap上  叫env
    // 接下来做卷积！
    unsigned int irradianceMap;
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (int i = 0; i < 6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    irradianceShader->use();
    irradianceShader->setInt("environmentMap", 0);
    irradianceShader->setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (unsigned int i = 0; i < 6; ++i) {
        irradianceShader->setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap,
                               0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderCube();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 以上是diffuse部分

    // 以下是specular部分
    unsigned int prefilterMap;
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    prefilterShader->use();
    prefilterShader->setInt("environmentMap", 0);
    prefilterShader->setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
        // reisze framebuffer according to mip-level size.
        unsigned int mipWidth = 128 * std::pow(0.5, mip);
        unsigned int mipHeight = 128 * std::pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader->setFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i) {
            prefilterShader->setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                   prefilterMap, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderCube();
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Texture tex1png("1.png");

    // BRDF

    unsigned int brdfLUTTexture;
    glGenTextures(1, &brdfLUTTexture);

    // pre-allocate enough memory for the LUT texture.
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    glViewport(0, 0, 512, 512);
    auto brdfShader = std::make_shared<Shader>("brdf.vs", "pbr/BRDFfilter.fs");
    brdfShader->use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //
    glActiveTexture(GL_TEXTURE18);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    glActiveTexture(GL_TEXTURE17);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    glActiveTexture(GL_TEXTURE16);
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

    PbrShader->use();
    PbrShader->setInt("irradianceMap", 18);
    PbrShader->setInt("prefilterMap", 17);
    PbrShader->setInt("brdfLUT", 16);
    glActiveTexture(GL_TEXTURE0);

    /* MSSAO */
    auto lerp = [](GLfloat a, GLfloat b, GLfloat f) -> GLfloat { return a + f * (b - a); };
    for (int i = 0; i < 64; i++) {
        glm::vec3 sample(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f,
                         randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        GLfloat scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }
    for (int i = 0; i < 16; i++) {
        glm::vec3 sample(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f,
                         randomFloats(generator));
        ssaoNoise.push_back(sample);
    }
    unsigned int noiseTexture;
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

    unsigned int ssaoFBO;
    glGenFramebuffers(1, &ssaoFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

    unsigned int ssaoColorBuffer;
    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1600, 900, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint ssaoBlurFBO, ssaoColorBufferBlur;
    glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1600, 900, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);

    /* MSSR */
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);

    const int numSamples = 64; // 要生成的采样点数量

    std::vector<glm::vec3> ssrSamples;
    ssrSamples.reserve(numSamples);

    for (int i = 0; i < numSamples; ++i) {
        // 生成球面坐标
        glm::vec3 sample = glm::sphericalRand(1.0f); // 生成在单位球面上的随机点
        ssrSamples.push_back(sample);
    }

    unsigned int ssrFBO;
    glGenFramebuffers(1, &ssrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);

    unsigned int ssrTexture;
    glGenTextures(1, &ssrTexture);
    glBindTexture(GL_TEXTURE_2D, ssrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 1600, 900, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssrTexture, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /* GB */
    GBuffer gBuffer;
    renderTexture = gBuffer.gPosition;

    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glEnable(GL_CULL_FACE);

    int frameCount = 0;

    /* RWhile */
    while (!glfwWindowShouldClose(window)) {
        curTime = glfwGetTime();
        deltaTime = curTime - lastTime;
        if ((frameCount++) % 100 == 0)
            fps = 1 / deltaTime;
        lastTime = curTime;

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Your ImGui code here

        AppMainFunction();
        AppModelList();
        AppModelEdit();
        AppLightList();
        AppLightEdit();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        if (display_h < 1)
            display_h = 1;
        glViewport(0, 0, display_w, display_h);
        mat4 projection(1.0f), view(1.0f);

        // MYrender
        if (renderMode == FORWARD_RENDER) {

            // ---- shadow map ----
            // shadow map
            glEnable(GL_DEPTH_TEST);

            glViewport(0, 0, shadowWidth, shadowHeight);
            glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);

            vec3 lightFront = vec3(0.0f) - scene->pointLights[0]->light.position;
            auto right = normalize(glm::cross(lightFront, vec3(0.0f, 1.0f, 0.0f)));
            vec3 lightUp = normalize(glm::cross(right, lightFront));
            auto lightView = glm::lookAt(scene->pointLights[0]->light.position, vec3(0.0f), lightUp);

            simpleDepthShader->use();
            simpleDepthShader->setMat4("lightProjection", lightProjection);
            simpleDepthShader->setMat4("lightView", lightView);

            for (auto &model : scene->models) {
                simpleDepthShader->setMat4("model", model->ModelMat(curTime));
                for (auto &mesh : model->meshes) {
                    glBindVertexArray(mesh.VAO);
                    glDrawElements(GL_TRIANGLES, (unsigned int)mesh.indices.size(), GL_UNSIGNED_INT, 0);
                    glBindVertexArray(0);
                }
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // shadow map end

            view = scene->camera->ViewMat();
            projection = glm::perspective(radians(scene->camera->fov), (float)display_w / display_h, 0.1f, 100.0f);
            // ------------  State 1 -------------
            glEnable(GL_FRAMEBUFFER_SRGB);

            glBindFramebuffer(GL_FRAMEBUFFER, 0); /* 🫣 绑定帧缓冲*/
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.5f, 0.0f, 0.0f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            // 绘制天空盒
            // if (skyboxOn) {
            //     glDisable(GL_DEPTH_TEST);
            //     glDisable(GL_STENCIL_TEST);
            //     glDisable(GL_CULL_FACE);
            //     SkyboxShader->use();
            //     SkyboxShader->setMat4("view", mat4(glm::mat3(view)));
            //     SkyboxShader->setMat4("projection", projection);
            //     SkyboxShader->setInt("skybox", 0);
            //     glActiveTexture(GL_TEXTURE0);
            //     glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
            //     renderCube();
            //     glDepthMask(GL_TRUE);
            //     glBindVertexArray(0);
            // }
            // 渲染

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_STENCIL_TEST);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glEnable(GL_CULL_FACE);

            glActiveTexture(GL_TEXTURE30);
            glBindTexture(GL_TEXTURE_2D, shadowMap.depthMapTexture);
            glActiveTexture(GL_TEXTURE0);

            MainRender(view, projection);

            glDepthFunc(GL_LEQUAL); // change depth function so depth test passes when values are equal to depth
                                    // buffer's content
            if (skyboxOn) {
                glDisable(GL_STENCIL_TEST);
                glDisable(GL_CULL_FACE);
                SkyboxShader->use();
                SkyboxShader->setMat4("view", mat4(glm::mat3(view)));
                SkyboxShader->setMat4("projection", projection);
                SkyboxShader->setInt("skybox", 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
                renderCube();
                // glDepthMask(GL_TRUE);
                glBindVertexArray(0);
            }
            glDepthFunc(GL_LESS);
        }
        else if (renderMode == DEFERRED_RENDER) {
            // ---- shadow map ----
            // shadow map
            glEnable(GL_DEPTH_TEST);

            glViewport(0, 0, shadowWidth, shadowHeight);
            glBindFramebuffer(GL_FRAMEBUFFER, shadowMap.depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);

            vec3 lightFront = vec3(0.0f) - scene->pointLights[0]->light.position;
            auto right = normalize(glm::cross(lightFront, vec3(0.0f, 1.0f, 0.0f)));
            vec3 lightUp = normalize(glm::cross(right, lightFront));
            auto lightView = glm::lookAt(scene->pointLights[0]->light.position, vec3(0.0f), lightUp);

            simpleDepthShader->use();
            simpleDepthShader->setMat4("lightProjection", lightProjection);
            simpleDepthShader->setMat4("lightView", lightView);

            for (auto &model : scene->models) {
                simpleDepthShader->setMat4("model", model->ModelMat(curTime));
                for (auto &mesh : model->meshes) {
                    glBindVertexArray(mesh.VAO);
                    glDrawElements(GL_TRIANGLES, (unsigned int)mesh.indices.size(), GL_UNSIGNED_INT, 0);
                    glBindVertexArray(0);
                }
            }

            //  shadow cube

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // shadow map end
            glViewport(0, 0, display_w, display_h);
            view = scene->camera->ViewMat();
            projection = glm::perspective(radians(scene->camera->fov), (float)display_w / display_h, 0.1f, 100.0f);

            // ----------- Gbuffer ---------------
            glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.gBuffer);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glEnable(GL_DEPTH_TEST);
            glDisable(GL_STENCIL_TEST);
            glDisable(GL_CULL_FACE);

            glActiveTexture(GL_TEXTURE30);
            glBindTexture(GL_TEXTURE_2D, shadowMap.depthMapTexture);
            glActiveTexture(GL_TEXTURE0);

            GBufferShader->use();
            GBufferShader->setCam(scene->camera);
            GBufferShader->setInt("shadowMap", 30);

            for (int i = 0; i < scene->pointLights.size(); i++) {
                vec3 lightFront = vec3(0.0f) - scene->pointLights[i]->light.position;
                auto right = normalize(glm::cross(lightFront, vec3(0.0f, 1.0f, 0.0f)));
                vec3 lightUp = normalize(glm::cross(right, lightFront));
                auto lightView = glm::lookAt(scene->pointLights[i]->light.position, vec3(0.0f), lightUp);
                GBufferShader->setMat4("lightSpaceMatrix", lightProjection * lightView);
            }

            for (auto &model : scene->models) {
                GBufferShader->setMVPS(model->ModelMat(curTime), view, projection);
                GBufferShader->setMaterial(model->mat);
                for (auto &mesh : model->meshes) {
                    mesh.Draw(GBufferShader);
                }
            }

            for (auto &light : scene->pointLights) {
                light->updatePosition(curTime);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // -------- SSAO ---------
            glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
            glClearColor(0.0f, 0.0f, 1.0f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            ssaoShader->use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gBuffer.gPosition);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gBuffer.gNormal);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, gBuffer.gAlbedoSpec);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, gBuffer.gShadowMap);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, noiseTexture);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, gBuffer.gViewPos);

            ssaoShader->setMat4("projection", projection);
            ssaoShader->setMat4("view", view);
            for (int i = 0; i < 64; i++)
                ssaoShader->setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);

            ssaoShader->setInt("gPosition", 0);
            ssaoShader->setInt("gNormal", 1);
            ssaoShader->setInt("gAlbedoSpec", 2);
            ssaoShader->setInt("gShadowMap", 3);
            ssaoShader->setInt("texNoise", 4);
            ssaoShader->setInt("gViewPos", 5);
            quadMesh->Draw(ssaoShader);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // --------- SSAO BLUR -----------
            glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
            glClearColor(0.0f, 0.0f, 1.0f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            ssaoBlurShader->use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
            for (int i = 0; i < 64; i++)
                ssaoShader->setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);

            ssaoShader->setInt("ssao", 0);
            quadMesh->Draw(ssaoBlurShader);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // ------------ SSR ----------------
            glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);
            glClearColor(0.0f, 0.0f, 1.0f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);
            ssrShader->use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gBuffer.gPosition);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gBuffer.gNormal);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, gBuffer.gAlbedoSpec);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, gBuffer.gShadowMap);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, noiseTexture);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, gBuffer.gViewPos);

            ssrShader->setMat4("projection", projection);
            ssrShader->setMat4("view", view);
            ssrShader->setVec3("cameraPos", scene->camera->position);
            for (int i = 0; i < 64; i++)
                ssaoShader->setVec3("samples[" + std::to_string(i) + "]", ssrSamples[i]);
            ssrShader->setInt("gPosition", 0);
            ssrShader->setInt("gNormal", 1);
            ssrShader->setInt("gAlbedoSpec", 2);
            ssrShader->setInt("gShadowMap", 3);
            ssrShader->setInt("texNoise", 4);
            ssrShader->setInt("gViewPos", 5);

            quadMesh->Draw(ssrShader);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // ------------  State 1 -------------
            glEnable(GL_FRAMEBUFFER_SRGB);

            glBindFramebuffer(GL_FRAMEBUFFER, 0); /* 🫣 绑定帧缓冲*/

            glClearColor(0.0f, 0.0f, 1.0f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_STENCIL_TEST);
            glDisable(GL_CULL_FACE);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            D_Shader->use();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gBuffer.gPosition);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, gBuffer.gNormal);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, gBuffer.gAlbedoSpec);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, gBuffer.gShadowMap);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, ssrTexture);

            D_Shader->setInt("gPosition", 0);
            D_Shader->setInt("gNormal", 1);
            D_Shader->setInt("gAlbedoSpec", 2);
            D_Shader->setInt("gShadowMap", 3);
            D_Shader->setInt("ssao", 4);
            D_Shader->setInt("ssaoOn", ssaoOn);
            D_Shader->setInt("ssr", 5);
            D_Shader->setFloat("ssrPow", ssrPos);

            D_Shader->setMat4("projection", projection);
            D_Shader->setCam(scene->camera);
            // 多光源设置
            D_Shader->setInt("lightNum", scene->pointLights.size());
            for (int i = 0; i < scene->pointLights.size(); i++) {
                D_Shader->setPointLight(i, scene->pointLights[i]->light);
            }

            quadMesh->Draw(D_Shader);

            // 渲染其余物体（正向）
            glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.gBuffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBlitFramebuffer(0, 0, 1600, 900, 0, 0, 1600, 900, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glEnable(GL_DEPTH_TEST);
            for (auto &light : scene->pointLights) {
                light->shader->use();
                light->shader->setMVPS(light->ModelMat(), view, projection);
                light->Draw();
            }

            glDepthFunc(GL_LEQUAL);

            if (skyboxOn) {
                glDisable(GL_STENCIL_TEST);
                glDisable(GL_CULL_FACE);
                SkyboxShader->use();
                SkyboxShader->setMat4("view", mat4(glm::mat3(view)));
                SkyboxShader->setMat4("projection", projection);
                SkyboxShader->setInt("skybox", 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
                renderCube();
                glBindVertexArray(0);
            }
            glDepthFunc(GL_LESS);

            // MainRender(view, projection);
        }
        // --------------- State 1 End -----------------
        // glBindFramebuffer(GL_FRAMEBUFFER, 0); //  解绑 返回默认帧缓冲

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        // Poll and handle events
        glfwPollEvents();
        ProcessKeyInput(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

/* Render */
void MainRender(const mat4 &view, const mat4 &projection) {

    int lightIndex = 0;
    glStencilMask(0x00);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    for (auto &light : scene->pointLights) {
        light->updatePosition(curTime);
        light->shader->use();
        light->shader->setMVPS(light->ModelMat(), view, projection);
        light->Draw();
    }

    int modelIndex = 0;
    glStencilMask(0x00);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    for (auto &model : scene->models) {
        if (modelIndex++ == current_model_index) {
            continue;
        }
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        model->shader->use();
        model->shader->setMVPS(model->ModelMat(curTime), view, projection);
        model->shader->setCam(scene->camera);
        // 多光源设置
        model->shader->setInt("lightNum", scene->pointLights.size());
        for (int i = 0; i < scene->pointLights.size(); i++) {
            vec3 lightFront = vec3(0.0f) - scene->pointLights[i]->light.position;
            auto right = normalize(glm::cross(lightFront, vec3(0.0f, 1.0f, 0.0f)));
            vec3 lightUp = normalize(glm::cross(right, lightFront));
            auto lightView = glm::lookAt(scene->pointLights[i]->light.position, vec3(0.0f), lightUp);
            model->shader->setMat4("lightSpaceMatrix", lightProjection * lightView);
            model->shader->setPointLight(i, scene->pointLights[i]->light);
        }
        model->shader->setInt("shadowMap", 30);
        model->shader->setMaterial(model->mat);
        model->shader->setPbr(model->pbr);
        model->shader->setVec4("ObjectColor", model->ObjectColor);
        model->Draw();
    }

    if (current_model_index != -1) {
        auto &model = scene->models[current_model_index];
        glStencilMask(0xff);
        glStencilFunc(GL_ALWAYS, 1, 0xff);
        model->shader->use();
        model->shader->setMVPS(model->ModelMat(curTime), view, projection);
        model->shader->setCam(scene->camera);
        // 多光源设置
        model->shader->setInt("lightNum", scene->pointLights.size());
        for (int i = 0; i < scene->pointLights.size(); i++) {
            vec3 lightFront = vec3(0.0f) - scene->pointLights[i]->light.position;
            auto right = normalize(glm::cross(lightFront, vec3(0.0f, 1.0f, 0.0f)));
            vec3 lightUp = normalize(glm::cross(right, lightFront));
            auto lightView = glm::lookAt(scene->pointLights[i]->light.position, vec3(0.0f), lightUp);
            model->shader->setMat4("lightSpaceMatrix", lightProjection * lightView);
            model->shader->setPointLight(i, scene->pointLights[i]->light);
        }
        model->shader->setInt("shadowMap", 30);
        model->shader->setMaterial(model->mat);
        model->shader->setPbr(model->pbr);
        model->shader->setVec4("ObjectColor", model->ObjectColor);
        model->Draw();

        // 绘制边框
        auto preShader = model->shader;
        model->shader = BorderShader;
        glStencilMask(0x00);
        glStencilFunc(GL_NOTEQUAL, 1, 0XFF);
        glDisable(GL_DEPTH_TEST);

        model->shader->use();
        model->shader->setMVPS(model->ModelMat(curTime), view, projection);
        model->Draw();
        model->shader = preShader;

        glStencilMask(0xFF);
        glStencilFunc(GL_ALWAYS, 1, 0x00);
        glEnable(GL_DEPTH_TEST);
    }
}

void RenderAll(const mat4 &view, const mat4 &projection) {
    int lightIndex = 0;
    glStencilMask(0x00);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    for (auto &light : scene->pointLights) {
        light->updatePosition(curTime);
        light->shader->use();
        light->shader->setMVPS(light->ModelMat(), view, projection);
        light->Draw();
    }

    int modelIndex = 0;
    glStencilMask(0x00);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    for (auto &model : scene->models) {
        if (modelIndex++ == current_model_index) {
            continue;
        }
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        model->shader->use();
        model->shader->setMVPS(model->ModelMat(), view, projection);
        model->shader->setCam(scene->camera);
        // 多光源设置
        model->shader->setInt("lightNum", scene->pointLights.size());
        for (int i = 0; i < scene->pointLights.size(); i++) {
            model->shader->setPointLight(i, scene->pointLights[i]->light);
        }
        model->shader->setMaterial(model->mat);
        model->shader->setVec4("ObjectColor", model->ObjectColor);
        model->Draw();
    }
}
/* ImGui */
/*
    主功能界面：
    包括：   进入摄像机
            导入模型 - 默认使用光照模型（纯白）
*/
void AppMainFunction() {
    ImGui::Begin("MainFunction");
    ImGui::Text("FPS: %.1f", fps);
    if (ImGui::Button("InMode")) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        CursorIsIn = true;
    }

    if (ImGui::Button("Forward Render")) {
        renderMode = FORWARD_RENDER;
    }

    if (ImGui::Button("DEFERRED Render")) {
        renderMode = DEFERRED_RENDER;
    }

    if (ImGui::Button("SSAO")) {
        ssaoOn = !ssaoOn;
    }

    ImGui::SliderFloat("ssr", &ssrPos, 0.0f, 1.0f);

    static float a;
    ImGui::SliderFloat("a", &a, 0.0f, 10.0f);
    Phong_ShadowMapShader->use();
    Phong_ShadowMapShader->setFloat("a", a);

    static char filepath[256] = ""; // 存储文件路径的缓冲区
    if (ImGui::Button("Import")) {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = {0};

        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = szFile;
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn) == TRUE) {
            // 用户选择了文件，将文件路径复制到缓冲区中
            strncpy_s(filepath, sizeof(filepath), szFile, _TRUNCATE);
            string file = string(filepath);
            replaceBackslashWithForwardslash(file);
            for (auto &model : scene->models) {
                if (model->name == file) {
                    auto n = model->name[model->name.size() - 1];
                    file += n + 1;
                    break;
                }
            }
            scene->models.emplace_back(std::make_shared<Model>(file, Phong_ShadowMapShader, 1));
            current_model_index = 0;
        }
    }

    if (ImGui::Button("Add Light")) {
        scene->pointLights.push_back(std::make_shared<PointLightModel>(LightShader));
        scene->pointLights[scene->pointLights.size() - 1]->name =
            "Light" + std::to_string(scene->pointLights.size() - 1);
    }

    if (ImGui::Button("skyBox switch")) {
        skyboxOn = !skyboxOn;
    }

    if (ImGui::CollapsingHeader("Screen Effects")) {
        if (ImGui::Button("Nothing")) {
            ScreenShader = ScreenNothing;
        }
        if (ImGui::Button("Blur")) {
            ScreenShader = ScreenBlur;
        }
        if (ImGui::Button("GrayScale")) {
            ScreenShader = ScreenGrayScale;
        }
        if (ImGui::Button("Sharpen")) {
            ScreenShader = ScreenSharpen;
        }
    }

    ImGui::End();
}

void AppModelList() {
    ImGui::Begin("Model List");
    ImGui::Text("Select a model to render:");
    bool flag = true;
    if (ImGui::Selectable("None", current_model_index == -1)) {
        current_model_index = -1;
    }
    for (int i = 0; i < scene->models.size(); ++i) {
        if (ImGui::Selectable(scene->models[i]->name.c_str(), current_model_index == i)) {
            current_model_index = i;
        }
    }

    ImGui::End();
}

void AppModelEdit() {
    if (current_model_index == -1)
        return;
    ImGui::Begin("Model Edit");
    auto curModel = scene->models[current_model_index];
    ImGui::Text("Selected Model : %s", curModel->name.c_str());

    if (ImGui::Button("转")) {
        auto curTime = glfwGetTime();
        curModel->spin = !curModel->spin;
    }

    if (ImGui::CollapsingHeader("Position")) {
        ImGui::SliderFloat("translate.x", &curModel->translate.x, -10.0f, 10.0f);
        ImGui::SliderFloat("translate.y", &curModel->translate.y, -10.0f, 10.0f);
        ImGui::SliderFloat("translate.z", &curModel->translate.z, -10.0f, 10.0f);
    }

    if (ImGui::CollapsingHeader("Scale")) {
        ImGui::SliderFloat("scale", &curModel->scale.x, 0.0f, 2.5f);
        ImGui::SliderFloat("scale", &curModel->scale.y, 0.0f, 2.5f);
        ImGui::SliderFloat("scale", &curModel->scale.z, 0.0f, 2.5f);
    }

    if (ImGui::CollapsingHeader("Rotate")) {
        ImGui::SliderFloat("rotate.x", &curModel->rotate.x, -180.0f, 180.0f);
        ImGui::SliderFloat("rotate.y", &curModel->rotate.y, -180.0f, 180.0f);
        ImGui::SliderFloat("rotate.z", &curModel->rotate.z, -180.0f, 180.0f);
    }

    if (ImGui::CollapsingHeader("Shader")) {
        static const char *shader_items[] = {"Blinn Phong", "Phong",   "Z-depth", "Normal",
                                             "Light",       "Reflect", "Refract"};
        static int current_shader_index = 0;
        if (ImGui::Combo("Select Shader", &current_shader_index, shader_items, IM_ARRAYSIZE(shader_items))) {
            // 当选择发生变化时，执行相应的逻辑
            // 例如：切换Shader
            curModel->shader = MyShaders[current_shader_index];
        }
    }

    if (ImGui::CollapsingHeader("Material")) {
        ImGui::SliderFloat("diffuse", &curModel->mat.diffuse.x, -1.0f, 1.0f);
        ImGui::SliderFloat("diffuse", &curModel->mat.diffuse.y, -1.0f, 1.0f);
        ImGui::SliderFloat("diffuse", &curModel->mat.diffuse.z, -1.0f, 1.0f);
        ImGui::SliderFloat("specular", &curModel->mat.specular.x, -1.0f, 1.0f);
        ImGui::SliderFloat("specular", &curModel->mat.specular.y, -1.0f, 1.0f);
        ImGui::SliderFloat("specular", &curModel->mat.specular.z, -1.0f, 1.0f);
        ImGui::SliderFloat("ambient", &curModel->mat.ambient.x, -1.0f, 1.0f);
        ImGui::SliderFloat("ambient", &curModel->mat.ambient.y, -1.0f, 1.0f);
        ImGui::SliderFloat("ambient", &curModel->mat.ambient.z, -1.0f, 1.0f);
        ImGui::SliderFloat("shininess", &curModel->mat.shininess, 0.0f, 100.0f);
        ImGui::SliderFloat("ObjectColor.r", &curModel->ObjectColor.r, 0.0f, 1.0f);
        ImGui::SliderFloat("ObjectColor.g", &curModel->ObjectColor.g, 0.0f, 1.0f);
        ImGui::SliderFloat("ObjectColor.b", &curModel->ObjectColor.b, 0.0f, 1.0f);
        ImGui::SliderFloat("ObjectColor.a", &curModel->ObjectColor.a, 0.0f, 1.0f);
    }

    if (ImGui::CollapsingHeader("pbr")) {
        ImGui::SliderFloat("albedo.x", &curModel->pbr.albedo.x, 0.0f, 1.0f);
        ImGui::SliderFloat("albedo.y", &curModel->pbr.albedo.y, 0.0f, 1.0f);
        ImGui::SliderFloat("albedo.z", &curModel->pbr.albedo.z, 0.0f, 1.0f);

        ImGui::SliderFloat("metallic", &curModel->pbr.metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("roughness", &curModel->pbr.roughness, 0.0f, 1.0f);
        ImGui::SliderFloat("ao", &curModel->pbr.ao, 0.0f, 1.0f);
    }

    if (ImGui::CollapsingHeader("Texture")) {
        ImGui::Text("Texture num: %zu", curModel->textures_loaded.size());
        // 导入纹理
        static char filepath[256] = ""; // 存储文件路径的缓冲区
        // 选择纹理种类
        static bool texType[4] = {true, false, false, false};
        static string texTypeString[4] = {"diffuse", "specular", "normal", "height"};
        for (int i = 0; i < 4; i++) {
            ImGui::Checkbox((texTypeString[i] + "_tex").c_str(), &texType[i]);
        }

        if (ImGui::Button("Import Tex")) {
            OPENFILENAMEA ofn;
            CHAR szFile[260] = {0};

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = NULL;
            ofn.lpstrFile = szFile;
            ofn.lpstrFile[0] = '\0';
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

            if (GetOpenFileNameA(&ofn) == TRUE) {
                // 用户选择了文件，将文件路径复制到缓冲区中
                int p = -1;
                for (int i = 0; i < 4; i++) {
                    if (texType[i]) {
                        p = i;
                        break;
                    }
                }
                if (p == -1) {
                    std::clog << "ERROR::TEXTURE::SELECT TEXTURE TYPE FIRST" << std::endl;
                }

                strncpy_s(filepath, sizeof(filepath), szFile, _TRUNCATE);
                string file = string(filepath);
                replaceBackslashWithForwardslash(file);
                auto filenameLoc = file.find_last_of('/');

                Texture texture(file.substr(filenameLoc + 1));
                texture.type = "texture_" + texTypeString[p];
                texture.path = file.substr(0, filenameLoc) + '/';
                curModel->meshes[0].textures.push_back(texture);
                curModel->textures_loaded.push_back(texture);
            }
        }

        if (ImGui::Button("Delete Tex")) {
            if (curModel->meshes[0].textures.size()) {
                curModel->meshes[0].textures.pop_back();
                curModel->textures_loaded.pop_back();
            }
            else
                std::clog << "Delete Tex Error!" << std::endl;
        }
    }
    ImGui::End();
}

void AppLightList() {
    ImGui::Begin("Light List");
    ImGui::Text("Select a Light to edit:");
    for (int i = 0; i < scene->pointLights.size(); ++i) {
        if (ImGui::Selectable(scene->pointLights[i]->name.c_str(), current_light_index == i)) {
            current_light_index = i;
        }
    }

    ImGui::End();
}

void AppLightEdit() {
    if (current_light_index == -1)
        return;
    ImGui::Begin("Light Edit");
    auto curLight = scene->pointLights[current_light_index];
    ImGui::Text("Selected light : %s", curLight->name.c_str());

    if (ImGui::CollapsingHeader("Edit Position")) {
        ImGui::SliderFloat("translate.x", &curLight->light.position.x, -50.0f, 50.0f);
        ImGui::SliderFloat("translate.y", &curLight->light.position.y, -50.0f, 50.0f);
        ImGui::SliderFloat("translate.z", &curLight->light.position.z, -50.0f, 50.0f);
    }

    if (ImGui::CollapsingHeader("Edit Scale")) {
        ImGui::SliderFloat("scale", &curLight->scale.x, 0.0f, 2.5f);
        ImGui::SliderFloat("scale", &curLight->scale.y, 0.0f, 2.5f);
        ImGui::SliderFloat("scale", &curLight->scale.z, 0.0f, 2.5f);
    }

    if (ImGui::CollapsingHeader("Edit Rotate")) {
        ImGui::SliderFloat("rotate.x", &curLight->rotate.x, -180.0f, 180.0f);
        ImGui::SliderFloat("rotate.y", &curLight->rotate.y, -180.0f, 180.0f);
        ImGui::SliderFloat("rotate.z", &curLight->rotate.z, -180.0f, 180.0f);
    }

    if (ImGui::CollapsingHeader("Edit light")) {
        ImGui::SliderFloat("diffuse", &curLight->light.diffuse.r, -1.0f, 1.0f);
        ImGui::SliderFloat("diffuse", &curLight->light.diffuse.g, -1.0f, 1.0f);
        ImGui::SliderFloat("diffuse", &curLight->light.diffuse.b, -1.0f, 1.0f);
        ImGui::SliderFloat("specular", &curLight->light.specular.r, -1.0f, 1.0f);
        ImGui::SliderFloat("specular", &curLight->light.specular.g, -1.0f, 1.0f);
        ImGui::SliderFloat("specular", &curLight->light.specular.b, -1.0f, 1.0f);
        ImGui::SliderFloat("ambient", &curLight->light.ambient.r, -1.0f, 1.0f);
        ImGui::SliderFloat("ambient", &curLight->light.ambient.g, -1.0f, 1.0f);
        ImGui::SliderFloat("ambient", &curLight->light.ambient.b, -1.0f, 1.0f);
        ImGui::SliderFloat("color.x", &curLight->light.color.x, 0.0f, 300.0f);
        ImGui::SliderFloat("color.y", &curLight->light.color.y, 0.0f, 300.0f);
        ImGui::SliderFloat("color.z", &curLight->light.color.z, 0.0f, 300.0f);
        ImGui::SliderFloat("oncs", &curLight->light.ones, 0.0f, 0.5f);
        ImGui::SliderFloat("secs", &curLight->light.secs, 0.0f, 0.05f);
    }

    if (ImGui::CollapsingHeader("Shader")) {
        static const char *shader_items[] = {"Blinn Phong", "Phong",   "Z-depth", "Normal",
                                             "Light",       "Reflect", "Refract"};
        static int current_shader_index = 0;
        if (ImGui::Combo("Select Shader", &current_shader_index, shader_items, IM_ARRAYSIZE(shader_items))) {
            // 当选择发生变化时，执行相应的逻辑
            // 例如：切换Shader
            curLight->shader = MyShaders[current_shader_index];
        }
    }

    if (ImGui::Button("GoRound-Y")) {
        curLight->goRoundY = !curLight->goRoundY;
    }

    ImGui::End();
}

void renderCube() {
    // initialize (if necessary)
    if (cubeVAO == 0) {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
            1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,  // bottom-right
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,  // top-left
            // front face
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
            -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
            -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top-left
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
            -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
                                                                // right face
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
            1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,    // top-right
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,   // bottom-right
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,     // top-left
            1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,    // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // top-left
            1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
            1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
            -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
            -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
            1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // top-right
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f   // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void renderQuad() {
    if (quadVAO == 0) {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}