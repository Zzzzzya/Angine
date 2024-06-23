#version 330 core

// Camera / EYE
struct Camera{
    vec3 position;
    vec3 front;
};

// Light
struct PointLight{
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
struct Material{
    vec3 diffuse;
    vec3 specular;
    float shininess;
};


in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

out vec4 FragColor;

uniform Camera camera;
uniform PointLight light;
uniform Material material;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main(){
    vec3 LightDir = light.position - FragPos;
    vec3 NormalLightDir = normalize(LightDir);
    float LightDis = length(LightDir);
    float reduce = 1 / (light.constant + light.ones * LightDis + light.secs * LightDis * LightDis);

    // ambient
    vec3 diffColor = vec3(texture(texture_diffuse1,TexCoords));
    if(diffColor == vec3(0.0f,0.0f,0.0f)){
        diffColor = material.diffuse;
    }
    vec3 ambient = light.ambient * diffColor;

    // diffuse
    float diff = max(dot(Normal,NormalLightDir),0);
    vec3 diffuse = diff * light.diffuse * diffColor;


    // specular
    vec3 ViewDir = camera.position - FragPos;
    vec3 NormalViewDir = normalize(ViewDir);
    vec3 LightReflect = normalize(reflect(-NormalLightDir,Normal));
    float spec = pow(max(dot(NormalViewDir,LightReflect),0.0f),material.shininess);
    vec3 specColor = vec3(texture(texture_specular1,TexCoords));
    if(specColor == vec3(0.0f,0.0f,0.0f)){
        specColor = material.specular;
    }
    vec3 specular = spec * light.specular * specColor;

    vec3 result = reduce * (ambient + diffuse + specular);
    FragColor = vec4(result,1.0f);
}