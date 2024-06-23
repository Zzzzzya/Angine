#pragma once
#ifndef MODEL_HPP
#define MODEL_HPP

#include "Header.hpp"

#include "Shader.hpp"
#include "Texture.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Light.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model {
  public:
    vector<Texture> textures_loaded; // 用来存储已经加载过的纹理
    vector<Mesh> meshes;             // 读入的model会是一系列mesh的集合
    std::string directory;
    std::string name;

    vec3 translate = vec3(0.0f);
    vec3 scale = vec3(1.0f);
    vec3 rotate = vec3(0.0f);

    mat4 model = mat4(1.0f);

    shared_ptr<Shader> shader;

    Material mat = Material();
    vec4 ObjectColor = vec4(1.0f);
    Model(const std::string &filename, shared_ptr<Shader> &shader, int i = 0);
    virtual mat4 ModelMat();
    void Draw();

  private:
    void loadModel(const std::string &filename);
    void loadModel(int i, const std::string &filename);
    void processNode(aiNode *node, const aiScene *scene);
    Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string &typeName);
};

class PointLightModel : public Model {
  public:
    PointLightModel(shared_ptr<Shader> &shader, const PointLight &light = PointLight());
    PointLight light;
    bool goRoundY = false;
    virtual mat4 ModelMat() override;
    void updatePosition(double curTime);
};
#endif