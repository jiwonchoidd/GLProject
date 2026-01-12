#include "DD_Actor.h"
#include "DD_MeshComponent.h"

DD_Actor::DD_Actor() {}
DD_Actor::~DD_Actor() {}

Matrix4 DD_Actor::GetModelMatrix() const
{
    return BuildModelMatrix(m_transform);
}

void DD_Actor::Render(const Matrix4& view, const Matrix4& projection)
{
    if (meshComp)
    {
        meshComp->MeshRender(GetModelMatrix(), view, projection);
    }
}
