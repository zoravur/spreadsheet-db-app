//
// Created by zoravur on 6/21/24.
//

#ifndef VIEW_HPP
#define VIEW_HPP

#include "Text.hpp"
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
#include "Model.hpp"
#include <fmt/format.h>

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

void setSelectedCellColor(GLuint shader_program, const glm::vec4& color, const glm::vec2& pos, const glm::vec2& size) {
    GLint selectedCellColorLoc = glGetUniformLocation(shader_program, "selectedCellColor");
    GLint selectedCellPosLoc = glGetUniformLocation(shader_program, "selectedCellPos");
    GLint cellSizeLoc = glGetUniformLocation(shader_program, "cellSize");

    glUseProgram(shader_program);
    glUniform4fv(selectedCellColorLoc, 1, glm::value_ptr(color));
    glUniform2fv(selectedCellPosLoc, 1, glm::value_ptr(pos));
    glUniform2fv(cellSizeLoc, 1, glm::value_ptr(size));

    glUniform4f(glGetUniformLocation(shader_program, "shadowColor"), 0.f, 0.25f, 1.f, .3f);
    glUniform1f(glGetUniformLocation(shader_program, "shadowSize"), config["shadow_size"].get<float>());

}

void changeSelectedCell(std::vector<float>& grid_vertices, const glm::vec2& selectedCellPos) {
    float shadow_size = config["shadow_size"].get<float>();
    float row_height = config["row_height"].get<float>();
    float col_width = config["col_width"].get<float>();
    std::vector replacement = {
        selectedCellPos.x-shadow_size, selectedCellPos.y+0.f-shadow_size,
        selectedCellPos.x-shadow_size, selectedCellPos.y+row_height+shadow_size,
        selectedCellPos.x+col_width+shadow_size, selectedCellPos.y+0.f-shadow_size,
        selectedCellPos.x-shadow_size, selectedCellPos.y+row_height+shadow_size,
        selectedCellPos.x+col_width+shadow_size, selectedCellPos.y+0.f-shadow_size,
        selectedCellPos.x+col_width+shadow_size, selectedCellPos.y+row_height+shadow_size
    };

    std::copy(replacement.begin(), replacement.end(), grid_vertices.begin());

    // grid_vertices.erase(grid_vertices.end()-16, grid_vertices.end());
    //
    // grid_vertices.push_back(selectedCellPos.x+1);
    // grid_vertices.push_back(selectedCellPos.y+1);
    // grid_vertices.push_back(selectedCellPos.x + 1);
    // grid_vertices.push_back(selectedCellPos.y + 29.f);
    //
    // grid_vertices.push_back(selectedCellPos.x+1);
    // grid_vertices.push_back(selectedCellPos.y+1);
    // grid_vertices.push_back(selectedCellPos.x + 99.f);
    // grid_vertices.push_back(selectedCellPos.y + 1);
    //
    // grid_vertices.push_back(selectedCellPos.x+1);
    // grid_vertices.push_back(selectedCellPos.y+29.f);
    // grid_vertices.push_back(selectedCellPos.x + 99.f);
    // grid_vertices.push_back(selectedCellPos.y + 29.f);
    //
    // grid_vertices.push_back(selectedCellPos.x+99.f);
    // grid_vertices.push_back(selectedCellPos.y+1);
    // grid_vertices.push_back(selectedCellPos.x + 99.f);
    // grid_vertices.push_back(selectedCellPos.y + 29.f);
}

class View {

public:
    GLFWwindow *window;
    // create vbo and vao
    GLuint vbo, vao;
    GLuint text_vao, text_vbo;
    Model& model;
    GLuint shader_program;
    GLuint text_shader_program;

    std::vector<float> grid_vertices;
    std::vector<float> shadow_coords;


    explicit View(Model& model): model(model) {}

    ~View() {
        // cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteProgram(shader_program);

        glDeleteVertexArrays(1, &text_vao);
        glDeleteBuffers(1, &text_vbo);
        glDeleteProgram(text_shader_program);

        glfwDestroyWindow(window);
        glfwTerminate();
    }

    int initGl() {
        // initialize glfw
        if (!glfwInit()) {
            return -1;
        }

        fmt::print("{}\n", __LINE__);


        // configure glfw
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, config["gl_samples"].get<int>());

        fmt::print("{}\n", __LINE__);


        // create window
        window = glfwCreateWindow(config["width"], config["height"], "Grid", nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);

        fmt::print("{}\n", __LINE__);

