#include "DD_MeshComponent.h"
#include "DD_Mesh.h"

void DD_MeshComponent::MeshRender(const Matrix4& model, const Matrix4& view, const Matrix4& projection)
{
    if (!mesh_) return;
    mesh_->PrepareForRender(model, view, projection);
}
