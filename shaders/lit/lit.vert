//default lit vertex shader

#version 330 core

struct Light {
    vec3 position;
    vec3 color;

    float point_spot_influence;
    float shadows_influence;
    vec3 attenuation;

    float ambient_strength;
    float specular_strength;

    vec3 spot_dir;
    float spot_cutoff;

    mat4 light_view_projection;
    sampler2D depth_texture;
};

uniform Light u_lights[3];

uniform mat4 u_model_transform; //model matrix
uniform mat4 u_view_projection; //view projection matrix

uniform vec2 u_texture_tiling; //texture (uv) tiling

layout (location = 0) in vec3 vPos; //vertex input position
layout (location = 1) in vec3 vNormal; //vertex input normal
layout (location = 2) in vec2 vUv; //vertex input uv

out vec3 Normal;
out vec3 FragPos;
out vec4 FragPosLightSpace[3];
out vec2 FragUv;

void main() {
    Normal = mat3(transpose(inverse(u_model_transform))) * vNormal; //we need to transform the normal with the normal matrix (https://learnopengl.com/Lighting/Basic-Lighting & http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/the-normal-matrix/)

    FragPos = vec3(u_model_transform * vec4(vPos, 1.0));
    FragPosLightSpace[0] = u_lights[0].light_view_projection * vec4(FragPos, 1.0);
    FragPosLightSpace[1] = u_lights[1].light_view_projection * vec4(FragPos, 1.0);
    FragUv = vUv / u_texture_tiling;

    gl_Position = u_view_projection * u_model_transform * vec4(vPos, 1.0); //gl_Position is a built-in property of a vertex shader
}