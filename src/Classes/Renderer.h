#ifndef RENDERER_H
#define RENDERER_H
#include "Shader.h"
#include "Mesh.h"
#include "Material.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Renderer
{
public:
    Renderer(Mesh* meshes, Material materials);
    Material* GetMainMaterial();
    Material GetMainMaterialCopy(){ return m_material;}
    void SetMaterial(Material mat) { m_material = mat; }
    void Draw();
    void DrawInstanced(int count);
    void DrawWithShader(Shader& shader);
    void Dispose();
    bool IsTransparent = false;
    bool IsBackFaceCull = true;
    bool IsSkinnedMesh = false;
private:
    Mesh* m_meshe;
    Material m_material;
};
#endif