#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// include glad

// glfw error callback
static void glfw_error_callback(int error, const char* description)
{
    std::cerr << "Glfw Error " << error << ": " << description << std::endl;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(int, char**)
{
    // set up glfw
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit())
        return -1;
    
    // create window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Hello, world!", NULL, NULL);
    if (!window)
        return -1;
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSwapInterval(1); // enable vsync

    // initialize glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL loader!" << std::endl;
        return -1;
    }

    // setup imgui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf", 21);

    // setup imgui style
    ImGui::StyleColorsDark();
    ImGui::GetStyle().ScaleAllSizes(1.5f); // Scale everything by 1.5


    // setup platform/renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");


    // main loop
    while (!glfwWindowShouldClose(window))
    {
        // poll and handle events
        glfwPollEvents();

        // start the imgui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // create imgui window
        ImGui::Begin("Hello, world!");
        ImGui::Text("This is a simple Hello, World!");
        ImGui::Text("This is a simple Hello, World! application too.");
        ImGui::End();

        ImGui::ShowDemoWindow();

        ImGui::Begin("Viewport");
        ImGui::End();

        // rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // swap buffers
        glfwSwapBuffers(window);
    }

    // cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

