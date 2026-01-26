#include "DD_Actor.h"
#include "DD_Component.h"
#include "DD_MeshComponent.h"
#include "DD_CollisionComponent.h"

DD_Actor::DD_Actor()
    : meshComp(nullptr)
    , collisionComp(nullptr)
    , m_active(true)
{
}

DD_Actor::~DD_Actor()
{
    // Detach all components
    for (auto* comp : m_components)
    {
        if (comp) comp->OnDetach();
    }
    m_components.clear();
}

void DD_Actor::AddComponent(DD_Component* component)
{
    if (!component) return;
    
    // Check if already added
    for (auto* comp : m_components)
    {
        if (comp == component) return;
    }

    m_components.push_back(component);
    component->OnAttach(this);
}

void DD_Actor::RemoveComponent(DD_Component* component)
{
    if (!component) return;

    auto it = std::find(m_components.begin(), m_components.end(), component);
    if (it != m_components.end())
    {
        (*it)->OnDetach();
        m_components.erase(it);
    }
}

void DD_Actor::SetMeshComponent(DD_MeshComponent* mesh)
{
    meshComp = mesh;
}

void DD_Actor::SetCollisionComponent(DD_CollisionComponent* coll)
{
    collisionComp = coll;
}

Matrix4 DD_Actor::GetModelMatrix() const
{
    return BuildModelMatrix(m_transform);
}

void DD_Actor::Update(float deltaTime)
{
    if (!m_active) return;

    // Update all components
    for (auto* comp : m_components)
    {
        if (comp && comp->IsEnabled())
        {
            comp->Update(deltaTime);
        }
    }
}

void DD_Actor::Render(const Matrix4& view, const Matrix4& projection)
{
    if (!m_active) return;

    if (meshComp)
    {
        meshComp->MeshRender(GetModelMatrix(), view, projection);
    }
}
