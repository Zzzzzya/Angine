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
uniform sampler2D texture_normal1;

uniform bool ExistDiffuseTexture;
uniform bool ExistSpecularTexture;
uniform bool ExistNormalTexture;
uniform bool ExistHeightTexture;

// 泊松
vec2 poissonDisk[150];

#define NUM_RINGS 10
#define NUM_SAMPLES 150
#define NUM_BLOCK_FILTER 150

#define EPS 1e-3
#define PI 3.141592653589793
#define PI2 6.283185307179586

highp float rand_1to1(highp float x) {
    // -1 -1
    return fract(sin(x) * 10000.0);
}

highp float rand_2to1(vec2 uv) {
    // 0 - 1
    const highp float a = 12.9898, b = 78.233, c = 43758.5453;
    highp float dt = dot(uv.xy, vec2(a, b)), sn = mod(dt, PI);
    return fract(sin(sn) * c);
}

void poissonDiskSamples(const in vec2 randomSeed) {

    float ANGLE_STEP = PI2 * float(NUM_RINGS) / float(NUM_SAMPLES);
    float INV_NUM_SAMPLES = 1.0 / float(NUM_SAMPLES);

    float angle = rand_2to1(randomSeed) * PI2;
    float radius = INV_NUM_SAMPLES;
    float radiusStep = radius;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        poissonDisk[i] = vec2(cos(angle), sin(angle)) * pow(radius, 0.75);
        radius += radiusStep;
        angle += ANGLE_STEP;
    }
}

// PCF
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
    // for (int x = -6; x <= 6; ++x) {
    //     for (int y = -6; y <= 6; ++y) {
    //         float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
    //         shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    //     }
    // }
    // shadow /= 13 * 13;

    // 泊松圆环采样
    float diskRadius = 7.0 / (1024);
    poissonDiskSamples(projCoords.xy);
    for (int i = 0; i < 20; ++i) {
        vec2 offset = poissonDisk[i] * diskRadius;
        float sampleDepth = texture(shadowMap, projCoords.xy + offset).r;
        if (currentDepth < sampleDepth + 0.0002) {
            shadow += 1.0;
        }
    }

    shadow /= 20.0;
    return shadow;
}

const float Wlight = 10.0f;
uniform float a;

float unpack(vec4 rgbaDepth) {
    const vec4 bitShift = vec4(1.0, 1.0 / 256.0, 1.0 / (256.0 * 256.0), 1.0 / (256.0 * 256.0 * 256.0));
    return dot(rgbaDepth, bitShift);
}

float PCSS(vec4 fragPosLightSpace) {

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0f)
        return 0.0f;

    float shadow = 0.0f;
    float currentDepth = projCoords.z;
    float d_blocker = 0.0f;
    int num_d_blocker = 0;
    float bias = 0.0002f;
    // 生成泊松数组
    poissonDiskSamples(projCoords.xy);
    //  1.  对shadowMap做范围深度测试 来查找blocker遮挡物的平均深度
    float del = 10.0 / (1024);
    for (int i = 0; i < NUM_BLOCK_FILTER; i++) {
        vec2 sampleuv = projCoords.xy + poissonDisk[i] * del;
        float sampleDepth = (texture(shadowMap, sampleuv).r);
        if (currentDepth - bias > sampleDepth) {
            d_blocker += sampleDepth;
            num_d_blocker += 1;
        }
    }

    if (num_d_blocker < 1e-7) {
        // 根本没遮挡
        return 0.0f;
    }

    d_blocker = d_blocker / float(num_d_blocker);
    d_blocker = min(d_blocker, projCoords.z);

    // 计算滤波大小
    float Wpen = Wlight * (projCoords.z / d_blocker - 1);

    // 泊松圆环采样
    float diskRadius = Wpen / a;
    // poissonDiskSamples(projCoords.xy);
    for (int i = 0; i < 150; ++i) {
        vec2 offset = poissonDisk[i] * diskRadius;
        float sampleDepth = texture(shadowMap, projCoords.xy + offset).r;
        if (currentDepth - bias > sampleDepth) {
            shadow += 1.0;
        }
    }

    shadow /= 150.0;
    return shadow;
}

void main() {
    vec3 realNormal;
    if (ExistNormalTexture) {
        realNormal = normalize(texture(texture_normal1, TexCoords).rgb * 2 - 1.0f);
    }
    else {
        realNormal = normalize(Normal);
    }

    vec3 result = vec3(0.0f);
    for (int i = 0; i < lightNum; i++) {
        vec3 LightDir = light[i].position - FragPos;
        vec3 NormalLightDir = normalize(LightDir);
        float LightDis = length(LightDir);
        // float reduce = 1 / (light[i].constant + light[i].ones * LightDis + light[i].secs * LightDis * LightDis);
        float reduce = 1.0f;
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
        float diff = max(dot(realNormal, NormalLightDir), 0);
        vec3 diffuse = diff * light[i].diffuse * diffColor;

        // specular
        vec3 ViewDir = camera.position - FragPos;
        vec3 NormalViewDir = normalize(ViewDir);
        vec3 LightReflect = normalize(reflect(-NormalLightDir, realNormal));
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
        // float shadow = PCSS(FragPosLightSpace);
        result = result + reduce * shadow * (ambient + (diffuse + specular));
    }

    FragColor = vec4(result, 1.0f) * ObjectColor;
}