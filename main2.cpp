#include "json.hpp"
using json = nlohmann::json;
json config;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <ft2build.h>
#include <map>
#include <iostream>
#include <fstream>
// #include "shaders.hpp"
// #include "FrameRateCalculator.hpp"
#include "InputController.hpp"
#include "Text.hpp"
#include <fmt/format.h>

GLuint shader_program;
GLuint text_shader_program;

float offsetX = 0;
float offsetY = 0;
int width = 0;
int height = 0;

void set_projection() {
    glm::mat4 projection = glm::ortho(-(float)config["padding"], (float)width-(float)config["padding"], float(height)-(float)config["padding"], -(float)config["padding"], -1.0f, 1.0f); // adjust for top-left origin

    glUseProgram(shader_program);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glUseProgram(text_shader_program);
    glUniformMatrix4fv(glGetUniformLocation(text_shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void compile_shader(GLuint shader, const char* source) {
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetShaderInfoLog(shader, 512, nullptr, info_log);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << info_log << std::endl;
    }
}

GLuint create_shader_program(const char* vert_src, const char* frag_src) {
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    compile_shader(vertex_shader, vert_src);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    compile_shader(fragment_shader, frag_src);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    int success;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(shader_program, 512, nullptr, info_log);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return shader_program;
}


void read_config_json(json& config) {
    std::ifstream input_file("config.json");
    input_file >> config;
}


void setSelectedCellColor(GLuint shader_program, const glm::vec4& color, const glm::vec2& pos, const glm::vec2& size) {
    GLint selectedCellColorLoc = glGetUniformLocation(shader_program, "selectedCellColor");
    GLint selectedCellPosLoc = glGetUniformLocation(shader_program, "selectedCellPos");
    GLint cellSizeLoc = glGetUniformLocation(shader_program, "cellSize");

    glUseProgram(shader_program);
    glUniform4fv(selectedCellColorLoc, 1, glm::value_ptr(color));
    glUniform2fv(selectedCellPosLoc, 1, glm::value_ptr(pos));
    glUniform2fv(cellSizeLoc, 1, glm::value_ptr(size));
}

void changeSelectedCell(std::vector<float>& grid_vertices, glm::vec2& selectedCellPos, const glm::vec2& oldCellPos) {
    size_t n = grid_vertices.size();
    for (int j = n-2; j >= n-16; j -= 2) {
        grid_vertices[j] += (selectedCellPos.x - oldCellPos.x);
        grid_vertices[j+1] += (selectedCellPos.y - oldCellPos.y);
    }
}

int main() {
    std::vector<std::string> event_log;
    FrameRateCalculator framerate_calculator;

    read_config_json(config);
    // initialize glfw
    if (!glfwInit()) {
        return -1;
    }

    // configure glfw
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create window
    GLFWwindow* window = glfwCreateWindow(config["width"], config["height"], "Grid", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // initialize glad
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        return -1;
    }

    // set viewport
    glViewport(0,0, config["width"], config["height"]);

    auto framebuffer_size_callback = [](GLFWwindow* window, int w, int h) {
        glViewport(0.0f, 0.0f, w, h);

        width = w; height = h;

        set_projection();
    };

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    shader_program = create_shader_program(shaders::vertex_shader_source, shaders::fragment_shader_source);
    text_shader_program = create_shader_program(shaders::text_vertex_shader_source, shaders::text_fragment_shader_source);

    load_font_textures(config["font"].get<std::string>().c_str());

    glUseProgram(shader_program);

    width = config["width"];
    height = config["height"];
    set_projection();


    std::vector<float> grid_vertices;
    float max_x = (float)config["num_rows"] * (float)config["row_height"];
    float max_y = (float)config["num_cols"] * (float)config["col_width"];
    for (int i = 0; i <= config["num_rows"]; ++i) {
        float y = i * (float)config["row_height"];
        float ndc_y = y;
        grid_vertices.push_back(max_x);
        grid_vertices.push_back(ndc_y);
        grid_vertices.push_back(0.0f);
        grid_vertices.push_back(ndc_y);
    }
    for (int j = 0; j <= config["num_cols"]; ++j) {
        float x = j * (float)config["col_width"];
        float ndc_x = x;
        grid_vertices.push_back(ndc_x);
        grid_vertices.push_back(max_y);
        grid_vertices.push_back(ndc_x);
        grid_vertices.push_back(0.0f);
    }

    glm::vec2 selectedCellPos = glm::vec2(3 * 100.f, 4 * 30.f);

    grid_vertices.push_back(selectedCellPos.x+1);
    grid_vertices.push_back(selectedCellPos.y+1);
    grid_vertices.push_back(selectedCellPos.x + 1);
    grid_vertices.push_back(selectedCellPos.y + 29.f);

    grid_vertices.push_back(selectedCellPos.x+1);
    grid_vertices.push_back(selectedCellPos.y+1);
    grid_vertices.push_back(selectedCellPos.x + 99.f);
    grid_vertices.push_back(selectedCellPos.y + 1);

    grid_vertices.push_back(selectedCellPos.x+1);
    grid_vertices.push_back(selectedCellPos.y+29.f);
    grid_vertices.push_back(selectedCellPos.x + 99.f);
    grid_vertices.push_back(selectedCellPos.y + 29.f);

    grid_vertices.push_back(selectedCellPos.x+99.f);
    grid_vertices.push_back(selectedCellPos.y+1);
    grid_vertices.push_back(selectedCellPos.x + 99.f);
    grid_vertices.push_back(selectedCellPos.y + 29.f);

    // grid_vertices.insert(grid_vertices.end(), { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 });




    // create vbo and vao
    GLuint vbo, vao;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, grid_vertices.size() * sizeof(float), grid_vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    setSelectedCellColor(shader_program, glm::vec4(0.f, 0.25f, 1.f, 1.0f), selectedCellPos, glm::vec2(100.f, 30.f));

    auto keyupdate_callback = [&vbo, &grid_vertices, &selectedCellPos]() {
        glm::vec2 oldCellPos = selectedCellPos;
        selectedCellPos = glm::vec2(offsetX * 100.f, offsetY * 30.f);

        changeSelectedCell(grid_vertices, selectedCellPos , oldCellPos);
        setSelectedCellColor(shader_program, glm::vec4(0.f, 0.25f, 1.f, 1.0f), selectedCellPos, glm::vec2(100.f, 30.f));

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, grid_vertices.size() * sizeof(float), grid_vertices.data(), GL_STATIC_DRAW);
    };


    GLuint text_vao, text_vbo;
    glGenVertexArrays(1, &text_vao);
    glGenBuffers(1, &text_vbo);
    glBindVertexArray(text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // setup imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // set clear color to white for better visibility
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);


    InputController input_controller(window);

    input_controller.subscribe([=](GLFWwindow* window, int key, int scancode, int action, int mods) {
        switch (key) {
            case GLFW_KEY_LEFT:
                switch (action) {
                    case GLFW_PRESS:
                    case GLFW_REPEAT:
                        offsetX -= 1;
                        offsetX = offsetX > 0 ? offsetX : 0;
                        keyupdate_callback();
                        break;
                    default:
                        break;
                }
                break;

            case GLFW_KEY_RIGHT:
                switch (action) {
                    case GLFW_PRESS:
                    case GLFW_REPEAT:
                        offsetX += 1;
                        keyupdate_callback();
                        break;
                    default:
                        break;
                }
                break;

            case GLFW_KEY_UP:
                switch (action) {
                    case GLFW_PRESS:
                    case GLFW_REPEAT:
                        offsetY -= 1;
                        offsetY = offsetY > 0 ? offsetY : 0;
                        keyupdate_callback();
                        break;
                    default:
                        break;
                }
                break;

            case GLFW_KEY_DOWN:
                switch (action) {
                    case GLFW_PRESS:
                    case GLFW_REPEAT:
                        offsetY += 1;
                        keyupdate_callback();
                        break;
                    default:
                        break;
                }
                break;

            default:
                break;
        }
    });

    // render loop
    while (!glfwWindowShouldClose(window)) {
        input_controller.waitEvents();

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            event_log.push_back("Mouse Left Button Pressed at (" + std::to_string(xpos) + ", " + std::to_string(ypos) + ")");
        }

        // start imgui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // render
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader_program);
        glBindVertexArray(vao);
        glDrawArrays(GL_LINES, 0, grid_vertices.size() / 2);



        //////////////////////// Render text /////////////////////
        std::ostringstream textInput;
        textInput << "Hello, OpenGL! Compiled at " << __TIME__;
        std::string text = std::move(textInput.str());

        draw_text_in_cell(text_shader_program, text_vao, text_vbo, text, 2, 2);
        draw_text_in_cell(text_shader_program, text_vao, text_vbo, std::string("D4"), 4,4);
        /////////////////////////Render text /////////////////////
        
        ImGui::Begin("Framerate", nullptr, ImGuiWindowFlags_NoMove);
        ImGui::SetWindowPos(ImVec2(0,0));

        framerate_calculator.frame();
        ImGui::Text("%f", framerate_calculator.getFPS());

        ImGui::End();



        ImGui::Begin("Event Log", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SetWindowPos(ImVec2(io.DisplaySize.x - ImGui::GetWindowWidth(), io.DisplaySize.y - ImGui::GetWindowHeight()));

        for (auto event = event_log.size() > 100 ? event_log.end()-100 : event_log.begin(); event != event_log.end(); event++) {
            ImGui::Text("%s", event->c_str());
        }

        ImGui::End();

        // render imgui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shader_program);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

