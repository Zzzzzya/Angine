#pragma once

#ifndef LIGHT_HPP
#define LIGHT_HPP

#include "Header.hpp"

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float ones;
    float secs;
};

#endif