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
    vec3 color;

    // 光强衰减
    float constant;
    float ones;
    float secs;
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;
in vec4 FragPosLightSpace;
out vec4 FragColor;

const float PI = 3.14159265359;

uniform Camera camera;
uniform int lightNum;
uniform PointLight light[10];

// PBR Material

uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

uniform sampler2D shadowMap;
uniform sampler2D texture_albedo1;
uniform sampler2D texture_metallic1;
uniform sampler2D texture_roughness1;
uniform sampler2D texture_ao1;

uniform bool ExistAlbedoTexture;
uniform bool ExistMetallicTexture;
uniform bool ExistRoughnessTexture;
uniform bool ExistAoTexture;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform bool ExistDiffuseTexture;
uniform bool ExistSpecularTexture;
uniform bool ExistNormalTexture;
uniform bool ExistHeightTexture;

uniform samplerCube irradianceMap;

// BRDF
// D F G / (4 * (Wi dot n)* (Wo dot n))
// F 菲尼尔项
vec3 fresnelShlick(float cosTheta, vec3 F0) {
    return F0 + (1 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// D 法线分布函数
float DistributionGGX(vec3 N, vec3 H, float r) {
    float a = r * r;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = NdotH2 * (a2 - 1.0f) + 1.0f;
    denom = PI * denom * denom;

    return num / denom;
}
// G 几何函数
float GeometrySchlickGGX(vec3 N, vec3 V, float r) {
    float r1 = (r + 1.0f);
    float k = (r1 * r1) / 8.0f;
    float NdotV = dot(N, V);

    float num = NdotV;
    float denum = NdotV * (1 - k) + k;

    return num / denum;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float r) {
    return GeometrySchlickGGX(N, V, r) * GeometrySchlickGGX(N, L, r);
}

void main() {
    vec3 the_albedo = albedo;
    float the_metallic = metallic;
    float the_ao = 1.0f;
    float the_roughness = roughness;
    vec3 normal = Normal;

    if (ExistAlbedoTexture) {
        the_albedo.r = pow(texture(texture_albedo1, TexCoords).r, 2.2);
        the_albedo.g = pow(texture(texture_albedo1, TexCoords).g, 2.2);
        the_albedo.b = pow(texture(texture_albedo1, TexCoords).b, 2.2);
    }

    if (ExistMetallicTexture)
        the_metallic = texture(texture_metallic1, TexCoords).r;
    if (ExistRoughnessTexture)
        the_roughness = texture(texture_roughness1, TexCoords).r;
    if (ExistAoTexture)
        the_ao = texture(texture_ao1, TexCoords).r;

    vec3 N = normalize(normal);
    vec3 V = normalize(camera.position - FragPos);

    vec3 Lo = vec3(0.0f);
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, the_albedo, the_metallic);
    for (int i = 0; i < lightNum; i++) {
        // 对所有光源做累加计算
        // 1.可预计算部分
        vec3 Li = normalize(light[i].position - FragPos);
        vec3 H = normalize(Li + V); // 半程向量

        float lightDis = length(light[i].position - FragPos);
        float attenuation = 1.0f / (lightDis * lightDis);
        vec3 radiance = light[i].color * attenuation;

        float NDF = DistributionGGX(N, H, the_roughness);
        float G = GeometrySmith(N, V, Li, the_roughness);

        vec3 F = fresnelShlick(max(dot(H, V), 0.0f), F0);

        vec3 nominator = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0f) * max(dot(N, Li), 0.0f) + 0.001;

        vec3 kS = F;
        vec3 kD = vec3(1.0f) - kS;
        kD *= 1.0f - the_metallic;

        vec3 specular = nominator / denominator;
        vec3 diffuse = the_albedo / PI;

        float NdotL = max(dot(N, Li), 0.0f);
        Lo += (kD * diffuse + specular) * radiance * NdotL;
    }
    vec3 kS = fresnelShlick(max(dot(N, V), 0.0f), F0);
    vec3 kD = 1.0f - kS;
    kD *= 1.0f - the_metallic;
    vec3 irradiance = texture(irradianceMap, normal).rgb;
    vec3 diffuse = irradiance * the_albedo;
    vec3 ambient = kD * diffuse * the_ao;
    vec3 color = ambient + Lo;
    // color = color / (color + vec3(1.0f));
    // color = pow(color, vec3(1.0f / 2.2f));

    FragColor = vec4(color, 1.0f);
}