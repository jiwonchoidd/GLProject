#pragma once
#include "DD_GLHelper.h"

class DD_Mesh;
class DD_Material;

class DD_MeshComponent
{
public:
    DD_MeshComponent();
    explicit DD_MeshComponent(DD_Mesh* m);
    ~DD_MeshComponent();

public:
    void MeshRender(const Matrix4& model, const Matrix4& view, const Matrix4& projection);

    // Mesh
    void SetMesh(DD_Mesh* m) { m_mesh = m; }
    DD_Mesh* GetMesh() const { return m_mesh; }

    // Material
    void SetMaterial(DD_Material* mat) { m_material = mat; }
    DD_Material* GetMaterial() const { return m_material; }

    // Visibility
    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }

    // Cast shadow
    void SetCastShadow(bool cast) { m_castShadow = cast; }
    bool GetCastShadow() const { return m_castShadow; }

    // Receive shadow
    void SetReceiveShadow(bool receive) { m_receiveShadow = receive; }
    bool GetReceiveShadow() const { return m_receiveShadow; }

private:
    DD_Mesh* m_mesh;
    DD_Material* m_material;
    bool m_visible;
    bool m_castShadow;
    bool m_receiveShadow;
};
