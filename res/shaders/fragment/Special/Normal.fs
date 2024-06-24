#version 330 core

in vec3 Normal;

out vec4 FragColor;

void main(){
    FragColor = vec4((Normal +vec3(1.0f))*0.5,1.0f);
}