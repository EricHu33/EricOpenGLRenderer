#ifndef MESH_CLASS_H
#define MESH_CLASS_H
#include <string>
#include <vector>
#include <array>
#include <glad/glad.h>
#include <glm/detail/type_vec.hpp>
#include <glm/detail/type_vec2.hpp>
#include <glm/detail/type_vec3.hpp>
#include "Shader.h"

#define MAX_BONE_INFLUENCE 4

struct Texture {
    unsigned int id;
    std::string samplerName;
    std::string Path;
};

class Mesh
{
public:
    unsigned int _VAO, _VBO, _EBO;
    std::vector<glm::vec3> Positions;
    std::vector<glm::vec3> Normals;
    std::vector<glm::vec2> TexCoords;
    std::vector<glm::vec3> Tangents;
    std::vector<std::array<int, MAX_BONE_INFLUENCE>> BoneIDsArray;
    std::vector<std::array<float, MAX_BONE_INFLUENCE>> WeightsArray;
    
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    bool isTransparent;
    Mesh(std::vector<glm::vec3> positions,
        std::vector<glm::vec3> normals,
        std::vector<glm::vec2> texCoords,
        std::vector<glm::vec3> tangents,
        std::vector<std::array<int, MAX_BONE_INFLUENCE>> bonesIdsArray,
        std::vector<std::array<float, MAX_BONE_INFLUENCE>> weightsArray,
        std::vector<unsigned int> indices,
        std::vector<Texture> textures,
        bool isTransparent)
    {
        this->Positions = positions;
        this->Normals = normals;
        this->TexCoords = texCoords;
        this->Tangents = tangents;
        this->BoneIDsArray = bonesIdsArray;
        this->WeightsArray = weightsArray;
        this->indices = indices;
        this->textures = textures;
        this->isTransparent = isTransparent;

        setupMesh();
    }
    
    void Draw(Shader& shader)
    {
        glBindVertexArray(_VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    void DrawInstanced(Shader& shader, int count)
    {
        glBindVertexArray(_VAO);
        glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, count);
        glBindVertexArray(0);
    }

    void Dispose()
    {
        glDeleteVertexArrays(1, &_VAO);
        glDeleteBuffers(1, &_VBO);
        glDeleteBuffers(1, &_EBO);
    }

private:
    void setupMesh();
};
#endif