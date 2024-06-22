#pragma once

#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <glm/glm.hpp>

class Shader {
  public:
    unsigned int pro = -1;
    Shader(const std::string &vertexPath, const std::string &fragmentPath,
           const std::string &vertexDirectory = "../res/shaders/vertex/",
           const std::string &fragmentDirectory = "../res/shaders/fragment/");
    void use();

    // Uniform functions
    // 1
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;

    // 2
    void setVec2(const std::string &name, const glm::vec2 &value) const;

    // 3
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setMat3(const std::string &name, const glm::mat3 &value) const;

    // 4
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setMat4(const std::string &name, const glm::mat4 &value) const;

  private:
    void SetUpShader(const std::string &vertexName, const std::string &fragmentName, const std::string &vertexDirectory,
                     const std::string &fragmentDirectory);
};

#endif