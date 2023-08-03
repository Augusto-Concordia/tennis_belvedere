//default lit fragment shader

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

uniform vec3 u_cam_pos; //cam position

uniform Light u_lights[3];

uniform vec3 u_color; //color
uniform float u_alpha; //opacity
uniform int u_shininess; //material shininess

uniform float u_shadows_influence = 1.0; //are shadows enabled?
uniform float u_texture_influence = 0.5; //are textures enabled?

uniform sampler2D u_texture; //object texture

in vec3 FragPos;
in vec3 Normal;
in vec4 FragPosLightSpace[3];
in vec2 FragUv;

layout(location = 0) out vec4 out_color; //rgba color output

vec3 calculateSpotLight(Light light, vec4 fragPosLightSpace) {
    //diffuse lighting calculation
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float lightDistance = length(light.position - FragPos);

    //spotlight calculation
    float theta = dot(lightDir, light.spot_dir);

    float diffFactor = max(dot(lightDir, norm), 0.0);
    vec3 diffuse = diffFactor * light.color;

    //specular lighting calculation
    vec3 viewDir = normalize(u_cam_pos - FragPos);
    vec3 reflectDir = normalize(reflect(-lightDir, norm));

    float specularFactor = pow(max(dot(viewDir, reflectDir), 0.0), u_shininess);
    vec3 specular = specularFactor * light.specular_strength * light.color;

    //shadow calculation
    vec3 projectedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projectedCoords = projectedCoords * 0.5 + 0.5;

    // get closest depth value from light's perspective (using [0,1] range LightSpaceFragPos as coords)
    float closestDepth = texture(light.depth_texture, projectedCoords.xy).r;

    // get current linear depth as stored in the depth buffer
    float currentDepth = projectedCoords.z;

    float shadowScalar = (currentDepth - max(0.000100 * (1.0 - dot(norm, lightDir)), 0.000025)) < closestDepth ? 1.0 : light.shadows_influence; //bias calculation comes from: https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping

    vec3 colorResult = (diffuse + specular) * max(theta - light.spot_cutoff, 0.0) * //spotlight
                            shadowScalar * //shadows
                            2.0 / (light.attenuation.x + light.attenuation.y * lightDistance + light.attenuation.z * lightDistance * lightDistance); //attenuation

    return colorResult;
}

//entrypoint
void main() {
    vec3 approximateAmbient = (u_lights[0].ambient_strength * u_lights[0].color + u_lights[1].ambient_strength * u_lights[1].color) / 2.0;
    vec3 colorResult = (approximateAmbient + calculateSpotLight(u_lights[0], FragPosLightSpace[0]) + calculateSpotLight(u_lights[1], FragPosLightSpace[1])) * vec3(mix(vec4(u_color, 1.0), texture(u_texture, FragUv), u_texture_influence)); //pure color or texture, mixed with lighting

    out_color = vec4(colorResult, u_alpha);
}