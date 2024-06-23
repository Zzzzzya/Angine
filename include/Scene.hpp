#pragma once

#ifndef SCENE_HPP
#define SCENE_HPP

#include "Header.hpp"
#include "Light.hpp"
#include "Model.hpp"
#include "Camera.hpp"

class Scene {
  public:
    vector<shared_ptr<Model>> models;
    vector<shared_ptr<PointLightModel>> pointLights;
    shared_ptr<Camera> camera;
};

#endif