#include "Model.hpp"

// 加载filename的Model
Model::Model(const std::string &filename, shared_ptr<Shader> &shader, int i) : shader(shader) {
    if (i == 0)
        loadModel(filename);
    else
        loadModel(i, filename);
}

mat4 Model::ModelMat() {
    model = glm::mat4(1.0f);

    model = glm::translate(model, translate);
    model = glm::scale(model, scale);
    model = glm::rotate(model, radians(rotate.x), vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, radians(rotate.y), vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, radians(rotate.z), vec3(0.0f, 0.0f, -1.0f));

    return model;
}

void Model::Draw() {
    for (auto &mesh : meshes) {
        mesh.Draw(shader);
    }
}

/*
Assimp!!! -- 介绍一下
Assimp会将各种不同形式的模型加载进一个
Scene对象！
由Node组织Meshes，每个Node包含一个mMeshes -> 子Node包含的是Mesh的索引 得到SceneRoot节点中的Meshes里面来找
每个Mesh会有一个mMaterialIndex是材质索引 要到根节点的mMaterials里面找具体的材质
每个Mesh有mVertices mNormals mTextureCoords三个表，表示顶点的数据
然后有mFaces --> mIndices来组织顶点

*/

// 真正的加载函数
void Model::loadModel(const std::string &filename) {
    Assimp::Importer importer;
    auto path = "../res/models/" + filename;

    // 读取Scene对象!                              |     自动生成法线     |     自动拆为三角形       |    反转UV |
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::clog << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    auto lastl = path.find_last_of('/');
    directory = path.substr(0, lastl) + '/';
    name = path.substr(lastl, path.length());
    processNode(scene->mRootNode, scene);
}

void Model::loadModel(int i, const std::string &filename) {
    Assimp::Importer importer;
    auto path = filename;

    // 读取Scene对象!                              |     自动生成法线     |     自动拆为三角形       |    反转UV |
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::clog << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    auto lastl = path.find_last_of('/');
    directory = path.substr(0, lastl) + '/';
    name = path.substr(lastl, path.length());
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode *node, const aiScene *scene) {
    // 树状的节点 采用递归来把每个node中的meshes拆进meshes里
    for (int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    vector<Vertex> vertice;
    vector<unsigned int> indices;
    vector<Texture> textures;

    // VAO && VBO -- 顶点数据
    for (int i = 0; i < mesh->mNumVertices; i++) {
        Vertex v;
        auto &mVer = mesh->mVertices[i];
        auto &mNor = mesh->mNormals[i];
        auto &mTexCoors = mesh->mTextureCoords[0][i];
        v.position = vec3(mVer.x, mVer.y, mVer.z);
        v.normal = vec3(mNor.x, mNor.y, mNor.z);
        if (mesh->mTextureCoords[0]) {
            v.texCoords = vec2(mTexCoors.x, mTexCoors.y);
        }
        else
            v.texCoords = vec2(0.0f);
        vertice.push_back(v);
    }

    // EBO -- 索引数据
    for (int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // 纹理
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }
    return Mesh(vertice, indices, textures);
}

vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string &typeName) {

    // 遍历给定纹理类型的所有纹理位置，获取纹理的文件位置
    vector<Texture> textures;
    for (int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++) {
            if (std::strcmp(textures_loaded[j].path.c_str(), str.C_Str()) == 0) {
                textures.push_back(textures_loaded[j]);
                skip = true;
                break;
            }
        }
        if (!skip) { // 如果纹理还没有被加载，则加载它
            Texture texture(str.C_Str(), directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture); // 添加到已加载的纹理中
        }
    }
    return textures;
}

PointLightModel::PointLightModel(shared_ptr<Shader> &shader, const PointLight &tlight)
    : Model("Sphere/Sphere.obj", shader), light(tlight) {
    this->translate = this->light.position;
}

mat4 PointLightModel::ModelMat() {
    mat4 model(1.0f);

    model = glm::translate(model, light.position);
    model = glm::scale(model, scale);
    return model;
}

void PointLightModel::updatePosition(double curTime) {
    if (goRoundY) {
        auto r = sqrt(light.position.x * light.position.x + light.position.z * light.position.z);
        light.position.x = cos(radians(curTime * 100)) * r;
        light.position.z = sin(radians(curTime * 100)) * r;
    }
}
