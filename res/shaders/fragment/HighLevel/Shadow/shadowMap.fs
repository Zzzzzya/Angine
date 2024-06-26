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

// Material
struct Material {
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

uniform sampler2D shadowMap;

out vec4 FragColor;

uniform Camera camera;
uniform int lightNum;
uniform PointLight light[10];
uniform Material material;
uniform vec4 ObjectColor;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

uniform bool ExistDiffuseTexture;
uniform bool ExistSpecularTexture;
uniform bool ExistNormalTexture;
uniform bool ExistHeightTexture;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir) {
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0)
        return 0.0;

    float shadow = 0.0f;
    float bias = 0.0002f;
    float currentDepth = projCoords.z;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -3; x <= 3; ++x) {
        for (int y = -3; y <= 3; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 49.0;
    return shadow;
}

void main() {
    vec3 result = vec3(0.0f);
    for (int i = 0; i < lightNum; i++) {
        vec3 LightDir = light[i].position - FragPos;
        vec3 NormalLightDir = normalize(LightDir);
        float LightDis = length(LightDir);
        float reduce = 1 / (light[i].constant + light[i].ones * LightDis + light[i].secs * LightDis * LightDis);

        // ambient
        vec3 diffColor = vec3(0.0f);
        if (!ExistDiffuseTexture) {
            diffColor = material.diffuse;
        }
        else {
            diffColor = vec3(texture(texture_diffuse1, TexCoords));
        }
        vec3 ambient = light[i].ambient * diffColor;

        // diffuse
        float diff = max(dot(Normal, NormalLightDir), 0);
        vec3 diffuse = diff * light[i].diffuse * diffColor;

        // specular
        vec3 ViewDir = camera.position - FragPos;
        vec3 NormalViewDir = normalize(ViewDir);
        vec3 LightReflect = normalize(reflect(-NormalLightDir, Normal));
        // vec3 h = normalize(NormalLightDir + NormalViewDir);
        // float spec = pow(max(dot(h,Normal),0.0f),material.shininess);
        float spec = pow(max(dot(LightReflect, NormalViewDir), 0.0f), material.shininess);
        vec3 specColor = vec3(0.0f);
        if (!ExistSpecularTexture) {
            specColor = material.specular;
        }
        else {
            specColor = vec3(texture(texture_specular1, TexCoords));
        }
        vec3 specular = spec * light[i].specular * specColor;

        float shadow = ShadowCalculation(FragPosLightSpace, NormalLightDir);

        result = result + reduce * (ambient + (1.0 - shadow) * (diffuse + specular));
    }

    FragColor = vec4(result, 1.0f) * ObjectColor;
}