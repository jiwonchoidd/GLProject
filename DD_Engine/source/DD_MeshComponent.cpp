#include "DD_MeshComponent.h"
#include "DD_Mesh.h"
#include "DD_Material.h"

DD_MeshComponent::DD_MeshComponent()
    : m_mesh(nullptr)
    , m_material(nullptr)
    , m_visible(true)
    , m_castShadow(true)
    , m_receiveShadow(true)
{
}

DD_MeshComponent::DD_MeshComponent(DD_Mesh* m)
    : m_mesh(m)
    , m_material(nullptr)
    , m_visible(true)
    , m_castShadow(true)
    , m_receiveShadow(true)
{
}

DD_MeshComponent::~DD_MeshComponent()
{
}

void DD_MeshComponent::MeshRender(const Matrix4& model, const Matrix4& view, const Matrix4& projection)
{
    if (!m_mesh || !m_visible) return;
    m_mesh->PrepareForRender(model, view, projection);
}
