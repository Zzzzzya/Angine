#include "Applications.hpp"

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

/* 场景 */

shared_ptr<Scene> scene = std::make_shared<Scene>();

/* 帧渲染时间 */
float deltaTime = 0;
float lastTime = 0;
float curTime = 0;

/* 光标位置 */
float LastCursorX = 0;
float LastCursorY = 0;

/* 处理&回调函数*/
bool CursorIsIn = true;
bool firstIn = false;

GLFWwindow *window = nullptr;

auto ProcessCursorPos = [](GLFWwindow *window, double xpos, double ypos) -> void {
    if (!CursorIsIn)
        return;

    if (!firstIn) {
        LastCursorX = xpos;
        LastCursorY = ypos;
        firstIn = true;
    }
    int offsetx = xpos - LastCursorX;
    int offsety = LastCursorY - ypos;
    LastCursorX = xpos;
    LastCursorY = ypos;

    scene->camera->ProcessCursorPos(offsetx, offsety);
};

auto ProcessScroll = [](GLFWwindow *window, double xoffset, double yoffset) {
    if (!CursorIsIn)
        return;
    scene->camera->ProcessScroll(yoffset);
};

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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

    glfwSetCursorPosCallback(window, ProcessCursorPos);
    glfwSetScrollCallback(window, ProcessScroll);
    // Mine
    glEnable(GL_DEPTH_TEST);

    Shader myshader("MVP_3.vs", "Model.fs");
    Shader LightShader("MVP_3.vs", "Light.fs");
    Shader EmptyPhoneShader("MVP_3.vs", "Empty_Blinn_Phone.fs");
    Shader PhoneShader("MVP_3.vs", "Blinn_Phone.fs");

    scene->models.push_back(std::make_shared<Model>("nanosuit/nanosuit.obj", PhoneShader));
    scene->models.push_back(std::make_shared<Model>("floor/floor.obj", EmptyPhoneShader));
    scene->pointLights.push_back(std::make_shared<PointLightModel>(LightShader));
    // 相机创建！

    scene->camera = std::make_shared<Camera>(vec3(0.0f, 5.0f, 5.0f));

    PointLight theLight;
    theLight.position = vec3(0.0f, 10.0f, 10.0f);
    theLight.ambient = vec3(0.2f, 0.2f, 0.2f);
    theLight.diffuse = vec3(0.5f, 0.5f, 0.5f);
    theLight.specular = vec3(0.0f, 1.0f, 1.0f);
    theLight.ones = 0.3;
    theLight.secs = 0.032;
    scene->pointLights[0]->light = theLight;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        curTime = glfwGetTime();
        deltaTime = curTime - lastTime;
        lastTime = curTime;

        ProcessKeyInput(window);
        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Your ImGui code here
        ImGui::Begin("Hello, world!");
        ImGui::Text("This is some useful text.");
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
                strncpy(filepath, szFile, sizeof(filepath));
                string file = string(filepath);
                replaceBackslashWithForwardslash(file);
                auto shader = Shader("MVP_3.vs", "Light.fs");
                scene->models.emplace_back(std::make_shared<Model>(file, shader, 1));
            }
        }

        ImGui::End();

        ImGui::Begin("Model List");
        ImGui::Text("Select a model to render:");
        static int current_model_index = 0;
        for (int i = 0; i < scene->models.size(); ++i) {
            if (ImGui::Selectable(scene->models[i]->name.c_str(), current_model_index == i)) {
                current_model_index = i;
            }
        }
        ImGui::End();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        // MYrender
        glClearColor(0.45f, 0.35f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4 projection(1.0f), view(1.0f), object(1.0f);
        view = scene->camera->ViewMat();
        projection = glm::perspective(radians(scene->camera->fov), (float)display_w / display_h, 0.1f, 1000.0f);

        for (auto &model : scene->models) {
            model->shader.use();
            model->shader.setMVPS(model->ModelMat(), view, projection);
            model->shader.setCam(scene->camera);
            model->shader.setPointLight(scene->pointLights[0]->light);
            model->shader.setMaterial(model->mat);
            model->Draw();
        }

        for (auto &light : scene->pointLights) {
            light->shader.use();
            light->shader.setMVPS(light->ModelMat(), view, projection);
            light->Draw();
        }
        // END Myrender

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        // Poll and handle events
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
