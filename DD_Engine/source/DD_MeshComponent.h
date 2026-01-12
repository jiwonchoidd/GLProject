#pragma once
#include "DD_GLHelper.h"

class DD_Mesh;

class DD_MeshComponent
{
public:
    DD_MeshComponent() : mesh_(nullptr) {}
    explicit DD_MeshComponent(DD_Mesh* m) : mesh_(m) {}
public:
    void MeshRender(const Matrix4& model, const Matrix4& view, const Matrix4& projection);

    void SetMesh(DD_Mesh* m) { mesh_ = m; }
    DD_Mesh* GetMesh() const { return mesh_; }
private:
    DD_Mesh* mesh_;
};
