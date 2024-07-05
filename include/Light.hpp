#pragma once

#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "Header.hpp"

struct PointLight {
    vec3 position = vec3(0.8f);
    vec3 ambient = vec3(0.0f);
    vec3 diffuse = vec3(0.3f);
    vec3 specular = vec3(0.8f);
    vec3 color = vec3(0.0f);

    float constant;
    float ones;
    float secs;
};

#endif