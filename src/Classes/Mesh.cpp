#include "Mesh.h"

void Mesh::setupMesh()
{
    glGenVertexArrays(1, &_VAO);
        glGenBuffers(1, &_VBO);
        glGenBuffers(1, &_EBO);

        glBindVertexArray(_VAO);
        
        glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        float totalSize =
            Positions.size()    * sizeof(glm::vec3) +
            Normals.size()      * sizeof(glm::vec3) +
            TexCoords.size()    * sizeof(glm::vec2) +
            Tangents.size()     * sizeof(glm::vec3) +
            BoneIDsArray.size() * sizeof(int) * MAX_BONE_INFLUENCE +
            WeightsArray.size() * sizeof(float) * MAX_BONE_INFLUENCE ;

        auto currentOffset = 0;
        glBufferData(GL_ARRAY_BUFFER, totalSize, NULL, GL_STATIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER, currentOffset, Positions.size() * sizeof(glm::vec3), &Positions[0]);
        currentOffset += Positions.size() * sizeof(glm::vec3);

        glBufferSubData(GL_ARRAY_BUFFER, currentOffset, Normals.size() * sizeof(glm::vec3), &Normals[0]);
        currentOffset += Normals.size() * sizeof(glm::vec3);

        glBufferSubData(GL_ARRAY_BUFFER, currentOffset,
            TexCoords.size() * sizeof(glm::vec2), &TexCoords[0]);
        currentOffset += TexCoords.size() * sizeof(glm::vec2);
        
        glBufferSubData(GL_ARRAY_BUFFER, currentOffset,
            Tangents.size() * sizeof(glm::vec3), &Tangents[0]);
        currentOffset += Tangents.size() * sizeof(glm::vec3);
        
        glBufferSubData(GL_ARRAY_BUFFER, currentOffset ,BoneIDsArray.size() * MAX_BONE_INFLUENCE * sizeof(int),
            &BoneIDsArray[0]);
        currentOffset += BoneIDsArray.size() * MAX_BONE_INFLUENCE * sizeof(int);
        
        glBufferSubData(GL_ARRAY_BUFFER, currentOffset,  WeightsArray.size() * MAX_BONE_INFLUENCE * sizeof(float),
            &WeightsArray[0]);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE,  3 * sizeof(float),
            (void*)0);
        glEnableVertexAttribArray(0);
        
        glVertexAttribPointer(1,3,GL_FLOAT, GL_FALSE, 3* sizeof(float),
            (void*)(Positions.size() * sizeof(glm::vec3)));
        glEnableVertexAttribArray(1);
        
        glVertexAttribPointer(2,2,GL_FLOAT, GL_FALSE, 2 * sizeof(float),
            (void*)(Positions.size() * sizeof(glm::vec3) +
            Normals.size() * sizeof(glm::vec3)));
        glEnableVertexAttribArray(2);
        
        glVertexAttribPointer(3,3,GL_FLOAT, GL_FALSE, 3 * sizeof(float),
            (void*)(Positions.size() * sizeof(glm::vec3) +
            Normals.size() * sizeof(glm::vec3) +
            TexCoords.size() * sizeof(glm::vec2)));
        glEnableVertexAttribArray(3);
        
        glVertexAttribIPointer(4,4,GL_INT, 4 * sizeof(int),
            (void*)(Positions.size() * sizeof(glm::vec3) +
            Normals.size()     * sizeof(glm::vec3) +
            TexCoords.size()   * sizeof(glm::vec2) +
            Tangents.size()    * sizeof(glm::vec3)));
        glEnableVertexAttribArray(4);
        
        glVertexAttribPointer(5,4,GL_FLOAT, GL_FALSE, 4 * sizeof(float),
            (void*)(Positions.size() * sizeof(glm::vec3) +
            Normals.size()      * sizeof(glm::vec3) +
            TexCoords.size()    * sizeof(glm::vec2) +
            Tangents.size()     * sizeof(glm::vec3) +
            BoneIDsArray.size() * MAX_BONE_INFLUENCE * sizeof(int)));
        glEnableVertexAttribArray(5);
    
        glBindVertexArray(0);
}
