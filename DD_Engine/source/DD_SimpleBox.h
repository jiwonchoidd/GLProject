#pragma once
#include "framework.h"
#include "DD_GLHelper.h"

class DD_SimpleBox
{
public:
	static bool CachePipline();
	static void ClearPipline();
public:
	void Create(const Vec3& pos);
public:
	void Render();
public:
	void AddPos(const Vec3& addpos);
	void AddRot(const Vec3& addrot);
	void SetPos(const Vec3& pos) { m_position = pos; }
	Vec3 GetPos() const { return m_position; }
private:
	GLuint m_vao;
	GLuint m_vbo;
	GLuint m_ibo;
	GLuint m_vertexBuffer;
	GLuint m_indexBuffer;
	int m_indexCount;
private:
	Vec3 m_rotation;
	Vec3 m_position;
	Color m_color;
public:
	DD_SimpleBox();
	virtual ~DD_SimpleBox();
};

