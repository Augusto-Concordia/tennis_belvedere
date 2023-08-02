//default shadow mapper fragment shader

#version 330 core

in vec4 gl_FragCoord;

layout(location = 0) out vec4 out_color; //rgba color output

//entrypoint
void main() {
    //eliminates the harsh edge of the shadow map, comes from: https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
    gl_FragDepth = gl_FragCoord.z > 1.0 ? 0.0 : gl_FragCoord.z;

    //out_color = vec4(vec3(gl_FragCoord.z), 1.0f);
}