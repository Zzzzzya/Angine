#include "Header.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#include "Shader.hpp"
#include "Camera.hpp"

#include <iostream>

static void glfw_error_callback(int error, const char *description) {
    std::cerr << "Glfw Error " << error << ": " << description << std::endl;
}

/* 窗口大小 */
int imageWidth = 1600;
int imageHeight = 900;

/* 视口相机 */
using Camera::Movement::BACKWARD;
using Camera::Movement::FORWARD;
using Camera::Movement::LEFT;
using Camera::Movement::RIGHT;
shared_ptr<Camera> TheCam;

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
        TheCam->ProcessKeyBoard(FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        TheCam->ProcessKeyBoard(BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        TheCam->ProcessKeyBoard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        TheCam->ProcessKeyBoard(RIGHT, deltaTime);
    }
};

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

    TheCam->ProcessCursorPos(offsetx, offsety);
};

auto ProcessScroll = [](GLFWwindow *window, double xoffset, double yoffset) {
    if (!CursorIsIn)
        return;
    TheCam->ProcessScroll(yoffset);
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
    GLFWwindow *window = glfwCreateWindow(imageWidth, imageHeight, "Angine", NULL, NULL);
    if (window == NULL)
        return -1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // 回调函数绑定！ -- 鼠标移动 && 滚轮
    glfwSetCursorPosCallback(window, ProcessCursorPos);
    glfwSetScrollCallback(window, ProcessScroll);
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

    // Mine
    glEnable(GL_DEPTH_TEST);
    float vertex[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    glBindVertexArray(0);

    Shader myshader("MVP_0.vs", "SingleColor.fs");
    myshader.use();
    auto color = glm::vec4(0.2f, 0.3f, 0.6f, 1.0f);
    myshader.setVec4("singleColor", color);

    // 相机创建！
    TheCam = std::make_shared<Camera>(vec3(0.0f, 0.0f, 3.0f));

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
        ImGui::End();

        ImGui::Begin("Hello, world!");
        ImGui::Text("This is some useful text.");
        if (ImGui::Button("InMode")) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            CursorIsIn = true;
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

        myshader.use();

        mat4 projection(1.0f), view(1.0f), model(1.0f);
        view = TheCam->ViewMat();
        projection = glm::perspective(radians(TheCam->fov), (float)display_w / display_h, 0.1f, 1000.0f);
        myshader.setMat4("model", model);
        myshader.setMat4("view", view);
        myshader.setMat4("projection", projection);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
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
