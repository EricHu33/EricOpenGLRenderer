#include "ModelLoader.h"
#include <iostream>
#include <stb/stb_image.h>
#include <assimp/postprocess.h>
#include "../assimp_glm_helpers.h"

ModelLoader::ModelLoader(const char* path)
{
    loadModel(path);
}

void ModelLoader::Dispose()
{
    for(int i =0; i < _meshes.size();i++)
    {
        _meshes[i].Dispose();
    }
}

std::vector<Material> ModelLoader::GetMaterials(Material materialPrototype)
{
    std::vector<Material> mats;
    for(int i = 0; i < _meshes.size();i++)
    {
        auto mat = materialPrototype;
        auto texures = _meshes[i].textures;
        for(auto texture : texures)
        {
            if(texture.samplerName == "_baseMap")
            {
                mat.SetTexture("_baseMap", texture.id, texture.Path.c_str());
            }
            else if(texture.samplerName == "_normalMap")
            {
                mat.SetTexture("_normalMap", texture.id, texture.Path.c_str());
            }
            else if(texture.samplerName == "_occlusionRoughMetalMap")
            {
                mat.SetTexture("_occlusionRoughMetalMap", texture.id, texture.Path.c_str());
            }
        }
        mats.push_back(mat);
    }
    return mats;
}


void ModelLoader::loadModel(std::string path)
{
    _fileName = path;
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path,   aiProcess_Triangulate | aiProcess_GenSmoothNormals  | aiProcess_CalcTangentSpace );	
    float scaleFactor = 0.0f;
    scene->mMetaData->Get("UnitScaleFactor", scaleFactor);
    printf("scale :  %f\n", scaleFactor);
    //std::string list;
    //import.GetExtensionList(list);
    //std::cout << list << std::endl;
    
    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
    {
        std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }
    _directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}

void ModelLoader::processNode(aiNode* node, const aiScene* scene)
{
    // process all the node's meshes (if any)
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        std::string str;
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        _meshes.push_back(processMesh(mesh, scene));			
    }
    // then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uv0;
    std::vector<glm::vec3> tangents;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    std::vector<std::array<int, MAX_BONE_INFLUENCE>> boneIDsArray;
    std::vector<std::array<float, MAX_BONE_INFLUENCE>> weightsArray;

    //process vertices
    for(unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        //initialize bones

        std::array<int, MAX_BONE_INFLUENCE> boneIds;
        std::array<float, MAX_BONE_INFLUENCE> weights;

        for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
            boneIds[i] = -1;
            weights[i] = 0.0f;
        }
        boneIDsArray.push_back(boneIds);
        weightsArray.push_back(weights);
        
        positions.push_back(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
        if (mesh->HasNormals())
        {
            normals.push_back(glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
        }
       
        if(mesh->HasTextureCoords(0))
        {
            uv0.push_back( glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
        }
        else
        {
            uv0.push_back(  glm::vec2(0));
        }
        
        tangents.push_back(glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z));
    }
    // process indices
    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    ExtractBoneWeightForVertices(boneIDsArray, weightsArray ,mesh, scene);
    
    // process material
    unsigned int noMat = 0;
    bool isTransparent = false;
    if(mesh->mMaterialIndex >= noMat)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        unsigned int noTexture = -1;
        aiString texturePath ;
        material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
        std::cout << "material : "<< material->GetName().C_Str() << std::endl;
        for (int p = 0; p < material->mNumProperties; p++)
        {
            aiMaterialProperty* pt = material->mProperties[p];
            if (strcmp(pt->mKey.C_Str(), "$mat.gltf.alphaMode") == 0)
            {
                if (pt->mDataLength > 0 && pt->mData)
                {
                    aiString& s = *((aiString*)pt->mData);
                   if(strcmp(s.C_Str(), "BLEND") == 0)
                   {
                       isTransparent = true;
                      std::cout<<"found alpha mode property!"<<std::endl;
                       std::cout<<"mode : " << s.C_Str() <<std::endl;
                   }

                }
            }
        }
        //std::cout <<" diffuseTextureCount : " <<material->GetTextureCount(aiTextureType_DIFFUSE)<<std::endl;
        //std::cout <<" expect texture name : " << texturePath.C_Str() << std::endl;
        Texture diffuse = loadMaterialTextures(material, aiTextureType_DIFFUSE,true);
        if(diffuse.id != noTexture)
        {
           // std::cout <<"success load base map";
            textures.push_back(diffuse);
        }

        Texture specMap = loadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, false);
        if(specMap.id != noTexture)
        {
            textures.push_back(specMap);
        }

        Texture normalMap = loadMaterialTextures(material, aiTextureType_NORMALS, false);
        if(normalMap.id != noTexture)
        {
            textures.push_back(normalMap);
        }
        //copy texture
    }

    return Mesh(positions, normals, uv0,tangents, boneIDsArray, weightsArray, indices, textures, isTransparent);
}

