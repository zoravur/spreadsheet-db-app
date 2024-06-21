//
// Created by zoravur on 6/19/24.
//

#ifndef SHADERS_HPP
#define SHADERS_HPP

namespace shaders {


//     const char* vertex_shader_source = R"(
// #version 330 core
// layout(location = 0) in vec2 aPos;
// uniform mat4 projection;
// void main() {
//     gl_Position = projection * vec4(aPos, 0.0, 1.0);
// }
// )";
//
//     const char* fragment_shader_source = R"(
// #version 330 core
// out vec4 FragColor;
// void main() {
//     FragColor = vec4(0.0, 0.0, 0.0, 1.0);
// }
// )";
//
//
//     const char* text_vertex_shader_source = R"(
// #version 330 core
// layout(location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
// out vec2 TexCoords;
//
// uniform mat4 projection;
//
// void main() {
//     gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
//     TexCoords = vertex.zw;
// }
// )";
//
//
//     const char* text_fragment_shader_source = R"(
// #version 330 core
// in vec2 TexCoords;
// out vec4 FragColor;
//
// uniform sampler2D text;
// uniform vec3 textColor;
//
// void main() {
//     vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
//     FragColor = vec4(textColor, 1.0) * sampled;
// }
// )";


const char* vertex_shader_source = R"(
#version 330 core

layout(location = 0) in vec2 aPos; // vertex position

uniform mat4 projection;
out vec2 fragPos; // pass the position to the fragment shader

void main() {
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
    fragPos = aPos.xy;
}
)";

const char* fragment_shader_source = R"(
#version 330 core

in vec2 fragPos; // interpolated fragment position from the vertex shader
out vec4 FragColor;

uniform vec4 selectedCellColor; // color of the selected cell
uniform vec2 selectedCellPos;   // position of the selected cell
uniform vec2 cellSize;          // size of each cell

void main() {
    // determine if the current fragment is in the selected cell
    if (fragPos.x >= selectedCellPos.x && fragPos.x <= selectedCellPos.x + cellSize.x &&
        fragPos.y >= selectedCellPos.y && fragPos.y <= selectedCellPos.y + cellSize.y) {
        FragColor = selectedCellColor; // set the color of the selected cell
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0); // default color
    }
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

}

#endif