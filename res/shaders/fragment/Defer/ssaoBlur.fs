#version 330 core
in vec2 TexCoords;
out float FragColor;

uniform sampler2D ssao;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(ssao, 0));
    float result = 0.0;

    for (int i = -2; i < 2; i++) {
        for (int j = -2; j < 2; j++) {
            vec2 offset = vec2(float(i), float(j)) * (texelSize);
            result += texture(ssao, TexCoords + offset).r;
        }
    }

    FragColor = result / (16.0f);
}