Texture ModelLoader::loadMaterialTextures(aiMaterial* mat, aiTextureType type, bool isSRGB)
{
    
    aiString texturePath ;
    mat->GetTexture(type, 0, &texturePath);
    auto samplerName = "_baseMap";
    if(type == aiTextureType_DIFFUSE)
    {
        samplerName = "_baseMap";
    }
    else if(type == aiTextureType_NORMALS)
    {
        samplerName = "_normalMap";
    }
    else if(type == aiTextureType_DIFFUSE_ROUGHNESS)
    {
        samplerName = "_occlusionRoughMetalMap";
    }
    //std::cout << samplerName  << " : " << texturePath.C_Str() << std::endl;
    
    Texture texture;
    texture.id = -1;
    // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
    if(_textures_loaded.count(texturePath.C_Str()) > 0)
    {
        std::cout<<"early return"<<std::endl;
        return  _textures_loaded[texturePath.C_Str()];
    }
    texture.id = loadTextureFromFile(texturePath.C_Str(), this->_directory, isSRGB);
    texture.samplerName = samplerName;
    texture.Path = this->_directory + '/' + texturePath.C_Str();
    _textures_loaded[texturePath.C_Str()] = texture;
   
    return texture;
}

unsigned int ModelLoader::loadTextureFromFile(const char* path, const std::string& directory, bool isSRGB)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;
    unsigned int textureId = -1;
    int width, height, nrChannels;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format;
        GLenum internalFormat;
        if (nrChannels == 1)
        {
            format = GL_RED;
            internalFormat =  GL_RED;
        }
        else if (nrChannels == 3)
        {
            format = GL_RGB;
            internalFormat = isSRGB ? GL_SRGB : GL_RGB;
        }
        else if (nrChannels == 4)
        {
            format = GL_RGBA;
             internalFormat = isSRGB ? GL_SRGB_ALPHA : GL_RGBA;
        }
        
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height,0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        std::cout << "success load texture at " << filename << std::endl;
    }
    else
    {
        textureId = -1;
        //std::cout << "Failed to load texture at " << filename << std::endl;
    }
    stbi_image_free(data);
    return textureId;
}

void ModelLoader::ExtractBoneWeightForVertices(std::vector<std::array<int, 4>>& boneIdsArray,
    std::vector<std::array<float, 4>>& weightsArray, aiMesh* mesh, const aiScene* scene)
{
    auto& boneInfoMap = _boneInfoMap;
    int& boneCount = _boneCounter;

    for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
    {
        int boneID = -1;
        size_t boneNameHash = std::hash<std::string>{}(mesh->mBones[boneIndex]->mName.C_Str());
        if (boneInfoMap.find(boneNameHash) == boneInfoMap.end())
        {
            BoneInfo newBoneInfo;
            newBoneInfo.id = boneCount;
            newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
            boneInfoMap[boneNameHash] = newBoneInfo;
            boneID = boneCount;
            boneCount++;
        }
        else
        {
            boneID = boneInfoMap[boneNameHash].id;
        }
        assert(boneID != -1);
        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
        {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;
            assert(vertexId <= boneIdsArray.size());
            for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
            {
                if (boneIdsArray[vertexId][i] < 0)
                {
                    boneIdsArray[vertexId][i] = boneID;
                    weightsArray[vertexId][i] = weight;
                    break;
                }
            }
        }
    }
}


