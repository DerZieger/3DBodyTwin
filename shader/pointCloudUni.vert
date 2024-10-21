#version 330
layout(location = 0) in vec3 in_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform vec4 color;

out vec4 col;

void main() {
    col = color;
    gl_Position = proj * view * model * vec4(in_pos, 1.0);
}