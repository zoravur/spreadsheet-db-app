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

const char* fragment_shader_source_old = R"(
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


const char* fragment_shader_source = R"(
#version 330 core

in vec2 fragPos; // interpolated fragment position from the vertex shader
out vec4 FragColor;

uniform vec4 selectedCellColor; // color of the selected cell
uniform vec4 shadowColor;       // color of the shadow
uniform vec2 selectedCellPos;   // position of the selected cell
uniform vec2 cellSize;          // size of each cell
uniform float shadowSize;       // size of the shadow extension

void main() {
    // shadowColor = vec4(1.0, 1.0, 0.0, 1.0);

    // calculate the shadow boundaries
    vec2 shadowMin = selectedCellPos - vec2(shadowSize);
    vec2 shadowMax = selectedCellPos + cellSize + vec2(shadowSize);



    // determine if the current fragment is in the selected cell
    if (fragPos.x >= selectedCellPos.x && fragPos.x <= selectedCellPos.x + cellSize.x &&
        fragPos.y >= selectedCellPos.y && fragPos.y <= selectedCellPos.y + cellSize.y) {

        FragColor = selectedCellColor; // set the color of the selected cell


        if (fragPos.x >= selectedCellPos.x + 2 && fragPos.x <= selectedCellPos.x + cellSize.x - 2 &&
            fragPos.y >= selectedCellPos.y + 2 && fragPos.y <= selectedCellPos.y + cellSize.y - 2) {
            FragColor = vec4(0.0, 0.0, 0.0, 0.0);
        }

    } else
    // determine if the current fragment is in the shadow area
    if (fragPos.x >= shadowMin.x && fragPos.x <= shadowMax.x &&
        fragPos.y >= shadowMin.y && fragPos.y <= shadowMax.y) {
        FragColor = shadowColor; // set the color of the shadow
    } else {
        // default color if not in shadow or selected cell
        FragColor = vec4(0.0, 0.0, 0.0, 0.25);
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