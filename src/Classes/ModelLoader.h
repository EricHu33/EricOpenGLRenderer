#ifndef MODEL_LOADER_CLASS_H
#define MODEL_LOADER_CLASS_H

#include "Shader.h"
#include "Mesh.h"
#include "Material.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Renderer.h"
#include <robin_hood/robin_hood.h>
struct BoneInfo
{
    int id;
    glm::mat4 offset;
};

class ModelLoader
{
public:
    ModelLoader(){};
    ModelLoader(const char* path);
    std::vector<Mesh>* GetMeshes() { return &_meshes; }
    std::vector<Material> GetMaterials(Material materialPrototype);
    void Dispose();
    robin_hood::unordered_map<size_t, BoneInfo>& GetBoneInfoMap(){ return _boneInfoMap; }
    int& GetBoneCount(){ return _boneCounter; }
private:
    // model data
    std::unordered_map<std::string, Texture> _textures_loaded;
    robin_hood::unordered_map<size_t, BoneInfo> _boneInfoMap;
    int _boneCounter = 0;
    std::string _directory;
    std::string _fileName;
    std::vector<Mesh> _meshes;
    void ExtractBoneWeightForVertices(
        std::vector<std::array<int, MAX_BONE_INFLUENCE>>& boneIds,
        std::vector<std::array<float, MAX_BONE_INFLUENCE>>& weights,
        aiMesh* mesh, const aiScene* scene);
    void loadModel(std::string path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    Texture loadMaterialTextures(aiMaterial* mat, aiTextureType type,bool isSRGB);
    unsigned int loadTextureFromFile(const char *path, const std::string &directory, bool isSRGB);
};

#endif