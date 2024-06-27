#include "Header.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Scene.hpp"
#include <GLFW/glfw3.h>
#include <Windows.h>
#include "Buffer.hpp"

using Camera::Movement::BACKWARD;
using Camera::Movement::FORWARD;
using Camera::Movement::LEFT;
using Camera::Movement::RIGHT;

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

/* 窗口大小 */
int imageWidth = 1600;
int imageHeight = 900;
int shadowWidth = 1024;
int shadowHeight = 1024;

/* 场景 */
shared_ptr<Scene> scene = std::make_shared<Scene>();

bool skyboxOn = false;
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
/* Screen */
shared_ptr<Shader> ScreenNothing;
shared_ptr<Shader> ScreenBlur;
shared_ptr<Shader> ScreenGrayScale;
shared_ptr<Shader> ScreenSharpen;

vector<shared_ptr<Shader>> MyShaders;
/* actual screenShader we r using */
shared_ptr<Shader> ScreenShader;

const static mat4 lightProjection = glm::perspective(radians(90.0f), 1.0f, 0.1f, 1000.0f);

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
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glEnable(GL_CULL_FACE);

    // 帧缓冲
    FrameBuffer framebuffer(imageWidth, imageHeight);
    FrameBufferDepthMap shadowMap(shadowWidth, shadowHeight);
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

    ScreenNothing = std::make_shared<Shader>("Nothing_vec2.vs", "Screen/Nothing_vec2.fs");
    ScreenBlur = std::make_shared<Shader>("Nothing_vec2.vs", "Screen/Blur.fs");
    ScreenGrayScale = std::make_shared<Shader>("Nothing_vec2.vs", "Screen/GrayScale.fs");
    ScreenSharpen = std::make_shared<Shader>("Nothing_vec2.vs", "Screen/Sharpen.fs");

    ScreenShader = ScreenNothing;
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
    // scene->models.push_back(std::make_shared<Model>("nanosuit/nanosuit.obj", RefractShader));
    scene->models.push_back(std::make_shared<Model>("mari/Marry.obj", Phong_ShadowMapShader));
    scene->models.push_back(std::make_shared<Model>("floor/bigfloor.obj", Phong_ShadowMapShader));
    // scene->models.push_back(std::make_shared<Model>("floor/floor.obj", ReflectShader));

    scene->pointLights.push_back(std::make_shared<PointLightModel>(LightShader));
    scene->pointLights[scene->pointLights.size() - 1]->name = "Light" + std::to_string(scene->pointLights.size() - 1);

    scene->pointLights[scene->pointLights.size() - 1]->scale = vec3(0.1f);
    //  // 相机创建！

    scene->camera = std::make_shared<Camera>(vec3(0.0f, 5.0f, 10.0f));

    PointLight theLight;
    theLight.position = vec3(0.0f, 5.0f, 3.0f);
    theLight.ambient = vec3(0.0f, 0.0f, 0.0f);
    theLight.diffuse = vec3(0.5f, 0.5f, 0.5f);
    theLight.specular = vec3(1.0f, 1.0f, 1.0f);
    theLight.ones = 0.00;
    theLight.secs = 0.00;
    scene->pointLights[0]->light = theLight;

    /* 🫣 天空盒 */
    skybox = std::make_shared<CubeMap>(faces);
    skyboxArrayMesh = std::make_shared<ArrayMesh>(skyboxVertex);

    int frameCount = 0;

    // Main loop
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
            simpleDepthShader->setMat4("model", model->ModelMat());
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
        // ------------  State 1 ------------- 渲染至帧缓冲 👌👌👌👌
        glEnable(GL_FRAMEBUFFER_SRGB);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.FrameBufferID); /* 🫣 绑定帧缓冲*/
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.0f, 0.0f, 0.0f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // 绘制天空盒
        if (skyboxOn) {
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_STENCIL_TEST);
            glDisable(GL_CULL_FACE);
            SkyboxShader->use();
            SkyboxShader->setMat4("view", mat4(glm::mat3(view)));
            SkyboxShader->setMat4("projection", projection);
            SkyboxShader->setInt("skybox", 0);
            glBindVertexArray(skyboxArrayMesh->VAO);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->id);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glDepthMask(GL_TRUE);
            glBindVertexArray(0);
        }
        // 渲染

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glEnable(GL_CULL_FACE);

        glActiveTexture(GL_TEXTURE30);
        glBindTexture(GL_TEXTURE_2D, shadowMap.depthMapTexture);
        glActiveTexture(GL_TEXTURE0);
        MainRender(view, projection);

        // --------------- State 1 End -----------------
        glBindFramebuffer(GL_FRAMEBUFFER, 0); //  解绑 返回默认帧缓冲

        // --------------- State 2 ---------- 渲染到屏幕上
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_CULL_FACE);

        glClearColor(1.0f, 1.0f, 1.0f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        ScreenShader->use();
        glBindVertexArray(quadMesh->VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffer.TextureColorBuffer);

        ScreenShader->setInt("screenTexture", 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glDisable(GL_FRAMEBUFFER_SRGB);
        // END Myrender

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
        model->shader->setMVPS(model->ModelMat(), view, projection);
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
        model->shader->setVec4("ObjectColor", model->ObjectColor);
        model->Draw();
    }

    if (current_model_index != -1) {
        auto &model = scene->models[current_model_index];
        glStencilMask(0xff);
        glStencilFunc(GL_ALWAYS, 1, 0xff);
        model->shader->use();
        model->shader->setMVPS(model->ModelMat(), view, projection);
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
        model->shader->setVec4("ObjectColor", model->ObjectColor);
        model->Draw();

        // 绘制边框
        auto preShader = model->shader;
        model->shader = BorderShader;
        glStencilMask(0x00);
        glStencilFunc(GL_NOTEQUAL, 1, 0XFF);
        glDisable(GL_DEPTH_TEST);

        model->shader->use();
        model->shader->setMVPS(model->ModelMat(), view, projection);
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
            scene->models.emplace_back(std::make_shared<Model>(file, PhoneShader, 1));
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
