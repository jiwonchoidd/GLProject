#include "DD_Mesh.h"

DD_Mesh::DD_Mesh() : m_vao(0), m_vbo(0), m_ibo(0), m_vertexBuffer(0), m_indexBuffer(0), m_indexCount(0), m_color{ 1.0f, 1.0f, 1.0f, 1.0f }
{

}

DD_Mesh::~DD_Mesh()
{
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vertexBuffer) glDeleteBuffers(1, &m_vertexBuffer);
    if (m_indexBuffer) glDeleteBuffers(1, &m_indexBuffer);
}
