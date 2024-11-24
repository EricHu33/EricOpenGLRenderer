#include "Renderer.h"

#include <stb/stb_image.h>

Renderer::Renderer(Mesh* meshes, Material materials) 
{
    m_meshe = meshes;
    m_material = materials;
}

Material* Renderer::GetMainMaterial()
{
    return &m_material;
}

void Renderer::DrawWithShader(Shader& shader)
{
    m_meshe->Draw(shader);
}

void Renderer::Draw()
{
    m_material.UpdateVariablesToShader();
    m_meshe->Draw((*m_material.GetShader()));
}

void Renderer::DrawInstanced(int count)
{
    m_material.UpdateVariablesToShader();
    m_meshe->DrawInstanced((*m_material.GetShader()), count);
}

void Renderer::Dispose()
{
}






