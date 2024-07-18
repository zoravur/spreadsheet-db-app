//
// Created by zoravur on 6/21/24.
//

#ifndef TEXT_HPP
#define TEXT_HPP

#include <../lib/glad/include/glad/glad.h>
#include <GLFW/glfw3.h>
#include "../lib/imgui/imgui.h"
#include "../lib/imgui/backends/imgui_impl_glfw.h"
#include "../lib/imgui/backends/imgui_impl_opengl3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <iostream>
#include <ft2build.h>
#include <map>
#include <iostream>
#include <fstream>

#include FT_FREETYPE_H

#include <nlohmann/json.hpp>
#include "shaders.hpp"
#include "FrameRateCalculator.hpp"
#include "InputController.hpp"
extern json config;


std::map<char, GLuint> character_textures;
std::map<char, glm::ivec2> character_sizes;
std::map<char, glm::ivec2> character_bearings;
std::map<char, GLuint> character_advances;

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

    FT_Set_Pixel_Sizes(face, 0, config["row_height"].get<int>()-2*config["cell_padding"].get<int>());
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

void compute_cell_text_coords(int j, int i, float *x, float *y) {
    *x = config["col_width"].get<float>() * (static_cast<float>(j) - 1) + (config["cell_padding"].get<float>());
    *y = config["row_height"].get<float>() * (static_cast<float>(i) - 1) - (config["cell_padding"].get<float>()/2);
}


void draw_text_in_cell(GLuint text_shader_program, GLuint text_vao, GLuint text_vbo, const std::string& text, int i, int j) {
    GLfloat x, y;
    compute_cell_text_coords(j, i, &x, &y);

    constexpr auto color =  glm::vec3(1.0f, 0.0f, 0.0f);

    glUseProgram(text_shader_program);
    glUniform3f(glGetUniformLocation(text_shader_program, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(text_vao);

    glDisable(GL_TEXTURE_2D);

    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        GLuint texture = character_textures[*c];
        // glUniformMatrix4fv(glGetUniformLocation(text_shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(text_projection));;
        glm::ivec2 size = character_sizes[*c];
        glm::ivec2 bearing = character_bearings[*c];
        GLuint advance = character_advances[*c];

        GLfloat xpos = x + bearing.x;
        GLfloat ypos = y + (size.y - bearing.y);
        GLfloat w = size.x;
        GLfloat h = size.y;

        float yposoffset = config["row_height"].get<float>() - config["cell_padding"].get<float>();

        GLfloat vertices[6][4] = {
            { xpos, yposoffset + ypos - h, 0.0f, 0.0f },
            { xpos, yposoffset + ypos, 0.0f, 1.0f },
            { xpos + w, yposoffset + ypos, 1.0f, 1.0f },

            { xpos, yposoffset + ypos - h, 0.0f, 0.0f },
            { xpos + w, yposoffset + ypos, 1.0f, 1.0f },
            { xpos + w, yposoffset + ypos - h, 1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, texture);

        glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (advance >> 6); // bitshift by 6 to get value in pixels
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

#endif //TEXT_HPP
