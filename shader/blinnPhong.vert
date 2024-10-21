#version 330
#define SOLUTION

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTc;

uniform mat4 model;
uniform mat4 model_normal;
uniform mat4 view;
uniform mat4 view_normal;
uniform mat4 proj;
uniform vec4 lightDir;


out vec3 N;
out vec4 L;
out vec4 P;
out vec2 TC;
out vec4 pv_color;

void main()
{
    mat3 normalMatrix = transpose(inverse(mat3(view * model)));

    N  = normalize(normalMatrix * vNormal.xyz);
    L  = view * lightDir;
    P  = view * model * vec4(vPosition, 1);
    TC = vTc;
    pv_color = vec4(-1);
    gl_Position = proj * P;

}
