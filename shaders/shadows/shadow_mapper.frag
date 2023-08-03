//default shadow mapper fragment shader

#version 330 core

in vec4 gl_FragCoord;

layout(location = 1) out vec3 out_color; //rgba color output

// standard value mapping function
float map(float value, float min1, float max1, float min2, float max2) {
    return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

//entrypoint
void main() {
    //eliminates the harsh edge of the shadow map, comes from: https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
    //gl_FragDepth = gl_FragCoord.z > 1.0 ? 0.0 : gl_FragCoord.z;

    out_color = vec3(map(gl_FragCoord.z, 0.1, 400.0, 0.0, 1.0));
}