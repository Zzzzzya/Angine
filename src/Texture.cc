#include "Texture.hpp"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "stb_image.h"

Texture::Texture() {
}

Texture::Texture(const std::string &filename, const std::string &directory) {
    loadTexture(filename, directory);
}

void Texture::loadTexture(const std::string &filename, const std::string &directory) {
    path = directory + filename;

    int width, height, nrComponents;
    auto data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (!data) {
        std::clog << "ERROR::TEXTURE::LOAD TEXTURE FAILED ON PATH " << path << std::endl;
        stbi_image_free(data);
        return;
    }
    GLenum format = (nrComponents == 1) ? GL_RED : (nrComponents == 3 ? GL_RGB : GL_RGBA);
    unsigned int textureID;
    glGenTextures(1, &textureID);
    id = textureID;
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D); // 纹理记得生成MipMap！
    setTexParam();
    stbi_image_free(data);
    return;
}

void Texture::setTexParam() {
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
}
