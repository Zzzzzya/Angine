#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normalMatrix;
uniform mat4 lightSpaceMatrix;

out vec3 Normal;
out vec3 FragPos;
out vec4 ViewPos;
out vec2 TexCoords;
out vec4 FragPosLightSpace;

void main() {
    Normal = (normalMatrix * aNormal);
    TexCoords = aTexCoords;
    vec4 ModelPos = model * vec4(aPos, 1.0f);
    ViewPos = view * ModelPos;

    FragPos = ModelPos.xyz;
    gl_Position = projection * ViewPos;
    FragPosLightSpace = lightSpaceMatrix * ModelPos;
}