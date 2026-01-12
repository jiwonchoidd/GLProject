#pragma once
#include "DD_GLHelper.h"

class DD_Mesh
{
public:
    DD_Mesh();
    virtual ~DD_Mesh();
public:
    virtual void CreateMesh() = 0;

    // Hook for mesh to set shader and uniforms before draw
    virtual void PrepareForRender(const Matrix4& model, const Matrix4& view, const Matrix4& projection) = 0;

    // Accessors for GL buffers/count
    GLuint GetVAO() const { return m_vao; }
    GLuint GetVertexBuffer() const { return m_vertexBuffer; }
    GLuint GetIndexBuffer() const { return m_indexBuffer; }
    int GetIndexCount() const { return m_indexCount; }

protected:
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
    GLuint m_vertexBuffer;
    GLuint m_indexBuffer;
    int m_indexCount;
    Color m_color;
};
