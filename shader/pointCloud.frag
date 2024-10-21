#version 330
in vec4 col;

layout (location = 0) out vec4 out_col;

void main() {
    out_col = col;
}
