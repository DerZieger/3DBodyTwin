#version 330

layout(location = 0) out vec4 out_col;// color

in vec3 N;
in vec4 L;
in vec4 P;
in vec4 pv_color;

uniform vec4 color;

const float shinyness   = 128.0;
const vec3 ambient_col  = vec3(0.2, 0.2, 0.3);// greyw
const vec3 diffuse_col  = vec3(0.3, 0.6, 0.2);// green
const vec3 specular_col = vec3(0.4, 0.4, 0.4);// white
const vec3 light_color  = vec3(1, 1, 1);

vec4 blinn_phong_reflectance(vec3 N, vec3 L, vec3 V, vec3 light_col, float is_lit)
{
    vec3 H = normalize(L + V);// half way vector
    vec4 color2=pv_color;
    if (pv_color.x<0){
        color2=color;
    }
    vec3 amb  = vec3(color2) * ambient_col;
    vec3 diff = is_lit * max(0.0, dot(L, N)) * vec3(color2) * light_col;
    vec3 spec = is_lit * pow(max(0.0, dot(H, N)), shinyness) * specular_col * light_col;

    return vec4(amb + diff + spec, color2.w);
}

void main()
{
    vec3 viewDir  = normalize(-P.xyz);
    vec3 normal   = normalize(N);

    out_col = blinn_phong_reflectance(normal, vec3(-L), viewDir, light_color, 1.0);
}