        // initialize glad
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            return -1;
        }

        fmt::print("{}\n", __LINE__);

        // set viewport
        glViewport(0,0, config["width"], config["height"]);

        shader_program = create_shader_program(shaders::vertex_shader_source, shaders::fragment_shader_source);
        text_shader_program = create_shader_program(shaders::text_vertex_shader_source, shaders::text_fragment_shader_source);

        load_font_textures(config["font"].get<std::string>().c_str());

        glUseProgram(shader_program);

        fmt::print("{}\n", __LINE__);

        set_projection(glm::vec2(0), glm::vec2(config["width"].get<float>(), config["height"].get<float>()));


        grid_vertices.insert(grid_vertices.end(), {0,0,0,0,0,0,0,0,0,0,0,0});

        auto selected_cell = model.selected_cell_.get();
        auto selectedCellPos = glm::vec2((selected_cell.x-1) * config["col_width"].get<float>(), (selected_cell.y-1) * config["row_height"].get<float>());
        changeSelectedCell(grid_vertices, selectedCellPos);

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


        setSelectedCellColor(shader_program, glm::vec4(0.f, 0.25f, 1.f, 1.0f), selectedCellPos, glm::vec2( config["col_width"].get<float>(), config["row_height"].get<float>()));


        glGenVertexArrays(1, &text_vao);
        glGenBuffers(1, &text_vbo);
        glBindVertexArray(text_vao);
        glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        fmt::print("{}\n", __LINE__);

        // setup imgui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        fmt::print("{}\n", __LINE__);



        // set clear color to white for better visibility
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        return 0;
    }

    void setupWatchers() {

        fmt::print("{}\n", __LINE__);

        model.selected_cell_.subscribe([=](glm::vec2 selected_cell, auto _) {

            auto selectedCellPos = glm::vec2((selected_cell.x-1) * config["col_width"].get<float>(), (selected_cell.y-1) * config["row_height"].get<float>());

            changeSelectedCell(grid_vertices, selectedCellPos);
            setSelectedCellColor(shader_program, glm::vec4(0.f, 0.25f, 1.f, 1.0f), selectedCellPos, glm::vec2(config["col_width"].get<float>(), config["row_height"].get<float>()));

            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, grid_vertices.size() * sizeof(float), grid_vertices.data(), GL_STATIC_DRAW);

            render();

        });

        // fmt::print("{}\n", __LINE__);

        model.view_size_.subscribe([this](glm::vec2 dims, auto _) {
            // fmt::print("{}\n", __LINE__);

            glViewport(config["padding"].get<float>(), -config["padding"].get<float>(), dims.x, dims.y);
            set_projection(model.view_origin_.get(), dims);

            render();
        });

        model.view_origin_.subscribe([this](glm::vec2 view_origin, auto _) {
            set_projection(view_origin, model.view_size_.get());

            render();
        });

        // fmt::print("{}\n", __LINE__);

        model.spreadsheet_model_.subscribe([=](auto j, auto i, const std::string& value) {
            // fmt::print("{}\n", __LINE__);
            // fmt::print("{}\n", value);

            // draw_text_in_cell(text_shader_program, text_vao, text_vbo, value, i, j);

            render();
        });

        // fmt::print("{}\n", __LINE__);
    }

    void render() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // render
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shader_program);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 6); // draw selected cell, with drop shadow if needed
        glDrawArrays(GL_LINES, 6, grid_vertices.size() / 2 + 6);



        // render spreadsheet
        for (size_t j = 1; j <= model.spreadsheet_model_.n_cols; ++j) {
            for (size_t i = 1; i <= model.spreadsheet_model_.n_rows; ++i) {
                draw_text_in_cell(text_shader_program, text_vao, text_vbo, model.spreadsheet_model_.getCellValue(j, i), i, j);
            }
        }



        //////////////////////// Render text /////////////////////
        // std::ostringstream textInput;
        // textInput << "Hello, OpenGL! Compiled at " << __TIME__;
        // std::string text = std::move(textInput.str());

        // draw_text_in_cell(text_shader_program, text_vao, text_vbo, text, 2, 2);
        // draw_text_in_cell(text_shader_program, text_vao, text_vbo, std::string("D4"), 4,4);
        /////////////////////////Render text /////////////////////

        // ImGui::Begin("Framerate", nullptr, ImGuiWindowFlags_NoMove);
        // ImGui::SetWindowPos(ImVec2(0,0));
        //
        // framerate_calculator.frame();
        // ImGui::Text("%f", framerate_calculator.getFPS());
        //
        // ImGui::End();



        // ImGui::Begin("Event Log", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        // ImGui::SetWindowPos(ImVec2(io.DisplaySize.x - ImGui::GetWindowWidth(), io.DisplaySize.y - ImGui::GetWindowHeight()));
        //
        // // for (auto event = event_log.size() > 100 ? event_log.end()-100 : event_log.begin(); event != event_log.end(); event++) {
        // //     ImGui::Text("%s", event->c_str());
        // // }
        //
        // ImGui::End();

        // render imgui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
private:


    void set_projection(glm::vec2 view_origin, glm::vec2 view_size) {
        auto padding = config["padding"].get<float>();

        glm::mat4 projection = glm::ortho(
            -padding+view_origin.x,
            view_size.x-padding+view_origin.x,
            view_size.y-padding+view_origin.y,
            -padding+view_origin.y,
            -1.0f, 1.0f); // adjust for top-left origin

        glUseProgram(shader_program);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glUseProgram(text_shader_program);
        glUniformMatrix4fv(glGetUniformLocation(text_shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    }




};



#endif //VIEW_HPP
