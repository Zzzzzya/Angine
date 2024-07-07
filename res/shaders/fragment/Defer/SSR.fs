#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gShadowMap;
uniform sampler2D texNoise;
uniform sampler2D gViewPos;

uniform vec3 cameraPos;
uniform mat4 projection;
uniform mat4 view;

const float radius = 0.5f;
const int SAMPLE_NUM = 64;
uniform vec3 samples[SAMPLE_NUM];

const float NEAR = 0.1f;
const float FAR = 100.0f;
float GetDepth(vec3 pos) {
    vec4 a = projection * view * vec4(pos, 1.0f);
    a = a / a.w;
    float z = a.z;
    return (2.0f * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));
}

vec2 GetScreenCoordinate(vec3 pos) {
    vec4 a = projection * view * vec4(pos, 1.0f);
    a = a / a.w;
    vec2 uv = a.xy * 0.5 + 0.5;
    return uv;
}

bool RayHit(vec3 ori, vec3 dir, out vec3 hitPos) {
    // 步长
    float step = 0.05;
    // 步数
    const int stepNums = 150;
    // 发射射线的方向
    vec3 stepDir = dir;
    // 每次射线前进的距离
    vec3 stepDistance = stepDir * step;
    // 从原点开始出发
    vec3 curPos = ori;
    for (int cursteps = 0; cursteps < stepNums; cursteps++) {
        vec2 uv = GetScreenCoordinate(curPos);
        // 通过对比深度判断射线是否有交点
        float rayDepth = GetDepth(curPos);
        float gBufferDepth = texture(gPosition, uv).w;
        // 如果在一定范围内存在交点则执行下面逻辑
        if (rayDepth - gBufferDepth > 0.0001) {
            // 通过hitPos传出该交点
            hitPos = curPos;
            return true;
        }
        curPos += stepDistance;
    }
    return false;
}

vec3 GetAlbedoColor(vec3 pos) {
    vec2 uv = GetScreenCoordinate(pos);
    return texture(gAlbedoSpec, uv).xyz;
}

void main() {
    vec3 WorldFragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;

    vec3 ViewDir = normalize(WorldFragPos - cameraPos);
    vec3 reflectDir = normalize(reflect(ViewDir, Normal));

    vec3 result = vec3(0.0f);
    int realSample = 0;
    for (int i = 0; i < 1; i++) {
        vec3 sampleDir = normalize(reflectDir + radius * samples[i]);
        if (dot(Normal, sampleDir) <= 0)
            continue;

        vec3 hitPos;
        if (RayHit(WorldFragPos, reflectDir, hitPos)) {
            vec3 hitColor = GetAlbedoColor(hitPos);
            result += hitColor;
            realSample++;
        }
    }

    result /= float(realSample);
    FragColor = vec4(result, 1.0f);
}