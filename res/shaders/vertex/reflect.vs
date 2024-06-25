#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

void main() {
    TexCoords = vec3(model * vec4(aPos, 1.0f));
    gl_Position = projection * view * vec4(TexCoords, 1.0f);
    Normal = mat3(normalMatrix) * aNormal;
}