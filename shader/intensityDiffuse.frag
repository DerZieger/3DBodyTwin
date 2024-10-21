#version 330

layout(location = 0) out vec4 out_col;// color

in vec3 N;
in vec4 L;
in vec4 P;
in vec4 pv_color;

uniform vec4 color;

void main() {
    vec3 viewDir = normalize(-P.xyz);
    vec3 normal = normalize(N);
    vec4 color2 = pv_color;
    if (pv_color.x < 0){
        color2 = color;
    }
    out_col = vec4(color2.xyz * max(dot(viewDir, normal), 0),color2.w);
}
