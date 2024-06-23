#pragma once

#include "Header.hpp"
struct Material {
    vec3 ambient = vec3(0.2f, 0.2f, 0.2f);
    vec3 diffuse = vec3(0.5f, 0.5f, 0.5f);
    vec3 specular = vec3(0.0f, 0.0f, 0.0f);
    float shininess = 64.0f;
};