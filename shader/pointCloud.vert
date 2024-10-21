#version 330
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec4 in_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec4 col;

void main() {
    col = in_color;
    gl_Position = proj * view * model * vec4(in_pos, 1.0);
}