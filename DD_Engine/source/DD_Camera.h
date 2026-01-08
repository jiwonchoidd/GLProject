#pragma once
#include "DD_GLHelper.h"

class DD_Camera
{
public:
    DD_Camera();
    ~DD_Camera();

    void SetPosition(const Vec3& pos) { m_position = pos; UpdateView(); }
    void SetTarget(const Vec3& target) { m_target = target; UpdateView(); }
    void SetUp(const Vec3& up) { m_up = up; UpdateView(); }

    void UpdateProjection(int width, int height);
    void UpdateView();

    const Matrix4& GetViewMatrix() const { return m_view; }
    const Matrix4& GetProjectionMatrix() const { return m_projection; }

private:
    Vec3 m_position;
    Vec3 m_target;
    Vec3 m_up;
    Matrix4 m_view;
    Matrix4 m_projection;
    float m_fov;
};