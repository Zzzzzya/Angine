#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;



void main(){
    vec4 viewPos = view * model * vec4(aPos,1.0f);
    gl_Position = projection * viewPos;
}