#version 330 core
layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;
layout(location = 3) out vec3 gShadowMap;
layout(location = 4) out vec4 gViewPos;

// Material
struct Material {
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

in vec4 FragPosLightSpace;
in vec2 TexCoords;
in vec3 FragPos;
in vec4 ViewPos;
in vec3 Normal;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform bool ExistDiffuseTexture;
uniform bool ExistSpecularTexture;
uniform Material material;
uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace) {
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0)
        return 0.0;

    float shadow = 0.0f;
    float bias = 0.0002f;
    float currentDepth = projCoords.z;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -6; x <= 6; ++x) {
        for (int y = -6; y <= 6; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 13 * 13;
    return shadow;
}

const float NEAR = 0.1f;
const float FAR = 100.0f;
float LinearizeDepth(float d) {
    float z = 2 * d - 1;
    return (2.0f * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));
}

void main() {
    gPosition.xyz = FragPos;
    gPosition.w = LinearizeDepth(gl_FragCoord.z);
    gNormal = normalize(Normal);
    if (ExistDiffuseTexture)
        gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;
    else
        gAlbedoSpec.rgb = material.diffuse;

    if (ExistSpecularTexture)
        gAlbedoSpec.a = texture(texture_specular1, TexCoords).r;
    else
        gAlbedoSpec.a = material.specular.r;

    gShadowMap = vec3(ShadowCalculation(FragPosLightSpace));
    gViewPos = ViewPos;
}