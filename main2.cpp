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

std::map<char, GLuint> character_textures;
std::map<char, glm::ivec2> character_sizes;
std::map<char, glm::ivec2> character_bearings;
std::map<char, GLuint> character_advances;

#include FT_FREETYPE_H

// FT_Library library;
// FT_Face face;

GLuint shader_program;
GLuint text_shader_program;
int width = 800;
int height = 600;
int padding = 20;


float cell_height = 30.0f;
float cell_width = 100.0f;

void load_font_textures(const char* font_path) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "Could not init FreeType Library" << std::endl;
        return;
    }

    FT_Face face;
    if (FT_New_Face(ft, font_path, 0, &face)) {
        std::cerr << "Failed to load font" << std::endl;
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, 30);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    for (GLubyte c = 0; c < 128; c++) {
        // load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // store character information
        character_textures[c] = texture;
        character_sizes[c] = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
        character_bearings[c] = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
        character_advances[c] = face->glyph->advance.x;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}



const char* vertex_shader_source = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
uniform mat4 projection;
void main() {
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
}
)";

const char* fragment_shader_source = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
)";


const char* text_vertex_shader_source = R"(
#version 330 core
layout(location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";


const char* text_fragment_shader_source = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D text;
uniform vec3 textColor;

void main() {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    FragColor = vec4(textColor, 1.0) * sampled;
}
)";



void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0.0f, 0.0f, width, height);

    glm::mat4 projection = glm::ortho(-(float)padding, (float)width-(float)padding, float(height)-(float)padding, -(float)padding, -1.0f, 1.0f); // adjust for top-left origin
    glUseProgram(shader_program);
    int projection_loc = glGetUniformLocation(shader_program, "projection");
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection));

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

GLuint create_text_shader_program() {
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    compile_shader(vertex_shader, text_vertex_shader_source);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    compile_shader(fragment_shader, text_fragment_shader_source);

    // GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, text_vertex_shader_source);
    // GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, text_fragment_shader_source);

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

void compute_cell_text_coords(int i, int j, float *x, float *y) {
    *x = cell_width * (i - 1) + 1.0f;
    *y = cell_height * (j - 1) - 1.0f;
}


int main() {

    std::vector<std::string> event_log;

    // initialize glfw
    if (!glfwInit()) {
        return -1;
    }

    // configure glfw
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Grid", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // initialize glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return -1;
    }

    // set viewport
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // compile shaders
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    compile_shader(vertex_shader, vertex_shader_source);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    compile_shader(fragment_shader, fragment_shader_source);

    // link shaders
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    text_shader_program = create_text_shader_program();

    load_font_textures("/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf");
    // check for linking errors
    int success;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        glGetProgramInfoLog(shader_program, 512, nullptr, info_log);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
    }

    glUseProgram(shader_program); // ensure the shader program is in use
    // initialize the projection matrix
    glm::mat4 projection = glm::ortho(-(float)padding, (float)width-(float)padding, float(height)-(float)padding, -(float)padding, -1.0f, 1.0f); // adjust for top-left origin
    // glm::mat4 projection = glm::ortho(0.0f, (float)window_width, (float)window_height, 0.0f, -1.0f, 1.0f); // adjust for top-left origin
    int projection_loc = glGetUniformLocation(shader_program, "projection");
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection));

    // glUseProgram(text_shader_program);



    std::vector<float> grid_vertices;
    int window_width = 800;
    int window_height = 600;

    // vertical lines
    for (float x = 0.0f; x <= window_width; x += cell_width) {
        float ndc_x = (x);
        grid_vertices.push_back(ndc_x);
        grid_vertices.push_back(1.0f * window_height);
        grid_vertices.push_back(ndc_x);
        grid_vertices.push_back(0.0f);
    }

    // horizontal lines
    for (float y = 0.0f; y <= window_height; y += cell_height) {
        float ndc_y = y; // invert y-axis
        grid_vertices.push_back(1.0f * window_width);
        grid_vertices.push_back(ndc_y);
        grid_vertices.push_back(0.0f);
        grid_vertices.push_back(ndc_y);
    }

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

    glm::mat4 text_proj = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f);

    GLuint text_vao, text_vbo;
    glGenVertexArrays(1, &text_vao);
    glGenBuffers(1, &text_vbo);
    glBindVertexArray(text_vao);
    glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
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

    // render loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            event_log.push_back("Key A Pressed");
        }

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
        
        std::string text = "Hello, OpenGL!";
        

        // GLfloat x = 25.0f, y = 25.0f, scale = 1.0f;
        GLfloat x, y, scale = 1.0f;
        compute_cell_text_coords(4, 6, &x, &y);
        // x = y = 0.0f;
        
        glm::vec3 color =  glm::vec3(1.0f, 0.0f, 0.0f);
        glUseProgram(text_shader_program);
    // glm::mat4 projection = glm::ortho(-(float)padding, (float)width-(float)padding, float(height)-(float)padding, -(float)padding, -1.0f, 1.0f); // adjust for top-left origin
        // glUniformMatrix4fv(glGetUniformLocation(text_shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3f(glGetUniformLocation(text_shader_program, "textColor"), color.x, color.y, color.z);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(text_vao);


        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++) {
            GLuint texture = character_textures[*c];
   // glUniformMatrix4fv(glGetUniformLocation(text_shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(text_projection));;
            glm::ivec2 size = character_sizes[*c];
            glm::ivec2 bearing = character_bearings[*c];
            GLuint advance = character_advances[*c];

            GLfloat xpos = x + bearing.x * scale;
            GLfloat ypos = y + (size.y - bearing.y) * scale;
            GLfloat w = size.x * scale;
            GLfloat h = size.y * scale;

            GLfloat vertices[6][4] = {
                { xpos, 28.0f + ypos - h, 0.0f, 0.0f },
                { xpos, 28.0f + ypos, 0.0f, 1.0f },
                { xpos + w, 28.0f + ypos, 1.0f, 1.0f },

                { xpos, 28.0f + ypos - h, 0.0f, 0.0f },
                { xpos + w, 28.0f + ypos, 1.0f, 1.0f },
                { xpos + w, 28.0f + ypos - h, 1.0f, 0.0f }
            };
            /*
            GLfloat vertices[6][4] = {
                { xpos, ypos - h, 0.0f, 0.0f },
                { xpos, ypos, 0.0f, 1.0f },
                { xpos + w, ypos, 1.0f, 1.0f },

                { xpos, ypos - h, 0.0f, 0.0f },
                { xpos + w, ypos, 1.0f, 1.0f },
                { xpos + w, ypos - h, 1.0f, 0.0f }
            };
            */
            glBindTexture(GL_TEXTURE_2D, texture);

            glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            x += (advance >> 6) * scale; // bitshift by 6 to get value in pixels
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        /////////////////////////Render text /////////////////////
        


        ImGui::Begin("Event Log", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SetWindowPos(ImVec2(io.DisplaySize.x - ImGui::GetWindowWidth(), io.DisplaySize.y - ImGui::GetWindowHeight()));

        for (const auto& event : event_log) {
            ImGui::Text("%s", event.c_str());
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

