#include "Mesh.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

Mesh::Mesh(const vector<Vertex> &vertice, const vector<unsigned int> &indices, const vector<Texture> &textures)
    : vertice(vertice), indices(indices), textures(textures) {
    setupMesh();
}

void Mesh::Draw(shared_ptr<Shader> &shader) {
    shader->use();

    int diffuseNr = 1;
    int specularNr = 1;
    int normalNr = 1;
    int heightNr = 1;

    int albedoNr = 1;
    int metallicNr = 1;
    int roughnessNr = 1;
    int aoNr = 1;

    shader->setBool("ExistDiffuseTexture", false);
    shader->setBool("ExistSpecularTexture", false);
    shader->setBool("ExistNormalTexture", false);
    shader->setBool("ExistHeightTexture", false);

    shader->setBool("ExistAlbedoTexture", false);
    shader->setBool("ExistMetallicTexture", false);
    shader->setBool("ExistRoughnessTexture", false);
    shader->setBool("ExistAoTexture", false);

    // 设置纹理 -> 这里保证shader中各种纹理的命名是这种确定的格式
    // 在Model自动读入的过程中，会设置纹理的type
    for (int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string number;
        std::string name = textures[i].type;
        if (name == "texture_diffuse") {
            number = std::to_string(diffuseNr++);
            shader->setBool("ExistDiffuseTexture", true);
        }
        else if (name == "texture_specular") {
            number = std::to_string(specularNr++);
            shader->setBool("ExistSpecularTexture", true);
        }
        else if (name == "texture_normal") {
            number = std::to_string(normalNr++);
            shader->setBool("ExistNormalTexture", true);
        }
        else if (name == "texture_height") {
            number = std::to_string(heightNr++);
            shader->setBool("ExistHeightTexture", true);
        }
        else if (name == "texture_albedo") {
            number = std::to_string(albedoNr++);
            shader->setBool("ExistAlbedoTexture", true);
        }
        else if (name == "texture_metallic") {
            number = std::to_string(metallicNr++);
            shader->setBool("ExistMetallicTexture", true);
        }
        else if (name == "texture_roughness") {
            number = std::to_string(roughnessNr++);
            shader->setBool("ExistRoughnessTexture", true);
        }
        else if (name == "texture_ao") {
            number = std::to_string(heightNr++);
            shader->setBool("ExistAoTexture", true);
        }

        shader->setInt(name + number, i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, (unsigned int)indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    for (int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
}

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertice.size() * sizeof(Vertex), &vertice[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, position)));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, normal)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, texCoords)));
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, tangent)));

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    glBindVertexArray(0);
}

QuadMesh2D::QuadMesh2D(const vector<Vertex2D> &vers) : vertice(vers) {

    setupQuadMesh();
}

void QuadMesh2D::Draw(shared_ptr<Shader> &Shader) {
    Shader->use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void QuadMesh2D::setupQuadMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertice.size() * sizeof(Vertex2D), &vertice[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void *)offsetof(Vertex2D, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void *)offsetof(Vertex2D, texCoords));

    glBindVertexArray(0);
}

ArrayMesh::ArrayMesh(const vector<Vertex> &vertice) : vertice(vertice), textures(vector<Texture>()) {
    setupMesh();
}

ArrayMesh::ArrayMesh(const vector<Vertex> &vertice, const vector<Texture> &textures)
    : vertice(vertice), textures(textures) {
    setupMesh();
}

void ArrayMesh::Draw(shared_ptr<Shader> &Shader) {
    Shader->use();

    int diffuseNr = 1;
    int specularNr = 1;
    int normalNr = 1;
    int heightNr = 1;

    Shader->setBool("ExistDiffuseTexture", false);
    Shader->setBool("ExistSpecularTexture", false);
    Shader->setBool("ExistNormalTexture", false);
    Shader->setBool("ExistHeightTexture", false);

    // 设置纹理 -> 这里保证shader中各种纹理的命名是这种确定的格式
    // 在Model自动读入的过程中，会设置纹理的type
    for (int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string number;
        std::string name = textures[i].type;
        if (name == "texture_diffuse") {
            number = std::to_string(diffuseNr++);
            Shader->setBool("ExistDiffuseTexture", true);
        }
        else if (name == "texture_specular") {
            number = std::to_string(specularNr++);
            Shader->setBool("ExistSpecularTexture", true);
        }
        else if (name == "texture_normal") {
            number = std::to_string(normalNr++);
            Shader->setBool("ExistNormalTexture", true);
        }
        else if (name == "texture_height") {
            number = std::to_string(heightNr++);
            Shader->setBool("ExistHeightTexture", true);
        }

        Shader->setInt(name + number, i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, (unsigned int)vertice.size());

    glBindVertexArray(0);
    for (int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void ArrayMesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertice.size() * sizeof(Vertex), &vertice[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, position)));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, normal)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, texCoords)));

    glBindVertexArray(0);
}
