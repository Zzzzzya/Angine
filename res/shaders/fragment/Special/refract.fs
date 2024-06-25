#version 330 core

struct Camera {
    vec3 position;
    vec3 front;
};

uniform Camera camera;

in vec3 Normal;
in vec3 TexCoords;

out vec4 FragColor;

uniform samplerCube skybox;
float refractRatio = 1.0 / 1.52;

void main() {
    vec3 I = normalize(TexCoords - camera.position);
    vec3 R = refract(I, normalize(Normal), refractRatio);
    FragColor = texture(skybox, R);
}