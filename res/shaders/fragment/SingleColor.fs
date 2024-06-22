#version 330 core

uniform vec4 singleColor;

out vec4 FragColor;

void main(){
    FragColor = singleColor;
}