#pragma once
#ifndef MESH_HPP
#define MESH_HPP

#include "Header.hpp"

/* 顶点类 */
struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texCoords;
};

class Texture;
class Shader;
/* 网格 - 一套物体 */
class Mesh {
  public:
    /*网格数据 --> 顶点(顶点位置 + 法线 + 纹理坐标) + 索引 + 纹理的集合 */
    vector<Vertex> vertice;
    vector<unsigned int> indices;
    vector<Texture> textures;

    Mesh(const vector<Vertex> &vertice, const vector<unsigned int> &indices, const vector<Texture> &textures);
    void Draw(Shader &shader);

  private:
    unsigned int VAO, VBO, EBO;
    void setupMesh();
};

#endif