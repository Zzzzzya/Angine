#version 330 core

in vec2 TexCoords;
out float FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D gShadowMap;
uniform sampler2D texNoise;
uniform sampler2D gViewPos;

uniform vec3 samples[64];
uniform mat4 projection;
uniform mat4 view;

const vec2 noiseScale = vec2(1600.0 / 4.0, 900 / 4.0);
const int kernelSize = 64;
const float radius = 1.0f;

void main() {
    vec3 fragPos = texture(gViewPos, TexCoords).xyz;
    vec3 normal = texture(gNormal, TexCoords).xyz;
    vec3 randomVec = texture(texNoise, TexCoords * noiseScale).xyz;

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0f;
    for (int i = 0; i < kernelSize; i++) {
        vec3 sample = TBN * samples[i];
        sample = fragPos + sample * radius;

        vec4 offset = vec4(sample, 1.0f);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float sampleDepth = -texture(gPosition, offset.xy).w;
        float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= sample.z ? 1.0f : 0.0f) * rangeCheck;
    }
    occlusion = 1.0f - (occlusion / kernelSize);
    FragColor = occlusion;
}