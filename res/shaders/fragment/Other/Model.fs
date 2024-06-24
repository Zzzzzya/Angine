#version 330 core

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D texture_diffuse1;

void main(){
    FragColor = texture(texture_diffuse1,TexCoords);
}