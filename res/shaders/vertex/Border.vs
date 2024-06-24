#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normalMatrix;

void main(){
    vec3 FragPos = vec3(model * vec4(aPos+0.1*aNormal,1.0f));
    gl_Position = projection * view * vec4(FragPos,1.0f);
}