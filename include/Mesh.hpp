#pragma once
#ifndef MESH_HPP
#define MESH_HPP

#include "Header.hpp"

/* 顶点类 */
struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 texCoords;

    vec3 tangent;
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
    unsigned int VAO, VBO, EBO;

    Mesh(const vector<Vertex> &vertice, const vector<unsigned int> &indices, const vector<Texture> &textures);
    void Draw(shared_ptr<Shader> &Shader);

  private:
    void setupMesh();
};

class ArrayMesh {
  public:
    unsigned int VAO, VBO;
    vector<Vertex> vertice;
    vector<Texture> textures;
    ArrayMesh(const vector<Vertex> &vertice);
    ArrayMesh(const vector<Vertex> &vertice, const vector<Texture> &textures);
    void Draw(shared_ptr<Shader> &Shader);

  private:
    void setupMesh();
};

struct Vertex2D {
    vec2 position;
    vec2 texCoords;
};

class QuadMesh2D {
  public:
    vector<Vertex2D> vertice;
    unsigned int VAO, VBO;
    QuadMesh2D(const vector<Vertex2D> &vers);
    void Draw(shared_ptr<Shader> &Shader);

  private:
    void setupQuadMesh();
};

#endif