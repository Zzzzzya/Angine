#version 330 core

// Camera / EYE
struct Camera {
    vec3 position;
    vec3 front;
};

// Light
struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    // 光强衰减
    float constant;
    float ones;
    float secs;
};

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gShadowMap;

uniform sampler2D ssao;
uniform bool ssaoOn;

out vec4 FragColor;

uniform Camera camera;
uniform int lightNum;
uniform PointLight light[10];
uniform mat4 projection;

void main() {
    vec3 realNormal;
    realNormal = normalize(texture(gNormal, TexCoords).rgb);

    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    float AmbientOcclusion = texture(ssao, TexCoords).r;
    if (!ssaoOn)
        AmbientOcclusion = 1.0f;

    vec3 result = vec3(0.0f);
    for (int i = 0; i < lightNum; i++) {
        vec3 LightDir = light[i].position - FragPos;
        vec3 NormalLightDir = normalize(LightDir);
        float LightDis = length(LightDir);
        float reduce = 1 / (light[i].constant + light[i].ones * LightDis + light[i].secs * LightDis * LightDis);

        // ambient
        vec3 diffColor = vec3(0.0f);
        diffColor = texture(gAlbedoSpec, TexCoords).rgb;
        vec3 ambient = light[i].ambient * AmbientOcclusion * diffColor;

        // diffuse
        float diff = max(dot(realNormal, NormalLightDir), 0);
        vec3 diffuse = diff * light[i].diffuse * diffColor;

        // specular
        vec3 ViewDir = camera.position - FragPos;
        vec3 NormalViewDir = normalize(ViewDir);
        vec3 LightReflect = normalize(reflect(-NormalLightDir, realNormal));
        // vec3 h = normalize(NormalLightDir + NormalViewDir);
        // float spec = pow(max(dot(h,Normal),0.0f),material.shininess);
        float spec = pow(max(dot(LightReflect, NormalViewDir), 0.0f), 64);
        vec3 specColor = vec3(texture(gAlbedoSpec, TexCoords).a);
        vec3 specular = spec * light[i].specular * specColor;
        float shadow = texture(gShadowMap, TexCoords).r;

        result = result + reduce * (ambient + diffuse + specular) * (1 - shadow);
    }

    FragColor = vec4(result, 1.0f);
}